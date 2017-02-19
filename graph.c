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

struct graphedge {
	int type, i, j;
	struct graphedge *rev; /* edge which is reverse to this one */
	struct graphedge *prev, *next;
};

struct graph {
	int nalloc, nelts;
	struct graphedge **edges;
};

static void
remove_edge(struct graph *graph, struct graphedge *edge)
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

	graph = xcalloc(1, sizeof *graph);
	graph->nalloc = 8;
	graph->edges = xcalloc(graph->nalloc, sizeof *graph->edges);

	return (graph);
}

struct graph *
graph_copy(struct graph *graph)
{
	struct graph *copy;
	struct graphedge *edge;
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

	if (graph) {
		for (i = 0; i < graph_get_vertex_count(graph); i++)
			graph_remove_vertex_edges(graph, i);
		free(graph->edges);
		free(graph);
	}
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
		graph->edges = xrealloc(graph->edges,
		    graph->nalloc * sizeof *graph->edges);
	}
	graph->edges[graph->nelts] = NULL;
	graph->nelts++;
}

void
graph_vertex_remove(struct graph *graph, int idx)
{
	struct graphedge *edge;
	int i;

	graph_remove_vertex_edges(graph, idx);

	graph->nelts--;

	memmove(graph->edges + idx, graph->edges + idx + 1,
	    (graph->nelts - idx) * sizeof *graph->edges);

	for (i = 0; i < graph_get_vertex_count(graph); i++) {
		for (edge = graph->edges[i]; edge; edge = edge->next) {
			if (edge->i > idx) edge->i--;
			if (edge->j > idx) edge->j--;
		}
	}
}

int
graph_get_vertex_count(struct graph *graph)
{
	return (graph->nelts);
}

int
graph_get_edge_count(struct graph *graph, int idx)
{
	struct graphedge *edge;
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
	struct graphedge *edge_i, *edge_j;

	assert(i >= 0 && i < graph_get_vertex_count(graph));
	assert(j >= 0 && j < graph_get_vertex_count(graph));
	assert(i != j);

	if ((edge_i = graph_edge_find(graph, i, j))) {
		graph_edge_set_type(edge_i, type);
		return;
	}

	edge_i = xcalloc(1, sizeof *edge_i);
	edge_j = xcalloc(1, sizeof *edge_j);

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
	struct graphedge *edge;

	assert(i >= 0 && i < graph_get_vertex_count(graph));
	assert(j >= 0 && j < graph_get_vertex_count(graph));
	assert(i != j);

	edge = graph_edge_find(graph, i, j);

	if (edge) {
		remove_edge(graph, edge->rev);
		remove_edge(graph, edge);
	}
}

struct graphedge *
graph_get_edges(struct graph *graph, int idx)
{
	assert(idx >= 0 && idx < graph_get_vertex_count(graph));

	return (graph->edges[idx]);
}

struct graphedge *
graph_edge_find(struct graph *graph, int i, int j)
{
	struct graphedge *edge;

	assert(i >= 0 && i < graph_get_vertex_count(graph));
	assert(j >= 0 && j < graph_get_vertex_count(graph));
	assert(i != j);

	for (edge = graph->edges[i]; edge; edge = edge->next)
		if (edge->j == j)
			return (edge);

	return (NULL);
}

struct graphedge *
graph_edge_prev(struct graphedge *edge)
{
	return (edge->prev);
}

struct graphedge *
graph_edge_next(struct graphedge *edge)
{
	return (edge->next);
}

int
graph_edge_get_type(struct graphedge *edge)
{
	return (edge->type);
}

void
graph_edge_set_type(struct graphedge *edge, int type)
{
	edge->type = type;
	edge->rev->type = type;
}

int
graph_edge_i(struct graphedge *edge)
{
	return (edge->i);
}

int
graph_edge_j(struct graphedge *edge)
{
	return (edge->j);
}
