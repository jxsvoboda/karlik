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
 * Icon dialog
 */

#include <errno.h>
#include <SDL.h>
#include <stdbool.h>
#include "gfx.h"
#include "map.h"
#include "icon.h"
#include "icondlg.h"

enum {
	icon_mag = 4
};

static void icondlg_canvas_repaint(void *);

/** Icon dialog canvas callbacks */
static canvas_cb_t icondlg_canvas_cb = {
	.repaint = icondlg_canvas_repaint
};

/** Create icon dialog.
 *
 * @param icon Icon
 * @param ricondlg Place to store pointer to new icon dialog
 *
 * @return Zero on success or error code
 */
int icondlg_create(icon_t *icon, icondlg_t **ricondlg)
{
	icondlg_t *icondlg;
	int rc;

	icondlg = calloc(1, sizeof(icondlg_t));
	if (icondlg == NULL)
		return ENOMEM;

	rc = canvas_create(icon->bmp, &icondlg->canvas);
	if (rc != 0) {
		free(icondlg);
		return rc;
	}

	icondlg->icon = icon;
	canvas_set_cb(icondlg->canvas, &icondlg_canvas_cb, icondlg);

	*ricondlg = icondlg;
	return 0;
}

/** Destroy icon dialog.
 *
 * @param icondlg Error dialog
 */
void icondlg_destroy(icondlg_t *icondlg)
{
	canvas_destroy(icondlg->canvas);
	free(icondlg);
}

/** Load icon dialog from file.
 *
 * @param f File
 * @param ricondlg Place to store pointer to new icon dialog
 * @return Zero on success or an error code
 */
int icondlg_load(FILE *f, icondlg_t **ricondlg)
{
	icon_t *icon;
	int rc;

	rc = icon_load(f, &icon);
	if (rc != 0)
		return rc;

	rc = icondlg_create(icon, ricondlg);
	if (rc != 0) {
		icon_destroy(icon);
		return rc;
	}

	return 0;
}

/** Save icon dialog to file.
 *
 * @param icondlg Icon dialog
 * @param f File
 * @return Zero on success or an error code
 */
int icondlg_save(icondlg_t *icondlg, FILE *f)
{
	int rc;

	rc = icon_save(icondlg->icon, f);
	if (rc != 0)
		return rc;

	return 0;
}

/** Set icon dialog dimensions.
 *
 * @param x X origin
 * @param y Y origin
 * @param w Width
 * @param h Height
 */
void icondlg_set_dims(icondlg_t *icondlg, int x, int y, int w, int h)
{
	int cx, cy;

	icondlg->orig_x = x;
	icondlg->orig_y = y;
	icondlg->width = w;
	icondlg->height = h;

	/* Center canvas in the dialog window */
	cx = icondlg->orig_x + icondlg->width / 2 -
	    icondlg->icon->bmp->w * icon_mag / 2;
	cy = icondlg->orig_y + icondlg->height / 2 -
	    icondlg->icon->bmp->h * icon_mag / 2;

	canvas_set_orig(icondlg->canvas, cx, cy);
	canvas_set_mag(icondlg->canvas, icon_mag);
}

/** Set icon dialog callbacks.
 *
 * @param icondlg Error dialog
 * @param cb Callbacks
 * @param arg Callback argument
 */
void icondlg_set_cb(icondlg_t *icondlg, icondlg_cb_t *cb, void *arg)
{
	icondlg->cb = cb;
	icondlg->cb_arg = arg;
}

/** Draw icon dialog.
 *
 * @param icondlg Error dialog
 * @param gfx Graphics object to draw to
 */
void icondlg_draw(icondlg_t *icondlg, gfx_t *gfx)
{
	uint32_t color;

	color = gfx_rgb(gfx, 0, 255, 255);
	gfx_rect(gfx, icondlg->orig_x, icondlg->orig_y,
	    icondlg->width, icondlg->height, color);

	color = gfx_rgb(gfx, 72, 72, 72);
	gfx_rect(gfx, icondlg->orig_x + 1, icondlg->orig_y + 1,
	    icondlg->width - 2, icondlg->height - 2, color);

	canvas_draw(icondlg->canvas, gfx);
}

/** Process input event in icon dialog.
 *
 * @param icondlg Error dialog
 * @param event Event
 * @return @c true if event is claimed
 */
bool icondlg_event(icondlg_t *icondlg, SDL_Event *event)
{
	SDL_MouseButtonEvent *mbe;

	if (canvas_event(icondlg->canvas, event))
		return true;

	if (event->type == SDL_MOUSEBUTTONDOWN) {
		mbe = (SDL_MouseButtonEvent *)event;
		if (mbe->x >= icondlg->orig_x && mbe->y >= icondlg->orig_y &&
		    mbe->x < icondlg->orig_x + icondlg->width &&
		    mbe->y < icondlg->orig_y + icondlg->height) {
			if (icondlg->cb != NULL && icondlg->cb->accept != NULL)
				icondlg->cb->accept(icondlg->cb_arg);
			return true;
		}
	}

	return false;
}

/** Handle canvas repaint request in icon dialog.
 *
 * @param arg Argument (icondlg_t *)
 */
static void icondlg_canvas_repaint(void *arg)
{
	icondlg_t *icondlg = (icondlg_t *)arg;

	if (icondlg->cb != NULL && icondlg->cb->repaint != NULL)
		icondlg->cb->repaint(icondlg->cb_arg);
}
