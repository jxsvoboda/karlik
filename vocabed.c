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
 * Vocabulary editor
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <SDL.h>
#include "gfx.h"
#include "mapview.h"
#include "vocabed.h"

enum {
	tile_xs = 32,
	tile_ys = 32,

	orig_x = 320,
	orig_y = 240
};

static const char *verb_icons[] = {
	"img/verb/move.bmp",
	"img/verb/turnleft.bmp",
	"img/verb/putwhite.bmp",
	"img/verb/putgrey.bmp",
	"img/verb/putblack.bmp",
	"img/verb/pickup.bmp",
	NULL
};

static void vocabed_map_setup(vocabed_t *);
static void vocabed_verbs_cb(void *, void *);

/** Display Map editor.
 *
 * @param vocabed Map editor
 * @param gfx Graphics
 */
void vocabed_display(vocabed_t *vocabed, gfx_t *gfx)
{
	mapview_draw(vocabed->mapview, gfx);
	wordlist_draw(vocabed->verbs, gfx);
}

/** Request repaint.
 *
 * @param vocabed Map editor
 */
static void vocabed_repaint_req(vocabed_t *vocabed)
{
	vocabed->cb->repaint(vocabed->arg);
}

/** Create map editor.
 *
 * @param map Map
 * @param cb Callbacks
 * @param arg Callback arguments
 * @param rvocabed Place to store pointer to new map editor
 * @return Zero on success or an error code
 */
static int vocabed_create(map_t *map, vocabed_cb_t *cb, void *arg,
    vocabed_t **rvocabed)
{
	vocabed_t *vocabed;
	const char **cp;
	SDL_Surface *icon;
	int rc;

	vocabed = calloc(1, sizeof(vocabed_t));
	if (vocabed == NULL)
		return ENOMEM;

	rc = mapview_create(map, &vocabed->mapview);
	if (rc != 0) {
		rc = ENOMEM;
		goto error;
	}

	rc = wordlist_create(&vocabed->verbs);
	if (rc != 0) {
		rc = ENOMEM;
		goto error;
	}

	wordlist_set_origin(vocabed->verbs, 0, 400);
	wordlist_set_cb(vocabed->verbs, vocabed_verbs_cb, vocabed);

	cp = verb_icons;
	while (*cp != NULL) {
		printf("Load '%s'\n", *cp);
		icon = SDL_LoadBMP(*cp);
		if (icon == NULL) {
			rc = ENOMEM;
			goto error;
		}

		rc = wordlist_add(vocabed->verbs, icon, (void *) *cp);
		if (rc != 0) {
			rc = ENOMEM;
			goto error;
		}

		++cp;
	}

	vocabed->cb = cb;
	vocabed->arg = arg;

	*rvocabed = vocabed;
	return 0;
error:
	vocabed_destroy(vocabed);
	return rc;
}

/** Create new, empty map.
 *
 * @param map Map
 * @param cb Callbacks
 * @param arg Callback arguments
 * @param rvocabed Place to store pointer to new map editor
 * @return Zero on success or an error code
 */
int vocabed_new(map_t *map, vocabed_cb_t *cb, void *arg, vocabed_t **rvocabed)
{
	vocabed_t *vocabed;
	int rc;

	rc = vocabed_create(map, cb, arg, &vocabed);
	if (rc != 0) {
		map_destroy(map);
		return rc;
	}

	vocabed_map_setup(vocabed);
	vocabed_repaint_req(vocabed);

	*rvocabed = vocabed;
	return 0;
}

/** Load map editor.
 *
 * @param map Map
 * @param f File
 * @param cb Callbacks
 * @param arg Callback arguments
 * @param rvocabed Place to store pointer to new map editor
 * @return Zero on success or an error code
 */
int vocabed_load(map_t *map, FILE *f, vocabed_cb_t *cb, void *arg, vocabed_t **rvocabed)
{
	vocabed_t *vocabed = NULL;
	int rc;

	(void) f;

	rc = vocabed_create(map, cb, arg, &vocabed);
	if (rc != 0) {
		rc = ENOMEM;
		goto error;
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

/** Save map editor.
 *
 * @param vocabed Map editor
 * @param f File
 * @return Zero on success or an error code
 */
int vocabed_save(vocabed_t *vocabed, FILE *f)
{
	(void) vocabed;
	(void) f;
	return 0;
}

static void key_press(vocabed_t *vocabed, SDL_Scancode scancode)
{
	(void) vocabed;

	switch (scancode) {
	default:
		break;
	}
}

void vocabed_event(vocabed_t *vocabed, SDL_Event *e, gfx_t *gfx)
{
	SDL_KeyboardEvent *ke;
	SDL_MouseButtonEvent *me;

	(void) gfx;

	(void) mapview_event(vocabed->mapview, e);
	(void) wordlist_event(vocabed->verbs, e);

	switch (e->type) {
	case SDL_KEYDOWN:
		ke = (SDL_KeyboardEvent *) e;
		key_press(vocabed, ke->keysym.scancode);
		break;
	case SDL_MOUSEBUTTONDOWN:
		me = (SDL_MouseButtonEvent *) e;
		(void) me;
		break;
	}
}

/** Map callback.
 *
 * @param arg Map editor (vocabed_t *)
 * @param x Tile X coordinate
 * @param y Tile Y coordinate
 */
static void vocabed_mapview_cb(void *arg, int x, int y)
{
	vocabed_t *vocabed = (vocabed_t *)arg;
	printf("vocabed_mapview_cb(%d,%d)\n", x, y);

	vocabed_repaint_req(vocabed);
}

static void vocabed_verbs_cb(void *arg, void *earg)
{
	vocabed_t *vocabed = (vocabed_t *)arg;
	const char *str = (const char *)earg;

	(void)vocabed;
	printf("Entry '%s'\n", str);
}

/** Set up new map for use.
 *
 * @param vocabed Map editor
 */
static void vocabed_map_setup(vocabed_t *vocabed)
{
	mapview_set_orig(vocabed->mapview, 0, 88);
	mapview_set_cb(vocabed->mapview, vocabed_mapview_cb, vocabed);
}

/** Destroy map editor.
 *
 * @param vocabed Map editor
 */
void vocabed_destroy(vocabed_t *vocabed)
{
	if (vocabed->mapview != NULL)
		mapview_destroy(vocabed->mapview);
	if (vocabed->verbs != NULL)
		wordlist_destroy(vocabed->verbs);
	free(vocabed);
}
