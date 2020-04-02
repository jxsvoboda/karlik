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

#ifndef MAPEDIT_H
#define MAPEDIT_H

#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include "gfx.h"
#include "mapview.h"
#include "robots.h"
#include "toolbar.h"

typedef struct {
	void (*repaint)(void *);
} mapedit_cb_t;

/** Map editor */
typedef struct {
	/** Map view */
	mapview_t *mapview;
	/** Robots */
	robots_t *robots;
	/** Selected tile type */
	map_tile_t ttype;
	/** Map editor toolbar */
	toolbar_t *map_tb;
	/** @c true to quit */
	bool quit;
	/** Callbacks */
	mapedit_cb_t *cb;
	/** Callback argument */
	void *arg;
} mapedit_t;

extern int mapedit_new(map_t *, robots_t *, mapedit_cb_t *, void *,
    mapedit_t **);
extern int mapedit_load(map_t *, robots_t *, FILE *, mapedit_cb_t *, void *,
    mapedit_t **);
extern void mapedit_destroy(mapedit_t *);
extern void mapedit_display(mapedit_t *, gfx_t *gfx);
extern int mapedit_save(mapedit_t *, FILE *);
extern void mapedit_event(mapedit_t *, SDL_Event *, gfx_t *);

#endif
