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

struct camera {
	mat_t rotation;
	vec_t translation;
	double scale;       /* pixels per angstrom */
	double radius;      /* visible radius */
};

struct camera *
camera_create(void)
{
	struct camera *camera;

	camera = xcalloc(1, sizeof *camera);
	camera_reset(camera);

	return (camera);
}

void
camera_free(struct camera *camera)
{
	free(camera);
}

void
camera_reset(struct camera *camera)
{
	camera->rotation = mat_identity();
	camera->translation = vec_zero();
	camera->scale = 50.0;
	camera->radius = 10.0;
}

void
camera_move(struct camera *camera, vec_t xyz)
{
	camera->translation.x += xyz.x * camera->radius;
	camera->translation.y += xyz.y * camera->radius;
	camera->translation.z += xyz.z * camera->radius;
}

void
camera_move_to(struct camera *camera, vec_t xyz)
{
	camera->translation = mat_vec(&camera->rotation, &xyz);
	vec_scale(&camera->translation, -1.0);
}

void
camera_rotate(struct camera *camera, vec_t xyz)
{
	mat_t rotmat;

	rotmat = mat_rotation_x(xyz.x);
	camera->rotation = mat_mat(&rotmat, &camera->rotation);
	camera->translation = mat_vec(&rotmat, &camera->translation);

	rotmat = mat_rotation_y(xyz.y);
	camera->rotation = mat_mat(&rotmat, &camera->rotation);
	camera->translation = mat_vec(&rotmat, &camera->translation);

	rotmat = mat_rotation_z(xyz.z);
	camera->rotation = mat_mat(&rotmat, &camera->rotation);
	camera->translation = mat_vec(&rotmat, &camera->translation);
}

mat_t
camera_get_rotation(struct camera *camera)
{
	return (camera->rotation);
}

void
camera_set_scale(struct camera *camera, double scale)
{
	if (scale > 0.0)
		camera->scale = scale;
}

double
camera_get_scale(struct camera *camera)
{
	return (camera->scale);
}

void
camera_set_radius(struct camera *camera, double radius)
{
	if (radius > 0.01 && radius < 160.0) /* font breaks on small zoom */
		camera->radius = radius;
}

double
camera_get_radius(struct camera *camera)
{
	return (camera->radius);
}

double
camera_get_zoom(struct camera *camera, int width, int height)
{
	return (0.45 * (width < height ? width : height) /
	    camera->radius / camera->scale);
}

point_t
camera_transform(struct camera *camera, vec_t xyz)
{
	point_t point;

	point.x = (camera->rotation.xx * xyz.x +
		   camera->rotation.xy * xyz.y +
		   camera->rotation.xz * xyz.z +
		   camera->translation.x) * camera->scale;

	point.y = (camera->rotation.yx * xyz.x +
		   camera->rotation.yy * xyz.y +
		   camera->rotation.yz * xyz.z +
		   camera->translation.y) * camera->scale;

	return (point);
}
