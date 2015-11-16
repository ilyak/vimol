#ifndef VIMOL_EDIT_H
#define VIMOL_EDIT_H

/* string edit */
struct edit;

struct edit *edit_create(void);
void edit_free(struct edit *);
const char *edit_get_text(struct edit *);
int edit_get_text_length(struct edit *);
int edit_get_pos(struct edit *);
void edit_set_pos(struct edit *, int);
void edit_set_text(struct edit *, const char *, ...);
void edit_insert_char(struct edit *, char);
void edit_insert_text(struct edit *, const char *);
void edit_backspace_char(struct edit *);
void edit_delete_char(struct edit *);
void edit_backspace_word(struct edit *);
void edit_clear(struct edit *);

#endif /* VIMOL_EDIT_H */
