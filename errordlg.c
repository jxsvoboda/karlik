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
 * Error dialog
 */

#include <errno.h>
#include <SDL.h>
#include <stdbool.h>
#include "gfx.h"
#include "map.h"
#include "errordlg.h"

enum {
	error_frame_width = 1
};

/** Create error dialog.
 *
 * @param bmp Bitmap
 * @param rerrordlg Place to store pointer to new error dialog
 *
 * @return Zero on success or error code
 */
int errordlg_create(gfx_bmp_t *bmp, errordlg_t **rerrordlg)
{
	errordlg_t *errordlg;

	errordlg = calloc(1, sizeof(errordlg_t));
	if (errordlg == NULL)
		return ENOMEM;

	errordlg->bmp = bmp;
	*rerrordlg = errordlg;
	return 0;
}

/** Destroy error dialog.
 *
 * @param errordlg Error dialog
 */
void errordlg_destroy(errordlg_t *errordlg)
{
	free(errordlg);
}

/** Set error dialog dimensions.
 *
 * @param x X origin
 * @param y Y origin
 * @param w Width
 * @param h Height
 */
void errordlg_set_dims(errordlg_t *errordlg, int x, int y, int w, int h)
{
	errordlg->orig_x = x;
	errordlg->orig_y = y;
	errordlg->width = w;
	errordlg->height = h;
}

/** Set error dialog callback.
 *
 * @param errordlg Error dialog
 * @param cb Callback
 * @param arg Callback argument
 */
void errordlg_set_cb(errordlg_t *errordlg, errordlg_cb_t cb, void *arg)
{
	errordlg->cb = cb;
	errordlg->cb_arg = arg;
}

/** Draw error dialog.
 *
 * @param errordlg Error dialog
 * @param gfx Graphics object to draw to
 */
void errordlg_draw(errordlg_t *errordlg, gfx_t *gfx)
{
	uint32_t color;
	int ix, iy;

	color = gfx_rgb(gfx, 0, 255, 255);
	gfx_rect(gfx, errordlg->orig_x, errordlg->orig_y,
	    errordlg->width, errordlg->height, color);

	color = gfx_rgb(gfx, 72, 72, 72);
	gfx_rect(gfx, errordlg->orig_x + 1, errordlg->orig_y + 1,
	    errordlg->width - 2, errordlg->height - 2, color);

	/* Center bitmap in the dialog window */
	ix = errordlg->orig_x + errordlg->width / 2 - errordlg->bmp->w / 2;
	iy = errordlg->orig_y + errordlg->height / 2 - errordlg->bmp->h / 2;

	gfx_bmp_render(gfx, errordlg->bmp, ix, iy);
}

/** Process input event in error dialog.
 *
 * @param errordlg Error dialog
 * @param event Event
 * @return @c true if event is claimed
 */
bool errordlg_event(errordlg_t *errordlg, SDL_Event *event)
{
	SDL_MouseButtonEvent *mbe;

	if (event->type == SDL_MOUSEBUTTONDOWN) {
		mbe = (SDL_MouseButtonEvent *)event;
		if (mbe->x >= errordlg->orig_x && mbe->y >= errordlg->orig_y &&
		    mbe->x < errordlg->orig_x + errordlg->width &&
		    mbe->y < errordlg->orig_y + errordlg->height) {
			if (errordlg->cb != NULL)
				errordlg->cb(errordlg->cb_arg);
			return true;
		}
	}

	return true;
}
