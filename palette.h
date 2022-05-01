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

#ifndef PALETTE_H
#define PALETTE_H

#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include "gfx.h"

enum {
	pal_cols = 8,
	pal_rows = 2,
	pal_num_entries = pal_cols * pal_rows
};

/** Palette callbacks */
typedef struct {
	void (*selected)(void *arg, int);
} palette_cb_t;

/** Palette entry */
typedef struct {
	uint8_t r, g, b;
} palette_entry_t;

/** Palette
 *
 * Palette control for selecting color
 */
typedef struct {
	/** Color entries */
	palette_entry_t entry[pal_num_entries];
	/** X coordinate of top-left corner on the screen */
	int orig_x;
	/** Y coordinate of top-left corner on the screen */
	int orig_y;
	/** Palette entry width */
	int entry_w;
	/** Palette entry height */
	int entry_h;
	/** Index of selected entry */
	int sel_idx;
	/** Callbacks */
	palette_cb_t *cb;
	/** Callback argument */
	void *cb_arg;
} palette_t;

extern int palette_create(palette_t **);
extern void palette_destroy(palette_t *);
extern void palette_set_orig(palette_t *, int, int);
extern void palette_set_entry_dims(palette_t *, int, int);
extern void palette_set_entry_color(palette_t *, int, uint8_t, uint8_t, uint8_t);
extern void palette_set_cb(palette_t *, palette_cb_t *, void *);
extern void palette_draw(palette_t *, gfx_t *);
extern bool palette_event(palette_t *, SDL_Event *);

#endif
