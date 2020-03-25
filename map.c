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

	for (x = 0; x < map->width; x++) {
		if (map->tile[x] != NULL)
			free(map->tile[x]);
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

	for (x = 0; x < map->width; x++) {
		dx = map->orig_x + (1 + x) * map->margin_x + x * map->tile_w;

		for (y = 0; y < map->height; y++) {
			dy = map->orig_y + (1 + y) * map->margin_y +
			    y * map->tile_h;

			color = gfx_rgb(gfx, 128 + 128 * map->tile[x][y], 0, 0);
			gfx_rect(gfx, dx, dy, map->tile_w, map->tile_h, color);
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

int map_load(FILE *f, map_t **rmap)
{
	return ENOTSUP;
}

/** Save map to file.
 *
 * @param map Map
 * @param f File
 */
int map_save(map_t *map, FILE *f)
{
	return 0;
}
