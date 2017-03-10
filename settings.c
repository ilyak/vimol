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

enum node_type {
	NODE_TYPE_INT,
	NODE_TYPE_DOUBLE,
	NODE_TYPE_BOOL,
	NODE_TYPE_STRING,
	NODE_TYPE_COLOR
};

union data {
	int xint;
	double xdouble;
	int xbool;
	char *xstring;
	color_t xcolor;
};

struct node {
	const char *name;
	enum node_type type;
	union data data;
};

struct settings {
	int nelts, nalloc;
	struct node *data;
};

static struct settings *settings = NULL;

static int
compare(const void *a, const void *b)
{
	const struct node *aa = (const struct node *)a;
	const struct node *bb = (const struct node *)b;

	return (strcasecmp(aa->name, bb->name));
}

static struct node *
find_node(const char *name)
{
	struct node node;

	node.name = name;

	return (bsearch(&node, settings->data, settings->nelts,
	    sizeof node, compare));
}

static int
set_data(struct node *node, union data data)
{
	if (node->type == NODE_TYPE_STRING)
		free(node->data.xstring);

	node->data = data;

	return (1);
}

static int
set_from_string(struct node *node, const char *value)
{
	union data data;
	int r = 0, g = 0, b = 0;

	switch (node->type) {
	case NODE_TYPE_INT:
		data.xint = tok_int((tok_t)value);
		break;
	case NODE_TYPE_DOUBLE:
		data.xdouble = tok_double((tok_t)value);
		break;
	case NODE_TYPE_BOOL:
		data.xbool = tok_bool((tok_t)value);
		break;
	case NODE_TYPE_STRING:
		data.xstring = xstrdup(value);
		break;
	case NODE_TYPE_COLOR:
		sscanf(value, "%d %d %d", &r, &g, &b);
		data.xcolor = color_rgb(r, g, b);
		break;
	}

	return (set_data(node, data));
}

static void
add_node(const char *name, enum node_type type, const char *default_value)
{
	if (settings->nalloc == settings->nelts) {
		settings->nalloc *= 2;
		settings->data = xrealloc(settings->data,
		    settings->nalloc * sizeof *settings->data);
	}
	memset(&settings->data[settings->nelts], 0, sizeof *settings->data);
	settings->data[settings->nelts].name = name;
	settings->data[settings->nelts].type = type;
	set_from_string(settings->data + settings->nelts, default_value);
	settings->nelts++;
}

static void
add_data_path(void)
{
	char *path, *prefix;

	prefix = getenv("HOME");

	if (prefix) {
		xasprintf(&path, "%s/.vimolrc", prefix);
		add_node("vimolrc-path", NODE_TYPE_STRING, path);
		free(path);
		xasprintf(&path, "%s/.vimolhistory", prefix);
		add_node("vimolhistory-path", NODE_TYPE_STRING, path);
		free(path);
	} else {
		add_node("vimolrc-path", NODE_TYPE_STRING, "");
		add_node("vimolhistory-path", NODE_TYPE_STRING, "");
	}
}

static const struct {
	const char *name;
	enum node_type type;
	const char *default_value;
} nodelist[] = {
	{ "atom-size", NODE_TYPE_DOUBLE, "8.0" },
	{ "atom-visible", NODE_TYPE_BOOL, "true" },
	{ "bg-color", NODE_TYPE_COLOR, "0 0 0" },
	{ "bond-size", NODE_TYPE_DOUBLE, "3.0" },
	{ "bond-visible", NODE_TYPE_BOOL, "true" },
	{ "id-color", NODE_TYPE_COLOR, "255 255 255" },
	{ "id-font", NODE_TYPE_STRING, VIMOL_DEFAULT_FONT },
	{ "id-font-size", NODE_TYPE_DOUBLE, "12.0" },
	{ "id-visible", NODE_TYPE_BOOL, "false" },
	{ "name-color", NODE_TYPE_COLOR, "0 255 0" },
	{ "name-font", NODE_TYPE_STRING, VIMOL_DEFAULT_FONT },
	{ "name-font-size", NODE_TYPE_DOUBLE, "12.0" },
	{ "name-visible", NODE_TYPE_BOOL, "false" },
	{ "origin-color", NODE_TYPE_COLOR, "255 255 255" },
	{ "origin-font", NODE_TYPE_STRING, VIMOL_DEFAULT_FONT },
	{ "origin-font-size", NODE_TYPE_DOUBLE, "16.0" },
	{ "origin-line-width", NODE_TYPE_DOUBLE, "2.0" },
	{ "origin-visible", NODE_TYPE_BOOL, "false" },
	{ "selection-color", NODE_TYPE_COLOR, "255 255 0" },
	{ "selection-size", NODE_TYPE_DOUBLE, "12.0" },
	{ "statusbar-color", NODE_TYPE_COLOR, "220 220 220" },
	{ "statusbar-error-color", NODE_TYPE_COLOR, "220 0 0" },
	{ "statusbar-font", NODE_TYPE_STRING, VIMOL_DEFAULT_FONT },
	{ "statusbar-font-size", NODE_TYPE_DOUBLE, "16.0" },
	{ "statusbar-text-color", NODE_TYPE_COLOR, "0 0 0" },
	{ "statusbar-visible", NODE_TYPE_BOOL, "true" },
	{ "color-x", NODE_TYPE_COLOR, "255 0 255" },
	{ "color-h", NODE_TYPE_COLOR, "255 255 255" },
	{ "color-he", NODE_TYPE_COLOR, "217 255 255" },
	{ "color-li", NODE_TYPE_COLOR, "205 126 255" },
	{ "color-be", NODE_TYPE_COLOR, "197 255 0" },
	{ "color-b", NODE_TYPE_COLOR, "255 183 183" },
	{ "color-c", NODE_TYPE_COLOR, "146 146 146" },
	{ "color-n", NODE_TYPE_COLOR, "143 143 255" },
	{ "color-o", NODE_TYPE_COLOR, "240 0 0" },
	{ "color-f", NODE_TYPE_COLOR, "179 255 255" },
	{ "color-ne", NODE_TYPE_COLOR, "175 227 244" },
	{ "color-na", NODE_TYPE_COLOR, "170 94 242" },
	{ "color-mg", NODE_TYPE_COLOR, "137 255 0" },
	{ "color-al", NODE_TYPE_COLOR, "210 165 165" },
	{ "color-si", NODE_TYPE_COLOR, "129 154 154" },
	{ "color-p", NODE_TYPE_COLOR, "255 128 0" },
	{ "color-s", NODE_TYPE_COLOR, "255 200 50" },
	{ "color-cl", NODE_TYPE_COLOR, "32 240 32" },
	{ "color-ar", NODE_TYPE_COLOR, "129 209 228" },
	{ "color-k", NODE_TYPE_COLOR, "143 65 211" },
	{ "color-ca", NODE_TYPE_COLOR, "61 255 0" },
	{ "color-sc", NODE_TYPE_COLOR, "230 230 228" },
	{ "color-ti", NODE_TYPE_COLOR, "192 195 198" },
	{ "color-v", NODE_TYPE_COLOR, "167 165 172" },
	{ "color-cr", NODE_TYPE_COLOR, "139 153 198" },
	{ "color-mn", NODE_TYPE_COLOR, "156 123 198" },
	{ "color-fe", NODE_TYPE_COLOR, "129 123 198" },
	{ "color-co", NODE_TYPE_COLOR, "112 123 195" },
	{ "color-ni", NODE_TYPE_COLOR, "93 123 195" },
	{ "color-cu", NODE_TYPE_COLOR, "255 123 98" },
	{ "color-zn", NODE_TYPE_COLOR, "124 129 175" },
	{ "color-ga", NODE_TYPE_COLOR, "195 146 145" },
	{ "color-ge", NODE_TYPE_COLOR, "102 146 146" },
	{ "color-as", NODE_TYPE_COLOR, "190 129 227" },
	{ "color-se", NODE_TYPE_COLOR, "255 162 0" },
	{ "color-br", NODE_TYPE_COLOR, "165 42 42" },
	{ "color-kr", NODE_TYPE_COLOR, "93 186 209" },
	{ "color-rb", NODE_TYPE_COLOR, "113 46 178" },
	{ "color-sr", NODE_TYPE_COLOR, "0 254 0" },
	{ "color-y", NODE_TYPE_COLOR, "150 253 255" },
	{ "color-zr", NODE_TYPE_COLOR, "150 225 225" },
	{ "color-nb", NODE_TYPE_COLOR, "116 195 203" },
	{ "color-mo", NODE_TYPE_COLOR, "85 181 183" },
	{ "color-tc", NODE_TYPE_COLOR, "60 159 168" },
	{ "color-ru", NODE_TYPE_COLOR, "35 142 151" },
	{ "color-rh", NODE_TYPE_COLOR, "11 124 140" },
	{ "color-pd", NODE_TYPE_COLOR, "0 104 134" },
	{ "color-ag", NODE_TYPE_COLOR, "153 198 255" },
	{ "color-cd", NODE_TYPE_COLOR, "255 216 145" },
	{ "color-in", NODE_TYPE_COLOR, "167 118 115" },
	{ "color-sn", NODE_TYPE_COLOR, "102 129 129" },
	{ "color-sb", NODE_TYPE_COLOR, "159 101 181" },
	{ "color-te", NODE_TYPE_COLOR, "213 123 0" },
	{ "color-i", NODE_TYPE_COLOR, "147 0 147" },
	{ "color-xe", NODE_TYPE_COLOR, "66 159 176" },
	{ "color-cs", NODE_TYPE_COLOR, "87 25 143" },
	{ "color-ba", NODE_TYPE_COLOR, "0 202 0" },
	{ "color-la", NODE_TYPE_COLOR, "112 222 255" },
	{ "color-ce", NODE_TYPE_COLOR, "255 255 200" },
	{ "color-pr", NODE_TYPE_COLOR, "217 255 200" },
	{ "color-nd", NODE_TYPE_COLOR, "198 255 200" },
	{ "color-pm", NODE_TYPE_COLOR, "164 255 200" },
	{ "color-sm", NODE_TYPE_COLOR, "146 255 200" },
	{ "color-eu", NODE_TYPE_COLOR, "99 255 200" },
	{ "color-gd", NODE_TYPE_COLOR, "71 255 200" },
	{ "color-tb", NODE_TYPE_COLOR, "50 255 200" },
	{ "color-dy", NODE_TYPE_COLOR, "31 255 183" },
	{ "color-ho", NODE_TYPE_COLOR, "0 254 157" },
	{ "color-er", NODE_TYPE_COLOR, "0 230 118" },
	{ "color-tm", NODE_TYPE_COLOR, "0 210 83" },
	{ "color-yb", NODE_TYPE_COLOR, "0 191 57" },
	{ "color-lu", NODE_TYPE_COLOR, "0 172 35" },
	{ "color-hf", NODE_TYPE_COLOR, "77 194 255" },
	{ "color-ta", NODE_TYPE_COLOR, "77 167 255" },
	{ "color-w", NODE_TYPE_COLOR, "39 148 214" },
	{ "color-re", NODE_TYPE_COLOR, "39 126 172" },
	{ "color-os", NODE_TYPE_COLOR, "39 104 151" },
	{ "color-ir", NODE_TYPE_COLOR, "24 85 135" },
	{ "color-pt", NODE_TYPE_COLOR, "24 91 145" },
	{ "color-au", NODE_TYPE_COLOR, "255 209 36" },
	{ "color-hg", NODE_TYPE_COLOR, "181 181 195" },
	{ "color-tl", NODE_TYPE_COLOR, "167 85 77" },
	{ "color-pb", NODE_TYPE_COLOR, "87 90 96" },
	{ "color-bi", NODE_TYPE_COLOR, "159 79 181" },
	{ "color-po", NODE_TYPE_COLOR, "172 93 0" },
	{ "color-at", NODE_TYPE_COLOR, "118 79 69" },
	{ "color-rn", NODE_TYPE_COLOR, "66 132 151" },
	{ "color-fr", NODE_TYPE_COLOR, "66 0 102" },
	{ "color-ra", NODE_TYPE_COLOR, "0 123 0" },
	{ "color-ac", NODE_TYPE_COLOR, "113 170 252" },
	{ "color-th", NODE_TYPE_COLOR, "0 186 255" },
	{ "color-pa", NODE_TYPE_COLOR, "0 160 255" },
	{ "color-u", NODE_TYPE_COLOR, "0 145 255" },
	{ "color-np", NODE_TYPE_COLOR, "0 128 242" },
	{ "color-pu", NODE_TYPE_COLOR, "0 106 242" },
	{ "color-am", NODE_TYPE_COLOR, "85 91 242" },
	{ "color-cm", NODE_TYPE_COLOR, "120 91 227" },
	{ "color-bk", NODE_TYPE_COLOR, "137 79 227" },
	{ "color-cf", NODE_TYPE_COLOR, "161 55 213" },
	{ "color-es", NODE_TYPE_COLOR, "179 31 213" },
	{ "color-fm", NODE_TYPE_COLOR, "179 31 186" },
	{ "color-md", NODE_TYPE_COLOR, "179 13 167" },
	{ "color-no", NODE_TYPE_COLOR, "189 13 135" },
	{ "color-lr", NODE_TYPE_COLOR, "201 0 102" },
};
static const size_t nnodelist = sizeof nodelist / sizeof *nodelist;

void
settings_init(void)
{
	size_t i;

	settings = xcalloc(1, sizeof *settings);
	settings->nalloc = 8;
	settings->data = xcalloc(settings->nalloc, sizeof *settings->data);

	add_data_path();

	for (i = 0; i < nnodelist; i++)
		add_node(nodelist[i].name, nodelist[i].type,
		    nodelist[i].default_value);

	qsort(settings->data, settings->nelts, sizeof *settings->data, compare);
}

void
settings_free(void)
{
	int i;

	if (settings) {
		for (i = 0; i < settings->nelts; i++)
			if (settings->data[i].type == NODE_TYPE_STRING)
				free(settings->data[i].data.xstring);
		free(settings->data);
		free(settings);
	}
}

int
settings_printf(char *buf, size_t size, const char *name)
{
	struct node *node;
	int rc = 0;

	node = find_node(name);
	assert(node != NULL);

	switch (node->type) {
	case NODE_TYPE_INT:
		rc = snprintf(buf, size, "%d", node->data.xint);
		break;
	case NODE_TYPE_DOUBLE:
		rc = snprintf(buf, size, "%.3lf", node->data.xdouble);
		break;
	case NODE_TYPE_BOOL:
		rc = snprintf(buf, size, "%s",
		    node->data.xbool ? "true" : "false");
		break;
	case NODE_TYPE_STRING:
		rc = snprintf(buf, size, "%s", node->data.xstring);
		break;
	case NODE_TYPE_COLOR:
		rc = color_to_string(buf, size, node->data.xcolor);
		break;
	}
	return (rc);
}

int
settings_has_int(const char *name)
{
	struct node *node;

	node = find_node(name);

	return (node && node->type == NODE_TYPE_INT);
}

int
settings_has_double(const char *name)
{
	struct node *node;

	node = find_node(name);

	return (node && node->type == NODE_TYPE_DOUBLE);
}

int
settings_has_bool(const char *name)
{
	struct node *node;

	node = find_node(name);

	return (node && node->type == NODE_TYPE_BOOL);
}

int
settings_has_string(const char *name)
{
	struct node *node;

	node = find_node(name);

	return (node && node->type == NODE_TYPE_STRING);
}

int
settings_has_color(const char *name)
{
	struct node *node;

	node = find_node(name);

	return (node && node->type == NODE_TYPE_COLOR);
}

int
settings_has_node(const char *name)
{
	return (find_node(name) != NULL);
}

int
settings_get_int(const char *name)
{
	struct node *node;

	node = find_node(name);

	assert(node != NULL);
	assert(node->type == NODE_TYPE_INT);

	return (node->data.xint);
}

double
settings_get_double(const char *name)
{
	struct node *node;

	node = find_node(name);

	assert(node != NULL);
	assert(node->type == NODE_TYPE_DOUBLE);

	return (node->data.xdouble);
}

int
settings_get_bool(const char *name)
{
	struct node *node;

	node = find_node(name);

	assert(node != NULL);
	assert(node->type == NODE_TYPE_BOOL);

	return (node->data.xbool);
}

const char *
settings_get_string(const char *name)
{
	struct node *node;

	node = find_node(name);

	assert(node != NULL);
	assert(node->type == NODE_TYPE_STRING);

	return (node->data.xstring);
}

color_t
settings_get_color(const char *name)
{
	struct node *node;

	node = find_node(name);

	assert(node != NULL);
	assert(node->type == NODE_TYPE_COLOR);

	return (node->data.xcolor);
}

int
settings_set(const char *name, const char *value)
{
	struct node *node;

	if ((node = find_node(name)) == NULL) {
		error_set("unknown setting %s", name);
		return (0);
	}

	return (set_from_string(node, value));
}
