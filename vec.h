/*
 * Copyright (c) 2013-2017 Ilya Kaliman
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef VIMOL_VEC_H
#define VIMOL_VEC_H

#define PI 3.14159265358979323846

typedef struct {
	double x, y;
} point_t;

typedef struct {
	double x, y, z;
} vec_t;

typedef struct {
	double xx, xy, xz, yx, yy, yz, zx, zy, zz;
} mat_t;

vec_t vec_zero(void);
vec_t vec_new(double, double, double);
vec_t vec_random(void);
vec_t vec_add(const vec_t *, const vec_t *);
vec_t vec_sub(const vec_t *, const vec_t *);
void vec_scale(vec_t *, double);
void vec_normalize(vec_t *);
double vec_len(const vec_t *);
double vec_lensq(const vec_t *);
double vec_dist(const vec_t *, const vec_t *);
double vec_distsq(const vec_t *, const vec_t *);
double vec_dot(const vec_t *, const vec_t *);
vec_t vec_cross(const vec_t *, const vec_t *);
double vec_angle(const vec_t *, const vec_t *, const vec_t *);
double vec_torsion(const vec_t *, const vec_t *, const vec_t *, const vec_t *);

mat_t mat_zero(void);
mat_t mat_identity(void);
mat_t mat_transpose(const mat_t *);
mat_t mat_rotation_x(double);
mat_t mat_rotation_y(double);
mat_t mat_rotation_z(double);
vec_t mat_vec(const mat_t *, const vec_t *);
mat_t mat_mat(const mat_t *, const mat_t *);
mat_t mat_align(const vec_t *, const vec_t *, const vec_t *);

#endif /* VIMOL_VEC_H */
