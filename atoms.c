/*-
 * Copyright (c) 2013-2014 Ilya Kaliman
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
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

	atoms = calloc(1, sizeof(struct atoms));
	atoms->nalloc = 8;
	atoms->data = calloc(atoms->nalloc, sizeof(struct atom));

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
		atoms->data = realloc(atoms->data,
		    atoms->nalloc * sizeof(struct atom));
	}

	atoms->data[atoms->nelts++] = atom;
}

void
atoms_remove(struct atoms *atoms, int idx)
{
	assert(idx >= 0 && idx < atoms_get_count(atoms));

	free(atoms->data[idx].name);
	atoms->nelts--;
	memmove(atoms->data + idx, atoms->data + idx + 1,
	    (atoms->nelts - idx) * sizeof(struct atom));
}

void
atoms_swap(struct atoms *atoms, int i, int j)
{
	struct atom atom;

	assert(i >= 0 && i < atoms_get_count(atoms));
	assert(j >= 0 && j < atoms_get_count(atoms));

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
	assert(idx >= 0 && idx < atoms_get_count(atoms));

	return (atoms->data[idx].name);
}

void
atoms_set_name(struct atoms *atoms, int idx, const char *name)
{
	assert(idx >= 0 && idx < atoms_get_count(atoms));

	atoms->data[idx].name = xstrcpy(atoms->data[idx].name, name);
}

vec_t
atoms_get_xyz(struct atoms *atoms, int idx)
{
	assert(idx >= 0 && idx < atoms_get_count(atoms));

	return (atoms->data[idx].xyz);
}

void
atoms_set_xyz(struct atoms *atoms, int idx, vec_t xyz)
{
	assert(idx >= 0 && idx < atoms_get_count(atoms));

	atoms->data[idx].xyz = xyz;
}
