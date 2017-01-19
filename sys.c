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

struct atom {
	char name[64];
	vec_t xyz;
};

struct sys {
	int is_modified;
	struct graph *graph;
	struct sel *sel;
	struct sel *visible;
	struct atoms *atoms;
};

static const vec_t h_table[] = {
	{  1.090,  0.000,  0.000 }, /*  0 C(sp3) H-1 */
	{ -0.360,  1.029,  0.000 }, /*  1 C(sp3) H-2 */
	{ -0.350, -0.523,  0.890 }, /*  2 C(sp3) H-3 */
	{ -0.350, -0.523, -0.890 }, /*  3 C(sp3) H-4 */
	{ -0.542,  0.946,  0.000 }, /*  4 C(sp2) H-1 */
	{ -0.542, -0.946,  0.000 }, /*  5 C(sp2) H-2 */
	{ -1.090,  0.000,  0.000 }, /*  6 C(sp)  H-1 */
	{  1.070,  0.000,  0.000 }, /*  7 N      H-1 */
	{ -0.353,  1.010,  0.000 }, /*  8 N      H-2 */
	{ -0.350, -0.510,  0.873 }, /*  9 N      H-3 */
	{  1.380,  0.000,  0.000 }, /* 10 P      H-1 */
	{ -0.455,  1.303,  0.000 }, /* 11 P      H-2 */
	{ -0.451, -0.658,  1.126 }, /* 12 P      H-3 */
	{  1.050,  0.000,  0.000 }, /* 13 O      H-1 */
	{ -0.350,  0.990,  0.000 }, /* 14 O      H-2 */
	{  1.340,  0.000,  0.000 }, /* 15 S      H-1 */
	{ -0.447,  1.263,  0.000 }, /* 16 S      H-2 */
};

static void
add_hydrogens(struct sys *sys, int i, int j, int k, int offset, int count)
{
	mat_t rotmat;
	vec_t pi, pj, pk;

	pi = sys_get_atom_xyz(sys, i);

	if (j < 0) {
		pj = vec_random();
		pj = vec_add(&pj, &pi);
	} else
		pj = sys_get_atom_xyz(sys, j);

	if (k < 0) {
		pk = vec_random();
		pk = vec_add(&pk, &pi);
	} else
		pk = sys_get_atom_xyz(sys, k);

	rotmat = mat_align(&pi, &pj, &pk);

	for (k = 0; k < count; k++) {
		pk = h_table[offset + k];
		pk = mat_vec(&rotmat, &pk);
		pk = vec_add(&pk, &pi);

		sys_add_atom(sys, "H", pk);

		j = sys_get_atom_count(sys) - 1;
		graph_edge_create(sys->graph, i, j, 1);
	}
}

static int
get_bond_count(struct graph *graph, int idx)
{
	struct graphedge *edge;
	int count = 0;

	for (edge = graph_get_edges(graph, idx); edge;
	    edge = graph_edge_next(edge))
		count += graph_edge_get_type(edge);

	return (count);
}

static int
parse_atom_pdb(const char *buf, struct atom *atom)
{
	size_t i, j, buflen;

	if ((buflen = strlen(buf)) < 54) {
		error_set("incorrect pdb atom record format");
		return (0);
	}

	memset(atom, 0, sizeof(*atom));

	if (buflen >= 72) {
		for (i = 70, j = 0; i < 72; i++)
			if (isalpha(buf[i]))
				atom->name[j++] = buf[i];
	}

	if (atom->name[0] == '\0' && buflen >= 78) {
		for (i = 76, j = 0; i < 78; i++)
			if (isalpha(buf[i]))
				atom->name[j++] = buf[i];
	}

	if (atom->name[0] == '\0') {
		for (i = 12, j = 0; i < 14; i++)
			if (isalpha(buf[i]))
				atom->name[j++] = buf[i];
	}

	if (atom->name[0] == '\0')
		atom->name[0] = 'X';

	sscanf(buf+30, "%lf%lf%lf", &atom->xyz.x, &atom->xyz.y, &atom->xyz.z);

	return (1);
}

static int
load_from_pdb(struct sys *sys, const char *path)
{
	FILE *fp;
	struct atom atom;
	char *buffer;
	int i, newframe = 0;

	if ((fp = fopen(path, "r")) == NULL) {
		error_set("unable to open %s", path);
		return (0);
	}

	buffer = NULL;
	i = 0;

	while ((buffer = util_next_line(buffer, fp)) != NULL) {
		if (strncasecmp(buffer, "ATOM  ", 6) == 0 ||
		    strncasecmp(buffer, "HETATM", 6) == 0) {
			if (newframe) {
				atoms_add_frame(sys->atoms);
				newframe = 0;
			}
			if (!parse_atom_pdb(buffer, &atom))
				goto error;
			if (sys_get_frame_count(sys) == 1)
				sys_add_atom(sys, atom.name, atom.xyz);
			else
				atoms_set_xyz(sys->atoms, i, atom.xyz);
			i++;
		}
		if (strncasecmp(buffer, "END", 3) == 0) {
			i = 0;
			newframe = 1;
		}
	}

	fclose(fp);
	return (1);
error:
	free(buffer);
	fclose(fp);
	return (0);
}

static int
parse_atom_xyz(const char *buffer, struct atom *atom)
{
	memset(atom, 0, sizeof(struct atom));
	sscanf(buffer, "%32s%lf%lf%lf", atom->name, &atom->xyz.x,
	    &atom->xyz.y, &atom->xyz.z);
	return (1);
}

static int
load_from_xyz(struct sys *sys, const char *path)
{
	FILE *fp;
	char *buffer;
	struct atom atom;
	int i, n;

	if ((fp = fopen(path, "r")) == NULL) {
		error_set("unable to open %s", path);
		return (0);
	}

	if ((buffer = util_next_line(NULL, fp)) == NULL) {
		error_set("unexpected end of file");
		goto error;
	}

	if (sscanf(buffer, "%d", &n) != 1 || n < 1) {
		error_set("unexpected number of atoms");
		goto error;
	}

	buffer = util_next_line(buffer, fp);

	for (i = 0; i < n; i++) {
		if ((buffer = util_next_line(buffer, fp)) == NULL) {
			error_set("unexpected end of file");
			goto error;
		}

		if (!parse_atom_xyz(buffer, &atom))
			goto error;

		sys_add_atom(sys, atom.name, atom.xyz);
	}

	while ((buffer = util_next_line(buffer, fp)) != NULL) {
		if (string_is_whitespace(buffer))
			continue;

		if (sscanf(buffer, "%d", &n) != 1 ||
		    n != sys_get_atom_count(sys)) {
			error_set("unexpected number of atoms");
			goto error;
		}

		atoms_add_frame(sys->atoms);
		buffer = util_next_line(buffer, fp);

		for (i = 0; i < n; i++) {
			if ((buffer = util_next_line(buffer, fp)) == NULL) {
				error_set("unexpected end of file");
				goto error;
			}
			if (!parse_atom_xyz(buffer, &atom))
				goto error;
			atoms_set_xyz(sys->atoms, i, atom.xyz);
		}
	}

	fclose(fp);
	return (1);
error:
	free(buffer);
	fclose(fp);
	return (0);
}

static int
load_file(struct sys *sys, const char *path)
{
	if (string_has_suffix(path, ".pdb"))
		return (load_from_pdb(sys, path));

	if (string_has_suffix(path, ".xyz"))
		return (load_from_xyz(sys, path));

	error_set("unknown file type");
	return (0);
}

static void
save_to_xyz(struct sys *sys, FILE *fp)
{
	vec_t xyz;
	const char *name;
	int i, j, frame;

	frame = atoms_get_frame(sys->atoms);

	for (i = 0; i < sys_get_frame_count(sys); i++) {
		atoms_set_frame(sys->atoms, i);
		fprintf(fp, "%d\n\n", atoms_get_count(sys->atoms));
		for (j = 0; j < atoms_get_count(sys->atoms); j++) {
			name = atoms_get_name(sys->atoms, j);
			xyz = atoms_get_xyz(sys->atoms, j);
#define XYZFMT "%-4s %11.6lf %11.6lf %11.6lf"
			fprintf(fp, XYZFMT, name, xyz.x, xyz.y, xyz.z);
			fprintf(fp, "\n");
		}
	}
	atoms_set_frame(sys->atoms, frame);
}

static void
save_to_pdb(struct sys *sys, FILE *fp)
{
	vec_t xyz;
	const char *name;
	int i, j, frame;

	frame = atoms_get_frame(sys->atoms);

	for (i = 0; i < sys_get_frame_count(sys); i++) {
		atoms_set_frame(sys->atoms, i);
		for (j = 0; j < atoms_get_count(sys->atoms); j++) {
			name = atoms_get_name(sys->atoms, j);
			xyz = atoms_get_xyz(sys->atoms, j);
#define PDBFMT "ATOM  %5d%3s                %8.3lf%8.3lf%8.3lf"
			fprintf(fp, PDBFMT, j+1, name, xyz.x, xyz.y, xyz.z);
			fprintf(fp, "\n");
		}
		fprintf(fp, "END\n");
	}
	atoms_set_frame(sys->atoms, frame);
}

struct sys *
sys_create(const char *path)
{
	struct sys *sys;

	sys = xcalloc(1, sizeof *sys);
	sys->graph = graph_create();
	sys->sel = sel_create(0);
	sys->visible = sel_create(0);
	sys->atoms = atoms_create();

	if (path == NULL || !util_file_exists(path))
		return (sys);

	if (!load_file(sys, path)) {
		sys_free(sys);
		return (NULL);
	}

	sys_set_frame(sys, 0);
	sys_reset_bonds(sys);
	sys->is_modified = 0;

	return (sys);
}

struct sys *
sys_copy(struct sys *sys)
{
	struct sys *copy;

	copy = xcalloc(1, sizeof *sys);
	copy->is_modified = sys->is_modified;
	copy->graph = graph_copy(sys->graph);
	copy->sel = sel_copy(sys->sel);
	copy->visible = sel_copy(sys->visible);
	copy->atoms = atoms_copy(sys->atoms);

	return (copy);
}

void
sys_free(struct sys *sys)
{
	if (sys) {
		sel_free(sys->sel);
		sel_free(sys->visible);
		graph_free(sys->graph);
		atoms_free(sys->atoms);
	}
}

struct graph *
sys_get_graph(struct sys *sys)
{
	return (sys->graph);
}

struct sel *
sys_get_sel(struct sys *sys)
{
	return (sys->sel);
}

struct sel *
sys_get_visible(struct sys *sys)
{
	return (sys->visible);
}

int
sys_is_modified(struct sys *sys)
{
	return (sys->is_modified);
}

int
sys_get_frame(struct sys *sys)
{
	return (atoms_get_frame(sys->atoms));
}

void
sys_set_frame(struct sys *sys, int frame)
{
	int nframes = sys_get_frame_count(sys);

	if (frame < 0)
		frame = 0;
	if (frame > nframes-1)
		frame = nframes-1;

	atoms_set_frame(sys->atoms, frame);
}

int
sys_get_frame_count(struct sys *sys)
{
	return (atoms_get_frame_count(sys->atoms));
}

void
sys_add_atom(struct sys *sys, const char *name, vec_t xyz)
{
	atoms_add(sys->atoms, name, xyz);
	graph_vertex_add(sys->graph);
	sel_expand(sys->sel);
	sel_expand(sys->visible);
	sel_add(sys->visible, sel_get_size(sys->visible)-1);
	sys->is_modified = 1;
}

void
sys_remove_atom(struct sys *sys, int idx)
{
	atoms_remove(sys->atoms, idx);
	graph_vertex_remove(sys->graph, idx);
	sel_contract(sys->sel, idx);
	sel_contract(sys->visible, idx);
	sys->is_modified = 1;
}

void
sys_swap_atoms(struct sys *sys, int i, int j)
{
	atoms_swap(sys->atoms, i, j);
	graph_vertex_swap(sys->graph, i, j);
	sel_swap(sys->sel, i, j);
	sel_swap(sys->visible, i, j);
}

int
sys_get_atom_count(struct sys *sys)
{
	return (atoms_get_count(sys->atoms));
}

const char *
sys_get_atom_name(struct sys *sys, int idx)
{
	return (atoms_get_name(sys->atoms, idx));
}

int
sys_get_atom_type(struct sys *sys, int idx)
{
	return (atoms_get_type(sys->atoms, idx));
}

void
sys_set_atom_name(struct sys *sys, int idx, const char *name)
{
	atoms_set_name(sys->atoms, idx, name);
	sys->is_modified = 1;
}

vec_t
sys_get_atom_xyz(struct sys *sys, int idx)
{
	return (atoms_get_xyz(sys->atoms, idx));
}

void
sys_set_atom_xyz(struct sys *sys, int idx, vec_t xyz)
{
	atoms_set_xyz(sys->atoms, idx, xyz);
	sys->is_modified = 1;
}

void
sys_add_hydrogens(struct sys *sys, struct sel *sel)
{
	vec_t r1, r2, r3;
	int n_bond, n_neig;
	int i, j, k;

	sel_iter_start(sel);

	while (sel_iter_next(sel, &i)) {
		if (sys_get_atom_type(sys, i) == 6) { /* C */
			n_neig = graph_get_edge_count(sys->graph, i);
			n_bond = get_bond_count(sys->graph, i);
			j = k = -1;

			if (n_bond == 0) {
				/* add 4 sp3 */
				add_hydrogens(sys, i, j, k, 0, 4);
			} else if (n_bond == 1) {
				j = graph_edge_j(graph_get_edges(sys->graph, i));
				/* add 3 sp3 */
				add_hydrogens(sys, i, j, k, 1, 3);
			} else if (n_bond == 2) {
				j = graph_edge_j(graph_get_edges(sys->graph, i));
				if (n_neig == 1) {
					/* keep double bond atoms in plane */
					if (graph_get_edge_count(sys->graph, j) > 1) {
						if ((k = graph_edge_j(graph_get_edges(sys->graph, j))) == i)
							k = graph_edge_j(graph_edge_next(graph_get_edges(sys->graph, j)));
					}
					/* add 2 sp2 */
					add_hydrogens(sys, i, j, k, 4, 2);
				} else if (n_neig == 2) {
					k = graph_edge_j(graph_edge_next(graph_get_edges(sys->graph, i)));
					/* add 2 sp3 */
					add_hydrogens(sys, i, j, k, 2, 2);
				}
			} else if (n_bond == 3) {
				j = graph_edge_j(graph_get_edges(sys->graph, i));
				if (n_neig == 1) {
					/* add 1 sp */
					add_hydrogens(sys, i, j, k, 6, 1);
				} else if (n_neig == 2) {
					k = graph_edge_j(graph_edge_next(graph_get_edges(sys->graph, i)));
					/* add 1 sp2 */
					add_hydrogens(sys, i, j, k, 5, 1);
				} else if (n_neig == 3) {
					k = graph_edge_j(graph_edge_next(graph_get_edges(sys->graph, i)));
					/* add 1 sp3 */
					add_hydrogens(sys, i, j, k, 2, 1);

					/* check if it's the correct one */
					r1 = sys_get_atom_xyz(sys, graph_edge_j(graph_edge_next(graph_edge_next(graph_get_edges(sys->graph, i)))));
					r2 = sys_get_atom_xyz(sys, sys_get_atom_count(sys) - 1);
					r3 = sys_get_atom_xyz(sys, i);

					r1 = vec_sub(&r1, &r3);
					r2 = vec_sub(&r2, &r3);

					if (vec_dot(&r1, &r2) > 0.0) {
						sys_remove_atom(sys, sys_get_atom_count(sys) - 1);
						add_hydrogens(sys, i, j, k, 3, 1);
					}
				}
			}
		} else if (sys_get_atom_type(sys, i) == 7) { /* N */
			n_neig = graph_get_edge_count(sys->graph, i);
			n_bond = get_bond_count(sys->graph, i);
			j = k = -1;

			if (n_bond == 0) {
				/* add 3 */
				add_hydrogens(sys, i, j, k, 7, 3);
			} else if (n_bond == 1) {
				j = graph_edge_j(graph_get_edges(sys->graph, i));
				/* add 2 */
				add_hydrogens(sys, i, j, k, 8, 2);
			} else if (n_bond == 2) {
				j = graph_edge_j(graph_get_edges(sys->graph, i));

				if (n_neig > 1)
					k = graph_edge_j(graph_edge_next(graph_get_edges(sys->graph, i)));

				/* add 1 */
				add_hydrogens(sys, i, j, k, 9, 1);
			}
		} else if (sys_get_atom_type(sys, i) == 8) { /* O */
			n_bond = get_bond_count(sys->graph, i);
			j = k = -1;

			if (n_bond == 0) {
				/* add 2 */
				add_hydrogens(sys, i, j, k, 13, 2);
			} else if (n_bond == 1) {
				j = graph_edge_j(graph_get_edges(sys->graph, i));
				/* add 1 */
				add_hydrogens(sys, i, j, k, 14, 1);
			}
		} else if (sys_get_atom_type(sys, i) == 15) { /* P */
			n_neig = graph_get_edge_count(sys->graph, i);
			n_bond = get_bond_count(sys->graph, i);
			j = k = -1;

			if (n_bond == 0) {
				/* add 3 */
				add_hydrogens(sys, i, j, k, 10, 3);
			} else if (n_bond == 1) {
				j = graph_edge_j(graph_get_edges(sys->graph, i));
				/* add 2 */
				add_hydrogens(sys, i, j, k, 11, 2);
			} else if (n_bond == 2) {
				j = graph_edge_j(graph_get_edges(sys->graph, i));

				if (n_neig > 1)
					k = graph_edge_j(graph_edge_next(graph_get_edges(sys->graph, i)));

				/* add 1 */
				add_hydrogens(sys, i, j, k, 12, 1);
			}
		} else if (sys_get_atom_type(sys, i) == 16) { /* S */
			n_bond = get_bond_count(sys->graph, i);
			j = k = -1;

			if (n_bond == 0) {
				/* add 2 */
				add_hydrogens(sys, i, j, k, 15, 2);
			} else if (n_bond == 1) {
				j = graph_edge_j(graph_get_edges(sys->graph, i));
				/* add 1 */
				add_hydrogens(sys, i, j, k, 16, 1);
			}
		}
	}
}

vec_t
sys_get_sel_center(struct sys *sys, struct sel *sel)
{
	vec_t center, xyz;
	int idx;

	center = vec_zero();

	if (sel_get_count(sel) == 0)
		return (center);

	sel_iter_start(sel);

	while (sel_iter_next(sel, &idx)) {
		xyz = sys_get_atom_xyz(sys, idx);
		center = vec_add(&center, &xyz);
	}

	vec_scale(&center, 1.0 / sel_get_count(sel));

	return (center);
}

void
sys_reset_bonds(struct sys *sys)
{
	struct pair pair;
	struct spi *spi;
	int i, j, k, n, np;

	n = sys_get_atom_count(sys);
	spi = spi_create();

	for (i = 0; i < n; i++) {
		graph_remove_vertex_edges(sys->graph, i);
		spi_add_point(spi, sys_get_atom_xyz(sys, i));
	}

	spi_compute(spi, 1.6);
	np = spi_get_pair_count(spi);

	for (k = 0; k < np; k++) {
		pair = spi_get_pair(spi, k);
		i = pair.i;
		j = pair.j;

		/* Hydrogens */
		if (sys_get_atom_type(sys, i) == 1 &&
		    sys_get_atom_type(sys, j) == 1)
			continue;

		if (graph_edge_find(sys->graph, i, j) == NULL)
			graph_edge_create(sys->graph, i, j, 1);
	}

	for (i = 0; i < n; i++) {
		/* Oxygen */
		if (sys_get_atom_type(sys, i) == 8 &&
		    graph_get_edge_count(sys->graph, i) == 1)
			graph_edge_set_type(graph_get_edges(sys->graph, i), 2);
	}

	spi_free(spi);
}

int
sys_save_to_file(struct sys *sys, const char *path)
{
	FILE *fp;

	if (!string_has_suffix(path, ".xyz") &&
	    !string_has_suffix(path, ".pdb")) {
		error_set("\"%s\": unknown file format", path);
		return (0);
	}

	if ((fp = fopen(path, "w")) == NULL) {
		error_set("unable to open \"%s\" for writing", path);
		return (0);
	}

	if (string_has_suffix(path, ".xyz"))
		save_to_xyz(sys, fp);
	else if (string_has_suffix(path, ".pdb"))
		save_to_pdb(sys, fp);

	fclose(fp);
	sys->is_modified = 0;
	return (1);
}
