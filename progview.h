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

#ifndef PROGVIEW_H
#define PROGVIEW_H

#include <SDL.h>
#include <stdio.h>
#include "gfx.h"
#include "icondict.h"
#include "prog.h"

/** Program view callbacks */
typedef struct {
	void (*dummy)(void *arg);
} progview_cb_t;

/** Program view
 *
 * Displays a procedure, highlights the current statement. Future:
 * editing.
 */
typedef struct {
	/** Program procedure */
	prog_proc_t *proc;
	/** Highlighted statement */
	prog_stmt_t *hgl_stmt;
	/** X coordinate of top-left corner on the screen */
	int orig_x;
	/** Y coordinate of top-left corner on the screen */
	int orig_y;
	/** Icon width */
	int icon_w;
	/** Icon height */
	int icon_h;
	/** Horizontal spacing around icons */
	int margin_x;
	/** Vertical spacing around icons */
	int margin_y;
	/** Intrinsic icons */
	gfx_bmp_t *intr_img[progin_limit];
	/** Procedure icons */
	icondict_t *icondict;
	/** Callbacks */
	progview_cb_t *cb;
	/** Callback argument */
	void *cb_arg;
} progview_t;

extern int progview_create(icondict_t *, progview_t **);
extern void progview_destroy(progview_t *);
extern void progview_set_orig(progview_t *, int, int);
extern void progview_set_cb(progview_t *, progview_cb_t *, void *);
extern void progview_set_proc(progview_t *, prog_proc_t *);
extern void progview_set_hgl_stmt(progview_t *, prog_stmt_t *);
extern prog_proc_t *progview_get_proc(progview_t *);
extern void progview_draw(progview_t *, gfx_t *);
extern bool progview_event(progview_t *, SDL_Event *);

#endif
