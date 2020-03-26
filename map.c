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
 * City map
 */

#include <errno.h>
#include <SDL.h>
#include <stdbool.h>
#include "gfx.h"
#include "map.h"

/** Create icon menu.
 *
 * @param width in tiles
 * @param height in tiles
 * @param rmap Place to store pointer to new map
 *
 * @return Zero on success or error code
 */
int map_create(int w, int h, map_t **rmap)
{
	map_t *map;
	int x;

	map = calloc(1, sizeof(map_t));
	if (map == NULL)
		return ENOMEM;

	map->width = w;
	map->height = h;

	map->tile = calloc(w, sizeof(map_tile_t *));
	if (map->tile == NULL)
		goto error;

	for (x = 0; x < w; x++) {
		map->tile[x] = calloc(h, sizeof(map_tile_t));
		if (map->tile[x] == NULL)
			goto error;
	}

	*rmap = map;
	return 0;
error:
	map_destroy(map);
	return ENOMEM;
}

/** Destroy map.
 *
 * @param map Icon menu
 */
void map_destroy(map_t *map)
{
	int x;

	if (map->tile != NULL) {
		for (x = 0; x < map->width; x++) {
			if (map->tile[x] != NULL)
				free(map->tile[x]);
		}
	}

	free(map->tile);
	free(map);
}

/** Set map origin.
 *
 * @param x X origin
 * @param y Y origin
 */
void map_set_orig(map_t *map, int x, int y)
{
	map->orig_x = x;
	map->orig_y = y;
}

/** Set map tile size.
 *
 * @param w Tile width
 * @param h Tile height
 */
void map_set_tile_size(map_t *map, int w, int h)
{
	map->tile_w = w;
	map->tile_h = h;
}

/** Set map tile margins.
 *
 * @param x Horizontal margin
 * @param y Vertical margin
 */
void map_set_tile_margins(map_t *map, int x, int y)
{
	map->margin_x = x;
	map->margin_y = y;
}

/** Set map callback.
 *
 * @param map Map
 * @param cb Callback
 * @param arg Callback argument
 */
void map_set_cb(map_t *map, map_cb_t cb, void *arg)
{
	map->cb = cb;
	map->cb_arg = arg;
}

/** Load tile images.
 *
 * @param map Map
 * @param fname Null-terminated list of file names
 * @return Zero on success or an error code
 */
int map_load_tile_img(map_t *map, const char **fname)
{
	int nimages;
	int i;
	const char **cp;
	SDL_Surface **images;

	/* Count number of entries */
	cp = fname;
	nimages = 0;
	while (*cp != NULL) {
		++nimages;
		++cp;
	}

	images = calloc(nimages, sizeof(SDL_Surface *));
	if (images == NULL)
		return ENOMEM;

	for (i = 0; i < nimages; i++) {
		printf("load %s\n", fname[i]);
		images[i] = SDL_LoadBMP(fname[i]);
		if (images[i] == NULL)
			goto error;
	}

	map->image = images;
	map->nimages = nimages;
	return 0;
error:
	for (i = 0; i < nimages; i++)
		if (images[i] != NULL)
			SDL_FreeSurface(images[i]);
	free(images);
	return EIO;
}

/** Draw map.
 *
 * @param map Icon menu
 * @param gfx Graphics object to draw to
 */
void map_draw(map_t *map, gfx_t *gfx)
{
	uint32_t color;
	int x, y;
	int dx, dy;
	map_tile_t ttype;
	SDL_Surface *surf;
	SDL_Rect drect;

	surf = SDL_GetWindowSurface(gfx->win);

	for (x = 0; x < map->width; x++) {
		dx = map->orig_x + (1 + x) * map->margin_x + x * map->tile_w;

		for (y = 0; y < map->height; y++) {
			dy = map->orig_y + (1 + y) * map->margin_y +
			    y * map->tile_h;

			color = gfx_rgb(gfx, 128 + 128 * map->tile[x][y], 0, 0);
			gfx_rect(gfx, dx, dy, map->tile_w, map->tile_h, color);
			drect.x = dx;
			drect.y = dy;
			drect.w = map->tile_w;
			drect.h = map->tile_h;

			ttype = map->tile[x][y];

			if ((int)ttype < map->nimages)
				SDL_BlitScaled(map->image[ttype], NULL, surf, &drect);
		}
	}
}

/** Process input event in map.
 *
 * @param map Icon menu
 * @param event Event
 * @return @c true if event is claimed
 */
bool map_event(map_t *map, SDL_Event *event)
{
	SDL_Rect drect;
	SDL_MouseButtonEvent *mbe;
	int x, y;

	drect.w = map->tile_w;
	drect.h = map->tile_h;

	for (x = 0; x < map->width; x++) {
		drect.x = map->orig_x + (x + 1) * map->margin_x +
		    x * map->tile_w;

		for (y = 0; y < map->height; y++) {
			drect.y = map->orig_y + (y + 1) * map->margin_y +
			    y * map->tile_h;

			if (event->type == SDL_MOUSEBUTTONDOWN) {
				mbe = (SDL_MouseButtonEvent *)event;
				if (mbe->x >= drect.x && mbe->y >= drect.y &&
				    mbe->x < drect.x + drect.w &&
				    mbe->y < drect.y + drect.h) {
					if (map->cb)
						map->cb(map->cb_arg, x, y);
					return true;
				}
			}
		}
	}

	return false;
}

/** Load map from file.
 *
 * @param f File
 * @param rmap Place to store pointer to new map
 * @return Zero on success or an error code
 */
int map_load(FILE *f, map_t **rmap)
{
	map_t *map;
	int x, y;
	int w, h;
	int nitem;
	int tile;
	int rc;

	nitem = fscanf(f, "%d %d\n\n", &w, &h);
	if (nitem != 2)
		return EIO;

	rc = map_create(w, h, &map);
	if (rc != 0)
		return rc;

	for (y = 0; y < map->height; y++) {
		for (x = 0; x < map->width; x++) {
			nitem = fscanf(f, "%d", &tile);
			if (nitem != 1)
				goto error;

			if (tile < 0 || tile > mapt_robot)
				goto error;

			map->tile[x][y] = (map_tile_t) tile;
		}
	}

	*rmap = map;
	return 0;
error:
	map_destroy(map);
	return EIO;
}

/** Save map to file.
 *
 * @param map Map
 * @param f File
 */
int map_save(map_t *map, FILE *f)
{
	int x, y;
	int rv;

	rv = fprintf(f, "%d %d\n\n", map->width, map->height);
	if (rv < 0)
		return EIO;

	for (y = 0; y < map->height; y++) {
		for (x = 0; x < map->width; x++) {
			rv = fprintf(f, "%d%c", map->tile[x][y],
			    x < map->width - 1 ? ' ' : '\n');
			if (rv < 0)
				return EIO;
		}
	}

	return 0;
}
