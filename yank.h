#ifndef VIMOL_YANK_H
#define VIMOL_YANK_H

/* number of registers */
#define YANK_SIZE 26

/* copy-paste buffer */
struct yank;

struct yank *yank_create(void);
void yank_free(struct yank *);
void yank_set_register(struct yank *, int);
int yank_get_register(struct yank *);
void yank_copy(struct yank *, struct sys *, struct sel *);
void yank_paste(struct yank *, struct sys *);

#endif /* VIMOL_YANK_H */
