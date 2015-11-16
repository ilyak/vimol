#ifndef VIMOL_CAMERA_H
#define VIMOL_CAMERA_H

/* an eye of a user */
struct camera;

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

#endif /* VIMOL_CAMERA_H */
