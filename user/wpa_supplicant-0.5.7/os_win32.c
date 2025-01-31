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
 * wpa_supplicant/hostapd / OS specific functions for Win32 systems
 * Copyright (c) 2005-2006, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include "includes.h"
#include <winsock2.h>
#include <wincrypt.h>

#include "os.h"

void os_sleep(os_time_t sec, os_time_t usec)
{
	if (sec)
		Sleep(sec * 1000);
	if (usec)
		Sleep(usec / 1000);
}


int os_get_time(struct os_time *t)
{
#define EPOCHFILETIME (116444736000000000ULL)
	FILETIME ft;
	LARGE_INTEGER li;
	ULONGLONG tt;

#ifdef _WIN32_WCE
	SYSTEMTIME st;

	GetSystemTime(&st);
	SystemTimeToFileTime(&st, &ft);
#else /* _WIN32_WCE */
	GetSystemTimeAsFileTime(&ft);
#endif /* _WIN32_WCE */
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;
	tt = (li.QuadPart - EPOCHFILETIME) / 10;
	t->sec = (os_time_t) (tt / 1000000);
	t->usec = (os_time_t) (tt % 1000000);

	return 0;
}


int os_mktime(int year, int month, int day, int hour, int min, int sec,
	      os_time_t *t)
{
	struct tm tm;

	if (year < 1970 || month < 1 || month > 12 || day < 1 || day > 31 ||
	    hour < 0 || hour > 23 || min < 0 || min > 59 || sec < 0 ||
	    sec > 60)
		return -1;

	memset(&tm, 0, sizeof(tm));
	tm.tm_year = year - 1900;
	tm.tm_mon = month - 1;
	tm.tm_mday = day;
	tm.tm_hour = hour;
	tm.tm_min = min;
	tm.tm_sec = sec;

	*t = (os_time_t) mktime(&tm);
	return 0;
}


int os_daemonize(const char *pid_file)
{
	/* TODO */
	return -1;
}


void os_daemonize_terminate(const char *pid_file)
{
}


int os_get_random(unsigned char *buf, size_t len)
{
	HCRYPTPROV prov;
	BOOL ret;

	if (!CryptAcquireContext(&prov, NULL, NULL, PROV_RSA_FULL,
				 CRYPT_VERIFYCONTEXT))
		return -1;

	ret = CryptGenRandom(prov, len, buf);
	CryptReleaseContext(prov, 0);

	return ret ? 0 : -1;
}


unsigned long os_random(void)
{
	return rand();
}


char * os_rel2abs_path(const char *rel_path)
{
	return _strdup(rel_path);
}


int os_program_init(void)
{
#ifdef CONFIG_NATIVE_WINDOWS
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 0), &wsaData)) {
		printf("Could not find a usable WinSock.dll\n");
		return -1;
	}
#endif /* CONFIG_NATIVE_WINDOWS */
	return 0;
}


void os_program_deinit(void)
{
#ifdef CONFIG_NATIVE_WINDOWS
	WSACleanup();
#endif /* CONFIG_NATIVE_WINDOWS */
}


int os_setenv(const char *name, const char *value, int overwrite)
{
	return -1;
}


int os_unsetenv(const char *name)
{
	return -1;
}


char * os_readfile(const char *name, size_t *len)
{
	FILE *f;
	char *buf;

	f = fopen(name, "rb");
	if (f == NULL)
		return NULL;

	fseek(f, 0, SEEK_END);
	*len = ftell(f);
	fseek(f, 0, SEEK_SET);

	buf = malloc(*len);
	if (buf == NULL) {
		fclose(f);
		return NULL;
	}

	fread(buf, 1, *len, f);
	fclose(f);

	return buf;
}


void * os_zalloc(size_t size)
{
	return calloc(1, size);
}
