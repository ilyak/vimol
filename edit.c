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

struct edit {
	int pos, len;
	char text[1024];
};

struct edit *
edit_create(void)
{
	struct edit *edit;

	edit = calloc(1, sizeof(*edit));

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
	edit->len = vsnprintf(edit->text, sizeof(edit->text), fmt, ap);
	va_end(ap);

	edit->pos = edit->len;
}

void
edit_insert_char(struct edit *edit, char ch)
{
	int i;

	if (edit->len == sizeof(edit->text) - 1)
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
