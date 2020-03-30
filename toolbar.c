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
	toolbar->icon = calloc(nentries, sizeof(SDL_Surface *));
	if (toolbar->icon == NULL) {
		free(toolbar);
		return ENOMEM;
	}

	for (i = 0; i < nentries; i++) {
		toolbar->icon[i] = SDL_LoadBMP(fname[i]);
		if (toolbar->icon[i] == NULL)
			goto error;
	}

	*rtoolbar = toolbar;
	return 0;
error:
	for (i = 0; i < nentries; i++) {
		if (toolbar->icon[i] != NULL)
			SDL_FreeSurface(toolbar->icon[i]);
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
			SDL_FreeSurface(toolbar->icon[i]);
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
	SDL_Surface *surf;
	SDL_Rect drect;
	SDL_Rect frect;
	Uint32 color;
	int i;

	surf = SDL_GetWindowSurface(gfx->win);

	drect.x = toolbar->orig_x;
	drect.y = toolbar->orig_y;

	for (i = 0; i < toolbar->nentries; i++) {
		drect.x += toolbar_hmargin;
		drect.w = 2 * toolbar->icon[i]->w;
		drect.h = 2 * toolbar->icon[i]->h;

		frect = drect;
		frect.x -= toolbar_frame_width;
		frect.y -= toolbar_frame_width;
		frect.w += 2 * toolbar_frame_width;
		frect.h += 2 * toolbar_frame_width;

		color = SDL_MapRGB(surf->format, 255, 0, 0);
		if (i == toolbar->sel)
			SDL_FillRect(surf, &frect, color);

		SDL_BlitScaled(toolbar->icon[i], NULL, surf, &drect);
		drect.x += drect.w + toolbar_hmargin;
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
	SDL_Rect drect;
	SDL_MouseButtonEvent *mbe;
	int i;

	drect.x = toolbar->orig_x + toolbar_hmargin;
	drect.y = toolbar->orig_y;

	for (i = 0; i < toolbar->nentries; i++) {
		drect.x += toolbar_hmargin;
		drect.w = 2 * toolbar->icon[i]->w;
		drect.h = 2 * toolbar->icon[i]->h;

		if (event->type == SDL_MOUSEBUTTONDOWN) {
			mbe = (SDL_MouseButtonEvent *)event;
			if (mbe->x >= drect.x && mbe->y >= drect.y &&
			    mbe->x < drect.x + drect.w &&
			    mbe->y < drect.y + drect.h) {
				toolbar->sel = i;
				if (toolbar->cb != NULL)
					toolbar->cb(toolbar->arg, i);
				return true;
			}
		}

		drect.x += drect.w + toolbar_hmargin;
	}

	return false;
}
