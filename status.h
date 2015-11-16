#ifndef VIMOL_STATUS_H
#define VIMOL_STATUS_H

/* status bar and command line */
struct status;

struct status *status_create(void);
void status_free(struct status *);
const char *status_get_text(struct status *);
void status_set_text(struct status *, const char *, ...);
void status_set_error(struct status *, const char *, ...);
void status_clear_text(struct status *);
const char *status_get_info_text(struct status *);
void status_set_info_text(struct status *, const char *, ...);
void status_clear_info_text(struct status *);
void status_set_cursor_pos(struct status *, int);
void status_render(struct status *, cairo_t *);

#endif /* VIMOL_STATUS_H */
