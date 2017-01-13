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

struct atom {
	char *name;
	vec_t xyz;
};

struct atoms {
	int nelts, nalloc;
	struct atom *data;
};

struct atoms *
atoms_create(void)
{
	struct atoms *atoms;

	atoms = xcalloc(1, sizeof(struct atoms));
	atoms->nalloc = 8;
	atoms->data = xcalloc(atoms->nalloc, sizeof(struct atom));

	return (atoms);
}

struct atoms *
atoms_copy(struct atoms *atoms)
{
	struct atoms *copy;
	int i;

	copy = atoms_create();

	for (i = 0; i < atoms_get_count(atoms); i++)
		atoms_add(copy, atoms_get_name(atoms, i),
		    atoms_get_xyz(atoms, i));

	return (copy);
}

void
atoms_free(struct atoms *atoms)
{
	atoms_clear(atoms);
	free(atoms->data);
	free(atoms);
}

void
atoms_add(struct atoms *atoms, const char *name, vec_t xyz)
{
	struct atom atom;

	atom.name = xstrdup(name);
	atom.xyz = xyz;

	if (atoms->nelts == atoms->nalloc) {
		atoms->nalloc *= 2;
		atoms->data = xrealloc(atoms->data,
		    atoms->nalloc * sizeof(struct atom));
	}

	atoms->data[atoms->nelts++] = atom;
}

void
atoms_remove(struct atoms *atoms, int idx)
{
	util_assert(idx >= 0 && idx < atoms_get_count(atoms));

	free(atoms->data[idx].name);
	atoms->nelts--;
	memmove(atoms->data + idx, atoms->data + idx + 1,
	    (atoms->nelts - idx) * sizeof(struct atom));
}

void
atoms_swap(struct atoms *atoms, int i, int j)
{
	struct atom atom;

	util_assert(i >= 0 && i < atoms_get_count(atoms));
	util_assert(j >= 0 && j < atoms_get_count(atoms));

	atom = atoms->data[i];
	atoms->data[i] = atoms->data[j];
	atoms->data[j] = atom;
}

void
atoms_clear(struct atoms *atoms)
{
	int i;

	for (i = 0; i < atoms_get_count(atoms); i++)
		free(atoms->data[i].name);

	atoms->nelts = 0;
}

int
atoms_get_count(struct atoms *atoms)
{
	return (atoms->nelts);
}

const char *
atoms_get_name(struct atoms *atoms, int idx)
{
	util_assert(idx >= 0 && idx < atoms_get_count(atoms));

	return (atoms->data[idx].name);
}

void
atoms_set_name(struct atoms *atoms, int idx, const char *name)
{
	util_assert(idx >= 0 && idx < atoms_get_count(atoms));

	if (name != atoms->data[idx].name) {
		free(atoms->data[idx].name);
		atoms->data[idx].name = xstrdup(name);
	}
}

vec_t
atoms_get_xyz(struct atoms *atoms, int idx)
{
	util_assert(idx >= 0 && idx < atoms_get_count(atoms));

	return (atoms->data[idx].xyz);
}

void
atoms_set_xyz(struct atoms *atoms, int idx, vec_t xyz)
{
	util_assert(idx >= 0 && idx < atoms_get_count(atoms));

	atoms->data[idx].xyz = xyz;
}
