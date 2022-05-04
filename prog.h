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

#ifndef PROG_H
#define PROG_H

#include <stdbool.h>
#include <stdio.h>
#include "adt/list.h"

enum {
	/** Procedure identifier length */
	prog_proc_id_len = 8
};

/** Intrinsic type */
typedef enum {
	/** Turn left */
	progin_turn_left = 0,
	/** Move */
	progin_move = 1,
	/** Put white tag */
	progin_put_white = 2,
	/** Put grey tag */
	progin_put_grey = 3,
	/** Put black tag */
	progin_put_black = 4,
	/** Pick up */
	progin_pick_up = 5
} prog_intr_type_t;

enum {
	/** Number of intrinsic types */
	progin_limit = 6
};

/** Program statement type */
typedef enum {
	/** Intrinsic command */
	progst_intrinsic = 0,
	/** Call procedure */
	progst_call = 1,
	/** If statement */
	progst_if = 2,
	/** Repeat statement */
	progst_repeat = 3,
	/** Recurse */
	progst_recurse = 4
} prog_stmt_type_t;

/** Program condition type */
typedef enum {
	/** Wall */
	progct_wall,
	/** White tag */
	progct_wtag,
	/** Grey tag */
	progct_gtag,
	/** Black tag */
	progct_btag,
	/** Tag */
	progct_tag,
	/** East */
	progct_east,
	/** North */
	progct_north,
	/** West */
	progct_west,
	/** South */
	progct_south
} prog_ctype_t;

/** Program condition */
typedef struct {
	/** Is not? */
	bool not;
	/** Condition type */
	prog_ctype_t ctype;
} prog_cond_t;

/** Program block */
typedef struct {
	/** Statements */
	list_t stmts; /* of prog_stmt_t */
} prog_block_t;

/** Program module */
typedef struct {
	list_t procs; /* of prog_proc_t */
} prog_module_t;

/** Program procedure */
typedef struct {
	/** Containing module */
	prog_module_t *mod;
	/** Link to @c mod->procs */
	link_t lprocs;
	/** Body */
	prog_block_t *body;
	/** Icon identifier */
	char *ident;
} prog_proc_t;

/** Program intrinsic */
typedef struct {
	/** Intrinsic type */
	prog_intr_type_t itype;
} prog_intr_t;

/** Procedure call */
typedef struct {
	/** Procedure */
	prog_proc_t *proc;
} prog_call_t;

/** If statement */
typedef struct {
	/** Condition */
	prog_cond_t cond;
	/** True branch */
	prog_block_t *btrue;
	/** False branch */
	prog_block_t *bfalse;
} prog_if_t;

/** Repeat statement */
typedef struct {
	/** Repeat count or zero if not count-delimited */
	unsigned repcnt;
	/** Do we have a start condition? */
	bool have_scond;
	/** Loop start condition (valid if repcnt == 0) */
	prog_cond_t scond;
	/** Body */
	prog_block_t *body;
	/** Do we have an end condition? */
	bool have_econd;
	/** End contition */
	prog_cond_t econd;
} prog_repeat_t;

/** Program statement */
typedef struct {
	/** Statement type */
	prog_stmt_type_t stype;
	/** Containing block */
	prog_block_t *block;
	/** Link to @c block->stmts */
	link_t lstmts;
	union {
		/** Intrinsic statement */
		prog_intr_t sintr;
		/** Call statement */
		prog_call_t scall;
		/** If statement */
		prog_if_t sif;
		/** Repeat statement */
		prog_repeat_t srepeat;
	} s;
} prog_stmt_t;

extern int prog_module_create(prog_module_t **);
extern void prog_module_destroy(prog_module_t *);
extern void prog_module_append(prog_module_t *, prog_proc_t *);
extern int prog_module_load(FILE *, prog_module_t **);
extern int prog_module_save(prog_module_t *, FILE *);
extern int prog_module_gen_ident(prog_module_t *, char **);
extern prog_proc_t *prog_module_first(prog_module_t *);
extern prog_proc_t *prog_module_next(prog_proc_t *);
extern prog_proc_t *prog_module_last(prog_module_t *);
extern prog_proc_t *prog_module_prev(prog_proc_t *);
extern prog_proc_t *prog_module_proc_by_ident(prog_module_t *, const char *);
extern int prog_proc_create(const char *, prog_proc_t **);
extern void prog_proc_destroy(prog_proc_t *);
extern int prog_proc_load(prog_module_t *, FILE *, prog_proc_t **);
extern int prog_proc_save(prog_proc_t *, FILE *);
extern int prog_proc_load_ident(FILE *, char *);
extern int prog_proc_save_ident(const char *, FILE *);
extern unsigned prog_proc_get_stmt_index(prog_proc_t *, prog_stmt_t *);
extern prog_stmt_t *prog_proc_stmt_by_index(prog_proc_t *, unsigned);
extern int prog_block_create(prog_block_t **);
extern void prog_block_destroy(prog_block_t *);
extern void prog_block_append(prog_block_t *, prog_stmt_t *);
extern int prog_block_load(prog_module_t *, FILE *, prog_block_t **);
extern int prog_block_save(prog_block_t *, FILE *);
extern prog_stmt_t *prog_block_first(prog_block_t *);
extern prog_stmt_t *prog_block_next(prog_stmt_t *);
extern prog_stmt_t *prog_block_last(prog_block_t *);
extern prog_stmt_t *prog_block_prev(prog_stmt_t *);
extern int prog_stmt_intrinsic_create(prog_intr_type_t, prog_stmt_t **);
extern int prog_stmt_call_create(prog_proc_t *, prog_stmt_t **);
extern int prog_stmt_if_create(prog_stmt_t **);
extern int prog_stmt_repeat_create(prog_stmt_t **);
extern int prog_stmt_recurse_create(prog_stmt_t **);
extern void prog_stmt_destroy(prog_stmt_t *);
extern int prog_stmt_load(prog_module_t *, FILE *, prog_stmt_t **);
extern int prog_stmt_save(prog_stmt_t *, FILE *);

#endif
