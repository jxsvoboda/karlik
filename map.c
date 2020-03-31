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

#include <assert.h>
#include <errno.h>
#include <SDL.h>
#include <stdbool.h>
#include "gfx.h"
#include "map.h"

/** Create map.
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
 * @param map Map
 */
void map_destroy(map_t *map)
{
	int x;
	int i;

	if (map->tile != NULL) {
		for (x = 0; x < map->width; x++) {
			if (map->tile[x] != NULL)
				free(map->tile[x]);
		}
	}

	free(map->tile);

	for (i = 0; i < map->nimages; i++) {
		if (map->image[i] != NULL)
			SDL_FreeSurface(map->image[i]);
	}

	free(map->image);
	free(map);
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

/** Set map tile.
 *
 * @param map Map
 * @param x X tile coordinate
 * @param y Y tile coordinate
 * @param ttype Tile type
 */
void map_set(map_t *map, int x, int y, map_tile_t ttype)
{
	assert(x >= 0);
	assert(y >= 0);
	assert(x < map->width);
	assert(y < map->height);

	map->tile[x][y] = ttype;
}

/** Get map tile.
 *
 * Get map tile contents. If the coordinates lie outside of the map,
 * return 'wall'.
 *
 * @param map Map
 * @param x X tile coordinate
 * @param y Y tile coordinate
 */
map_tile_t map_get(map_t *map, int x, int y)
{
	if (x < 0 && y < 0 && x >= map->width && y >= map->height)
		return mapt_wall;

	return map->tile[x][y];
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

			map_set(map, x, y, (map_tile_t) tile);
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
			rv = fprintf(f, "%d%c", map_get(map, x, y),
			    x < map->width - 1 ? ' ' : '\n');
			if (rv < 0)
				return EIO;
		}
	}

	return 0;
}
