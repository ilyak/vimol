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

color_t
color_rgb(int r, int g, int b)
{
	color_t color;

	r = r < 0 ? 0 : r > 255 ? 255 : r;
	g = g < 0 ? 0 : g > 255 ? 255 : g;
	b = b < 0 ? 0 : b > 255 ? 255 : b;

	color.r = (double)r / 255;
	color.g = (double)g / 255;
	color.b = (double)b / 255;

	return (color);
}

int
color_to_string(char *buf, size_t size, color_t color)
{
	int r, g, b;

	r = (int)(color.r * 255);
	g = (int)(color.g * 255);
	b = (int)(color.b * 255);

	return (snprintf(buf, size, "%d %d %d", r, g, b));
}

int
string_is_whitespace(const char *str)
{
	while (*str && isspace(*str))
		str++;

	return (*str == '\0');
}

int
string_is_comment(const char *str)
{
	while (*str && isspace(*str))
		str++;

	return (*str == '#' || *str == '\0');
}

int
string_has_suffix(const char *str, const char *suf)
{
	size_t strl, sufl;

	strl = strlen(str);
	sufl = strlen(suf);

	return (strl >= sufl && strcasecmp(str + strl - sufl, suf) == 0);
}

int
util_file_exists(const char *path)
{
	FILE *fp;

	if ((fp = fopen(path, "r"))) {
		fclose(fp);
		return (1);
	}

	return (0);
}

const char *
util_basename(const char *path)
{
	const char *ptr;

	if ((ptr = strrchr(path, '/')) == NULL)
		if ((ptr = strrchr(path, '\\')) == NULL)
			ptr = path;
	if (ptr != path)
		ptr++;

	return (ptr);
}

char *
util_next_line(char *buffer, FILE *fp)
{
	size_t len, size;
	int ch;

	if (feof(fp)) {
		free(buffer);
		return (NULL);
	}

	len = 0;
	size = 256;

	buffer = xrealloc(buffer, size);

	while ((ch = fgetc(fp)) != EOF) {
		if (len == size) {
			size *= 2;
			buffer = xrealloc(buffer, size);
		}

		if (ch == '\n' || ch == '\r') {
			if (ch == '\r') {
				if ((ch = fgetc(fp)) != '\n')
					ungetc(ch, fp);
			}
			buffer[len] = '\0';
			return (buffer);
		}

		buffer[len++] = (char)ch;
	}

	if (len == size)
		buffer = xrealloc(buffer, size + 1);

	buffer[len] = '\0';

	return (buffer);
}

static void
verrorbox(const char *fmt, va_list ap)
{
	char msg[1024];

	vsnprintf(msg, sizeof msg, fmt, ap);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", msg, NULL);
}

void
warn(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	verrorbox(fmt, ap);
	va_end(ap);
}

void
fatal(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	verrorbox(fmt, ap);
	va_end(ap);

	exit(1);
}
