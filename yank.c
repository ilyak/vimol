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

struct yank {
	struct atoms *atoms;
	struct graph *graph;
};

struct yank *
yank_create(void)
{
	struct yank *yank;

	yank = xcalloc(1, sizeof *yank);
	yank->atoms = atoms_create();
	yank->graph = graph_create();

	return (yank);
}

void
yank_free(struct yank *yank)
{
	if (yank) {
		atoms_free(yank->atoms);
		graph_free(yank->graph);
		free(yank);
	}
}

int
yank_get_atom_count(struct yank *yank)
{
	return (atoms_get_count(yank->atoms));
}

void
yank_copy(struct yank *yank, struct sys *sys, struct sel *sel)
{
	struct graphedge *edge;
	const char *name;
	vec_t xyz;
	int i, j, type, *map;

	map = xcalloc(sys_get_atom_count(sys), sizeof *map);

	atoms_clear(yank->atoms);
	graph_clear(yank->graph);

	j = 0;
	sel_iter_start(sel);

	while (sel_iter_next(sel, &i)) {
		name = sys_get_atom_name(sys, i);
		xyz = sys_get_atom_xyz(sys, i);

		atoms_add(yank->atoms, name, xyz);
		graph_vertex_add(yank->graph);

		map[i] = j++;
	}

	sel_iter_start(sel);

	while (sel_iter_next(sel, &i)) {
		edge = graph_get_edges(sys_get_graph(sys), i);

		while (edge) {
			type = graph_edge_get_type(edge);
			j = graph_edge_j(edge);

			if (sel_selected(sel, j))
				graph_edge_create(yank->graph, map[i], map[j],
				    type);

			edge = graph_edge_next(edge);
		}
	}

	free(map);
}

void
yank_paste(struct yank *yank, struct sys *sys)
{
	struct graphedge *edge;
	const char *name;
	vec_t xyz;
	int i, j, n, type;

	n = sys_get_atom_count(sys);

	for (i = 0; i < atoms_get_count(yank->atoms); i++) {
		name = atoms_get_name(yank->atoms, i);
		xyz = atoms_get_xyz(yank->atoms, i);
		sys_add_atom(sys, name, xyz);
	}

	for (i = 0; i < graph_get_vertex_count(yank->graph); i++) {
		edge = graph_get_edges(yank->graph, i);

		while (edge) {
			type = graph_edge_get_type(edge);
			j = graph_edge_j(edge);
			graph_edge_create(sys_get_graph(sys),
			    n + i, n + j, type);
			edge = graph_edge_next(edge);
		}
	}
}
