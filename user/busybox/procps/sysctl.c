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
 * Sysctl 1.01 - A utility to read and manipulate the sysctl parameters
 *
 * Copyright 1999 George Staikos
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 *
 * Changelog:
 *	v1.01:
 *		- added -p <preload> to preload values from a file
 *	v1.01.1
 *		- busybox applet aware by <solar@gentoo.org>
 *
 */

#include "libbb.h"

static int sysctl_read_setting(const char *setting);
static int sysctl_write_setting(const char *setting);
static int sysctl_display_all(const char *path);
static int sysctl_preload_file_and_exit(const char *filename);

static const char ETC_SYSCTL_CONF[] ALIGN1 = "/etc/sysctl.conf";
static const char PROC_SYS[] ALIGN1 = "/proc/sys/";
enum { strlen_PROC_SYS = sizeof(PROC_SYS) - 1 };

/* error messages */
static const char ERR_MALFORMED_SETTING[] ALIGN1 =
	"error: malformed setting '%s'";
static const char ERR_NO_EQUALS[] ALIGN1 =
	"error: '%s' must be of the form name=value";
static const char ERR_INVALID_KEY[] ALIGN1 =
	"error: '%s' is an unknown key";
static const char ERR_UNKNOWN_WRITING[] ALIGN1 =
	"error setting key '%s'";
static const char ERR_UNKNOWN_READING[] ALIGN1 =
	"error reading key '%s'";
static const char ERR_PERMISSION_DENIED[] ALIGN1 =
	"error: permission denied on key '%s'";
static const char WARN_BAD_LINE[] ALIGN1 =
	"warning: %s(%d): invalid syntax, continuing";


static void dwrite_str(int fd, const char *buf)
{
	write(fd, buf, strlen(buf));
}

enum {
	FLAG_SHOW_KEYS       = 1 << 0,
	FLAG_SHOW_KEY_ERRORS = 1 << 1,
	FLAG_TABLE_FORMAT    = 1 << 2, /* not implemented */
	FLAG_SHOW_ALL        = 1 << 3,
	FLAG_PRELOAD_FILE    = 1 << 4,
	FLAG_WRITE           = 1 << 5,
};

int sysctl_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int sysctl_main(int argc UNUSED_PARAM, char **argv)
{
	int retval;
	int opt;

	opt = getopt32(argv, "+neAapw"); /* '+' - stop on first non-option */
	argv += optind;
	opt ^= (FLAG_SHOW_KEYS | FLAG_SHOW_KEY_ERRORS);
	option_mask32 ^= (FLAG_SHOW_KEYS | FLAG_SHOW_KEY_ERRORS);

	if (opt & (FLAG_TABLE_FORMAT | FLAG_SHOW_ALL))
		return sysctl_display_all(PROC_SYS);
	if (opt & FLAG_PRELOAD_FILE)
		return sysctl_preload_file_and_exit(*argv ? *argv : ETC_SYSCTL_CONF);

	retval = 0;
	while (*argv) {
		if (opt & FLAG_WRITE)
			retval |= sysctl_write_setting(*argv);
		else
			retval |= sysctl_read_setting(*argv);
		argv++;
	}

	return retval;
} /* end sysctl_main() */

/*
 * preload the sysctl's from a conf file
 * - we parse the file and then reform it (strip out whitespace)
 */

static int sysctl_preload_file_and_exit(const char *filename)
{
	char *token[2];
	parser_t *parser;

	parser = config_open(filename);
	while (config_read(parser, token, 2, 2, "# \t=", PARSE_NORMAL)) { // TODO: ';' is comment char too
//		if (!token[1]) {
//			bb_error_msg(WARN_BAD_LINE, filename, parser->lineno);
//		} else {
		{
#if 0
			char *s = xasprintf("%s=%s", token[0], token[1]);
			sysctl_write_setting(s);
			free(s);
#else // PLAY_WITH_FIRE for -4 bytes?
			sprintf(parser->line, "%s=%s", token[0], token[1]); // must have room by definition
			sysctl_write_setting(parser->line);
#endif
		}
	}
	if (ENABLE_FEATURE_CLEAN_UP)
		config_close(parser);
	return 0;
} /* end sysctl_preload_file_and_exit() */

/*
 *     Write a single sysctl setting
 */
static int sysctl_write_setting(const char *setting)
{
	int retval;
	const char *name;
	const char *value;
	const char *equals;
	char *tmpname, *outname, *cptr;
	int fd;

	name = setting;
	equals = strchr(setting, '=');
	if (!equals) {
		bb_error_msg(ERR_NO_EQUALS, setting);
		return EXIT_FAILURE;
	}

	value = equals + 1;	/* point to the value in name=value */
	if (name == equals || !*value) {
		bb_error_msg(ERR_MALFORMED_SETTING, setting);
		return EXIT_FAILURE;
	}

	tmpname = xasprintf("%s%.*s", PROC_SYS, (int)(equals - name), name);
	outname = xstrdup(tmpname + strlen_PROC_SYS);

	while ((cptr = strchr(tmpname, '.')) != NULL)
		*cptr = '/';

	while ((cptr = strchr(outname, '/')) != NULL)
		*cptr = '.';

	fd = open(tmpname, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (fd < 0) {
		switch (errno) {
		case ENOENT:
			if (option_mask32 & FLAG_SHOW_KEY_ERRORS)
				bb_error_msg(ERR_INVALID_KEY, outname);
			break;
		case EACCES:
			bb_perror_msg(ERR_PERMISSION_DENIED, outname);
			break;
		default:
			bb_perror_msg(ERR_UNKNOWN_WRITING, outname);
			break;
		}
		retval = EXIT_FAILURE;
	} else {
		dwrite_str(fd, value);
		close(fd);
		if (option_mask32 & FLAG_SHOW_KEYS) {
			printf("%s = ", outname);
		}
		puts(value);
		retval = EXIT_SUCCESS;
	}

	free(tmpname);
	free(outname);
	return retval;
} /* end sysctl_write_setting() */

/*
 *     Read a sysctl setting
 */
static int sysctl_read_setting(const char *name)
{
	int retval;
	char *tmpname, *outname, *cptr;
	char inbuf[1025];
	FILE *fp;

	if (!*name) {
		if (option_mask32 & FLAG_SHOW_KEY_ERRORS)
			bb_error_msg(ERR_INVALID_KEY, name);
		return -1;
	}

	tmpname = concat_path_file(PROC_SYS, name);
	outname = xstrdup(tmpname + strlen_PROC_SYS);

	while ((cptr = strchr(tmpname, '.')) != NULL)
		*cptr = '/';
	while ((cptr = strchr(outname, '/')) != NULL)
		*cptr = '.';

	fp = fopen_for_read(tmpname);
	if (fp == NULL) {
		switch (errno) {
		case ENOENT:
			if (option_mask32 & FLAG_SHOW_KEY_ERRORS)
				bb_error_msg(ERR_INVALID_KEY, outname);
			break;
		case EACCES:
			bb_error_msg(ERR_PERMISSION_DENIED, outname);
			break;
		default:
			bb_perror_msg(ERR_UNKNOWN_READING, outname);
			break;
		}
		retval = EXIT_FAILURE;
	} else {
		while (fgets(inbuf, sizeof(inbuf) - 1, fp)) {
			if (option_mask32 & FLAG_SHOW_KEYS) {
				printf("%s = ", outname);
			}
			fputs(inbuf, stdout);
		}
		fclose(fp);
		retval = EXIT_SUCCESS;
	}

	free(tmpname);
	free(outname);
	return retval;
} /* end sysctl_read_setting() */

/*
 *     Display all the sysctl settings
 */
static int sysctl_display_all(const char *path)
{
	int retval = 0;
	DIR *dp;
	struct dirent *de;
	char *tmpdir;
	struct stat ts;

	dp = opendir(path);
	if (!dp) {
		return EXIT_FAILURE;
	}
	while ((de = readdir(dp)) != NULL) {
		tmpdir = concat_subpath_file(path, de->d_name);
		if (tmpdir == NULL)
			continue; /* . or .. */
		if (stat(tmpdir, &ts) != 0) {
			bb_perror_msg(tmpdir);
		} else if (S_ISDIR(ts.st_mode)) {
			retval |= sysctl_display_all(tmpdir);
		} else {
			retval |= sysctl_read_setting(tmpdir + strlen_PROC_SYS);
		}
		free(tmpdir);
	} /* end while */
	closedir(dp);

	return retval;
} /* end sysctl_display_all() */
