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

#include "vimol.h"

vec_t
vec_zero(void)
{
	vec_t vec;

	vec.x = 0.0;
	vec.y = 0.0;
	vec.z = 0.0;

	return (vec);
}

vec_t
vec_new(double x, double y, double z)
{
	vec_t vec;

	vec.x = x;
	vec.y = y;
	vec.z = z;

	return (vec);
}

vec_t
vec_random(void)
{
	vec_t vec;
	double rho, phi;

	vec.z = 2.0 * drand48() - 1.0;
	rho = sqrt(1.0 - vec.z * vec.z);
	phi = 2.0 * PI * drand48();
	vec.x = rho * cos(phi);
	vec.y = rho * sin(phi);

	return (vec);
}

vec_t
vec_add(const vec_t *a, const vec_t *b)
{
	vec_t vec;

	vec.x = a->x + b->x;
	vec.y = a->y + b->y;
	vec.z = a->z + b->z;

	return (vec);
}

vec_t
vec_sub(const vec_t *a, const vec_t *b)
{
	vec_t vec;

	vec.x = a->x - b->x;
	vec.y = a->y - b->y;
	vec.z = a->z - b->z;

	return (vec);
}

void
vec_scale(vec_t *vec, double s)
{
	vec->x *= s;
	vec->y *= s;
	vec->z *= s;
}

void
vec_normalize(vec_t *vec)
{
	vec_scale(vec, 1.0 / vec_len(vec));
}

double
vec_len(const vec_t *a)
{
	return (sqrt(vec_lensq(a)));
}

double
vec_lensq(const vec_t *a)
{
	return (a->x * a->x + a->y * a->y + a->z * a->z);
}

double
vec_dist(const vec_t *a, const vec_t *b)
{
	return (sqrt(vec_distsq(a, b)));
}

double
vec_distsq(const vec_t *a, const vec_t *b)
{
	vec_t vec;

	vec.x = a->x - b->x;
	vec.y = a->y - b->y;
	vec.z = a->z - b->z;

	return (vec_lensq(&vec));
}

double
vec_dot(const vec_t *a, const vec_t *b)
{
	return (a->x * b->x + a->y * b->y + a->z * b->z);
}

vec_t
vec_cross(const vec_t *a, const vec_t *b)
{
	vec_t vec;

	vec.x = a->y * b->z - a->z * b->y;
	vec.y = a->z * b->x - a->x * b->z;
	vec.z = a->x * b->y - a->y * b->x;

	return (vec);
}

double
vec_angle(const vec_t *a, const vec_t *b, const vec_t *c)
{
	vec_t ab = vec_sub(a, b);
	vec_t cb = vec_sub(c, b);

	double r2ab = vec_lensq(&ab);
	double r2cb = vec_lensq(&cb);

	return (acos(vec_dot(&ab, &cb) / sqrt(r2ab * r2cb)));
}

double
vec_torsion(const vec_t *a, const vec_t *b, const vec_t *c, const vec_t *d)
{
	vec_t ba = vec_sub(b, a);
	vec_t cb = vec_sub(c, b);
	vec_t dc = vec_sub(d, c);

	vec_t t = vec_cross(&ba, &cb);
	vec_t u = vec_cross(&cb, &dc);

	double r2t = vec_lensq(&t);
	double r2u = vec_lensq(&u);

	return (acos(vec_dot(&t, &u) / sqrt(r2t * r2u)));
}

mat_t
mat_zero(void)
{
	mat_t mat;

	mat.xx = 0.0;
	mat.xy = 0.0;
	mat.xz = 0.0;
	mat.yx = 0.0;
	mat.yy = 0.0;
	mat.yz = 0.0;
	mat.zx = 0.0;
	mat.zy = 0.0;
	mat.zz = 0.0;

	return (mat);
}

mat_t
mat_identity(void)
{
	mat_t mat;

	mat.xx = 1.0;
	mat.xy = 0.0;
	mat.xz = 0.0;
	mat.yx = 0.0;
	mat.yy = 1.0;
	mat.yz = 0.0;
	mat.zx = 0.0;
	mat.zy = 0.0;
	mat.zz = 1.0;

	return (mat);
}

mat_t
mat_transpose(const mat_t *mat)
{
	mat_t out;

	out.xx = mat->xx;
	out.xy = mat->yx;
	out.xz = mat->zx;
	out.yx = mat->xy;
	out.yy = mat->yy;
	out.yz = mat->zy;
	out.zx = mat->xz;
	out.zy = mat->yz;
	out.zz = mat->zz;

	return (out);
}

mat_t
mat_rotation_x(double angle)
{
	mat_t mat;

	mat.xx = 1.0;
	mat.xy = 0.0;
	mat.xz = 0.0;
	mat.yx = 0.0;
	mat.yy = cos(angle);
	mat.yz = -sin(angle);
	mat.zx = 0.0;
	mat.zy = sin(angle);
	mat.zz = cos(angle);

	return (mat);
}

mat_t
mat_rotation_y(double angle)
{
	mat_t mat;

	mat.xx = cos(angle);
	mat.xy = 0.0;
	mat.xz = sin(angle);
	mat.yx = 0.0;
	mat.yy = 1.0;
	mat.yz = 0.0;
	mat.zx = -sin(angle);
	mat.zy = 0.0;
	mat.zz = cos(angle);

	return (mat);
}

mat_t
mat_rotation_z(double angle)
{
	mat_t mat;

	mat.xx = cos(angle);
	mat.xy = -sin(angle);
	mat.xz = 0.0;
	mat.yx = sin(angle);
	mat.yy = cos(angle);
	mat.yz = 0.0;
	mat.zx = 0.0;
	mat.zy = 0.0;
	mat.zz = 1.0;

	return (mat);
}

vec_t
mat_vec(const mat_t *mat, const vec_t *vec)
{
	vec_t out;

	out.x = mat->xx * vec->x + mat->xy * vec->y + mat->xz * vec->z;
	out.y = mat->yx * vec->x + mat->yy * vec->y + mat->yz * vec->z;
	out.z = mat->zx * vec->x + mat->zy * vec->y + mat->zz * vec->z;

	return (out);
}

mat_t
mat_mat(const mat_t *m1, const mat_t *m2)
{
	mat_t out;

	out.xx = m1->xx * m2->xx + m1->xy * m2->yx + m1->xz * m2->zx;
	out.xy = m1->xx * m2->xy + m1->xy * m2->yy + m1->xz * m2->zy;
	out.xz = m1->xx * m2->xz + m1->xy * m2->yz + m1->xz * m2->zz;
	out.yx = m1->yx * m2->xx + m1->yy * m2->yx + m1->yz * m2->zx;
	out.yy = m1->yx * m2->xy + m1->yy * m2->yy + m1->yz * m2->zy;
	out.yz = m1->yx * m2->xz + m1->yy * m2->yz + m1->yz * m2->zz;
	out.zx = m1->zx * m2->xx + m1->zy * m2->yx + m1->zz * m2->zx;
	out.zy = m1->zx * m2->xy + m1->zy * m2->yy + m1->zz * m2->zy;
	out.zz = m1->zx * m2->xz + m1->zy * m2->yz + m1->zz * m2->zz;

	return (out);
}

mat_t
mat_align(const vec_t *p1, const vec_t *p2, const vec_t *p3)
{
	mat_t rotmat;
	vec_t cross, r12, r13;
	double dot;

	r12 = vec_sub(p2, p1);
	r13 = vec_sub(p3, p1);

	vec_normalize(&r12);
	vec_normalize(&r13);

	dot = vec_dot(&r12, &r13);

	r13.x -= dot * r12.x;
	r13.y -= dot * r12.y;
	r13.z -= dot * r12.z;

	cross = vec_cross(&r12, &r13);

	vec_normalize(&r13);
	vec_normalize(&cross);

	rotmat.xx = r12.x, rotmat.xy = r13.x, rotmat.xz = cross.x;
	rotmat.yx = r12.y, rotmat.yy = r13.y, rotmat.yz = cross.y;
	rotmat.zx = r12.z, rotmat.zy = r13.z, rotmat.zz = cross.z;

	return (rotmat);
}
