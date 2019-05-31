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
#include <openssl/bio.h>

#if BIO_FLAGS_UPLINK==0
/* Shortcut UPLINK calls on most platforms... */
#define	UP_stdin	stdin
#define	UP_stdout	stdout
#define	UP_stderr	stderr
#define	UP_fprintf	fprintf
#define	UP_fgets	fgets
#define	UP_fread	fread
#define	UP_fwrite	fwrite
#undef	UP_fsetmod
#define	UP_feof		feof
#define	UP_fclose	fclose

#define	UP_fopen	fopen
#define	UP_fseek	fseek
#define	UP_ftell	ftell
#define	UP_fflush	fflush
#define	UP_ferror	ferror
#define	UP_fileno	fileno

#define	UP_open		open
#define	UP_read		read
#define	UP_write	write
#define	UP_lseek	lseek
#define	UP_close	close
#endif
