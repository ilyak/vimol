#ifndef VIMOL_SYS_H
#define VIMOL_SYS_H

/* molecular system info */
struct sys;

struct sys *sys_create(const char *);
struct sys *sys_copy(struct sys *);
void sys_free(struct sys *);
struct graph *sys_get_graph(struct sys *);
struct sel *sys_get_sel(struct sys *);
struct sel *sys_get_visible(struct sys *);
int sys_read(struct sys *, const char *);
int sys_is_modified(struct sys *);
int sys_get_frame(struct sys *);
void sys_set_frame(struct sys *, int);
int sys_get_frame_count(struct sys *);
void sys_add_atom(struct sys *, const char *, vec_t);
void sys_remove_atom(struct sys *, int);
void sys_swap_atoms(struct sys *, int, int);
int sys_get_atom_count(struct sys *);
const char *sys_get_atom_name(struct sys *, int);
void sys_set_atom_name(struct sys *, int, const char *);
vec_t sys_get_atom_xyz(struct sys *, int);
void sys_set_atom_xyz(struct sys *, int, vec_t);
void sys_add_hydrogens(struct sys *, struct sel *);
vec_t sys_get_sel_center(struct sys *, struct sel *);
void sys_reset_bonds(struct sys *, struct sel *);
int sys_save_to_file(struct sys *, const char *);

#endif /* VIMOL_SYS_H */
