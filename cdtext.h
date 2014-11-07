/* cdtext.h
 * - reference: MMC3 (Annex J)
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

/* cdtext keywords */
#define TITLE "TITLE"			/* title of album name or track titles */
#define PERFORMER "PERFORMER"		/* name(s) of the performer(s) */
#define SONGWRITER "SONGWRITER"		/* name(s) of the songwriter(s) */
#define COMPOSER "COMPOSER"		/* name(s) of the composer(s) */
#define ARRANGER "ARRANGER"		/* name(s) of the arranger(s) */
#define MESSAGE "MESSAGE"		/* message(s) from the content provider and/or artist */
#define DISC_ID "DISC_ID"		/* disc identification information */
#define GENRE "GENRE"			/* genre identification and genre information */
#define TOC_INFO "TOC_INFO"		/* table of contents information */
#define TOC_INFO2 "TOC_INFO2"		/* second table of contents information */
#define UPC_EAN "UPC_EAN"		/* UPC/EAN code of the album */
#define ISRC "ISRC"			/* ISRC code of each track */
#define SIZE_INFO "SIZE_INFO"		/* size information of the block */

typedef struct cdtext CdText;

CdText *cdtext_init ();
void cdtext_delete ();
int cdtext_is_keyword (char *key);
void cdtext_set (char *key, char *value, CdText *cdtext);
char *cdtext_get (char *key, CdText *cdtext);
void cdtext_print (FILE *fp, CdText *cdtext);
