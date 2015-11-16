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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "pair.h"

struct pairs {
	int nelts, nalloc;
	struct pair *data;
};

struct pairs *
pairs_create(void)
{
	struct pairs *pairs;

	pairs = calloc(1, sizeof(*pairs));
	pairs->nalloc = 8;
	pairs->data = calloc(pairs->nalloc, sizeof(struct pair));

	return (pairs);
}

void
pairs_free(struct pairs *pairs)
{
	free(pairs->data);
	free(pairs);
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
		pairs->data = realloc(pairs->data,
		    pairs->nalloc * sizeof(struct pair));
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
