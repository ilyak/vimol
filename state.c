/*
 * Copyright (c) 2013-2017 Ilya Kaliman
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "vimol.h"

struct state {
	int is_input;
	int is_search;
	int force_quit;
	int index;
	struct bind *bind;
	struct edit *edit;
	struct history *history;
	struct rec *rec;
	struct statusbar *statusbar;
	struct tabs *tabs;
	struct yank *yank;
	SDL_Window *window;
	cairo_t *cairo;
};

static void
key_string(char **buf, SDL_Keycode sym, Uint16 mod)
{
	char cms[5]; /* CMS- */
	const char *key;
	int i = 0;

	memset(cms, 0, sizeof cms);
	if (mod & KMOD_CTRL)
		cms[i++] = 'C';
	if (mod & KMOD_ALT)
		cms[i++] = 'M';
	if (mod & KMOD_SHIFT)
		cms[i++] = 'S';
	if (i > 0)
		cms[i] = '-';
	key = SDL_GetKeyName(sym);
	xasprintf(buf, "%s%s", cms, key);
}

static void
create_window(struct state *state)
{
	state->window = SDL_CreateWindow("vimol",
	    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	    800, 600, SDL_WINDOW_RESIZABLE);
	if (state->window == NULL)
		fatal("%s", SDL_GetError());
}

static int
window_is_resized(struct state *state)
{
	SDL_Surface *sdl_surface;
	cairo_surface_t *cairo_surface;
	int w, h;

	if ((cairo_surface = cairo_get_target(state->cairo)) == NULL)
		fatal("cairo_get_target");
	if ((sdl_surface = SDL_GetWindowSurface(state->window)) == NULL)
		fatal("%s", SDL_GetError());

	w = cairo_image_surface_get_width(cairo_surface);
	h = cairo_image_surface_get_height(cairo_surface);

	return (sdl_surface->w != w || sdl_surface->h != h);
}

static void
create_cairo(struct state *state)
{
	SDL_Surface *sdl_surface;
	cairo_surface_t *cairo_surface;

	if (state->cairo)
		cairo_destroy(state->cairo);

	if ((sdl_surface = SDL_GetWindowSurface(state->window)) == NULL)
		fatal("%s", SDL_GetError());

	cairo_surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24,
	    sdl_surface->w, sdl_surface->h);
	if (cairo_surface_status(cairo_surface) != CAIRO_STATUS_SUCCESS)
		fatal("unable to create cairo surface");

	state->cairo = cairo_create(cairo_surface);
	if (cairo_status(state->cairo) != CAIRO_STATUS_SUCCESS)
		fatal("unable to create cairo object");

	/* cairo_create references cairo_surface, so deref here */
	cairo_surface_destroy(cairo_surface);
}

static void
state_blit(struct state *state)
{
	SDL_Surface *sdl_surface;
	cairo_surface_t *cairo_surface;
	unsigned char *data;

	if (window_is_resized(state))
		fatal("unexpected window size");
	if ((cairo_surface = cairo_get_target(state->cairo)) == NULL)
		fatal("cairo_get_target");
	if ((sdl_surface = SDL_GetWindowSurface(state->window)) == NULL)
		fatal("%s", SDL_GetError());

	cairo_surface_flush(cairo_surface);
	data = cairo_image_surface_get_data(cairo_surface);
	SDL_LockSurface(sdl_surface);
	memcpy(sdl_surface->pixels, data, sdl_surface->pitch * sdl_surface->h);
	SDL_UnlockSurface(sdl_surface);
	SDL_UpdateWindowSurface(state->window);
}

static void
set_default_bindings(struct state *state)
{
	size_t i;

#include "keybind.h"

	for (i = 0; i < sizeof keys / sizeof *keys; i++) {
		if (!cmd_is_valid(keys[i].command))
			fatal("invalid binding for key %s", keys[i].key);
		bind_set(state->bind, keys[i].key, keys[i].command);
	}
}

static void
set_statusbar_text(struct state *state)
{
	struct view *view = state_get_view(state);
	struct sys *sys = view_get_sys(view);
	char buf[BUFSIZ];
	const char *filename;

	if (state->is_search)
		statusbar_set_text(state->statusbar, "(search %s ):%s",
		    edit_get_text(state->edit), history_get(state->history));
	else if (state->is_input)
		statusbar_set_text(state->statusbar, ":%s",
		    edit_get_text(state->edit));

	memset(buf, 0, sizeof buf);
	if (state->index > 0)
		snprintf(buf, sizeof buf, "%d ", state->index);
	if (rec_is_recording(state->rec))
		snprintf(buf + strlen(buf), sizeof buf - strlen(buf), "rec ");
	if (sys_is_modified(sys))
		snprintf(buf + strlen(buf), sizeof buf - strlen(buf), "*");
	filename = util_basename(view_get_path(view));
	if (filename[0] == '\0')
		filename = "[no name]";
	snprintf(buf + strlen(buf), sizeof buf - strlen(buf),
	    "%s %d/%d %d/%d", filename,
	    sys_get_frame(sys) + 1, sys_get_frame_count(sys),
	    tabs_get_index(state->tabs) + 1, tabs_get_count(state->tabs));
	statusbar_set_info_text(state->statusbar, "%s", buf);
}

static void
stop_input(struct state *state)
{
	SDL_StopTextInput();
	statusbar_clear_text(state->statusbar);
	state->is_input = 0;
}

static void
run_cmd(struct state *state, const char *command)
{
	error_clear();

	if (!cmd_is_valid(command)) {
		statusbar_set_error(state->statusbar, error_get());
		return;
	}

	if (rec_is_recording(state->rec))
		rec_add(state->rec, command);

	if (cmd_exec(command, state))
		statusbar_set_text(state->statusbar, error_get());
	else
		statusbar_set_error(state->statusbar, error_get());

	/* bind dot to last executed command */
	bind_set(state->bind, ".", command);
}

static void
run_input(struct state *state)
{
	const char *text;

	text = state->is_search ? history_get(state->history) :
	    edit_get_text(state->edit);

	if (string_is_whitespace(text))
		return;

	history_push(state->history, text);
	run_cmd(state, text);
}

static void
insert_clipboard_text(struct edit *edit)
{
	char *text;

	text = SDL_GetClipboardText();
	edit_insert_text(edit, text);
	SDL_free(text);
}

static void
key_down_statusbar(struct state *state, SDL_Keysym keysym)
{
	const char *text;

	switch (keysym.sym) {
	case SDLK_RETURN:
		stop_input(state);
		run_input(state);
		state->is_search = 0;
		break;
	case SDLK_UP:
		state->is_search = 0;
		history_prev(state->history);
		edit_set_text(state->edit, "%s", history_get(state->history));
		break;
	case SDLK_DOWN:
		state->is_search = 0;
		history_next(state->history);
		edit_set_text(state->edit, "%s", history_get(state->history));
		break;
	case SDLK_c:
		if (keysym.mod & KMOD_CTRL) {
			state->is_search = 0;
			history_reset_current(state->history);
			edit_clear(state->edit);
		}
		break;
	case SDLK_r:
		if (keysym.mod & KMOD_CTRL) {
			state->is_search = 1;
			text = edit_get_text(state->edit);
			if (!string_is_whitespace(text)) {
				history_prev(state->history);
				if (!history_search(state->history, text))
					history_next(state->history);
			}
		}
		break;
	case SDLK_v:
		if (keysym.mod & KMOD_CTRL)
			insert_clipboard_text(state->edit);
		break;
	case SDLK_w:
		if (keysym.mod & KMOD_CTRL)
			edit_backspace_word(state->edit);
		break;
	case SDLK_LEFT:
		if (state->is_search) {
			state->is_search = 0;
			text = history_get(state->history);
			edit_set_text(state->edit, "%s", text);
		}
		edit_set_pos(state->edit, edit_get_pos(state->edit) - 1);
		break;
	case SDLK_RIGHT:
		if (state->is_search) {
			state->is_search = 0;
			text = history_get(state->history);
			edit_set_text(state->edit, "%s", text);
		}
		edit_set_pos(state->edit, edit_get_pos(state->edit) + 1);
		break;
	case SDLK_HOME:
		if (state->is_search) {
			state->is_search = 0;
			text = history_get(state->history);
			edit_set_text(state->edit, "%s", text);
		}
		edit_set_pos(state->edit, 0);
		break;
	case SDLK_END:
		if (state->is_search) {
			state->is_search = 0;
			text = history_get(state->history);
			edit_set_text(state->edit, "%s", text);
		}
		edit_set_pos(state->edit, edit_get_text_length(state->edit));
		break;
	case SDLK_BACKSPACE:
		if (edit_get_text_length(state->edit) == 0) {
			stop_input(state);
			state->is_search = 0;
		} else
			edit_backspace_char(state->edit);
		break;
	case SDLK_DELETE:
		edit_delete_char(state->edit);
		break;
	case SDLK_ESCAPE:
		stop_input(state);
		state->is_search = 0;
		break;
	}
}

static void
key_down_view(struct state *state, SDL_Keysym keysym)
{
	char *keystr;
	const char *command;
	int i, index;

	switch (keysym.sym) {
	case SDLK_LSHIFT:
	case SDLK_RSHIFT:
	case SDLK_LCTRL:
	case SDLK_RCTRL:
	case SDLK_LALT:
	case SDLK_RALT:
		/* ignore */
		break;
	case SDLK_SEMICOLON:
		if (keysym.mod & KMOD_SHIFT) {
			state->is_input = 1;
			edit_clear(state->edit);
			history_reset_current(state->history);
			SDL_StartTextInput();
		}
		break;
	case SDLK_0:
	case SDLK_1:
	case SDLK_2:
	case SDLK_3:
	case SDLK_4:
	case SDLK_5:
	case SDLK_6:
	case SDLK_7:
	case SDLK_8:
	case SDLK_9:
		if (keysym.mod == KMOD_NONE) {
			index = state->index * 10 + (keysym.sym - SDLK_0);
			if (index <= 999999)
				state->index = index;
		}
		break;
	default:
		key_string(&keystr, keysym.sym, keysym.mod);
		if ((command = bind_get(state->bind, keystr)) != NULL) {
			if (keysym.sym == SDLK_PERIOD && state->index > 0) {
				index = state->index;
				state->index = 0;
				for (i = 0; i < index; i++)
					if (!cmd_exec(command, state))
						break;
			} else
				run_cmd(state, command);
		}
		state->index = 0;
		free(keystr);
		break;
	}
}

static int
process_event(struct state *state, SDL_Event *event)
{
	switch (event->type) {
	case SDL_QUIT:
		if (state->force_quit || !tabs_any_modified(state->tabs))
			return (0);
		statusbar_set_error(state->statusbar,
		    "save changes or add ! to override");
		break;
	case SDL_KEYDOWN:
		if (state->is_input)
			key_down_statusbar(state, event->key.keysym);
		else
			key_down_view(state, event->key.keysym);
		break;
	case SDL_TEXTINPUT:
		if (strlen(event->text.text) == 1)
			edit_insert_char(state->edit, event->text.text[0]);
		if (state->is_search) {
			const char *text = edit_get_text(state->edit);
			history_search(state->history, text);
		}
		break;
	}
	return (1);
}

struct state *
state_create(void)
{
	struct state *state;
	const char *path;

	state = xcalloc(1, sizeof *state);
	state->bind = bind_create();
	state->edit = edit_create();
	state->history = history_create();
	state->rec = rec_create();
	state->statusbar = statusbar_create();
	state->tabs = tabs_create();
	state->yank = yank_create();

	create_window(state);
	create_cairo(state);

	set_default_bindings(state);

	path = settings_get_string("vimolhistory-path");
	history_load(state->history, path);

	return (state);
}

void
state_free(struct state *state)
{
	if (state) {
		bind_free(state->bind);
		edit_free(state->edit);
		history_free(state->history);
		rec_free(state->rec);
		statusbar_free(state->statusbar);
		tabs_free(state->tabs);
		yank_free(state->yank);
		cairo_destroy(state->cairo);
		SDL_DestroyWindow(state->window);
		free(state);
	}
}

struct bind *
state_get_bind(struct state *state)
{
	return (state->bind);
}

struct rec *
state_get_rec(struct state *state)
{
	return (state->rec);
}

struct view *
state_get_view(struct state *state)
{
	return (tabs_get_view(state->tabs));
}

struct tabs *
state_get_tabs(struct state *state)
{
	return (state->tabs);
}

struct yank *
state_get_yank(struct state *state)
{
	return (state->yank);
}

int
state_get_index(struct state *state)
{
	return (state->index);
}

int
state_source(struct state *state, const char *path)
{
	FILE *fp;
	char *buffer;

	if (strlen(path) == 0)
		return (1);
	if ((fp = fopen(path, "r")) == NULL) {
		error_set("unable to open file %s", path);
		return (0);
	}

	buffer = NULL;

	while ((buffer = util_next_line(buffer, fp)) != NULL) {
		if (string_is_comment(buffer))
			continue;
		cmd_exec(buffer, state);
	}

	fclose(fp);
	return (1);
}

void
state_render(struct state *state)
{
	int pos;

	if (window_is_resized(state))
		create_cairo(state);

	if (!state->is_search && !state->is_input)
		view_render(state_get_view(state), state->cairo);

	if (settings_get_bool("statusbar-visible")) {
		set_statusbar_text(state);
		pos = edit_get_pos(state->edit);

		if (state->is_search)
			statusbar_set_cursor_pos(state->statusbar, pos + 8);
		else if (state->is_input)
			statusbar_set_cursor_pos(state->statusbar, pos + 1);
		else
			statusbar_set_cursor_pos(state->statusbar, -1);

		statusbar_render(state->statusbar, state->cairo);
	}

	state_blit(state);
}

void
state_toggle_fullscreen(struct state *state)
{
	Uint32 flags;

	flags = SDL_GetWindowFlags(state->window);
	flags ^= SDL_WINDOW_FULLSCREEN_DESKTOP;

	if (SDL_SetWindowFullscreen(state->window, flags))
		fatal("%s", SDL_GetError());
}

void
state_quit(struct state *state, int force_quit)
{
	SDL_Event event;

	memset(&event, 0, sizeof event);
	event.type = SDL_QUIT;

	SDL_PushEvent(&event);

	state->force_quit = force_quit;
}

void
state_event_loop(struct state *state)
{
	SDL_Event event;

	for (;;) {
		SDL_WaitEvent(NULL);

		while (SDL_PollEvent(&event))
			if (!process_event(state, &event))
				return;

		state_render(state);
	}
}

void
state_save(struct state *state)
{
	const char *path;

	path = settings_get_string("vimolhistory-path");
	history_save(state->history, path);
}
