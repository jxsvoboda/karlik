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
 * Robots
 */

#include <assert.h>
#include <errno.h>
#include "dir.h"
#include "prog.h"
#include "robot.h"
#include "robots.h"

static void robots_add_robot(robots_t *, robot_t *);

/** Create robots.
 *
 * @param prog Program module used by robots
 * @param map Map used by robots
 * @param rrobots Place to store pointer to new robots
 *
 * @return Zero on success or error code
 */
int robots_create(prog_module_t *prog, map_t *map, robots_t **rrobots)
{
	robots_t *robots;

	robots = calloc(1, sizeof(robots_t));
	if (robots == NULL)
		return ENOMEM;

	list_initialize(&robots->robots);
	list_initialize(&robots->dorder);
	robots->prog = prog;
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
 * @param prog Program module
 * @param map Map used by robots
 * @param rrobots Place to store pointer to loaded robots
 *
 * @return Zero on success or error code
 */
int robots_load(FILE *f, prog_module_t *prog, map_t *map, robots_t **rrobots)
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

	rc = robots_create(prog, map, &robots);
	if (rc != 0)
		return rc;

	for (i = 0; i < nrobots; i++) {
		rc = robot_load(prog, f, &robot);
		if (rc != 0)
			goto error;

		robots_add_robot(robots, robot);
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
 * @param robots Robots
 * @param robot Robot
 */
static void robots_add_robot(robots_t *robots, robot_t *robot)
{
	robot_t *oldr;

	oldr = robots_dorder_first(robots);
	while (oldr != NULL && oldr->y < robot->y)
		oldr = robots_dorder_next(oldr);

	robot->robots = robots;
	list_append(&robot->lrobots, &robots->robots);

	if (oldr != NULL)
		list_insert_before(&robot->ldorder, &oldr->ldorder);
	else
		list_append(&robot->ldorder, &robots->dorder);
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
	rstack_t *rstack;
	int rc;

	oldr = robots_get(robots, x, y);
	if (oldr != NULL)
		return EEXIST;

	rc = rstack_create(robots->prog, &rstack);
	if (rc != 0)
		return rc;

	rc = robot_create(x, y, dir_south, rstack, &robot);
	if (rc != 0) {
		rstack_destroy(rstack);
		assert(rc == ENOMEM);
		return ENOMEM;
	}

	robots_add_robot(robots, robot);
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
	list_remove(&oldr->ldorder);
	robot_destroy(oldr);
}

/** Change robot position.
 *
 * @param robots Robots
 * @param robot Robot
 * @param dx X coordinate change
 * @param dy Y coordinate change
 */
void robots_move_robot(robots_t *robots, robot_t *robot, int dx, int dy)
{
	robot_t *next;

	printf("robots_move_robot(robot=%p dx=%d dy=%d\n",
	    robot, dx, dy);
	if (dy < 0) {
		next = robots_dorder_prev(robot);
		while (next != NULL && robot->y + dy < next->y)
			next = robots_dorder_prev(next);

		list_remove(&robot->ldorder);
		if (next != NULL)
			list_insert_before(&robot->ldorder, &next->ldorder);
		else
			list_prepend(&robot->ldorder, &robots->dorder);
	} else {
		next = robots_dorder_next(robot);
		while (next != NULL && robot->y + dy > next->y)
			next = robots_dorder_next(next);

		list_remove(&robot->ldorder);
		if (next != NULL)
			list_insert_after(&robot->ldorder, &next->ldorder);
		else
			list_append(&robot->ldorder, &robots->dorder);
	}

	robot->x += dx;
	robot->y += dy;
}

/** Get first robot.
 *
 * @param robots Robot
 * @return First robot or @c NULL if there are no robots
 */
robot_t *robots_first(robots_t *robots)
{
	link_t *link;

	link = list_first(&robots->robots);
	if (link == NULL)
		return NULL;

	return list_get_instance(link, robot_t, lrobots);
}

/** Get last robot.
 *
 * @param robots Robot
 * @return Last robot or @c NULL if there are no robots
 */
robot_t *robots_last(robots_t *robots)
{
	link_t *link;

	link = list_last(&robots->robots);
	if (link == NULL)
		return NULL;

	return list_get_instance(link, robot_t, lrobots);
}

/** Get next robot.
 *
 * @param cur Current robot
 * @return Next robot or @c NULL if @a cur is the last
 */
robot_t *robots_next(robot_t *cur)
{
	link_t *link;

	link = list_next(&cur->lrobots, &cur->robots->robots);
	if (link == NULL)
		return NULL;

	return list_get_instance(link, robot_t, lrobots);
}

/** Get previous robot.
 *
 * @param cur Current robot
 * @return Previous robot or @c NULL if @a cur is the first
 */
robot_t *robots_prev(robot_t *cur)
{
	link_t *link;

	link = list_prev(&cur->lrobots, &cur->robots->robots);
	if (link == NULL)
		return NULL;

	return list_get_instance(link, robot_t, lrobots);
}

/** Get first robot in display order.
 *
 * @param robots Robot
 * @return First robot in display order or @c NULL if there are no robots
 */
robot_t *robots_dorder_first(robots_t *robots)
{
	link_t *link;

	link = list_first(&robots->dorder);
	if (link == NULL)
		return NULL;

	return list_get_instance(link, robot_t, ldorder);
}

/** Get last robot in display order.
 *
 * @param robots Robot
 * @return Last robot in display order or @c NULL if there are no robots
 */
robot_t *robots_dorder_last(robots_t *robots)
{
	link_t *link;

	link = list_last(&robots->dorder);
	if (link == NULL)
		return NULL;

	return list_get_instance(link, robot_t, ldorder);
}

/** Get next robot in display order.
 *
 * @param cur Current robot
 * @return Next robot in display order or @c NULL if @a cur is the last
 */
robot_t *robots_dorder_next(robot_t *cur)
{
	link_t *link;

	link = list_next(&cur->ldorder, &cur->robots->dorder);
	if (link == NULL)
		return NULL;

	return list_get_instance(link, robot_t, ldorder);
}

/** Get previous robot in display order.
 *
 * @param cur Current robot
 * @return Previous robot in display order or @c NULL if @a cur is the first
 */
robot_t *robots_dorder_prev(robot_t *cur)
{
	link_t *link;

	link = list_prev(&cur->ldorder, &cur->robots->dorder);
	if (link == NULL)
		return NULL;

	return list_get_instance(link, robot_t, ldorder);
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
	robot_t *robot;
	int x, y;
	int dir;

	if (robots->nimages < 4)
		return;

	robot = robots_dorder_first(robots);
	while (robot != NULL) {
		x = orig_x + robots->tile_w * robot->x + robots->rel_x;
		y = orig_y + robots->tile_h * robot->y + robots->rel_y;
		dir = (int)robot->dir;

		gfx_bmp_render(gfx, robots->image[dir], x, y);

		robot = robots_dorder_next(robot);
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
	gfx_bmp_t **images;
	int rc;

	/* Count number of entries */
	cp = fname;
	nimages = 0;
	while (*cp != NULL) {
		++nimages;
		++cp;
	}

	images = calloc(nimages, sizeof(gfx_bmp_t *));
	if (images == NULL)
		return ENOMEM;

	for (i = 0; i < nimages; i++) {
		rc = gfx_bmp_load(fname[i], &images[i]);
		if (rc != 0)
			goto error;

		gfx_bmp_set_color_key(images[i], r, g, b);
	}

	robots->image = images;
	robots->nimages = nimages;
	return 0;
error:
	for (i = 0; i < nimages; i++)
		if (images[i] != NULL)
			gfx_bmp_destroy(images[i]);
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
