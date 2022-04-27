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

#ifndef ICONDLG_H
#define ICONDLG_H

#include <SDL.h>
#include <stdio.h>
#include "canvas.h"
#include "gfx.h"
#include "map.h"
#include "robots.h"

/** Icon dialog callbacks */
typedef struct {
	void (*accept)(void *);
	void (*repaint)(void *);
} icondlg_cb_t;

/** Icon dialog
 *
 * Allows the user to select from a list of existing icons, to modify
 * it and finally to submit it.
 */
typedef struct {
	/** Error icon bitmap */
	gfx_bmp_t *bmp;
	/** Canvas */
	canvas_t *canvas;
	/** X coordinate of top-left corner on the screen */
	int orig_x;
	/** Y coordinate of top-left corner on the screen */
	int orig_y;
	/** Width in pixels */
	int width;
	/** Height in pixels */
	int height;
	/** Callbacks */
	icondlg_cb_t *cb;
	/** Callback argument */
	void *cb_arg;
} icondlg_t;

extern int icondlg_create(gfx_bmp_t *, icondlg_t **);
extern void icondlg_destroy(icondlg_t *);
extern void icondlg_set_dims(icondlg_t *, int, int, int, int);
extern void icondlg_set_cb(icondlg_t *, icondlg_cb_t *, void *);
extern void icondlg_draw(icondlg_t *, gfx_t *);
extern bool icondlg_event(icondlg_t *, SDL_Event *);

#endif
