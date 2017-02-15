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
	edge = graph_get_edges(graph, idx);
	while (edge) {
		select_connected(graph, graph_edge_j(edge), sel);
		edge = graph_edge_next(edge);
	}
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

static vec_t
parse_vec(struct tokq *args, int start)
{
	vec_t xyz = { 0, 0, 0 };
	char *s;

	s = tokq_strcat(args, start, 3);
	sscanf(s, "%lf %lf %lf", &xyz.x, &xyz.y, &xyz.z);
	free(s);

	return (xyz);
}

static int
get_register(struct state *state)
{
	int reg = state_get_number(state);

	if (reg < 0 || reg >= NUM_REGISTERS)
		reg = 0;
	return (reg);
}

static int
fn_about(struct tokq *args __unused, struct state *state __unused)
{
	error_set("vimol %s (c) 2013-2017 Ilya Kaliman", VIMOL_VERSION);

	return (1);
}

static int
fn_add_hydrogens(struct tokq *args, struct state *state)
{
	struct view *view = state_get_view(state);
	struct sel *sel;

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
fn_atom(struct tokq *args, struct state *state)
{
	struct view *view = state_get_view(state);
	const char *name = "C";
	vec_t xyz;

	if (tokq_count(args) > 0)
		name = tok_string(tokq_tok(args, 0));
	xyz = parse_vec(args, 1);
	view_snapshot(view);
	sys_add_atom(view_get_sys(view), name, xyz);
	return (1);
}

static int
fn_bind(struct tokq *args, struct state *state)
{
	struct bind *bind = state_get_bind(state);
	const char *name, *value;
	char *cmd;

	if (tokq_count(args) == 0)
		return (1);
	name = tok_string(tokq_tok(args, 0));
	if (tokq_count(args) == 1) {
		if ((value = bind_get(bind, name)) != NULL)
			error_set("\"%s\" is bound to \"%s\"", name, value);
		else
			error_set("\"%s\" is not bound", name);
	} else {
		cmd = tokq_strcat(args, 1, tokq_count(args) - 1);
		if (!cmdq_validate_string(cmd)) {
			error_set("\"%s\": invalid command", cmd);
			free(cmd);
			return (0);
		}
		bind_set(bind, name, cmd);
		free(cmd);
	}
	return (1);
}

static int
fn_bond(struct tokq *args, struct state *state)
{
	struct view *view = state_get_view(state);
	struct graph *graph;
	struct sel *sel;
	struct graphedge *edge;
	int a, b, type;

	graph = view_get_graph(view);
	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));
	if (sel_get_count(sel) != 2) {
		error_set("select 2 atoms");
		sel_free(sel);
		return (0);
	}
	sel_iter_start(sel);
	sel_iter_next(sel, &a);
	sel_iter_next(sel, &b);
	if ((edge = graph_edge_find(graph, a, b)) == NULL)
		graph_edge_create(graph, a, b, 1);
	else {
		if ((type = graph_edge_get_type(edge)) == 3)
			graph_edge_remove(graph, a, b);
		else
			graph_edge_set_type(edge, type+1);
	}
	sel_free(sel);
	return (1);
}

static int
fn_chain(struct tokq *args, struct state *state)
{
	struct view *view = state_get_view(state);
	struct graph *graph;
	struct sys *sys;
	mat_t rotmat;
	vec_t u, v, xyz;
	int i, natoms, nchain = 4;

	if (tokq_count(args) > 0) {
		nchain = tok_int(tokq_tok(args, 0));
		if (nchain < 1) {
			error_set("specify a positive number");
			return (0);
		}
	}
	view_snapshot(view);
	sys = view_get_sys(view);
	graph = view_get_graph(view);
	rotmat = camera_get_rotation(view_get_camera(view));
	rotmat = mat_transpose(&rotmat);
	u = vec_new(1.336, 0.0, 0.0);
	v = vec_new(0.0, 0.766, 0.0);
	u = mat_vec(&rotmat, &u);
	v = mat_vec(&rotmat, &v);
	xyz.x = -(u.x * (nchain - 1) + v.x) / 2;
	xyz.y = -(u.y * (nchain - 1) + v.y) / 2;
	xyz.z = -(u.z * (nchain - 1) + v.z) / 2;
	for (i = 0; i < nchain; i++) {
		sys_add_atom(sys, "C", xyz);
		xyz = vec_add(&xyz, &u);
		xyz = vec_add(&xyz, &v);
		vec_scale(&v, -1.0);
		if (i > 0) {
			natoms = sys_get_atom_count(sys);
			graph_edge_create(graph, natoms-2, natoms-1, 1);
		}
	}
	return (1);
}

static int
fn_close(struct tokq *args __unused, struct state *state)
{
	return (wnd_close(state_get_wnd(state), 0));
}

static int
fn_force_close(struct tokq *args __unused, struct state *state)
{
	return (wnd_close(state_get_wnd(state), 1));
}

static int
fn_copy_selection(struct tokq *args, struct state *state)
{
	struct view *view = state_get_view(state);
	struct sys *sys;
	struct yank *yank;
	struct sel *sel;
	int reg;

	reg = get_register(state);
	yank = state_get_yank(state);
	sys = view_get_sys(view);
	sel = make_sel(args, 1, tokq_count(args), view_get_sel(view));
	if (sel_get_count(sel) == 0) {
		sel_free(sel);
		return (1);
	}
	yank_copy(yank, sys, sel, reg);
	error_set("copied %d atoms", sel_get_count(sel));
	sel_free(sel);
	return (1);
}

static int
fn_delete_selection(struct tokq *args, struct state *state)
{
	struct view *view = state_get_view(state);
	struct sel *sel;
	struct sys *sys;
	int i;

	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));
	if (sel_get_count(sel) == 0) {
		sel_free(sel);
		return (1);
	}
	view_snapshot(view);
	sys = view_get_sys(view);
	for (i = sys_get_atom_count(sys) - 1; i >= 0; i--)
		if (sel_selected(sel, i))
			sys_remove_atom(sys, i);
	error_set("deleted %d atoms", sel_get_count(sel));
	sel_free(sel);
	return (1);
}

static int
fn_view_center_selection(struct tokq *args, struct state *state)
{
	struct view *view = state_get_view(state);
	struct sel *sel;

	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));
	view_center_sel(view, sel);
	sel_free(sel);
	return (1);
}

static int
fn_view_fit_selection(struct tokq *args, struct state *state)
{
	struct view *view = state_get_view(state);
	struct sel *sel;

	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));
	view_fit_sel(view, sel);
	sel_free(sel);
	return (1);
}

static int
fn_set_frame(struct tokq *args, struct state *state)
{
	struct view *view = state_get_view(state);
	struct sys *sys;
	int n;

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
fn_next_frame(struct tokq *args, struct state *state)
{
	struct view *view = state_get_view(state);
	struct sys *sys = view_get_sys(view);
	int n = 1;

	if (tokq_count(args) > 0)
		n = tok_int(tokq_tok(args, 0));
	sys_set_frame(sys, sys_get_frame(sys) + n);
	return (1);
}

static int
fn_fullscreen(struct tokq *args __unused, struct state *state)
{
	state_toggle_fullscreen(state);

	return (1);
}

static int
fn_hide_selection(struct tokq *args, struct state *state)
{
	struct view *view = state_get_view(state);
	struct sel *sel;
	int idx;

	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));
	sel_iter_start(sel);
	while (sel_iter_next(sel, &idx)) {
		sel_remove(view_get_visible(view), idx);
		sel_remove(view_get_sel(view), idx);
	}
	sel_free(sel);
	return (1);
}

static int
fn_invert_selection(struct tokq *args __unused, struct state *state)
{
	struct view *view = state_get_view(state);
	struct sel *visible;
	int idx;

	visible = view_get_visible(view);
	sel_iter_start(visible);
	while (sel_iter_next(visible, &idx)) {
		if (sel_selected(view_get_sel(view), idx))
			sel_remove(view_get_sel(view), idx);
		else
			sel_add(view_get_sel(view), idx);
	}
	return (1);
}

static int
fn_view_move(struct tokq *args, struct state *state)
{
	struct view *view = state_get_view(state);
	vec_t xyz;

	xyz = parse_vec(args, 0);
	camera_move(view_get_camera(view), xyz);
	return (1);
}

static int
fn_set_element(struct tokq *args, struct state *state)
{
	struct view *view = state_get_view(state);
	struct sel *sel;
	const char *name;
	int idx;

	if (tokq_count(args) < 1) {
		error_set("specify element name");
		return (0);
	}
	name = tok_string(tokq_tok(args, 0));
	sel = make_sel(args, 1, tokq_count(args), view_get_sel(view));
	if (sel_get_count(sel) == 0) {
		sel_free(sel);
		return (1);
	}
	view_snapshot(view);
	sel_iter_start(sel);
	while (sel_iter_next(sel, &idx))
		sys_set_atom_name(view_get_sys(view), idx, name);
	sel_free(sel);
	return (1);
}

static int
fn_new(struct tokq *args, struct state *state)
{
	const char *path = "";

	if (tokq_count(args) > 0)
		path = tok_string(tokq_tok(args, 0));
	return (wnd_open(state_get_wnd(state), path));
}

static int
fn_first_window(struct tokq *args __unused, struct state *state)
{
	wnd_first(state_get_wnd(state));

	return (1);
}

static int
fn_last_window(struct tokq *args __unused, struct state *state)
{
	wnd_last(state_get_wnd(state));

	return (1);
}

static int
fn_prev_window(struct tokq *args __unused, struct state *state)
{
	if (!wnd_prev(state_get_wnd(state))) {
		error_set("no previous window");
		return (0);
	}
	return (1);
}

static int
fn_next_window(struct tokq *args __unused, struct state *state)
{
	if (!wnd_next(state_get_wnd(state))) {
		error_set("no next window");
		return (0);
	}
	return (1);
}

static int
fn_paste(struct tokq *args __unused, struct state *state)
{
	struct view *view = state_get_view(state);
	struct yank *yank = state_get_yank(state);
	int i, natoms, npaste, reg;

	reg = get_register(state);
	if ((npaste = yank_get_atom_count(yank, reg)) == 0)
		return (1);
	view_snapshot(view);
	yank_paste(yank, view_get_sys(view), reg);
	natoms = sys_get_atom_count(view_get_sys(view));
	sel_clear(view_get_sel(view));
	for (i = 0; i < npaste; i++)
		sel_add(view_get_sel(view), natoms-npaste+i);
	return (1);
}

static int
fn_replay(struct tokq *args __unused, struct state *state)
{
	struct rec *rec = state_get_rec(state);
	int reg;

	reg = get_register(state);
	if (rec_is_playing(rec))
		return (1);
	if (rec_is_recording(rec)) {
		error_set("cannot replay during recording");
		return (0);
	}
	if (!rec_play(rec, reg, state))
		return (0);
	return (1);
}

static int
fn_set_position(struct tokq *args, struct state *state)
{
	struct view *view = state_get_view(state);
	struct sys *sys;
	struct sel *sel;
	vec_t dr, xyz;
	int idx;

	dr = parse_vec(args, 0);
	sel = make_sel(args, 3, tokq_count(args), view_get_sel(view));
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
fn_measure(struct tokq *args, struct state *state)
{
	struct view *view = state_get_view(state);
	struct sys *sys;
	struct sel *sel;
	vec_t p[4];
	int a[4], i;

	sys = view_get_sys(view);
	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));
	if (sel_get_count(sel) < 1 || sel_get_count(sel) > 4) {
		sel_free(sel);
		error_set("select 1, 2, 3, or 4 atoms");
		return (0);
	}
	sel_iter_start(sel);
	for (i = 0; i < sel_get_count(sel); i++) {
		sel_iter_next(sel, &a[i]);
		p[i] = sys_get_atom_xyz(sys, a[i]);
	}
	switch (sel_get_count(sel)) {
	case 1:
		error_set("%d position: %.3lf %.3lf %.3lf", a[0]+1,
		    p[0].x, p[0].y, p[0].z);
		break;
	case 2:
		error_set("%d-%d distance: %.3lf", a[0]+1, a[1]+1,
		    vec_dist(&p[0], &p[1]));
		break;
	case 3:
		error_set("%d-%d-%d angle: %.3lf", a[0]+1, a[1]+1, a[2]+1,
		    vec_angle(&p[0], &p[1], &p[2]) * 180 / PI);
		break;
	case 4:
		error_set("%d-%d-%d-%d torsion: %.3lf", a[0]+1, a[1]+1, a[2]+1,
		    a[3]+1, vec_torsion(&p[0], &p[1], &p[2], &p[3]) * 180 / PI);
		break;
	}
	sel_free(sel);
	return (1);
}

static int
fn_move_selection(struct tokq *args, struct state *state)
{
	struct view *view = state_get_view(state);
	struct sys *sys;
	struct sel *sel;
	mat_t rotmat;
	vec_t dr, xyz;
	int idx;

	dr = parse_vec(args, 0);
	sel = make_sel(args, 3, tokq_count(args), view_get_sel(view));
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
fn_rec(struct tokq *args __unused, struct state *state)
{
	struct rec *rec = state_get_rec(state);

	if (rec_is_playing(rec))
		return (1);
	if (rec_is_recording(rec)) {
		rec_stop(rec);
		return (1);
	}
	rec_start(rec, get_register(state));
	return (1);
}

static int
fn_reset_bonds(struct tokq *args __unused, struct state *state)
{
	struct view *view = state_get_view(state);

	sys_reset_bonds(view_get_sys(view));

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
	struct view *view = state_get_view(state);
	struct graph *graph;
	struct sys *sys;
	mat_t rotmat;
	vec_t xyz;
	double angle, radius;
	int i, natoms, nring = 6;

	if (tokq_count(args) > 0) {
		nring = tok_int(tokq_tok(args, 0));
		if (nring < 3) {
			error_set("specify at least 3 atoms");
			return (0);
		}
	}
	view_snapshot(view);
	sys = view_get_sys(view);
	graph = view_get_graph(view);
	rotmat = camera_get_rotation(view_get_camera(view));
	rotmat = mat_transpose(&rotmat);
	radius = 1.54 / (2.0 * sin(PI / nring));
	for (i = 0; i < nring; i++) {
		angle = (1 + 2 * i) * PI / nring;
		xyz.x = radius * cos(angle);
		xyz.y = radius * sin(angle);
		xyz.z = 0.0;
		xyz = mat_vec(&rotmat, &xyz);
		sys_add_atom(sys, "C", xyz);
		if (i > 0) {
			natoms = sys_get_atom_count(sys);
			graph_edge_create(graph, natoms-2, natoms-1, 1);
		}
	}
	natoms = sys_get_atom_count(sys);
	graph_edge_create(graph, natoms-1, natoms-nring, 1);
	return (1);
}

static int
fn_rotate_selection(struct tokq *args, struct state *state)
{
	struct view *view = state_get_view(state);
	struct sys *sys;
	struct sel *sel;
	mat_t matrix, rotmat;
	vec_t abc, pos, xyz;
	int idx;

	if (tokq_count(args) < 1) {
		error_set("specify rotation angles a b c");
		return (0);
	}
	abc = parse_vec(args, 0);
	sel = make_sel(args, 3, tokq_count(args), view_get_sel(view));
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
	struct view *view = state_get_view(state);
	vec_t xyz;

	xyz = parse_vec(args, 0);
	vec_scale(&xyz, PI / 180.0);
	camera_rotate(view_get_camera(view), xyz);
	return (1);
}

static int
fn_write(struct tokq *args, struct state *state)
{
	struct view *view = state_get_view(state);
	struct sys *sys;
	const char *path;

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
fn_select(struct tokq *args, struct state *state)
{
	struct view *view = state_get_view(state);
	struct sel *visible;
	struct sel *sel;
	int idx;

	visible = view_get_visible(view);
	if (tokq_count(args) == 0) {
		idx = state_get_number(state) - 1;
		if (idx < 0) {
			sel_iter_start(visible);
			while (sel_iter_next(visible, &idx)) {
				if (!sel_selected(view_get_sel(view), idx)) {
					sel_add(view_get_sel(view), idx);
					return (1);
				}
			}
			return (1);
		}
		if (idx < sel_get_size(view_get_sel(view)))
			if (sel_selected(visible, idx))
				sel_add(view_get_sel(view), idx);
		return (1);
	}
	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));
	sel_iter_start(sel);
	while (sel_iter_next(sel, &idx))
		if (sel_selected(visible, idx))
			sel_add(view_get_sel(view), idx);
	sel_free(sel);
	return (1);
}

static int
fn_select_box(struct tokq *args, struct state *state)
{
	struct view *view = state_get_view(state);
	struct sys *sys;
	struct sel *visible;
	struct sel *sel;
	struct spi *spi;
	struct pair pair;
	vec_t pi, pj;
	double radius = 4.0;
	int idx, npairs;

	if (tokq_count(args) > 0) {
		radius = tok_double(tokq_tok(args, 0));
		if (radius <= 0.0) {
			error_set("specify a positive number");
			return (0);
		}
	}
	sys = view_get_sys(view);
	visible = view_get_visible(view);
	spi = spi_create();
	for (idx = 0; idx < sys_get_atom_count(sys); idx++)
		spi_add_point(spi, sys_get_atom_xyz(sys, idx));
	spi_compute(spi, sqrt(3.0) * radius);
	npairs = spi_get_pair_count(spi);
	sel = make_sel(args, 1, tokq_count(args), view_get_sel(view));
	sel_iter_start(sel);
	while (sel_iter_next(sel, &idx))
		if (sel_selected(visible, idx))
			sel_add(view_get_sel(view), idx);
	for (idx = 0; idx < npairs; idx++) {
		pair = spi_get_pair(spi, idx);
		if (!sel_selected(visible, pair.i) ||
		    !sel_selected(visible, pair.j))
			continue;
		if (sel_selected(sel, pair.i) || sel_selected(sel, pair.j)) {
			pi = sys_get_atom_xyz(sys, pair.i);
			pj = sys_get_atom_xyz(sys, pair.j);
			if (fabs(pi.x-pj.x) <= radius &&
			    fabs(pi.y-pj.y) <= radius &&
			    fabs(pi.z-pj.z) <= radius) {
				sel_add(view_get_sel(view), pair.i);
				sel_add(view_get_sel(view), pair.j);
			}
		}
	}
	sel_free(sel);
	spi_free(spi);
	return (1);
}

static int
fn_select_sphere(struct tokq *args, struct state *state)
{
	struct view *view = state_get_view(state);
	struct sys *sys;
	struct sel *visible;
	struct sel *sel;
	struct spi *spi;
	struct pair pair;
	double radius = 4.0;
	int idx, npairs;

	if (tokq_count(args) > 0) {
		radius = tok_double(tokq_tok(args, 0));
		if (radius <= 0.0) {
			error_set("specify a positive number");
			return (0);
		}
	}
	sys = view_get_sys(view);
	visible = view_get_visible(view);
	spi = spi_create();
	for (idx = 0; idx < sys_get_atom_count(sys); idx++)
		spi_add_point(spi, sys_get_atom_xyz(sys, idx));
	spi_compute(spi, radius);
	npairs = spi_get_pair_count(spi);
	sel = make_sel(args, 1, tokq_count(args), view_get_sel(view));
	sel_iter_start(sel);
	while (sel_iter_next(sel, &idx))
		if (sel_selected(visible, idx))
			sel_add(view_get_sel(view), idx);
	for (idx = 0; idx < npairs; idx++) {
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
fn_select_molecule(struct tokq *args, struct state *state)
{
	struct view *view = state_get_view(state);
	struct sys *sys;
	struct graph *graph;
	struct sel *visible;
	struct sel *selected;
	struct sel *connected;
	int idx;

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
fn_select_element(struct tokq *args, struct state *state)
{
	struct view *view = state_get_view(state);
	struct sys *sys;
	struct sel *visible;
	const char *name;
	int i, k;

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
fn_select_water(struct tokq *args __unused, struct state *state)
{
	struct view *view = state_get_view(state);
	struct sys *sys;
	struct graph *graph;
	struct sel *visible;
	int i, j, k;

	sys = view_get_sys(view);
	graph = view_get_graph(view);
	visible = view_get_visible(view);
	sel_iter_start(visible);
	while (sel_iter_next(visible, &i)) {
		if (graph_get_edge_count(graph, i) != 2)
			continue;
		j = graph_edge_j(graph_get_edges(graph, i));
		k = graph_edge_j(graph_edge_next(graph_get_edges(graph, i)));
		if (!sel_selected(visible, j) ||
		    !sel_selected(visible, k))
			continue;
		if (sys_get_atom_type(sys, i) != 8 || /* O */
		    sys_get_atom_type(sys, j) != 1 || /* H */
		    sys_get_atom_type(sys, k) != 1)   /* H */
			continue;
		sel_add(view_get_sel(view), i);
		sel_add(view_get_sel(view), j);
		sel_add(view_get_sel(view), k);
	}
	return (1);
}

static int
fn_set(struct tokq *args, struct state *state __unused)
{
	const char *name, *value;
	char buf[1024];

	if (tokq_count(args) == 0) {
		error_set("specify a setting name");
		return (0);
	}
	name = tok_string(tokq_tok(args, 0));
	if (tokq_count(args) == 1) {
		if (!settings_has_node(name)) {
			error_set("unknown setting \"%s\"", name);
			return (0);
		}
		settings_printf(buf, sizeof buf, name);
		error_set("%s is %s", name, buf);
		return (1);
	}
	value = tok_string(tokq_tok(args, 1));
	return (settings_set(name, value));
}

static int
fn_show_all(struct tokq *args __unused, struct state *state)
{
	struct view *view = state_get_view(state);

	sel_all(view_get_visible(view));

	return (1);
}

static int
fn_source(struct tokq *args, struct state *state)
{
	if (tokq_count(args) < 1) {
		error_set("specify a file");
		return (0);
	}
	return (state_source(state, tok_string(tokq_tok(args, 0))));
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
fn_undo(struct tokq *args __unused, struct state *state)
{
	if (!view_undo(state_get_view(state))) {
		error_set("already at oldest change");
		return (0);
	}
	return (1);
}

static int
fn_redo(struct tokq *args __unused, struct state *state)
{
	if (!view_redo(state_get_view(state))) {
		error_set("already at newest change");
		return (0);
	}
	return (1);
}

static int
fn_unselect(struct tokq *args, struct state *state)
{
	struct view *view = state_get_view(state);
	struct sel *sel;
	int idx;

	if (tokq_count(args) == 0) {
		idx = state_get_number(state) - 1;
		if (idx < 0) {
			sel_iter_start(view_get_sel(view));
			while (sel_iter_next(view_get_sel(view), &idx))
				continue;
			if (idx >= 0)
				sel_remove(view_get_sel(view), idx);
			return (1);
		}
		if (idx < sel_get_size(view_get_sel(view)))
			sel_remove(view_get_sel(view), idx);
		return (1);
	}
	sel = make_sel(args, 0, tokq_count(args), view_get_sel(view));
	sel_iter_start(sel);
	while (sel_iter_next(sel, &idx))
		sel_remove(view_get_sel(view), idx);
	sel_free(sel);
	return (1);
}

static int
fn_view_zoom(struct tokq *args, struct state *state)
{
	struct view *view = state_get_view(state);
	double factor, radius;

	if (tokq_count(args) < 1)
		return (0);
	if ((factor = tok_double(tokq_tok(args, 0))) <= 0.0) {
		error_set("zoom factor must be positive");
		return (0);
	}
	radius = camera_get_radius(view_get_camera(view));
	camera_set_radius(view_get_camera(view), radius / factor);
	return (1);
}

typedef int (*exec_fn_t)(struct tokq *, struct state *);

/* this list MUST remain sorted */
static const struct node {
	const char *name;
	exec_fn_t fn;
} execlist[] = {
	{ "?", fn_about },
	{ "add-hydrogens", fn_add_hydrogens },
	{ "atom", fn_atom },
	{ "bind", fn_bind },
	{ "bond", fn_bond },
	{ "chain", fn_chain },
	{ "clo", fn_close },
	{ "clo!", fn_force_close },
	{ "clos", fn_close },
	{ "clos!", fn_force_close },
	{ "close", fn_close },
	{ "close!", fn_force_close },
	{ "copy-selection", fn_copy_selection },
	{ "delete-selection", fn_delete_selection },
	{ "first", fn_first_window },
	{ "first-window", fn_first_window },
	{ "fullscreen", fn_fullscreen },
	{ "hide-selection", fn_hide_selection },
	{ "invert-selection", fn_invert_selection },
	{ "last", fn_last_window },
	{ "last-window", fn_last_window },
	{ "measure", fn_measure },
	{ "move-selection", fn_move_selection },
	{ "new", fn_new },
	{ "next-frame", fn_next_frame },
	{ "next-window", fn_next_window },
	{ "open", fn_new },
	{ "paste", fn_paste },
	{ "prev-window", fn_prev_window },
	{ "q", fn_quit },
	{ "q!", fn_force_quit },
	{ "quit", fn_quit },
	{ "quit!", fn_force_quit },
	{ "rec", fn_rec },
	{ "redo", fn_redo },
	{ "replay", fn_replay },
	{ "reset-bonds", fn_reset_bonds },
	{ "ring", fn_ring },
	{ "rotate-selection", fn_rotate_selection },
	{ "select", fn_select },
	{ "select-box", fn_select_box },
	{ "select-element", fn_select_element },
	{ "select-molecule", fn_select_molecule },
	{ "select-sphere", fn_select_sphere },
	{ "select-water", fn_select_water },
	{ "set", fn_set },
	{ "set-element", fn_set_element },
	{ "set-frame", fn_set_frame },
	{ "set-position", fn_set_position },
	{ "show-all", fn_show_all },
	{ "source", fn_source },
	{ "toggle", fn_toggle },
	{ "undo", fn_undo },
	{ "unselect", fn_unselect },
	{ "view-center-selection", fn_view_center_selection },
	{ "view-fit-selection", fn_view_fit_selection },
	{ "view-move", fn_view_move },
	{ "view-reset", fn_view_reset },
	{ "view-rotate", fn_view_rotate },
	{ "view-zoom", fn_view_zoom },
	{ "w", fn_write },
	{ "write", fn_write },
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
	    sizeof node, compare));
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
