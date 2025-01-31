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
 * eject implementation for busybox
 *
 * Copyright (C) 2004  Peter Willis <psyphreak@phreaker.net>
 * Copyright (C) 2005  Tito Ragusa <farmatito@tiscali.it>
 *
 * Licensed under the GPL v2 or later, see the file LICENSE in this tarball.
 */

/*
 * This is a simple hack of eject based on something Erik posted in #uclibc.
 * Most of the dirty work blatantly ripped off from cat.c =)
 */

#include "libbb.h"

/* various defines swiped from linux/cdrom.h */
#define CDROMCLOSETRAY            0x5319  /* pendant of CDROMEJECT  */
#define CDROMEJECT                0x5309  /* Ejects the cdrom media */
#define CDROM_DRIVE_STATUS        0x5326  /* Get tray position, etc. */
/* drive status possibilities returned by CDROM_DRIVE_STATUS ioctl */
#define CDS_TRAY_OPEN        2

#define dev_fd 3

/* Code taken from the original eject (http://eject.sourceforge.net/),
 * refactored it a bit for busybox (ne-bb@nicoerfurth.de) */

#include <scsi/sg.h>
#include <scsi/scsi.h>

static void eject_scsi(const char *dev)
{
	static const char sg_commands[3][6] = {
		{ ALLOW_MEDIUM_REMOVAL, 0, 0, 0, 0, 0 },
		{ START_STOP, 0, 0, 0, 1, 0 },
		{ START_STOP, 0, 0, 0, 2, 0 }
	};

	unsigned i;
	unsigned char sense_buffer[32];
	unsigned char inqBuff[2];
	sg_io_hdr_t io_hdr;

	if ((ioctl(dev_fd, SG_GET_VERSION_NUM, &i) < 0) || (i < 30000))
		bb_error_msg_and_die("not a sg device or old sg driver");

	memset(&io_hdr, 0, sizeof(sg_io_hdr_t));
	io_hdr.interface_id = 'S';
	io_hdr.cmd_len = 6;
	io_hdr.mx_sb_len = sizeof(sense_buffer);
	io_hdr.dxfer_direction = SG_DXFER_NONE;
	/* io_hdr.dxfer_len = 0; */
	io_hdr.dxferp = inqBuff;
	io_hdr.sbp = sense_buffer;
	io_hdr.timeout = 2000;

	for (i = 0; i < 3; i++) {
		io_hdr.cmdp = (void *)sg_commands[i];
		ioctl_or_perror_and_die(dev_fd, SG_IO, (void *)&io_hdr, "%s", dev);
	}

	/* force kernel to reread partition table when new disc is inserted */
	ioctl(dev_fd, BLKRRPART);
}

#define FLAG_CLOSE  1
#define FLAG_SMART  2
#define FLAG_SCSI   4

static void eject_cdrom(unsigned flags, const char *dev)
{
	int cmd = CDROMEJECT;

	if (flags & FLAG_CLOSE
	 || (flags & FLAG_SMART && ioctl(dev_fd, CDROM_DRIVE_STATUS) == CDS_TRAY_OPEN)
	) {
		cmd = CDROMCLOSETRAY;
	}

	ioctl_or_perror_and_die(dev_fd, cmd, NULL, "%s", dev);
}

int eject_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int eject_main(int argc UNUSED_PARAM, char **argv)
{
	unsigned flags;
	const char *device;

	opt_complementary = "?1:t--T:T--t";
	flags = getopt32(argv, "tT" USE_FEATURE_EJECT_SCSI("s"));
	device = argv[optind] ? argv[optind] : "/dev/cdrom";

	/* We used to do "umount <device>" here, but it was buggy
	   if something was mounted OVER cdrom and
	   if cdrom is mounted many times.

	   This works equally well (or better):
	   #!/bin/sh
	   umount /dev/cdrom
	   eject /dev/cdrom
	*/

	xmove_fd(xopen(device, O_RDONLY|O_NONBLOCK), dev_fd);

	if (ENABLE_FEATURE_EJECT_SCSI && (flags & FLAG_SCSI))
		eject_scsi(device);
	else
		eject_cdrom(flags, device);

	if (ENABLE_FEATURE_CLEAN_UP)
		close(dev_fd);

	return EXIT_SUCCESS;
}
