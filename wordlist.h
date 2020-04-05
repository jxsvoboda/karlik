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

#ifndef WORDLIST_H
#define WORDLIST_H

#include <stdbool.h>
#include <SDL.h>
#include "adt/list.h"
#include "gfx.h"

/** Wordlist callback.
 *
 * Called when a word is selected. The first argument is provided
 * by the caller upon wordlist creation. The second argument is the per-word
 * user argument.
 */
typedef void (*wordlist_cb_t)(void *, void *);

/** Wordlist entry */
typedef struct {
	/** Containing wordlist */
	struct wordlist *wlist;
	/** Link to @c wlist->entries */
	link_t lwlist;
	/** Icon */
	gfx_bmp_t *icon;
	/** User argument */
	void *arg;
} wordlist_entry_t;

/** Wordlist */
typedef struct wordlist {
	/** Entries (wordlist_entry_t) */
	list_t entries;
	/** Origin X coordinate */
	int orig_x;
	/** Origin Y coordinate */
	int orig_y;
	/** Callback */
	wordlist_cb_t cb;
	/** Callback argument */
	void *arg;
} wordlist_t;

extern int wordlist_create(wordlist_t **);
extern void wordlist_set_origin(wordlist_t *, int, int);
extern void wordlist_set_cb(wordlist_t *, wordlist_cb_t, void *);
extern void wordlist_destroy(wordlist_t *);
extern int wordlist_add(wordlist_t *, gfx_bmp_t *, void *);
extern void wordlist_draw(wordlist_t *, gfx_t *);
extern bool wordlist_event(wordlist_t *, SDL_Event *);

#endif
