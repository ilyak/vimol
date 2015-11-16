#ifndef VIMOL_UNDO_H
#define VIMOL_UNDO_H

/* undo/redo management */
struct undo;

struct undo *undo_create(void *, void *(*)(void *), void (*)(void *));
void undo_free(struct undo *);
void *undo_get_data(struct undo *);
void undo_snapshot(struct undo *);
int undo_undo(struct undo *);
int undo_redo(struct undo *);

#endif /* VIMOL_UNDO_H */
