#ifndef VIMOL_GRAPH_H
#define VIMOL_GRAPH_H

/* vertices connected with edges */
struct graph;

/* edge of a graph */
struct edge;

struct graph *graph_create(void);
struct graph *graph_copy(struct graph *);
void graph_free(struct graph *);
void graph_clear(struct graph *);
void graph_vertex_add(struct graph *);
void graph_vertex_remove(struct graph *, int);
void graph_vertex_swap(struct graph *, int, int);
int graph_get_vertex_count(struct graph *);
int graph_get_edge_count(struct graph *, int);
void graph_remove_vertex_edges(struct graph *, int);
void graph_edge_create(struct graph *, int, int, int);
void graph_edge_remove(struct graph *, int, int);
struct edge *graph_edges(struct graph *, int);
struct edge *graph_edge_find(struct graph *, int, int);
struct edge *graph_edge_prev(struct edge *);
struct edge *graph_edge_next(struct edge *);
int graph_edge_get_type(struct edge *);
void graph_edge_set_type(struct edge *, int);
int graph_edge_i(struct edge *);
int graph_edge_j(struct edge *);

#endif /* VIMOL_GRAPH_H */
