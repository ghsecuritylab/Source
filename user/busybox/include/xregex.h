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
 * Busybox xregcomp utility routine.  This isn't in libbb.h because the
 * C library we're linking against may not support regex.h.
 *
 * Based in part on code from sash, Copyright (c) 1999 by David I. Bell
 * Permission has been granted to redistribute this code under the GPL.
 *
 * Licensed under GPLv2 or later, see file License in this tarball for details.
 */
#ifndef __BB_REGEX__
#define __BB_REGEX__

#include <regex.h>

#if __GNUC_PREREQ(4,1)
# pragma GCC visibility push(hidden)
#endif

char* regcomp_or_errmsg(regex_t *preg, const char *regex, int cflags) FAST_FUNC;
void xregcomp(regex_t *preg, const char *regex, int cflags) FAST_FUNC;

#if __GNUC_PREREQ(4,1)
# pragma GCC visibility pop
#endif

#endif
