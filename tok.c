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

struct tokq {
	int nelts, nalloc;
	char **data;
};

int
tok_int(tok_t tok)
{
	assert(tok != NULL);

	return ((int)strtol(tok, NULL, 10));
}

double
tok_double(tok_t tok)
{
	assert(tok != NULL);

	return (strtod(tok, NULL));
}

int
tok_bool(tok_t tok)
{
	assert(tok != NULL);

	return (strcasecmp(tok, "true") == 0);
}

const char *
tok_string(tok_t tok)
{
	assert(tok != NULL);

	return ((const char *)tok);
}

color_t
tok_color(tok_t tok)
{
	int r, g, b;

	assert(tok != NULL);

	r = g = b = 0;
	sscanf(tok, "[ %d %d %d ]", &r, &g, &b);

	return (color_rgb(r, g, b));
}

vec_t
tok_vec(tok_t tok)
{
	double x, y, z;

	assert(tok != NULL);

	x = y = z = 0.0;
	sscanf(tok, "[ %lf %lf %lf ]", &x, &y, &z);

	return (vec_xyz(x, y, z));
}

static void
tokq_push_back(struct tokq *tokq, char *str)
{
	if (tokq->nelts == tokq->nalloc) {
		tokq->nalloc *= 2;
		tokq->data = realloc(tokq->data, tokq->nalloc * sizeof(char *));
	}
	tokq->data[tokq->nelts++] = str;
}

static char *
next_token(const char **str)
{
	size_t size = 0;
	const char *ptr = *str;

	if (**str == ';')
		return (xstrndup((*str)++, 1));

	if (**str == '"') {
		(*str)++;

		while (**str && **str != '"')
			(*str)++, size++;

		if (**str == '"')
			(*str)++;
		else {
			error_set("no matching '\"' found");
			return (NULL);
		}

		return (xstrndup(ptr + 1, size));
	}

	if (**str == '[') {
		(*str)++, size++;

		while (**str && **str != ']')
			(*str)++, size++;

		if (**str == ']')
			(*str)++, size++;
		else {
			error_set("no matching ']' found");
			return (NULL);
		}

		return (xstrndup(ptr, size));
	}

	while (**str && !isspace(**str) && !strchr(";\"[", **str))
		(*str)++, size++;

	return (xstrndup(ptr, size));
}

static int
parse_string(struct tokq *tokq, const char *str)
{
	char *tok;

	while (*str && isspace(*str))
		str++;

	while (*str) {
		if ((tok = next_token(&str)) == NULL)
			return (FALSE);

		tokq_push_back(tokq, tok);

		while (*str && isspace(*str))
			str++;
	}

	return (TRUE);
}

static struct tokq *
tokq_create_empty(void)
{
	struct tokq *tokq;

	tokq = calloc(1, sizeof(*tokq));
	tokq->nalloc = 8;
	tokq->data = calloc(tokq->nalloc, sizeof(char *));

	return (tokq);
}

struct tokq *
tokq_create(const char *str)
{
	struct tokq *tokq = tokq_create_empty();

	if (!parse_string(tokq, str)) {
		tokq_free(tokq);
		return (NULL);
	}

	return (tokq);
}

struct tokq *
tokq_copy(struct tokq *tokq, int start, int count)
{
	struct tokq *copy;
	int i;

	assert(start >= 0 && start + count <= tokq_count(tokq));

	copy = tokq_create_empty();

	for (i = start; i < start + count; i++)
		tokq_push_back(copy, xstrdup(tokq_tok(tokq, i)));

	return (copy);
}

void
tokq_free(struct tokq *tokq)
{
	int i;

	for (i = 0; i < tokq_count(tokq); i++)
		free(tokq->data[i]);

	free(tokq->data);
	free(tokq);
}

char *
tokq_strcat(struct tokq *tokq, int start, int count)
{
	char *s;
	int i;

	assert(start >= 0 && start + count <= tokq_count(tokq));

	s = xstrdup("");

	for (i = start; i < start + count; i++) {
		s = xstrcat(s, tokq->data[i]);

		if (i < start + count - 1)
			s = xstrcat(s, " ");
	}

	return (s);
}

int
tokq_count(struct tokq *tokq)
{
	return (tokq->nelts);
}

tok_t
tokq_tok(struct tokq *tokq, int idx)
{
	assert(idx >= 0 && idx < tokq_count(tokq));

	return (tokq->data[idx]);
}
