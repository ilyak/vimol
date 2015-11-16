#ifndef VIMOL_REC_H
#define VIMOL_REC_H

/* number of registers */
#define REC_SIZE 26

/* command recording */
struct rec;

struct rec *rec_create(void);
void rec_free(struct rec *);
int rec_is_playing(struct rec *);
int rec_is_recording(struct rec *);
int rec_get_register(struct rec *);
void rec_set_register(struct rec *, int);
void rec_load(struct rec *, const char *);
void rec_save(struct rec *, const char *);
void rec_start(struct rec *);
void rec_add(struct rec *, const char *);
void rec_stop(struct rec *);
int rec_play(struct rec *, struct state *);

#endif /* VIMOL_REC_H */
