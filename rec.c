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
	int is_playing;
	int is_recording;
	int reg;
	char *data[REC_SIZE];
};

struct rec *
rec_create(void)
{
	struct rec *rec;
	int i;

	rec = calloc(1, sizeof(*rec));

	for (i = 0; i < REC_SIZE; i++)
		rec->data[i] = xstrdup("");

	return (rec);
}

void
rec_free(struct rec *rec)
{
	int i;

	for (i = 0; i < REC_SIZE; i++)
		free(rec->data[i]);

	free(rec);
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

int
rec_get_register(struct rec *rec)
{
	return (rec->reg);
}

void
rec_set_register(struct rec *rec, int reg)
{
	assert(reg >= 0 && reg < REC_SIZE);
	assert(!rec->is_recording);

	rec->reg = reg;
}

void
rec_load(struct rec *rec, const char *path)
{
	FILE *fp;
	int i;
	char *buffer;

	if ((fp = fopen(path, "r")) == NULL) {
		log_warn("unable to open rec file %s", path);
		return;
	}

	buffer = NULL;

	for (i = 0; i < REC_SIZE; i++) {
		buffer = util_next_line(buffer, fp);
		rec->data[i] = xstrcpy(rec->data[i], buffer ? buffer : "");
	}

	free(buffer);
	fclose(fp);
}

void
rec_save(struct rec *rec, const char *path)
{
	FILE *fp;
	int i;

	if ((fp = fopen(path, "w")) == NULL) {
		log_warn("unable to write to %s", path);
		return;
	}

	for (i = 0; i < REC_SIZE; i++)
		fprintf(fp, "%s\n", rec->data[i]);

	fclose(fp);
}

void
rec_start(struct rec *rec)
{
	assert(!rec->is_recording);

	rec->data[rec->reg][0] = '\0';
	rec->is_recording = TRUE;
}

void
rec_add(struct rec *rec, const char *add)
{
	char *s = rec->data[rec->reg];

	assert(rec->is_recording);

	if (s[0] == '\0')
		s = xstrcpy(s, add);
	else
		s = xstrcat(xstrcat(s, "; "), add);

	rec->data[rec->reg] = s;
}

void
rec_stop(struct rec *rec)
{
	assert(rec->is_recording);

	rec->is_recording = FALSE;
}

int
rec_play(struct rec *rec, struct state *state)
{
	struct alias *alias;
	struct cmdq *cmdq;

	assert(!rec->is_recording && !rec->is_playing);

	alias = state_get_alias(state);

	if ((cmdq = cmdq_from_string(rec->data[rec->reg], alias)) == NULL)
		return (FALSE);

	rec->is_playing = TRUE;
	cmdq_exec(cmdq, state);
	rec->is_playing = FALSE;
	cmdq_free(cmdq);

	return (TRUE);
}
