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
