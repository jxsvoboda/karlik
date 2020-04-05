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
 * Wordlist - a list of icons representing words for selection
 */

#include <errno.h>
#include <SDL.h>
#include <stdbool.h>
#include "gfx.h"
#include "wordlist.h"

enum {
	wordlist_hmargin = 2,
	wordlist_frame_width = 1
};

static wordlist_entry_t *wordlist_first(wordlist_t *);
static wordlist_entry_t *wordlist_next(wordlist_entry_t *);

/** Create wordlist.
 *
 * @param rwordlist Place to store pointer to new wordlist
 *
 * @return Zero on success or error code
 */
int wordlist_create(wordlist_t **rwordlist)
{
	wordlist_t *wordlist;

	wordlist = calloc(1, sizeof(wordlist_t));
	if (wordlist == NULL)
		return ENOMEM;

	list_initialize(&wordlist->entries);
	*rwordlist = wordlist;
	return 0;
}

/** Set wordlist origin.
 *
 * @param wordlist Wordlist
 * @param x X coordinate
 * @param y Y coordinate
 */
void wordlist_set_origin(wordlist_t *wordlist, int x, int y)
{
	wordlist->orig_x = x;
	wordlist->orig_y = y;
}

/** Set callback.
 *
 * @param wordlist wordlist
 * @param cb Callback function
 * @param arg Callback argument
 */
void wordlist_set_cb(wordlist_t *wordlist, wordlist_cb_t cb, void *arg)
{
	wordlist->cb = cb;
	wordlist->arg = arg;
}

/** Destroy wordlist.
 *
 * @param wordlist Wordlist
 */
void wordlist_destroy(wordlist_t *wordlist)
{
	wordlist_entry_t *entry;

	entry = wordlist_first(wordlist);
	while (entry != NULL) {
		list_remove(&entry->lwlist);
		free(entry);

		entry = wordlist_first(wordlist);
	}

	free(wordlist);
}

/** Add new entry to wordlist.
 *
 * @param wordlist Wordlist
 * @param icon Icon
 * @param arg User argument
 *
 * @return Zero on success, ENOMEM if out of memory
 */
int wordlist_add(wordlist_t *wordlist, gfx_bmp_t *icon, void *arg)
{
	wordlist_entry_t *entry;

	entry = calloc(1, sizeof(wordlist_entry_t));
	if (entry == NULL)
		return ENOMEM;

	entry->wlist = wordlist;
	list_append(&entry->lwlist, &wordlist->entries);
	entry->icon = icon;
	entry->arg = arg;

	return 0;
}

/** Draw wordlist.
 *
 * @param wordlist Wordlist
 * @param gfx Graphics object to draw to
 */
void wordlist_draw(wordlist_t *wordlist, gfx_t *gfx)
{
	wordlist_entry_t *entry;
	int x, y;

	x = wordlist->orig_x;
	y = wordlist->orig_y;

	entry = wordlist_first(wordlist);
	while (entry != NULL) {
		x += wordlist_hmargin;

		gfx_bmp_render(gfx, entry->icon, x, y);
		x += entry->icon->w + wordlist_hmargin;

		entry = wordlist_next(entry);
	}
}

/** Process input event in wordlist.
 *
 * @param wordlist Wordlist
 * @param event Event
 * @return @c true if event is claimed
 */
bool wordlist_event(wordlist_t *wordlist, SDL_Event *event)
{
	SDL_MouseButtonEvent *mbe;
	wordlist_entry_t *entry;
	int x, y;
	int w, h;

	x = wordlist->orig_x + wordlist_hmargin;
	y = wordlist->orig_y;

	entry = wordlist_first(wordlist);
	while (entry != NULL) {
		x += wordlist_hmargin;
		w = entry->icon->w;
		h = entry->icon->h;

		if (event->type == SDL_MOUSEBUTTONDOWN) {
			mbe = (SDL_MouseButtonEvent *)event;
			if (mbe->x >= x && mbe->y >= y &&
			    mbe->x < x + w && mbe->y < y + h) {
				printf("Select entry %p\n", entry);
				if (wordlist->cb != NULL)
					wordlist->cb(wordlist->arg, entry->arg);
				return true;
			}
		}

		x += w + wordlist_hmargin;
		entry = wordlist_next(entry);
	}

	return false;
}

/** Get first wordlist entry.
 *
 * @param wordlist Wordlist
 * @return First entry or @c NULL if the list is empty
 */
static wordlist_entry_t *wordlist_first(wordlist_t *wordlist)
{
	link_t *link;

	link = list_first(&wordlist->entries);
	if (link == NULL)
		return NULL;

	return list_get_instance(link, wordlist_entry_t, lwlist);
}

/** Get next wordlist entry.
 *
 * @param cur Current entry
 * @return Next entry or @c NULL if @a cur is the last
 */
static wordlist_entry_t *wordlist_next(wordlist_entry_t *cur)
{
	link_t *link;

	link = list_next(&cur->lwlist, &cur->wlist->entries);
	if (link == NULL)
		return NULL;

	return list_get_instance(link, wordlist_entry_t, lwlist);
}
