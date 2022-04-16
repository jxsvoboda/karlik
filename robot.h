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

#include <stdio.h>
#include "adt/list.h"
#include "dir.h"

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
} robot_t;

extern int robot_create(int, int, dir_t, robot_t **);
extern void robot_destroy(robot_t *);
extern int robot_load(FILE *, robot_t **);
extern int robot_save(robot_t *, FILE *);
extern void robot_turn_left(robot_t *);
extern int robot_move(robot_t *);
extern int robot_put_white(robot_t *);
extern int robot_put_grey(robot_t *);
extern int robot_put_black(robot_t *);
extern int robot_pick_up(robot_t *);

#endif
