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
#include "errordlg.h"
#include "gfx.h"
#include "icondict.h"
#include "icondlg.h"
#include "mapview.h"
#include "prog.h"
#include "progview.h"
#include "robots.h"
#include "toolbar.h"
#include "wordlist.h"

typedef struct {
	void (*repaint)(void *);
} vocabed_cb_t;

/** Vocabulary editor verb types */
typedef enum {
	verb_move,
	verb_turn_left,
	verb_put_white,
	verb_put_grey,
	verb_put_black,
	verb_pick_up,
	verb_end,

	/** Above verbs have fixed icons */
	verb_limit,

	verb_call
} vocabed_verb_type_t;

/** Call verb */
typedef struct {
	/** Called procedure */
	prog_proc_t *proc;
} vocabed_verb_call_t;

/** Vocabulary editor verb */
typedef struct {
	vocabed_verb_type_t vtype;
	union {
		vocabed_verb_call_t vcall;
	} v;
} vocabed_verb_t;

/** Vocabulary editor state */
typedef enum {
	/** Work state */
	vst_work,
	/** Learn new procedure state */
	vst_learn,
	/** Examine command state */
	vst_examine
} vocabed_state_t;

/** Map editor */
typedef struct {
	/** Map view */
	mapview_t *mapview;
	/** Program view */
	progview_t *progview;
	/** Verb view */
	wordlist_t *verbs;
	/** Robots */
	robots_t *robots;
	/** Program module */
	prog_module_t *prog;
	/** Procedure currently learning */
	prog_proc_t *learn_proc;
	/** Icon dictionary */
	icondict_t *icondict;
	/** State */
	vocabed_state_t state;
	/** Vocabulary editor toolbar */
	toolbar_t *tb;
	/** Error dialog or @c NULL */
	errordlg_t *errordlg;
	/** Error displayed in error dialog */
	robot_error_t errordlg_error;
	/** Icon dialog or @c NULL */
	icondlg_t *icondlg;
	/** Callbacks */
	vocabed_cb_t *cb;
	/** Verb icons */
	gfx_bmp_t *verb_icons[verb_limit];
	/** Error icons */
	gfx_bmp_t *error_icons[errt_limit];
	/** Callback argument */
	void *arg;
} vocabed_t;

extern int vocabed_new(map_t *, robots_t *, prog_module_t *, vocabed_cb_t *,
    void *,
    vocabed_t **);
extern int vocabed_load(map_t *, robots_t *, prog_module_t *, FILE *,
    vocabed_cb_t *, void *, vocabed_t **);
extern void vocabed_destroy(vocabed_t *);
extern void vocabed_display(vocabed_t *, gfx_t *gfx);
extern int vocabed_save(vocabed_t *, FILE *);
extern void vocabed_event(vocabed_t *, SDL_Event *, gfx_t *);

#endif
