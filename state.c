/*-
 * Copyright (c) 2013-2014 Ilya Kaliman
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "vimol.h"

struct state {
	int is_input;
	int is_search;
	int force_quit;
	int repeat;
	struct alias *alias;
	struct alias *bind;
	struct edit *edit;
	struct history *history;
	struct rec *rec;
	struct status *status;
	struct wins *wins;
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
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		800, 600, SDL_WINDOW_RESIZABLE);

	if (state->window == NULL)
		log_fatal("cannot create SDL window");
}

static void
load_info(struct state *state, const char *path)
{
	FILE *fp;
	int x, y, w, h;

	if ((fp = fopen(path, "r")) == NULL)
		return;

	if (fscanf(fp, "%d %d %d %d", &x, &y, &w, &h) == 4) {
		SDL_SetWindowPosition(state->window, x, y);
		SDL_SetWindowSize(state->window, w, h);
	}

	fclose(fp);
}

static void
save_info(struct state *state, const char *path)
{
	FILE *fp;
	int x, y, w, h;

	if ((fp = fopen(path, "w")) == NULL)
		return;

	SDL_GetWindowPosition(state->window, &x, &y);
	SDL_GetWindowSize(state->window, &w, &h);

	fprintf(fp, "%d %d %d %d\n", x, y, w, h);
	fclose(fp);
}

static void
create_cairo(struct state *state)
{
	SDL_Surface *window_surface;
	cairo_surface_t *cairo_surface;

	if (state->cairo)
		cairo_destroy(state->cairo);

	window_surface = SDL_GetWindowSurface(state->window);

	if (window_surface == NULL)
		log_fatal("cannot acquire SDL window surface");

	cairo_surface = cairo_image_surface_create_for_data(
		(unsigned char *)window_surface->pixels, CAIRO_FORMAT_RGB24,
		window_surface->w, window_surface->h, window_surface->pitch);

	if (cairo_surface == NULL)
		log_fatal("cannot create cairo surface");

	state->cairo = cairo_create(cairo_surface);
	cairo_surface_destroy(cairo_surface);

	if (state->cairo == NULL)
		log_fatal("cannot create cairo object");
}

static void
assign_default_bindings(struct state *state)
{
	unsigned i;

#include "keys.h"

	for (i = 0; i < sizeof(keys) / sizeof(*keys); i++)
		alias_set(state->bind, keys[i].key, keys[i].command);
}

static void
set_status_text(struct state *state)
{
	struct view *view;
	struct sys *sys;
	int frame, nframes;
	const char *text;
	char *buffer, tmp[128];

	view = state_get_view(state);
	sys = view_get_sys(view);
	frame = sys_get_frame(sys) + 1;
	nframes = sys_get_frame_count(sys);
	text = edit_get_text(state->edit);

	if (state->is_search)
		status_set_text(state->status, "(search %s ):%s", text,
		    history_get(state->history));
	else if (state->is_input)
		status_set_text(state->status, ":%s", text);

	buffer = xstrdup("");

	if (rec_is_recording(state->rec)) {
		snprintf(tmp, sizeof(tmp), "[rec %c] ",
		    rec_get_register(state->rec) + 'a');
		buffer = xstrcat(buffer, tmp);
	}

	if (state->repeat > 0) {
		snprintf(tmp, sizeof(tmp), "[%d] ", state->repeat);
		buffer = xstrcat(buffer, tmp);
	}

	if (sys_is_modified(sys)) {
		snprintf(tmp, sizeof(tmp), "[+] ");
		buffer = xstrcat(buffer, tmp);
	}

	snprintf(tmp, sizeof(tmp), "[%d/%d]", frame, nframes);
	buffer = xstrcat(buffer, tmp);

	status_set_info_text(state->status, "%s", buffer);
	free(buffer);
}

static void
stop_input(struct state *state)
{
	SDL_StopTextInput();
	status_clear_text(state->status);
	state->is_input = FALSE;
}

static void
run_cmd(struct state *state, const char *command)
{
	struct cmdq *cmdq;

	error_clear();

	if ((cmdq = cmdq_from_string(command, state->alias)) == NULL) {
		status_set_error(state->status, error_get());
		return;
	}

	if (rec_is_recording(state->rec))
		rec_add(state->rec, command);

	if (cmdq_exec(cmdq, state))
		status_set_text(state->status, error_get());
	else
		status_set_error(state->status, error_get());

	cmdq_free(cmdq);
}

static void
run_input(struct state *state)
{
	const char *text;

	text = state->is_search ? history_get(state->history) :
	    edit_get_text(state->edit);

	if (util_is_empty(text))
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
key_down_status(struct state *state, SDL_Keysym keysym)
{
	const char *text;

	switch (keysym.sym) {
	case SDLK_RETURN:
		stop_input(state);
		run_input(state);
		state->is_search = FALSE;
		break;
	case SDLK_UP:
		state->is_search = FALSE;
		history_prev(state->history);
		edit_set_text(state->edit, "%s", history_get(state->history));
		break;
	case SDLK_DOWN:
		state->is_search = FALSE;
		history_next(state->history);
		edit_set_text(state->edit, "%s", history_get(state->history));
		break;
	case SDLK_c:
		if (keysym.mod & KMOD_CTRL) {
			state->is_search = FALSE;
			history_reset_current(state->history);
			edit_clear(state->edit);
		}
		break;
	case SDLK_r:
		if (keysym.mod & KMOD_CTRL) {
			state->is_search = TRUE;
			text = edit_get_text(state->edit);

			if (!util_is_empty(text)) {
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
			state->is_search = FALSE;
			text = history_get(state->history);
			edit_set_text(state->edit, "%s", text);
		}
		edit_set_pos(state->edit, edit_get_pos(state->edit) - 1);
		break;
	case SDLK_RIGHT:
		if (state->is_search) {
			state->is_search = FALSE;
			text = history_get(state->history);
			edit_set_text(state->edit, "%s", text);
		}
		edit_set_pos(state->edit, edit_get_pos(state->edit) + 1);
		break;
	case SDLK_HOME:
		if (state->is_search) {
			state->is_search = FALSE;
			text = history_get(state->history);
			edit_set_text(state->edit, "%s", text);
		}
		edit_set_pos(state->edit, 0);
		break;
	case SDLK_END:
		if (state->is_search) {
			state->is_search = FALSE;
			text = history_get(state->history);
			edit_set_text(state->edit, "%s", text);
		}
		edit_set_pos(state->edit, edit_get_text_length(state->edit));
		break;
	case SDLK_BACKSPACE:
		edit_backspace_char(state->edit);
		break;
	case SDLK_DELETE:
		edit_delete_char(state->edit);
		break;
	case SDLK_ESCAPE:
		stop_input(state);
		state->is_search = FALSE;
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
key_is_period(SDL_Keysym keysym)
{
	return (keysym.mod == KMOD_NONE && keysym.sym == SDLK_PERIOD);
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

	/* this cannot be in exec because of possible infinite recursion */
	if (key_is_period(keysym)) {
		history_reset_current(state->history);
		history_prev(state->history);
		run_cmd(state, history_get(state->history));
		history_reset_current(state->history);
		return;
	}

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
		if (state->force_quit || !wins_any_modified(state->wins))
			return (FALSE);
		status_set_error(state->status,
		    "save changes or add ! to override");
		break;
	case SDL_KEYDOWN:
		if (state->is_input)
			key_down_status(state, event->key.keysym);
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
	case SDL_WINDOWEVENT:
		switch (event->window.event) {
		case SDL_WINDOWEVENT_RESIZED:
			create_cairo(state);
			break;
		}
		break;
	}
	return (TRUE);
}

struct state *
state_create(void)
{
	struct state *state;
	const char *path;

	if ((state = calloc(1, sizeof(*state))) == NULL)
		return (NULL);

	state->alias = alias_create();
	state->bind = alias_create();
	state->edit = edit_create();
	state->history = history_create();
	state->rec = rec_create();
	state->status = status_create();
	state->wins = wins_create();
	state->yank = yank_create();

	create_window(state);

	path = settings_get_string("info.path");
	load_info(state, path);

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
	alias_free(state->alias);
	alias_free(state->bind);
	edit_free(state->edit);
	history_free(state->history);
	rec_free(state->rec);
	status_free(state->status);
	wins_free(state->wins);
	yank_free(state->yank);
	cairo_destroy(state->cairo);
	SDL_DestroyWindow(state->window);
	free(state);
}

struct alias *
state_get_alias(struct state *state)
{
	return (state->alias);
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
	return (wins_get_view(state->wins));
}

struct wins *
state_get_wins(struct state *state)
{
	return (state->wins);
}

struct yank *
state_get_yank(struct state *state)
{
	return (state->yank);
}

void
state_start_edit(struct state *state)
{
	state->is_input = TRUE;

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
		return (FALSE);
	}

	buffer = NULL;

	while ((buffer = util_next_line(buffer, fp)) != NULL) {
		if (util_is_comment(buffer))
			continue;

		if ((cmdq = cmdq_from_string(buffer, state->alias))) {
			cmdq_exec(cmdq, state);
			cmdq_free(cmdq);
		}
	}

	fclose(fp);
	return (TRUE);
}

void
state_render(struct state *state)
{
	int pos;

	view_render(state_get_view(state), state->cairo);

	if (settings_get_bool("status.visible")) {
		set_status_text(state);
		pos = edit_get_pos(state->edit);

		if (state->is_search)
			status_set_cursor_pos(state->status, pos + 8);
		else if (state->is_input)
			status_set_cursor_pos(state->status, pos + 1);
		else
			status_set_cursor_pos(state->status, -1);

		status_render(state->status, state->cairo);
	}

	SDL_UpdateWindowSurface(state->window);
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

	SDL_SetWindowFullscreen(state->window, flags);
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

	path = settings_get_string("info.path");
	save_info(state, path);
}
