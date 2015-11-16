#ifndef VIMOL_ATOMS_H
#define VIMOL_ATOMS_H

/* atom storage */
struct atoms;

struct atoms *atoms_create(void);
struct atoms *atoms_copy(struct atoms *);
void atoms_free(struct atoms *);
void atoms_add(struct atoms *, const char *, vec_t);
void atoms_remove(struct atoms *, int);
void atoms_swap(struct atoms *, int, int);
void atoms_clear(struct atoms *);
int atoms_get_count(struct atoms *);
const char *atoms_get_name(struct atoms *, int);
void atoms_set_name(struct atoms *, int, const char *);
vec_t atoms_get_xyz(struct atoms *, int);
void atoms_set_xyz(struct atoms *, int, vec_t);

#endif /* VIMOL_ATOMS_H */
