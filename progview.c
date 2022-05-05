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
 * Program view
 */

#include <assert.h>
#include <errno.h>
#include <SDL.h>
#include <stdbool.h>
#include "gfx.h"
#include "prog.h"
#include "progview.h"

enum {
	/** Number of icons to display on one line */
	progview_columns = 6
};

/** Intrinsic statement icon files */
static const char *intr_icon_files[] = {
	[progin_move] = "img/verb/move.bmp",
	[progin_turn_left] = "img/verb/turnleft.bmp",
	[progin_put_white] = "img/verb/putwhite.bmp",
	[progin_put_grey] = "img/verb/putgrey.bmp",
	[progin_put_black] = "img/verb/putblack.bmp",
	[progin_pick_up] = "img/verb/pickup.bmp"
};

/** Create program view.
 *
 * @param icondict Icon dictionary
 * @param rprogview Place to store pointer to new program view
 *
 * @return Zero on success or error code
 */
int progview_create(icondict_t *icondict, progview_t **rprogview)
{
	progview_t *progview;
	int rc;
	unsigned i;

	progview = calloc(1, sizeof(progview_t));
	if (progview == NULL)
		return ENOMEM;

	for (i = 0; i < progin_limit; i++) {
		printf("Load '%s'\n", intr_icon_files[i]);
		rc = gfx_bmp_load(intr_icon_files[i], &progview->intr_img[i]);
		if (rc != 0)
			goto error;
	}

	progview->icondict = icondict;
	*rprogview = progview;
	return 0;
error:
	free(progview);
	return rc;
}

/** Destroy program view.
 *
 * @param progview Program view
 */
void progview_destroy(progview_t *progview)
{
	unsigned i;

	for (i = 0; i < progin_limit; i++)
		gfx_bmp_destroy(progview->intr_img[i]);
	free(progview);
}

/** Set program view origin.
 *
 * @param x X origin
 * @param y Y origin
 */
void progview_set_orig(progview_t *progview, int x, int y)
{
	progview->orig_x = x;
	progview->orig_y = y;
}

/** Set program view callbacks.
 *
 * @param progview Program view
 * @param cb Callbacks
 * @param arg Callback argument
 */
void progview_set_cb(progview_t *progview, progview_cb_t *cb, void *arg)
{
	progview->cb = cb;
	progview->cb_arg = arg;
}

/** Set the displayed procedure.
 *
 * @param progview Program view
 * @param proc Procedure to display or @c NULL
 */
void progview_set_proc(progview_t *progview, prog_proc_t *proc)
{
	progview->proc = proc;
}

/** Set which statement should be highlighted.
 *
 * @param progview Program view
 * @param hgl_stmt Highlighted statement or @c NULL
 */
void progview_set_hgl_stmt(progview_t *progview, prog_stmt_t *stmt)
{
	progview->hgl_stmt = stmt;
}

/** Get the displayed procedure.
 *
 * @param progview Program view
 * @retur Displayed procedure or @c NULL
 */
prog_proc_t *progview_get_proc(progview_t *progview)
{
	return progview->proc;
}

/** Draw program view.
 *
 * @param progview Program view
 * @param gfx Graphics object to draw to
 */
void progview_draw(progview_t *progview, gfx_t *gfx)
{
	int x, y;
	int dx, dy;
	prog_proc_t *proc;
	prog_stmt_t *stmt;
	gfx_bmp_t *bmp;
	icondict_entry_t *entry;
	uint32_t color;

	proc = progview->proc;
	if (proc == NULL)
		return;

	/* Current procedure icon */
	entry = icondict_find(progview->icondict, proc->ident);
	printf("ident='%s' entry=%p\n", proc->ident, entry);
	if (entry != NULL) {
		bmp = entry->icon->bmp;
		gfx_bmp_render(gfx, bmp, progview->orig_x, progview->orig_y);
	}

	x = 0;
	y = 1;

	stmt = prog_block_first(proc->body);
	while (stmt != NULL) {
		dx = progview->orig_x + (1 + x) * progview->margin_x +
		    x * progview->icon_w;
		dy = progview->orig_y + (1 + y) * progview->margin_y +
		    y * progview->icon_h;

		if (stmt->stype == progst_intrinsic || stmt->stype == progst_call) {
			if (stmt->stype == progst_intrinsic) {
				bmp = progview->intr_img[stmt->s.sintr.itype];
			} else {
				printf("ident='%s'\n",
				    stmt->s.scall.proc->ident);
				entry = icondict_find(progview->icondict,
				    stmt->s.scall.proc->ident);
				assert(entry != NULL);
				bmp = entry->icon->bmp;
			}
			if (stmt == progview->hgl_stmt) {
				color = gfx_rgb(gfx, 0, 255, 255);
				gfx_rect(gfx, dx - 1, dy - 1,
				    progview->icon_w + 2, progview->icon_h + 2,
				    color);
			}
			gfx_bmp_render(gfx, bmp, dx, dy);
		}

		++x;
		if (x >= progview_columns) {
			x = 0;
			++y;
		}
		stmt = prog_block_next(stmt);
	}
}

/** Process input event in program view.
 *
 * @param progview Program view
 * @param event Event
 * @return @c true if event is claimed
 */
bool progview_event(progview_t *progview, SDL_Event *event)
{
	(void)progview;
	(void)event;
	return false;
}
