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
 * Program
 */

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "prog.h"

/** Create module.
 *
 * @param rmod Place to store pointer to new module
 * @return Zero or an error code
 */
int prog_module_create(prog_module_t **rmod)
{
	prog_module_t *mod;

	mod = calloc(1, sizeof(prog_module_t));
	if (mod == NULL)
		return ENOMEM;

	list_initialize(&mod->procs);

	*rmod = mod;
	return 0;
}

/** Destroy module.
 *
 * @param mod Module or @c NULL
 */
void prog_module_destroy(prog_module_t *mod)
{
	prog_proc_t *proc;

	if (mod == NULL)
		return;

	proc = prog_module_first(mod);
	while (proc != NULL) {
		list_remove(&proc->lprocs);
		proc->mod = NULL;

		prog_proc_destroy(proc);
		proc = prog_module_first(mod);
	}
}

/** Append procedure to module.
 *
 * @param mod Module
 * @param proc Procedure
 */
void prog_module_append(prog_module_t *mod, prog_proc_t *proc)
{
	list_append(&proc->lprocs, &mod->procs);
	proc->mod = mod;
}

/** Load module from file.
 *
 * @param f File
 * @param rmod Place to store pointer to new module
 */
int prog_module_load(FILE *f, prog_module_t **rmod)
{
	int nitem;
	unsigned cnt;
	unsigned i;
	prog_module_t *mod;
	prog_proc_t *proc;
	int rc;

	rc = prog_module_create(&mod);
	if (rc != 0)
		return rc;

	nitem = fscanf(f, "%u\n", &cnt);
	if (nitem != 1) {
		rc = EIO;
		goto error;
	}

	for (i = 0; i < cnt; i++) {
		rc = prog_proc_load(mod, f, &proc);
		if (rc != 0)
			goto error;

		prog_module_append(mod, proc);
	}

	*rmod = mod;
	return 0;
error:
	prog_module_destroy(mod);
	return rc;
}

/** Save module to file.
 *
 * @param mod Module
 * @param f File
 */
int prog_module_save(prog_module_t *mod, FILE *f)
{
	prog_proc_t *proc;
	unsigned cnt;
	int rc;
	int rv;

	cnt = list_count(&mod->procs);
	rv = fprintf(f, "%u\n", cnt);
	if (rv < 0)
		return EIO;

	proc = prog_module_first(mod);
	while (proc != NULL) {
		rc = prog_proc_save(proc, f);
		if (rc != 0)
			return rc;

		proc = prog_module_next(proc);
	}

	return 0;
}

/** Generate new procedure identifier.
 *
 * @param mod Module that will contain the procedure
 * @param rident Place to store pointer to newly allocated string
 * @return Zero on success, ENOMEM if out of memory
 */
int prog_module_gen_ident(prog_module_t *mod, char **rident)
{
	char *ident;
	unsigned i;
	prog_proc_t *proc;

	ident = malloc(prog_proc_id_len + 1);
	if (ident == NULL)
		return ENOMEM;

	do {
		/* Generate a new identifier */
		for (i = 0; i < prog_proc_id_len; i++) {
			ident[i] = 'A' + random() % 26;
		}

		proc = prog_module_proc_by_ident(mod, ident);
	} while (proc != NULL);

	ident[i] = '\0';
	*rident = ident;
	return 0;
}

/** Get first procedure in module.
 *
 * @param mod Module
 * @return First procedure in module or @c NULL if module is empty
 */
prog_proc_t *prog_module_first(prog_module_t *mod)
{
	link_t *link;

	link = list_first(&mod->procs);
	if (link == NULL)
		return NULL;

	return list_get_instance(link, prog_proc_t, lprocs);
}

/** Get next procedure in module.
 *
 * @param cur Current procedure
 * @return Next procedure or @c NULL if @a cur is the last procedure
 */
prog_proc_t *prog_module_next(prog_proc_t *cur)
{
	link_t *link;

	link = list_next(&cur->lprocs, &cur->mod->procs);
	if (link == NULL)
		return NULL;

	return list_get_instance(link, prog_proc_t, lprocs);
}

/** Get last procedure in module.
 *
 * @param mod Module
 * @return Last procedure in module or @c NULL if module is epmty
 */
prog_proc_t *prog_module_last(prog_module_t *mod)
{
	link_t *link;

	link = list_last(&mod->procs);
	if (link == NULL)
		return NULL;

	return list_get_instance(link, prog_proc_t, lprocs);
}

/** Get previous procedure in module.
 *
 * @param cur Current procedure
 * @return Previous procedure or @c NULL if @a cur is the first procedure
 */
prog_proc_t *prog_module_prev(prog_proc_t *cur)
{
	link_t *link;

	link = list_prev(&cur->lprocs, &cur->mod->procs);
	if (link == NULL)
		return NULL;

	return list_get_instance(link, prog_proc_t, lprocs);
}

/** Find procedure by identifier.
 *
 * @param mod Module
 * @param ident Identifier
 * @return Procedure or @c NULL if not found
 */
prog_proc_t *prog_module_proc_by_ident(prog_module_t *mod, const char *ident)
{
	prog_proc_t *proc;

	proc = prog_module_first(mod);
	while (proc != NULL) {
		if (strcmp(proc->ident, ident) == 0)
			return proc;

		proc = prog_module_next(proc);
	}

	return NULL;
}

/** Create procedure.
 *
 * @param ident Identifier
 * @param rproc Place to store pointer to new procedure
 * @return Zero on success or an error code
 */
int prog_proc_create(const char *ident, prog_proc_t **rproc)
{
	prog_proc_t *proc;
	int rc;

	proc = calloc(1, sizeof(prog_proc_t));
	if (proc == NULL)
		return ENOMEM;

	proc->ident = strdup(ident);
	if (proc->ident == NULL) {
		rc = ENOMEM;
		goto error;
	}

	*rproc = proc;
	return 0;
error:
	prog_proc_destroy(proc);
	return rc;
}

/** Destroy procedure.
 *
 * @param proc Procedure or @c NULL
 */
void prog_proc_destroy(prog_proc_t *proc)
{
	if (proc == NULL)
		return;

	if (proc->body != NULL)
		prog_block_destroy(proc->body);
	if (proc->ident != NULL)
		free(proc->ident);
	free(proc);
}

/** Load procedure identifier from file.
 *
 * @param f File
 * @param ident Array of prog_proc_id_len + 1 characters to hold identifier
 * @return Zero on success or an error code
 */
int prog_proc_load_ident(FILE *f, char *ident)
{
	unsigned i;
	int c;

	for (i = 0; i < prog_proc_id_len; i++) {
		c = fgetc(f);
		if (c < 0)
			return EIO;

		ident[i] = c;
	}

	ident[i] = '\0';

	c = fgetc(f);
	if (c != '\n')
		return EIO;

	return 0;
}

/** Save procedure identifier to file.
 *
 * @param ident Identifier
 * @param f File
 * @return Zero on success or an error code
 */
int prog_proc_save_ident(const char *ident, FILE *f)
{
	int rv;

	rv = fprintf(f, "%s\n", ident);
	if (rv < 0)
		return EIO;

	return 0;
}

/** Load procedure from file.
 *
 * @param mod Containing module
 * @param f File
 * @param rproc Place to store pointer to new procedure
 * @return Zero on success or an error code
 */
int prog_proc_load(prog_module_t *mod, FILE *f, prog_proc_t **rproc)
{
	prog_proc_t *proc;
	char ident[prog_proc_id_len + 1];
	int rc;

	rc = prog_proc_load_ident(f, ident);
	if (rc != 0)
		return rc;

	rc = prog_proc_create(ident, &proc);
	if (rc != 0)
		return rc;

	rc = prog_block_load(mod, f, &proc->body);
	if (rc != 0)
		goto error;

	*rproc = proc;
	return 0;
error:
	prog_proc_destroy(proc);
	return rc;
}

/** Save procedure to file.
 *
 * @param proc Procedure
 * @param f File
 * @return Zero on success or an error code
 */
int prog_proc_save(prog_proc_t *proc, FILE *f)
{
	int rc;

	rc = prog_proc_save_ident(proc->ident, f);
	if (rc != 0)
		return rc;

	rc = prog_block_save(proc->body, f);
	if (rc != 0)
		return rc;

	return 0;
}

/** Create block.
 *
 * @param rblock Place to store pointer to new block.
 * @return Zero on success or an error code
 */
int prog_block_create(prog_block_t **rblock)
{
	prog_block_t *block;

	block = calloc(1, sizeof(prog_block_t));
	if (block == NULL)
		return ENOMEM;

	list_initialize(&block->stmts);
	*rblock = block;
	return 0;
}

/** Destroy block.
 *
 * @param block Block or @c NULL
 */
void prog_block_destroy(prog_block_t *block)
{
	prog_stmt_t *stmt;

	if (block == NULL)
		return;

	stmt = prog_block_first(block);
	while (stmt != NULL) {
		list_remove(&stmt->lstmts);
		stmt->block = NULL;
		prog_stmt_destroy(stmt);

		stmt = prog_block_first(block);
	}

	free(block);
}

/** Append statement to block.
 *
 * @param block Block
 * @param stmt Statement
 */
void prog_block_append(prog_block_t *block, prog_stmt_t *stmt)
{
	list_append(&stmt->lstmts, &block->stmts);
	stmt->block = block;
}

/** Load block from file.
 *
 * @param mod Containing module
 * @param f File
 * @param rblock Place to store pointer to new block
 * @return Zero on success or an error code
 */
int prog_block_load(prog_module_t *mod, FILE *f, prog_block_t **rblock)
{
	int nitem;
	unsigned cnt;
	unsigned i;
	prog_block_t *block = NULL;
	prog_stmt_t *stmt;
	int rc;

	rc = prog_block_create(&block);
	if (rc != 0)
		goto error;

	nitem = fscanf(f, "%u\n", &cnt);
	if (nitem != 1) {
		rc = EIO;
		goto error;
	}

	for (i = 0; i < cnt; i++) {
		rc = prog_stmt_load(mod, f, &stmt);
		if (rc != 0)
			goto error;

		prog_block_append(block, stmt);
	}

	*rblock = block;
	return 0;
error:
	prog_block_destroy(block);
	return rc;
}

/** Save block to file.
 *
 * @param block Block
 * @param f File
 * @return Zero on success or an error code
 */
int prog_block_save(prog_block_t *block, FILE *f)
{
	prog_stmt_t *stmt;
	unsigned cnt;
	int rc;
	int rv;

	cnt = list_count(&block->stmts);
	rv = fprintf(f, "%u\n", cnt);
	if (rv < 0)
		return EIO;

	stmt = prog_block_first(block);
	while (stmt != NULL) {
		rc = prog_stmt_save(stmt, f);
		if (rc != 0)
			return rc;

		stmt = prog_block_next(stmt);
	}

	return 0;
}

/** Get first statement in block.
 *
 * @param block Block
 * @return First statement or @c NULL if block is empty
 */
prog_stmt_t *prog_block_first(prog_block_t *block)
{
	link_t *link;

	link = list_first(&block->stmts);
	if (link == NULL)
		return NULL;

	return list_get_instance(link, prog_stmt_t, lstmts);
}

/** Get next statement in block.
 *
 * @param cur Current statement
 * @return Next statement or @c NULL if @a cur is the last statement
 */
prog_stmt_t *prog_block_next(prog_stmt_t *cur)
{
	link_t *link;

	link = list_next(&cur->lstmts, &cur->block->stmts);
	if (link == NULL)
		return NULL;

	return list_get_instance(link, prog_stmt_t, lstmts);
}

/** Get last statement in block.
 *
 * @param block Block
 * @return Last statement or @c NULL if block is empty
 */
prog_stmt_t *prog_block_last(prog_block_t *block)
{
	link_t *link;

	link = list_last(&block->stmts);
	if (link == NULL)
		return NULL;

	return list_get_instance(link, prog_stmt_t, lstmts);
}

/** Get previous statement in block.
 *
 * @param cur Current statement
 * @return Previous statement or @c NULL if @a cur is the first statement
 */
prog_stmt_t *prog_block_prev(prog_stmt_t *cur)
{
	link_t *link;

	link = list_prev(&cur->lstmts, &cur->block->stmts);
	if (link == NULL)
		return NULL;

	return list_get_instance(link, prog_stmt_t, lstmts);
}

/** Get linear statement index within procedure.
 *
 * @param proc Procedure
 * @param stmt Statement inside procedure (must be inside @a proc)
 * @return Linear index
 */
unsigned prog_proc_get_stmt_index(prog_proc_t *proc, prog_stmt_t *stmt)
{
	prog_stmt_t *s;
	unsigned index;

	index = 0;
	s = prog_block_first(proc->body);

	while (s != stmt) {
		++index;
		s = prog_block_next(s);
	}

	assert(s == stmt);
	return index;
}

/** Find statement in procedure by linear index.
 *
 * @pram proc Procedure
 * @param index Linear index (must be valid)
 * @return Statement
 */
prog_stmt_t *prog_proc_stmt_by_index(prog_proc_t *proc, unsigned index)
{
	prog_stmt_t *stmt;
	unsigned i;

	i = 0;
	stmt = prog_block_first(proc->body);

	while (i != index) {
		++i;
		stmt = prog_block_next(stmt);
	}

	assert(i == index);
	return stmt;
}

/** Create intrinsic statement.
 *
 * @param itype Intrinsic type
 * @param rstmt Place to store pointer to new statement
 * @return Zero on succes or an error code
 */
int prog_stmt_intrinsic_create(prog_intr_type_t itype, prog_stmt_t **rstmt)
{
	prog_stmt_t *stmt;

	stmt = calloc(1, sizeof(prog_stmt_t));
	if (stmt == NULL)
		return ENOMEM;

	stmt->stype = progst_intrinsic;
	stmt->s.sintr.itype = itype;
	*rstmt = stmt;
	return 0;
}

/** Create call statement.
 *
 * @param proc Called procedure
 * @param rstmt Place to store pointer to new statement
 * @return Zero on succes or an error code
 */
int prog_stmt_call_create(prog_proc_t *proc, prog_stmt_t **rstmt)
{
	prog_stmt_t *stmt;

	stmt = calloc(1, sizeof(prog_stmt_t));
	if (stmt == NULL)
		return ENOMEM;

	stmt->stype = progst_call;
	stmt->s.scall.proc = proc;
	*rstmt = stmt;
	return 0;
}

/** Create if statement.
 *
 * @param rstmt Place to store pointer to new statement
 * @return Zero on succes or an error code
 */
int prog_stmt_if_create(prog_stmt_t **rstmt)
{
	prog_stmt_t *stmt;

	stmt = calloc(1, sizeof(prog_stmt_t));
	if (stmt == NULL)
		return ENOMEM;

	stmt->stype = progst_if;
	*rstmt = stmt;
	return 0;
}

/** Create repeat statement.
 *
 * @param rstmt Place to store pointer to new statement
 * @return Zero on succes or an error code
 */
int prog_stmt_repeat_create(prog_stmt_t **rstmt)
{
	prog_stmt_t *stmt;

	stmt = calloc(1, sizeof(prog_stmt_t));
	if (stmt == NULL)
		return ENOMEM;

	stmt->stype = progst_repeat;
	*rstmt = stmt;
	return 0;
}

/** Create recurse statement.
 *
 * @param rstmt Place to store pointer to new statement
 * @return Zero on succes or an error code
 */
int prog_stmt_recurse_create(prog_stmt_t **rstmt)
{
	prog_stmt_t *stmt;

	stmt = calloc(1, sizeof(prog_stmt_t));
	if (stmt == NULL)
		return ENOMEM;

	stmt->stype = progst_recurse;
	*rstmt = stmt;
	return 0;
}

/** Destroy statement.
 *
 * @param stmt Statement or @c NULL
 */
void prog_stmt_destroy(prog_stmt_t *stmt)
{
	if (stmt == NULL)
		return;

	free(stmt);
}

/** Load condition from file.
 *
 * @param f File
 * @param cond Condition
 * @return Zero on success or an error code
 */
static int prog_cond_load(FILE *f, prog_cond_t *cond)
{
	unsigned not;
	unsigned ctype;
	int nitem;

	nitem = fscanf(f, "%u %u\n", &not, &ctype);
	if (nitem != 2)
		return EIO;

	if (ctype > progct_south)
		return EIO;

	cond->not = (not != 0);
	cond->ctype = (prog_ctype_t)ctype;
	return 0;
}

/** Save condition to file.
 *
 * @param cond Condition
 * @param f File
 * @return Zero on success or an error code
 */
static int prog_cond_save(prog_cond_t *cond, FILE *f)
{
	int rv;

	rv = fprintf(f, "%u %u\n", cond->not ? 1 : 0, (unsigned)cond->ctype);
	if (rv < 0)
		return EIO;

	return 0;
}

/** Load intrinsic statement from file.
 *
 * @param f File
 * @param rstmt Place to store pointer to new statement
 * @return Zero on success or an error code
 */
static int prog_stmt_intrinsic_load(FILE *f, prog_stmt_t **rstmt)
{
	unsigned itype;
	int nitem;

	nitem = fscanf(f, "%u\n", &itype);
	if (nitem != 1)
		return EIO;

	if (itype > progin_pick_up)
		return EIO;

	return prog_stmt_intrinsic_create((prog_intr_type_t)itype, rstmt);
}

/** Save intrinsic statement to file.
 *
 * @param stmt Intrinsic statement
 * @param f File
 * @return Zero on success or an error code
 */
static int prog_stmt_intrinsic_save(prog_stmt_t *stmt, FILE *f)
{
	int rv;

	assert(stmt->stype == progst_intrinsic);

	rv = fprintf(f, "%u\n", stmt->s.sintr.itype);
	if (rv < 0)
		return EIO;

	return 0;
}

/** Load call statement from file.
 *
 * @param mod Containing module
 * @param f File
 * @param rstmt Place to store pointer to new statement
 * @return Zero on success or an error code
 */
static int prog_stmt_call_load(prog_module_t *mod, FILE *f, prog_stmt_t **rstmt)
{
	char ident[prog_proc_id_len + 1];
	prog_proc_t *proc;
	int rc;

	rc = prog_proc_load_ident(f, ident);
	if (rc != 0)
		return rc;

	proc = prog_module_proc_by_ident(mod, ident);
	if (proc == NULL)
		return EIO;

	return prog_stmt_call_create(proc, rstmt);
}

/** Save call statement to file.
 *
 * @param stmt Call statement
 * @param f File
 * @return Zero on success or an error code
 */
static int prog_stmt_call_save(prog_stmt_t *stmt, FILE *f)
{
	assert(stmt->stype == progst_call);

	return prog_proc_save_ident(stmt->s.scall.proc->ident, f);
}

/** Load if statement from file.
 *
 * @param mod Containing module
 * @param f File
 * @param rstmt Place to store pointer to new statement
 * @return Zero on success or an error code
 */
static int prog_stmt_if_load(prog_module_t *mod, FILE *f, prog_stmt_t **rstmt)
{
	prog_stmt_t *stmt = NULL;
	unsigned have_false;
	int nitem;
	int rc;

	rc = prog_stmt_if_create(&stmt);
	if (rc != 0)
		goto error;

	rc = prog_cond_load(f, &stmt->s.sif.cond);
	if (rc != 0)
		goto error;

	rc = prog_block_load(mod, f, &stmt->s.sif.btrue);
	if (rc != 0)
		goto error;

	nitem = fscanf(f, "%u\n", &have_false);
	if (nitem != 1)
		return EIO;

	if (have_false != 0) {
		rc = prog_block_load(mod, f, &stmt->s.sif.bfalse);
		if (rc != 0)
			goto error;
	}

	*rstmt = stmt;
	return 0;
error:
	prog_stmt_destroy(stmt);
	return rc;
}

/** Save if statement to file.
 *
 * @param mod Containing module
 * @param stmt If statement
 * @param f File
 * @return Zero on success or an error code
 */
static int prog_stmt_if_save(prog_stmt_t *stmt, FILE *f)
{
	int rc;
	int rv;

	assert(stmt->stype == progst_if);

	rc = prog_cond_save(&stmt->s.sif.cond, f);
	if (rc != 0)
		return rc;

	rc = prog_block_save(stmt->s.sif.btrue, f);
	if (rc != 0)
		return rc;

	/* Flag if we have a false branch */
	rv = fprintf(f, "%u\n", stmt->s.sif.bfalse != NULL ? 1 : 0);
	if (rv < 0)
		return EIO;

	if (stmt->s.sif.bfalse != NULL) {
		rc = prog_block_save(stmt->s.sif.bfalse, f);
		if (rc != 0)
			return rc;
	}

	return 0;
}

/** Load repeat statement from file.
 *
 * @param mod Containing module
 * @param f File
 * @param rstmt Place to store pointer to new statement
 * @return Zero on success or an error code
 */
static int prog_stmt_repeat_load(prog_module_t *mod, FILE *f,
    prog_stmt_t **rstmt)
{
	prog_stmt_t *stmt = NULL;
	unsigned repcnt;
	unsigned have_scond;
	unsigned have_econd;
	int nitem;
	int rc;

	rc = prog_stmt_repeat_create(&stmt);
	if (rc != 0)
		goto error;

	nitem = fscanf(f, "%u\n", &repcnt);
	if (nitem != 1)
		return EIO;

	nitem = fscanf(f, "%u\n", &have_scond);
	if (nitem != 1)
		return EIO;

	if (have_scond != 0) {
		rc = prog_cond_load(f, &stmt->s.srepeat.scond);
		if (rc != 0)
			goto error;

		stmt->s.srepeat.have_scond = true;
	}

	rc = prog_block_load(mod, f, &stmt->s.srepeat.body);
	if (rc != 0)
		goto error;

	nitem = fscanf(f, "%u\n", &have_econd);
	if (nitem != 1)
		return EIO;

	if (have_econd != 0) {
		rc = prog_cond_load(f, &stmt->s.srepeat.econd);
		if (rc != 0)
			goto error;

		stmt->s.srepeat.have_econd = true;
	}

	*rstmt = stmt;
	return 0;
error:
	prog_stmt_destroy(stmt);
	return rc;
}

/** Save repeat statement to file.
 *
 * @param stmt Repeat statement
 * @param f File
 * @return Zero on success or an error code
 */
static int prog_stmt_repeat_save(prog_stmt_t *stmt, FILE *f)
{
	int rc;
	int rv;

	assert(stmt->stype == progst_repeat);

	rv = fprintf(f, "%u\n", stmt->s.srepeat.repcnt);
	if (rv < 0)
		return EIO;

	/* Save whether we have a start condition */
	rv = fprintf(f, "%u\n", stmt->s.srepeat.have_scond ? 1 : 0);
	if (rv < 0)
		return EIO;

	if (stmt->s.srepeat.have_scond) {
		rc = prog_cond_save(&stmt->s.srepeat.scond, f);
		if (rc != 0)
			return rc;
	}

	rc = prog_block_save(stmt->s.srepeat.body, f);
	if (rc != 0)
		return rc;

	/* Save whether we have an end condition */
	rv = fprintf(f, "%u\n", stmt->s.srepeat.have_econd ? 1 : 0);
	if (rv < 0)
		return EIO;

	if (stmt->s.srepeat.have_econd) {
		rc = prog_cond_save(&stmt->s.srepeat.econd, f);
		if (rc != 0)
			return rc;
	}

	return 0;
}

/** Load recurse statement from file.
 *
 * @param f File
 * @param rstmt Place to store pointer to new statement
 * @return Zero on success or an error code
 */
static int prog_stmt_recurse_load(FILE *f, prog_stmt_t **rstmt)
{
	char c;
	int nitem;

	nitem = fscanf(f, "%c\n", &c);
	if (nitem != 1)
		return EIO;

	if (c != 'R')
		return EIO;

	return prog_stmt_recurse_create(rstmt);
}

/** Save recurse statement to file.
 *
 * @param stmt Recurse statement
 * @param f File
 * @return Zero on success or an error code
 */
static int prog_stmt_recurse_save(prog_stmt_t *stmt, FILE *f)
{
	int rv;

	assert(stmt->stype == progst_recurse);

	/* No more data to save, just terminate the line */
	rv = fprintf(f, "R\n");
	if (rv < 0)
		return EIO;

	return 0;
}

/** Load statement from file.
 *
 * @param mod Containing module
 * @param f File
 * @param rstmt Place to store pointer to new statement
 * @return Zero on success or an error code
 */
int prog_stmt_load(prog_module_t *mod, FILE *f, prog_stmt_t **rstmt)
{
	unsigned stype;
	int nitem;

	nitem = fscanf(f, "%u ", &stype);
	if (nitem != 1)
		return EIO;

	if (stype > progst_recurse)
		return EIO;

	switch (stype) {
	case progst_intrinsic:
		return prog_stmt_intrinsic_load(f, rstmt);
	case progst_call:
		return prog_stmt_call_load(mod, f, rstmt);
	case progst_if:
		return prog_stmt_if_load(mod, f, rstmt);
	case progst_repeat:
		return prog_stmt_repeat_load(mod, f, rstmt);
	case progst_recurse:
		return prog_stmt_recurse_load(f, rstmt);
	}

	return EINVAL;
}

/** Save statement to file.
 *
 * @param stmt Statement
 * @param f File
 * @return Zero on success or an error code
 */
int prog_stmt_save(prog_stmt_t *stmt, FILE *f)
{
	int rv;

	rv = fprintf(f, "%u ", (unsigned)stmt->stype);
	if (rv < 0)
		return EIO;

	switch (stmt->stype) {
	case progst_intrinsic:
		return prog_stmt_intrinsic_save(stmt, f);
	case progst_call:
		return prog_stmt_call_save(stmt, f);
	case progst_if:
		return prog_stmt_if_save(stmt, f);
	case progst_repeat:
		return prog_stmt_repeat_save(stmt, f);
	case progst_recurse:
		return prog_stmt_recurse_save(stmt, f);
	}

	/* Should not be reached */
	return EINVAL;
}
