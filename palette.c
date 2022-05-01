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
 * Palette
 */

#include <errno.h>
#include <SDL.h>
#include <stdbool.h>
#include "palette.h"
#include "gfx.h"

/** Create palette.
 *
 * @param rpalette Place to store pointer to new palette
 *
 * @return Zero on success or error code
 */
int palette_create(palette_t **rpalette)
{
	palette_t *palette;

	palette = calloc(1, sizeof(palette_t));
	if (palette == NULL)
		return ENOMEM;

	*rpalette = palette;
	return 0;
}

/** Destroy palette.
 *
 * @param palette Palette
 */
void palette_destroy(palette_t *palette)
{
	free(palette);
}

/** Set palette origin.
 *
 * @param x X origin
 * @param y Y origin
 */
void palette_set_orig(palette_t *palette, int x, int y)
{
	palette->orig_x = x;
	palette->orig_y = y;
}

/** Set palette entry dimensions.
 *
 * @param w Entry width
 * @param h Entry height
 */
void palette_set_entry_dims(palette_t *palette, int w, int h)
{
	palette->entry_w = w;
	palette->entry_h = h;
}

/** Set palette entry color.
 *
 * @param idx Entry index
 * @param r Red component
 * @param g Green component
 * @param b Blue component
 */
void palette_set_entry_color(palette_t *palette, int idx, uint8_t r,
    uint8_t g, uint8_t b)
{
	palette->entry[idx].r = r;
	palette->entry[idx].g = g;
	palette->entry[idx].b = b;
}

/** Set palette callbacks.
 *
 * @param palette Palette
 * @param cb Callbacks
 * @param arg Callback argument
 */
void palette_set_cb(palette_t *palette, palette_cb_t *cb, void *arg)
{
	palette->cb = cb;
	palette->cb_arg = arg;
}

/** Draw palette.
 *
 * @param palette Palette
 * @param gfx Graphics object to draw to
 */
void palette_draw(palette_t *palette, gfx_t *gfx)
{
	uint32_t color;
	palette_entry_t *e;
	int x, y;

	for (y = 0; y < pal_rows; y++) {
		for (x = 0; x < pal_cols; x++) {
			if (y * pal_cols + x == palette->sel_idx)
				color = gfx_rgb(gfx, 255, 0, 0);
			else
				color = gfx_rgb(gfx, 108, 108, 108);

			gfx_rect(gfx, palette->orig_x + x * palette->entry_w,
			    palette->orig_y + y * palette->entry_h,
			    palette->entry_w, palette->entry_h, color);

			e = &palette->entry[y * pal_cols + x];
			color = gfx_rgb(gfx, e->r, e->g, e->b);
			gfx_rect(gfx, palette->orig_x + x * palette->entry_w + 1,
			    palette->orig_y + y * palette->entry_h + 1,
			    palette->entry_w - 2, palette->entry_h - 2, color);
		}
	}
}

/** Process input event in palette.
 *
 * @param palette Palette
 * @param event Event
 * @return @c true if event is claimed
 */
bool palette_event(palette_t *palette, SDL_Event *event)
{
	SDL_MouseButtonEvent *mbe;
	int x, y;

	if (event->type == SDL_MOUSEBUTTONDOWN) {
		mbe = (SDL_MouseButtonEvent *)event;
		x = (mbe->x - palette->orig_x) / palette->entry_w;
		y = (mbe->y - palette->orig_y) / palette->entry_h;
		printf("x=%d y=%d\n", x, y);
		if (x >= 0 && y >= 0 && x < pal_cols && y < pal_rows) {
			palette->sel_idx = y * pal_cols + x;
			if (palette->cb != NULL && palette->cb->selected != NULL)
				palette->cb->selected(palette->cb_arg,
				    palette->sel_idx);
			return true;
		}

	}

	return false;
}
