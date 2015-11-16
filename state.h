#ifndef VIMOL_STATE_H
#define VIMOL_STATE_H

/* program state */
struct state;

struct state *state_create(void);
void state_free(struct state *);
struct alias *state_get_alias(struct state *);
struct alias *state_get_bind(struct state *);
struct rec *state_get_rec(struct state *);
struct view *state_get_view(struct state *);
struct wins *state_get_wins(struct state *);
struct yank *state_get_yank(struct state *);
void state_start_edit(struct state *);
int state_source(struct state *, const char *);
void state_render(struct state *);
void state_save_png(struct state *, const char *);
void state_toggle_fullscreen(struct state *);
void state_quit(struct state *, int);
void state_event_loop(struct state *);
void state_save(struct state *);

#endif /* VIMOL_STATE_H */
