/*
 * Copyright 2020 Jiri Svoboda
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

#ifndef KARLIK_H
#define KARLIK_H

#include <SDL.h>
#include <stdbool.h>
#include "gfx.h"
#include "mapedit.h"
#include "toolbar.h"
#include "vocabed.h"

/** Selected mode */
typedef enum {
	/** Map editing mode */
	km_map,
	/** Vocabulary mode */
	km_vocab
} karlik_mode_t;

/** Karlik instance */
typedef struct {
	/** Selected tile type */
	karlik_mode_t kmode;
	/** Main toolbar */
	toolbar_t *main_tb;
	/** @c true to quit */
	bool quit;
	/** Graphics */
	gfx_t *gfx;
	/** Map */
	map_t *map;
	/** Map editor */
	mapedit_t *mapedit;
	/** Vocabulary editor */
	vocabed_t *vocabed;
} karlik_t;

extern int karlik_create(gfx_t *, karlik_t **);
extern void karlik_destroy(karlik_t *);
extern int karlik_save(karlik_t *);
extern void karlik_event(karlik_t *, SDL_Event *, gfx_t *);

#endif
