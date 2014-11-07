/* cd.c
 * - cd structure
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
#include <string.h>
#include "cd.h"

/* returns length of track i or 0 if length is indeterminate (last track) */
long cd_track_length (Cd *cd, int i)
{
	long length = 0;

	if (i < cd->ntrack - 1)
		length = cd->tracks[i + 1].indexes[0] - cd->tracks[i].indexes[0];

	return (length);
}

/* returns the first filename found starting at track i and working backwards
 * if no filename is found, returns filename of track 0 (NULL)
 */
char *cd_track_filename (Cd *cd, int i)
{
	for (; 0 <= i && NULL == cd->tracks[i].filename; i--)
		;

	return (cd->tracks[i].filename);
}
