/* cuerename.c
 * - renames files based on a template
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
#include <stdlib.h>		/* getsubopt () todo: still getting implicit decl warning */
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#ifdef __EMX__
	#include "os2/os2additional.h"
#endif
#include "cue.h"
#include "msf.h"

#define OPT_FILE ".cuerenamerc"	/* option file */
#define MAXFILENAME 257		/* maximum filename length + 1 */
#define MAXREPLACES 256		/* maximum number of string replaces */
#define TEMPLATE "%a-%t%F"	/* default renaming template */
#define CHARS "._-~"		/* characters allowed (besides alphanumeric) */

char *progname;

/* getsubopt () options */
enum {OPT_CASE = 0, OPT_UNDERSCORE, OPT_STRICT, OPT_NOSTRICT, OPT_REPLACE, OPT_TEMPLATE, OPT_TEST, OPT_NOTEST};
char *options[] = {
	[OPT_CASE] = "case",
	[OPT_UNDERSCORE] = "underscore",
	[OPT_STRICT] = "strict",
	[OPT_NOSTRICT] = "nostrict",
	[OPT_REPLACE] = "replace",
	[OPT_TEMPLATE] = "template",
	[OPT_TEST] = "test",
	[OPT_NOTEST] = "notest",
	NULL
};

/* options */
int verbose = 0;		/* verbose mode */
enum {
	NOCONV,
	LOWER,
	UPPER
} caseconv = NOCONV;		/* perform case conversion */
int underscore = 0;		/* replace spaces with underscores */
char *strict = NULL;		/* only allow alphanumeric chars or chars within strict */
int num_replace = 0;		/* number of string replacements */
char *replace[MAXREPLACES];	/* array of string replacements */
char *template = TEMPLATE;	/* renaming template */
int test = 0;			/* print what would be moved, but do not move anything */


void usage (int status)
{
	if (0 == status) {
		fprintf (stdout, "%s: usage: cuerename [-v] [-o options] [-t cue|toc] -f <file> files...\n", progname);
		fputs ("\
\n\
OPTIONS\n\
-v				verbose; print filenames\n\
-o options			comma separated string of options:\n\
   case=none|lower|upper	perform case conversion\n\
   underscore			replace spaces with underscores\n\
   strict[=CHARS]		strict filenames (only allow safe characters),\n\
   				can optionally specify allowable characters\n\
   				(besides alphanumeric)\n\
   nostrict			do not use strict filenames\n\
   replace=FROM/TO		replace occurances of FROM with TO\n\
   template=TEMPLATE		set template (see Template Expansion)\n\
   test				test mode; do not move files, implies -v\n\
   notest			unset test mode\n\
\n\
Template Expansion\n\
%N - number of tracks\n\
%n - track number\n\
%T - album title\n\
%t - track title\n\
%M - album message\n\
%m - track message\n\
%A - album performer\n\
%a - track performer\n\
%G - album genre\n\
%g - track genre\n\
%F - original filename suffix (not including '.')\n\
%f - original filename prefix (not including '.')\n\
%. - '.' if original filename contains one\n\
\n\
Any other %<character> is expanded to that character.  For example, to get a\n\
'%', use %%.\n\
\n", stdout);
		fprintf (stdout, "default template is \"%s\"\n", TEMPLATE);
		fprintf (stdout, "strict characters include alphanumeric and \"%s\"\n", CHARS);
	} else {
		fprintf (stderr, "%s: syntax error\n", progname);
		fprintf (stderr, "run `%s' for usage\n", progname);
	}

	exit (status);
}

void replace_all (char *s, char *from, char *to)
{
	char *save, *p;

	if (NULL != (s = strstr (s, from))) {
		save = strdup (s + strlen (from));

		p = to;
		while ('\0' != *p)
			*s++ = *p++;

		strcpy (s, save);

		replace_all (s, from, to);
	}
}

void replacestr (char *s)
{
	int i;
	char *from, *to, *p;

	for (i = 0; i < num_replace; i++) {
		from = strdup (replace[i]);
		to = NULL;

		for (p = from; '\0' != *p; p++) {
			if ('\\' == *p) /* strip escape character */
				strcpy (p, p + 1);
			else if ('/' == *p && NULL == to)
				to = p;
		}
		if (NULL != to) {
			*to++ = '\0';
			replace_all (s, from, to);
		}
	}
}

void lowerstr (char *s)
{
	for (; '\0' != *s; s++)
		*s = tolower (*s);
}

void upperstr (char *s)
{
	for (; '\0' != *s; s++)
		*s = toupper (*s);
}

void spacetounderscore (char *s)
{
	for (; '\0' != *s; s++)
		if (' ' == *s)
			*s = '_';
}

void strictchar (char *s)
{
	for (; *s != '\0'; s++)
		if (0 == isalnum (*s) && NULL == strchr (strict, *s))
			strcpy (s, s + 1);	/* remove char from string and shift 1 */
}

void addfield (char *s, char *field)
{
	char *field_cpy;

	if (NULL != field) {
		field_cpy = strdup (field);

		replacestr (field_cpy);
		if (LOWER == caseconv)
			lowerstr (field_cpy);
		else if (UPPER == caseconv)
			upperstr (field_cpy);
		if (1 == underscore)
			spacetounderscore (field_cpy);
		if (NULL != strict)
			strictchar (field_cpy);

		strcat (s, field_cpy);
	}
}

void addsuffix (char *s, char *filename)
{
	char *p;

	if (NULL != (p = strrchr (filename, '.')))
		addfield (s, p + 1);
}

void addprefix (char *s, char *filename)
{
	char *filename_cpy, *p;

	filename_cpy = strdup (filename);

	if (NULL != (p = strchr (filename_cpy, '.')))
		*p = '\0';

	addfield (s, filename_cpy);
}

void build_filenames (Cd *cd, char *newfn[MAXTRACKS], char **oldfn)
{
	int i;
	char s[MAXFILENAME];	/* new filename string */
	char *c;

	/* todo: better cheking for oldfn bounds */
	for (i = 0; i < cd->ntrack && NULL != oldfn[i]; i++) {
		s[0] = '\0';
		for (c = template; '\0' != *c; c++) {
			if ('%' == *c) {
				c++;
				switch (*c) {
				case 'A':
					addfield (s, cdtext_get (PERFORMER, cd->cdtext));
					break;
				case 'a':
					addfield (s, cdtext_get (PERFORMER, cd->tracks[i].cdtext));
					break;
				case 'T':
					addfield (s, cdtext_get (TITLE, cd->cdtext));
					break;
				case 't':
					addfield (s, cdtext_get (TITLE, cd->tracks[i].cdtext));
					break;
				case 'G':
					addfield (s, cdtext_get (GENRE, cd->cdtext));
					break;
				case 'g':
					addfield (s, cdtext_get (GENRE, cd->tracks[i].cdtext));
					break;
				case 'M':
					addfield (s, cdtext_get (MESSAGE, cd->cdtext));
					break;
				case 'm':
					addfield (s, cdtext_get (MESSAGE, cd->tracks[i].cdtext));
					break;
				case 'N':
					sprintf (&s[strlen (s)], "%02d", cd->ntrack);
					break;
				case 'n':
					sprintf (&s[strlen (s)], "%02d", i + 1);
					break;
				case 'F':
					addsuffix (s, oldfn[i]);
					break;
				case 'f':
					addprefix (s, oldfn[i]);
					break;
				case '.':
					if (NULL != strchr (oldfn[i], '.'))
						strncat (s, ".", 1);
					break;
				default:
					strncat (s, c, 1);
				}
			} else {
				strncat (s, c, 1);
			}
		}

		newfn[i] = strdup (s);
	}
}

void rename_files (char **oldfn, char **newfn)
{
	for (; NULL != *oldfn && NULL != *newfn; oldfn++, newfn++) {
		if (1 == verbose)
			fprintf (stdout, "%s -> %s\n", *oldfn, *newfn);
		if (0 == test)
			rename (*oldfn, *newfn);
	}
}

int set_opts (char *subopts)
{
	char *value;

	while ('\0' != *subopts) {
		switch (getsubopt (&subopts, options, &value)) {
		case OPT_CASE:
			if (NULL != value)
				if (0 == strcmp ("lower", value))
					caseconv = LOWER;
				else if (0 == strcmp ("upper", value))
					caseconv = UPPER;
				else if (0 == strcmp ("none", value))
					caseconv = NOCONV;
				else
					return (1);
			else
				return (1);
			break;
		case OPT_UNDERSCORE:
			if (NULL != value)
				return (1);
			else
				underscore = 1;
			break;
		case OPT_STRICT:
			if (NULL != value)
				strict = strdup (value);
			else
				strict = CHARS;
			break;
		case OPT_NOSTRICT:
			if (NULL != value)
				return (1);
			else
				strict = NULL;
			break;
		case OPT_REPLACE:
			if (NULL != value)
				replace[num_replace++] = strdup (value);
			else
				return (1);
			break;
		case OPT_TEMPLATE:
			if (NULL != value)
				template = strdup (value);
			else
				return (1);
			break;
		case OPT_TEST:
			if (NULL != value)
				return (1);
			else
				test = verbose = 1;
			break;
		case OPT_NOTEST:
			if (NULL != value)
				return (1);
			else
				test = 0;
			break;
		default:
			return (1);
		}
	}

	return (0);
}

void parse_opt_file (FILE *fp)
{
	char buffer[256];
	int line_num = 0;

	while (NULL != fgets (buffer, sizeof (buffer), fp)) {
		line_num++;

		if ('#' == buffer[0]) {
			;	/* comment */
		} else {
			if ('\n' == buffer[strlen (buffer) - 1])
				buffer[strlen (buffer) - 1] = '\0';

			if (0 != set_opts (buffer))
				fprintf (stderr, "%d: warning: error processing option file\n", line_num);
		}
	}
}

void read_opt_file ()
{
	char *home = NULL;
	char opt_file[MAXFILENAME];
	FILE *fp;

	if (NULL == (home = getenv ("HOME"))) {
		fprintf (stderr, "warning:  cannot determine home directory\n");
		return;
	} else {
		strcpy (opt_file, home);
		strcat (opt_file, "/");
		strcat (opt_file, OPT_FILE);
	}

	if (NULL != (fp = fopen (opt_file, "rb"))) {
		parse_opt_file (fp);
		fclose (fp);
	}
}

int main (int argc, char **argv)
{
	Cd cd;
	char **oldfn;			/* array of old filenames */
	char *newfn[MAXTRACKS + 1];	/* array of new filenmaes */
	CueFile file = {NULL, NULL, UNKNOWN};
	/* getsubopt () variables */
	char *subopts;
	/* getopt () variables */
	char c;
	extern char *optarg;
	extern int optind;

	progname = *argv;

	read_opt_file ();

	while (-1 != (c = getopt (argc, argv, "f:t:vo:"))) {
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
		case 'o':
			subopts = optarg;
			if (0 != set_opts (subopts))
				usage (1);
			break;
		default:
			usage (1);
		}
	}

	if (1 == argc) {
		usage (0);
	} else if (optind < argc && NULL != file.file) {
		oldfn = argv + optind;
		cue_read_file (&cd, &file);
		build_filenames (&cd, newfn, oldfn);
		rename_files (oldfn, newfn);
	} else {
		usage (1);
	}

	return (0);
}
