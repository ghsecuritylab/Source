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
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

#include "libbb.h"
#include <sys/mtio.h>

/* missing: eod/seod, stoptions, stwrthreshold, densities */
static const short opcode_value[] = {
	MTBSF,
	MTBSFM,
	MTBSR,
	MTBSS,
	MTCOMPRESSION,
	MTEOM,
	MTERASE,
	MTFSF,
	MTFSFM,
	MTFSR,
	MTFSS,
	MTLOAD,
	MTLOCK,
	MTMKPART,
	MTNOP,
	MTOFFL,
	MTOFFL,
	MTRAS1,
	MTRAS2,
	MTRAS3,
	MTRESET,
	MTRETEN,
	MTREW,
	MTSEEK,
	MTSETBLK,
	MTSETDENSITY,
	MTSETDRVBUFFER,
	MTSETPART,
	MTTELL,
	MTWSM,
	MTUNLOAD,
	MTUNLOCK,
	MTWEOF,
	MTWEOF
};

static const char opcode_name[] ALIGN1 =
	"bsf"             "\0"
	"bsfm"            "\0"
	"bsr"             "\0"
	"bss"             "\0"
	"datacompression" "\0"
	"eom"             "\0"
	"erase"           "\0"
	"fsf"             "\0"
	"fsfm"            "\0"
	"fsr"             "\0"
	"fss"             "\0"
	"load"            "\0"
	"lock"            "\0"
	"mkpart"          "\0"
	"nop"             "\0"
	"offline"         "\0"
	"rewoffline"      "\0"
	"ras1"            "\0"
	"ras2"            "\0"
	"ras3"            "\0"
	"reset"           "\0"
	"retension"       "\0"
	"rewind"          "\0"
	"seek"            "\0"
	"setblk"          "\0"
	"setdensity"      "\0"
	"drvbuffer"       "\0"
	"setpart"         "\0"
	"tell"            "\0"
	"wset"            "\0"
	"unload"          "\0"
	"unlock"          "\0"
	"eof"             "\0"
	"weof"            "\0";

int mt_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int mt_main(int argc UNUSED_PARAM, char **argv)
{
	const char *file = "/dev/tape";
	struct mtop op;
	struct mtpos position;
	int fd, mode, idx;

	if (!argv[1]) {
		bb_show_usage();
	}

	if (strcmp(argv[1], "-f") == 0) {
		if (!argv[2] || !argv[3])
			bb_show_usage();
		file = argv[2];
		argv += 2;
	}

	idx = index_in_strings(opcode_name, argv[1]);

	if (idx < 0)
		bb_error_msg_and_die("unrecognized opcode %s", argv[1]);

	op.mt_op = opcode_value[idx];
	if (argv[2])
		op.mt_count = xatoi_u(argv[2]);
	else
		op.mt_count = 1;		/* One, not zero, right? */

	switch (opcode_value[idx]) {
		case MTWEOF:
		case MTERASE:
		case MTWSM:
		case MTSETDRVBUFFER:
			mode = O_WRONLY;
			break;

		default:
			mode = O_RDONLY;
			break;
	}

	fd = xopen(file, mode);

	switch (opcode_value[idx]) {
		case MTTELL:
			ioctl_or_perror_and_die(fd, MTIOCPOS, &position, "%s", file);
			printf("At block %d\n", (int) position.mt_blkno);
			break;

		default:
			ioctl_or_perror_and_die(fd, MTIOCTOP, &op, "%s", file);
			break;
	}

	return EXIT_SUCCESS;
}
