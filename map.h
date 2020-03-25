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

#ifndef MAP_H
#define MAP_H

#include <stdio.h>

typedef void (*map_cb_t)(void *arg, int x, int y);

typedef enum {
	mapt_none,
	mapt_wall,
	mapt_wtag,
	mapt_gtag,
	mapt_btag,
	mapt_robot
} map_tile_t;

/** City map */
typedef struct {
	/** Width in tiles */
	int width;
	/** Height in tiles */
	int height;
	/** Tiles (an array of columns) */
	map_tile_t **tile;
	/** Tile width */
	int tile_w;
	/** Tile height */
	int tile_h;
	/** X coordinate of top-left corner on the screen */
	int orig_x;
	/** Y coordinate of top-left conrer on the screen */
	int orig_y;
	/** Margin to the left of each column */
	int margin_x;
	/** Margin above each row */
	int margin_y;
	/** Called when user clicks on a map tile */
	map_cb_t cb;
	/** Callback argument */
	void *cb_arg;
} map_t;

extern int map_create(int, int, map_t **);
extern void map_destroy(map_t *);
extern void map_set_orig(map_t *, int, int);
extern void map_set_tile_size(map_t *, int, int);
extern void map_set_tile_margins(map_t *, int, int);
extern void map_set_cb(map_t *, map_cb_t, void *);
extern int map_load(FILE *, map_t **);
extern int map_save(map_t *, FILE *);
extern void map_draw(map_t *, gfx_t *);
extern bool map_event(map_t *, SDL_Event *);

#endif
