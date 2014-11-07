/* msf.c
 * - CD-DA msf functions
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
#include "msf.h"

long  msf_frame_from_mmssff (char *s)
{
	int field;
	long ret;
	char c;

	if (0 == strcmp (s, "0"))
		return 0;

	c = *s++;
	if(c >= '0' && c <= '9')
		field = (c - '0');
	else
		return -1;
	while(':' != (c = *s++)) {
		if(c >= '0' && c <= '9')
			field = field * 10 + (c - '0');
		else
			return -1;
	}

	ret = msf_msf_to_frame (field, 0, 0);

	c = *s++;
	if(c >= '0' && c <= '9')
		 field = (c - '0');
	else
		return -1;
	if(':' != (c = *s++)) {
		if(c >= '0' && c <= '9') {
			field = field * 10 + (c - '0');
			c = *s++;
			if(c != ':')
				return -1;
		}
		else
				return -1;
	}

	if(field >= 60)
		return -1;

	ret += msf_msf_to_frame (0, field, 0);

	c = *s++;
	if(c >= '0' && c <= '9')
		field = (c - '0');
	else
		return -1;
	if('\0' != (c = *s++)) {
		if(c >= '0' && c <= '9') {
			field = field * 10 + (c - '0');
			c = *s++;
		}
		else
			return -1;
	}

	if('\0' != c)
		return -1;

	if(field >= 75)
		return -1;

	ret += field;

	return ret;
}

void msf_frame_to_msf (long frame, int *minutes, int *seconds, int *frames)
{
	*frames = frame % 75;		/* 0 <= frames <= 74 */
	frame /= 75;
	*seconds = frame % 60;		/* 0 <= seconds <= 59 */
	frame /= 60;
	*minutes = frame;		/* 0 <= minutes */
}

long msf_msf_to_frame (int minutes, int seconds, int frames)
{
	return ((minutes * 60 + seconds) * 75 + frames);
}

/* print frame in mm:ss:ff format */
void msf_print_frame (FILE *fp, long frame)
{
	int minutes, seconds, frames;

	msf_frame_to_msf (frame, &minutes, &seconds, &frames);
	fprintf (fp, "%02d:%02d:%02d", minutes, seconds, frames);
}
