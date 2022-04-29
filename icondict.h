/*
 * Copyright 2022 Jiri Svoboda
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef ICONDICT_H
#define ICONDICT_H

#include <stdbool.h>
#include <stdio.h>
#include "adt/list.h"
#include "gfx.h"

/** Icon dictionary */
typedef struct {
	/** Containing icon dictionary */
	struct icondict *icondict;
	/** Link to @c wlist->entries */
	link_t lidict;
	/** Identifier */
	char *ident;
	/** Icon */
	gfx_bmp_t *icon;
} icondict_entry_t;

/** Icon dictionary */
typedef struct icondict {
	/** Entries (icondict_entry_t) */
	list_t entries;
} icondict_t;

extern int icondict_create(icondict_t **);
extern void icondict_destroy(icondict_t *);
extern int icondict_add(icondict_t *, const char *, gfx_bmp_t *);
extern void icondict_remove(icondict_entry_t *);
extern icondict_entry_t *icondict_first(icondict_t *);
extern icondict_entry_t *icondict_next(icondict_entry_t *);
extern icondict_entry_t *icondict_find(icondict_t *, const char *);
extern int icondict_load(FILE *, icondict_t **);
extern int icondict_save(icondict_t *, FILE *);

#endif
