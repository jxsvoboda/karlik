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
#include "karlik.h"
#include "mapedit.h"
#include "toolbar.h"
#include "vocabed.h"

static void karlik_cb_repaint(void *);
static void karlik_display(karlik_t *, gfx_t *);

static const char *main_tb_files[] = {
	"img/main/tool/vocab.bmp",
	"img/main/tool/map.bmp",
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

	toolbar_draw(karlik->main_tb, gfx);
}

/** Create new Karlik workspace.
 *
 * @param gfx Graphics
 * @param karlik Map editor
 * @return Zero on success or an error code
 */
static int karlik_new(gfx_t *gfx, karlik_t *karlik)
{
	int rc;

	rc = mapedit_new(&karlik_mapedit_cb, (void *)karlik,
	    &karlik->mapedit);
	if (rc != 0)
		return rc;

	rc = vocabed_new(&karlik_vocabed_cb, (void *)karlik,
	    &karlik->vocabed);
	if (rc != 0)
		return rc;

	return 0;
}

/** Load Karlik state.
 *
 * @param gfx Graphics
 * @param mapedit Map editor
 */
static int karlik_load(gfx_t *gfx, karlik_t *karlik)
{
	FILE *f;
	int rc;
	int nitem;
	int kmode;

	karlik->gfx = gfx;

	f = fopen("karlik.dat", "r");
	if (f == NULL)
		return EIO;

	nitem = fscanf(f, "%d\n", &kmode);
	if (nitem != 1)
		goto error;

	printf("kmode=%d\n", kmode);
	if (kmode >= 0 && kmode <= km_vocab)
		karlik->kmode = kmode;

	rc = mapedit_load(f, &karlik_mapedit_cb, (void *)karlik,
	    &karlik->mapedit);
	if (rc != 0) {
		rc = EIO;
		goto error;
	}

	rc = vocabed_load(f, &karlik_vocabed_cb, (void *)karlik,
	    &karlik->vocabed);
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

	rv = fprintf(f, "%d\n", karlik->kmode);
	if (rv < 0) {
		fclose(f);
		return EIO;
	}

	rc = mapedit_save(karlik->mapedit, f);
	if (rc != 0) {
		printf("Error saving map editor.\n");
		fclose(f);
		return EIO;
	}

	rc = vocabed_save(karlik->vocabed, f);
	if (rc != 0) {
		printf("Error saving vocabulary editor.\n");
		fclose(f);
		return EIO;
	}

	if (fclose(f) < 0)
		return EIO;

	return 0;
}

static void key_press(karlik_t *karlik, SDL_Scancode scancode)
{
	gfx_t *gfx = karlik->gfx;

	switch (scancode) {
	case SDL_SCANCODE_L:
		karlik_load(gfx, karlik);
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
		key_press(karlik, ke->keysym.scancode);
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

	karlik->quit = false;
	karlik->kmode = km_map;

	rc = toolbar_create(main_tb_files, &karlik->main_tb);
	if (rc != 0) {
		printf("Error creating menu.\n");
		goto error;
	}

	toolbar_set_origin(karlik->main_tb, 8, 8);
	toolbar_set_cb(karlik->main_tb, karlik_main_toolbar_cb, karlik);

	rc = karlik_load(gfx, karlik);
	if (rc != 0) {
		rc = karlik_new(gfx, karlik);
		if (rc != 0)
			goto error;
	}

	printf("kmode=%d\n", karlik->kmode);
	toolbar_select(karlik->main_tb,
	    karlik_mode_to_toolbar_idx(karlik->kmode));

	karlik->gfx = gfx;
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
	if (karlik->mapedit != NULL)
		mapedit_destroy(karlik->mapedit);
	free(karlik);
}
