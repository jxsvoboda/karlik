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

#ifndef ROBOTS_H
#define ROBOTS_H

#include <stdio.h>
#include "adt/list.h"
#include "gfx.h"
#include "map.h"
#include "robot.h"

/** Robots */
typedef struct robots {
	/** Map used by robots */
	map_t *map;
	/** List of robots (robot_t) */
	list_t robots;
	/** Robot images */
	gfx_bmp_t **image;
	/** Number of images */
	int nimages;
	/** Tile width */
	int tile_w;
	/** Tile height */
	int tile_h;
	/** Relative position to map tile */
	int rel_x;
	/** Relative position to map tile */
	int rel_y;
} robots_t;

extern int robots_create(map_t *, robots_t **);
extern int robots_load(FILE *, map_t *, robots_t **);
extern int robots_save(robots_t *, FILE *);
extern void robots_destroy(robots_t *);
extern int robots_add(robots_t *, int, int);
extern void robots_remove(robots_t *, int, int);
extern robot_t *robots_get(robots_t *, int, int);
extern void robots_draw(robots_t *, int, int, gfx_t *);
extern int robots_load_img(robots_t *, int, int, int, const char **);
extern void robots_set_tile_size(robots_t *, int, int);
extern void robots_set_rel_pos(robots_t *, int, int);

#endif
