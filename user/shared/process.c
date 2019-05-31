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
/*
 * Copyright 2013, ASUSTek Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of ASUSTek Inc.;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of ASUSTek Inc..
 */

#include <stdio.h>

/* Find process name by pid from /proc directory */
char *find_proc_name_by_pid(int pid)
{
	FILE *fp;
	char _file[32], _line[256], _name[64];

	snprintf(_file, sizeof(_file), "/proc/%d/status", pid);

	if ((fp = fopen(_file, "r"))) {
		fgets(_line, sizeof(_line), fp);
		sscanf(_line, "Name:\t%s", _name);
		fclose(fp);
		return _name;
	}
	return "";
}
