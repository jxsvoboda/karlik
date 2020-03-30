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
#include "map.h"
#include "vocabed.h"

enum {
	tile_xs = 32,
	tile_ys = 32,

	orig_x = 320,
	orig_y = 240
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

static const char *verb_icons[] = {
	"img/verb/move.bmp",
	"img/verb/turnleft.bmp",
	"img/verb/putwhite.bmp",
	"img/verb/putgrey.bmp",
	"img/verb/putblack.bmp",
	"img/verb/pickup.bmp",
	NULL
};

static int vocabed_map_setup(vocabed_t *);
static void vocabed_verbs_cb(void *, void *);

/** Display Map editor.
 *
 * @param vocabed Map editor
 * @param gfx Graphics
 */
void vocabed_display(vocabed_t *vocabed, gfx_t *gfx)
{
	map_draw(vocabed->map, gfx);
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
 * @param cb Callbacks
 * @param arg Callback arguments
 * @param map Map
 * @param rvocabed Place to store pointer to new map editor
 * @return Zero on success or an error code
 */
static int vocabed_create(vocabed_cb_t *cb, void *arg, map_t *map,
    vocabed_t **rvocabed)
{
	vocabed_t *vocabed;
	const char **cp;
	SDL_Surface *icon;
	int rc;

	vocabed = calloc(1, sizeof(vocabed_t));
	if (vocabed == NULL)
		return ENOMEM;

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

	vocabed->quit = false;

	vocabed->map = map;
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
 * @param cb Callbacks
 * @param arg Callback arguments
 * @param rvocabed Place to store pointer to new map editor
 * @return Zero on success or an error code
 */
int vocabed_new(vocabed_cb_t *cb, void *arg, vocabed_t **rvocabed)
{
	map_t *map;
	vocabed_t *vocabed;
	int rc;

	rc = map_create(8, 8, &map);
	if (rc != 0)
		return rc;

	rc = vocabed_create(cb, arg, map, &vocabed);
	if (rc != 0) {
		map_destroy(map);
		return rc;
	}

	rc = vocabed_map_setup(vocabed);
	if (rc != 0) {
		vocabed_destroy(vocabed);
		return rc;
	}

	vocabed_repaint_req(vocabed);

	*rvocabed = vocabed;
	return 0;
}

/** Load map editor.
 *
 * @param f File
 * @param cb Callbacks
 * @param arg Callback arguments
 * @param rvocabed Place to store pointer to new map editor
 * @return Zero on success or an error code
 */
int vocabed_load(FILE *f, vocabed_cb_t *cb, void *arg, vocabed_t **rvocabed)
{
	map_t *map = NULL;
	vocabed_t *vocabed = NULL;
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

	rc = vocabed_create(cb, arg, map, &vocabed);
	if (rc != 0) {
		rc = ENOMEM;
		goto error;
	}

	map = NULL;

	rc = vocabed_map_setup(vocabed);
	if (rc != 0)
		goto error;

	printf("ttype=%d\n", ttype);

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
	int rc;

	rc = map_save(vocabed->map, f);
	if (rc != 0) {
		printf("Error saving map.\n");
		return EIO;
	}

	return 0;
}

static void key_press(vocabed_t *vocabed, SDL_Scancode scancode)
{
	switch (scancode) {
	default:
		break;
	}
}

void vocabed_event(vocabed_t *vocabed, SDL_Event *e, gfx_t *gfx)
{
	SDL_KeyboardEvent *ke;
	SDL_MouseButtonEvent *me;

	(void) map_event(vocabed->map, e);
	(void) wordlist_event(vocabed->verbs, e);

	switch (e->type) {
	case SDL_QUIT:
		vocabed->quit = true;
		break;
	case SDL_KEYDOWN:
		ke = (SDL_KeyboardEvent *) e;
		if (ke->keysym.scancode == SDL_SCANCODE_ESCAPE)
			vocabed->quit = true;
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
static void vocabed_map_cb(void *arg, int x, int y)
{
	vocabed_t *vocabed = (vocabed_t *)arg;
	printf("vocabed_map_cb(%d,%d)\n", x, y);

	vocabed->map->tile[x][y] = 0;

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
static int vocabed_map_setup(vocabed_t *vocabed)
{
	map_set_orig(vocabed->map, 0, 88);
	map_set_tile_size(vocabed->map, 32, 32);
	map_set_tile_margins(vocabed->map, 4, 4);
	map_set_cb(vocabed->map, vocabed_map_cb, vocabed);

	return map_load_tile_img(vocabed->map, map_tile_files);
}

/** Destroy map editor.
 *
 * @param vocabed Map editor
 */
void vocabed_destroy(vocabed_t *vocabed)
{
	if (vocabed->map != NULL)
		map_destroy(vocabed->map);
	if (vocabed->verbs != NULL)
		wordlist_destroy(vocabed->verbs);
	free(vocabed);
}
