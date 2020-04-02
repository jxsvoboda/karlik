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
