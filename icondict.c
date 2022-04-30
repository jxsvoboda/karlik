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
 * Icon dictionary - maps identifers to icons
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "gfx.h"
#include "icon.h"
#include "icondict.h"
#include "prog.h"

/** Create icon dictionary.
 *
 * @param ricondict Place to store pointer to new icon dictionary
 *
 * @return Zero on success or error code
 */
int icondict_create(icondict_t **ricondict)
{
	icondict_t *icondict;

	icondict = calloc(1, sizeof(icondict_t));
	if (icondict == NULL)
		return ENOMEM;

	list_initialize(&icondict->entries);
	*ricondict = icondict;
	return 0;
}

/** Destroy icon dictionary.
 *
 * @param icondict Icon dictionary
 */
void icondict_destroy(icondict_t *icondict)
{
	icondict_entry_t *entry;

	entry = icondict_first(icondict);
	while (entry != NULL) {
		icondict_remove(entry);
		entry = icondict_first(icondict);
	}

	free(icondict);
}

/** Add new entry to icon dictionary.
 *
 * @param icondict Icon dictionary
 * @param ident Identifier (will be duplicated)
 * @param icon Icon (ownership transferred to icon dictionary)
 *
 * @return Zero on success, ENOMEM if out of memory
 */
int icondict_add(icondict_t *icondict, const char *ident, icon_t *icon)
{
	icondict_entry_t *entry;

	entry = calloc(1, sizeof(icondict_entry_t));
	if (entry == NULL)
		return ENOMEM;

	entry->icondict = icondict;
	list_append(&entry->lidict, &icondict->entries);
	entry->ident = strdup(ident);
	if (entry->ident == NULL) {
		free(entry);
		return ENOMEM;
	}

	entry->icon = icon;
	return 0;
}

/** Remove icon dictionary entry.
 *
 * @param entry Entry
 */
void icondict_remove(icondict_entry_t *entry)
{
	list_remove(&entry->lidict);
	free(entry->ident);
	icon_destroy(entry->icon);
	free(entry);
}

/** Get first icon dictionary entry.
 *
 * @param icondict Icon dictionary
 * @return First entry or @c NULL if the list is empty
 */
icondict_entry_t *icondict_first(icondict_t *icondict)
{
	link_t *link;

	link = list_first(&icondict->entries);
	if (link == NULL)
		return NULL;

	return list_get_instance(link, icondict_entry_t, lidict);
}

/** Get next icon dictionary entry.
 *
 * @param cur Current entry
 * @return Next entry or @c NULL if @a cur is the last
 */
icondict_entry_t *icondict_next(icondict_entry_t *cur)
{
	link_t *link;

	link = list_next(&cur->lidict, &cur->icondict->entries);
	if (link == NULL)
		return NULL;

	return list_get_instance(link, icondict_entry_t, lidict);
}

/** Find icon dictionary entry by identifier.
 *
 * @param icondict Icon dictionary
 * @param ident Identifier
 * @return Icon or @c NULL if not found
 */
icondict_entry_t *icondict_find(icondict_t *icondict, const char *ident)
{
	icondict_entry_t *entry;

	entry = icondict_first(icondict);
	while (entry != NULL) {
		if (strcmp(entry->ident, ident) == 0)
			return entry;

		entry = icondict_next(entry);
	}

	return NULL;
}

/** Load icon dictionary entry from file.
 *
 * @param f File
 * @param icondict Icon dictionary to append the entry to
 * @return Zero on success or an error code
 */
static int icondict_entry_load(FILE *f, icondict_t *icondict)
{
	int rc;
	char ident[prog_proc_id_len + 1];
	icon_t *icon;

	rc = prog_proc_load_ident(f, ident);
	if (rc != 0)
		return rc;

	rc = icon_load(f, &icon);
	if (rc != 0)
		return rc;

	rc = icondict_add(icondict, ident, icon);
	if (rc != 0)
		goto error;

	return 0;
error:
	icon_destroy(icon);
	return rc;
}

/** Load icon dictionary from file.
 *
 * This includes the actual icons.
 */
int icondict_load(FILE *f, icondict_t **ricondict)
{
	icondict_t *icondict = NULL;
	int nitem;
	unsigned nentries;
	unsigned i;
	int rc;

	rc = icondict_create(&icondict);
	if (rc != 0)
		goto error;

	nitem = fscanf(f, "%u\n", &nentries);
	if (nitem != 1)
		return EIO;

	printf("nentries:%u\n", nentries);
	for (i = 0; i < nentries; i++) {
		rc = icondict_entry_load(f, icondict);
		if (rc != 0)
			goto error;
	}

	*ricondict = icondict;
	return 0;
error:
	if (icondict != NULL)
		icondict_destroy(icondict);
	return rc;
}

/** Save icon dictionary entry to file.
 *
 * @param entry Icon dictionary entry
 * @param f File
 * @return Zero on success or an error code
 */
static int icondict_entry_save(icondict_entry_t *entry, FILE *f)
{
	int rc;

	rc = prog_proc_save_ident(entry->ident, f);
	if (rc != 0)
		return rc;

	rc = icon_save(entry->icon, f);
	if (rc != 0)
		return rc;

	return 0;
}

/** Save icon dictionary to file.
 *
 * This includes the actual icons.
 *
 * @param icondict Icon dictionary
 * @param f File
 * @return Zero on success or an error code
 */
int icondict_save(icondict_t *icondict, FILE *f)
{
	icondict_entry_t *entry;
	int rv;
	int rc;

	rv = fprintf(f, "%u\n", (unsigned)list_count(&icondict->entries));
	if (rv < 0)
		return EIO;

	entry = icondict_first(icondict);
	while (entry != NULL) {
		rc = icondict_entry_save(entry, f);
		if (rc != 0)
			return rc;

		entry = icondict_next(entry);
	}

	return 0;
}
