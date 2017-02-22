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

struct tabs {
	struct node *iter;
};

static struct node *
create_node(const char *path)
{
	struct node *node;

	node = xcalloc(1, sizeof *node);

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

struct tabs *
tabs_create(void)
{
	struct tabs *tabs;

	tabs = xcalloc(1, sizeof *tabs);
	tabs->iter = create_node("");

	return (tabs);
}

void
tabs_free(struct tabs *tabs)
{
	struct node *node;

	if (tabs) {
		while (tabs->iter->prev)
			tabs->iter = tabs->iter->prev;
		while (tabs->iter) {
			node = tabs->iter;
			tabs->iter = tabs->iter->next;
			view_free(node->view);
			free(node);
		}
		free(tabs);
	}
}

struct view *
tabs_get_view(struct tabs *tabs)
{
	return (tabs->iter->view);
}

int
tabs_open(struct tabs *tabs, const char *path)
{
	struct node *node;
	struct view *view;

	assert(path);

	if (path[0] != '\0' && view_is_new(tabs->iter->view)) {
		if ((view = view_create(path)) == NULL)
			return (0);
		view_free(tabs->iter->view);
		tabs->iter->view = view;
		return (1);
	}

	if ((node = create_node(path)) == NULL)
		return (0);

	insert_after(tabs->iter, node);
	tabs->iter = node;

	return (1);
}

int
tabs_close(struct tabs *tabs, int force)
{
	struct node *node = tabs->iter;

	if (!force && tabs_is_modified(tabs)) {
		error_set("save changes or add ! to override");
		return (0);
	}

	if (node->next == NULL && node->prev == NULL) {
		error_set("cannot close last tab");
		return (0);
	}

	if (node->next) {
		node->next->prev = node->prev;

		if (node->prev)
			node->prev->next = node->next;

		tabs->iter = node->next;
	} else {
		node->prev->next = NULL;
		tabs->iter = node->prev;
	}

	view_free(node->view);
	free(node);

	return (1);
}

int
tabs_is_modified(struct tabs *tabs)
{
	return (view_is_modified(tabs_get_view(tabs)));
}

int
tabs_any_modified(struct tabs *tabs)
{
	struct node *node;

	for (node = tabs->iter->next; node; node = node->next)
		if (view_is_modified(node->view))
			return (1);

	for (node = tabs->iter->prev; node; node = node->prev)
		if (view_is_modified(node->view))
			return (1);

	return (tabs_is_modified(tabs));
}

int
tabs_next(struct tabs *tabs)
{
	if (tabs->iter->next == NULL)
		return (0);

	tabs->iter = tabs->iter->next;

	return (1);
}

int
tabs_prev(struct tabs *tabs)
{
	if (tabs->iter->prev == NULL)
		return (0);

	tabs->iter = tabs->iter->prev;

	return (1);
}

void
tabs_first(struct tabs *tabs)
{
	while (tabs->iter->prev)
		tabs->iter = tabs->iter->prev;
}

void
tabs_last(struct tabs *tabs)
{
	while (tabs->iter->next)
		tabs->iter = tabs->iter->next;
}

int
tabs_get_index(struct tabs *tabs)
{
	struct node *iter = tabs->iter;
	int idx = 0;

	while (iter->prev)
		iter = iter->prev;

	while (iter != tabs->iter) {
		iter = iter->next;
		idx++;
	}

	return (idx);
}

int
tabs_get_count(struct tabs *tabs)
{
	struct node *iter = tabs->iter;
	int cnt = 1;

	while (iter->prev)
		iter = iter->prev;

	while (iter->next) {
		iter = iter->next;
		cnt++;
	}

	return (cnt);
}
