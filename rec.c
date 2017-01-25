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
	int is_playing, reg;
	char *data[REC_SIZE];
};

struct rec *
rec_create(void)
{
	struct rec *rec;
	int i;

	rec = xcalloc(1, sizeof *rec);
	rec->reg = -1;

	for (i = 0; i < REC_SIZE; i++)
		rec->data[i] = xstrdup("");

	return (rec);
}

void
rec_free(struct rec *rec)
{
	int i;

	if (rec) {
		for (i = 0; i < REC_SIZE; i++)
			free(rec->data[i]);
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
	return (rec->reg != -1);
}

int
rec_get_register(struct rec *rec)
{
	assert(rec->reg != -1);

	return (rec->reg);
}

void
rec_load(struct rec *rec, const char *path)
{
	FILE *fp;
	char *buffer;
	int i;

	if ((fp = fopen(path, "r")) == NULL)
		return;

	buffer = NULL;

	for (i = 0; i < REC_SIZE; i++) {
		buffer = util_next_line(buffer, fp);
		free(rec->data[i]);
		rec->data[i] = xstrdup(buffer ? buffer : "");
	}

	free(buffer);
	fclose(fp);
}

void
rec_save(struct rec *rec, const char *path)
{
	FILE *fp;
	int i;

	if ((fp = fopen(path, "w")) == NULL)
		return;

	for (i = 0; i < REC_SIZE; i++)
		fprintf(fp, "%s\n", rec->data[i]);

	fclose(fp);
}

void
rec_start(struct rec *rec, int reg)
{
	assert(reg >= 0 && reg < REC_SIZE);
	assert(rec->reg == -1);

	rec->reg = reg;
	rec->data[reg][0] = '\0';
}

void
rec_add(struct rec *rec, const char *add)
{
	char *s, *p;

	assert(rec->reg != -1);

	s = rec->data[rec->reg];

	if (s[0] == '\0')
		p = xstrdup(add);
	else
		xasprintf(&p, "%s; %s", s, add);

	free(s);
	rec->data[rec->reg] = p;
}

void
rec_stop(struct rec *rec)
{
	assert(rec->reg != -1);

	rec->reg = -1;
}

int
rec_play(struct rec *rec, int reg, struct state *state)
{
	struct cmdq *cmdq;

	assert(reg >= 0 && reg < REC_SIZE);
	assert(rec->reg == -1 && !rec->is_playing);

	if ((cmdq = cmdq_from_string(rec->data[reg])) == NULL)
		return (0);

	rec->is_playing = 1;
	cmdq_exec(cmdq, state);
	rec->is_playing = 0;
	cmdq_free(cmdq);

	return (1);
}
