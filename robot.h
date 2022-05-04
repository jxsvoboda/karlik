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

#ifndef ROBOT_H
#define ROBOT_H

#include <stdbool.h>
#include <stdio.h>
#include "adt/list.h"
#include "dir.h"
#include "prog.h"
#include "rstack.h"

/** Robot errors */
typedef enum {
	/** No error */
	errt_none,
	/** Robot tried to move to a square occupied by wall */
	errt_hit_wall,
	/** Robot tried to put a tag on a square already occupied by a tag */
	errt_already_tag,
	/** Robot tried to pick up a tag, but there was no tag */
	errt_no_tag
} robot_error_t;

/** Robot */
typedef struct {
	/** Containing robots structure */
	struct robots *robots;
	/** Link to @c robots->robots */
	link_t lrobots;
	/** Link to @c robots->dorder */
	link_t ldorder;
	/** X tile coordinate */
	int x;
	/** Y tile coordinate */
	int y;
	/** Direction robot is facing */
	dir_t dir;
	/** Current statement or @c NULL if not executing code */
	prog_stmt_t *cur_stmt;
	/** Current procedure */
	prog_proc_t *cur_proc;
	/** Robot stack */
	rstack_t *rstack;
	/** Was robot stopped due to error? */
	robot_error_t error;
} robot_t;

enum {
	errt_limit = errt_no_tag + 1
};

extern int robot_create(int, int, dir_t, rstack_t *, robot_t **);
extern void robot_destroy(robot_t *);
extern int robot_load(prog_module_t *, FILE *, robot_t **);
extern int robot_save(robot_t *, FILE *);
extern void robot_turn_left(robot_t *);
extern void robot_move(robot_t *);
extern void robot_put_white(robot_t *);
extern void robot_put_grey(robot_t *);
extern void robot_put_black(robot_t *);
extern void robot_pick_up(robot_t *);
extern int robot_run_proc(robot_t *, prog_proc_t *);
extern int robot_is_busy(robot_t *);
extern robot_error_t robot_error(robot_t *);
extern void robot_reset(robot_t *);
extern int robot_step(robot_t *);
extern prog_proc_t *robot_cur_proc(robot_t *);
extern prog_stmt_t *robot_cur_stmt(robot_t *);

#endif
