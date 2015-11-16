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

	alias = calloc(1, sizeof(*alias));
	alias->nalloc = 8;
	alias->data = calloc(alias->nalloc, sizeof(struct node));

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
			alias->data = realloc(alias->data,
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
