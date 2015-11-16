#ifndef VIMOL_HISTORY_H
#define VIMOL_HISTORY_H

/* command line history management */
struct history;

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

#endif /* VIMOL_HISTORY_H */
