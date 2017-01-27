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

struct edit {
	int pos, len;
	char text[1024];
};

struct edit *
edit_create(void)
{
	struct edit *edit;

	edit = xcalloc(1, sizeof *edit);

	return (edit);
}

void
edit_free(struct edit *edit)
{
	free(edit);
}

const char *
edit_get_text(struct edit *edit)
{
	return (edit->text);
}

int
edit_get_text_length(struct edit *edit)
{
	return (edit->len);
}

int
edit_get_pos(struct edit *edit)
{
	return (edit->pos);
}

void
edit_set_pos(struct edit *edit, int pos)
{
	if (pos < 0 || pos > edit->len)
		return;

	edit->pos = pos;
}

void
edit_set_text(struct edit *edit, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	edit->len = vsnprintf(edit->text, sizeof edit->text, fmt, ap);
	va_end(ap);

	edit->pos = edit->len;
}

void
edit_insert_char(struct edit *edit, char ch)
{
	int i;

	if (edit->len + 1 == sizeof edit->text)
		return;

	for (i = edit->len + 1; i > edit->pos; i--)
		edit->text[i] = edit->text[i - 1];

	edit->text[edit->pos] = ch;
	edit->pos++;
	edit->len++;
}

void
edit_insert_text(struct edit *edit, const char *text)
{
	while (*text)
		edit_insert_char(edit, *text++);
}

void
edit_backspace_char(struct edit *edit)
{
	int i;

	if (edit->pos == 0 || edit->len == 0)
		return;

	for (i = edit->pos - 1; i < edit->len; i++)
		edit->text[i] = edit->text[i + 1];

	edit->pos--;
	edit->len--;
}

void
edit_delete_char(struct edit *edit)
{
	int i;

	if (edit->pos == edit->len || edit->len == 0)
		return;

	for (i = edit->pos; i < edit->len; i++)
		edit->text[i] = edit->text[i + 1];

	edit->len--;
}

void
edit_backspace_word(struct edit *edit)
{
	while (edit->pos > 0 && isspace(edit->text[edit->pos - 1]))
		edit_backspace_char(edit);

	while (edit->pos > 0 && !isspace(edit->text[edit->pos - 1]))
		edit_backspace_char(edit);
}

void
edit_clear(struct edit *edit)
{
	edit->text[0] = '\0';
	edit->pos = 0;
	edit->len = 0;
}
