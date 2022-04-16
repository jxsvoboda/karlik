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

#ifndef MAP_H
#define MAP_H

#include <stdio.h>
#include "gfx.h"

typedef enum {
	/** Empty tile */
	mapt_none,
	/** Wall */
	mapt_wall,
	/** White tag */
	mapt_wtag,
	/** Grey tag */
	mapt_gtag,
	/** Black tag */
	mapt_btag,
	/** Robot */
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
	/** Margin to the left of each column */
	int margin_x;
	/** Margin above each row */
	int margin_y;
	/** Tile images */
	gfx_bmp_t **image;
	/** Number of images */
	int nimages;
} map_t;

extern int map_create(int, int, map_t **);
extern void map_destroy(map_t *);
extern void map_set_tile_size(map_t *, int, int);
extern void map_set_tile_margins(map_t *, int, int);
extern void map_set(map_t *, int, int, map_tile_t);
extern map_tile_t map_get(map_t *, int, int);
extern int map_load_tile_img(map_t *, const char **);
extern int map_load(FILE *, map_t **);
extern int map_save(map_t *, FILE *);
extern int map_tile_walkable(map_tile_t);

#endif
