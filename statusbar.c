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

struct statusbar {
	int is_error;
	int cursor_pos;
	char text[1024];
	char info[1024];
};

struct statusbar *
statusbar_create(void)
{
	struct statusbar *statusbar;

	statusbar = xcalloc(1, sizeof *statusbar);

	return (statusbar);
}

void
statusbar_free(struct statusbar *statusbar)
{
	free(statusbar);
}

const char *
statusbar_get_text(struct statusbar *statusbar)
{
	return (statusbar->text);
}

void
statusbar_set_text(struct statusbar *statusbar, const char *fmt, ...)
{
	va_list ap;

	statusbar->is_error = 0;

	va_start(ap, fmt);
	vsnprintf(statusbar->text, sizeof statusbar->text, fmt, ap);
	va_end(ap);
}

void
statusbar_set_error(struct statusbar *statusbar, const char *fmt, ...)
{
	va_list ap;

	statusbar->is_error = 1;

	va_start(ap, fmt);
	vsnprintf(statusbar->text, sizeof statusbar->text, fmt, ap);
	va_end(ap);
}

void
statusbar_clear_text(struct statusbar *statusbar)
{
	statusbar->text[0] = '\0';
}

const char *
statusbar_get_info_text(struct statusbar *statusbar)
{
	return (statusbar->info);
}

void
statusbar_set_info_text(struct statusbar *statusbar, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(statusbar->info, sizeof statusbar->info, fmt, ap);
	va_end(ap);
}

void
statusbar_clear_info_text(struct statusbar *statusbar)
{
	statusbar->info[0] = '\0';
}

void
statusbar_set_cursor_pos(struct statusbar *statusbar, int value)
{
	statusbar->cursor_pos = value;
}

void
statusbar_render(struct statusbar *statusbar, cairo_t *cairo)
{
	cairo_font_extents_t exfont;
	cairo_text_extents_t extext;
	color_t color;
	double size;
	int width, height;
	const char *font;
	char *text;

	font = settings_get_string("statusbar-font");
	size = settings_get_double("statusbar-font-size");

	cairo_reset_clip(cairo);
	cairo_identity_matrix(cairo);
	cairo_select_font_face(cairo, font, CAIRO_FONT_SLANT_NORMAL,
	    CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cairo, size);
	cairo_font_extents(cairo, &exfont);

	width = cairo_image_surface_get_width(cairo_get_target(cairo));
	height = cairo_image_surface_get_height(cairo_get_target(cairo));

	color = settings_get_color("statusbar-color");
	cairo_set_source_rgb(cairo, color.r, color.g, color.b);
	cairo_rectangle(cairo, 0, height - exfont.height - 10,
	    width, exfont.height + 10);
	cairo_fill(cairo);

	color = settings_get_color("statusbar-text-color");
	cairo_set_source_rgb(cairo, color.r, color.g, color.b);

	cairo_text_extents(cairo, statusbar->info, &extext);
	cairo_move_to(cairo, width - extext.x_advance - 5,
	    height - exfont.descent - 5);
	cairo_show_text(cairo, statusbar->info);

	cairo_rectangle(cairo, 0, height - exfont.height - 10,
	    width - extext.x_advance - 10, exfont.height + 10);
	cairo_clip(cairo);

	if (statusbar->is_error) {
		color = settings_get_color("statusbar-error-color");
		cairo_set_source_rgb(cairo, color.r, color.g, color.b);
	}

	cairo_move_to(cairo, 5, height - exfont.descent - 5);
	cairo_show_text(cairo, statusbar->text);

	if (statusbar->cursor_pos > -1) {
		text = xstrndup(statusbar->text, (size_t)statusbar->cursor_pos);
		cairo_text_extents(cairo, text, &extext);
		cairo_move_to(cairo, extext.x_advance + 5,
		    height - exfont.descent - 5);
		cairo_show_text(cairo, "_");
		free(text);
	}
}
