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

/** Map editor */
typedef struct {
	/** Map */
	map_t *map;
	/** Selected tile type */
	map_tile_t ttype;
	/** Map editor toolbar */
	toolbar_t *map_tb;
	/** @c true to quit */
	bool quit;
	/** Graphics */
	gfx_t *gfx;
} mapedit_t;

static int mapedit_map_setup(mapedit_t *);

/** Display Map editor.
 *
 * @param mapedit Map editor
 * @param gfx Graphics
 */
static void mapedit_display(mapedit_t *mapedit, gfx_t *gfx)
{
	gfx_clear(gfx);
	map_draw(mapedit->map, gfx);
	toolbar_draw(mapedit->map_tb, gfx);
}

/** Create new, empty map.
 *
 * @param mapedit Map editor
 * @return Zero on success or an error code
 */
static int mapedit_new(mapedit_t *mapedit)
{
	int rc;

	rc = map_create(8, 8, &mapedit->map);
	if (rc != 0)
		return rc;

	return mapedit_map_setup(mapedit);
}

/** Load project.
 *
 * @param mapedit Map editor
 */
static int mapedit_load(mapedit_t *mapedit)
{
	FILE *f;
	int rc;
	int nitem;
	int ttype;

	f = fopen("city.map", "r");
	if (f == NULL)
		return EIO;

	nitem = fscanf(f, "%d\n", &ttype);
	if (nitem != 1) {
		rc = EIO;
		goto error;
	}

	rc = map_load(f, &mapedit->map);
	if (rc != 0) {
		rc = EIO;
		goto error;
	}

	rc = mapedit_map_setup(mapedit);
	if (rc != 0) {
		map_destroy(mapedit->map);
		mapedit->map = NULL;
		goto error;
	}

	(void) fclose(f);
	if (ttype >= 0 && ttype <= mapt_robot)
		mapedit->ttype = ttype;
	return 0;
error:
	printf("Error loading map.\n");
	fclose(f);
	return rc;
}

/** Save current project.
 *
 * @param mapedit Map editor
 */
static int mapedit_save(mapedit_t *mapedit)
{
	FILE *f;
	int rc;
	int rv;

	f = fopen("city.map", "w");
	if (f == NULL)
		return EIO;

	rv = fprintf(f, "%d\n", (int)mapedit->ttype);
	if (rv < 0) {
		printf("Error saving map.\n");
		fclose(f);
		return EIO;
	}

	rc = map_save(mapedit->map, f);
	if (rc != 0) {
		printf("Error saving map.\n");
		fclose(f);
		return EIO;
	}

	if (fclose(f) < 0)
		return EIO;

	return 0;
}

static void key_press(mapedit_t *mapedit, SDL_Scancode scancode)
{
	switch (scancode) {
	case SDL_SCANCODE_L:
		mapedit_load(mapedit);
		break;
	case SDL_SCANCODE_S:
		mapedit_save(mapedit);
		break;
	default:
		break;
	}
}

static void mapedit_event(mapedit_t *mapedit, SDL_Event *e, gfx_t *gfx)
{
	SDL_KeyboardEvent *ke;
	SDL_MouseButtonEvent *me;

	(void) map_event(mapedit->map, e);

	switch (e->type) {
	case SDL_QUIT:
		mapedit->quit = true;
		break;
	case SDL_KEYDOWN:
		ke = (SDL_KeyboardEvent *) e;
		if (ke->keysym.scancode == SDL_SCANCODE_ESCAPE)
			mapedit->quit = true;
		key_press(mapedit, ke->keysym.scancode);
		mapedit_display(mapedit, gfx);
		gfx_update(gfx);
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

	mapedit_display(mapedit, mapedit->gfx);
	gfx_update(mapedit->gfx);
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

	mapedit_display(mapedit, mapedit->gfx);
	gfx_update(mapedit->gfx);
}

/** Set up new map for use.
 *
 * @param mapedit Map editor
 */
static int mapedit_map_setup(mapedit_t *mapedit)
{
	map_set_orig(mapedit->map, 0, 50);
	map_set_tile_size(mapedit->map, 32, 32);
	map_set_tile_margins(mapedit->map, 4, 4);
	map_set_cb(mapedit->map, mapedit_map_cb, mapedit);

	return map_load_tile_img(mapedit->map, map_tile_files);
}

static void print_syntax(void)
{
	printf("Syntax: karlik [-f]\n");
}

int main(int argc, char *argv[])
{
	gfx_t gfx;
	SDL_Event e;
	mapedit_t mapedit;
	bool fs = false;
	int rc;

	if (argc >= 2) {
		if (strcmp(argv[1], "-f") == 0) {
			fs = true;
		} else {
			print_syntax();
			return 1;
		}

		if (argc > 2) {
			print_syntax();
			return 1;
		}
	}

	mapedit.quit = false;
	mapedit.ttype = mapt_wall;

	rc = toolbar_create(map_tb_files, &mapedit.map_tb);
	if (rc != 0) {
		printf("Error creating menu.\n");
		return 1;
	}

	toolbar_set_origin(mapedit.map_tb, 8, 8);
	toolbar_set_cb(mapedit.map_tb, mapedit_map_toolbar_cb, &mapedit);

	rc = mapedit_load(&mapedit);
	if (rc != 0) {
		rc = mapedit_new(&mapedit);
		if (rc != 0)
			return 1;
	}

	toolbar_select(mapedit.map_tb,
	    mapedit_mapt_to_toolbar_idx(mapedit.ttype));

	rc = gfx_init(&gfx, fs);
	if (rc != 0)
		return 1;

	SDL_Surface *appicon = SDL_LoadBMP("img/appicon.bmp");
	if (appicon == NULL)
		return 1;

	SDL_SetWindowIcon(gfx.win, appicon);

	mapedit.gfx = &gfx;
	mapedit_display(&mapedit, &gfx);
	gfx_update(&gfx);

	while (!mapedit.quit && SDL_WaitEvent(&e)) {
		if (toolbar_event(mapedit.map_tb, &e))
			continue;
		mapedit_event(&mapedit, &e, &gfx);
	}

	rc = mapedit_save(&mapedit);
	if (rc != 0) {
		printf("Error saving map!\n");
		return 1;
	}

	toolbar_destroy(mapedit.map_tb);
	gfx_quit(&gfx);

	return 0;
}
