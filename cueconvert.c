/* cueconvert.c
 * - convert cue/toc to cue/toc
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

/* options */
int verbose = 1;

char *progname;

void usage (int status)
{
	if (0 == status) {
		fprintf (stdout, "%s: usage: cueconvert [-v] [-t cue|toc] -f <infile> [-t cue|toc] -f <outfile>\n", progname);
	} else {
		fprintf (stderr, "%s: syntax error\n", progname);
		fprintf (stderr, "run `%s' for usage\n", progname);
	}

	exit (status);
}

int main (int argc, char **argv)
{
	Cd cd;
	CueFile infile = {NULL, NULL, UNKNOWN};
	CueFile outfile = {NULL, NULL, UNKNOWN};
	/* todo: find better way to do 2 files */
	/* option variables */
	char c;
	/* getopt () variables */
	extern char *optarg;
	extern int optind;

	progname = *argv;

	while (-1 != (c = getopt (argc, argv, "f:t:vo:"))) {
		switch (c) {
		case 'f':
			if (NULL == infile.file)
				infile.file = optarg;
			else
				outfile.file = optarg;
			break;
		case 't':
			if (0 == strcmp ("cue", optarg)) {
				if (NULL == infile.file)
					infile.format = CUE;
				else
					outfile.format = CUE;
			} else if (0 == strcmp ("toc", optarg)) {
				if (NULL == infile.file)
					infile.format = TOC;
				else
					outfile.format = TOC;
			}
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
	} else if (optind >= argc && NULL != infile.file && NULL != outfile.file) {
		cue_read_file (&cd, &infile);
		cue_write_file (&cd, &outfile);
	} else {
		usage (1);
	}

	return (0);
}
