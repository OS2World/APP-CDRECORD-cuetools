/* cuebreakpoints.c
 * - ouputs track breakpoints
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "cue.h"
#include "msf.h"

char *progname;

/* options */
int verbose = 0;

void usage (int status)
{
	if (0 == status) {
		fprintf (stdout, "usage: %s [-v] [-t cue|toc] -f <file>\n", progname);
	} else {
		fprintf (stderr, "%s: syntax error\n", progname);
		fprintf (stderr, "run `%s' for usage\n", progname);
	}

	exit (status);
}

/* prints track breaks in m:ss.ff format */
void print_m_ss_ff (long frame)
{
	int minutes, seconds, frames;

	msf_frame_to_msf (frame, &minutes, &seconds, &frames);
	fprintf (stdout, "%d:%02d.%02d\n", minutes, seconds, frames);
}

void print_breaks (Cd *cd)
{
	int i;

	for (i = 0; i < cd->ntrack; i++) {
		/* do not print zero indexes */
		if (0 != cd->tracks[i].indexes[0])
			print_m_ss_ff (cd->tracks[i].indexes[0]);
	}
}

int main (int argc, char **argv)
{
	Cd cd;
	CueFile file = {NULL, NULL, UNKNOWN};
	/* option variables */
	char c;
	/* getopt () variables */
	extern char *optarg;
	extern int optind;

	progname = *argv;

	while (-1 != (c = getopt (argc, argv, "f:t:v"))) {
		switch (c) {
		case 'f':
			file.file = optarg;
			break;
		case 't':
			if (0 == strcmp ("cue", optarg))
				file.format = CUE;
			else if (0 == strcmp ("toc", optarg))
				file.format = TOC;
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			usage (1);
		}
	}

	if (1 == argc) {
		usage (0);
	} else if (optind >= argc && NULL != file.file) {
		cue_read_file (&cd, &file);
		print_breaks (&cd);
	} else {
		usage (1);
	}

	return (0);
}
