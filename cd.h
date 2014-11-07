/* cd.h
 * - cd structure
 * - reference: MMC-3 draft revsion - 10g
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

#include "cdtext.h"

#define MAXTRACKS 99			/* Red Book track limit */
#define MAXINDEXES 99			/* index limit */

/* track flags
 * Q Sub-channel Control Field (4.2.3.3)
 */
typedef enum {
	NONE = 			0x00,	/* no flags set */
	PRE_EMPHASIS =		0x01,	/* audio track recorded with pre-emphasis */
	COPY_PERMITTED =	0x02,	/* digital copy permitted */
	DATA =			0x04,	/* data track */
	FOUR_CHANNEL_AUDIO =	0x08,	/* 4 audio channels */
	SCMS =			0x10	/* SCMS (5.29.2.7) */
} Flag;

/* track modes (Table 350) */
typedef enum {
	AUDIO,				/* 2352 byte block length */
	MODE1,				/* 2048 byte block length */
	MODE1_RAW,			/* 2352 byte block length */
	MODE2,				/* 2336 byte block length */
	MODE2_FORM1,			/* 2048 byte block length */
	MODE2_FORM2,			/* 2324 byte block length */
	MODE2_FORM_MIX,			/* 2336 byte block length */
	MODE2_RAW			/* 2352 byte block length */
} TrackMode;

/* disc modes (5.29.2.8) */
typedef enum {
	CD_DA,				/* CD-DA */
	CD_ROM,				/* CD-ROM mode 1 */
	CD_ROM_XA			/* CD-ROM XA and CD-I */
} DiscMode;

/* track structure */
typedef struct track {
	char *filename;			/* track filename */
	TrackMode mode;			/* (toc) track mode */
	Flag flags;			/* flags */
	long pregap;			/* pre-gap with zero audio data */
	long length;			/* (toc) length of track (in frames) */
	char *isrc;			/* IRSC Code (5.22.4) exactly 12 bytes */
	CdText *cdtext;			/* CD-TEXT */
	int nindex;			/* number of indexes */
	long indexes[MAXINDEXES];	/* indexes (in frames) (5.29.2.5) */
} Track;

/* cd structure */
typedef struct cd {
	DiscMode mode;			/* (toc) disc mode */
	char *catalog;			/* Media Catalog Number (5.22.3) exactly 13 bytes */
	CdText *cdtext;			/* CD-TEXT */
	int ntrack;			/* number of tracks in album */
	Track tracks[MAXTRACKS];	/* array of tracks */
} Cd;

long cd_track_length (Cd *cd, int track);
char *cd_track_filename (Cd *cd, int track);
