#ifndef VIMOL_WINS_H
#define VIMOL_WINS_H

/* window management */
struct wins;

struct wins *wins_create(void);
void wins_free(struct wins *);
struct view *wins_get_view(struct wins *);
int wins_open(struct wins *, const char *);
int wins_close(struct wins *);
int wins_is_modified(struct wins *);
int wins_any_modified(struct wins *);
int wins_next(struct wins *);
int wins_prev(struct wins *);
void wins_first(struct wins *);
void wins_last(struct wins *);

#endif /* VIMOL_WINS_H */
