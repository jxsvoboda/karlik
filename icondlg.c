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

static void icondlg_palette_selected(void *, int);

/** Icon dialog palette callbacks */
static palette_cb_t icondlg_palette_cb = {
	.selected = icondlg_palette_selected
};

/** Palette colors */
static uint8_t pal_colors[3 * pal_num_entries] = {
	0, 0, 0, /* Black */
	0, 12, 84, /* Dark blue */
	0, 127, 14, /* Dark green */
	59, 165, 197, /* Dark cyan */

	156, 28, 20, /* Dark red */
	127, 0, 110, /* Dark magenta */
	156, 156, 108, /* Dark yellow */
	160, 160, 160, /* Dark white */

	72, 72, 72, /* Dark grey */
	0, 148, 255, /* Light blue */
	76, 255, 0, /* Light green */
	0, 255, 255, /* Light cyan */

	255, 0, 0, /* Light red */
	255, 0, 220, /* Light magenta */
	255, 216, 0, /* Light yellow */
	255, 255, 255 /* White */
};

/** Create icon dialog.
 *
 * @param icon Icon
 * @param ok_icon OK icon
 * @param ricondlg Place to store pointer to new icon dialog
 *
 * @return Zero on success or error code
 */
int icondlg_create(icon_t *icon, gfx_bmp_t *ok_icon, icondlg_t **ricondlg)
{
	icondlg_t *icondlg;
	int i;
	int rc;

	icondlg = calloc(1, sizeof(icondlg_t));
	if (icondlg == NULL)
		return ENOMEM;

	rc = canvas_create(icon->bmp, &icondlg->canvas);
	if (rc != 0) {
		free(icondlg);
		return rc;
	}

	rc = palette_create(&icondlg->palette);
	if (rc != 0) {
		canvas_destroy(icondlg->canvas);
		free(icondlg);
		return rc;
	}

	for (i = 0; i < pal_num_entries; i++) {
		palette_set_entry_color(icondlg->palette, i, pal_colors[3 * i],
		    pal_colors[3 * i + 1], pal_colors[3 * i + 2]);
	}

	palette_set_cb(icondlg->palette, &icondlg_palette_cb, icondlg);

	icondlg->icon = icon;
	icondlg->ok_icon = ok_icon;
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
	palette_destroy(icondlg->palette);
	canvas_destroy(icondlg->canvas);
	free(icondlg);
}

/** Load icon dialog from file.
 *
 * @param f File
 * @param ok_icon OK icon
 * @param ricondlg Place to store pointer to new icon dialog
 * @return Zero on success or an error code
 */
int icondlg_load(FILE *f, gfx_bmp_t *ok_icon, icondlg_t **ricondlg)
{
	icon_t *icon;
	int rc;

	rc = icon_load(f, &icon);
	if (rc != 0)
		return rc;

	rc = icondlg_create(icon, ok_icon, ricondlg);
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

	canvas_set_orig(icondlg->canvas, cx, cy - 20);
	canvas_set_mag(icondlg->canvas, icon_mag);

	/* Center palette in the dialog window */
	cx = icondlg->orig_x + icondlg->width / 2 -
	    pal_cols * 12 / 2;
	cy = icondlg->orig_y + icondlg->height / 2 -
	    pal_rows * 12 / 2;

	palette_set_orig(icondlg->palette, cx, cy + 50);
	palette_set_entry_dims(icondlg->palette, 12, 12);
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
	int x, y;

	color = gfx_rgb(gfx, 0, 255, 255);
	gfx_rect(gfx, icondlg->orig_x, icondlg->orig_y,
	    icondlg->width, icondlg->height, color);

	color = gfx_rgb(gfx, 72, 72, 72);
	gfx_rect(gfx, icondlg->orig_x + 1, icondlg->orig_y + 1,
	    icondlg->width - 2, icondlg->height - 2, color);

	canvas_draw(icondlg->canvas, gfx);
	palette_draw(icondlg->palette, gfx);

	x = icondlg->orig_x + icondlg->width - icondlg->ok_icon->w - 10;
	y = icondlg->orig_y + icondlg->height - icondlg->ok_icon->h - 10;

	color = gfx_rgb(gfx, 108, 108, 108);
	gfx_rect(gfx, x - 1, y - 1, icondlg->ok_icon->w + 2,
	    icondlg->ok_icon->h + 2, color);
	gfx_bmp_render(gfx, icondlg->ok_icon, x, y);
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
	int x, y;

	if (canvas_event(icondlg->canvas, event))
		return true;

	if (palette_event(icondlg->palette, event))
		return true;

	if (event->type == SDL_MOUSEBUTTONDOWN) {
		mbe = (SDL_MouseButtonEvent *)event;
		x = icondlg->orig_x + icondlg->width - icondlg->ok_icon->w - 10;
		y = icondlg->orig_y + icondlg->height - icondlg->ok_icon->h - 10;

		if (mbe->x >= x && mbe->y >= y &&
		    mbe->x < x + icondlg->ok_icon->w &&
		    mbe->y < y + icondlg->ok_icon->h) {
			if (icondlg->cb != NULL && icondlg->cb->accept != NULL)
				icondlg->cb->accept(icondlg->cb_arg);
			return true;
		}
	}

	/* This is a modal dialog. Block everything else. */
	return true;
}

/** Handle canvas repaint request in icon dialog.
 *
 * @param arg Argument (icondlg_t *)
 */
static void icondlg_repaint_req(icondlg_t *icondlg)
{
	if (icondlg->cb != NULL && icondlg->cb->repaint != NULL)
		icondlg->cb->repaint(icondlg->cb_arg);
}

/** Handle canvas repaint request in icon dialog.
 *
 * @param arg Argument (icondlg_t *)
 */
static void icondlg_canvas_repaint(void *arg)
{
	icondlg_t *icondlg = (icondlg_t *)arg;

	icondlg_repaint_req(icondlg);
}

/** Handle palette selected event in icon dialog.
 *
 * @param arg Argument (icondlg_t *)
 * @param idx Selected palette index
 */
static void icondlg_palette_selected(void *arg, int idx)
{
	icondlg_t *icondlg = (icondlg_t *)arg;

	canvas_set_drawing_color(icondlg->canvas, pal_colors[3 * idx],
	    pal_colors[3 * idx + 1], pal_colors[3 * idx + 2]);
	icondlg_repaint_req(icondlg);
}
