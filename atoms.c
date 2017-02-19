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

struct atoms {
	int frame;
	int nframes;
	int natoms;
	int *type;
	vec_t *xyz;
};

static const char *elementnames[] = {
	"X", "H", "He", "Li", "Be", "B", "C", "N", "O", "F", "Ne", "Na", "Mg",
	"Al", "Si", "P", "S", "Cl", "Ar", "K", "Ca", "Sc", "Ti", "V", "Cr",
	"Mn", "Fe", "Co", "Ni", "Cu", "Zn", "Ga", "Ge", "As", "Se", "Br", "Kr",
	"Rb", "Sr", "Y", "Zr", "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd",
	"In", "Sn", "Sb", "Te", "I", "Xe", "Cs", "Ba", "La", "Ce", "Pr", "Nd",
	"Pm", "Sm", "Eu", "Gd", "Tb", "Dy", "Ho", "Er", "Tm", "Yb", "Lu", "Hf",
	"Ta", "W", "Re", "Os", "Ir", "Pt", "Au", "Hg", "Tl", "Pb", "Bi", "Po",
	"At", "Rn", "Fr", "Ra", "Ac", "Th", "Pa", "U", "Np", "Pu", "Am", "Cm",
	"Bk", "Cf", "Es", "Fm", "Md", "No", "Lr"
};
static const size_t nelementnames = sizeof elementnames / sizeof *elementnames;

static int
atoms_name_to_type(const char *name)
{
	size_t i;

	for (i = 0; i < nelementnames; i++) {
		if (toupper(name[0]) == elementnames[i][0] &&
		    tolower(name[1]) == elementnames[i][1])
			return (i);
	}
	return (0);
}

struct atoms *
atoms_create(void)
{
	struct atoms *atoms;

	atoms = xcalloc(1, sizeof *atoms);
	atoms->nframes = 1;

	return (atoms);
}

struct atoms *
atoms_copy(struct atoms *atoms)
{
	struct atoms *copy;

	copy = xcalloc(1, sizeof *copy);
	copy->natoms = atoms->natoms;
	copy->nframes = atoms->nframes;
	copy->frame = atoms->frame;
	copy->type = xcalloc(copy->natoms, sizeof *copy->type);
	memcpy(copy->type, atoms->type, copy->natoms * sizeof *copy->type);
	copy->xyz = xcalloc(copy->natoms * copy->nframes, sizeof *copy->xyz);
	memcpy(copy->xyz, atoms->xyz, copy->natoms * copy->nframes *
	    sizeof *copy->xyz);

	return (copy);
}

void
atoms_free(struct atoms *atoms)
{
	if (atoms) {
		free(atoms->type);
		free(atoms->xyz);
		free(atoms);
	}
}

int
atoms_get_frame(struct atoms *atoms)
{
	return (atoms->frame);
}

void
atoms_set_frame(struct atoms *atoms, int frame)
{
	if (frame < 0)
		frame = 0;
	if (frame >= atoms->nframes)
		frame = atoms->nframes - 1;
	atoms->frame = frame;
}

int
atoms_get_frame_count(struct atoms *atoms)
{
	return (atoms->nframes);
}

void
atoms_add_frame(struct atoms *atoms)
{
	atoms->nframes++;
	atoms->xyz = xrealloc(atoms->xyz,
	    atoms->natoms * atoms->nframes * sizeof *atoms->xyz);

	if (atoms->frame < atoms->nframes - 2) {
		memmove(atoms->xyz + atoms->natoms * (atoms->frame + 2),
		    atoms->xyz + atoms->natoms * (atoms->frame + 1),
		    atoms->natoms * (atoms->nframes - 2 - atoms->frame) *
		    sizeof *atoms->xyz);
	}
	memcpy(atoms->xyz + atoms->natoms * (atoms->frame + 1),
	    atoms->xyz + atoms->natoms * atoms->frame,
	    atoms->natoms * sizeof *atoms->xyz);
	atoms->frame++;
}

void
atoms_add(struct atoms *atoms, const char *name, vec_t xyz)
{
	int i, j;

	atoms->natoms++;
	atoms->type = xrealloc(atoms->type,
	    atoms->natoms * sizeof *atoms->type);
	atoms->type[atoms->natoms - 1] = atoms_name_to_type(name);
	atoms->xyz = xrealloc(atoms->xyz,
	    atoms->natoms * atoms->nframes * sizeof *atoms->xyz);

	i = (atoms->natoms - 1) * atoms->nframes - 1;
	j = atoms->natoms * atoms->nframes - 1;
	while (j >= 0) {
		if (j % atoms->natoms == atoms->natoms - 1)
			atoms->xyz[j--] = xyz;
		else
			atoms->xyz[j--] = atoms->xyz[i--];
	}
}

void
atoms_remove(struct atoms *atoms, int idx)
{
	int i, j;

	assert(idx >= 0 && idx < atoms_get_count(atoms));

	for (i = idx; i < atoms->natoms - 1; i++)
		atoms->type[i] = atoms->type[i + 1];
	for (i = 0, j = 0; i < atoms->natoms * atoms->nframes; i++)
		if (i % atoms->natoms != idx)
			atoms->xyz[j++] = atoms->xyz[i];
	atoms->natoms--;
}

void
atoms_clear(struct atoms *atoms)
{
	atoms->natoms = 0;
	atoms->nframes = 1;
	atoms->frame = 0;
	free(atoms->type);
	atoms->type = NULL;
	free(atoms->xyz);
	atoms->xyz = NULL;
}

int
atoms_get_count(struct atoms *atoms)
{
	return (atoms->natoms);
}

const char *
atoms_get_name(struct atoms *atoms, int idx)
{
	assert(idx >= 0 && idx < atoms_get_count(atoms));

	return (elementnames[atoms->type[idx]]);
}

int
atoms_get_type(struct atoms *atoms, int idx)
{
	assert(idx >= 0 && idx < atoms_get_count(atoms));

	return (atoms->type[idx]);
}

void
atoms_set_name(struct atoms *atoms, int idx, const char *name)
{
	assert(idx >= 0 && idx < atoms_get_count(atoms));

	atoms->type[idx] = atoms_name_to_type(name);
}

vec_t
atoms_get_xyz(struct atoms *atoms, int idx)
{
	assert(idx >= 0 && idx < atoms_get_count(atoms));

	return (atoms->xyz[atoms->frame * atoms->natoms + idx]);
}

void
atoms_set_xyz(struct atoms *atoms, int idx, vec_t xyz)
{
	assert(idx >= 0 && idx < atoms_get_count(atoms));

	atoms->xyz[atoms->frame * atoms->natoms + idx] = xyz;
}
