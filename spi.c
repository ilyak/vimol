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

struct cell {
	int nelts, nalloc;
	int *data;
};

struct spi {
	struct pairs *pairs;
	int npoints, npointsalloc;
	vec_t *points;
};

#define CIDX(x, y, z) ((x) * ny * nz + (y) * nz + (z))

static void
cell_add_idx(struct cell *cell, int idx)
{
	if (cell->nelts == cell->nalloc) {
		cell->nalloc *= 2;
		cell->data = xrealloc(cell->data,
		    cell->nalloc * sizeof *cell->data);
	}
	cell->data[cell->nelts] = idx;
	cell->nelts++;
}

static void
calc_box(struct spi *spi, vec_t *pmin, vec_t *pmax)
{
	vec_t xyz;
	int i;

	*pmin = vec_new( 1.0e100,  1.0e100,  1.0e100);
	*pmax = vec_new(-1.0e100, -1.0e100, -1.0e100);

	for (i = 0; i < spi_get_point_count(spi); i++) {
		xyz = spi_get_point(spi, i);

		if (xyz.x < pmin->x) pmin->x = xyz.x;
		if (xyz.y < pmin->y) pmin->y = xyz.y;
		if (xyz.z < pmin->z) pmin->z = xyz.z;
		if (xyz.x > pmax->x) pmax->x = xyz.x;
		if (xyz.y > pmax->y) pmax->y = xyz.y;
		if (xyz.z > pmax->z) pmax->z = xyz.z;
	}
}

static int
get_cell_count(double x, double dist)
{
	int n = (int)(x / dist);

	return (n > 16 ? 16 : n < 3 ? 3 : n);
}

static int
get_cell_idx(double x, double xmin, double xmax, int nx)
{
	int idx;

	if (xmax - xmin < 1.0e-6)
		return (1);

	idx = (int)((nx - 2) * (x - xmin) / (xmax - xmin));

	return (idx == nx - 2 ? idx : idx + 1);
}

static void
calc_self(struct spi *spi, double dist, struct cell *c1)
{
	double r2;
	vec_t xyz_i, xyz_j;
	int i, j;

	r2 = dist * dist;

	for (i = 0; i < c1->nelts; i++) {
		for (j = i + 1; j < c1->nelts; j++) {
			xyz_i = spi_get_point(spi, c1->data[i]);
			xyz_j = spi_get_point(spi, c1->data[j]);

			if (vec_distsq(&xyz_i, &xyz_j) < r2)
				pairs_add(spi->pairs, c1->data[i], c1->data[j]);
		}
	}
}

static void
calc_cell(struct spi *spi, double dist, struct cell *c1, struct cell *c2)
{
	double r2;
	vec_t xyz_i, xyz_j;
	int i, j;

	r2 = dist * dist;

	for (i = 0; i < c1->nelts; i++) {
		for (j = 0; j < c2->nelts; j++) {
			xyz_i = spi_get_point(spi, c1->data[i]);
			xyz_j = spi_get_point(spi, c2->data[j]);

			if (vec_distsq(&xyz_i, &xyz_j) < r2)
				pairs_add(spi->pairs, c1->data[i], c2->data[j]);
		}
	}
}

struct spi *
spi_create(void)
{
	struct spi *spi;

	spi = xcalloc(1, sizeof *spi);
	spi->npointsalloc = 8;
	spi->points = xcalloc(spi->npointsalloc, sizeof *spi->points);
	spi->pairs = pairs_create();

	return (spi);
}

void
spi_free(struct spi *spi)
{
	if (spi) {
		pairs_free(spi->pairs);
		free(spi->points);
		free(spi);
	}
}

void
spi_add_point(struct spi *spi, vec_t xyz)
{
	if (spi->npoints == spi->npointsalloc) {
		spi->npointsalloc *= 2;
		spi->points = xrealloc(spi->points,
		    spi->npointsalloc * sizeof *spi->points);
	}
	spi->points[spi->npoints] = xyz;
	spi->npoints++;
}

int
spi_get_point_count(struct spi *spi)
{
	return (spi->npoints);
}

vec_t
spi_get_point(struct spi *spi, int idx)
{
	assert(idx >= 0 && idx < spi_get_point_count(spi));

	return (spi->points[idx]);
}

void
spi_clear(struct spi *spi)
{
	spi->npoints = 0;
}

void
spi_compute(struct spi *spi, double dist)
{
	struct cell *cells;
	int ncells;
	vec_t pmin, pmax, xyz;
	int i, ix, iy, iz, nx, ny, nz, c1, c2;

	pairs_clear(spi->pairs);
	calc_box(spi, &pmin, &pmax);

	nx = get_cell_count(pmax.x - pmin.x, dist);
	ny = get_cell_count(pmax.y - pmin.y, dist);
	nz = get_cell_count(pmax.z - pmin.z, dist);

	ncells = nx * ny * nz;
	cells = xcalloc(ncells, sizeof *cells);

	for (i = 0; i < ncells; i++) {
		cells[i].nalloc = 8;
		cells[i].data = xcalloc(cells[i].nalloc, sizeof *cells->data);
	}

	for (i = 0; i < spi_get_point_count(spi); i++) {
		xyz = spi_get_point(spi, i);

		ix = get_cell_idx(xyz.x, pmin.x, pmax.x, nx);
		iy = get_cell_idx(xyz.y, pmin.y, pmax.y, ny);
		iz = get_cell_idx(xyz.z, pmin.z, pmax.z, nz);

		c1 = CIDX(ix, iy, iz);
		cell_add_idx(&cells[c1], i);
	}

	for (ix = 1; ix < nx - 1; ix++) {
		for (iy = 1; iy < ny - 1; iy++) {
			for (iz = 1; iz < nz - 1; iz++) {
				c1 = CIDX(ix, iy, iz);
				calc_self(spi, dist, &cells[c1]);
				c2 = CIDX(ix, iy, iz + 1);
				calc_cell(spi, dist, &cells[c1], &cells[c2]);
				c2 = CIDX(ix, iy + 1, iz - 1);
				calc_cell(spi, dist, &cells[c1], &cells[c2]);
				c2 = CIDX(ix, iy + 1, iz);
				calc_cell(spi, dist, &cells[c1], &cells[c2]);
				c2 = CIDX(ix, iy + 1, iz + 1);
				calc_cell(spi, dist, &cells[c1], &cells[c2]);
				c2 = CIDX(ix + 1, iy - 1, iz - 1);
				calc_cell(spi, dist, &cells[c1], &cells[c2]);
				c2 = CIDX(ix + 1, iy - 1, iz);
				calc_cell(spi, dist, &cells[c1], &cells[c2]);
				c2 = CIDX(ix + 1, iy - 1, iz + 1);
				calc_cell(spi, dist, &cells[c1], &cells[c2]);
				c2 = CIDX(ix + 1, iy, iz - 1);
				calc_cell(spi, dist, &cells[c1], &cells[c2]);
				c2 = CIDX(ix + 1, iy, iz);
				calc_cell(spi, dist, &cells[c1], &cells[c2]);
				c2 = CIDX(ix + 1, iy, iz + 1);
				calc_cell(spi, dist, &cells[c1], &cells[c2]);
				c2 = CIDX(ix + 1, iy + 1, iz - 1);
				calc_cell(spi, dist, &cells[c1], &cells[c2]);
				c2 = CIDX(ix + 1, iy + 1, iz);
				calc_cell(spi, dist, &cells[c1], &cells[c2]);
				c2 = CIDX(ix + 1, iy + 1, iz + 1);
				calc_cell(spi, dist, &cells[c1], &cells[c2]);
			}
		}
	}

	for (i = 0; i < ncells; i++)
		free(cells[i].data);
	free(cells);
}

int
spi_get_pair_count(struct spi *spi)
{
	return (pairs_get_count(spi->pairs));
}

struct pair
spi_get_pair(struct spi *spi, int idx)
{
	return (pairs_get(spi->pairs, idx));
}
