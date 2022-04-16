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

#ifndef VOCABED_H
#define VOCABED_H

#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include "gfx.h"
#include "mapview.h"
#include "robots.h"
#include "toolbar.h"
#include "wordlist.h"

typedef struct {
	void (*repaint)(void *);
} vocabed_cb_t;

/** Map editor */
typedef struct {
	/** Map view */
	mapview_t *mapview;
	/** Verb view */
	wordlist_t *verbs;
	/** Robots */
	robots_t *robots;
	/** Callbacks */
	vocabed_cb_t *cb;
	/** Callback argument */
	void *arg;
} vocabed_t;

extern int vocabed_new(map_t *, robots_t *, vocabed_cb_t *, void *,
    vocabed_t **);
extern int vocabed_load(map_t *, robots_t *, FILE *, vocabed_cb_t *, void *,
    vocabed_t **);
extern void vocabed_destroy(vocabed_t *);
extern void vocabed_display(vocabed_t *, gfx_t *gfx);
extern int vocabed_save(vocabed_t *, FILE *);
extern void vocabed_event(vocabed_t *, SDL_Event *, gfx_t *);

#endif
