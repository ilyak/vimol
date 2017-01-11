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
	struct view *view;
	struct node *next;
	struct node *prev;
};

struct wnd {
	struct node *iter;
};

static struct node *
create_node(const char *path)
{
	struct node *node;

	node = xcalloc(1, sizeof(*node));

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

struct wnd *
wnd_create(void)
{
	struct wnd *wnd;

	wnd = xcalloc(1, sizeof(*wnd));
	wnd->iter = create_node("");

	return (wnd);
}

void
wnd_free(struct wnd *wnd)
{
	struct node *node;

	wnd_first(wnd);

	while (wnd->iter) {
		node = wnd->iter;
		wnd->iter = wnd->iter->next;
		view_free(node->view);
		free(node);
	}

	free(wnd);
}

struct view *
wnd_get_view(struct wnd *wnd)
{
	return (wnd->iter->view);
}

int
wnd_open(struct wnd *wnd, const char *path)
{
	struct node *node;

	if ((node = create_node(path)) == NULL)
		return (0);

	insert_after(wnd->iter, node);
	wnd->iter = node;

	return (1);
}

int
wnd_close(struct wnd *wnd)
{
	struct node *node;

	node = wnd->iter;

	if (node->next == NULL && node->prev == NULL) {
		error_set("cannot close last window");
		return (0);
	}

	if (node->next) {
		node->next->prev = node->prev;

		if (node->prev)
			node->prev->next = node->next;

		wnd->iter = node->next;
	} else {
		node->prev->next = NULL;
		wnd->iter = node->prev;
	}

	view_free(node->view);
	free(node);

	return (1);
}

int
wnd_is_modified(struct wnd *wnd)
{
	return (view_is_modified(wnd_get_view(wnd)));
}

int
wnd_any_modified(struct wnd *wnd)
{
	struct node *node;

	for (node = wnd->iter->next; node; node = node->next)
		if (view_is_modified(node->view))
			return (1);

	for (node = wnd->iter->prev; node; node = node->prev)
		if (view_is_modified(node->view))
			return (1);

	return (wnd_is_modified(wnd));
}

int
wnd_next(struct wnd *wnd)
{
	if (wnd->iter->next == NULL)
		return (0);

	wnd->iter = wnd->iter->next;

	return (1);
}

int
wnd_prev(struct wnd *wnd)
{
	if (wnd->iter->prev == NULL)
		return (0);

	wnd->iter = wnd->iter->prev;

	return (1);
}

void
wnd_first(struct wnd *wnd)
{
	while (wnd->iter->prev)
		wnd->iter = wnd->iter->prev;
}

void
wnd_last(struct wnd *wnd)
{
	while (wnd->iter->next)
		wnd->iter = wnd->iter->next;
}

int
wnd_get_index(struct wnd *wnd)
{
	struct node *iter = wnd->iter;
	int idx = 0;

	while (iter->prev)
		iter = iter->prev;

	while (iter != wnd->iter) {
		iter = iter->next;
		idx++;
	}

	return (idx);
}

int
wnd_get_count(struct wnd *wnd)
{
	struct node *iter = wnd->iter;
	int cnt = 1;

	while (iter->prev)
		iter = iter->prev;

	while (iter->next) {
		iter = iter->next;
		cnt++;
	}

	return (cnt);
}
