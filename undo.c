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

	undo = calloc(1, sizeof(*undo));
	undo->copy = copy;
	undo->free = free;
	undo->iter = calloc(1, sizeof(struct node));
	undo->iter->data = data;

	return (undo);
}

void
undo_free(struct undo *undo)
{
	while (undo->iter->prev)
		undo->iter = undo->iter->prev;

	free_all(undo, undo->iter);
	free(undo);
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

	node = calloc(1, sizeof(*node));
	node->data = (undo->copy)(undo->iter->data);

	undo->iter->next = node;
	node->prev = undo->iter;
	undo->iter = node;
}

int
undo_undo(struct undo *undo)
{
	if (undo->iter->prev == NULL)
		return (FALSE);

	undo->iter = undo->iter->prev;

	return (TRUE);
}

int
undo_redo(struct undo *undo)
{
	if (undo->iter->next == NULL)
		return (FALSE);

	undo->iter = undo->iter->next;

	return (TRUE);
}
