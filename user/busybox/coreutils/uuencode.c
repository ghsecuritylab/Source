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
/*
 *  Copyright (C) 2000 by Glenn McGrath
 *
 *  based on the function base64_encode from http.c in wget v1.6
 *  Copyright (C) 1995, 1996, 1997, 1998, 2000 Free Software Foundation, Inc.
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

#include "libbb.h"

enum {
	SRC_BUF_SIZE = 45,  /* This *MUST* be a multiple of 3 */
	DST_BUF_SIZE = 4 * ((SRC_BUF_SIZE + 2) / 3),
};

int uuencode_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int uuencode_main(int argc, char **argv)
{
	struct stat stat_buf;
	int src_fd = STDIN_FILENO;
	const char *tbl;
	mode_t mode;
	char src_buf[SRC_BUF_SIZE];
	char dst_buf[DST_BUF_SIZE + 1];

	tbl = bb_uuenc_tbl_std;
	mode = 0666 & ~umask(0666);
	opt_complementary = "-1:?2"; /* must have 1 or 2 args */
	if (getopt32(argv, "m")) {
		tbl = bb_uuenc_tbl_base64;
	}
	argv += optind;
	if (argc == optind + 2) {
		src_fd = xopen(*argv, O_RDONLY);
		fstat(src_fd, &stat_buf);
		mode = stat_buf.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
		argv++;
	}

	printf("begin%s %o %s", tbl == bb_uuenc_tbl_std ? "" : "-base64", mode, *argv);
	while (1) {
		size_t size = full_read(src_fd, src_buf, SRC_BUF_SIZE);
		if (!size)
			break;
		if ((ssize_t)size < 0)
			bb_perror_msg_and_die(bb_msg_read_error);
		/* Encode the buffer we just read in */
		bb_uuencode(dst_buf, src_buf, size, tbl);
		bb_putchar('\n');
		if (tbl == bb_uuenc_tbl_std) {
			bb_putchar(tbl[size]);
		}
		fflush(stdout);
		xwrite(STDOUT_FILENO, dst_buf, 4 * ((size + 2) / 3));
	}
	printf(tbl == bb_uuenc_tbl_std ? "\n`\nend\n" : "\n====\n");

	fflush_stdout_and_exit(EXIT_SUCCESS);
}
