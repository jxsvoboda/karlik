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

#ifndef RSTACK_H
#define RSTACK_H

#include <stdio.h>
#include "adt/list.h"
#include "prog.h"

/** Robot stack */
typedef struct {
	/** Program module */
	prog_module_t *prog;
	/** Stack entries (robot_stack_entry_t) */
	list_t entries;
} rstack_t;

/** Robot stack entry */
typedef struct {
	/** Containing robot stack */
	rstack_t *rstack;
	/** Link to @c stack->entries */
	link_t lentries;
	/** Calling procedure */
	prog_proc_t *caller_proc;
	/** Calling statement */
	prog_stmt_t *caller_stmt;
} rstack_entry_t;

extern int rstack_create(prog_module_t *, rstack_t **);
extern void rstack_destroy(rstack_t *);
extern int rstack_load(prog_module_t *, FILE *, rstack_t **);
extern int rstack_save(rstack_t *, FILE *);
extern rstack_entry_t *rstack_first(rstack_t *);
extern rstack_entry_t *rstack_next(rstack_entry_t *);
extern rstack_entry_t *rstack_last(rstack_t *);
extern rstack_entry_t *rstack_prev(rstack_entry_t *);
extern int rstack_push_caller(rstack_t *, prog_proc_t *, prog_stmt_t *);
extern void rstack_pop_caller(rstack_t *, prog_proc_t **, prog_stmt_t **);

#endif
