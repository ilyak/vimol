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
	struct wins *wins;
	int idx;

	srand((unsigned)(time(NULL)));

	exec_init();
	settings_init();

	log_open(settings_get_string("log.path"));

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		log_fatal("cannot initialize SDL");

	SDL_StopTextInput();

	if ((state = state_create()) == NULL)
		log_fatal("unable to create a program state");

	wins = state_get_wins(state);

	for (idx = 1; idx < argc; idx++)
		if (!wins_open(wins, argv[idx]))
			log_warn("error opening \"%s\" (%s)", argv[idx],
			    error_get());

	wins_first(wins);
	wins_close(wins);

	if (!state_source(state, settings_get_string("rc.path")))
		log_warn("%s", error_get());

	state_event_loop(state);

	state_save(state);
	state_free(state);

	settings_free();
	exec_free();

	log_close();
	SDL_Quit();

	return (0);
}
