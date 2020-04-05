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

/*
 * Map editor
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <SDL.h>
#include "gfx.h"
#include "map.h"
#include "mapedit.h"
#include "robots.h"
#include "toolbar.h"

enum {
	tile_xs = 16,
	tile_ys = 16,

	orig_x = 160,
	orig_y = 120
};

static const char *map_tb_files[] = {
	"img/tool/wall.bmp",
	"img/tool/white.bmp",
	"img/tool/grey.bmp",
	"img/tool/black.bmp",
	"img/tool/robot.bmp",
	"img/tool/delete.bmp",
	NULL
};

static void mapedit_mapview_setup(mapedit_t *);
static void mapedit_map_toolbar_cb(void *, int);
static int mapedit_mapt_to_toolbar_idx(map_tile_t);

/** Display Map editor.
 *
 * @param mapedit Map editor
 * @param gfx Graphics
 */
void mapedit_display(mapedit_t *mapedit, gfx_t *gfx)
{
	mapview_draw(mapedit->mapview, gfx);
	toolbar_draw(mapedit->map_tb, gfx);
}

/** Request repaint.
 *
 * @param mapedit Map editor
 */
static void mapedit_repaint_req(mapedit_t *mapedit)
{
	mapedit->cb->repaint(mapedit->arg);
}

/** Create map editor.
 *
 * @param map Map
 * @param robots Robots
 * @param cb Callbacks
 * @param arg Callback arguments
 * @param rmapedit Place to store pointer to new map editor
 * @return Zero on success or an error code
 */
static int mapedit_create(map_t *map, robots_t *robots, mapedit_cb_t *cb,
    void *arg, mapedit_t **rmapedit)
{
	mapedit_t *mapedit;
	int rc;

	mapedit = calloc(1, sizeof(mapedit_t));
	if (mapedit == NULL)
		return ENOMEM;

	mapedit->ttype = mapt_wall;

	rc = mapview_create(map, robots, &mapedit->mapview);
	if (rc != 0)
		goto error;

	mapedit->robots = robots;

	rc = toolbar_create(map_tb_files, &mapedit->map_tb);
	if (rc != 0) {
		printf("Error creating toolbar.\n");
		goto error;
	}

	toolbar_set_origin(mapedit->map_tb, 4, 26);
	toolbar_set_cb(mapedit->map_tb, mapedit_map_toolbar_cb, mapedit);

	toolbar_select(mapedit->map_tb,
	    mapedit_mapt_to_toolbar_idx(mapedit->ttype));

	mapedit->cb = cb;
	mapedit->arg = arg;

	*rmapedit = mapedit;
	return 0;
error:
	mapedit_destroy(mapedit);
	return rc;
}

/** Create new map editor.
 *
 * @param map Map
 * @param robots Robots
 * @param cb Callbacks
 * @param arg Callback arguments
 * @param rmapedit Place to store pointer to new map editor
 * @return Zero on success or an error code
 */
int mapedit_new(map_t *map, robots_t *robots, mapedit_cb_t *cb, void *arg,
    mapedit_t **rmapedit)
{
	mapedit_t *mapedit;
	int rc;

	rc = mapedit_create(map, robots, cb, arg, &mapedit);
	if (rc != 0)
		return rc;

	mapedit_mapview_setup(mapedit);
	mapedit_repaint_req(mapedit);

	*rmapedit = mapedit;
	return 0;
}

/** Load map editor.
 *
 * @param map Map
 * @param robots Robots
 * @param f File
 * @param cb Callbacks
 * @param arg Callback arguments
 * @param rmapedit Place to store pointer to new map editor
 * @return Zero on success or an error code
 */
int mapedit_load(map_t *map, robots_t *robots, FILE *f, mapedit_cb_t *cb,
    void *arg, mapedit_t **rmapedit)
{
	mapedit_t *mapedit = NULL;
	int rc;
	int nitem;
	int ttype;

	nitem = fscanf(f, "%d\n", &ttype);
	if (nitem != 1) {
		rc = EIO;
		goto error;
	}

	rc = mapedit_create(map, robots, cb, arg, &mapedit);
	if (rc != 0) {
		rc = ENOMEM;
		goto error;
	}

	map = NULL;

	mapedit_mapview_setup(mapedit);

	printf("ttype=%d\n", ttype);
	if (ttype >= 0 && ttype <= mapt_robot)
		mapedit->ttype = ttype;

	toolbar_select(mapedit->map_tb,
	    mapedit_mapt_to_toolbar_idx(mapedit->ttype));

	mapedit_repaint_req(mapedit);

	*rmapedit = mapedit;
	return 0;
error:
	if (mapedit != NULL)
		mapedit_destroy(mapedit);
	printf("Error loading map.\n");
	return rc;
}

/** Save map editor.
 *
 * @param mapedit Map editor
 * @param f File
 * @return Zero on success or an error code
 */
int mapedit_save(mapedit_t *mapedit, FILE *f)
{
	int rv;

	rv = fprintf(f, "%d\n", (int)mapedit->ttype);
	if (rv < 0) {
		printf("Error saving map.\n");
		return EIO;
	}

	return 0;
}

static void mapedit_key_press(mapedit_t *mapedit, SDL_Scancode scancode)
{
	(void) mapedit;

	switch (scancode) {
	default:
		break;
	}
}

void mapedit_event(mapedit_t *mapedit, SDL_Event *e, gfx_t *gfx)
{
	SDL_KeyboardEvent *ke;
	SDL_MouseButtonEvent *me;

	(void) gfx;

	if (toolbar_event(mapedit->map_tb, e))
		return;

	(void) mapview_event(mapedit->mapview, e);

	switch (e->type) {
	case SDL_KEYDOWN:
		ke = (SDL_KeyboardEvent *) e;
		mapedit_key_press(mapedit, ke->keysym.scancode);
		break;
	case SDL_MOUSEBUTTONDOWN:
		me = (SDL_MouseButtonEvent *) e;
		(void) me;
		break;
	}
}

/** Map toolbar callback.
 *
 * @param arg Map editor (mapedit_t *)
 * @param idx Index of the selected entry
 */
static void mapedit_map_toolbar_cb(void *arg, int idx)
{
	mapedit_t *mapedit = (mapedit_t *)arg;
	printf("mapedit_map_toolbar_cb(%d)\n", idx);

	switch (idx) {
	case 0:
		mapedit->ttype = mapt_wall;
		break;
	case 1:
		mapedit->ttype = mapt_wtag;
		break;
	case 2:
		mapedit->ttype = mapt_gtag;
		break;
	case 3:
		mapedit->ttype = mapt_btag;
		break;
	case 4:
		mapedit->ttype = mapt_robot;
		break;
	case 5:
		mapedit->ttype = mapt_none;
		break;
	}

	mapedit_repaint_req(mapedit);
}

/** Get toolbar index corresponsing to map tile type.
 *
 * @param mapt Map tile type
 * @return Toolbar index
 */
static int mapedit_mapt_to_toolbar_idx(map_tile_t mapt)
{
	switch (mapt) {
	case mapt_wall:
		return 0;
	case mapt_wtag:
		return 1;
	case mapt_gtag:
		return 2;
	case mapt_btag:
		return 3;
	case mapt_robot:
		return 4;
	case mapt_none:
		return 5;
	}

	assert(false);
	return 0;
}

/** Map callback.
 *
 * @param arg Map editor (mapedit_t *)
 * @param x Tile X coordinate
 * @param y Tile Y coordinate
 */
static void mapedit_mapview_cb(void *arg, int x, int y)
{
	mapedit_t *mapedit = (mapedit_t *)arg;
	map_tile_t oldt;
	robot_t *oldr;

	printf("mapedit_map_cb(%d,%d)\n", x, y);

	oldt = map_get(mapedit->mapview->map, x, y);
	oldr = robots_get(mapedit->robots, x, y);

	if (mapedit->ttype != mapt_robot) {
		if (mapedit->ttype != mapt_wall || oldr == NULL)
			map_set(mapedit->mapview->map, x, y, mapedit->ttype);
	}

	if (mapedit->ttype == mapt_none)
		robots_remove(mapedit->robots, x, y);

	if (mapedit->ttype == mapt_robot && oldt != mapt_wall)
		(void) robots_add(mapedit->robots, x, y);

	mapedit_repaint_req(mapedit);
}

/** Set up new map view for use.
 *
 * @param mapedit Map editor
 */
static void mapedit_mapview_setup(mapedit_t *mapedit)
{
	mapview_set_orig(mapedit->mapview, 0, 56);
	mapview_set_cb(mapedit->mapview, mapedit_mapview_cb, mapedit);
}

/** Destroy map editor.
 *
 * @param mapedit Map editor
 */
void mapedit_destroy(mapedit_t *mapedit)
{
	if (mapedit->map_tb != NULL)
		toolbar_destroy(mapedit->map_tb);
	if (mapedit->mapview != NULL)
		mapview_destroy(mapedit->mapview);
	free(mapedit);
}
