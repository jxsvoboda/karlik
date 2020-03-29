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
 * Karlik main
 */

#include <stdbool.h>
#include <stdio.h>
#include <SDL.h>
#include "gfx.h"
#include "karlik.h"

static void print_syntax(void)
{
	printf("Syntax: karlik [-f]\n");
}

int main(int argc, char *argv[])
{
	gfx_t gfx;
	SDL_Event e;
	karlik_t *karlik = NULL;
	bool fs = false;
	int rc;

	if (argc >= 2) {
		if (strcmp(argv[1], "-f") == 0) {
			fs = true;
		} else {
			print_syntax();
			return 1;
		}

		if (argc > 2) {
			print_syntax();
			return 1;
		}
	}

	rc = gfx_init(&gfx, fs);
	if (rc != 0)
		return 1;

	SDL_Surface *appicon = SDL_LoadBMP("img/appicon.bmp");
	if (appicon == NULL)
		return 1;

	SDL_SetWindowIcon(gfx.win, appicon);

	rc = karlik_create(&gfx, &karlik);
	if (rc != 0)
		return 1;

	while (!karlik->quit && SDL_WaitEvent(&e)) {
		karlik_event(karlik, &e, &gfx);
	}

	rc = karlik_save(karlik);
	if (rc != 0) {
		printf("Error saving map!\n");
		return 1;
	}

	karlik_destroy(karlik);
	gfx_quit(&gfx);

	return 0;
}
