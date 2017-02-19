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

/* cannot use pointers because of realloc */
struct node {
	int prev, next;
};

struct sel {
	int nelts, nalloc, count;
	struct node *data;
	int head, tail, iter;
};

struct sel *
sel_create(int size)
{
	struct sel *sel;

	sel = xcalloc(1, sizeof *sel);
	sel->nalloc = 8;
	sel->data = xcalloc(sel->nalloc, sizeof *sel->data);
	sel->head = sel->tail = sel->iter = -1;

	while (size > 0) {
		sel_expand(sel);
		size--;
	}

	return (sel);
}

struct sel *
sel_copy(struct sel *sel)
{
	struct sel *copy;
	int idx;

	copy = sel_create(sel_get_size(sel));
	sel_iter_start(sel);

	while (sel_iter_next(sel, &idx))
		sel_add(copy, idx);

	return (copy);
}

void
sel_free(struct sel *sel)
{
	if (sel) {
		free(sel->data);
		free(sel);
	}
}

int
sel_get_size(struct sel *sel)
{
	return (sel->nelts);
}

int
sel_get_count(struct sel *sel)
{
	return (sel->count);
}

void
sel_expand(struct sel *sel)
{
	if (sel->nelts == sel->nalloc) {
		sel->nalloc *= 2;
		sel->data = xrealloc(sel->data,
		    sel->nalloc * sizeof *sel->data);
	}
	sel->data[sel->nelts].prev = -1;
	sel->data[sel->nelts].next = -1;
	sel->nelts++;
}

void
sel_contract(struct sel *sel, int idx)
{
	int i;

	assert(idx >= 0 && idx < sel_get_size(sel));
	assert(sel->iter == -1);

	sel_remove(sel, idx);

	sel->nelts--;

	memmove(sel->data + idx, sel->data + idx + 1,
	    (sel->nelts - idx) * sizeof *sel->data);

	if (sel->head > idx) sel->head--;
	if (sel->tail > idx) sel->tail--;

	for (i = 0; i < sel_get_size(sel); i++) {
		if (sel->data[i].prev > idx) sel->data[i].prev--;
		if (sel->data[i].next > idx) sel->data[i].next--;
	}
}

void
sel_add(struct sel *sel, int idx)
{
	assert(idx >= 0 && idx < sel_get_size(sel));

	if (sel_selected(sel, idx))
		return;

	if (sel->tail == -1)
		sel->head = sel->tail = idx;
	else {
		sel->data[sel->tail].next = idx;
		sel->data[idx].prev = sel->tail;
		sel->tail = idx;
	}

	sel->count++;
}

void
sel_remove(struct sel *sel, int idx)
{
	struct node *node;

	assert(idx >= 0 && idx < sel_get_size(sel));

	if (!sel_selected(sel, idx))
		return;

	node = sel->data + idx;

	if (node->prev != -1)
		sel->data[node->prev].next = node->next;
	else
		sel->head = node->next;

	if (node->next != -1)
		sel->data[node->next].prev = node->prev;
	else
		sel->tail = node->prev;

	sel->data[idx].prev = sel->data[idx].next = -1;
	sel->count--;
}

void
sel_all(struct sel *sel)
{
	int i;

	for (i = 0; i < sel_get_size(sel); i++)
		sel_add(sel, i);
}

void
sel_clear(struct sel *sel)
{
	int i;

	for (i = 0; i < sel_get_size(sel); i++)
		sel_remove(sel, i);
}

int
sel_selected(struct sel *sel, int idx)
{
	assert(idx >= 0 && idx < sel_get_size(sel));

	return (idx == sel->head || sel->data[idx].next != -1 ||
	    sel->data[idx].prev != -1);
}

void
sel_iter_start(struct sel *sel)
{
	sel->iter = sel->head;
}

int
sel_iter_next(struct sel *sel, int *idx)
{
	if (sel->iter == -1)
		return (0);

	*idx = sel->iter;
	sel->iter = sel->data[sel->iter].next;

	return (1);
}
