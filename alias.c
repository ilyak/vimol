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

struct alias {
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
alias_find_node(struct alias *alias, const char *name)
{
	struct node node;

	node.name = (char *)name;

	return (bsearch(&node, alias->data, alias->nelts,
	    sizeof(struct node), compare));
}

struct alias *
alias_create(void)
{
	struct alias *alias;

	alias = xcalloc(1, sizeof(*alias));
	alias->nalloc = 8;
	alias->data = xcalloc(alias->nalloc, sizeof(struct node));

	return (alias);
}

void
alias_free(struct alias *alias)
{
	int i;

	for (i = 0; i < alias->nelts; i++) {
		free(alias->data[i].name);
		free(alias->data[i].value);
	}

	free(alias->data);
	free(alias);
}

int
alias_set(struct alias *alias, const char *name, const char *value)
{
	struct node node;
	int i, res;

	node.name = xstrdup(name);
	node.value = xstrdup(value);
	res = 1;

	for (i = 0; i < alias->nelts; i++)
		if ((res = compare(&alias->data[i], &node)) >= 0)
			break;

	if (res == 0) {
		free(alias->data[i].name);
		free(alias->data[i].value);
	} else {
		if (alias->nelts == alias->nalloc) {
			alias->nalloc *= 2;
			alias->data = xrealloc(alias->data,
			    alias->nalloc * sizeof(struct node));
		}
		memmove(alias->data + i + 1, alias->data + i,
		    (alias->nelts - i) * sizeof(struct node));
		alias->nelts++;
	}

	alias->data[i] = node;
	return (res == 0);
}

const char *
alias_get(struct alias *alias, const char *name)
{
	struct node *node = alias_find_node(alias, name);

	return (node ? node->value : NULL);
}
