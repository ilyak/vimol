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
	int repeat;
	struct alias *bind;
	struct edit *edit;
	struct history *history;
	struct rec *rec;
	struct statusbar *statusbar;
	struct wnd *wnd;
	struct yank *yank;
	SDL_Window *window;
	cairo_t *cairo;
};

static void
key_string(char *buffer, size_t size, SDL_Keycode sym, Uint16 mod)
{
	const char *ctrl, *alt, *shift, *name;

	 ctrl = mod & KMOD_CTRL  ?  "ctrl-" : "";
	  alt = mod & KMOD_ALT   ?   "alt-" : "";
	shift = mod & KMOD_SHIFT ? "shift-" : "";

	 name = SDL_GetKeyName(sym);

	snprintf(buffer, size, "%s%s%s%s", ctrl, alt, shift, name);
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
window_size_changed(struct state *state)
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

	/* cairo_create references cairo_surface so deref here */
	cairo_surface_destroy(cairo_surface);
}

static void
state_blit(struct state *state)
{
	SDL_Surface *sdl_surface;
	cairo_surface_t *cairo_surface;
	unsigned char *data;

	assert(!window_size_changed(state));

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
assign_default_bindings(struct state *state)
{
	unsigned i;

#include "keybind.h"

	for (i = 0; i < sizeof(keys) / sizeof(*keys); i++)
		alias_set(state->bind, keys[i].key, keys[i].command);
}

static void
set_statusbar_text(struct state *state)
{
	struct view *view;
	struct sys *sys;
	int iframe, nframe, iwnd, nwnd;
	const char *text;
	char *buf1, *buf2, *bufout;

	view = state_get_view(state);
	sys = view_get_sys(view);
	iframe = sys_get_frame(sys);
	nframe = sys_get_frame_count(sys);
	iwnd = wnd_get_index(state->wnd);
	nwnd = wnd_get_count(state->wnd);
	text = edit_get_text(state->edit);

	if (state->is_search)
		statusbar_set_text(state->statusbar, "(search %s ):%s", text,
		    history_get(state->history));
	else if (state->is_input)
		statusbar_set_text(state->statusbar, ":%s", text);

	if (rec_is_recording(state->rec))
		xasprintf(&buf1, "rec-%c ", rec_get_register(state->rec)+'a');
	else
		buf1 = xstrdup("");

	if (state->repeat > 0)
		xasprintf(&buf2, "%d ", state->repeat);
	else
		buf2 = xstrdup("");

	xasprintf(&bufout, "%s%s%sf%d/%d w%d/%d", buf1, buf2,
	    sys_is_modified(sys) ? "[+] " : "", iframe+1, nframe, iwnd+1, nwnd);
	statusbar_set_info_text(state->statusbar, "%s", bufout);

	free(buf1);
	free(buf2);
	free(bufout);
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
	struct cmdq *cmdq;

	error_clear();

	if ((cmdq = cmdq_from_string(command)) == NULL) {
		statusbar_set_error(state->statusbar, error_get());
		return;
	}

	if (rec_is_recording(state->rec))
		rec_add(state->rec, command);

	if (cmdq_exec(cmdq, state))
		statusbar_set_text(state->statusbar, error_get());
	else
		statusbar_set_error(state->statusbar, error_get());

	cmdq_free(cmdq);
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

static int
key_is_ctrl_alt_shift(SDL_Keysym keysym)
{
	return (keysym.sym == SDLK_LSHIFT ||
		keysym.sym == SDLK_RSHIFT ||
		keysym.sym == SDLK_LCTRL  ||
		keysym.sym == SDLK_RCTRL  ||
		keysym.sym == SDLK_LALT   ||
		keysym.sym == SDLK_RALT);
}

static int
key_is_number(SDL_Keysym keysym)
{
	return (keysym.mod == KMOD_NONE &&
	    keysym.sym >= SDLK_0 && keysym.sym <= SDLK_9);
}

static void
key_down_view(struct state *state, SDL_Keysym keysym)
{
	char buffer[1024];
	const char *command;
	int repeat;

	if (key_is_ctrl_alt_shift(keysym))
		return;

	if (key_is_number(keysym)) {
		repeat = state->repeat * 10 + (keysym.sym - SDLK_0);

		if (repeat <= 999999)
			state->repeat = repeat;

		return;
	}

	key_string(buffer, sizeof(buffer), keysym.sym, keysym.mod);

	if ((command = alias_get(state->bind, buffer)) == NULL) {
		state->repeat = 0;
		return;
	}

	if (state->repeat > 1)
		snprintf(buffer, sizeof(buffer), "repeat %d \"%s\"",
		    state->repeat, command);
	else
		snprintf(buffer, sizeof(buffer), "%s", command);

	state->repeat = 0;
	run_cmd(state, buffer);
}

static int
process_event(struct state *state, SDL_Event *event)
{
	switch (event->type) {
	case SDL_QUIT:
		if (state->force_quit || !wnd_any_modified(state->wnd))
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

	if ((state = xcalloc(1, sizeof(*state))) == NULL)
		return (NULL);

	state->bind = alias_create();
	state->edit = edit_create();
	state->history = history_create();
	state->rec = rec_create();
	state->statusbar = statusbar_create();
	state->wnd = wnd_create();
	state->yank = yank_create();

	create_window(state);
	create_cairo(state);

	assign_default_bindings(state);

	path = settings_get_string("rec.path");
	rec_load(state->rec, path);

	path = settings_get_string("history.path");
	history_load(state->history, path);

	return (state);
}

void
state_free(struct state *state)
{
	alias_free(state->bind);
	edit_free(state->edit);
	history_free(state->history);
	rec_free(state->rec);
	statusbar_free(state->statusbar);
	wnd_free(state->wnd);
	yank_free(state->yank);
	cairo_destroy(state->cairo);
	SDL_DestroyWindow(state->window);
	free(state);
}

struct alias *
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
	return (wnd_get_view(state->wnd));
}

struct wnd *
state_get_wnd(struct state *state)
{
	return (state->wnd);
}

struct yank *
state_get_yank(struct state *state)
{
	return (state->yank);
}

void
state_start_edit(struct state *state)
{
	state->is_input = 1;

	edit_clear(state->edit);
	history_reset_current(state->history);

	SDL_StartTextInput();
}

int
state_source(struct state *state, const char *path)
{
	FILE *fp;
	struct cmdq *cmdq;
	char *buffer;

	if ((fp = fopen(path, "r")) == NULL) {
		error_set("unable to open file %s", path);
		return (0);
	}

	buffer = NULL;

	while ((buffer = util_next_line(buffer, fp)) != NULL) {
		if (string_is_comment(buffer))
			continue;

		if ((cmdq = cmdq_from_string(buffer))) {
			cmdq_exec(cmdq, state);
			cmdq_free(cmdq);
		}
	}

	fclose(fp);
	return (1);
}

void
state_render(struct state *state)
{
	int pos;

	if (window_size_changed(state))
		create_cairo(state);

	view_render(state_get_view(state), state->cairo);

	if (settings_get_bool("statusbar.visible")) {
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
state_save_png(struct state *state, const char *path)
{
	state_render(state);
	cairo_surface_write_to_png(cairo_get_target(state->cairo), path);
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

	memset(&event, 0, sizeof(event));
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

	path = settings_get_string("history.path");
	history_save(state->history, path);

	path = settings_get_string("rec.path");
	rec_save(state->rec, path);
}
