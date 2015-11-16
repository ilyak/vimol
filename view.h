#ifndef VIMOL_VIEW_H
#define VIMOL_VIEW_H

/* molecule view */
struct view;

struct view *view_create(const char *);
void view_free(struct view *);
struct camera *view_get_camera(struct view *);
struct sys *view_get_sys(struct view *);
struct graph *view_get_graph(struct view *);
struct sel *view_get_sel(struct view *);
struct sel *view_get_visible(struct view *);
const char *view_get_path(struct view *);
void view_set_path(struct view *, const char *);
int view_is_empty(struct view *);
int view_is_modified(struct view *);
int view_undo(struct view *);
int view_redo(struct view *);
void view_snapshot(struct view *);
void view_reset(struct view *);
void view_center_sel(struct view *, struct sel *);
void view_fit_sel(struct view *, struct sel *);
void view_render(struct view *, cairo_t *);

#endif /* VIMOL_VIEW_H */
