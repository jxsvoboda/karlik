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
 * Icon - user drawn image representing a procedure
 */

#include <errno.h>
#include <stdint.h>
#include <string.h>
#include "gfx.h"
#include "icon.h"

/** Create icon.
 *
 * @param w Width
 * @param h Height
 * @param ricon Place to store pointer to new icon
 *
 * @return Zero on success or error code
 */
int icon_create(int w, int h, icon_t **ricon)
{
	icon_t *icon;
	int rc;

	icon = calloc(1, sizeof(icon_t));
	if (icon == NULL)
		return ENOMEM;

	rc = gfx_bmp_create(w, h, &icon->bmp);
	if (rc != 0) {
		free(icon);
		return rc;
	}

	*ricon = icon;
	return 0;
}

/** Destroy icon.
 *
 * @param icon Icon
 */
void icon_destroy(icon_t *icon)
{
	gfx_bmp_destroy(icon->bmp);
	free(icon);
}

/** Load icon from file.
 *
 * @param f File
 * @param ricon Place to store pointer to new icon
 * @return Zero on success or an error code
 */
int icon_load(FILE *f, icon_t **ricon)
{
	int rc;
	int x, y;
	int w, h;
	int nitem;
	icon_t *icon = NULL;
	int c;
	unsigned r, g, b;

	nitem = fscanf(f, "%d %d\n", &w, &h);
	if (nitem != 2)
		return EIO;

	rc = icon_create(w, h, &icon);
	if (rc != 0)
		goto error;

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			if (x > 0) {
				c = fgetc(f);
				if (c != ' ') {
					rc = EIO;
					goto error;
				}
			}

			nitem = fscanf(f, "%u,%u,%u", &r, &g, &b);
			if (nitem != 3) {
				rc = EIO;
				goto error;
			}

			gfx_bmp_set_pixel(icon->bmp, x, y, r, g, b);
		}

		c = fgetc(f);
		if (c != '\n') {
			rc = EIO;
			goto error;
		}
	}

	*ricon = icon;
	return 0;
error:
	if (icon != NULL)
		icon_destroy(icon);
	return rc;
}

/** Save icon to file.
 *
 * @param icon Icon
 * @param f File
 * @return Zero on success or an error code
 */
int icon_save(icon_t *icon, FILE *f)
{
	int rv;
	int x, y;
	uint8_t r, g, b;

	rv = fprintf(f, "%d %d\n", icon->bmp->w, icon->bmp->h);
	if (rv < 0)
		return EIO;

	for (y = 0; y < icon->bmp->h; y++) {
		for (x = 0; x < icon->bmp->w; x++) {
			gfx_bmp_get_pixel(icon->bmp, x, y, &r, &g, &b);
			rv = fprintf(f, "%s%u,%u,%u", x > 0 ? " " : "",
			    r, g, b);
			if (rv < 0)
				return EIO;
		}

		rv = fputc('\n', f);
		if (rv < 0)
			return EIO;
	}

	return 0;
}
