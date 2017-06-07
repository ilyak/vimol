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
	vec_t xyz;
	int i, j, k = 0, natoms = 0;
	char *buf = NULL, name[8];

	while ((buf = util_next_line(buf, fp)) != NULL) {
		if (strncasecmp(buf, "ATOM  ", 6) == 0 ||
		    strncasecmp(buf, "HETATM", 6) == 0) {
			if (strlen(buf) < 54) {
				free(buf);
				return (0);
			}
			memset(name, 0, sizeof name);
			xyz.x = 0, xyz.y = 0, xyz.z = 0;
			if (strlen(buf) >= 72) {
				for (i = 70, j = 0; i < 72; i++)
					if (isalpha(buf[i]))
						name[j++] = buf[i];
			}
			if (name[0] == '\0' && strlen(buf) >= 78) {
				for (i = 76, j = 0; i < 78; i++)
					if (isalpha(buf[i]))
						name[j++] = buf[i];
			}
			if (name[0] == '\0') {
				for (i = 12, j = 0; i < 14; i++)
					if (isalpha(buf[i]))
						name[j++] = buf[i];
			}
			if (name[0] == '\0')
				name[0] = 'X';
			if (sscanf(buf+30, "%lf%lf%lf",
			    &xyz.x, &xyz.y, &xyz.z) != 3) {
				free(buf);
				return (0);
			}
			if (natoms == 0)
				atoms_add(atoms, name, xyz);
			else {
				if (k >= natoms) {
					free(buf);
					return (0);
				}
				if (k == 0)
					atoms_add_frame(atoms);
				atoms_set_xyz(atoms, k++, xyz);
			}
		}
		if (strncasecmp(buf, "END", 3) == 0) {
			if ((natoms = atoms_get_count(atoms)) < 1) {
				free(buf);
				return (0);
			}
			k = 0;
		}
	}
	return (1);
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
	vec_t xyz;
	int i, natoms;
	char *buf = NULL, name[32];

	if ((buf = util_next_line(buf, fp)) == NULL)
		return (0);
	if (sscanf(buf, "%d", &natoms) != 1 || natoms < 1) {
		free(buf);
		return (0);
	}
	if ((buf = util_next_line(buf, fp)) == NULL)
		return (0);
	for (i = 0; i < natoms; i++) {
		if ((buf = util_next_line(buf, fp)) == NULL)
			return (0);
		memset(name, 0, sizeof name);
		xyz.x = 0, xyz.y = 0, xyz.z = 0;
		if (sscanf(buf, "%31s%lf%lf%lf", name,
		    &xyz.x, &xyz.y, &xyz.z) != 4) {
			free(buf);
			return (0);
		}
		atoms_add(atoms, name, xyz);
	}
	while ((buf = util_next_line(buf, fp)) != NULL) {
		if (string_is_whitespace(buf))
			continue;
		if ((buf = util_next_line(buf, fp)) == NULL)
			return (0);
		atoms_add_frame(atoms);
		for (i = 0; i < natoms; i++) {
			if ((buf = util_next_line(buf, fp)) == NULL)
				return (0);
			memset(name, 0, sizeof name);
			xyz.x = 0, xyz.y = 0, xyz.z = 0;
			if (sscanf(buf, "%31s%lf%lf%lf", name,
			    &xyz.x, &xyz.y, &xyz.z) != 4) {
				free(buf);
				return (0);
			}
			atoms_set_xyz(atoms, i, xyz);
		}
	}
	return (1);
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
				error_set("unexpected file content");
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

	for (i = 0; i < nformatlist; i++)
		if (string_has_suffix(path, formatlist[i].ext)) {
			if ((fp = fopen(path, "w")) == NULL) {
				error_set("%s", strerror(errno));
				return (0);
			}
			saveframe = atoms_get_frame(atoms);
			formatlist[i].savefn(atoms, fp);
			atoms_set_frame(atoms, saveframe);
			fclose(fp);
			return (1);
		}

	error_set("unknown file format");
	return (0);
}
