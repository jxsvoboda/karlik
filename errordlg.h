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

#ifndef ERRORDLG_H
#define ERRORDLG_H

#include <SDL.h>
#include <stdio.h>
#include "gfx.h"
#include "map.h"
#include "robots.h"

typedef void (*errordlg_cb_t)(void *arg);

/** Error dialog
 *
 * Displays an error dialog that requires the user's confirmation
 */
typedef struct {
	/** Error icon bitmap */
	gfx_bmp_t *bmp;
	/** X coordinate of top-left corner on the screen */
	int orig_x;
	/** Y coordinate of top-left conrer on the screen */
	int orig_y;
	/** Width in pixels */
	int width;
	/** Height in pixels */
	int height;
	/** Called when user clicks on the dialog */
	errordlg_cb_t cb;
	/** Callback argument */
	void *cb_arg;
} errordlg_t;

extern int errordlg_create(gfx_bmp_t *, errordlg_t **);
extern void errordlg_destroy(errordlg_t *);
extern void errordlg_set_dims(errordlg_t *, int, int, int, int);
extern void errordlg_set_cb(errordlg_t *, errordlg_cb_t, void *);
extern void errordlg_draw(errordlg_t *, gfx_t *);
extern bool errordlg_event(errordlg_t *, SDL_Event *);

#endif
