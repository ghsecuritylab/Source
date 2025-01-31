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
#define APPMACROS_ONLY
#include "applink.c"

extern void *OPENSSL_UplinkTable[];

#define UP_stdin  (*(void *(*)(void))OPENSSL_UplinkTable[APPLINK_STDIN])()
#define UP_stdout (*(void *(*)(void))OPENSSL_UplinkTable[APPLINK_STDOUT])()
#define UP_stderr (*(void *(*)(void))OPENSSL_UplinkTable[APPLINK_STDERR])()
#define UP_fprintf (*(int (*)(void *,const char *,...))OPENSSL_UplinkTable[APPLINK_FPRINTF])
#define UP_fgets  (*(char *(*)(char *,int,void *))OPENSSL_UplinkTable[APPLINK_FGETS])
#define UP_fread  (*(size_t (*)(void *,size_t,size_t,void *))OPENSSL_UplinkTable[APPLINK_FREAD])
#define UP_fwrite (*(size_t (*)(const void *,size_t,size_t,void *))OPENSSL_UplinkTable[APPLINK_FWRITE])
#define UP_fsetmod (*(int (*)(void *,char))OPENSSL_UplinkTable[APPLINK_FSETMOD])
#define UP_feof   (*(int (*)(void *))OPENSSL_UplinkTable[APPLINK_FEOF])
#define UP_fclose (*(int (*)(void *))OPENSSL_UplinkTable[APPLINK_FCLOSE])

#define UP_fopen  (*(void *(*)(const char *,const char *))OPENSSL_UplinkTable[APPLINK_FOPEN])
#define UP_fseek  (*(int (*)(void *,long,int))OPENSSL_UplinkTable[APPLINK_FSEEK])
#define UP_ftell  (*(long (*)(void *))OPENSSL_UplinkTable[APPLINK_FTELL])
#define UP_fflush (*(int (*)(void *))OPENSSL_UplinkTable[APPLINK_FFLUSH])
#define UP_ferror (*(int (*)(void *))OPENSSL_UplinkTable[APPLINK_FERROR])
#define UP_clearerr (*(void (*)(void *))OPENSSL_UplinkTable[APPLINK_CLEARERR])
#define UP_fileno (*(int (*)(void *))OPENSSL_UplinkTable[APPLINK_FILENO])

#define UP_open   (*(int (*)(const char *,int,...))OPENSSL_UplinkTable[APPLINK_OPEN])
#define UP_read   (*(ssize_t (*)(int,void *,size_t))OPENSSL_UplinkTable[APPLINK_READ])
#define UP_write  (*(ssize_t (*)(int,const void *,size_t))OPENSSL_UplinkTable[APPLINK_WRITE])
#define UP_lseek  (*(long (*)(int,long,int))OPENSSL_UplinkTable[APPLINK_LSEEK])
#define UP_close  (*(int (*)(int))OPENSSL_UplinkTable[APPLINK_CLOSE])
