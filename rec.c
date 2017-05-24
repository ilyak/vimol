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

struct rec {
	int is_playing, is_recording;
	char *data;
};

struct rec *
rec_create(void)
{
	struct rec *rec;

	rec = xcalloc(1, sizeof *rec);
	rec->data = xstrdup("");

	return (rec);
}

void
rec_free(struct rec *rec)
{
	if (rec) {
		free(rec->data);
		free(rec);
	}
}

int
rec_is_playing(struct rec *rec)
{
	return (rec->is_playing);
}

int
rec_is_recording(struct rec *rec)
{
	return (rec->is_recording);
}

void
rec_start(struct rec *rec)
{
	assert(!rec->is_playing && !rec->is_recording);

	rec->is_recording = 1;
	rec->data[0] = '\0';
}

void
rec_add(struct rec *rec, const char *add)
{
	char *str;

	assert(rec->is_recording);

	if (rec->data[0] == '\0')
		str = xstrdup(add);
	else
		xasprintf(&str, "%s; %s", rec->data, add);

	free(rec->data);
	rec->data = str;
}

void
rec_stop(struct rec *rec)
{
	assert(rec->is_recording);

	rec->is_recording = 0;
}

int
rec_play(struct rec *rec, struct state *state)
{
	assert(!rec->is_playing && !rec->is_recording);

	if (!cmd_is_valid(rec->data))
		return (0);

	rec->is_playing = 1;
	cmd_exec(rec->data, state);
	rec->is_playing = 0;

	return (1);
}
