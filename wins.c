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
	struct view *view;
	struct node *next;
	struct node *prev;
};

struct wins {
	struct node *iter;
};

static struct node *
create_node(const char *path)
{
	struct node *node;

	node = calloc(1, sizeof(*node));

	if ((node->view = view_create(path)) == NULL) {
		free(node);
		return (NULL);
	}

	return (node);
}

static void
insert_after(struct node *iter, struct node *node)
{
	node->next = iter->next;
	node->prev = iter;

	if (iter->next)
		iter->next->prev = node;

	iter->next = node;
}

struct wins *
wins_create(void)
{
	struct wins *wins;

	wins = calloc(1, sizeof(*wins));
	wins->iter = create_node("");

	return (wins);
}

void
wins_free(struct wins *wins)
{
	struct node *node;

	wins_first(wins);

	while (wins->iter) {
		node = wins->iter;
		wins->iter = wins->iter->next;
		view_free(node->view);
		free(node);
	}

	free(wins);
}

struct view *
wins_get_view(struct wins *wins)
{
	return (wins->iter->view);
}

int
wins_open(struct wins *wins, const char *path)
{
	struct node *node;

	if ((node = create_node(path)) == NULL)
		return (FALSE);

	insert_after(wins->iter, node);
	wins->iter = node;

	return (TRUE);
}

int
wins_close(struct wins *wins)
{
	struct node *node;

	node = wins->iter;

	if (node->next == NULL && node->prev == NULL) {
		error_set("cannot close last window");
		return (FALSE);
	}

	if (node->next) {
		node->next->prev = node->prev;

		if (node->prev)
			node->prev->next = node->next;

		wins->iter = node->next;
	} else {
		node->prev->next = NULL;
		wins->iter = node->prev;
	}

	view_free(node->view);
	free(node);

	return (TRUE);
}

int
wins_is_modified(struct wins *wins)
{
	return (view_is_modified(wins_get_view(wins)));
}

int
wins_any_modified(struct wins *wins)
{
	struct node *node;

	for (node = wins->iter->next; node; node = node->next)
		if (view_is_modified(node->view))
			return (TRUE);

	for (node = wins->iter->prev; node; node = node->prev)
		if (view_is_modified(node->view))
			return (TRUE);

	return (wins_is_modified(wins));
}

int
wins_next(struct wins *wins)
{
	if (wins->iter->next == NULL)
		return (FALSE);

	wins->iter = wins->iter->next;

	return (TRUE);
}

int
wins_prev(struct wins *wins)
{
	if (wins->iter->prev == NULL)
		return (FALSE);

	wins->iter = wins->iter->prev;

	return (TRUE);
}

void
wins_first(struct wins *wins)
{
	while (wins->iter->prev)
		wins->iter = wins->iter->prev;
}

void
wins_last(struct wins *wins)
{
	while (wins->iter->next)
		wins->iter = wins->iter->next;
}
