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

struct status {
	int is_error;
	int cursor_pos;
	char text[1024];
	char info[1024];
};

struct status *
status_create(void)
{
	struct status *status;

	status = calloc(1, sizeof(*status));

	return (status);
}

void
status_free(struct status *status)
{
	free(status);
}

const char *
status_get_text(struct status *status)
{
	return (status->text);
}

void
status_set_text(struct status *status, const char *fmt, ...)
{
	va_list ap;

	status->is_error = 0;

	va_start(ap, fmt);
	vsnprintf(status->text, sizeof(status->text), fmt, ap);
	va_end(ap);
}

void
status_set_error(struct status *status, const char *fmt, ...)
{
	va_list ap;

	status->is_error = 1;

	va_start(ap, fmt);
	vsnprintf(status->text, sizeof(status->text), fmt, ap);
	va_end(ap);
}

void
status_clear_text(struct status *status)
{
	status->text[0] = '\0';
}

const char *
status_get_info_text(struct status *status)
{
	return (status->info);
}

void
status_set_info_text(struct status *status, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(status->info, sizeof(status->info), fmt, ap);
	va_end(ap);
}

void
status_clear_info_text(struct status *status)
{
	status->info[0] = '\0';
}

void
status_set_cursor_pos(struct status *status, int value)
{
	status->cursor_pos = value;
}

void
status_render(struct status *status, cairo_t *cairo)
{
	cairo_font_extents_t exfont;
	cairo_text_extents_t extext;
	color_t color;
	double size;
	int width, height;
	const char *font;
	char *text;

	font = settings_get_string("status.font");
	size = settings_get_double("status.font.size");

	cairo_reset_clip(cairo);
	cairo_identity_matrix(cairo);
	cairo_select_font_face(cairo, font, CAIRO_FONT_SLANT_NORMAL,
	    CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cairo, size);
	cairo_font_extents(cairo, &exfont);

	width = cairo_image_surface_get_width(cairo_get_target(cairo));
	height = cairo_image_surface_get_height(cairo_get_target(cairo));

	if (!settings_get_bool("status.transparent")) {
		color = settings_get_color("status.color");
		cairo_set_source_rgb(cairo, color.r, color.g, color.b);
		cairo_rectangle(cairo, 0, height - exfont.height - 10,
		    width, exfont.height + 10);
		cairo_fill(cairo);
	}

	color = settings_get_color("status.text.color");
	cairo_set_source_rgb(cairo, color.r, color.g, color.b);

	cairo_text_extents(cairo, status->info, &extext);
	cairo_move_to(cairo, width - extext.x_advance - 5,
	    height - exfont.descent - 5);
	cairo_show_text(cairo, status->info);

	cairo_rectangle(cairo, 0, height - exfont.height - 10,
	    width - extext.x_advance - 10, exfont.height + 10);
	cairo_clip(cairo);

	if (status->is_error) {
		color = settings_get_color("status.error.color");
		cairo_set_source_rgb(cairo, color.r, color.g, color.b);
	}

	cairo_move_to(cairo, 5, height - exfont.descent - 5);
	cairo_show_text(cairo, status->text);

	if (status->cursor_pos > -1) {
		text = xstrndup(status->text, (size_t)status->cursor_pos);
		cairo_text_extents(cairo, text, &extext);
		cairo_move_to(cairo, extext.x_advance + 5,
		    height - exfont.descent - 5);
		cairo_show_text(cairo, "_");
		free(text);
	}
}
