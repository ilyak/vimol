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
