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

static void
select_connected(struct graph *graph, int idx, struct sel *sel)
{
	struct graphedge *edge;

	if (sel_selected(sel, idx))
		return;

	sel_add(sel, idx);

	for (edge = graph_edges(graph, idx); edge; edge = graph_edge_next(edge))
		select_connected(graph, graph_edge_j(edge), sel);
}

static struct sel *
make_sel(struct tokq *args, int arg_start, int arg_end, struct sel *current)
{
	struct sel *ret;
	int i, k, size, start, end;
	const char *str;

	if (arg_start >= arg_end)
		return (sel_copy(current));

	size = sel_get_size(current);
	ret = sel_create(size);

	for (k = arg_start; k < arg_end; k++) {
		str = tok_string(tokq_tok(args, k));

		if (strcmp(str, "*") == 0) {
			sel_all(ret);
			break;
		}

		start = end = 0;
		sscanf(str, "%d:%d", &start, &end);

		start = start < 0 ? size + start : start - 1;
		end = end < 0 ? size + end : end - 1;

		if (start < 0 || start >= size)
			continue;

		if (end < 0)
			end = start;

		if (end >= size)
			end = size - 1;

		for (i = start; i <= end; i++)
			sel_add(ret, i);
	}

	return (ret);
}

static struct pairs *
make_pairs(struct tokq *args, int arg_start, int arg_end,
    struct sel *selection)
{
	struct pairs *pairs;
	int i, j, k, size;

	pairs = pairs_create();
	size = sel_get_size(selection);

	if (arg_start >= arg_end) {
		sel_iter_start(selection);

		while (sel_iter_next(selection, &i))
			if (sel_iter_next(selection, &j))
				pairs_add(pairs, i, j);

		return (pairs);
	}

	for (k = arg_start; k < arg_end; k++) {
		i = tok_int(tokq_tok(args, k));

		if (++k < arg_end) {
			j = tok_int(tokq_tok(args, k));

			if (i < 0) i = size + i;
			else i = i - 1;

			if (j < 0) j = size + j;
			else j = j - 1;

			if (i >= 0 && i < size && j >= 0 && j < size)
				pairs_add(pairs, i, j);
		}
	}

	return (pairs);
}

static int
parse_register(tok_t tok)
{
	const char *str;
	int reg;

	str = tok_string(tok);

	if (strlen(str) > 1) {
		error_set("register must be a single letter");
		return (-1);
	}

	reg = tolower(str[0]);

	if (reg < 'a' || reg > 'z') {
		error_set("register must be a letter from 'a' to 'z'");
		return (-1);
	}

	return (reg - 'a');
}

static int
fn_about(struct tokq *args __unused, struct state *state __unused)
{
	error_set("vimol (c) 2013-2017 Ilya Kaliman");

	return (1);
}

static int
fn_atom(struct tokq *args, struct state *state)
{
	struct view *view;
	const char *name;
	vec_t xyz;

	view = state_get_view(state);

	if (tokq_count(args) < 1)
		name = "X";
	else
		name = tok_string(tokq_tok(args, 0));

	if (tokq_count(args) < 2)
		xyz = vec_zero();
	else
		xyz = tok_vec(tokq_tok(args, 1));

	view_snapshot(view);
	sys_add_atom(view_get_sys(view), name, xyz);

	return (1);
}

static int
fn_bind(struct tokq *args, struct state *state)
{
	struct bind *bind = state_get_bind(state);
	const char *name;
	char *value;

	if (tokq_count(args) < 2) {
		error_set("specify name and value");
		return (0);
	}

	name = tok_string(tokq_tok(args, 0));
	value = tokq_strcat(args, 1, tokq_count(args) - 1);

	bind_set(bind, name, value);
	free(value);

	return (1);
}

static int
fn_get_bind(struct tokq *args, struct state *state)
{
	struct bind *bind = state_get_bind(state);
	const char *name, *value;

	if (tokq_count(args) < 1) {
		error_set("specify a name");
		return (0);
	}

	name = tok_string(tokq_tok(args, 0));

	if ((value = bind_get(bind, name)))
		error_set("\"%s\" is assigned to \"%s\"", name, value);
	else
		error_set("\"%s\" is not assigned", name);

	return (1);
}

static int
fn_all(struct tokq *args, struct state *state)
{
	struct view *view;
	struct cmdq *cmdq;
	int i, save, ret;
	char *str;

	view = state_get_view(state);
	str = tokq_strcat(args, 0, tokq_count(args));

	if ((cmdq = cmdq_from_string(str)) == NULL)
		return (0);

	ret = 1;
	save = sys_get_frame(view_get_sys(view));

	for (i = 0; i < sys_get_frame_count(view_get_sys(view)); i++) {
		sys_set_frame(view_get_sys(view), i);

		if (!(ret = cmdq_exec(cmdq, state)))
			break;
	}

	sys_set_frame(view_get_sys(view), save);

	cmdq_free(cmdq);
	free(str);

	return (ret);
}

static int
fn_get_angle(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sys *sys;
	struct sel *sel;
	vec_t pa, pb, pc;
	int a, b;
	double value;

	view = state_get_view(state);
	sys = view_get_sys(view);
	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));

	if (sel_get_count(sel) < 3) {
		sel_free(sel);
		error_set("specify at least 3 atoms");
		return (0);
	}

	sel_iter_start(sel);
	sel_iter_next(sel, &a);
	sel_iter_next(sel, &b);
	sel_remove(sel, a);
	sel_remove(sel, b);

	pa = sys_get_atom_xyz(sys, a);
	pb = sys_get_atom_xyz(sys, b);
	pc = sys_get_sel_center(sys, sel);

	value = vec_angle(&pa, &pb, &pc) * 180 / PI;
	error_set("angle %d-%d-sel: %.3lf", a + 1, b + 1, value);

	sel_free(sel);
	return (1);
}

static int
fn_coord(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sys *sys;
	vec_t xyz;
	double factor;
	int i;

	view = state_get_view(state);

	if (tokq_count(args) < 1) {
		error_set("specify units");
		return (0);
	}

	if (strcasecmp(tok_string(tokq_tok(args, 0)), "bohr") == 0) {
		factor = 0.52917721092;
	} else if (strcasecmp(tok_string(tokq_tok(args, 0)), "nm") == 0) {
		factor = 10.0;
	} else {
		error_set("specify 'bohr' or 'nm'");
		return (0);
	}

	view_snapshot(view);
	sys = view_get_sys(view);

	for (i = 0; i < sys_get_atom_count(sys); i++) {
		xyz = sys_get_atom_xyz(sys, i);
		vec_scale(&xyz, factor);
		sys_set_atom_xyz(sys, i, xyz);
	}

	return (1);
}

static int
fn_bond(struct tokq *args, struct state *state)
{
	struct view *view;
	struct graph *graph;
	struct pairs *pairs;
	struct pair pair;
	int k, type = 1;

	view = state_get_view(state);
	pairs = make_pairs(args, 0, tokq_count(args), view_get_sel(view));

	if (pairs_get_count(pairs) == 0) {
		pairs_free(pairs);
		return (1);
	}

	view_snapshot(view);
	graph = view_get_graph(view);

	for (k = 0; k < pairs_get_count(pairs); k++) {
		pair = pairs_get(pairs, k);
		graph_edge_create(graph, pair.i, pair.j, type);
	}

	pairs_free(pairs);
	return (1);
}

static int
fn_remove_bond(struct tokq *args, struct state *state)
{
	struct view *view;
	struct graph *graph;
	struct pairs *pairs;
	struct pair pair;
	int k;

	view = state_get_view(state);
	pairs = make_pairs(args, 0, tokq_count(args), view_get_sel(view));

	if (pairs_get_count(pairs) == 0) {
		pairs_free(pairs);
		return (1);
	}

	view_snapshot(view);
	graph = view_get_graph(view);

	for (k = 0; k < pairs_get_count(pairs); k++) {
		pair = pairs_get(pairs, k);
		graph_edge_remove(graph, pair.i, pair.j);
	}

	pairs_free(pairs);
	return (1);
}

static int
fn_view_center(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sel *sel;

	view = state_get_view(state);
	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));

	view_center_sel(view, sel);
	sel_free(sel);

	return (1);
}

static int
fn_chain(struct tokq *args, struct state *state)
{
	struct view *view;
	struct graph *graph;
	struct sys *sys;
	struct sel *sel;
	mat_t rotmat;
	vec_t dr1, dr2, xyz;
	int i, j;

	view = state_get_view(state);
	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));

	if (sel_get_count(sel) < 2) {
		sel_free(sel);
		error_set("select at least two atoms");
		return (0);
	}

	view_snapshot(view);
	sys = view_get_sys(view);
	graph = view_get_graph(view);

	rotmat = camera_get_rotation(view_get_camera(view));
	rotmat = mat_transpose(&rotmat);

	dr1 = vec_xyz(1.336, 0.0, 0.0);
	dr2 = vec_xyz(0.0, 0.766, 0.0);
	dr1 = mat_vec(&rotmat, &dr1);
	dr2 = mat_vec(&rotmat, &dr2);

	xyz = sys_get_sel_center(sys, sel);

	xyz.x -= (dr1.x * (sel_get_count(sel) - 1) + dr2.x) / 2;
	xyz.y -= (dr1.y * (sel_get_count(sel) - 1) + dr2.y) / 2;
	xyz.z -= (dr1.z * (sel_get_count(sel) - 1) + dr2.z) / 2;

	for (sel_iter_start(sel), j = -1; sel_iter_next(sel, &i); j = i) {
		sys_set_atom_xyz(sys, i, xyz);

		xyz = vec_add(&xyz, &dr1);
		xyz = vec_add(&xyz, &dr2);

		vec_scale(&dr2, -1.0);

		graph_remove_vertex_edges(graph, i);

		if (j > -1)
			graph_edge_create(graph, i, j, 1);
	}

	sel_free(sel);
	return (1);
}

static int
fn_clear(struct tokq *args, struct state *state)
{
	struct view *view;
	struct graph *graph;
	struct sel *sel;
	int i;

	view = state_get_view(state);
	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));

	if (sel_get_count(sel) == 0) {
		sel_free(sel);
		return (0);
	}

	view_snapshot(view);
	graph = view_get_graph(view);

	sel_iter_start(sel);
	while (sel_iter_next(sel, &i))
		graph_remove_vertex_edges(graph, i);

	sel_free(sel);
	return (1);
}

static int
fn_close(struct tokq *args __unused, struct state *state)
{
	struct wnd *wnd = state_get_wnd(state);

	if (wnd_is_modified(wnd)) {
		error_set("save changes or add ! to override");
		return (0);
	}

	return (wnd_close(wnd));
}

static int
fn_force_close(struct tokq *args __unused, struct state *state)
{
	struct wnd *wnd = state_get_wnd(state);

	return (wnd_close(wnd));
}

static int
fn_copy(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sys *sys;
	struct yank *yank;
	struct sel *sel;
	int reg;

	if (tokq_count(args) < 1) {
		error_set("specify copy register");
		return (0);
	}

	if ((reg = parse_register(tokq_tok(args, 0))) == -1)
		return (0);

	yank = state_get_yank(state);
	view = state_get_view(state);
	sys = view_get_sys(view);
	sel = make_sel(args, 1, tokq_count(args), view_get_sel(view));

	if (sel_get_count(sel) == 0) {
		sel_free(sel);
		return (1);
	}

	yank_set_register(yank, reg);
	yank_copy(yank, sys, sel);

	error_set("copied %d atoms to register '%c'", sel_get_count(sel),
	    'a' + reg);

	sel_free(sel);
	return (1);
}

static int
fn_count(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sel *sel;

	view = state_get_view(state);
	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));

	error_set("count is %d", sel_get_count(sel));

	sel_free(sel);
	return (1);
}

static int
fn_delete(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sel *sel;
	struct sys *sys;
	int i;

	view = state_get_view(state);
	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));

	if (sel_get_count(sel) == 0) {
		sel_free(sel);
		return (1);
	}

	view_snapshot(view);
	sys = view_get_sys(view);

	for (i = sys_get_atom_count(sys) - 1; i >= 0; i--) {
		if (!sel_selected(sel, i))
			continue;
		sys_remove_atom(sys, i);
	}

	error_set("deleted %d atoms", sel_get_count(sel));

	sel_free(sel);
	return (1);
}

static int
fn_dist(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sys *sys;
	struct sel *sel;
	vec_t pa, pb, dr;
	double rab, val;
	int idx;

	view = state_get_view(state);

	if (tokq_count(args) < 1) {
		error_set("specify distance");
		return (0);
	}

	val = tok_double(tokq_tok(args, 0));
	sel = make_sel(args, 1, tokq_count(args), view_get_sel(view));

	if (sel_get_count(sel) < 2) {
		sel_free(sel);
		error_set("specify at least 2 atoms");
		return (0);
	}

	view_snapshot(view);
	sys = view_get_sys(view);

	sel_iter_start(sel);
	sel_iter_next(sel, &idx);
	sel_remove(sel, idx);

	pa = sys_get_atom_xyz(sys, idx);
	pb = sys_get_sel_center(sys, sel);

	if ((rab = vec_dist(&pa, &pb)) < 1.0e-8)
		return (1);

	dr.x = (pb.x - pa.x) * (val - rab) / rab;
	dr.y = (pb.y - pa.y) * (val - rab) / rab;
	dr.z = (pb.z - pa.z) * (val - rab) / rab;

	sel_iter_start(sel);

	while (sel_iter_next(sel, &idx)) {
		pb = sys_get_atom_xyz(sys, idx);
		pb = vec_add(&pb, &dr);
		sys_set_atom_xyz(sys, idx, pb);
	}

	sel_free(sel);
	return (1);
}

static int
fn_plus_dist(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sys *sys;
	struct sel *sel;
	vec_t pa, pb, dr;
	double rab, val;
	int idx;

	view = state_get_view(state);

	if (tokq_count(args) < 1) {
		error_set("specify distance");
		return (0);
	}

	val = tok_double(tokq_tok(args, 0));
	sel = make_sel(args, 1, tokq_count(args), view_get_sel(view));

	if (sel_get_count(sel) < 2) {
		sel_free(sel);
		error_set("specify at least 2 atoms");
		return (0);
	}

	view_snapshot(view);
	sys = view_get_sys(view);

	sel_iter_start(sel);
	sel_iter_next(sel, &idx);
	sel_remove(sel, idx);

	pa = sys_get_atom_xyz(sys, idx);
	pb = sys_get_sel_center(sys, sel);

	if ((rab = vec_dist(&pa, &pb)) < 1.0e-8)
		return (1);

	dr.x = (pb.x - pa.x) * val / rab;
	dr.y = (pb.y - pa.y) * val / rab;
	dr.z = (pb.z - pa.z) * val / rab;

	sel_iter_start(sel);

	while (sel_iter_next(sel, &idx)) {
		pb = sys_get_atom_xyz(sys, idx);
		pb = vec_add(&pb, &dr);
		sys_set_atom_xyz(sys, idx, pb);
	}

	sel_free(sel);
	return (1);
}

static int
fn_get_dist(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sys *sys;
	struct sel *sel;
	vec_t pa, pb;
	double rab;
	int a;

	view = state_get_view(state);
	sys = view_get_sys(view);
	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));

	if (sel_get_count(sel) < 2) {
		sel_free(sel);
		error_set("specify at least 2 atoms");
		return (0);
	}

	sel_iter_start(sel);
	sel_iter_next(sel, &a);
	sel_remove(sel, a);

	pa = sys_get_atom_xyz(sys, a);
	pb = sys_get_sel_center(sys, sel);

	rab = vec_dist(&pa, &pb);
	error_set("dist %d-sel: %.3lf", a + 1, rab);

	sel_free(sel);
	return (1);
}

static int
fn_view_fit(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sel *sel;

	view = state_get_view(state);
	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));

	view_fit_sel(view, sel);
	sel_free(sel);

	return (1);
}

static int
fn_set_frame(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sys *sys;
	int n;

	view = state_get_view(state);
	sys = view_get_sys(view);

	if (tokq_count(args) < 1)
		return (0);

	if ((n = tok_int(tokq_tok(args, 0))) == 0)
		return (0);

	n = n < 0 ? sys_get_frame_count(sys) + n : n - 1;
	sys_set_frame(sys, n);

	return (1);
}

static int
fn_add_frame(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sys *sys;
	int n;

	view = state_get_view(state);
	sys = view_get_sys(view);

	if (tokq_count(args) < 1)
		return (0);

	if ((n = tok_int(tokq_tok(args, 0))) == 0)
		return (0);

	n = sys_get_frame(sys) + n;
	sys_set_frame(sys, n);

	return (1);
}

static int
fn_fullscreen(struct tokq *args __unused, struct state *state)
{
	state_toggle_fullscreen(state);

	return (1);
}

static int
fn_get(struct tokq *args, struct state *state __unused)
{
	char buffer[1024];
	const char *name;

	if (tokq_count(args) < 1) {
		error_set("specify setting name");
		return (0);
	}

	name = tok_string(tokq_tok(args, 0));

	if (!settings_has_node(name)) {
		error_set("unknown setting \"%s\"", name);
		return (0);
	}

	settings_printf(buffer, sizeof(buffer), name);
	error_set("%s = %s", name, buffer);

	return (1);
}

static int
fn_hide(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sel *visible;
	struct sel *sel;
	int idx;

	view = state_get_view(state);
	visible = view_get_visible(view);
	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));

	sel_iter_start(sel);

	while (sel_iter_next(sel, &idx))
		sel_remove(visible, idx);

	sel_free(sel);

	return (1);
}

static int
fn_add_hydrogens(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sel *sel;

	view = state_get_view(state);
	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));

	if (sel_get_count(sel) == 0) {
		sel_free(sel);
		return (1);
	}

	view_snapshot(view);
	sys_add_hydrogens(view_get_sys(view), sel);

	sel_free(sel);

	return (1);
}

static int
fn_invert(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sel *visible;
	struct sel *sel;
	int idx;

	view = state_get_view(state);
	visible = view_get_visible(view);
	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));

	sel_iter_start(sel);

	while (sel_iter_next(sel, &idx)) {
		if (sel_selected(view_get_sel(view), idx))
			sel_remove(view_get_sel(view), idx);
		else if (sel_selected(visible, idx))
			sel_add(view_get_sel(view), idx);
	}

	sel_free(sel);

	return (1);
}

static int
fn_view_pos(struct tokq *args, struct state *state)
{
	struct view *view;
	struct camera *camera;
	vec_t xyz;

	if (tokq_count(args) < 1)
		return (0);

	view = state_get_view(state);
	camera = view_get_camera(view);
	xyz = tok_vec(tokq_tok(args, 0));
	camera_move_to(camera, xyz);

	return (1);
}

static int
fn_view_pos_plus(struct tokq *args, struct state *state)
{
	struct view *view;
	struct camera *camera;
	vec_t xyz;

	if (tokq_count(args) < 1)
		return (0);

	view = state_get_view(state);
	camera = view_get_camera(view);
	xyz = tok_vec(tokq_tok(args, 0));
	camera_move(camera, xyz);

	return (1);
}

static int
fn_name(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sys *sys;
	struct sel *sel;
	const char *name;
	int idx;

	view = state_get_view(state);

	if (tokq_count(args) < 1) {
		error_set("specify a name");
		return (0);
	}

	name = tok_string(tokq_tok(args, 0));
	sel = make_sel(args, 1, tokq_count(args), view_get_sel(view));

	view_snapshot(view);
	sys = view_get_sys(view);

	sel_iter_start(sel);

	while (sel_iter_next(sel, &idx))
		sys_set_atom_name(sys, idx, name);

	sel_free(sel);
	return (1);
}

static int
fn_get_name(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sys *sys;
	struct sel *sel;
	const char *name;
	int idx;

	view = state_get_view(state);
	sys = view_get_sys(view);
	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));

	if (sel_get_count(sel) != 1) {
		sel_free(sel);
		error_set("specify 1 atom");
		return (0);
	}

	sel_iter_start(sel);
	sel_iter_next(sel, &idx);

	name = sys_get_atom_name(sys, idx);
	error_set("atom %d: %s", idx + 1, name);

	sel_free(sel);

	return (1);
}

static int
fn_new(struct tokq *args, struct state *state)
{
	struct wnd *wnd;
	const char *path;

	wnd = state_get_wnd(state);
	path = tokq_count(args) > 0 ? tok_string(tokq_tok(args, 0)) : "";

	return (wnd_open(wnd, path));
}

static int
fn_first(struct tokq *args __unused, struct state *state)
{
	struct wnd *wnd;

	wnd = state_get_wnd(state);
	wnd_first(wnd);

	return (1);
}

static int
fn_last(struct tokq *args __unused, struct state *state)
{
	struct wnd *wnd;

	wnd = state_get_wnd(state);
	wnd_last(wnd);

	return (1);
}

static int
fn_prev(struct tokq *args __unused, struct state *state)
{
	struct wnd *wnd;

	wnd = state_get_wnd(state);

	if (!wnd_prev(wnd)) {
		error_set("no previous window");
		return (0);
	}

	return (1);
}

static int
fn_next(struct tokq *args __unused, struct state *state)
{
	struct wnd *wnd;

	wnd = state_get_wnd(state);

	if (!wnd_next(wnd)) {
		error_set("no next window");
		return (0);
	}

	return (1);
}

static int
fn_nop(struct tokq *args __unused, struct state *state __unused)
{
	/* no op */

	return (1);
}

static int
fn_open(struct tokq *args, struct state *state)
{
	struct wnd *wnd;
	int is_empty, is_modified;
	const char *path;

	wnd = state_get_wnd(state);

	if (tokq_count(args) < 1) {
		error_set("specify a file");
		return (0);
	}

	path = tok_string(tokq_tok(args, 0));

	if (!util_file_exists(path)) {
		error_set("unable to find file \"%s\"", path);
		return (0);
	}

	is_empty = view_is_empty(wnd_get_view(wnd));
	is_modified = view_is_modified(wnd_get_view(wnd));

	if (!wnd_open(wnd, path))
		return (0);

	if (is_empty && !is_modified) {
		wnd_prev(wnd);
		wnd_close(wnd);
	}

	return (1);
}

static int
fn_paste(struct tokq *args, struct state *state)
{
	struct view *view;
	struct yank *yank;
	int count, reg;

	view = state_get_view(state);
	yank = state_get_yank(state);

	if (tokq_count(args) > 0) {
		if ((reg = parse_register(tokq_tok(args, 0))) == -1)
			return (0);

		yank_set_register(yank, reg);
	}

	count = sys_get_atom_count(view_get_sys(view));

	view_snapshot(view);
	yank_paste(yank, view_get_sys(view));

	sel_clear(view_get_sel(view));

	while (count < sys_get_atom_count(view_get_sys(view)))
		sel_add(view_get_sel(view), count++);

	return (1);
}

static int
fn_get_path(struct tokq *args __unused, struct state *state)
{
	struct view *view;
	const char *path;

	view = state_get_view(state);
	path = view_get_path(view);

	if (path[0] == '\0')
		error_set("no file name");
	else
		error_set("\"%s\"", path);

	return (1);
}

static int
fn_play(struct tokq *args, struct state *state)
{
	struct rec *rec;
	int reg;

	rec = state_get_rec(state);

	if (rec_is_playing(rec))
		return (1);

	if (rec_is_recording(rec)) {
		error_set("cannot play during recording");
		return (0);
	}

	if (tokq_count(args) > 0) {
		if ((reg = parse_register(tokq_tok(args, 0))) == -1)
			return (0);

		rec_set_register(rec, reg);
	}

	return (rec_play(rec, state));
}

static int
fn_pos(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sys *sys;
	struct sel *sel;
	vec_t dr, xyz;
	int idx;

	view = state_get_view(state);

	if (tokq_count(args) < 1) {
		error_set("specify a vector [x y z]");
		return (0);
	}

	dr = tok_vec(tokq_tok(args, 0));
	sel = make_sel(args, 1, tokq_count(args), view_get_sel(view));

	if (sel_get_count(sel) == 0) {
		sel_free(sel);
		return (1);
	}

	view_snapshot(view);
	sys = view_get_sys(view);
	xyz = sys_get_sel_center(sys, sel);
	dr = vec_sub(&dr, &xyz);

	sel_iter_start(sel);

	while (sel_iter_next(sel, &idx)) {
		xyz = sys_get_atom_xyz(sys, idx);
		xyz = vec_add(&xyz, &dr);
		sys_set_atom_xyz(sys, idx, xyz);
	}

	sel_free(sel);
	return (1);
}

static int
fn_pos_plus(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sys *sys;
	struct sel *sel;
	mat_t rotmat;
	vec_t dr, xyz;
	int idx;

	view = state_get_view(state);

	if (tokq_count(args) < 1) {
		error_set("specify a vector [x y z]");
		return (0);
	}

	dr = tok_vec(tokq_tok(args, 0));
	sel = make_sel(args, 1, tokq_count(args), view_get_sel(view));

	if (sel_get_count(sel) == 0) {
		sel_free(sel);
		return (1);
	}

	view_snapshot(view);
	sys = view_get_sys(view);
	rotmat = camera_get_rotation(view_get_camera(view));
	rotmat = mat_transpose(&rotmat);
	dr = mat_vec(&rotmat, &dr);

	sel_iter_start(sel);

	while (sel_iter_next(sel, &idx)) {
		xyz = sys_get_atom_xyz(sys, idx);
		xyz = vec_add(&xyz, &dr);
		sys_set_atom_xyz(sys, idx, xyz);
	}

	sel_free(sel);
	return (1);
}

static int
fn_get_pos(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sel *sel;
	char *buf;
	vec_t xyz;

	view = state_get_view(state);
	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));
	xyz = sys_get_sel_center(view_get_sys(view), sel);
	xasprintf(&buf, "pos: %.3lf %.3lf %.3lf", xyz.x, xyz.y, xyz.z);
	error_set("%s", buf);
	free(buf);
	sel_free(sel);

	return (1);
}

static int
fn_quit(struct tokq *args __unused, struct state *state)
{
	state_quit(state, 0);

	return (1);
}

static int
fn_force_quit(struct tokq *args __unused, struct state *state)
{
	state_quit(state, 1);

	return (1);
}

static int
fn_read(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sys *sys;
	const char *path;

	if (tokq_count(args) < 1) {
		error_set("specify file path");
		return (0);
	}

	view = state_get_view(state);
	path = tok_string(tokq_tok(args, 0));

	view_snapshot(view);
	sys = view_get_sys(view);

	if (!sys_read(sys, path)) {
		view_undo(view);
		return (0);
	}

	return (1);
}

static int
fn_rec(struct tokq *args, struct state *state)
{
	struct rec *rec;
	int reg;

	rec = state_get_rec(state);

	if (rec_is_playing(rec))
		return (1);

	if (rec_is_recording(rec)) {
		rec_stop(rec);
		return (1);
	}

	if (tokq_count(args) > 0) {
		if ((reg = parse_register(tokq_tok(args, 0))) == -1)
			return (0);

		rec_set_register(rec, reg);
	}

	rec_start(rec);

	return (1);
}

static int
fn_redo(struct tokq *args __unused, struct state *state)
{
	struct view *view;

	view = state_get_view(state);

	if (!view_redo(view)) {
		error_set("already at newest change");
		return (0);
	}

	return (1);
}

static int
fn_reload(struct tokq *args __unused, struct state *state)
{
	struct wnd *wnd;
	const char *path;

	wnd = state_get_wnd(state);

	if (wnd_is_modified(wnd)) {
		error_set("save changes or add ! to override");
		return (0);
	}

	path = view_get_path(wnd_get_view(wnd));

	if (path[0] == '\0') {
		error_set("no file name");
		return (0);
	}

	if (!wnd_open(wnd, path))
		return (0);

	wnd_prev(wnd);
	wnd_close(wnd);

	return (1);
}

static int
fn_force_reload(struct tokq *args __unused, struct state *state)
{
	struct wnd *wnd;
	const char *path;

	wnd = state_get_wnd(state);
	path = view_get_path(wnd_get_view(wnd));

	if (path[0] == '\0') {
		error_set("no file name");
		return (0);
	}

	if (!wnd_open(wnd, path))
		return (0);

	wnd_prev(wnd);
	wnd_close(wnd);

	return (1);
}

static int
fn_repeat(struct tokq *args, struct state *state)
{
	struct cmdq *cmdq;
	char *str;
	int i, n, ret;

	if (tokq_count(args) < 2)
		return (0);

	n = tok_int(tokq_tok(args, 0));
	str = tokq_strcat(args, 1, tokq_count(args) - 1);

	if ((cmdq = cmdq_from_string(str)) == NULL) {
		free(str);
		return (0);
	}

	ret = 1;

	for (i = 0; i < n; i++)
		if (!(ret = cmdq_exec(cmdq, state)))
			break;

	cmdq_free(cmdq);
	free(str);

	return (ret);
}

static int
fn_autobond(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sys *sys;
	struct sel *sel;

	view = state_get_view(state);
	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));

	if (sel_get_count(sel) == 0) {
		sel_free(sel);
		return (1);
	}

	view_snapshot(view);
	sys = view_get_sys(view);

	sys_reset_bonds(sys, sel);
	sel_free(sel);

	return (1);
}

static int
fn_view_reset(struct tokq *args __unused, struct state *state)
{
	view_reset(state_get_view(state));

	return (1);
}

static int
fn_ring(struct tokq *args, struct state *state)
{
	struct view *view;
	struct graph *graph;
	struct sys *sys;
	struct sel *sel;
	mat_t rotmat;
	vec_t center, xyz;
	double angle, radius;
	int i, j;

	view = state_get_view(state);
	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));

	if (sel_get_count(sel) < 2) {
		sel_free(sel);
		error_set("select at least two atoms");
		return (0);
	}

	view_snapshot(view);
	sys = view_get_sys(view);
	graph = view_get_graph(view);

	rotmat = camera_get_rotation(view_get_camera(view));
	rotmat = mat_transpose(&rotmat);
	center = sys_get_sel_center(sys, sel);
	radius = 1.54 / (2.0 * sin(PI / sel_get_count(sel)));
	angle = PI / sel_get_count(sel);

	for (sel_iter_start(sel), j = -1; sel_iter_next(sel, &i); j = i) {
		xyz.x = radius * cos(angle);
		xyz.y = radius * sin(angle);
		xyz.z = 0.0;

		xyz = mat_vec(&rotmat, &xyz);
		xyz = vec_add(&xyz, &center);

		sys_set_atom_xyz(sys, i, xyz);
		graph_remove_vertex_edges(graph, i);

		if (j > -1)
			graph_edge_create(graph, i, j, 1);

		angle += 2.0 * PI / sel_get_count(sel);
	}

	/* bond last to first */
	sel_iter_start(sel);
	sel_iter_next(sel, &i);
	graph_edge_create(graph, j, i, 1);

	sel_free(sel);

	return (1);
}

static int
fn_rotate(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sys *sys;
	struct sel *sel;
	mat_t matrix, rotmat;
	vec_t abc, pos, xyz;
	int idx;

	view = state_get_view(state);

	if (tokq_count(args) < 1) {
		error_set("specify rotation angles [a b c]");
		return (0);
	}

	abc = tok_vec(tokq_tok(args, 0));
	sel = make_sel(args, 1, tokq_count(args), view_get_sel(view));

	if (sel_get_count(sel) == 0) {
		sel_free(sel);
		return (1);
	}

	view_snapshot(view);
	sys = view_get_sys(view);

	vec_scale(&abc, PI / 180.0);
	xyz = sys_get_sel_center(sys, sel);

	matrix = camera_get_rotation(view_get_camera(view));
	rotmat = mat_transpose(&matrix);
	matrix = mat_rotation_z(abc.z);
	rotmat = mat_mat(&rotmat, &matrix);
	matrix = mat_rotation_y(abc.y);
	rotmat = mat_mat(&rotmat, &matrix);
	matrix = mat_rotation_x(abc.x);
	rotmat = mat_mat(&rotmat, &matrix);
	matrix = camera_get_rotation(view_get_camera(view));
	rotmat = mat_mat(&rotmat, &matrix);

	sel_iter_start(sel);

	while (sel_iter_next(sel, &idx)) {
		pos = sys_get_atom_xyz(sys, idx);
		pos = vec_sub(&pos, &xyz);
		pos = mat_vec(&rotmat, &pos);
		pos = vec_add(&pos, &xyz);
		sys_set_atom_xyz(sys, idx, pos);
	}

	sel_free(sel);
	return (1);
}

static int
fn_view_rotate(struct tokq *args, struct state *state)
{
	struct view *view;
	struct camera *camera;
	vec_t xyz;

	if (tokq_count(args) < 1)
		return (0);

	view = state_get_view(state);
	camera = view_get_camera(view);
	xyz = tok_vec(tokq_tok(args, 0));

	vec_scale(&xyz, PI / 180.0);
	camera_rotate(camera, xyz);

	return (1);
}

static int
fn_run(struct tokq *args, struct state *state __unused)
{
	int ret = system(tokq_strcat(args, 0, tokq_count(args)));

	error_set("finished with exit code '%d'", ret);

	return (1);
}

static int
fn_save(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sys *sys;
	const char *path;

	view = state_get_view(state);
	sys = view_get_sys(view);

	if (tokq_count(args) == 0) {
		path = view_get_path(view);

		if (path[0] == '\0') {
			error_set("no file name");
			return (0);
		}

		if (!string_has_suffix(path, ".xyz")) {
			error_set("add ! to overwrite and save in xyz format");
			return (0);
		}
	} else
		path = tok_string(tokq_tok(args, 0));

	if (!sys_save_to_file(sys, path))
		return (0);

	view_set_path(view, path);
	error_set("saved to \"%s\"", view_get_path(view));

	return (1);
}

static int
fn_force_save(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sys *sys;
	const char *path;

	view = state_get_view(state);
	sys = view_get_sys(view);

	if (tokq_count(args) == 0) {
		path = view_get_path(view);

		if (path[0] == '\0') {
			error_set("no file name");
			return (0);
		}
	} else
		path = tok_string(tokq_tok(args, 0));

	if (!sys_save_to_file(sys, path))
		return (0);

	view_set_path(view, path);
	error_set("saved to \"%s\"", view_get_path(view));

	return (1);
}

static int
fn_png(struct tokq *args, struct state *state)
{
	char path[256];
	int i;

	if (tokq_count(args) == 0) {
		for (i = 0; ; i++) {
			snprintf(path, sizeof(path), "%05d.png", i);

			if (!util_file_exists(path))
				break;
		}
	} else {
		snprintf(path, sizeof(path), "%s",
		    tok_string(tokq_tok(args, 0)));
	}

	state_save_png(state, path);
	error_set("saved to \"%s\"", path);

	return (1);
}

static int
fn_select(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sel *visible;
	struct sel *sel;
	int idx;

	view = state_get_view(state);
	visible = view_get_visible(view);
	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));

	sel_iter_start(sel);

	while (sel_iter_next(sel, &idx))
		if (sel_selected(visible, idx))
			sel_add(view_get_sel(view), idx);

	sel_free(sel);
	return (1);
}

static int
fn_select_bonded(struct tokq *args, struct state *state)
{
	struct view *view;
	struct graph *graph;
	struct graphedge *edge;
	struct sel *visible;
	struct sel *sel;
	int i;

	view = state_get_view(state);
	graph = view_get_graph(view);
	visible = view_get_visible(view);
	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));

	sel_iter_start(sel);

	while (sel_iter_next(sel, &i)) {
		for (edge = graph_edges(graph, i); edge;
		    edge = graph_edge_next(edge))
			if (sel_selected(visible, graph_edge_j(edge)))
				sel_add(view_get_sel(view), graph_edge_j(edge));
	}

	sel_free(sel);
	return (1);
}

static int
fn_select_box(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sys *sys;
	struct sel *visible;
	vec_t p1, p2, xyz;
	int idx;

	if (tokq_count(args) < 1) {
		error_set("specify box dimensions");
		return (0);
	}

	if (tokq_count(args) < 2) {
		p1 = vec_zero();
		p2 = tok_vec(tokq_tok(args, 0));
	} else {
		p1 = tok_vec(tokq_tok(args, 0));
		p2 = tok_vec(tokq_tok(args, 1));
	}

	view = state_get_view(state);
	sys = view_get_sys(view);
	visible = view_get_visible(view);

	sel_iter_start(visible);

	while (sel_iter_next(visible, &idx)) {
		xyz = sys_get_atom_xyz(sys, idx);

		if (xyz.x >= p1.x && xyz.x <= p2.x &&
		    xyz.y >= p1.y && xyz.y <= p2.y &&
		    xyz.z >= p1.z && xyz.z <= p2.z)
			sel_add(view_get_sel(view), idx);
	}

	return (1);
}

static int
fn_select_molecule(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sys *sys;
	struct graph *graph;
	struct sel *visible;
	struct sel *selected;
	struct sel *connected;
	int idx;

	view = state_get_view(state);
	sys = view_get_sys(view);
	graph = view_get_graph(view);
	visible = view_get_visible(view);
	selected = make_sel(args, 0, tokq_count(args), view_get_sel(view));
	connected = sel_create(sys_get_atom_count(sys));

	sel_iter_start(selected);

	while (sel_iter_next(selected, &idx))
		select_connected(graph, idx, connected);

	sel_iter_start(connected);

	while (sel_iter_next(connected, &idx))
		if (sel_selected(visible, idx))
			sel_add(view_get_sel(view), idx);

	sel_free(selected);
	sel_free(connected);

	return (1);
}

static int
fn_select_name(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sys *sys;
	struct sel *visible;
	const char *name;
	int i, k;

	view = state_get_view(state);
	sys = view_get_sys(view);
	visible = view_get_visible(view);

	for (k = 0; k < tokq_count(args); k++) {
		name = tok_string(tokq_tok(args, k));
		sel_iter_start(visible);

		while (sel_iter_next(visible, &i))
			if (strcasecmp(sys_get_atom_name(sys, i), name) == 0)
				sel_add(view_get_sel(view), i);
	}

	return (1);
}

static int
fn_select_next(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sel *visible;
	struct sel *sel;
	int count, i;

	view = state_get_view(state);
	visible = view_get_visible(view);
	sel = view_get_sel(view);
	count = tokq_count(args) > 0 ? tok_int(tokq_tok(args, 0)) : 1;

	if (count < 0) {
		for (i = sel_get_size(sel) - 1; i >= 0 && count < 0; i--) {
			if (sel_selected(visible, i) && !sel_selected(sel, i)) {
				sel_add(sel, i);
				count++;
			}
		}
	} else {
		for (i = 0; i < sel_get_size(sel) && count > 0; i++) {
			if (sel_selected(visible, i) && !sel_selected(sel, i)) {
				sel_add(sel, i);
				count--;
			}
		}
	}

	return (1);
}

static int
fn_select_water(struct tokq *args __unused, struct state *state)
{
	struct view *view;
	struct sys *sys;
	struct graph *graph;
	struct sel *visible;
	int i, j, k;

	view = state_get_view(state);
	sys = view_get_sys(view);
	graph = view_get_graph(view);
	visible = view_get_visible(view);

	sel_iter_start(visible);

	while (sel_iter_next(visible, &i)) {
		if (graph_get_edge_count(graph, i) != 2)
			continue;

		j = graph_edge_j(graph_edges(graph, i));
		k = graph_edge_j(graph_edge_next(graph_edges(graph, i)));

		if (!sel_selected(visible, j) ||
		    !sel_selected(visible, k))
			continue;

		if (strcasecmp(sys_get_atom_name(sys, i), "O") != 0 ||
		    strcasecmp(sys_get_atom_name(sys, j), "H") != 0 ||
		    strcasecmp(sys_get_atom_name(sys, k), "H") != 0)
			continue;

		sel_add(view_get_sel(view), i);
		sel_add(view_get_sel(view), j);
		sel_add(view_get_sel(view), k);
	}

	return (1);
}

static int
fn_select_within(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sys *sys;
	struct sel *visible;
	struct sel *sel;
	struct spi *spi;
	struct pair pair;
	double radius;
	int idx, n_pairs;

	if (tokq_count(args) < 1)
		return (0);

	if ((radius = tok_double(tokq_tok(args, 0))) <= 0.0)
		return (0);

	view = state_get_view(state);
	sys = view_get_sys(view);
	visible = view_get_visible(view);
	spi = spi_create();

	for (idx = 0; idx < sys_get_atom_count(sys); idx++)
		spi_add_point(spi, sys_get_atom_xyz(sys, idx));

	spi_compute(spi, radius);
	n_pairs = spi_get_pair_count(spi);

	sel = make_sel(args, 1, tokq_count(args), view_get_sel(view));

	sel_iter_start(sel);

	while (sel_iter_next(sel, &idx))
		if (sel_selected(visible, idx))
			sel_add(view_get_sel(view), idx);

	for (idx = 0; idx < n_pairs; idx++) {
		pair = spi_get_pair(spi, idx);

		if (!sel_selected(visible, pair.i) ||
		    !sel_selected(visible, pair.j))
			continue;

		if (sel_selected(sel, pair.i) || sel_selected(sel, pair.j)) {
			sel_add(view_get_sel(view), pair.i);
			sel_add(view_get_sel(view), pair.j);
		}
	}

	sel_free(sel);
	spi_free(spi);
	return (1);
}

static int
fn_set(struct tokq *args, struct state *state __unused)
{
	const char *name, *value;

	if (tokq_count(args) < 2) {
		error_set("specify setting name and value");
		return (0);
	}

	name = tok_string(tokq_tok(args, 0));
	value = tok_string(tokq_tok(args, 1));

	return (settings_set(name, value));
}

static int
fn_show(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sel *visible;
	struct sel *sel;
	int idx;

	view = state_get_view(state);
	visible = view_get_visible(view);
	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));

	sel_iter_start(sel);

	while (sel_iter_next(sel, &idx))
		sel_add(visible, idx);

	sel_free(sel);
	return (1);
}

static int
fn_group(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sys *sys;
	struct sel *sel;
	int i, j;

	view = state_get_view(state);
	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));

	if (sel_get_count(sel) == 0) {
		sel_free(sel);
		return (1);
	}

	view_snapshot(view);
	sys = view_get_sys(view);

	for (i = sys_get_atom_count(sys) - 1; i >= 0; i--) {
		if (!sel_selected(sel, i))
			continue;

		for (j = i + 1; j < sys_get_atom_count(sys); j++) {
			if (sel_selected(sel, j))
				break;

			sys_swap_atoms(sys, j - 1, j);
			sel_swap(sel, j - 1, j);
		}
	}

	sel_free(sel);
	return (1);
}

static int
fn_source(struct tokq *args, struct state *state)
{
	const char *path;

	if (tokq_count(args) < 1) {
		error_set("specify a file");
		return (0);
	}

	path = tok_string(tokq_tok(args, 0));

	return (state_source(state, path));
}

static int
fn_edit(struct tokq *args __unused, struct state *state)
{
	if (rec_is_playing(state_get_rec(state)))
		return (1);

	state_start_edit(state);

	return (1);
}

static int
fn_swap(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sys *sys;
	struct sel *sel;
	int i, j;

	view = state_get_view(state);
	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));

	if (sel_get_count(sel) != 2) {
		sel_free(sel);
		error_set("specify 2 indices");
		return (0);
	}

	view_snapshot(view);
	sys = view_get_sys(view);

	sel_iter_start(sel);
	sel_iter_next(sel, &i);
	sel_iter_next(sel, &j);

	sys_swap_atoms(sys, i, j);

	sel_free(sel);

	return (1);
}

static int
fn_toggle(struct tokq *args, struct state *state __unused)
{
	const char *name;
	int value;

	if (tokq_count(args) == 0)
		return (0);

	name = tok_string(tokq_tok(args, 0));

	if (!settings_has_bool(name))
		return (0);

	value = settings_get_bool(name);
	settings_set_bool(name, !value);

	return (1);
}

static int
fn_get_tors(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sys *sys;
	struct sel *sel;
	vec_t pa, pb, pc, pd;
	int a, b, c;
	double value;

	view = state_get_view(state);
	sys = view_get_sys(view);
	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));

	if (sel_get_count(sel) < 4) {
		sel_free(sel);
		error_set("specify at least 4 atoms");
		return (0);
	}

	sel_iter_start(sel);
	sel_iter_next(sel, &a);
	sel_iter_next(sel, &b);
	sel_iter_next(sel, &c);
	sel_remove(sel, a);
	sel_remove(sel, b);
	sel_remove(sel, c);

	pa = sys_get_atom_xyz(sys, a);
	pb = sys_get_atom_xyz(sys, b);
	pc = sys_get_atom_xyz(sys, c);
	pd = sys_get_sel_center(sys, sel);

	value = vec_torsion(&pa, &pb, &pc, &pd) * 180 / PI;
	error_set("torsion %d-%d-%d-sel: %.3lf", a + 1, b + 1, c + 1, value);

	sel_free(sel);
	return (1);
}

static int
fn_undo(struct tokq *args __unused, struct state *state)
{
	struct view *view;

	view = state_get_view(state);

	if (!view_undo(view)) {
		error_set("already at oldest change");
		return (0);
	}

	return (1);
}

static int
fn_unselect(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sel *sel;
	int idx;

	view = state_get_view(state);
	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));

	sel_iter_start(sel);

	while (sel_iter_next(sel, &idx))
		sel_remove(view_get_sel(view), idx);

	sel_free(sel);
	return (1);
}

static int
fn_unselect_next(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sel *visible;
	struct sel *sel;
	int count, i;

	view = state_get_view(state);
	visible = view_get_visible(view);
	sel = view_get_sel(view);
	count = tokq_count(args) > 0 ? tok_int(tokq_tok(args, 0)) : 1;

	if (count < 0) {
		for (i = sel_get_size(sel) - 1; i >= 0 && count < 0; i--) {
			if (sel_selected(visible, i) && sel_selected(sel, i)) {
				sel_remove(sel, i);
				count++;
			}
		}
	} else {
		for (i = 0; i < sel_get_size(sel) && count > 0; i++) {
			if (sel_selected(visible, i) && sel_selected(sel, i)) {
				sel_remove(sel, i);
				count--;
			}
		}
	}

	return (1);
}

static int
fn_view_zoom(struct tokq *args, struct state *state)
{
	struct view *view;
	struct camera *camera;
	double factor, radius;

	view = state_get_view(state);
	camera = view_get_camera(view);

	if (tokq_count(args) < 1)
		return (0);

	if ((factor = tok_double(tokq_tok(args, 0))) <= 0.0) {
		error_set("zoom factor must be positive");
		return (0);
	}

	radius = camera_get_radius(camera);
	camera_set_radius(camera, radius / factor);

	return (1);
}

static int
fn_wrap(struct tokq *args, struct state *state)
{
	struct view *view;
	struct sys *sys;
	struct graph *graph;
	struct sel *current, *wrapped;
	vec_t p1, p2, cell, dr, xyz;
	int i, j;

	view = state_get_view(state);

	if (tokq_count(args) < 1) {
		error_set("specify cell dimensions");
		return (0);
	}

	if (tokq_count(args) < 2) {
		p1 = vec_zero();
		p2 = tok_vec(tokq_tok(args, 0));
	} else {
		p1 = tok_vec(tokq_tok(args, 0));
		p2 = tok_vec(tokq_tok(args, 1));
	}

	cell = vec_sub(&p2, &p1);

	if (cell.x <= 0 || cell.y <= 0 || cell.z <= 0) {
		error_set("cell dimensions must be positive");
		return (0);
	}

	view_snapshot(view);
	sys = view_get_sys(view);
	graph = view_get_graph(view);

	current = sel_create(sys_get_atom_count(sys));
	wrapped = sel_create(sys_get_atom_count(sys));

	for (i = 0; i < sys_get_atom_count(sys); i++) {
		if (sel_selected(wrapped, i))
			continue;

		sel_clear(current);
		select_connected(graph, i, current);

		dr = sys_get_sel_center(sys, current);

		if (sel_get_count(current) > 0) {
			dr.x = p1.x + cell.x * floor(dr.x / cell.x);
			dr.y = p1.y + cell.y * floor(dr.y / cell.y);
			dr.z = p1.z + cell.z * floor(dr.z / cell.z);
		}

		sel_iter_start(current);

		while (sel_iter_next(current, &j)) {
			xyz = sys_get_atom_xyz(sys, j);
			xyz = vec_sub(&xyz, &dr);
			sys_set_atom_xyz(sys, j, xyz);
			sel_add(wrapped, j);
		}
	}

	sel_free(current);
	sel_free(wrapped);

	return (1);
}

typedef int (*exec_fn_t)(struct tokq *, struct state *);

/* this list MUST remain sorted */
static const struct node {
	const char *name;
	exec_fn_t fn;
} execlist[] = {
	{ "about", fn_about },
	{ "add.hydrogens", fn_add_hydrogens },
	{ "all", fn_all },
	{ "angle?", fn_get_angle },
	{ "atom", fn_atom },
	{ "autobond", fn_autobond },
	{ "bind", fn_bind },
	{ "bind?", fn_get_bind },
	{ "bond", fn_bond },
	{ "chain", fn_chain },
	{ "clear", fn_clear },
	{ "close", fn_close },
	{ "close!", fn_force_close },
	{ "coord", fn_coord },
	{ "copy", fn_copy },
	{ "count", fn_count },
	{ "delete", fn_delete },
	{ "dist", fn_dist },
	{ "dist+", fn_plus_dist },
	{ "dist?", fn_get_dist },
	{ "edit", fn_edit },
	{ "first", fn_first },
	{ "frame", fn_set_frame },
	{ "frame+", fn_add_frame },
	{ "fullscreen", fn_fullscreen },
	{ "get", fn_get },
	{ "group", fn_group },
	{ "hide", fn_hide },
	{ "invert", fn_invert },
	{ "last", fn_last },
	{ "name", fn_name },
	{ "name?", fn_get_name },
	{ "new", fn_new },
	{ "next", fn_next },
	{ "nop", fn_nop },
	{ "open", fn_open },
	{ "paste", fn_paste },
	{ "path?", fn_get_path },
	{ "play", fn_play },
	{ "png", fn_png },
	{ "pos", fn_pos },
	{ "pos+", fn_pos_plus },
	{ "pos?", fn_get_pos },
	{ "prev", fn_prev },
	{ "q", fn_quit },
	{ "q!", fn_force_quit },
	{ "read", fn_read },
	{ "rec", fn_rec },
	{ "redo", fn_redo },
	{ "reload", fn_reload },
	{ "reload!", fn_force_reload },
	{ "remove.bond", fn_remove_bond },
	{ "repeat", fn_repeat },
	{ "ring", fn_ring },
	{ "rotate", fn_rotate },
	{ "run", fn_run },
	{ "select", fn_select },
	{ "select.bonded", fn_select_bonded },
	{ "select.box", fn_select_box },
	{ "select.molecule", fn_select_molecule },
	{ "select.name", fn_select_name },
	{ "select.next", fn_select_next },
	{ "select.water", fn_select_water },
	{ "select.within", fn_select_within },
	{ "set", fn_set },
	{ "show", fn_show },
	{ "source", fn_source },
	{ "swap", fn_swap },
	{ "toggle", fn_toggle },
	{ "tors?", fn_get_tors },
	{ "undo", fn_undo },
	{ "unselect", fn_unselect },
	{ "unselect.next", fn_unselect_next },
	{ "view.center", fn_view_center },
	{ "view.fit", fn_view_fit },
	{ "view.pos", fn_view_pos },
	{ "view.pos+", fn_view_pos_plus },
	{ "view.reset", fn_view_reset },
	{ "view.rotate", fn_view_rotate },
	{ "view.zoom", fn_view_zoom },
	{ "w", fn_save },
	{ "w!", fn_force_save },
	{ "wrap", fn_wrap },
};
static const size_t nexeclist = sizeof execlist / sizeof *execlist;

static int
compare(const void *a, const void *b)
{
	const struct node *aa = (const struct node *)a;
	const struct node *bb = (const struct node *)b;

	return (strcasecmp(aa->name, bb->name));
}

static const struct node *
exec_find(const char *name)
{
	struct node node;

	node.name = name;

	return (bsearch(&node, execlist, nexeclist,
	    sizeof(struct node), compare));
}

int
exec_valid(const char *name)
{
	return (exec_find(name) != NULL);
}

int
exec_run(const char *name, struct tokq *args, struct state *state)
{
	const struct node *node = exec_find(name);

	assert(node != NULL);

	return (node->fn(args, state));
}
