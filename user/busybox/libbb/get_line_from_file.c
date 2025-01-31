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
 * Utility routines.
 *
 * Copyright (C) 2005, 2006 Rob Landley <rob@landley.net>
 * Copyright (C) 2004 Erik Andersen <andersen@codepoet.org>
 * Copyright (C) 2001 Matt Krai
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

/* for getline() [GNUism]
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
*/
#include "libbb.h"

/* This function reads an entire line from a text file, up to a newline
 * or NUL byte, inclusive.  It returns a malloc'ed char * which
 * must be free'ed by the caller.  If end is NULL '\n' isn't considered
 * end of line.  If end isn't NULL, length of the chunk is stored in it.
 * If lineno is not NULL, *lineno is incremented for each line,
 * and also trailing '\' is recognized as line continuation.
 *
 * Returns NULL if EOF/error. */
char* FAST_FUNC bb_get_chunk_with_continuation(FILE *file, int *end, int *lineno)
{
	int ch;
	int idx = 0;
	char *linebuf = NULL;
	int linebufsz = 0;

	while ((ch = getc(file)) != EOF) {
		/* grow the line buffer as necessary */
		if (idx >= linebufsz) {
			linebufsz += 256;
			linebuf = xrealloc(linebuf, linebufsz);
		}
		linebuf[idx++] = (char) ch;
		if (!ch)
			break;
		if (end && ch == '\n') {
			if (lineno == NULL)
				break;
			(*lineno)++;
			if (idx < 2 || linebuf[idx-2] != '\\')
				break;
			idx -= 2;
		}
	}
	if (end)
		*end = idx;
	if (linebuf) {
		// huh, does fgets discard prior data on error like this?
		// I don't think so....
		//if (ferror(file)) {
		//	free(linebuf);
		//	return NULL;
		//}
		linebuf = xrealloc(linebuf, idx + 1);
		linebuf[idx] = '\0';
	}
	return linebuf;
}

char* FAST_FUNC bb_get_chunk_from_file(FILE *file, int *end)
{
	return bb_get_chunk_with_continuation(file, end, NULL);
}

/* Get line, including trailing \n if any */
char* FAST_FUNC xmalloc_fgets(FILE *file)
{
	int i;

	return bb_get_chunk_from_file(file, &i);
}
/* Get line.  Remove trailing \n */
char* FAST_FUNC xmalloc_fgetline(FILE *file)
{
	int i;
	char *c = bb_get_chunk_from_file(file, &i);

	if (i && c[--i] == '\n')
		c[i] = '\0';

	return c;
}

#if 0
/* GNUism getline() should be faster (not tested) than a loop with fgetc */

/* Get line, including trailing \n if any */
char* FAST_FUNC xmalloc_fgets(FILE *file)
{
	char *res_buf = NULL;
	size_t res_sz;

	if (getline(&res_buf, &res_sz, file) == -1) {
		free(res_buf); /* uclibc allocates a buffer even on EOF. WTF? */
		res_buf = NULL;
	}
//TODO: trimming to res_sz?
	return res_buf;
}
/* Get line.  Remove trailing \n */
char* FAST_FUNC xmalloc_fgetline(FILE *file)
{
	char *res_buf = NULL;
	size_t res_sz;

	res_sz = getline(&res_buf, &res_sz, file);

	if ((ssize_t)res_sz != -1) {
		if (res_buf[res_sz - 1] == '\n')
			res_buf[--res_sz] = '\0';
//TODO: trimming to res_sz?
	} else {
		free(res_buf); /* uclibc allocates a buffer even on EOF. WTF? */
		res_buf = NULL;
	}
	return res_buf;
}

#endif

#if 0
/* Faster routines (~twice as fast). +170 bytes. Unused as of 2008-07.
 *
 * NB: they stop at NUL byte too.
 * Performance is important here. Think "grep 50gigabyte_file"...
 * Ironically, grep can't use it because of NUL issue.
 * We sorely need C lib to provide fgets which reports size!
 *
 * Update:
 * Actually, uclibc and glibc have it. man getline. It's GNUism,
 *   but very useful one (if it's as fast as this code).
 * TODO:
 * - currently, sed and sort use bb_get_chunk_from_file and heavily
 *   depend on its "stop on \n or \0" behavior, and STILL they fail
 *   to handle all cases with embedded NULs correctly. So:
 * - audit sed and sort; convert them to getline FIRST.
 * - THEN ditch bb_get_chunk_from_file, replace it with getline.
 * - provide getline implementation for non-GNU systems.
 */

static char* xmalloc_fgets_internal(FILE *file, int *sizep)
{
	int len;
	int idx = 0;
	char *linebuf = NULL;

	while (1) {
		char *r;

		linebuf = xrealloc(linebuf, idx + 0x100);
		r = fgets(&linebuf[idx], 0x100, file);
		if (!r) {
			/* need to terminate in case this is error
			 * (EOF puts NUL itself) */
			linebuf[idx] = '\0';
			break;
		}
		/* stupid. fgets knows the len, it should report it somehow */
		len = strlen(&linebuf[idx]);
		idx += len;
		if (len != 0xff || linebuf[idx - 1] == '\n')
			break;
	}
	*sizep = idx;
	if (idx) {
		/* xrealloc(linebuf, idx + 1) is up to caller */
		return linebuf;
	}
	free(linebuf);
	return NULL;
}

/* Get line, remove trailing \n */
char* FAST_FUNC xmalloc_fgetline_fast(FILE *file)
{
	int sz;
	char *r = xmalloc_fgets_internal(file, &sz);
	if (r && r[sz - 1] == '\n')
		r[--sz] = '\0';
	return r; /* not xrealloc(r, sz + 1)! */
}

char* FAST_FUNC xmalloc_fgets(FILE *file)
{
	int sz;
	return xmalloc_fgets_internal(file, &sz);
}

/* Get line, remove trailing \n */
char* FAST_FUNC xmalloc_fgetline(FILE *file)
{
	int sz;
	char *r = xmalloc_fgets_internal(file, &sz);
	if (!r)
		return r;
	if (r[sz - 1] == '\n')
		r[--sz] = '\0';
	return xrealloc(r, sz + 1);
}
#endif
