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

int
main(int argc, char **argv)
{
	struct state *state;
	struct tabs *tabs;
	int idx;

	settings_init();

	if (SDL_Init(SDL_INIT_VIDEO))
		fatal("SDL_Init: %s", SDL_GetError());

	SDL_StopTextInput();

	state = state_create();
	tabs = state_get_tabs(state);

	for (idx = 1; idx < argc; idx++)
		if (!tabs_open(tabs, argv[idx]))
			warn("error opening file \"%s\": %s", argv[idx],
			    error_get());

	tabs_first(tabs);
	state_source(state, settings_get_string("vimolrc-path"));
	state_event_loop(state);

	state_save(state);
	state_free(state);
	SDL_Quit();
	settings_free();

	return (0);
}
