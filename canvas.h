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

#ifndef CANVAS_H
#define CANVAS_H

#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include "gfx.h"

/** Canvas callbacks */
typedef struct {
	void (*repaint)(void *arg);
} canvas_cb_t;

/** Canvas
 *
 * Display / draw into a bitmap
 */
typedef struct {
	/** Bitmap */
	gfx_bmp_t *bmp;
	/** X coordinate of top-left corner on the screen */
	int orig_x;
	/** Y coordinate of top-left corner on the screen */
	int orig_y;
	/** Magnification */
	int mag;
	/** Mouse button held */
	bool button_held;
	/** Callbacks */
	canvas_cb_t *cb;
	/** Callback argument */
	void *cb_arg;
} canvas_t;

extern int canvas_create(gfx_bmp_t *, canvas_t **);
extern void canvas_destroy(canvas_t *);
extern void canvas_set_orig(canvas_t *, int, int);
extern void canvas_set_mag(canvas_t *, int);
extern void canvas_set_cb(canvas_t *, canvas_cb_t *, void *);
extern void canvas_draw(canvas_t *, gfx_t *);
extern bool canvas_event(canvas_t *, SDL_Event *);

#endif
