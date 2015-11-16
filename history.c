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

#define HIST_SIZE 10000

struct history {
	int top, current;
	char *list[HIST_SIZE];
};

struct history *
history_create(void)
{
	struct history *history;

	history = calloc(1, sizeof(*history));
	history_reset_current(history);

	return (history);
}

void
history_free(struct history *history)
{
	history_reset_current(history);

	while (history_prev(history))
		free(history->list[history->current]);

	free(history);
}

void
history_load(struct history *history, const char *path)
{
	FILE *fp;
	char *buffer;

	if ((fp = fopen(path, "r")) == NULL) {
		log_warn("unable to open history file %s", path);
		return;
	}

	buffer = NULL;

	while ((buffer = util_next_line(buffer, fp)) != NULL)
		if (!util_is_empty(buffer))
			history_push(history, buffer);

	fclose(fp);
}

void
history_save(struct history *history, const char *path)
{
	FILE *fp;
	int save;

	if ((fp = fopen(path, "w")) == NULL) {
		log_warn("unable to write history file %s", path);
		return;
	}

	save = history->current;
	history_reset_current(history);

	while (history_prev(history))
		;

	while (history->current != history->top) {
		fprintf(fp, "%s\n", history_get(history));
		history_next(history);
	}

	history->current = save;
	fclose(fp);
}

void
history_push(struct history *history, const char *str)
{
	history->list[history->top] = xstrdup(str);
	history->top = (history->top + 1) % HIST_SIZE;
	free(history->list[history->top]);
	history->list[history->top] = NULL;
	history_reset_current(history);
}

void
history_reset_current(struct history *history)
{
	history->current = history->top;
}

int
history_next(struct history *history)
{
	if (history->current == history->top)
		return (FALSE);

	history->current = (history->current + 1) % HIST_SIZE;

	return (TRUE);
}

int
history_prev(struct history *history)
{
	int prev = history->current - 1;

	if (prev < 0)
		prev = HIST_SIZE - 1;

	if (!history->list[prev])
		return (FALSE);

	history->current = prev;

	return (TRUE);
}

const char *
history_get(struct history *history)
{
	return (history->current == history->top ? "" :
	    history->list[history->current]);
}

int
history_search(struct history *history, const char *str)
{
	int save = history->current;

	for (;;) {
		if (strstr(history_get(history), str))
			return (TRUE);

		if (!history_prev(history))
			break;
	}

	history->current = save;
	return (FALSE);
}
