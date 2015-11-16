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

struct edge {
	int type, i, j;
	struct edge *prev, *next, *rev;
};

struct graph {
	int nalloc, nelts;
	struct edge **edges;
};

static void
remove_edge(struct graph *graph, struct edge *edge)
{
	if (edge->next)
		edge->next->prev = edge->prev;

	if (edge->prev)
		edge->prev->next = edge->next;
	else
		graph->edges[edge->i] = edge->next;

	free(edge);
}

struct graph *
graph_create(void)
{
	struct graph *graph;

	graph = calloc(1, sizeof(*graph));
	graph->nalloc = 8;
	graph->edges = calloc(graph->nalloc, sizeof(struct edge *));

	return (graph);
}

struct graph *
graph_copy(struct graph *graph)
{
	struct graph *copy;
	struct edge *edge;
	int i;

	copy = graph_create();

	for (i = 0; i < graph_get_vertex_count(graph); i++)
		graph_vertex_add(copy);

	for (i = 0; i < graph_get_vertex_count(graph); i++)
		for (edge = graph->edges[i]; edge; edge = edge->next)
			graph_edge_create(copy, edge->i, edge->j, edge->type);

	return (copy);
}

void
graph_free(struct graph *graph)
{
	int i;

	for (i = 0; i < graph_get_vertex_count(graph); i++)
		graph_remove_vertex_edges(graph, i);

	free(graph->edges);
	free(graph);
}

void
graph_clear(struct graph *graph)
{
	int i;

	for (i = 0; i < graph_get_vertex_count(graph); i++)
		graph_remove_vertex_edges(graph, i);

	graph->nelts = 0;
}

void
graph_vertex_add(struct graph *graph)
{
	if (graph->nelts == graph->nalloc) {
		graph->nalloc *= 2;
		graph->edges = realloc(graph->edges,
		    graph->nalloc * sizeof(struct edge *));
	}
	graph->edges[graph->nelts] = NULL;
	graph->nelts++;
}

void
graph_vertex_remove(struct graph *graph, int idx)
{
	struct edge *edge;
	int i;

	graph_remove_vertex_edges(graph, idx);

	graph->nelts--;

	memmove(graph->edges + idx, graph->edges + idx + 1,
	    (graph->nelts - idx) * sizeof(struct edge *));

	for (i = 0; i < graph_get_vertex_count(graph); i++) {
		for (edge = graph->edges[i]; edge; edge = edge->next) {
			if (edge->i > idx) edge->i--;
			if (edge->j > idx) edge->j--;
		}
	}
}

void
graph_vertex_swap(struct graph *graph, int i, int j)
{
	struct edge *edge;

	assert(i >= 0 && i < graph_get_vertex_count(graph));
	assert(j >= 0 && j < graph_get_vertex_count(graph));
	assert(i != j);

	for (edge = graph->edges[i]; edge; edge = edge->next) {
		edge->i = j;
		edge->rev->j = j;
	}

	for (edge = graph->edges[j]; edge; edge = edge->next) {
		edge->i = i;
		edge->rev->j = i;
	}

	edge = graph->edges[i];
	graph->edges[i] = graph->edges[j];
	graph->edges[j] = edge;
}

int
graph_get_vertex_count(struct graph *graph)
{
	return (graph->nelts);
}

int
graph_get_edge_count(struct graph *graph, int idx)
{
	struct edge *edge;
	int count;

	assert(idx >= 0 && idx < graph_get_vertex_count(graph));

	count = 0;

	for (edge = graph->edges[idx]; edge; edge = edge->next)
		count++;

	return (count);
}

void
graph_remove_vertex_edges(struct graph *graph, int idx)
{
	assert(idx >= 0 && idx < graph_get_vertex_count(graph));

	while (graph->edges[idx]) {
		remove_edge(graph, graph->edges[idx]->rev);
		remove_edge(graph, graph->edges[idx]);
	}
}

void
graph_edge_create(struct graph *graph, int i, int j, int type)
{
	struct edge *edge_i, *edge_j;

	assert(i >= 0 && i < graph_get_vertex_count(graph));
	assert(j >= 0 && j < graph_get_vertex_count(graph));
	assert(i != j);

	if ((edge_i = graph_edge_find(graph, i, j))) {
		graph_edge_set_type(edge_i, type);
		return;
	}

	edge_i = calloc(1, sizeof(struct edge));
	edge_j = calloc(1, sizeof(struct edge));

	edge_i->i = i;
	edge_i->j = j;
	edge_i->type = type;
	edge_i->rev = edge_j;
	if (graph->edges[i]) {
		edge_i->next = graph->edges[i];
		graph->edges[i]->prev = edge_i;
	}
	graph->edges[i] = edge_i;

	edge_j->i = j;
	edge_j->j = i;
	edge_j->type = type;
	edge_j->rev = edge_i;
	if (graph->edges[j]) {
		edge_j->next = graph->edges[j];
		graph->edges[j]->prev = edge_j;
	}
	graph->edges[j] = edge_j;
}

void
graph_edge_remove(struct graph *graph, int i, int j)
{
	struct edge *edge;

	assert(i >= 0 && i < graph_get_vertex_count(graph));
	assert(j >= 0 && j < graph_get_vertex_count(graph));
	assert(i != j);

	edge = graph_edge_find(graph, i, j);

	if (edge) {
		remove_edge(graph, edge->rev);
		remove_edge(graph, edge);
	}
}

struct edge *
graph_edges(struct graph *graph, int idx)
{
	assert(idx >= 0 && idx < graph_get_vertex_count(graph));

	return (graph->edges[idx]);
}

struct edge *
graph_edge_find(struct graph *graph, int i, int j)
{
	struct edge *edge;

	assert(i >= 0 && i < graph_get_vertex_count(graph));
	assert(j >= 0 && j < graph_get_vertex_count(graph));
	assert(i != j);

	for (edge = graph->edges[i]; edge; edge = edge->next)
		if (edge->j == j)
			return (edge);

	return (NULL);
}

struct edge *
graph_edge_prev(struct edge *edge)
{
	return (edge->prev);
}

struct edge *
graph_edge_next(struct edge *edge)
{
	return (edge->next);
}

int
graph_edge_get_type(struct edge *edge)
{
	return (edge->type);
}

void
graph_edge_set_type(struct edge *edge, int type)
{
	edge->type = type;
	edge->rev->type = type;
}

int
graph_edge_i(struct edge *edge)
{
	return (edge->i);
}

int
graph_edge_j(struct edge *edge)
{
	return (edge->j);
}
