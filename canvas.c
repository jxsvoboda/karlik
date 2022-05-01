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
 * Canvas
 */

#include <errno.h>
#include <SDL.h>
#include <stdbool.h>
#include "canvas.h"
#include "gfx.h"

/** Create canvas.
 *
 * @param bmp Bitmap
 * @param rcanvas Place to store pointer to new canvas
 *
 * @return Zero on success or error code
 */
int canvas_create(gfx_bmp_t *bmp, canvas_t **rcanvas)
{
	canvas_t *canvas;

	canvas = calloc(1, sizeof(canvas_t));
	if (canvas == NULL)
		return ENOMEM;

	canvas->bmp = bmp;
	*rcanvas = canvas;
	return 0;
}

/** Destroy canvas.
 *
 * @param canvas Canvas
 */
void canvas_destroy(canvas_t *canvas)
{
	free(canvas);
}

/** Set canvas origin.
 *
 * @param canvas Canvas
 * @param x X origin
 * @param y Y origin
 */
void canvas_set_orig(canvas_t *canvas, int x, int y)
{
	canvas->orig_x = x;
	canvas->orig_y = y;
}

/** Set canvas magnification.
 *
 * @param canvas Canvas
 * @param mag Magnification
 */
void canvas_set_mag(canvas_t *canvas, int mag)
{
	canvas->mag = mag;
}

/** Set canvas callbacks.
 *
 * @param canvas Canvas
 * @param cb Callbacks
 * @param arg Callback argument
 */
void canvas_set_cb(canvas_t *canvas, canvas_cb_t *cb, void *arg)
{
	canvas->cb = cb;
	canvas->cb_arg = arg;
}

/** Set canvas drawing color.
 *
 * @param canvas Canvas
 * @param r Red
 * @param g Green
 * @param b Blue
 */
void canvas_set_drawing_color(canvas_t *canvas, uint8_t r, uint8_t g,
    uint8_t b)
{
	canvas->drawing_clr_r = r;
	canvas->drawing_clr_g = g;
	canvas->drawing_clr_b = b;
}

/** Draw canvas.
 *
 * @param canvas Canvas
 * @param gfx Graphics object to draw to
 */
void canvas_draw(canvas_t *canvas, gfx_t *gfx)
{
	uint8_t r, g, b;
	uint32_t color;
	int x, y;

	color = gfx_rgb(gfx, 108, 108, 108);
	gfx_rect(gfx, canvas->orig_x - 1, canvas->orig_y - 1,
	    canvas->bmp->w * canvas->mag + 2,
	    canvas->bmp->h * canvas->mag + 2, color);

	for (y = 0; y < canvas->bmp->h; y++) {
		for (x = 0; x < canvas->bmp->w; x++) {
			gfx_bmp_get_pixel(canvas->bmp, x, y, &r, &g, &b);
			color = gfx_rgb(gfx, r, g, b);
			gfx_rect(gfx, canvas->orig_x + x * canvas->mag,
			    canvas->orig_y + y * canvas->mag, canvas->mag,
			    canvas->mag, color);
		}
	}
}

/** Process input event in canvas.
 *
 * @param canvas Canvas
 * @param event Event
 * @return @c true if event is claimed
 */
bool canvas_event(canvas_t *canvas, SDL_Event *event)
{
	SDL_MouseButtonEvent *mbe;
	SDL_MouseMotionEvent *mme;
	int x, y;

	if (event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP) {
		canvas->button_held = (event->type == SDL_MOUSEBUTTONDOWN);

		mbe = (SDL_MouseButtonEvent *)event;
		x = (mbe->x - canvas->orig_x) / canvas->mag;
		y = (mbe->y - canvas->orig_y) / canvas->mag;
		if (x >= 0 && y >= 0 && x < canvas->bmp->w &&
		    y < canvas->bmp->h) {
			gfx_bmp_set_pixel(canvas->bmp, x, y,
			    canvas->drawing_clr_r,
			    canvas->drawing_clr_g,
			    canvas->drawing_clr_b);
			if (canvas->cb != NULL && canvas->cb->repaint != NULL)
				canvas->cb->repaint(canvas->cb_arg);
			return true;
		}

	}

	if (event->type == SDL_MOUSEMOTION && canvas->button_held) {
		mme = (SDL_MouseMotionEvent *)event;
		x = (mme->x - canvas->orig_x) / canvas->mag;
		y = (mme->y - canvas->orig_y) / canvas->mag;
		if (x >= 0 && y >= 0 && x < canvas->bmp->w &&
		    y < canvas->bmp->h) {
			gfx_bmp_set_pixel(canvas->bmp, x, y,
			    canvas->drawing_clr_r,
			    canvas->drawing_clr_g,
			    canvas->drawing_clr_b);
			if (canvas->cb != NULL && canvas->cb->repaint != NULL)
				canvas->cb->repaint(canvas->cb_arg);
			return true;
		}

	}

	return false;
}
