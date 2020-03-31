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

#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <SDL.h>
#include <stdio.h>
#include "map.h"

typedef void (*mapview_cb_t)(void *arg, int x, int y);

/** Map view
 *
 * Displays a map and allows interacting with it
 */
typedef struct {
	/** Map */
	map_t *map;
	/** X coordinate of top-left corner on the screen */
	int orig_x;
	/** Y coordinate of top-left conrer on the screen */
	int orig_y;
	/** Called when user clicks on a map tile */
	mapview_cb_t cb;
	/** Callback argument */
	void *cb_arg;
} mapview_t;

extern int mapview_create(map_t *, mapview_t **);
extern void mapview_destroy(mapview_t *);
extern void mapview_set_orig(mapview_t *, int, int);
extern void mapview_set_cb(mapview_t *, mapview_cb_t, void *);
extern void mapview_draw(mapview_t *, gfx_t *);
extern bool mapview_event(mapview_t *, SDL_Event *);

#endif
