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
	char *name;
	char *value;
};

struct bind {
	int nelts, nalloc;
	struct node *data;
};

static int
compare(const void *a, const void *b)
{
	const struct node *aa = (const struct node *)a;
	const struct node *bb = (const struct node *)b;

	return (strcasecmp(aa->name, bb->name));
}

static struct node *
bind_find_node(struct bind *bind, const char *name)
{
	struct node node;

	node.name = (char *)name;

	return (bsearch(&node, bind->data, bind->nelts,
	    sizeof node, compare));
}

struct bind *
bind_create(void)
{
	struct bind *bind;

	bind = xcalloc(1, sizeof *bind);
	bind->nalloc = 8;
	bind->data = xcalloc(bind->nalloc, sizeof *bind->data);

	return (bind);
}

void
bind_free(struct bind *bind)
{
	int i;

	if (bind) {
		for (i = 0; i < bind->nelts; i++) {
			free(bind->data[i].name);
			free(bind->data[i].value);
		}
		free(bind->data);
		free(bind);
	}
}

int
bind_set(struct bind *bind, const char *name, const char *value)
{
	struct node node;
	int i, res;

	node.name = xstrdup(name);
	node.value = xstrdup(value);
	res = 1;

	for (i = 0; i < bind->nelts; i++)
		if ((res = compare(&bind->data[i], &node)) >= 0)
			break;

	if (res == 0) {
		free(bind->data[i].name);
		free(bind->data[i].value);
	} else {
		if (bind->nelts == bind->nalloc) {
			bind->nalloc *= 2;
			bind->data = xrealloc(bind->data,
			    bind->nalloc * sizeof *bind->data);
		}
		memmove(bind->data + i + 1, bind->data + i,
		    (bind->nelts - i) * sizeof *bind->data);
		bind->nelts++;
	}

	bind->data[i] = node;
	return (res == 0);
}

const char *
bind_get(struct bind *bind, const char *name)
{
	struct node *node = bind_find_node(bind, name);

	return (node ? node->value : NULL);
}
