/* cue.c
 * - toc/cue data structures and functions
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
#include <stdlib.h>
#include "cue.h"
#include "msf.h"

#ifdef __EMX__
	#include "os2/os2additional.h"
#endif

#define MAXLINE 4096		/* maximum line length + 1 */


void parse_toc (Cd *cd, char *line, int line_num)
{
	char *keyword, *field;
	int i = cd->ntrack - 1;

	/* strip comment from line */
	/* todo: // in quoted strings? */
	/* //comment */
	if (NULL != (field = strstr (line, "//")))
		*field = '\0';

	if (NULL != (keyword = strtok (line, " \t\n\r"))) {
		/* CATALOG "ddddddddddddd" */
		if (0 == strcmp ("CATALOG", keyword)) {
			if (-1 == i) {
				if (NULL != (field = strtok (NULL, "\"\t\n\r")))
					cd->catalog = strdup (field);
				if (NULL != (field = strtok (NULL, " \t\n\r")))
					fprintf (stderr, "%d: format error: %s\n", line_num, keyword);
			} else {
				fprintf (stderr, "%d: only allowed in global section: %s\n", line_num, keyword);
			}

		/* CD_DA | CD_ROM | CD_ROM_XA */
		} else if (0 == strcmp ("CD_DA", keyword)) {
			if (-1 == i)
				cd->mode = CD_DA;
			else
				fprintf (stderr, "%d: only allowed in global section: %s\n", line_num, keyword);
		} else if (0 == strcmp ("CD_ROM", keyword)) {
			if (-1 == i)
				cd->mode = CD_ROM;
			else
				fprintf (stderr, "%d: only allowed in global section: %s\n", line_num, keyword);
		} else if (0 == strcmp ("CD_ROM_XA", keyword)) {
			if (-1 == i)
				cd->mode = CD_ROM_XA;
			else
				fprintf (stderr, "%d: only allowed in global section: %s\n", line_num, keyword);

		/* TRACK <track-mode> [<sub-channel-mode>] */
		} else if (0 == strcmp ("TRACK", keyword)) {
			i++;
			cd->tracks[i].cdtext = NULL;
			if (NULL != (field = strtok (NULL, " \t\n\r"))) {
				if (0 == strcmp ("AUDIO", field))
					cd->tracks[i].mode = AUDIO;
				else if (0 == strcmp ("MODE1", field))
					cd->tracks[i].mode = MODE1;
				else if (0 == strcmp ("MODE1_RAW", field))
					cd->tracks[i].mode = MODE1_RAW;
				else if (0 == strcmp ("MODE2", field))
					cd->tracks[i].mode = MODE2;
				else if (0 == strcmp ("MODE2_FORM1", field))
					cd->tracks[i].mode = MODE2_FORM1;
				else if (0 == strcmp ("MODE2_FORM2", field))
					cd->tracks[i].mode = MODE2_FORM2;
				else if (0 == strcmp ("MODE2_FORM_MIX", field))
					cd->tracks[i].mode = MODE2_FORM_MIX;
				else if (0 == strcmp ("MODE2_RAW", field))
					cd->tracks[i].mode = MODE2_RAW;
				else
					fprintf (stderr, "%d: format error: %s\n", line_num, keyword);
			}
			if (NULL != (field = strtok (NULL, " \t\n\r"))) {
				/* todo: set sub-channel-mode */
				if (0 == strcmp ("RW", field))
					;
				else if (0 == strcmp ("RW_RAW", field))
					;
			}
			if (NULL != (field = strtok (NULL, " \t\n\r")))
				fprintf (stderr, "%d: format error: %s\n", line_num, keyword);

		/* track flags */
		/* [NO] COPY | [NO] PRE_EMPHASIS */
		} else if (0 == strcmp ("NO", keyword)) {
			if (NULL != (field = strtok (NULL, " \t\n\r"))) {
				if (0 == strcmp ("COPY", field))
					cd->tracks[i].flags &= ~COPY_PERMITTED;
				else if (0 == strcmp ("PRE_EMPHASIS", field))
					cd->tracks[i].flags &= ~PRE_EMPHASIS;
			} else {
				fprintf (stderr, "%d: format error: %s\n", line_num, keyword);
			}
			if (NULL != (field = strtok (NULL, " \t\n\r")))
				fprintf (stderr, "%d: format error: %s\n", line_num, keyword);
		} else if (0 == strcmp ("COPY", keyword)) {
			cd->tracks[i].flags |= COPY_PERMITTED;
		} else if (0 == strcmp ("PRE_EMPHASIS", keyword)) {
			cd->tracks[i].flags |= PRE_EMPHASIS;
		/* TWO_CHANNEL_AUDIO */
		} else if (0 == strcmp ("TWO_CHANNEL_AUDIO", keyword)) {
			cd->tracks[i].flags &= ~FOUR_CHANNEL_AUDIO;
		/* FOUR_CHANNEL_AUDIO */
		} else if (0 == strcmp ("FOUR_CHANNEL_AUDIO", keyword)) {
			cd->tracks[i].flags |= FOUR_CHANNEL_AUDIO;

		/* ISRC "CCOOOYYSSSSS" */
		} else if (0 == strcmp ("ISRC", keyword)) {
			if (NULL != (field = strtok (NULL, "\"\t\n\r"))) {
				cd->tracks[i].isrc = strdup(field);
			} else {
				fprintf (stderr, "%d: format error: %s\n", line_num, keyword);
			}

		/* SILENCE <length> */
		} else if (0 == strcmp ("SILENCE", keyword)) {
			fprintf (stderr, "%d: warning: unimplemented keyword: %s\n", line_num, keyword);

		/* ZERO <length> */
		} else if (0 == strcmp ("ZERO", keyword)) {
			fprintf (stderr, "%d: warning: unimplemented keyword: %s\n", line_num, keyword);

		/* [FILE|AUDIOFILE] "<filename>" <start> [<length>] */
		} else if (0 == strcmp ("FILE", keyword) || 0 == strcmp ("AUDIOFILE", keyword)) {
			if (0 <= i) {
				if (NULL != (field = strtok (NULL, "\"\t\n\r")))
					cd->tracks[i].filename = strdup (field);
				if (NULL != (field = strtok (NULL, " \t\n\r")))
					cd->tracks[i].indexes[cd->tracks[i].nindex++] = msf_frame_from_mmssff (field);
				if (NULL != (field = strtok (NULL, " \t\n\r")))
					cd->tracks[i].length = msf_frame_from_mmssff (field);
				if (NULL != (field = strtok (NULL, " \t\n\r")))
					fprintf (stderr, "%d: format error: %s\n", line_num, keyword);
			} else {
				fprintf (stderr, "%d: not allowed in global section: %s\n", line_num, keyword);
			}

		/* DATAFILE "<filename>" <start> [<length>] */
		} else if (0 == strcmp ("DATAFILE", keyword)) {
			fprintf (stderr, "%d: warning: unimplemented keyword: %s\n", line_num, keyword);

		/* FIFO "<fifo path>" [<length>] */
		} else if (0 == strcmp ("FIFO", keyword)) {
			fprintf (stderr, "%d: warning: unimplemented keyword: %s\n", line_num, keyword);

		/* START MM:SS:FF */
		} else if (0 == strcmp ("START", keyword)) {
			if (0 <= i) {
				if (NULL != (field = strtok (NULL, " \t\n\r")))
					/* todo: line is too long! */
					cd->tracks[i].indexes[cd->tracks[i].nindex++] = msf_frame_from_mmssff (field) + cd->tracks[i].indexes[cd->tracks[i].nindex - 2];
				if (NULL != (field = strtok (NULL, " \t\n\r")))
					fprintf (stderr, "%d: format error: %s\n", line_num, keyword);
			} else {
				fprintf (stderr, "%d: not allowed in global section: %s\n", line_num, keyword);
			}

		/* PREGAP MM:SS:FF */
		} else if (0 == strcmp ("PREGAP", keyword)) {
			if (0 <= i) {
				if (NULL != (field = strtok (NULL, " \t\n\r")))
					cd->tracks[i].pregap = msf_frame_from_mmssff (field);
				else
					fprintf (stderr, "%d: format error: %s\n", line_num, keyword);
				if (NULL != (field = strtok (NULL, " \t\n\r")))
					fprintf (stderr, "%d: format error: %s\n", line_num, keyword);
			} else {
				fprintf (stderr, "%d: not allowed in global section: %s\n", line_num, keyword);
			}

		/* INDEX MM:SS:FF */
		} else if (0 == strcmp ("INDEX", keyword)) {
			if (0 <= i) {
				if (NULL != (field = strtok (NULL, " \t\n\r")))
					cd->tracks[i].indexes[cd->tracks[i].nindex++] = msf_frame_from_mmssff (field);
				else
					fprintf (stderr, "%d: format error: %s\n", line_num, keyword);
				if (NULL != (field = strtok (NULL, " \t\n\r")))
					fprintf (stderr, "%d: format error: %s\n", line_num, keyword);
			} else {
				fprintf (stderr, "%d: not allowed in global section: %s\n", line_num, keyword);
			}

		/* CD_TEXT { ... } */
		/* todo: opening { must be on same line as CD_TEXT */
		} else if (0 == strcmp ("CD_TEXT", keyword)) {
		} else if (0 == strcmp ("LANGUAGE_MAP", keyword)) {
		} else if (0 == strcmp ("LANGUAGE", keyword)) {
		} else if (0 == strcmp ("}", keyword)) {
		} else if (0 == cdtext_is_keyword (keyword)) {
			if (-1 == i) {
				if (NULL == cd->cdtext)
					cd->cdtext = cdtext_init ();
				cdtext_set (keyword, strtok (NULL, "\"\t\n\r"), cd->cdtext);
			} else {
				if (NULL == cd->tracks[i].cdtext)
					cd->tracks[i].cdtext = cdtext_init ();
				cdtext_set (keyword, strtok (NULL, "\"\t\n\r"), cd->tracks[i].cdtext);
			}

		/* unrecognized line */
		} else {
			fprintf(stderr, "%d: warning: unrecognized keyword: %s\n", line_num, keyword);
		}
	}

	cd->ntrack = i + 1;
}


void parse_cue (Cd *cd, char *line, int line_num)
{
	char *keyword, *field;
	int i = cd->ntrack - 1;

	if (NULL != (keyword = strtok (line, " \t\n\r"))) {
		/* REM remarks ... */
		if (0 == strcmp ("REM", keyword)) {
			;

		/* global section */
		/* CATALOG ddddddddddddd */
		} else if (0 == strcmp ("CATALOG", keyword)) {
			if (-1 == i) {
				if (NULL != (field = strtok (NULL, " \t\n\r")))
					cd->catalog = strdup (field);
				if (NULL != (field = strtok (NULL, " \t\n\r")))
					fprintf (stderr, "%s: format error\n", keyword);
			} else {
				fprintf (stderr, "%d: only allowed in global section: %s\n", line_num, keyword);
			}

		/* FILE "<filename>" <BINARY|WAVE|other?> */
		} else if (0 == strcmp ("FILE", keyword)) {
			if (NULL != (field = strtok (NULL, "\"\t\n\r")))
				cd->tracks[i + 1].filename = strdup (field);
			else
				fprintf (stderr, "%d: format error: %s\n", line_num, keyword);

		/* TRACK N <mode> */
		} else if (0 == strcmp ("TRACK", keyword)) {
			i++;
			cd->tracks[i].cdtext = NULL;
			if (NULL != (field = strtok (NULL, " \t\n\r")))
				; /* skip index number */
			if (NULL != (field = strtok (NULL, " \t\n\r"))) {
				if (0 == strcmp ("AUDIO", field))
					cd->tracks[i].mode = AUDIO;
				else if (0 == strcmp ("MODE1/2048", field))
					cd->tracks[i].mode = MODE1;
				else if (0 == strcmp ("MODE1/2352", field))
					cd->tracks[i].mode = MODE1_RAW;
				else if (0 == strcmp ("MODE2/2336", field))
					cd->tracks[i].mode = MODE2;
				else if (0 == strcmp ("MODE2/2048", field))
					cd->tracks[i].mode = MODE2_FORM1;
				else if (0 == strcmp ("MODE2/2324", field))
					cd->tracks[i].mode = MODE2_FORM2;
				else if (0 == strcmp ("MODE2/2336", field))
					cd->tracks[i].mode = MODE2_FORM_MIX;
				else if (0 == strcmp ("MODE2/2352", field))
					cd->tracks[i].mode = MODE2_RAW;
				else
					fprintf (stderr, "%d: format error: %s\n", line_num, keyword);
			} else {
				fprintf (stderr, "%d: format error: %s\n", line_num, keyword);
			}

		/* FLAGS flag1 flag2 ... */
		} else if (0 == strcmp ("FLAGS", keyword)) {
			if (0 <= i) {
				while (NULL != (field = strtok (NULL, " \t\n\r"))) {
					if (0 == strcmp ("PRE", field)) {
						cd->tracks[i].flags |= PRE_EMPHASIS;
					} else if (0 == strcmp ("DCP", field)) {
						cd->tracks[i].flags |= COPY_PERMITTED;
					} else if (0 == strcmp ("4CH", field)) {
						cd->tracks[i].flags |= FOUR_CHANNEL_AUDIO;
					} else if (0 == strcmp ("SCMS", field)) {
						cd->tracks[i].flags |= SCMS;
					} else {
						fprintf (stderr, "%d: format error: %s\n", line_num, keyword);
					}
				}
			} else {
				fprintf (stderr, "%d: not allowed in global section: %s\n", line_num, keyword);
			}

		/* ISRC CCOOOYYSSSSS */
		} else if (0 == strcmp ("ISRC", keyword)) {
			if (0 <= i) {
				if (NULL != (field = strtok (NULL, " \t\n\r")))
					cd->tracks[i].isrc = strdup (field);
				else
					fprintf (stderr, "%d: format error: %s\n", line_num, keyword);
			} else {
				fprintf (stderr, "%d: not allowed in global section: %s\n", line_num, keyword);
			}

		/* PREGAP MM:SS:FF */
		} else if (0 == strcmp ("PREGAP", keyword)) {
			if (0 <= i) {
				if (NULL != (field = strtok (NULL, " \t\n\r")))
					cd->tracks[i].pregap = msf_frame_from_mmssff (field);
				else
					fprintf (stderr, "%d: format error: %s\n", line_num, keyword);
				if (NULL != (field = strtok (NULL, " \t\n\r")))
					fprintf (stderr, "%d: format error: %s\n", line_num, keyword);
			} else {
				fprintf (stderr, "%d: not allowed in global section: %s\n", line_num, keyword);
			}

		/* INDEX [##] MM:SS:FF */
		} else if (0 == strcmp ("INDEX", keyword)) {
			if (0 <= i) {
				if (NULL != (field = strtok (NULL, " \t\n\r")))
					; /* skip index number */
				if (NULL != (field = strtok (NULL, " \t\n\r")))
					cd->tracks[i].indexes[cd->tracks[i].nindex++] = msf_frame_from_mmssff (field);
				else
					fprintf (stderr, "%d: format error: %s\n", line_num, keyword);
			} else {
				fprintf (stderr, "%d: not allowed in global section: %s\n", line_num, keyword);
			}

		/* CD-TEXT */
		} else if (0 == cdtext_is_keyword (keyword)) {
			if (-1 == i) {
				if (NULL == cd->cdtext)
					cd->cdtext = cdtext_init ();
				cdtext_set (keyword, strtok (NULL, "\"\t\n\r"), cd->cdtext);
			} else {
				if (NULL == cd->tracks[i].cdtext)
					cd->tracks[i].cdtext = cdtext_init ();
				cdtext_set (keyword, strtok (NULL, "\"\t\n\r"), cd->tracks[i].cdtext);
			}

		/* unrecognized line */
		} else {
			fprintf(stderr, "%d: warning: unrecognized keyword: %s\n", line_num, keyword);
		}
	}

	cd->ntrack = i + 1;
}


/* prints cd in toc format */
void print_toc (FILE *fp, Cd *cd)
{
	int i, j;

	/* print global information */
	if (NULL != cd->catalog)
		fprintf(fp, "CATALOG \"%s\"\n", cd->catalog);

	if (CD_DA == cd->mode)
		fprintf(fp, "CD_DA\n");
	else if (CD_ROM == cd->mode)
		fprintf(fp, "CD_ROM\n");
	else if (CD_ROM_XA == cd->mode)
		fprintf(fp, "CD_ROM_XA\n");

	/* CD-TEXT */
	/* todo: cdtext could  be non NULL, but with no valid CD-TEXT */
	if (NULL != cd->cdtext) {
		fprintf(fp, "CD_TEXT {\n");
		fprintf(fp, "LANGUAGE_MAP {\n");
		fprintf(fp, "0:9\n");
		fprintf(fp, "}\n");
		fprintf(fp, "LANGUAGE 0 {\n");
		cdtext_print (fp, cd->cdtext);
		fprintf(fp, "}\n");
		fprintf(fp, "}\n");
	}

	/* print track information */
	for (i = 0; i < cd->ntrack; i++) {
		fprintf (fp, "\n");
		fprintf (fp, "TRACK ");
		if (AUDIO == cd->tracks[i].mode)
			fprintf(fp, "AUDIO\n");
		else if (MODE1 == cd->tracks[i].mode)
			fprintf(fp, "MODE1\n");
		else if (MODE1_RAW == cd->tracks[i].mode)
			fprintf(fp, "MODE1_RAW\n");
		else if (MODE2 == cd->tracks[i].mode)
			fprintf(fp, "MODE2\n");
		else if (MODE2_FORM1 == cd->tracks[i].mode)
			fprintf(fp, "MODE2_FORM1\n");
		else if (MODE2_FORM2 == cd->tracks[i].mode)
			fprintf(fp, "MODE2_FORM2\n");
		else if (MODE2_FORM_MIX == cd->tracks[i].mode)
			fprintf(fp, "MODE2_FORM_MIX\n");
		else if (MODE2_RAW == cd->tracks[i].mode)
			fprintf(fp, "MODE2_RAW\n");

		if (cd->tracks[i].flags & PRE_EMPHASIS)
			fprintf(fp, "PRE_EMPHASIS\n");
		if (cd->tracks[i].flags & COPY_PERMITTED)
			fprintf(fp, "COPY\n");
		if (cd->tracks[i].flags & FOUR_CHANNEL_AUDIO)
			fprintf(fp, "FOUR_CHANNEL_AUDIO\n");

		if (NULL != cd->tracks[i].isrc)
			fprintf(fp, "ISRC \"%s\"\n", cd->tracks[i].isrc);

		/* CD-TEXT */
		if (NULL != cd->cdtext) {
			fprintf(fp, "CD_TEXT {\n");
			fprintf(fp, "LANGUAGE 0 {\n");
			cdtext_print (fp, cd->tracks[i].cdtext);
			fprintf(fp, "}\n");
			fprintf(fp, "}\n");
		}

		fprintf(fp, "FILE ");
		fprintf(fp, "\"%s\" ", cd_track_filename (cd, i));
		msf_print_frame (fp, cd->tracks[i].indexes[0]);
		fprintf(fp, " ");
		if (0 != cd->tracks[i].length) {
			msf_print_frame (fp, cd->tracks[i].length);
		} else {
			msf_print_frame (fp, cd_track_length (cd, i));
		}
		fprintf(fp, "\n");

		for (j = 1; j < cd->tracks[i].nindex; j++) {
			fprintf (fp, "START ");
			msf_print_frame (fp, cd->tracks[i].indexes[j] - cd->tracks[i].indexes[j - 1]);
			fprintf (fp, "\n");
		}
	}
}


/* prints cd in cue format */
void print_cue (FILE *fp, Cd *cd)
{
	int i, j;	/* track, index */

	/* print global information */
	if (NULL != cd->catalog)
		fprintf(fp, "CATALOG \"%s\"\n", cd->catalog);

	if (NULL != cd->cdtext)
		cdtext_print (fp, cd->cdtext);


	/* print track information */
	for (i = 0; i < cd->ntrack; i++) {
		fprintf(fp, "\n");
		/* todo: clean up this section */
		if (NULL != cd->tracks[i].filename) {
			/* always print filename for track 1, afterwards only
			 * print filename if it differs from the previous track */
			/* todo: can pass -1 to cd_track_filename() */
			if (0 == i || 0 != strcmp (cd->tracks[i].filename, cd_track_filename (cd, i - 1))) {
				fprintf(fp, "FILE \"%s\" ", cd->tracks[i].filename);
				/* todo: what file formats are allowed (BINARY, MOTOROLA, AIFF, AU, WAVE, MP3)? */
				if (AUDIO == cd->tracks[i].mode)
					fprintf(fp, "WAVE\n");
				else
					fprintf(fp, "BINARY\n");
			}
		}

		fprintf(fp, "TRACK %02d ", i + 1);
		if (AUDIO == cd->tracks[i].mode)
			fprintf(fp, "AUDIO\n");
		else if (MODE1 == cd->tracks[i].mode)
			fprintf(fp, "MODE1/2048\n");
		else if (MODE1_RAW == cd->tracks[i].mode)
			fprintf(fp, "MODE1/2352\n");
		else if (MODE2 == cd->tracks[i].mode)
			fprintf(fp, "MODE2/2336\n");
		else if (MODE2_FORM1 == cd->tracks[i].mode)
			fprintf(fp, "MODE2/2048\n");
		else if (MODE2_FORM2 == cd->tracks[i].mode)
			fprintf(fp, "MODE2/2324\n");
		else if (MODE2_FORM_MIX == cd->tracks[i].mode)
			fprintf(fp, "MODE2/2336\n");
		else if (MODE2_RAW == cd->tracks[i].mode)
			fprintf(fp, "MODE2/2352\n");

		if (NONE != cd->tracks[i].flags) {
			fprintf(fp, "FLAGS");
			if (cd->tracks[i].flags & PRE_EMPHASIS)
				fprintf(fp, " PRE");
			if (cd->tracks[i].flags & COPY_PERMITTED)
				fprintf(fp, " DCP");
			if (cd->tracks[i].flags & FOUR_CHANNEL_AUDIO)
				fprintf(fp, " 4CH");
			if (cd->tracks[i].flags & SCMS)
				fprintf(fp, " SCMS");
			fprintf(fp, "\n");
		}

		if (NULL != cd->tracks[i].isrc)
			fprintf(fp, "ISRC %s\n", cd->tracks[i].isrc);
		if (NULL != cd->cdtext)
			cdtext_print (fp, cd->tracks[i].cdtext);
		for (j = 0; j < cd->tracks[i].nindex; j++) {
			if (1 == cd->tracks[i].nindex)
				fprintf(fp, "INDEX 01 ");
			else
				fprintf(fp, "INDEX %02d ", j);
			msf_print_frame (fp, cd->tracks[i].indexes[j]);
			fprintf(fp, "\n");
		}
	}
}


/* guesses the file format based on the suffix */
Format file_format (char *file)
{
	char *suffix;

	if (0 != (suffix = strrchr (file, '.'))) {
		if (0 == strcasecmp (".cue", suffix))
			return (CUE);
		else if (0 == strcasecmp (".toc", suffix))
			return (TOC);
	}

	fprintf (stderr, "error: %s: %s\n", "ambiguous file", file);
	return (UNKNOWN);
}


void read_file (CueFile *file, Cd *cd)
{
	char buffer[MAXLINE];
	int line_num = 0;

	cd->cdtext = NULL;
	cd->ntrack = 0;

	while (NULL != fgets (buffer, sizeof (buffer), file->fp)) {
		line_num++;
		if (CUE == file->format)
			parse_cue (cd, buffer, line_num);
		else if (TOC == file->format)
			parse_toc (cd, buffer, line_num);
	}
}


void cue_read_file (Cd *cd, CueFile *file)
{
	if (CUE != file->format && TOC != file->format)
		file->format = file_format (file->file);

	if (UNKNOWN == file->format)
		exit (1);

	if (0 == strcmp ("-", file->file)) {	/* read from stdin */
		file->fp = stdin;
		read_file (file, cd);
	} else {				/* read from file */
		if (NULL == (file->fp = fopen (file->file, "rb"))) {
			fprintf (stderr, "error: %s: %s\n", "unable to open file for reading", file->file);
			exit (1);
		} else {
			read_file (file, cd);
		}
	}
}


void write_file (Cd *cd, CueFile *file)
{
	if (CUE == file->format)
		print_cue (file->fp, cd);
	else if (TOC == file->format)
		print_toc (file->fp, cd);
}


void cue_write_file (Cd *cd, CueFile *file)
{
	if (CUE != file->format && TOC != file->format)
		file->format = file_format (file->file);

	if (UNKNOWN == file->format)
		exit (1);

	if (0 == strcmp ("-", file->file)) {	/* write to stdout */
		file->fp = stdout;
		write_file (cd, file);
	} else {				/* write to file */
		if (NULL == (file->fp = fopen (file->file, "wb"))) {
			fprintf (stderr, "error: %s: %s\n", "unable to open file for writing", file->file);
			exit (1);
		} else {
			write_file (cd, file);
			fclose (file->fp);
		}
	}
}
