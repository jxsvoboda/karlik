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
#include <stdbool.h>
#include <stdlib.h>
#include "dir.h"
#include "map.h"
#include "robot.h"
#include "robots.h"

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
	robot->dir = dir;
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
	unsigned error;
	robot_t *robot;

	nitem = fscanf(f, "%d %d %d %u\n", &x, &y, &dir, &error);
	if (nitem != 4)
		return EIO;

	rc = robot_create(x, y, (dir_t)dir, &robot);
	if (rc != 0) {
		assert(rc == ENOMEM);
		return ENOMEM;
	}

	if (error != 0)
		robot->error = true;

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

	rv = fprintf(f, "%d %d %d %u\n", robot->x, robot->y, robot->dir,
	    robot->error ? 1 : 0);
	if (rv < 0)
		return EIO;

	return 0;
}

/** Turn robot left.
 *
 * @param robot Robot
 */
void robot_turn_left(robot_t *robot)
{
	robot->dir = dir_next_ccw(robot->dir);
}

/** Move robot one square forward.
 *
 * @param robot Robot
 */
void robot_move(robot_t *robot)
{
	map_tile_t tile;
	int xoff, yoff;

	/* Get tile in front of robot */
	dir_get_off(robot->dir, &xoff, &yoff);
	tile = map_get(robot->robots->map, robot->x + xoff, robot->y + yoff);

	if (!map_tile_walkable(tile)) {
		robot->error = true;
		return;
	}

	robots_move_robot(robot->robots, robot, xoff, yoff);
}

/** Put down white tag.
 *
 * @param robot Robot
 */
void robot_put_white(robot_t *robot)
{
	map_tile_t tile;

	/* Get tile under robot */
	tile = map_get(robot->robots->map, robot->x, robot->y);

	if (tile != mapt_none) {
		robot->error = true;
		return;
	}

	map_set(robot->robots->map, robot->x, robot->y, mapt_wtag);
}

/** Put down grey tag.
 *
 * @param robot Robot
 */
void robot_put_grey(robot_t *robot)
{
	map_tile_t tile;

	/* Get tile under robot */
	tile = map_get(robot->robots->map, robot->x, robot->y);

	if (tile != mapt_none) {
		robot->error = true;
		return;
	}

	map_set(robot->robots->map, robot->x, robot->y, mapt_gtag);
}

/** Put down black tag.
 *
 * @param robot Robot
 */
void robot_put_black(robot_t *robot)
{
	map_tile_t tile;

	/* Get tile under robot */
	tile = map_get(robot->robots->map, robot->x, robot->y);

	if (tile != mapt_none) {
		robot->error = true;
		return;
	}

	map_set(robot->robots->map, robot->x, robot->y, mapt_btag);
}

/** Pick up tag.
 *
 * @param robot Robot
 */
void robot_pick_up(robot_t *robot)
{
	map_tile_t tile;

	/* Get tile under robot */
	tile = map_get(robot->robots->map, robot->x, robot->y);

	if (!map_tile_tag(tile)) {
		robot->error = true;
		return;
	}

	map_set(robot->robots->map, robot->x, robot->y, mapt_none);
}

/** Start executing procedure.
 *
 * Start executing program (run a procedure).
 *
 * @param robot Robot
 * @param proc Procedure
 * @return Zero on success. EBUSY if robot is already busy executing code
 *         or stopped due to error.
 */
int robot_run_proc(robot_t *robot, prog_proc_t *proc)
{
	if (robot->cur_stmt != NULL || robot->error)
		return EBUSY;

	robot->cur_stmt = prog_block_first(proc->body);
	return 0;
}

/** Determine if robot is busy executing code.
 *
 * @param robot Robot
 * @return Non-zero if robot is executing code, zero if it is not.
 */
int robot_is_busy(robot_t *robot)
{
	return robot->cur_stmt != NULL;
}

/** Determine if robot is stopped due to error.
 *
 * @param robot Robot
 * @return Non-zero if robot is executing code, zero if it is not.
 */
int robot_error(robot_t *robot)
{
	return robot->error;
}

void robot_clear_error(robot_t *robot)
{
	robot->error = false;
}

/** Step intrinsic statement.
 *
 * Steps the next statement, which must be an intrinsic statement.
 * @param robot Robot
 */
static void robot_stmt_intrinsic(robot_t *robot)
{
	assert(robot->cur_stmt->stype == progst_intrinsic);

	switch (robot->cur_stmt->s.sintr.itype) {
	case progin_turn_left:
		robot_turn_left(robot);
		break;
	case progin_move:
		robot_move(robot);
		break;
	case progin_put_white:
		robot_put_white(robot);
		break;
	case progin_put_grey:
		robot_put_grey(robot);
		break;
	case progin_put_black:
		robot_put_black(robot);
		break;
	case progin_pick_up:
		robot_pick_up(robot);
		break;
	}

	if (robot->error)
		return;

	robot->cur_stmt = prog_block_next(robot->cur_stmt);
}

/** Advance one step in robot execution.
 *
 * @parm robot Robot
 * @return Zero on success or an error code
 */
int robot_step(robot_t *robot)
{
	if (robot->cur_stmt == NULL)
		return EINVAL;
	if (robot->error)
		return EINVAL;

	switch (robot->cur_stmt->stype) {
	case progst_intrinsic:
		robot_stmt_intrinsic(robot);
		break;
	default:
		return ENOTSUP;
	}

	return 0;
}
