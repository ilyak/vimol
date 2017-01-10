#ifndef VIMOL_WND_H
#define VIMOL_WND_H

/* window management */
struct wnd;

struct wnd *wnd_create(void);
void wnd_free(struct wnd *);
struct view *wnd_get_view(struct wnd *);
int wnd_open(struct wnd *, const char *);
int wnd_close(struct wnd *);
int wnd_is_modified(struct wnd *);
int wnd_any_modified(struct wnd *);
int wnd_next(struct wnd *);
int wnd_prev(struct wnd *);
void wnd_first(struct wnd *);
void wnd_last(struct wnd *);

#endif /* VIMOL_WND_H */
