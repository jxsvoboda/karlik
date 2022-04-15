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
 * Robot
 */

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include "dir.h"
#include "robot.h"

/** Create new robot.
 *
 * @param x X tile coordinate
 * @param y Y tile coordinate
 * @parma dir Direction robot is facing
 * @param rrobot Place to store pointer to new robot
 */
int robot_create(int x, int y, dir_t dir, robot_t **rrobot)
{
	robot_t *robot;

	robot = calloc(1, sizeof(robot_t));
	if (robot == NULL)
		return ENOMEM;

	robot->x = x;
	robot->y = y;
	robot->dir = dir_south;
	*rrobot = robot;
	return 0;
}

/** Destroy robot.
 *
 * @param robot Robot
 */
void robot_destroy(robot_t *robot)
{
	free(robot);
}

/** Load robot.
 *
 * @param f File
 * @param rrobot Place to store pointer to loaded robot
 * @return Zero on success or an error code
 */
int robot_load(FILE *f, robot_t **rrobot)
{
	int nitem;
	int rc;
	int x, y, dir;
	robot_t *robot;

	nitem = fscanf(f, "%d %d %d\n", &x, &y, &dir);
	if (nitem != 3)
		return EIO;

	rc = robot_create(x, y, (dir_t)dir, &robot);
	if (rc != 0) {
		assert(rc == ENOMEM);
		return ENOMEM;
	}

	*rrobot = robot;
	return 0;
}

/** Save robot.
 *
 * @param robot Robot
 * @param f File
 * @return Zero on success or an error code
 */
int robot_save(robot_t *robot, FILE *f)
{
	int rv;

	rv = fprintf(f, "%d %d %d\n", robot->x, robot->y, robot->dir);
	if (rv < 0)
		return EIO;

	return 0;
}
