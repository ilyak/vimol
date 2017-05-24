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

#ifndef VIMOL_VIMOL_H
#define VIMOL_VIMOL_H

#define VIMOL_VERSION "2.0"

#define _GNU_SOURCE /* for vasprintf on linux */
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>
#include <cairo.h>

#ifndef __unused
#if defined(__GNUC__)
#define __unused __attribute__((__unused__))
#else
#define __unused
#endif
#endif /* __unused */

#ifndef __dead
#if defined(__GNUC__)
#define __dead __attribute__((__noreturn__))
#else
#define __dead
#endif
#endif /* __dead */

/* OS is defined in SDL */
#if defined(__WIN32__)
#define VIMOL_DEFAULT_FONT "consolas"
#elif defined(__MACOSX__)
#define VIMOL_DEFAULT_FONT "andalemono"
#else
#define VIMOL_DEFAULT_FONT "liberationmono"
#endif

#include "vec.h"

typedef struct {
	double r, g, b;
} color_t;

struct pair {
	int i, j;
};
struct pairs;       /* dynamic array of pairs */

typedef const char *tok_t; /* a tokq token */

struct atoms;       /* atom storage */
struct bind;        /* key-command bindings */
struct camera;      /* an eye of a user */
struct edit;        /* string edit control */
struct graph;       /* vertices connected with edges */
struct graphedge;   /* edge of a graph */
struct history;     /* command-line history management */
struct rec;         /* command recording */
struct sel;         /* selection of objects */
struct spi;         /* spatial index */
struct state;       /* app state */
struct statusbar;   /* status bar and command line */
struct sys;         /* molecular system structure */
struct tabs;        /* tabs */
struct tokq;        /* token list */
struct undo;        /* undo-redo management */
struct view;        /* viewport */
struct yank;        /* copy-paste buffer */

/* atoms.c */
struct atoms *atoms_create(void);
struct atoms *atoms_copy(struct atoms *);
void atoms_free(struct atoms *);
int atoms_get_frame(struct atoms *);
void atoms_set_frame(struct atoms *, int);
int atoms_get_frame_count(struct atoms *);
void atoms_add_frame(struct atoms *);
void atoms_add(struct atoms *, const char *, vec_t);
void atoms_remove(struct atoms *, int);
void atoms_clear(struct atoms *);
int atoms_get_count(struct atoms *);
const char *atoms_get_name(struct atoms *, int);
int atoms_get_type(struct atoms *, int);
void atoms_set_name(struct atoms *, int, const char *);
vec_t atoms_get_xyz(struct atoms *, int);
void atoms_set_xyz(struct atoms *, int, vec_t);

/* bind.c */
struct bind *bind_create(void);
void bind_free(struct bind *);
int bind_set(struct bind *, const char *, const char *);
const char *bind_get(struct bind *, const char *);

/* camera.c */
struct camera *camera_create(void);
void camera_free(struct camera *);
void camera_reset(struct camera *);
void camera_move(struct camera *, vec_t);
void camera_move_to(struct camera *, vec_t);
void camera_rotate(struct camera *, vec_t);
mat_t camera_get_rotation(struct camera *);
void camera_set_scale(struct camera *, double);
double camera_get_scale(struct camera *);
void camera_set_radius(struct camera *, double);
double camera_get_radius(struct camera *);
double camera_get_zoom(struct camera *, int, int);
point_t camera_transform(struct camera *, vec_t);

/* cmd.c */
int cmd_is_valid(const char *);
int cmd_exec(const char *, struct state *);

/* edit.c */
struct edit *edit_create(void);
void edit_free(struct edit *);
const char *edit_get_text(struct edit *);
int edit_get_text_length(struct edit *);
int edit_get_pos(struct edit *);
void edit_set_pos(struct edit *, int);
void edit_set_text(struct edit *, const char *, ...);
void edit_insert_char(struct edit *, char);
void edit_insert_text(struct edit *, const char *);
void edit_backspace_char(struct edit *);
void edit_delete_char(struct edit *);
void edit_backspace_word(struct edit *);
void edit_clear(struct edit *);

/* error.c */
void error_set(const char *, ...);
const char *error_get(void);
void error_clear(void);

/* exec.c */
int exec_is_valid(const char *);
int exec_run(const char *, struct tokq *, struct state *);

/* formats.c */
struct atoms *formats_load(const char *);
int formats_save(struct atoms *, const char *);

/* graph.c */
struct graph *graph_create(void);
struct graph *graph_copy(struct graph *);
void graph_free(struct graph *);
void graph_clear(struct graph *);
void graph_vertex_add(struct graph *);
void graph_vertex_remove(struct graph *, int);
int graph_get_vertex_count(struct graph *);
int graph_get_edge_count(struct graph *, int);
void graph_remove_vertex_edges(struct graph *, int);
void graph_edge_create(struct graph *, int, int, int);
void graph_edge_remove(struct graph *, int, int);
struct graphedge *graph_get_edges(struct graph *, int);
struct graphedge *graph_edge_find(struct graph *, int, int);
struct graphedge *graph_edge_prev(struct graphedge *);
struct graphedge *graph_edge_next(struct graphedge *);
int graph_edge_get_type(struct graphedge *);
void graph_edge_set_type(struct graphedge *, int);
int graph_edge_i(struct graphedge *);
int graph_edge_j(struct graphedge *);

/* history.c */
struct history *history_create(void);
void history_free(struct history *);
void history_load(struct history *, const char *);
void history_save(struct history *, const char *);
void history_push(struct history *, const char *);
void history_reset_current(struct history *);
int history_next(struct history *);
int history_prev(struct history *);
const char *history_get(struct history *);
int history_search(struct history *, const char *);

/* pair.c */
struct pairs *pairs_create(void);
void pairs_free(struct pairs *);
void pairs_clear(struct pairs *);
void pairs_add(struct pairs *, int, int);
int pairs_get_count(struct pairs *);
struct pair pairs_get(struct pairs *, int);

/* rec.c */
struct rec *rec_create(void);
void rec_free(struct rec *);
int rec_is_playing(struct rec *);
int rec_is_recording(struct rec *);
void rec_start(struct rec *);
void rec_add(struct rec *, const char *);
void rec_stop(struct rec *);
int rec_play(struct rec *, struct state *);

/* sel.c */
struct sel *sel_create(int);
struct sel *sel_copy(struct sel *);
void sel_free(struct sel *);
int sel_get_size(struct sel *);
int sel_get_count(struct sel *);
void sel_expand(struct sel *);
void sel_contract(struct sel *, int);
void sel_add(struct sel *, int);
void sel_remove(struct sel *, int);
void sel_all(struct sel *);
void sel_clear(struct sel *);
int sel_selected(struct sel *, int);
void sel_iter_start(struct sel *);
int sel_iter_next(struct sel *, int *);

/* settings.c */
void settings_init(void);
void settings_free(void);
int settings_printf(char *, size_t, const char *);
int settings_has_int(const char *);
int settings_has_double(const char *);
int settings_has_bool(const char *);
int settings_has_string(const char *);
int settings_has_color(const char *);
int settings_has_node(const char *);
int settings_get_int(const char *);
double settings_get_double(const char *);
int settings_get_bool(const char *);
const char *settings_get_string(const char *);
color_t settings_get_color(const char *);
int settings_set(const char *, const char *);

/* spi.c */
struct spi *spi_create(void);
void spi_free(struct spi *);
void spi_add_point(struct spi *, vec_t);
int spi_get_point_count(struct spi *);
vec_t spi_get_point(struct spi *, int);
void spi_clear(struct spi *);
void spi_compute(struct spi *, double);
int spi_get_pair_count(struct spi *);
struct pair spi_get_pair(struct spi *, int);

/* state.c */
struct state *state_create(void);
void state_free(struct state *);
struct bind *state_get_bind(struct state *);
struct rec *state_get_rec(struct state *);
struct view *state_get_view(struct state *);
struct tabs *state_get_tabs(struct state *);
struct yank *state_get_yank(struct state *);
int state_get_index(struct state *);
int state_source(struct state *, const char *);
void state_render(struct state *);
void state_toggle_fullscreen(struct state *);
void state_quit(struct state *, int);
void state_event_loop(struct state *);
void state_save(struct state *);

/* statusbar.c */
struct statusbar *statusbar_create(void);
void statusbar_free(struct statusbar *);
const char *statusbar_get_text(struct statusbar *);
void statusbar_set_text(struct statusbar *, const char *, ...);
void statusbar_set_error(struct statusbar *, const char *, ...);
void statusbar_clear_text(struct statusbar *);
const char *statusbar_get_info_text(struct statusbar *);
void statusbar_set_info_text(struct statusbar *, const char *, ...);
void statusbar_clear_info_text(struct statusbar *);
void statusbar_set_cursor_pos(struct statusbar *, int);
void statusbar_render(struct statusbar *, cairo_t *);

/* sys.c */
struct sys *sys_create(const char *);
struct sys *sys_copy(struct sys *);
void sys_free(struct sys *);
struct graph *sys_get_graph(struct sys *);
struct sel *sys_get_sel(struct sys *);
struct sel *sys_get_visible(struct sys *);
int sys_is_modified(struct sys *);
int sys_get_frame(struct sys *);
void sys_set_frame(struct sys *, int);
int sys_get_frame_count(struct sys *);
void sys_add_atom(struct sys *, const char *, vec_t);
void sys_remove_atom(struct sys *, int);
int sys_get_atom_count(struct sys *);
const char *sys_get_atom_name(struct sys *, int);
int sys_get_atom_type(struct sys *, int);
void sys_set_atom_name(struct sys *, int, const char *);
vec_t sys_get_atom_xyz(struct sys *, int);
void sys_set_atom_xyz(struct sys *, int, vec_t);
void sys_add_hydrogens(struct sys *, struct sel *);
vec_t sys_get_sel_center(struct sys *, struct sel *);
void sys_reset_bonds(struct sys *);
int sys_save_to_file(struct sys *, const char *);

/* tabs.c */
struct tabs *tabs_create(void);
void tabs_free(struct tabs *);
struct view *tabs_get_view(struct tabs *);
int tabs_open(struct tabs *, const char *);
int tabs_close(struct tabs *, int);
int tabs_is_modified(struct tabs *);
int tabs_any_modified(struct tabs *);
int tabs_next(struct tabs *);
int tabs_prev(struct tabs *);
void tabs_first(struct tabs *);
void tabs_last(struct tabs *);
int tabs_get_index(struct tabs *);
int tabs_get_count(struct tabs *);

/* tok.c */
int tok_int(tok_t);
double tok_double(tok_t);
int tok_bool(tok_t);
const char *tok_string(tok_t);
struct tokq *tokq_create(const char *);
struct tokq *tokq_copy(struct tokq *, int, int);
void tokq_free(struct tokq *);
char *tokq_strcat(struct tokq *, int, int);
int tokq_count(struct tokq *);
tok_t tokq_tok(struct tokq *, int);

/* undo.c */
struct undo *undo_create(void *, void *(*)(void *), void (*)(void *));
void undo_free(struct undo *);
void *undo_get_data(struct undo *);
void undo_snapshot(struct undo *);
int undo_undo(struct undo *);
int undo_redo(struct undo *);

/* util.c */
color_t color_rgb(int, int, int);
int color_to_string(char *, size_t, color_t);
int string_is_whitespace(const char *);
int string_is_comment(const char *);
int string_has_suffix(const char *, const char *);
int util_file_exists(const char *);
const char *util_basename(const char *);
char *util_next_line(char *, FILE *);
void warn(const char *, ...);
void fatal(const char *, ...) __dead;

/* view.c */
struct view *view_create(const char *);
void view_free(struct view *);
struct camera *view_get_camera(struct view *);
struct sys *view_get_sys(struct view *);
struct graph *view_get_graph(struct view *);
struct sel *view_get_sel(struct view *);
struct sel *view_get_visible(struct view *);
const char *view_get_path(struct view *);
void view_set_path(struct view *, const char *);
int view_is_new(struct view *);
int view_is_modified(struct view *);
int view_undo(struct view *);
int view_redo(struct view *);
void view_snapshot(struct view *);
void view_reset(struct view *);
void view_center_sel(struct view *, struct sel *);
void view_fit_sel(struct view *, struct sel *);
void view_render(struct view *, cairo_t *);

/* xmalloc.c */
void *xcalloc(size_t, size_t);
void *xrealloc(void *, size_t);
int xasprintf(char **, const char *, ...);
char *xstrdup(const char *);
char *xstrndup(const char *, size_t);

/* yank.c */
struct yank *yank_create(void);
void yank_free(struct yank *);
int yank_get_atom_count(struct yank *);
void yank_copy(struct yank *, struct sys *, struct sel *);
void yank_paste(struct yank *, struct sys *);

#endif /* VIMOL_VIMOL_H */
