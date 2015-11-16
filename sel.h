#ifndef VIMOL_SEL_H
#define VIMOL_SEL_H

/* selection of objects */
struct sel;

struct sel *sel_create(int);
struct sel *sel_copy(struct sel *);
void sel_free(struct sel *);
int sel_get_size(struct sel *);
int sel_get_count(struct sel *);
void sel_expand(struct sel *);
void sel_contract(struct sel *, int);
void sel_add(struct sel *, int);
void sel_remove(struct sel *, int);
void sel_swap(struct sel *, int, int);
void sel_all(struct sel *);
void sel_clear(struct sel *);
int sel_selected(struct sel *, int);
void sel_iter_start(struct sel *);
int sel_iter_next(struct sel *, int *);

#endif /* VIMOL_SEL_H */
