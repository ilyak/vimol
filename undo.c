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

struct node {
	void *data;
	struct node *next;
	struct node *prev;
};

struct undo {
	void *(*copy)(void *);
	void (*free)(void *);
	struct node *iter;
};

static void
free_all(struct undo *undo, struct node *node)
{
	struct node *next;

	while (node) {
		next = node->next;
		(undo->free)(node->data);
		free(node);
		node = next;
	}
}

struct undo *
undo_create(void *data, void *(*copy)(void *), void (*free)(void *))
{
	struct undo *undo;

	undo = xcalloc(1, sizeof *undo);
	undo->copy = copy;
	undo->free = free;
	undo->iter = xcalloc(1, sizeof *undo->iter);
	undo->iter->data = data;

	return (undo);
}

void
undo_free(struct undo *undo)
{
	if (undo) {
		while (undo->iter->prev)
			undo->iter = undo->iter->prev;
		free_all(undo, undo->iter);
		free(undo);
	}
}

void *
undo_get_data(struct undo *undo)
{
	return (undo->iter->data);
}

void
undo_snapshot(struct undo *undo)
{
	struct node *node;

	free_all(undo, undo->iter->next);

	node = xcalloc(1, sizeof *node);
	node->data = (undo->copy)(undo->iter->data);

	undo->iter->next = node;
	node->prev = undo->iter;
	undo->iter = node;
}

int
undo_undo(struct undo *undo)
{
	if (undo->iter->prev == NULL)
		return (0);

	undo->iter = undo->iter->prev;

	return (1);
}

int
undo_redo(struct undo *undo)
{
	if (undo->iter->next == NULL)
		return (0);

	undo->iter = undo->iter->next;

	return (1);
}
