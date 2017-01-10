#ifndef VIMOL_VIMOL_H
#define VIMOL_VIMOL_H

#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <SDL.h>
#include <cairo.h>

#define strcasecmp SDL_strcasecmp
#define strncasecmp SDL_strncasecmp
#define snprintf SDL_snprintf
#define vsnprintf SDL_vsnprintf

#ifndef __unused
#define __unused
#endif /* __unused */

#ifndef __dead
#define __dead
#endif /* __dead */

/* number of yank registers */
#define YANK_SIZE 26

#include "color.h"
#include "vec.h"
#include "tok.h"
#include "sel.h"
#include "sys.h"
#include "edit.h"
#include "error.h"
#include "state.h"
#include "cmd.h"
#include "exec.h"
#include "graph.h"
#include "history.h"
#include "log.h"
#include "pair.h"
#include "platform.h"
#include "rec.h"
#include "settings.h"
#include "spi.h"
#include "status.h"
#include "undo.h"
#include "util.h"
#include "view.h"
#include "wnd.h"
#include "xmalloc.h"

struct alias;   /* alias management */
struct atoms;   /* atom storage */
struct camera;  /* an eye of a user */
struct yank;    /* copy-paste buffer */

/* alias.c */
struct alias *alias_create(void);
void alias_free(struct alias *);
int alias_set(struct alias *, const char *, const char *);
const char *alias_get(struct alias *, const char *);

/* atoms.c */
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

/* camera.c */
struct camera *camera_create(void);
void camera_free(struct camera *);
void camera_reset(struct camera *);
void camera_move(struct camera *, vec_t);
void camera_move_to(struct camera *, vec_t);
void camera_rotate(struct camera *, vec_t);
mat_t camera_get_rotation(struct camera *);
void camera_set_scale(struct camera *, double);
double camera_get_scale(struct camera *);
void camera_set_radius(struct camera *, double);
double camera_get_radius(struct camera *);
double camera_get_zoom(struct camera *, int, int);
point_t camera_transform(struct camera *, vec_t);

/* yank.c */
struct yank *yank_create(void);
void yank_free(struct yank *);
void yank_set_register(struct yank *, int);
int yank_get_register(struct yank *);
void yank_copy(struct yank *, struct sys *, struct sel *);
void yank_paste(struct yank *, struct sys *);

#endif /* VIMOL_VIMOL_H */
