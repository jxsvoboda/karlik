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
 * Map view
 */

#include <errno.h>
#include <SDL.h>
#include <stdbool.h>
#include "gfx.h"
#include "map.h"
#include "mapview.h"

enum {
	error_frame_width = 1
};

/** Create map view.
 *
 * @param map Map
 * @param robots Robots
 * @param rmapview Place to store pointer to new map view
 *
 * @return Zero on success or error code
 */
int mapview_create(map_t *map, robots_t *robots, mapview_t **rmapview)
{
	mapview_t *mapview;

	mapview = calloc(1, sizeof(mapview_t));
	if (mapview == NULL)
		return ENOMEM;

	mapview->map = map;
	mapview->robots = robots;
	*rmapview = mapview;
	return 0;
}

/** Destroy map view.
 *
 * @param mapview Map view
 */
void mapview_destroy(mapview_t *mapview)
{
	free(mapview);
}

/** Set map view origin.
 *
 * @param x X origin
 * @param y Y origin
 */
void mapview_set_orig(mapview_t *mapview, int x, int y)
{
	mapview->orig_x = x;
	mapview->orig_y = y;
}

/** Set map view callback.
 *
 * @param mapview Map view
 * @param cb Callback
 * @param arg Callback argument
 */
void mapview_set_cb(mapview_t *mapview, mapview_cb_t cb, void *arg)
{
	mapview->cb = cb;
	mapview->cb_arg = arg;
}

/** Draw error frame around map tile.
 *
 * Since this is implemented as a solid rectangle, it needs to be drawn
 * prior to the tile itself.
 *
 * @param mapview Map view
 * @param x X tile coordinate
 * @param y Y tile coordinate
 * @param gfx Graphics object to draw to
 */
void mapview_draw_error(mapview_t *mapview, int x, int y, gfx_t *gfx)
{
	int dx, dy;
	int fx, fy, fw, fh;
	uint32_t color;

	dx = mapview->orig_x + (1 + x) * mapview->map->margin_x +
	    x * mapview->map->tile_w;
	dy = mapview->orig_y + (1 + y) * mapview->map->margin_y +
	    y * mapview->map->tile_h;

	fx = dx - error_frame_width;
	fy = dy - error_frame_width;
	fw = mapview->map->tile_w + 2 * error_frame_width;
	fh = mapview->map->tile_h + 2 * error_frame_width;

	color = gfx_rgb(gfx, 255, 0, 0);
	gfx_rect(gfx, fx, fy, fw, fh, color);
}

/** Draw map view.
 *
 * @param mapview Map view
 * @param gfx Graphics object to draw to
 */
void mapview_draw(mapview_t *mapview, gfx_t *gfx)
{
	int x, y;
	int dx, dy;
	map_t *map;
	map_tile_t ttype;
	robot_t *robot;

	map = mapview->map;

	for (x = 0; x < map->width; x++) {
		dx = mapview->orig_x + (1 + x) * map->margin_x +
		    x * map->tile_w;

		for (y = 0; y < map->height; y++) {
			dy = mapview->orig_y + (1 + y) * map->margin_y +
			    y * map->tile_h;

			robot = robots_get(mapview->robots, x, y);
			if (robot != NULL && robot_error(robot))
				mapview_draw_error(mapview, x, y, gfx);

			ttype = map_get(map, x, y);

			if ((int)ttype < map->nimages)
				gfx_bmp_render(gfx, map->image[ttype], dx, dy);
		}
	}

	robots_draw(mapview->robots, mapview->orig_x, mapview->orig_y, gfx);
}

/** Process input event in map view.
 *
 * @param mapview Map view
 * @param event Event
 * @return @c true if event is claimed
 */
bool mapview_event(mapview_t *mapview, SDL_Event *event)
{
	SDL_MouseButtonEvent *mbe;
	map_t *map;
	int tx, ty;
	int x, y;
	int w, h;

	map = mapview->map;

	w = map->tile_w;
	h = map->tile_h;

	for (tx = 0; tx < map->width; tx++) {
		x = mapview->orig_x + (tx + 1) * map->margin_x +
		    tx * map->tile_w;

		for (ty = 0; ty < map->height; ty++) {
			y = mapview->orig_y + (ty + 1) * map->margin_y +
			    ty * map->tile_h;

			if (event->type == SDL_MOUSEBUTTONDOWN) {
				mbe = (SDL_MouseButtonEvent *)event;
				if (mbe->x >= x && mbe->y >= y &&
				    mbe->x < x + w && mbe->y < y + h) {
					if (mapview->cb != NULL) {
						mapview->cb(mapview->cb_arg,
						    tx, ty);
					}
					return true;
				}
			}
		}
	}

	return false;
}
