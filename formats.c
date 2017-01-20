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

#define PDBFMT "ATOM  %5d%3s                %8.3lf%8.3lf%8.3lf"
#define XYZFMT "%-4s %11.6lf %11.6lf %11.6lf"

static int
load_from_pdb(struct atoms *atoms, FILE *fp)
{
	return (0);
}

static void
save_to_pdb(struct atoms *atoms, FILE *fp)
{
	vec_t xyz;
	int i, j, natoms, nframes;
	const char *name;

	natoms = atoms_get_count(atoms);
	nframes = atoms_get_frame_count(atoms);

	for (i = 0; i < nframes; i++) {
		atoms_set_frame(atoms, i);
		for (j = 0; j < natoms; j++) {
			name = atoms_get_name(atoms, j);
			xyz = atoms_get_xyz(atoms, j);
			fprintf(fp, PDBFMT, j+1, name, xyz.x, xyz.y, xyz.z);
			fprintf(fp, "\n");
		}
		fprintf(fp, "END\n");
	}
}

static int
load_from_xyz(struct atoms *atoms, FILE *fp)
{
	return (0);
}

static void
save_to_xyz(struct atoms *atoms, FILE *fp)
{
	vec_t xyz;
	int i, j, natoms, nframes;
	const char *name;

	natoms = atoms_get_count(atoms);
	nframes = atoms_get_frame_count(atoms);

	for (i = 0; i < nframes; i++) {
		atoms_set_frame(atoms, i);
		fprintf(fp, "%d\n\n", natoms);
		for (j = 0; j < natoms; j++) {
			name = atoms_get_name(atoms, j);
			xyz = atoms_get_xyz(atoms, j);
			fprintf(fp, XYZFMT, name, xyz.x, xyz.y, xyz.z);
			fprintf(fp, "\n");
		}
	}
}

typedef int (*loadfn_t)(struct atoms *, FILE *);
typedef void (*savefn_t)(struct atoms *, FILE *);

static const struct {
	const char *ext;
	loadfn_t loadfn;
	savefn_t savefn;
} formatlist[] = {
	{ ".pdb", load_from_pdb, save_to_pdb },
	{ ".xyz", load_from_xyz, save_to_xyz },
};
static const size_t nformatlist = sizeof formatlist / sizeof *formatlist;

struct atoms *
formats_load(const char *path)
{
	FILE *fp;
	struct atoms *atoms;
	size_t i;

	if ((fp = fopen(path, "r")) == NULL) {
		error_set("%s", strerror(errno));
		return (NULL);
	}

	atoms = atoms_create();

	for (i = 0; i < nformatlist; i++)
		if (string_has_suffix(path, formatlist[i].ext)) {
			if (formatlist[i].loadfn(atoms, fp)) {
				fclose(fp);
				atoms_set_frame(atoms, 0);
				return (atoms);
			} else {
				atoms_free(atoms);
				fclose(fp);
				return (NULL);
			}
		}

	atoms_free(atoms);
	fclose(fp);
	error_set("unknown file format");
	return (NULL);
}

int
formats_save(struct atoms *atoms, const char *path)
{
	FILE *fp;
	size_t i;
	int saveframe;

	if ((fp = fopen(path, "w")) == NULL) {
		error_set("%s", strerror(errno));
		return (0);
	}

	saveframe = atoms_get_frame(atoms);

	for (i = 0; i < nformatlist; i++)
		if (string_has_suffix(path, formatlist[i].ext)) {
			formatlist[i].savefn(atoms, fp);
			fclose(fp);
			atoms_set_frame(atoms, saveframe);
			return (1);
		}

	fclose(fp);
	error_set("unknown file format");
	return (0);
}
