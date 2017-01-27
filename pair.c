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

struct pairs {
	int nelts, nalloc;
	struct pair *data;
};

struct pairs *
pairs_create(void)
{
	struct pairs *pairs;

	pairs = xcalloc(1, sizeof *pairs);
	pairs->nalloc = 8;
	pairs->data = xcalloc(pairs->nalloc, sizeof *pairs->data);

	return (pairs);
}

void
pairs_free(struct pairs *pairs)
{
	if (pairs) {
		free(pairs->data);
		free(pairs);
	}
}

void
pairs_clear(struct pairs *pairs)
{
	pairs->nelts = 0;
}

void
pairs_add(struct pairs *pairs, int i, int j)
{
	if (pairs->nelts == pairs->nalloc) {
		pairs->nalloc *= 2;
		pairs->data = xrealloc(pairs->data,
		    pairs->nalloc * sizeof *pairs->data);
	}
	pairs->data[pairs->nelts].i = i;
	pairs->data[pairs->nelts].j = j;
	pairs->nelts++;
}

int
pairs_get_count(struct pairs *pairs)
{
	return (pairs->nelts);
}

struct pair
pairs_get(struct pairs *pairs, int idx)
{
	assert(idx >= 0 && idx < pairs_get_count(pairs));

	return (pairs->data[idx]);
}
