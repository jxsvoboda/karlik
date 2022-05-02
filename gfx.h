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

#ifndef GFX_H
#define GFX_H

#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
	SDL_Window *win;
} gfx_t;

typedef struct {
	SDL_Surface *surf;
	int w;
	int h;
} gfx_bmp_t;

/** Timer function */
typedef void (*gfx_timer_func_t)(void *);

typedef struct {
	SDL_TimerID id;
	gfx_timer_func_t func;
	uint32_t interval;
	void *arg;
} gfx_timer_t;

extern int gfx_init(gfx_t *, bool);
extern void gfx_quit(gfx_t *);
extern void gfx_clear(gfx_t *);
extern void gfx_rect(gfx_t *, int, int, int, int, uint32_t);
extern uint32_t gfx_rgb(gfx_t *, uint8_t, uint8_t, uint8_t);
extern int gfx_bmp_create(int, int, gfx_bmp_t **);
extern int gfx_bmp_load(const char *, gfx_bmp_t **);
extern void gfx_bmp_destroy(gfx_bmp_t *);
extern void gfx_bmp_set_color_key(gfx_bmp_t *, uint8_t, uint8_t, uint8_t);
extern void gfx_bmp_render(gfx_t *, gfx_bmp_t *, int, int);
extern void gfx_bmp_get_pixel(gfx_bmp_t *, int, int, uint8_t *, uint8_t *,
    uint8_t *);
extern void gfx_bmp_set_pixel(gfx_bmp_t *, int, int, uint8_t, uint8_t, uint8_t);
extern void gfx_set_wnd_icon(gfx_t *, gfx_bmp_t *);

extern void gfx_update(gfx_t *);
extern int gfx_wait_event(SDL_Event *);
extern int gfx_timer_create(uint32_t, gfx_timer_func_t, void *, gfx_timer_t **);
extern void gfx_timer_destroy(gfx_timer_t *);
extern void gfx_timer_start(gfx_timer_t *);
extern void gfx_timer_stop(gfx_timer_t *);
extern void gfx_handle_user_event(SDL_Event *);

#endif
