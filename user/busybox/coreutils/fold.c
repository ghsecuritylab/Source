/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/* vi: set sw=4 ts=4: */
/* fold -- wrap each input line to fit in specified width.

   Written by David MacKenzie, djm@gnu.ai.mit.edu.
   Copyright (C) 91, 1995-2002 Free Software Foundation, Inc.

   Modified for busybox based on coreutils v 5.0
   Copyright (C) 2003 Glenn McGrath

   Licensed under the GPL v2 or later, see the file LICENSE in this tarball.
*/

#include "libbb.h"

/* Must match getopt32 call */
#define FLAG_COUNT_BYTES        1
#define FLAG_BREAK_SPACES       2
#define FLAG_WIDTH              4

/* Assuming the current column is COLUMN, return the column that
   printing C will move the cursor to.
   The first column is 0. */
static int adjust_column(int column, char c)
{
	if (!(option_mask32 & FLAG_COUNT_BYTES)) {
		if (c == '\b') {
			if (column > 0)
				column--;
		} else if (c == '\r')
			column = 0;
		else if (c == '\t')
			column = column + 8 - column % 8;
		else			/* if (isprint (c)) */
			column++;
	} else
		column++;
	return column;
}

int fold_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int fold_main(int argc, char **argv)
{
	char *line_out = NULL;
	int allocated_out = 0;
	char *w_opt;
	int width = 80;
	int i;
	int errs = 0;

	if (ENABLE_INCLUDE_SUSv2) {
		/* Turn any numeric options into -w options.  */
		for (i = 1; i < argc; i++) {
			char const *a = argv[i];

			if (*a++ == '-') {
				if (*a == '-' && !a[1]) /* "--" */
					break;
				if (isdigit(*a))
					argv[i] = xasprintf("-w%s", a);
			}
		}
	}

	getopt32(argv, "bsw:", &w_opt);
	if (option_mask32 & FLAG_WIDTH)
		width = xatoul_range(w_opt, 1, 10000);

	argv += optind;
	if (!*argv)
		*--argv = (char*)"-";

	do {
		FILE *istream = fopen_or_warn_stdin(*argv);
		int c;
		int column = 0;		/* Screen column where next char will go. */
		int offset_out = 0;	/* Index in 'line_out' for next char. */

		if (istream == NULL) {
			errs |= EXIT_FAILURE;
			continue;
		}

		while ((c = getc(istream)) != EOF) {
			if (offset_out + 1 >= allocated_out) {
				allocated_out += 1024;
				line_out = xrealloc(line_out, allocated_out);
			}

			if (c == '\n') {
				line_out[offset_out++] = c;
				fwrite(line_out, sizeof(char), (size_t) offset_out, stdout);
				column = offset_out = 0;
				continue;
			}
 rescan:
			column = adjust_column(column, c);

			if (column > width) {
				/* This character would make the line too long.
				   Print the line plus a newline, and make this character
				   start the next line. */
				if (option_mask32 & FLAG_BREAK_SPACES) {
					/* Look for the last blank. */
					int logical_end;

					for (logical_end = offset_out - 1; logical_end >= 0; logical_end--) {
						if (isblank(line_out[logical_end])) {
							break;
						}
					}
					if (logical_end >= 0) {
						/* Found a blank.  Don't output the part after it. */
						logical_end++;
						fwrite(line_out, sizeof(char), (size_t) logical_end, stdout);
						bb_putchar('\n');
						/* Move the remainder to the beginning of the next line.
						   The areas being copied here might overlap. */
						memmove(line_out, line_out + logical_end, offset_out - logical_end);
						offset_out -= logical_end;
						for (column = i = 0; i < offset_out; i++) {
							column = adjust_column(column, line_out[i]);
						}
						goto rescan;
					}
				} else {
					if (offset_out == 0) {
						line_out[offset_out++] = c;
						continue;
					}
				}
				line_out[offset_out++] = '\n';
				fwrite(line_out, sizeof(char), (size_t) offset_out, stdout);
				column = offset_out = 0;
				goto rescan;
			}

			line_out[offset_out++] = c;
		}

		if (offset_out) {
			fwrite(line_out, sizeof(char), (size_t) offset_out, stdout);
		}

		if (fclose_if_not_stdin(istream)) {
			bb_simple_perror_msg(*argv);	/* Avoid multibyte problems. */
			errs |= EXIT_FAILURE;
		}
	} while (*++argv);

	fflush_stdout_and_exit(errs);
}
