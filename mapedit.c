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
#include "toolbar.h"

enum {
	tile_xs = 32,
	tile_ys = 32,

	orig_x = 320,
	orig_y = 240
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

static const char *map_tile_files[] = {
	"img/tile/empty.bmp",
	"img/tile/wall.bmp",
	"img/tile/white.bmp",
	"img/tile/grey.bmp",
	"img/tile/black.bmp",
	"img/tile/robot.bmp",
	NULL
};

static int mapedit_map_setup(mapedit_t *);
static void mapedit_map_toolbar_cb(void *, int);
static int mapedit_mapt_to_toolbar_idx(map_tile_t);

/** Display Map editor.
 *
 * @param mapedit Map editor
 * @param gfx Graphics
 */
void mapedit_display(mapedit_t *mapedit, gfx_t *gfx)
{
	map_draw(mapedit->map, gfx);
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
 * @param cb Callbacks
 * @param arg Callback arguments
 * @param map Map
 * @param rmapedit Place to store pointer to new map editor
 * @return Zero on success or an error code
 */
static int mapedit_create(mapedit_cb_t *cb, void *arg, map_t *map,
    mapedit_t **rmapedit)
{
	mapedit_t *mapedit;
	int rc;

	mapedit = calloc(1, sizeof(mapedit_t));
	if (mapedit == NULL)
		return ENOMEM;

	mapedit->ttype = mapt_wall;

	rc = toolbar_create(map_tb_files, &mapedit->map_tb);
	if (rc != 0) {
		printf("Error creating menu.\n");
		goto error;
	}

	toolbar_set_origin(mapedit->map_tb, 8, 52);
	toolbar_set_cb(mapedit->map_tb, mapedit_map_toolbar_cb, mapedit);

	toolbar_select(mapedit->map_tb,
	    mapedit_mapt_to_toolbar_idx(mapedit->ttype));

	mapedit->map = map;
	mapedit->cb = cb;
	mapedit->arg = arg;

	*rmapedit = mapedit;
	return 0;
error:
	mapedit_destroy(mapedit);
	return rc;
}

/** Create new, empty map.
 *
 * @param cb Callbacks
 * @param arg Callback arguments
 * @param rmapedit Place to store pointer to new map editor
 * @return Zero on success or an error code
 */
int mapedit_new(mapedit_cb_t *cb, void *arg, mapedit_t **rmapedit)
{
	map_t *map;
	mapedit_t *mapedit;
	int rc;

	rc = map_create(8, 8, &map);
	if (rc != 0)
		return rc;

	rc = mapedit_create(cb, arg, map, &mapedit);
	if (rc != 0) {
		map_destroy(map);
		return rc;
	}

	rc = mapedit_map_setup(mapedit);
	if (rc != 0) {
		mapedit_destroy(mapedit);
		return rc;
	}

	mapedit_repaint_req(mapedit);

	*rmapedit = mapedit;
	return 0;
}

/** Load map editor.
 *
 * @param f File
 * @param cb Callbacks
 * @param arg Callback arguments
 * @param rmapedit Place to store pointer to new map editor
 * @return Zero on success or an error code
 */
int mapedit_load(FILE *f, mapedit_cb_t *cb, void *arg, mapedit_t **rmapedit)
{
	map_t *map = NULL;
	mapedit_t *mapedit = NULL;
	int rc;
	int nitem;
	int ttype;

	nitem = fscanf(f, "%d\n", &ttype);
	if (nitem != 1) {
		rc = EIO;
		goto error;
	}

	rc = map_load(f, &map);
	if (rc != 0) {
		rc = EIO;
		goto error;
	}

	rc = mapedit_create(cb, arg, map, &mapedit);
	if (rc != 0) {
		rc = ENOMEM;
		goto error;
	}

	map = NULL;

	rc = mapedit_map_setup(mapedit);
	if (rc != 0)
		goto error;

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
	if (map != NULL)
		map_destroy(map);
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
	int rc;
	int rv;

	rv = fprintf(f, "%d\n", (int)mapedit->ttype);
	if (rv < 0) {
		printf("Error saving map.\n");
		return EIO;
	}

	rc = map_save(mapedit->map, f);
	if (rc != 0) {
		printf("Error saving map.\n");
		return EIO;
	}

	return 0;
}

static void key_press(mapedit_t *mapedit, SDL_Scancode scancode)
{
	switch (scancode) {
	default:
		break;
	}
}

void mapedit_event(mapedit_t *mapedit, SDL_Event *e, gfx_t *gfx)
{
	SDL_KeyboardEvent *ke;
	SDL_MouseButtonEvent *me;

	if (toolbar_event(mapedit->map_tb, e))
		return;

	(void) map_event(mapedit->map, e);

	switch (e->type) {
	case SDL_KEYDOWN:
		ke = (SDL_KeyboardEvent *) e;
		key_press(mapedit, ke->keysym.scancode);
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
static void mapedit_map_cb(void *arg, int x, int y)
{
	mapedit_t *mapedit = (mapedit_t *)arg;
	printf("mapedit_map_cb(%d,%d)\n", x, y);

	mapedit->map->tile[x][y] = mapedit->ttype;

	mapedit_repaint_req(mapedit);
}

/** Set up new map for use.
 *
 * @param mapedit Map editor
 */
static int mapedit_map_setup(mapedit_t *mapedit)
{
	map_set_orig(mapedit->map, 0, 88);
	map_set_tile_size(mapedit->map, 32, 32);
	map_set_tile_margins(mapedit->map, 4, 4);
	map_set_cb(mapedit->map, mapedit_map_cb, mapedit);

	return map_load_tile_img(mapedit->map, map_tile_files);
}

/** Destroy map editor.
 *
 * @param mapedit Map editor
 */
void mapedit_destroy(mapedit_t *mapedit)
{
	if (mapedit->map_tb != NULL)
		toolbar_destroy(mapedit->map_tb);
	if (mapedit->map != NULL)
		map_destroy(mapedit->map);
	free(mapedit);
}
