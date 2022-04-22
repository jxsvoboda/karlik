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

/*
 * Vocabulary editor
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <SDL.h>
#include "gfx.h"
#include "mapview.h"
#include "progview.h"
#include "robots.h"
#include "vocabed.h"
#include "toolbar.h"

enum {
	tile_xs = 32,
	tile_ys = 32,

	orig_x = 320,
	orig_y = 240
};

/** Verb icon files */
static const char *verb_icon_files[] = {
	[verb_move] = "img/verb/move.bmp",
	[verb_turn_left] = "img/verb/turnleft.bmp",
	[verb_put_white] = "img/verb/putwhite.bmp",
	[verb_put_grey] = "img/verb/putgrey.bmp",
	[verb_put_black] = "img/verb/putblack.bmp",
	[verb_pick_up] = "img/verb/pickup.bmp",
	[verb_end] = "img/verb/end.bmp",
	[verb_limit] = NULL
};

/** Toolbar icon files */
static const char *vocabed_tb_files[] = {
	"img/vocabed/tool/work.bmp",
	"img/vocabed/tool/learn.bmp",
	"img/vocabed/tool/examine.bmp",
	NULL
};

/** Verbs corresponding to intrinsic statements */
static const vocabed_verb_type_t intrinsic_verbs[] = {
	verb_move,
	verb_turn_left,
	verb_put_white,
	verb_put_grey,
	verb_put_black,
	verb_pick_up,
	verb_limit
};

static int vocabed_add_statement_verbs(vocabed_t *);
static int vocabed_add_predefined_verb(vocabed_t *, vocabed_verb_type_t);
static void vocabed_map_setup(vocabed_t *);
static void vocabed_learn(vocabed_t *);
static void vocabed_toolbar_cb(void *, int);

static void vocabed_work_verb_selected(void *, void *);
static void vocabed_learn_verb_selected(void *, void *);
static void vocabed_verb_destroy(void *, void *);

static wordlist_cb_t vocabed_work_verbs_cb = {
	.selected = vocabed_work_verb_selected,
	.destroy = vocabed_verb_destroy
};

static wordlist_cb_t vocabed_learn_verbs_cb = {
	.selected = vocabed_learn_verb_selected,
	.destroy = vocabed_verb_destroy
};

/** Display vocabulary editor.
 *
 * @param vocabed Vocabulary editor
 * @param gfx Graphics
 */
void vocabed_display(vocabed_t *vocabed, gfx_t *gfx)
{
	mapview_draw(vocabed->mapview, gfx);
	toolbar_draw(vocabed->tb, gfx);
	progview_draw(vocabed->progview, gfx);
	wordlist_draw(vocabed->verbs, gfx);
}

/** Request repaint.
 *
 * @param vocabed Vocabulary editor
 */
static void vocabed_repaint_req(vocabed_t *vocabed)
{
	vocabed->cb->repaint(vocabed->arg);
}

/** Set up vocabulary editor work state.
 *
 * @param vocabed Vocabulary editor
 */
static int vocabed_work(vocabed_t *vocabed)
{
	int rc;

	vocabed->state = vst_work;

	wordlist_clear(vocabed->verbs);
	wordlist_set_cb(vocabed->verbs, &vocabed_work_verbs_cb, vocabed);

	rc = vocabed_add_statement_verbs(vocabed);
	if (rc != 0)
		return rc;

	progview_set_proc(vocabed->progview, NULL);

	return 0;
}

/** Add verb with predefined icon to the verb list.
 *
 * @param vocabed Vocabulary editor
 * @param verb Predefined verb
 * @return EOK on success or an error code
 */
static int vocabed_add_predefined_verb(vocabed_t *vocabed,
    vocabed_verb_type_t vtype)
{
	vocabed_verb_t *verb;

	verb = calloc(1, sizeof(vocabed_verb_t));
	if (verb == NULL)
		return ENOMEM;

	verb->vtype = vtype;
	return wordlist_add(vocabed->verbs, vocabed->verb_icons[vtype], verb);
}

/** Add call verb to the verb list.
 *
 * @param vocabed Vocabulary editor
 * @param proc Called procedure
 * @return EOK on success or an error code
 */
static int vocabed_add_call_verb(vocabed_t *vocabed, prog_proc_t *proc)
{
	vocabed_verb_t *verb;

	verb = calloc(1, sizeof(vocabed_verb_t));
	if (verb == NULL)
		return ENOMEM;

	verb->vtype = verb_call;
	verb->v.vcall.proc = proc;
	return wordlist_add(vocabed->verbs, vocabed->verb_icons[verb_move],
	    verb);
}

/** Add statement verbs to the verb list.
 *
 * @param vocabed Vocabulary editor
 */
static int vocabed_add_statement_verbs(vocabed_t *vocabed)
{
	prog_proc_t *proc;
	int rc;
	unsigned i;

	/* Verbs for intrinsic statements */
	i = 0;
	while (intrinsic_verbs[i] < verb_limit) {
		rc = vocabed_add_predefined_verb(vocabed, intrinsic_verbs[i]);
		if (rc != 0)
			return rc;

		++i;
	}

	/* Add call verb for each defined procedure */
	proc = prog_module_first(vocabed->prog);
	while (proc != NULL) {
		rc = vocabed_add_call_verb(vocabed, proc);
		if (rc != 0)
			return rc;

		proc = prog_module_next(proc);
	}

	return 0;
}

/** Create vocabulary editor.
 *
 * @param map Map
 * @param robots Robots
 * @param prog Program module
 * @param cb Callbacks
 * @param arg Callback arguments
 * @param rvocabed Place to store pointer to new vocabulary editor
 * @return Zero on success or an error code
 */
static int vocabed_create(map_t *map, robots_t *robots, prog_module_t *prog,
    vocabed_cb_t *cb, void *arg, vocabed_t **rvocabed)
{
	vocabed_t *vocabed;
	const char **cp;
	unsigned i;
	int rc;

	vocabed = calloc(1, sizeof(vocabed_t));
	if (vocabed == NULL)
		return ENOMEM;

	rc = mapview_create(map, robots, &vocabed->mapview);
	if (rc != 0)
		goto error;

	rc = toolbar_create(vocabed_tb_files, &vocabed->tb);
	if (rc != 0)
		goto error;

	toolbar_set_origin(vocabed->tb, 4, 26);
	toolbar_set_cb(vocabed->tb, vocabed_toolbar_cb, vocabed);

	rc = progview_create(&vocabed->progview);
	if (rc != 0)
		goto error;

	progview_set_orig(vocabed->progview, 170, 36);
	vocabed->progview->icon_w = 16;
	vocabed->progview->icon_h = 16;
	vocabed->progview->margin_x = 2;
	vocabed->progview->margin_y = 2;

	rc = wordlist_create(&vocabed->verbs);
	if (rc != 0)
		goto error;

	wordlist_set_origin(vocabed->verbs, 0, 214);
	wordlist_set_cb(vocabed->verbs, &vocabed_work_verbs_cb, vocabed);

	cp = verb_icon_files;
	i = 0;
	while (*cp != NULL) {
		printf("Load '%s'\n", *cp);
		rc = gfx_bmp_load(*cp, &vocabed->verb_icons[i]);
		if (rc != 0)
			goto error;

		++cp;
		++i;
	}

	vocabed->cb = cb;
	vocabed->arg = arg;

	vocabed->robots = robots;
	vocabed->prog = prog;

	rc = vocabed_work(vocabed);
	if (rc != 0)
		goto error;

	*rvocabed = vocabed;
	return 0;
error:
	vocabed_destroy(vocabed);
	return rc;
}

/** Create new, empty vocabulary editor.
 *
 * @param map Map
 * @param robots Robots
 * @param prog Program module
 * @param cb Callbacks
 * @param arg Callback arguments
 * @param rvocabed Place to store pointer to new vocabulary editor
 * @return Zero on success or an error code
 */
int vocabed_new(map_t *map, robots_t *robots, prog_module_t *prog,
    vocabed_cb_t *cb, void *arg, vocabed_t **rvocabed)
{
	vocabed_t *vocabed;
	int rc;

	rc = vocabed_create(map, robots, prog, cb, arg, &vocabed);
	if (rc != 0) {
		map_destroy(map);
		return rc;
	}

	vocabed_map_setup(vocabed);
	vocabed_repaint_req(vocabed);

	*rvocabed = vocabed;
	return 0;
}

/** Load vocabulary editor.
 *
 * @param map Map
 * @param robots Robots
 * @param prog Program module
 * @param f File
 * @param cb Callbacks
 * @param arg Callback arguments
 * @param rvocabed Place to store pointer to new vocabulary editor
 * @return Zero on success or an error code
 */
int vocabed_load(map_t *map, robots_t *robots, prog_module_t *prog, FILE *f,
    vocabed_cb_t *cb, void *arg, vocabed_t **rvocabed)
{
	vocabed_t *vocabed = NULL;
	int nitem;
	int rc;
	unsigned state;
	unsigned have_learn_proc;

	rc = vocabed_create(map, robots, prog, cb, arg, &vocabed);
	if (rc != 0) {
		rc = ENOMEM;
		goto error;
	}

	nitem = fscanf(f, "%u\n", &state);
	if (nitem != 1) {
		rc = EIO;
		goto error;
	}

	nitem = fscanf(f, "%u\n", &have_learn_proc);
	if (nitem != 1) {
		rc = EIO;
		goto error;
	}

	switch (state) {
	case vst_work:
		vocabed_work(vocabed);
		break;
	case vst_learn:
		vocabed_learn(vocabed);
		break;
	default:
		rc = EIO;
		goto error;
	}

	toolbar_select(vocabed->tb, vocabed->state);

	if (have_learn_proc != 0) {
		printf("Have learn proc - yes!\n");
		rc = prog_proc_load(vocabed->prog, f, &vocabed->learn_proc);
		if (rc != 0)
			goto error;

		printf("Statements: %lu\n", list_count(&vocabed->learn_proc->body->stmts));
		progview_set_proc(vocabed->progview, vocabed->learn_proc);
	}

	map = NULL;

	vocabed_map_setup(vocabed);
	vocabed_repaint_req(vocabed);

	*rvocabed = vocabed;
	return 0;
error:
	if (vocabed != NULL)
		vocabed_destroy(vocabed);
	if (map != NULL)
		map_destroy(map);
	printf("Error loading map.\n");
	return rc;
}

/** Save vocabulary editor.
 *
 * @param vocabed Vocabulary editor
 * @param f File
 * @return Zero on success or an error code
 */
int vocabed_save(vocabed_t *vocabed, FILE *f)
{
	int rc;
	int rv;

	rv = fprintf(f, "%u\n", (unsigned)vocabed->state);
	if (rv < 0)
		return EIO;

	rv = fprintf(f, "%u\n", vocabed->learn_proc != NULL ? 1 : 0);
	if (rv < 0)
		return EIO;

	if (vocabed->learn_proc != NULL) {
		rc = prog_proc_save(vocabed->learn_proc, f);
		if (rc != 0)
			return rc;
	}

	return 0;
}

/** Handle key press in vocabulary editor.
 *
 * @param vocabed Vocabulary editor
 * @param scancode Scancode
 */
static void vocabed_key_press(vocabed_t *vocabed, SDL_Scancode scancode)
{
	(void) vocabed;

	switch (scancode) {
	default:
		break;
	}
}

/** Vocabulary editor input event.
 *
 * @param vocabed Vocabulary editor
 * @param e Event
 * @param gfx Graphics
 */
void vocabed_event(vocabed_t *vocabed, SDL_Event *e, gfx_t *gfx)
{
	SDL_KeyboardEvent *ke;
	SDL_MouseButtonEvent *me;

	(void) gfx;

	if (toolbar_event(vocabed->tb, e))
		return;

	(void) mapview_event(vocabed->mapview, e);
	(void) wordlist_event(vocabed->verbs, e);

	switch (e->type) {
	case SDL_KEYDOWN:
		ke = (SDL_KeyboardEvent *) e;
		vocabed_key_press(vocabed, ke->keysym.scancode);
		break;
	case SDL_MOUSEBUTTONDOWN:
		me = (SDL_MouseButtonEvent *) e;
		(void) me;
		break;
	}
}

/** Start learning a new procedure.
 *
 * @param vocabed Vocabulary editor
 */
static void vocabed_learn(vocabed_t *vocabed)
{
	char *ident;
	int rc;

	printf("Learn!\n");

	rc = prog_module_gen_ident(vocabed->prog, &ident);
	if (rc != 0)
		return;

	if (vocabed->learn_proc == NULL) {
		rc = prog_proc_create(ident, &vocabed->learn_proc);
		if (rc != 0) {
			free(ident);
			return;
		}

		rc = prog_block_create(&vocabed->learn_proc->body);
		if (rc != 0) {
			prog_proc_destroy(vocabed->learn_proc);
			vocabed->learn_proc = NULL;
			return;
		}
	}

	free(ident);

	vocabed->state = vst_learn;
	progview_set_proc(vocabed->progview, vocabed->learn_proc);

	wordlist_clear(vocabed->verbs);
	wordlist_set_cb(vocabed->verbs, &vocabed_learn_verbs_cb, vocabed);

	(void) vocabed_add_statement_verbs(vocabed);
	(void) vocabed_add_predefined_verb(vocabed, verb_end);
}

/** End block when learning a new procedure.
 *
 * @param vocabed Vocabulary editor
 */
static void vocabed_learn_end(vocabed_t *vocabed)
{
	printf("Learn end!\n");

	/* Append new procedure to program */
	prog_module_append(vocabed->prog, vocabed->learn_proc);
	vocabed->learn_proc = NULL;
	progview_set_proc(vocabed->progview, NULL);

	/* Return to work mode */
	(void) vocabed_work(vocabed);
	toolbar_select(vocabed->tb, vocabed->state);
}

/** Vocabulary editor callback.
 *
 * @param arg Vocabulary editor (vocabed_t *)
 * @param x Tile X coordinate
 * @param y Tile Y coordinate
 */
static void vocabed_mapview_cb(void *arg, int x, int y)
{
	vocabed_t *vocabed = (vocabed_t *)arg;
	printf("vocabed_mapview_cb(%d,%d)\n", x, y);

	vocabed_repaint_req(vocabed);
}

/** Vocabulary editor immeadite mode verbs callback.
 *
 * Called when a verb is selected in work mode.
 *
 * @param arg Vocabulary editor (vocabed_t)
 * @param earg Etry argument
 */
static void vocabed_work_verb_selected(void *arg, void *earg)
{
	vocabed_t *vocabed = (vocabed_t *)arg;
	vocabed_verb_t *verb = (vocabed_verb_t *)earg;
	robot_t *robot;

	printf("Work mode. Verb type '%u'\n", verb->vtype);

	robot = robots_first(vocabed->robots);
	while (robot != NULL) {
		switch (verb->vtype) {
		case verb_move:
			robot_move(robot);
			break;
		case verb_turn_left:
			robot_turn_left(robot);
			break;
		case verb_put_white:
			robot_put_white(robot);
			break;
		case verb_put_grey:
			robot_put_grey(robot);
			break;
		case verb_put_black:
			robot_put_black(robot);
			break;
		case verb_pick_up:
			robot_pick_up(robot);
			break;
		case verb_call:
			robot_run_proc(robot, verb->v.vcall.proc);
			while (robot_is_busy(robot))
				robot_step(robot);
		default:
			break;
		}

		robot = robots_next(robot);
	}

	vocabed_repaint_req(vocabed);
}

static void vocabed_verb_destroy(void *arg, void *earg)
{
	vocabed_t *vocabed = (vocabed_t *)arg;
	vocabed_verb_t *verb = (vocabed_verb_t *)earg;

	(void)vocabed;
	free(verb);
}

static void vocabed_learn_intrinsic(vocabed_t *vocabed, prog_intr_type_t itype)
{
	prog_stmt_t *stmt;
	int rc;

	rc = prog_stmt_intrinsic_create(itype, &stmt);
	if (rc != 0)
		return;

	prog_block_append(vocabed->learn_proc->body, stmt);
}

/** Vocabulary editor learn mode verb selected.
 *
 * Called when a verb is selected in learn mode.
 *
 * @param arg Vocabulary editor (vocabed_t)
 * @param earg Etry argument
 */
static void vocabed_learn_verb_selected(void *arg, void *earg)
{
	vocabed_t *vocabed = (vocabed_t *)arg;
	vocabed_verb_t *verb = (vocabed_verb_t *)earg;

	printf("Learn mode. Verb type '%u'\n", verb->vtype);

	switch (verb->vtype) {
	case verb_move:
		vocabed_learn_intrinsic(vocabed, progin_move);
		break;
	case verb_turn_left:
		vocabed_learn_intrinsic(vocabed, progin_turn_left);
		break;
	case verb_put_white:
		vocabed_learn_intrinsic(vocabed, progin_put_white);
		break;
	case verb_put_grey:
		vocabed_learn_intrinsic(vocabed, progin_put_grey);
		break;
	case verb_put_black:
		vocabed_learn_intrinsic(vocabed, progin_put_black);
		break;
	case verb_pick_up:
		vocabed_learn_intrinsic(vocabed, progin_pick_up);
		break;
	case verb_end:
		vocabed_learn_end(vocabed);
		break;
	default:
		break;
	}

	vocabed_repaint_req(vocabed);
}

/** Set up vocabulary editor's mapview for use.
 *
 * @param vocabed Vocabulary editor
 */
static void vocabed_map_setup(vocabed_t *vocabed)
{
	mapview_set_orig(vocabed->mapview, 0, 56);
	mapview_set_cb(vocabed->mapview, vocabed_mapview_cb, vocabed);
}

/** Vocabulary editor toolbar callback.
 *
 * @param arg Vocabulary editor (vocabed_t *)
 * @param idx Index of the selected entry
 */
static void vocabed_toolbar_cb(void *arg, int idx)
{
	vocabed_t *vocabed = (vocabed_t *)arg;
	printf("vocabed_toolbar_cb(%d)\n", idx);

	switch (idx) {
	case 0:
		vocabed_work(vocabed);
		break;
	case 1:
		vocabed_learn(vocabed);
		break;
	case 2:
		break;
	}

	vocabed_repaint_req(vocabed);
}

/** Destroy vocabulary editor.
 *
 * @param vocabed Vocabulary editor
 */
void vocabed_destroy(vocabed_t *vocabed)
{
	if (vocabed->mapview != NULL)
		mapview_destroy(vocabed->mapview);
	if (vocabed->progview != NULL)
		progview_destroy(vocabed->progview);
	if (vocabed->verbs != NULL)
		wordlist_destroy(vocabed->verbs);
	free(vocabed);
}
