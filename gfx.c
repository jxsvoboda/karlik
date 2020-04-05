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
 * Graphics
 */

#include <errno.h>
#include <SDL.h>
#include <stdbool.h>
#include "gfx.h"

/** Initialize graphics.
 *
 * @param gfx Graphics object to initialize
 * @param fullscreen @c true to start in fullscreen mode
 */
int gfx_init(gfx_t *gfx, bool fullscreen)
{
	int rc;
	SDL_Surface *surf;
	SDL_Rect rect;

	rc = SDL_Init(SDL_INIT_VIDEO);
	if (rc != 0)
		return -1;

	atexit(SDL_Quit);

	gfx->win = SDL_CreateWindow("Karlik", SDL_WINDOWPOS_CENTERED,
	    SDL_WINDOWPOS_CENTERED, 640, 480, fullscreen ?
	    SDL_WINDOW_FULLSCREEN : 0);

	if (gfx->win == NULL)
		return -1;

	surf = SDL_GetWindowSurface(gfx->win);
	rect.x = rect.y = 0;
	rect.w = 640;
	rect.h = 480;
	SDL_FillRect(surf, &rect, SDL_MapRGB(surf->format, 0, 0, 0));

	SDL_UpdateWindowSurface(gfx->win);

	return 0;
}

/** Deinitialize graphics.
 *
 * @param gfx Graphics
 */
void gfx_quit(gfx_t *gfx)
{
	SDL_DestroyWindow(gfx->win);
	gfx->win = NULL;

	SDL_Quit();
}

/** Draw a rectangle.
 *
 * @param gfx Graphics object
 * @param x X coordinate of top-left corner
 * @param y Y coordinate of top-left corner
 * @param w Width
 * @param h Height
 * @param color Color (from gfx_rgb)
 */
void gfx_rect(gfx_t *gfx, int x, int y, int w, int h, uint32_t color)
{
	SDL_Surface *surf;
	SDL_Rect rect;

	surf = SDL_GetWindowSurface(gfx->win);
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	SDL_FillRect(surf, &rect, color);
}

/** Map R, G, B coordinates to color.
 *
 * @param gfx Graphics object
 * @param r Red
 * @param g Green
 * @param b Blue
 * @return Color
 */
uint32_t gfx_rgb(gfx_t *gfx, uint8_t r, uint8_t g, uint8_t b)
{
	SDL_Surface *surf;

	surf = SDL_GetWindowSurface(gfx->win);
	return SDL_MapRGB(surf->format, r, g, b);
}

/** Clear graphics.
 *
 * @param gfx Graphics oject
 */
void gfx_clear(gfx_t *gfx)
{
	SDL_Surface *surf;

	surf = SDL_GetWindowSurface(gfx->win);
	SDL_FillRect(surf, NULL, SDL_MapRGB(surf->format, 0, 0, 0));
}

/** Update graphics output.
 *
 * @param gfx Graphics object
 */
void gfx_update(gfx_t *gfx)
{
	SDL_UpdateWindowSurface(gfx->win);
}

/** Load bitmap from BMP file.
 *
 * @param fname File name
 * @param rbmp Place to store pointer to new bitmap
 * @return Zero on success or an error code
 */
int gfx_bmp_load(const char *fname, gfx_bmp_t **rbmp)
{
	gfx_bmp_t *bmp;

	bmp = calloc(1, sizeof(gfx_bmp_t));
	if (bmp == NULL)
		return ENOMEM;

	bmp->surf = SDL_LoadBMP(fname);
	if (bmp->surf == NULL) {
		free(bmp);
		return EIO;
	}

	bmp->w = bmp->surf->w;
	bmp->h = bmp->surf->h;
	*rbmp = bmp;
	return 0;
}

/** Destroy bitmap.
 *
 * @param bmp Bitmap
 */
void gfx_bmp_destroy(gfx_bmp_t *bmp)
{
	SDL_FreeSurface(bmp->surf);
	free(bmp);
}

/** Set color key on bitmap.
 *
 * @param bmp Bitmap
 * @param r Red
 * @param g Green
 * @param b Blue
 */
void gfx_bmp_set_color_key(gfx_bmp_t *bmp, int r, int g, int b)
{
	Uint32 key;

	key = SDL_MapRGB(bmp->surf->format, r, g, b);
	SDL_SetColorKey(bmp->surf, SDL_TRUE, key);
}

/** Render bitmap.
 *
 * @param gfx Graphics
 * @param bmp Bitmap
 * @param x X coordinate on screen
 * @param y Y coordinate on screen
 */
void gfx_bmp_render(gfx_t *gfx, gfx_bmp_t *bmp, int x, int y)
{
	SDL_Surface *surf;
	SDL_Rect drect;

	surf = SDL_GetWindowSurface(gfx->win);

	drect.x = x;
	drect.y = y;
	drect.w = bmp->surf->w * 2;
	drect.h = bmp->surf->h * 2;

	SDL_BlitScaled(bmp->surf, NULL, surf, &drect);
}

/** Set window icon.
 *
 * @param gfx Graphics
 * @param icon Icon bitmap
 */
void gfx_set_wnd_icon(gfx_t *gfx, gfx_bmp_t *icon)
{
	SDL_SetWindowIcon(gfx->win, icon->surf);
}
