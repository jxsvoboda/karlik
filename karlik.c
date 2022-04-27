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
 * Karlik
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <SDL.h>
#include "gfx.h"
#include "karlik.h"
#include "mapedit.h"
#include "prog.h"
#include "toolbar.h"
#include "vocabed.h"

static void karlik_cb_repaint(void *);
static void karlik_display(karlik_t *, gfx_t *);

static const char *main_tb_files[] = {
	"img/main/tool/vocab.bmp",
	"img/main/tool/map.bmp",
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

/** Robot image color key */
static int robots_key[3] = {
	72, 72, 72
};

/** Robot image file names */
static const char *robots_files[] = {
	"img/robot/east.bmp",
	"img/robot/north.bmp",
	"img/robot/west.bmp",
	"img/robot/south.bmp",
	NULL
};

static mapedit_cb_t karlik_mapedit_cb = {
	.repaint = karlik_cb_repaint
};

static vocabed_cb_t karlik_vocabed_cb = {
	.repaint = karlik_cb_repaint
};

static void karlik_cb_repaint(void *arg)
{
	karlik_t *karlik = (karlik_t *) arg;

	karlik_display(karlik, karlik->gfx);
	gfx_update(karlik->gfx);
}

/** Display Map editor.
 *
 * @param mapedit Map editor
 * @param gfx Graphics
 */
static void karlik_display(karlik_t *karlik, gfx_t *gfx)
{
	gfx_clear(gfx);

	toolbar_draw(karlik->main_tb, gfx);

	switch (karlik->kmode) {
	case km_map:
		if (karlik->mapedit != NULL)
			mapedit_display(karlik->mapedit, gfx);
		break;
	case km_vocab:
		if (karlik->vocabed != NULL)
			vocabed_display(karlik->vocabed, gfx);
		break;
	}
}

/** Set up map for use (common between load and new)
 *
 * @param karlik Karlik
 */
static int karlik_map_setup(karlik_t *karlik)
{
	int rc;

	rc = map_load_tile_img(karlik->map, map_tile_files);
	if (rc != 0)
		return rc;

	map_set_tile_size(karlik->map, 16, 16);
	map_set_tile_margins(karlik->map, 2, 2);

	return 0;
}

/** Set up robots for use (common between load and new)
 *
 * @param karlik Karlik
 */
static int karlik_robots_setup(karlik_t *karlik)
{
	int rc;

	rc = robots_load_img(karlik->robots, robots_key[0], robots_key[1],
	    robots_key[2], robots_files);
	if (rc != 0) {
		printf("Error loading robot graphics.\n");
		return rc;
	}

	robots_set_tile_size(karlik->robots, 18, 18);
	robots_set_rel_pos(karlik->robots, -5, -16);

	return 0;
}

/** Create new Karlik workspace.
 *
 * @param karlik Map editor
 * @return Zero on success or an error code
 */
static int karlik_new(karlik_t *karlik)
{
	int rc;

	rc = map_create(8, 8, &karlik->map);
	if (rc != 0)
		return rc;

	rc = karlik_map_setup(karlik);
	if (rc != 0)
		return rc;

	rc = robots_create(karlik->map, &karlik->robots);
	if (rc != 0)
		return rc;

	rc = karlik_robots_setup(karlik);
	if (rc != 0)
		return rc;

	rc = prog_module_create(&karlik->prog);
	if (rc != 0)
		return rc;

	rc = mapedit_new(karlik->map, karlik->robots, &karlik_mapedit_cb,
	    (void *)karlik, &karlik->mapedit);
	if (rc != 0)
		return rc;

	rc = vocabed_new(karlik->map, karlik->robots, karlik->prog,
	    &karlik_vocabed_cb, (void *)karlik, &karlik->vocabed);
	if (rc != 0)
		return rc;

	return 0;
}

/** Load Karlik state.
 *
 * @param mapedit Map editor
 */
static int karlik_load(karlik_t *karlik)
{
	FILE *f;
	int rc;
	int nitem;
	int kmode;

	f = fopen("karlik.dat", "r");
	if (f == NULL)
		return EIO;

	rc = map_load(f, &karlik->map);
	if (rc != 0)
		goto error;

	rc = karlik_map_setup(karlik);
	if (rc != 0)
		return rc;

	rc = robots_load(f, karlik->map, &karlik->robots);
	if (rc != 0)
		return rc;

	rc = karlik_robots_setup(karlik);
	if (rc != 0)
		return rc;

	rc = prog_module_load(f, &karlik->prog);
	if (rc != 0)
		return rc;

	nitem = fscanf(f, "%d\n", &kmode);
	if (nitem != 1)
		goto error;

	printf("kmode=%d\n", kmode);
	if (kmode >= 0 && kmode <= km_vocab)
		karlik->kmode = kmode;

	rc = mapedit_load(karlik->map, karlik->robots, f, &karlik_mapedit_cb,
	    (void *)karlik, &karlik->mapedit);
	if (rc != 0) {
		rc = EIO;
		goto error;
	}

	rc = vocabed_load(karlik->map, karlik->robots, karlik->prog, f,
	    &karlik_vocabed_cb, (void *)karlik, &karlik->vocabed);
	if (rc != 0) {
		rc = EIO;
		goto error;
	}

	(void) fclose(f);
	return 0;
error:
	printf("Error loading.\n");
	fclose(f);
	return rc;
}

/** Save Karlik workspace.
 *
 * @param karlik Karlik
 */
int karlik_save(karlik_t *karlik)
{
	FILE *f;
	int rc;
	int rv;

	f = fopen("karlik.dat", "w");
	if (f == NULL)
		return EIO;

	rc = map_save(karlik->map, f);
	if (rc != 0)
		goto error;

	rc = robots_save(karlik->robots, f);
	if (rc != 0)
		goto error;

	rc = prog_module_save(karlik->prog, f);
	if (rc != 0)
		goto error;

	rv = fprintf(f, "%d\n", karlik->kmode);
	if (rv < 0) {
		rc = EIO;
		goto error;
	}

	rc = mapedit_save(karlik->mapedit, f);
	if (rc != 0) {
		printf("Error saving map editor.\n");
		rc = EIO;
		goto error;
	}

	rc = vocabed_save(karlik->vocabed, f);
	if (rc != 0) {
		printf("Error saving vocabulary editor.\n");
		rc = EIO;
		goto error;
	}

	if (fclose(f) < 0)
		return EIO;

	return 0;
error:
	fclose(f);
	return rc;
}

static void karlik_key_press(karlik_t *karlik, SDL_Scancode scancode)
{
	switch (scancode) {
	case SDL_SCANCODE_L:
		karlik_load(karlik);
		break;
	case SDL_SCANCODE_S:
		karlik_save(karlik);
		break;
	default:
		break;
	}
}

void karlik_event(karlik_t *karlik, SDL_Event *e, gfx_t *gfx)
{
	SDL_KeyboardEvent *ke;
	SDL_MouseButtonEvent *me;

	if (toolbar_event(karlik->main_tb, e))
		return;

	switch (karlik->kmode) {
	case km_map:
		(void) mapedit_event(karlik->mapedit, e, gfx);
		break;
	case km_vocab:
		(void) vocabed_event(karlik->vocabed, e, gfx);
		break;
	}

	switch (e->type) {
	case SDL_QUIT:
		karlik->quit = true;
		break;
	case SDL_KEYDOWN:
		ke = (SDL_KeyboardEvent *) e;
		if (ke->keysym.scancode == SDL_SCANCODE_ESCAPE)
			karlik->quit = true;
		karlik_key_press(karlik, ke->keysym.scancode);
		karlik_display(karlik, gfx);
		gfx_update(gfx);
		break;
	case SDL_MOUSEBUTTONDOWN:
		me = (SDL_MouseButtonEvent *) e;
		(void) me;
		break;
	}
}

/** Main toolbar callback.
 *
 * @param arg Karlik (karlik_t *)
 * @param idx Index of the selected entry
 */
static void karlik_main_toolbar_cb(void *arg, int idx)
{
	karlik_t *karlik = (karlik_t *)arg;
	printf("karlik_main_toolbar_cb(%d)\n", idx);

	switch (idx) {
	case 0:
		karlik->kmode = km_vocab;
		break;
	case 1:
		karlik->kmode = km_map;
		break;
	}

	karlik_display(karlik, karlik->gfx);
	gfx_update(karlik->gfx);
}

/** Get toolbar index corresponsing to Karlik mode.
 *
 * @param kmode Karlik mode
 * @return Toolbar index
 */
static int karlik_mode_to_toolbar_idx(karlik_mode_t kmode)
{
	switch (kmode) {
	case km_vocab:
		return 0;
	case km_map:
		return 1;
	}

	assert(false);
	return 0;
}

/** Create Karlik.
 *
 * @param gfx Graphics
 * @param rkarlik Place to store pointer to new Karlik
 * @return Zero on success or an error code
 */
int karlik_create(gfx_t *gfx, karlik_t **rkarlik)
{
	karlik_t *karlik;
	int rc;

	karlik = calloc(1, sizeof(karlik_t));
	if (karlik == NULL)
		return ENOMEM;

	karlik->gfx = gfx;
	karlik->quit = false;
	karlik->kmode = km_map;

	rc = toolbar_create(main_tb_files, &karlik->main_tb);
	if (rc != 0) {
		printf("Error creating menu.\n");
		goto error;
	}

	toolbar_set_origin(karlik->main_tb, 4, 4);
	toolbar_set_cb(karlik->main_tb, karlik_main_toolbar_cb, karlik);

	rc = karlik_load(karlik);
	if (rc != 0) {
		rc = karlik_new(karlik);
		if (rc != 0)
			goto error;
	}

	printf("kmode=%d\n", karlik->kmode);
	toolbar_select(karlik->main_tb,
	    karlik_mode_to_toolbar_idx(karlik->kmode));

	karlik_display(karlik, gfx);
	gfx_update(gfx);

	*rkarlik = karlik;
	return 0;
error:
	karlik_destroy(karlik);
	return rc;
}

/** Destroy Karlik.
 *
 * @param karlik Karlik
 */
void karlik_destroy(karlik_t *karlik)
{
	if (karlik->main_tb != NULL)
		toolbar_destroy(karlik->main_tb);
	if (karlik->map != NULL)
		map_destroy(karlik->map);
	if (karlik->prog != NULL)
		prog_module_destroy(karlik->prog);
	if (karlik->mapedit != NULL)
		mapedit_destroy(karlik->mapedit);
	if (karlik->vocabed != NULL)
		vocabed_destroy(karlik->vocabed);
	free(karlik);
}
