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

int
util_is_empty(const char *str)
{
	while (*str && isspace(*str))
		str++;

	return (*str == '\0');
}

int
util_is_comment(const char *str)
{
	while (*str && isspace(*str))
		str++;

	return (*str == '#' || *str == '\0');
}

int
util_file_exists(const char *path)
{
	FILE *fp;

	if ((fp = fopen(path, "r"))) {
		fclose(fp);
		return (TRUE);
	}

	return (FALSE);
}

int
util_has_suffix(const char *str, const char *suf)
{
	size_t strl, sufl;

	strl = strlen(str);
	sufl = strlen(suf);

	return (strl >= sufl && strcasecmp(str + strl - sufl, suf) == 0);
}

char *
util_next_line(char *buffer, FILE *fp)
{
	int ch;
	size_t len, size;

	if (feof(fp)) {
		free(buffer);
		return (NULL);
	}

	len = 0;
	size = 256;

	buffer = (char *)realloc(buffer, size);

	while ((ch = fgetc(fp)) != EOF) {
		if (len == size) {
			size *= 2;
			buffer = (char *)realloc(buffer, size);
		}

		if (ch == '\n' || ch == '\r') {
			buffer[len] = '\0';
			return (buffer);
		}

		buffer[len++] = (char)ch;
	}

	if (len == size)
		buffer = (char *)realloc(buffer, size + 1);

	buffer[len] = '\0';

	return (buffer);
}
