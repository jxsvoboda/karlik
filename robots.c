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
 * Robots
 */

#include <assert.h>
#include <errno.h>
#include "robot.h"
#include "robots.h"

static robot_t *robots_first(robots_t *);
static robot_t *robots_next(robot_t *);

/** Create robots.
 *
 * @param map Map used by robots
 * @param rrobots Place to store pointer to new robots
 *
 * @return Zero on success or error code
 */
int robots_create(map_t *map, robots_t **rrobots)
{
	robots_t *robots;

	robots = calloc(1, sizeof(robots_t));
	if (robots == NULL)
		return ENOMEM;

	list_initialize(&robots->robots);
	robots->map = map;
	*rrobots = robots;
	return 0;
}

/** Destroy robots.
 *
 * @param robots Robots
 */
void robots_destroy(robots_t *robots)
{
	robot_t *robot;

	robot = robots_first(robots);
	while (robot != NULL) {
		list_remove(&robot->lrobots);
		robot_destroy(robot);

		robot = robots_first(robots);
	}

	free(robots);
}

/** Load robots.
 *
 * @param f File
 * @param map Map used by robots
 * @param rrobots Place to store pointer to loaded robots
 *
 * @return Zero on success or error code
 */
int robots_load(FILE *f, map_t *map, robots_t **rrobots)
{
	robots_t *robots = NULL;
	robot_t *robot;
	int rc;
	int nitem;
	unsigned long nrobots;
	unsigned long i;

	nitem = fscanf(f, "%lu\n", &nrobots);
	if (nitem != 1)
		return EIO;

	rc = robots_create(map, &robots);
	if (rc != 0)
		return rc;

	for (i = 0; i < nrobots; i++) {
		rc = robot_load(f, &robot);
		if (rc != 0)
			goto error;

		robot->robots = robots;
		list_append(&robot->lrobots, &robots->robots);
	}

	*rrobots = robots;
	return 0;
error:
	robots_destroy(robots);
	return rc;
}

/** Save robots.
 *
 * @param robots Robots
 */
int robots_save(robots_t *robots, FILE *f)
{
	robot_t *robot;
	int rv;
	int rc;

	rv = fprintf(f, "%lu\n", list_count(&robots->robots));
	if (rv < 0)
		return EIO;

	robot = robots_first(robots);
	while (robot != NULL) {
		rc = robot_save(robot, f);
		if (rc != 0)
			return rc;

		robot = robots_next(robot);
	}

	return 0;
}

/** Add new robot at the specified tile coordinates.
 *
 * @param robots
 * @param x X tile coordinate
 * @param y Y tile coordinate
 *
 * @return Zero on success, ENOMEM if out of memory, EEXIST if tile is occupied
 *         by another robot
 */
int robots_add(robots_t *robots, int x, int y)
{
	robot_t *oldr;
	robot_t *robot;
	int rc;

	oldr = robots_get(robots, x, y);
	if (oldr != NULL)
		return EEXIST;

	rc = robot_create(x, y, &robot);
	if (rc != 0) {
		assert(rc == ENOMEM);
		return ENOMEM;
	}

	robot->robots = robots;
	list_append(&robot->lrobots, &robots->robots);
	return 0;
}

/** Remove robot at the specified tile coordinates.
 *
 * If no robot is present at those coordinate, no action is performed.
 *
 * @param x X tile coordinate
 * @param y Y tile coordinate
 */
void robots_remove(robots_t *robots, int x, int y)
{
	robot_t *oldr;

	oldr = robots_get(robots, x, y);
	if (oldr == NULL)
		return;

	list_remove(&oldr->lrobots);
	robot_destroy(oldr);
}

/** Get first robot.
 *
 * @param robots Robot
 * @return First robot or @c NULL if there are no robots
 */
static robot_t *robots_first(robots_t *robots)
{
	link_t *link;

	link = list_first(&robots->robots);
	if (link == NULL)
		return NULL;

	return list_get_instance(link, robot_t, lrobots);
}

/** Get next robot.
 *
 * @param cur Current robot
 * @return Next robot or @c NULL if @a cur is the last
 */
static robot_t *robots_next(robot_t *cur)
{
	link_t *link;

	link = list_next(&cur->lrobots, &cur->robots->robots);
	if (link == NULL)
		return NULL;

	return list_get_instance(link, robot_t, lrobots);
}

/** Get robot by tile coordinates.
 *
 * @param robots Robots
 * @param x X tile coordinate
 * @param y Y tile coordinate
 * @return Robot or @c NULL if there is no robot at the specified position
 */
robot_t *robots_get(robots_t *robots, int x, int y)
{
	robot_t *robot;

	robot = robots_first(robots);
	while (robot != NULL) {
		if (robot->x == x && robot->y == y)
			return robot;
		robot = robots_next(robot);
	}

	return NULL;
}

/** Draw robots.
 *
 * @param robots Robots
 * @param orig_x X coordinate of origin on the screen
 * @param orig_y Y coordinate of origin on the screen
 * @param gfx Graphics
 */
void robots_draw(robots_t *robots, int orig_x, int orig_y, gfx_t *gfx)
{
	SDL_Rect drect;
	robot_t *robot;
	SDL_Surface *surf;

	if (robots->nimages < 1)
		return;

	surf = SDL_GetWindowSurface(gfx->win);

	robot = robots_first(robots);
	while (robot != NULL) {
		drect.x = orig_x + robots->tile_w * robot->x + robots->rel_x;
		drect.y = orig_y + robots->tile_h * robot->y + robots->rel_y;
		drect.w = robots->image[0]->w * 2;
		drect.h = robots->image[0]->h * 2;
		SDL_BlitScaled(robots->image[0], NULL, surf, &drect);

		robot = robots_next(robot);
	}
}

/** Load robot images.
 *
 * @param robots Robots
 * @param r Red component of color key
 * @param g Green component of color key
 * @param b Blue component of color key
 * @param fname Null-terminated list of file names
 * @return Zero on success or an error code
 */
int robots_load_img(robots_t *robots, int r, int g, int b,
    const char **fname)
{
	int nimages;
	int i;
	const char **cp;
	SDL_Surface **images;
	Uint32 key;

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

		key = SDL_MapRGB(images[i]->format, r, g, b);
		SDL_SetColorKey(images[i], SDL_TRUE, key);
	}

	robots->image = images;
	robots->nimages = nimages;
	return 0;
error:
	for (i = 0; i < nimages; i++)
		if (images[i] != NULL)
			SDL_FreeSurface(images[i]);
	free(images);
	return EIO;
}

/** Set tile size.
 *
 * @param w Tile width
 * @param h Tile height
 */
void robots_set_tile_size(robots_t *robots, int w, int h)
{
	robots->tile_w = w;
	robots->tile_h = h;
}

/** Set position relative to map tile.
 *
 * @param x X relative position
 * @param y Y relative position
 */
void robots_set_rel_pos(robots_t *robots, int x, int y)
{
	robots->rel_x = x;
	robots->rel_y = y;
}
