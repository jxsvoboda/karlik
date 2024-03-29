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
 * Cardinal direction
 */

#include "dir.h"

/** Next direction counter-clockwise.
 *
 * @param cur Current direction
 * @return Next direction
 */
dir_t dir_next_ccw(dir_t cur)
{
	if (cur < dir_south)
		return cur + 1;
	else
		return dir_east;
}

/** Get X and Y offset for cardinal direction.
 *
 * @param dir Direction
 * @param x Place to store X offset
 * @param y Place to store Y offset
 */
void dir_get_off(dir_t dir, int *x, int *y)
{
	*x = 0;
	*y = 0;

	switch (dir) {
	case dir_east:
		*x = +1;
		*y = 0;
		break;
	case dir_north:
		*x = 0;
		*y = -1;
		break;
	case dir_west:
		*x = -1;
		*y = 0;
		break;
	case dir_south:
		*x = 0;
		*y = +1;
		break;
	}
}
