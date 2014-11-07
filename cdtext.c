/* cdtext.c
 * - cdtext data structure and functions
 * Copyright (C) 2003 Svend Sanjay Sorensen <ssorensen@fastmail.fm>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cdtext.h"

struct cdtext {
	const char *key;
	char *value;
};

CdText *cdtext_init ()
{
	CdText *new_cdtext = NULL;

	CdText cdtext[] = {
		{TITLE, NULL},
		{PERFORMER, NULL},
		{SONGWRITER, NULL},
		{COMPOSER, NULL},
		{ARRANGER, NULL},
		{MESSAGE, NULL},
		{DISC_ID, NULL},
		{GENRE, NULL},
		{TOC_INFO, NULL},
		{TOC_INFO2, NULL},
		{UPC_EAN, NULL},
		{ISRC, NULL},
		{SIZE_INFO, NULL},
		{NULL, NULL}
	};

	new_cdtext = (CdText *) calloc (sizeof (cdtext) / sizeof (CdText), sizeof (CdText));
	if (NULL == new_cdtext)
		fprintf (stderr, "problem allocating memory\n");
	else
		memcpy (new_cdtext, cdtext, sizeof(cdtext));

	return (new_cdtext);
}

void cdtext_delete (CdText *cdtext)
{
	int i;

	if (NULL != cdtext) {
		for (i = 0; NULL != cdtext[i].key; i++)
			free (cdtext[i].value);
		free (cdtext);
	}
}

/* returns 0 if field is a CD-TEXT keyword, returns non-zero otherwise */
int cdtext_is_keyword (char *key)
{
	int i;
	CdText *cdtext = cdtext_init ();

	for (i = 0; NULL != cdtext[i].key; i++)
		if (0 == strcmp (cdtext[i].key, key)) {
			free (cdtext);
			return (0);
		}

	free (cdtext);
	return (-1);
}

/* sets cdtext's keyword entry to field */
void cdtext_set (char *key, char *value, CdText *cdtext)
{
	if (NULL != value)	/* don't pass NULL to strdup! */
		for (; NULL != cdtext->key; cdtext++)
			if (0 == strcmp (cdtext->key, key)) {
				free (cdtext->value);
				cdtext->value = strdup (value);
			}
}

/* returns value for key, NULL if key is not found */
char *cdtext_get (char *key, CdText *cdtext)
{
	for (; NULL != cdtext->key; cdtext++)
		if (0 == strcmp (cdtext->key, key))
			return (cdtext->value);

	return (NULL);
}

/* prints non-empty CD-TEXT fields */
void cdtext_print (FILE *fp, CdText *cdtext)
{
	for (; NULL != cdtext->key; cdtext++)
		if (NULL != cdtext->value)
			fprintf(fp, "%s \"%s\"\n", cdtext->key, cdtext->value);
}
