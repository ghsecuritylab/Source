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
 * adjtimex.c - read, and possibly modify, the Linux kernel `timex' variables.
 *
 * Originally written: October 1997
 * Last hack: March 2001
 * Copyright 1997, 2000, 2001 Larry Doolittle <LRDoolittle@lbl.gov>
 *
 * busyboxed 20 March 2001, Larry Doolittle <ldoolitt@recycle.lbl.gov>
 *
 * Licensed under GPLv2 or later, see file License in this tarball for details.
 */

#include "libbb.h"
#include <sys/timex.h>

static const uint16_t statlist_bit[] = {
	STA_PLL,
	STA_PPSFREQ,
	STA_PPSTIME,
	STA_FLL,
	STA_INS,
	STA_DEL,
	STA_UNSYNC,
	STA_FREQHOLD,
	STA_PPSSIGNAL,
	STA_PPSJITTER,
	STA_PPSWANDER,
	STA_PPSERROR,
	STA_CLOCKERR,
	0
};
static const char statlist_name[] =
	"PLL"       "\0"
	"PPSFREQ"   "\0"
	"PPSTIME"   "\0"
	"FFL"       "\0"
	"INS"       "\0"
	"DEL"       "\0"
	"UNSYNC"    "\0"
	"FREQHOLD"  "\0"
	"PPSSIGNAL" "\0"
	"PPSJITTER" "\0"
	"PPSWANDER" "\0"
	"PPSERROR"  "\0"
	"CLOCKERR"
;

static const char ret_code_descript[] =
	"clock synchronized" "\0"
	"insert leap second" "\0"
	"delete leap second" "\0"
	"leap second in progress" "\0"
	"leap second has occurred" "\0"
	"clock not synchronized"
;

int adjtimex_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int adjtimex_main(int argc, char **argv)
{
	enum {
		OPT_quiet = 0x1
	};
	unsigned opt;
	char *opt_o, *opt_f, *opt_p, *opt_t;
	struct timex txc;
	int i, ret;
	const char *descript;
	txc.modes=0;

	opt = getopt32(argv, "qo:f:p:t:",
			&opt_o, &opt_f, &opt_p, &opt_t);
	//if (opt & 0x1) // -q
	if (opt & 0x2) { // -o
		txc.offset = xatol(opt_o);
		txc.modes |= ADJ_OFFSET_SINGLESHOT;
	}
	if (opt & 0x4) { // -f
		txc.freq = xatol(opt_f);
		txc.modes |= ADJ_FREQUENCY;
	}
	if (opt & 0x8) { // -p
		txc.constant = xatol(opt_p);
		txc.modes |= ADJ_TIMECONST;
	}
	if (opt & 0x10) { // -t
		txc.tick = xatol(opt_t);
		txc.modes |= ADJ_TICK;
	}
	if (argc != optind) { /* no valid non-option parameters */
		bb_show_usage();
	}

	ret = adjtimex(&txc);

	if (ret < 0) {
		bb_perror_nomsg_and_die();
	}

	if (!(opt & OPT_quiet)) {
		int sep;
		const char *name;

		printf(
			"    mode:         %d\n"
			"-o  offset:       %ld\n"
			"-f  frequency:    %ld\n"
			"    maxerror:     %ld\n"
			"    esterror:     %ld\n"
			"    status:       %d (",
		txc.modes, txc.offset, txc.freq, txc.maxerror,
		txc.esterror, txc.status);

		/* representative output of next code fragment:
		   "PLL | PPSTIME" */
		name = statlist_name;
		sep = 0;
		for (i = 0; statlist_bit[i]; i++) {
			if (txc.status & statlist_bit[i]) {
				if (sep)
					fputs(" | ", stdout);
				fputs(name, stdout);
				sep = 1;
			}
			name += strlen(name) + 1;
		}

		descript = "error";
		if (ret <= 5)
			descript = nth_string(ret_code_descript, ret);
		printf(")\n"
			"-p  timeconstant: %ld\n"
			"    precision:    %ld\n"
			"    tolerance:    %ld\n"
			"-t  tick:         %ld\n"
			"    time.tv_sec:  %ld\n"
			"    time.tv_usec: %ld\n"
			"    return value: %d (%s)\n",
		txc.constant,
		txc.precision, txc.tolerance, txc.tick,
		(long)txc.time.tv_sec, (long)txc.time.tv_usec, ret, descript);
	}

	return 0;
}
