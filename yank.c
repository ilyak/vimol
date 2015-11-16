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

struct tuple {
	struct atoms *atoms;
	struct graph *graph;
};

struct yank {
	struct tuple tuple[YANK_SIZE];
	int reg;
};

struct yank *
yank_create(void)
{
	struct yank *yank;
	int i;

	yank = calloc(1, sizeof(*yank));

	for (i = 0; i < YANK_SIZE; i++) {
		yank->tuple[i].atoms = atoms_create();
		yank->tuple[i].graph = graph_create();
	}

	return (yank);
}

void
yank_free(struct yank *yank)
{
	int i;

	for (i = 0; i < YANK_SIZE; i++) {
		atoms_free(yank->tuple[i].atoms);
		graph_free(yank->tuple[i].graph);
	}

	free(yank);
}

void
yank_set_register(struct yank *yank, int reg)
{
	assert(reg >= 0 && reg < YANK_SIZE);

	yank->reg = reg;
}

int
yank_get_register(struct yank *yank)
{
	return (yank->reg);
}

void
yank_copy(struct yank *yank, struct sys *sys, struct sel *sel)
{
	struct atoms *atoms;
	struct graph *graph;
	struct edge *edge;
	const char *name;
	vec_t xyz;
	int i, j, type, *map;

	atoms = yank->tuple[yank->reg].atoms;
	graph = yank->tuple[yank->reg].graph;

	map = calloc(sys_get_atom_count(sys), sizeof(int));

	atoms_clear(atoms);
	graph_clear(graph);

	j = 0;
	sel_iter_start(sel);

	while (sel_iter_next(sel, &i)) {
		name = sys_get_atom_name(sys, i);
		xyz = sys_get_atom_xyz(sys, i);

		atoms_add(atoms, name, xyz);
		graph_vertex_add(graph);

		map[i] = j++;
	}

	sel_iter_start(sel);

	while (sel_iter_next(sel, &i)) {
		edge = graph_edges(sys_get_graph(sys), i);

		while (edge) {
			type = graph_edge_get_type(edge);
			j = graph_edge_j(edge);

			if (sel_selected(sel, j))
				graph_edge_create(graph, map[i], map[j], type);

			edge = graph_edge_next(edge);
		}
	}

	free(map);
}

void
yank_paste(struct yank *yank, struct sys *sys)
{
	struct atoms *atoms;
	struct graph *graph;
	struct edge *edge;
	const char *name;
	vec_t xyz;
	int i, j, n, type;

	atoms = yank->tuple[yank->reg].atoms;
	graph = yank->tuple[yank->reg].graph;

	n = sys_get_atom_count(sys);

	for (i = 0; i < atoms_get_count(atoms); i++) {
		name = atoms_get_name(atoms, i);
		xyz = atoms_get_xyz(atoms, i);
		sys_add_atom(sys, name, xyz);
	}

	for (i = 0; i < graph_get_vertex_count(graph); i++) {
		edge = graph_edges(graph, i);

		while (edge) {
			type = graph_edge_get_type(edge);
			j = graph_edge_j(edge);
			graph_edge_create(sys_get_graph(sys),
			    n + i, n + j, type);
			edge = graph_edge_next(edge);
		}
	}
}
