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
 * Robot stack
 */

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include "rstack.h"

static void rstack_entry_destroy(rstack_entry_t *);
static int rstack_entry_load(FILE *, rstack_t *);
static int rstack_entry_save(rstack_entry_t *, FILE *);

/** Create new robot stack.
 *
 * @param prog Program module
 * @param rrstack Place to store pointer to new robot stack
 */
int rstack_create(prog_module_t *prog, rstack_t **rrstack)
{
	rstack_t *rstack;

	rstack = calloc(1, sizeof(rstack_t));
	if (rstack == NULL)
		return ENOMEM;

	list_initialize(&rstack->entries);
	rstack->prog = prog;
	*rrstack = rstack;
	return 0;
}

/** Destroy robot stack.
 *
 * @param robot Robot stack
 */
void rstack_destroy(rstack_t *rstack)
{
	rstack_entry_t *entry;

	entry = rstack_first(rstack);
	while (entry != NULL) {
		rstack_entry_destroy(entry);
		entry = rstack_first(rstack);
	}

	free(rstack);
}

/** Load robot stack.
 *
 * @param prog Program module
 * @param f File
 * @param rrstack Place to store pointer to loaded robot stack
 * @return Zero on success or an error code
 */
int rstack_load(prog_module_t *prog, FILE *f, rstack_t **rrstack)
{
	int nitem;
	int rc;
	unsigned nentries;
	unsigned i;
	rstack_t *rstack;

	nitem = fscanf(f, "%u\n", &nentries);
	if (nitem != 1)
		return EIO;

	rc = rstack_create(prog, &rstack);
	if (rc != 0) {
		assert(rc == ENOMEM);
		return ENOMEM;
	}

	for (i = 0; i < nentries; i++) {
		rc = rstack_entry_load(f, rstack);
		if (rc != 0) {
			rstack_destroy(rstack);
			return rc;
		}
	}

	*rrstack = rstack;
	return 0;
}

/** Save robot stack.
 *
 * @param rstack Robot stack
 * @param f File
 * @return Zero on success or an error code
 */
int rstack_save(rstack_t *rstack, FILE *f)
{
	rstack_entry_t *entry;
	int rc;
	int rv;

	rv = fprintf(f, "%u\n", (unsigned)list_count(&rstack->entries));
	if (rv < 0)
		return EIO;

	entry = rstack_first(rstack);
	while (entry != NULL) {
		rc = rstack_entry_save(entry, f);
		if (rc != 0)
			return rc;

		entry = rstack_next(entry);
	}

	return 0;
}

/** Get first robot stack entry.
 *
 * @param rstack Robot stack
 * @return First entry or @c NULL if stack is empty
 */
rstack_entry_t *rstack_first(rstack_t *rstack)
{
	link_t *link;

	link = list_first(&rstack->entries);
	if (link == NULL)
		return NULL;

	return list_get_instance(link, rstack_entry_t, lentries);
}

/** Get next robot stack entry.
 *
 * @param cur Current entry
 * @return Next entry or @c NULL if @a cur is the last entry
 */
rstack_entry_t *rstack_next(rstack_entry_t *cur)
{
	link_t *link;

	link = list_next(&cur->lentries, &cur->rstack->entries);
	if (link == NULL)
		return NULL;

	return list_get_instance(link, rstack_entry_t, lentries);
}

/** Get last robot stack entry.
 *
 * @param rstack Robot stack
 * @return Last entry or @c NULL if stack is empty
 */
rstack_entry_t *rstack_last(rstack_t *rstack)
{
	link_t *link;

	link = list_last(&rstack->entries);
	if (link == NULL)
		return NULL;

	return list_get_instance(link, rstack_entry_t, lentries);
}

/** Get previous robot stack entry.
 *
 * @param cur Current entry
 * @return Previous entry or @c NULL if @a cur is the last entry
 */
rstack_entry_t *rstack_prev(rstack_entry_t *cur)
{
	link_t *link;

	link = list_prev(&cur->lentries, &cur->rstack->entries);
	if (link == NULL)
		return NULL;

	return list_get_instance(link, rstack_entry_t, lentries);
}

/** Load robot stack entry.
 *
 * @param f File
 * @param rstack Robot stack where to append the entry
 */
static int rstack_entry_load(FILE *f, rstack_t *rstack)
{
	char ident[prog_proc_id_len + 1];
	int nitem;
	int rc;
	unsigned stmt_index;
	prog_proc_t *proc;
	prog_stmt_t *stmt;

	rc = prog_proc_load_ident(f, ident);
	if (rc != 0)
		return rc;

	nitem = fscanf(f, "%u\n", &stmt_index);
	if (nitem != 1)
		return EIO;

	proc = prog_module_proc_by_ident(rstack->prog, ident);
	if (proc == NULL)
		return EIO;

	stmt = prog_proc_stmt_by_index(proc, stmt_index);
	if (stmt == NULL)
		return EIO;

	return rstack_push_caller(rstack, proc, stmt);
}

/** Save robot stack entry.
 *
 * @param entry Robot stack entry
 * @param f File
 * @return Zero on success or an error code
 */
static int rstack_entry_save(rstack_entry_t *entry, FILE *f)
{
	int rc;
	int rv;
	unsigned stmt_index;

	stmt_index = prog_proc_get_stmt_index(entry->caller_proc,
	    entry->caller_stmt);

	rc = prog_proc_save_ident(entry->caller_proc->ident, f);
	if (rc != 0)
		return rc;

	rv = fprintf(f, "%u\n", stmt_index);
	if (rv < 0)
		return EIO;

	return 0;
}

/** Destroy robot stack entry.
 *
 * @param entry Robot stack entry
 */
static void rstack_entry_destroy(rstack_entry_t *entry)
{
	list_remove(&entry->lentries);
	free(entry);
}

/** Push caller information to robot stack.
 *
 * @param rstack Robot stack
 * @param proc Calling procedure
 * @param stmt Calling statement
 *
 * @return Zero on success, ENOMEM if out of memory
 */
int rstack_push_caller(rstack_t *rstack, prog_proc_t *proc, prog_stmt_t *stmt)
{
	rstack_entry_t *entry;

	entry = calloc(1, sizeof(rstack_entry_t));
	if (entry == NULL)
		return ENOMEM;

	entry->rstack = rstack;
	list_append(&entry->lentries, &rstack->entries);
	entry->caller_proc = proc;
	entry->caller_stmt = stmt;
	return 0;
}

/** Pop caller information from robot stack.
 *
 * @param rstack Robot stack
 * @param rproc Place to store pointer to calling procedure
 * @param rstmt Place to store pointer to calling statement
 */
void rstack_pop_caller(rstack_t *rstack, prog_proc_t **rproc,
    prog_stmt_t **rstmt)
{
	rstack_entry_t *entry;

	entry = rstack_last(rstack);
	assert(entry != NULL);

	*rproc = entry->caller_proc;
	*rstmt = entry->caller_stmt;
	rstack_entry_destroy(entry);
}
