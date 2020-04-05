/*
 * Copyright 2020 Jiri Svoboda
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
 * Toolbar
 */

#include <errno.h>
#include <SDL.h>
#include <stdbool.h>
#include "gfx.h"
#include "toolbar.h"

enum {
	toolbar_hmargin = 4,
	toolbar_frame_width = 2
};

/** Create icon menu.
 *
 * @param fname List of file names of BMP files, NULL-terminated
 * @param rtoolbar Place to store pointer to new toolbar
 *
 * @return Zero on success or error code
 */
int toolbar_create(const char **fname, toolbar_t **rtoolbar)
{
	toolbar_t *toolbar;
	int nentries;
	int i;
	const char **cp;
	int rc;

	toolbar = calloc(1, sizeof(toolbar_t));
	if (toolbar == NULL)
		return ENOMEM;

	/* Count number of entries */
	cp = fname;
	nentries = 0;
	while (*cp != NULL) {
		++nentries;
		++cp;
	}

	toolbar->nentries = nentries;
	toolbar->icon = calloc(nentries, sizeof(gfx_bmp_t *));
	if (toolbar->icon == NULL) {
		free(toolbar);
		return ENOMEM;
	}

	for (i = 0; i < nentries; i++) {
		rc = gfx_bmp_load(fname[i], &toolbar->icon[i]);
		if (rc != 0)
			goto error;
	}

	*rtoolbar = toolbar;
	return 0;
error:
	for (i = 0; i < nentries; i++) {
		if (toolbar->icon[i] != NULL)
			gfx_bmp_destroy(toolbar->icon[i]);
	}

	free(toolbar);
	return EIO;
}

/** Set toolbar origin.
 *
 * @param toolbar Icon menu
 * @param x X coordinate
 * @param y Y coordinate
 */
void toolbar_set_origin(toolbar_t *toolbar, int x, int y)
{
	toolbar->orig_x = x;
	toolbar->orig_y = y;
}

/** Set callback.
 *
 * @param toolbar Toolbar
 * @param cb Callback function
 * @param arg Callback argument
 */
void toolbar_set_cb(toolbar_t *toolbar, toolbar_cb_t cb, void *arg)
{
	toolbar->cb = cb;
	toolbar->arg = arg;
}

/** Destroy toolbar.
 *
 * @param toolbar Icon menu
 */
void toolbar_destroy(toolbar_t *toolbar)
{
	int i;

	for (i = 0; i < toolbar->nentries; i++) {
		if (toolbar->icon[i] != NULL)
			gfx_bmp_destroy(toolbar->icon[i]);
	}

	free(toolbar->icon);
	free(toolbar);
}

/** Select toolbar entry.
 *
 * @param toolbar Toolbar
 * @param sel Entry to select
 */
void toolbar_select(toolbar_t *toolbar, int sel)
{
	toolbar->sel = sel;
}

/** Draw toolbar.
 *
 * @param toolbar Icon menu
 * @param gfx Graphics object to draw to
 */
void toolbar_draw(toolbar_t *toolbar, gfx_t *gfx)
{
	int x, y;
	int w, h;
	int fx, fy;
	int fw, fh;
	uint32_t color;
	int i;

	x = toolbar->orig_x;
	y = toolbar->orig_y;

	for (i = 0; i < toolbar->nentries; i++) {
		x += toolbar_hmargin;
		w = 2 * toolbar->icon[i]->w;
		h = 2 * toolbar->icon[i]->h;

		fx = x - toolbar_frame_width;
		fy = y - toolbar_frame_width;
		fw = w + 2 * toolbar_frame_width;
		fh = h + 2 * toolbar_frame_width;

		color = gfx_rgb(gfx, 255, 0, 0);
		if (i == toolbar->sel)
			gfx_rect(gfx, fx, fy, fw, fh, color);

		gfx_bmp_render(gfx, toolbar->icon[i], x, y);
		x += w + toolbar_hmargin;
	}
}

/** Process input event in toolbar.
 *
 * @param toolbar Icon menu
 * @param event Event
 * @return @c true if event is claimed
 */
bool toolbar_event(toolbar_t *toolbar, SDL_Event *event)
{
	int x, y;
	int w, h;
	SDL_MouseButtonEvent *mbe;
	int i;

	x = toolbar->orig_x + toolbar_hmargin;
	y = toolbar->orig_y;

	for (i = 0; i < toolbar->nentries; i++) {
		x += toolbar_hmargin;
		w = 2 * toolbar->icon[i]->w;
		h = 2 * toolbar->icon[i]->h;

		if (event->type == SDL_MOUSEBUTTONDOWN) {
			mbe = (SDL_MouseButtonEvent *)event;
			if (mbe->x >= x && mbe->y >= y &&
			    mbe->x < x + w && mbe->y < y + h) {
				toolbar->sel = i;
				if (toolbar->cb != NULL)
					toolbar->cb(toolbar->arg, i);
				return true;
			}
		}

		x += w + toolbar_hmargin;
	}

	return false;
}
