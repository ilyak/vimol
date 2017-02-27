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

#define HIST_SIZE 10000

struct history {
	int top, current;
	char *list[HIST_SIZE];
};

struct history *
history_create(void)
{
	struct history *history;

	history = xcalloc(1, sizeof *history);
	history_reset_current(history);

	return (history);
}

void
history_free(struct history *history)
{
	int i;

	if (history) {
		for (i = 0; i < HIST_SIZE; i++)
			free(history->list[i]);
		free(history);
	}
}

void
history_load(struct history *history, const char *path)
{
	FILE *fp;
	char *buffer;

	if (strlen(path) == 0)
		return;
	if ((fp = fopen(path, "r")) == NULL)
		return;

	buffer = NULL;

	while ((buffer = util_next_line(buffer, fp)) != NULL)
		if (!string_is_whitespace(buffer))
			history_push(history, buffer);

	fclose(fp);
}

void
history_save(struct history *history, const char *path)
{
	FILE *fp;
	int save;

	if (strlen(path) == 0)
		return;
	if ((fp = fopen(path, "w")) == NULL)
		return;

	save = history->current;
	history_reset_current(history);

	while (history_prev(history))
		continue;

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
		return (0);

	history->current = (history->current + 1) % HIST_SIZE;

	return (1);
}

int
history_prev(struct history *history)
{
	int prev = history->current - 1;

	if (prev < 0)
		prev = HIST_SIZE - 1;

	if (!history->list[prev])
		return (0);

	history->current = prev;

	return (1);
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
			return (1);

		if (!history_prev(history))
			break;
	}

	history->current = save;
	return (0);
}
