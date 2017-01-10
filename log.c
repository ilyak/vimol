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

static FILE *log_file = NULL;

static void
log_vwrite(const char *fmt, va_list ap)
{
	assert(log_file != NULL);

	vfprintf(log_file, fmt, ap);
}

static void
log_write(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	log_vwrite(fmt, ap);
	va_end(ap);
}

void
log_open(const char *path)
{
	assert(log_file == NULL);

	log_file = fopen(path, "a");
}

void
log_close(void)
{
	if (log_file)
		fclose(log_file);

	log_file = NULL;
}

void
log_warn(const char *fmt, ...)
{
	va_list ap;

	if (log_file == NULL)
		return;

	log_write("warning: ");

	va_start(ap, fmt);
	log_vwrite(fmt, ap);
	va_end(ap);

	log_write("\n");
}

void
log_fatal(const char *fmt, ...)
{
	va_list ap;

	if (log_file == NULL)
		exit(1);

	log_write("fatal error: ");

	va_start(ap, fmt);
	log_vwrite(fmt, ap);
	va_end(ap);

	log_write("\n");
	log_close();

	exit(1);
}
