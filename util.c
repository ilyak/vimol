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
