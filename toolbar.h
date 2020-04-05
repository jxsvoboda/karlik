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

#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <stdbool.h>
#include <SDL.h>
#include "gfx.h"

/** Toolbar callback.
 *
 * Called when toolbar button is selected. The first argument is provided
 * by the caller upon registration. The second argument is the button index
 * (starting with zero).
 */
typedef void (*toolbar_cb_t)(void *, int);

/** Toolbar */
typedef struct {
	/** Number of entries */
	int nentries;
	/** Icon for each entry */
	gfx_bmp_t **icon;
	/** Origin X coordinate */
	int orig_x;
	/** Origin Y coordinate */
	int orig_y;
	/** Selected entry */
	int sel;
	/** Callback */
	toolbar_cb_t cb;
	/** Callback argument */
	void *arg;
} toolbar_t;

extern int toolbar_create(const char **, toolbar_t **);
extern void toolbar_set_origin(toolbar_t *, int, int);
extern void toolbar_set_cb(toolbar_t *, toolbar_cb_t, void *);
extern void toolbar_destroy(toolbar_t *);
extern void toolbar_select(toolbar_t *, int);
extern void toolbar_draw(toolbar_t *, gfx_t *);
extern bool toolbar_event(toolbar_t *, SDL_Event *);

#endif
