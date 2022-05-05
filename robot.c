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
#include "prog.h"
#include "robot.h"
#include "robots.h"
#include "rstack.h"

/** Create new robot.
 *
 * @param x X tile coordinate
 * @param y Y tile coordinate
 * @param dir Direction robot is facing
 * @param rstack Robot stack
 * @param rrobot Place to store pointer to new robot
 */
int robot_create(int x, int y, dir_t dir, rstack_t *rstack, robot_t **rrobot)
{
	robot_t *robot;

	robot = calloc(1, sizeof(robot_t));
	if (robot == NULL)
		return ENOMEM;

	robot->x = x;
	robot->y = y;
	robot->dir = dir;
	robot->rstack = rstack;
	*rrobot = robot;
	return 0;
}

/** Destroy robot.
 *
 * @param robot Robot
 */
void robot_destroy(robot_t *robot)
{
	rstack_destroy(robot->rstack);
	free(robot);
}

/** Load robot.
 *
 * @param prog Program module
 * @param f File
 * @param rrobot Place to store pointer to loaded robot
 * @return Zero on success or an error code
 */
int robot_load(prog_module_t *prog, FILE *f, robot_t **rrobot)
{
	int nitem;
	int rc;
	int x, y, dir;
	unsigned error;
	robot_t *robot;
	rstack_t *rstack;

	nitem = fscanf(f, "%d %d %d %u\n", &x, &y, &dir, &error);
	if (nitem != 4)
		return EIO;

	if (error >= errt_limit)
		return EIO;

	rc = rstack_load(prog, f, &rstack);
	if (rc != 0)
		return rc;

	rc = robot_create(x, y, (dir_t)dir, rstack, &robot);
	if (rc != 0) {
		rstack_destroy(rstack);
		assert(rc == ENOMEM);
		return ENOMEM;
	}

	if (error != 0)
		robot->error = error;

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

	return rstack_save(robot->rstack, f);
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
		robot->error = errt_hit_wall;
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
		robot->error = errt_already_tag;
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
		robot->error = errt_already_tag;
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
		robot->error = errt_already_tag;
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
		robot->error = errt_no_tag;
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

	robot->cur_proc = proc;
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
 * @return Robot error code if robot is stopped or errt_none if it is not.
 */
robot_error_t robot_error(robot_t *robot)
{
	return robot->error;
}

/** Clear robot error and run state.
 *
 * @param robot Robot
 */
void robot_reset(robot_t *robot)
{
	robot->error = errt_none;
	robot->cur_stmt = NULL;
}

/** Leave statement block.
 *
 * @param robot Robot
 */
static void robot_leave(robot_t *robot)
{
	prog_proc_t *next_proc;
	prog_stmt_t *next_stmt;

	if (rstack_is_empty(robot->rstack)) {
		robot->cur_proc = NULL;
		robot->cur_stmt = NULL;
		return;
	}

	rstack_pop_cont(robot->rstack, &next_proc, &next_stmt);

	robot->cur_proc = next_proc;
	robot->cur_stmt = next_stmt;
}

/** Execute intrinsic statement.
 *
 * Executes the next statement, which must be an intrinsic statement.
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
	if (robot->cur_stmt == NULL) {
		/* End of block */
		robot_leave(robot);
	}
}

/** Execeute call statement.
 *
 * Executes the next statement, which must be a call statement.
 * @param robot Robot
 * @return Zero on success or an error code
 */
static int robot_stmt_call(robot_t *robot)
{
	prog_stmt_t *scall;
	prog_stmt_t *snext;
	int rc;

	scall = robot->cur_stmt;
	assert(scall->stype == progst_call);

	snext = prog_block_next(scall);
	if (snext == NULL) {
		/* Tail call - do nothing */
	} else {
		/* Push next statement position */
		rc = rstack_push_cont(robot->rstack, robot->cur_proc,
		    snext);
		if (rc != 0)
			return rc;
	}

	/* Set current program position */
	robot->cur_proc = scall->s.scall.proc;
	robot->cur_stmt = prog_block_first(robot->cur_proc->body);

	return 0;
}

/** Advance one step in robot execution.
 *
 * @parm robot Robot
 * @return Zero on success or an error code
 */
int robot_step(robot_t *robot)
{
	int rc = 0;

	if (robot->cur_stmt == NULL)
		return EINVAL;
	if (robot->error)
		return EINVAL;

	switch (robot->cur_stmt->stype) {
	case progst_intrinsic:
		robot_stmt_intrinsic(robot);
		break;
	case progst_call:
		rc = robot_stmt_call(robot);
		break;
	default:
		return ENOTSUP;
	}

	return rc;
}

/** Return current procedure.
 *
 * @param robot Robot
 * @return Procedure currently being executed
 */
prog_proc_t *robot_cur_proc(robot_t *robot)
{
	return robot->cur_proc;
}

/** Return current statement.
 *
 * @param robot Robot
 * @return Current statement
 */
prog_stmt_t *robot_cur_stmt(robot_t *robot)
{
	return robot->cur_stmt;
}
