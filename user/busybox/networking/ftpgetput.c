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
 * ftpget
 *
 * Mini implementation of FTP to retrieve a remote file.
 *
 * Copyright (C) 2002 Jeff Angielski, The PTR Group <jeff@theptrgroup.com>
 * Copyright (C) 2002 Glenn McGrath
 *
 * Based on wget.c by Chip Rosenthal Covad Communications
 * <chip@laserlink.net>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

#include "libbb.h"

struct globals {
	const char *user;
	const char *password;
	struct len_and_sockaddr *lsa;
	FILE *control_stream;
	int verbose_flag;
	int do_continue;
	char buf[1]; /* actually [BUFSZ] */
};
#define G (*(struct globals*)&bb_common_bufsiz1)
enum { BUFSZ = COMMON_BUFSIZE - offsetof(struct globals, buf) };
struct BUG_G_too_big {
	char BUG_G_too_big[sizeof(G) <= COMMON_BUFSIZE ? 1 : -1];
};
#define user           (G.user          )
#define password       (G.password      )
#define lsa            (G.lsa           )
#define control_stream (G.control_stream)
#define verbose_flag   (G.verbose_flag  )
#define do_continue    (G.do_continue   )
#define buf            (G.buf           )
#define INIT_G() do { } while (0)


static void ftp_die(const char *msg) NORETURN;
static void ftp_die(const char *msg)
{
	char *cp = buf; /* buf holds peer's response */

	/* Guard against garbage from remote server */
	while (*cp >= ' ' && *cp < '\x7f')
		cp++;
	*cp = '\0';
	bb_error_msg_and_die("unexpected server response%s%s: %s",
			(msg ? " to " : ""), (msg ? msg : ""), buf);
}

static int ftpcmd(const char *s1, const char *s2)
{
	unsigned n;

	if (verbose_flag) {
		bb_error_msg("cmd %s %s", s1, s2);
	}

	if (s1) {
		fprintf(control_stream, (s2 ? "%s %s\r\n" : "%s %s\r\n"+3),
						s1, s2);
		fflush(control_stream);
	}

	do {
		strcpy(buf, "EOF");
		if (fgets(buf, BUFSZ - 2, control_stream) == NULL) {
			ftp_die(NULL);
		}
	} while (!isdigit(buf[0]) || buf[3] != ' ');

	buf[3] = '\0';
	n = xatou(buf);
	buf[3] = ' ';
	return n;
}

static void ftp_login(void)
{
	/* Connect to the command socket */
	control_stream = fdopen(xconnect_stream(lsa), "r+");
	if (control_stream == NULL) {
		/* fdopen failed - extremely unlikely */
		bb_perror_nomsg_and_die();
	}

	if (ftpcmd(NULL, NULL) != 220) {
		ftp_die(NULL);
	}

	/*  Login to the server */
	switch (ftpcmd("USER", user)) {
	case 230:
		break;
	case 331:
		if (ftpcmd("PASS", password) != 230) {
			ftp_die("PASS");
		}
		break;
	default:
		ftp_die("USER");
	}

	ftpcmd("TYPE I", NULL);
}

static int xconnect_ftpdata(void)
{
	char *buf_ptr;
	unsigned port_num;

/*
TODO: PASV command will not work for IPv6. RFC2428 describes
IPv6-capable "extended PASV" - EPSV.

"EPSV [protocol]" asks server to bind to and listen on a data port
in specified protocol. Protocol is 1 for IPv4, 2 for IPv6.
If not specified, defaults to "same as used for control connection".
If server understood you, it should answer "229 <some text>(|||port|)"
where "|" are literal pipe chars and "port" is ASCII decimal port#.

There is also an IPv6-capable replacement for PORT (EPRT),
but we don't need that.

NB: PASV may still work for some servers even over IPv6.
For example, vsftp happily answers
"227 Entering Passive Mode (0,0,0,0,n,n)" and proceeds as usual.

TODO2: need to stop ignoring IP address in PASV response.
*/

	if (ftpcmd("PASV", NULL) != 227) {
		ftp_die("PASV");
	}

	/* Response is "NNN garbageN1,N2,N3,N4,P1,P2[)garbage]
	 * Server's IP is N1.N2.N3.N4 (we ignore it)
	 * Server's port for data connection is P1*256+P2 */
	buf_ptr = strrchr(buf, ')');
	if (buf_ptr) *buf_ptr = '\0';

	buf_ptr = strrchr(buf, ',');
	*buf_ptr = '\0';
	port_num = xatoul_range(buf_ptr + 1, 0, 255);

	buf_ptr = strrchr(buf, ',');
	*buf_ptr = '\0';
	port_num += xatoul_range(buf_ptr + 1, 0, 255) * 256;

	set_nport(lsa, htons(port_num));
	return xconnect_stream(lsa);
}

static int pump_data_and_QUIT(int from, int to)
{
	/* copy the file */
	if (bb_copyfd_eof(from, to) == -1) {
		/* error msg is already printed by bb_copyfd_eof */
		return EXIT_FAILURE;
	}

	/* close data connection */
	close(from); /* don't know which one is that, so we close both */
	close(to);

	/* does server confirm that transfer is finished? */
	if (ftpcmd(NULL, NULL) != 226) {
		ftp_die(NULL);
	}
	ftpcmd("QUIT", NULL);

	return EXIT_SUCCESS;
}

#if !ENABLE_FTPGET
int ftp_receive(const char *local_path, char *server_path);
#else
static
int ftp_receive(const char *local_path, char *server_path)
{
	int fd_data;
	int fd_local = -1;
	off_t beg_range = 0;

	/* connect to the data socket */
	fd_data = xconnect_ftpdata();

	if (ftpcmd("SIZE", server_path) != 213) {
		do_continue = 0;
	}

	if (LONE_DASH(local_path)) {
		fd_local = STDOUT_FILENO;
		do_continue = 0;
	}

	if (do_continue) {
		struct stat sbuf;
		/* lstat would be wrong here! */
		if (stat(local_path, &sbuf) < 0) {
			bb_perror_msg_and_die("stat");
		}
		if (sbuf.st_size > 0) {
			beg_range = sbuf.st_size;
		} else {
			do_continue = 0;
		}
	}

	if (do_continue) {
		sprintf(buf, "REST %"OFF_FMT"d", beg_range);
		if (ftpcmd(buf, NULL) != 350) {
			do_continue = 0;
		}
	}

	if (ftpcmd("RETR", server_path) > 150) {
		ftp_die("RETR");
	}

	/* create local file _after_ we know that remote file exists */
	if (fd_local == -1) {
		fd_local = xopen(local_path,
			do_continue ? (O_APPEND | O_WRONLY)
			            : (O_CREAT | O_TRUNC | O_WRONLY)
		);
	}

	return pump_data_and_QUIT(fd_data, fd_local);
}
#endif

#if !ENABLE_FTPPUT
int ftp_send(const char *server_path, char *local_path);
#else
static
int ftp_send(const char *server_path, char *local_path)
{
	int fd_data;
	int fd_local;
	int response;

	/* connect to the data socket */
	fd_data = xconnect_ftpdata();

	/* get the local file */
	fd_local = STDIN_FILENO;
	if (NOT_LONE_DASH(local_path))
		fd_local = xopen(local_path, O_RDONLY);

	response = ftpcmd("STOR", server_path);
	switch (response) {
	case 125:
	case 150:
		break;
	default:
		ftp_die("STOR");
	}

	return pump_data_and_QUIT(fd_local, fd_data);
}
#endif

#if ENABLE_FEATURE_FTPGETPUT_LONG_OPTIONS
static const char ftpgetput_longopts[] ALIGN1 =
	"continue\0" Required_argument "c"
	"verbose\0"  No_argument       "v"
	"username\0" Required_argument "u"
	"password\0" Required_argument "p"
	"port\0"     Required_argument "P"
	;
#endif

int ftpgetput_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int ftpgetput_main(int argc UNUSED_PARAM, char **argv)
{
	unsigned opt;
	const char *port = "ftp";
	/* socket to ftp server */

#if ENABLE_FTPPUT && !ENABLE_FTPGET
# define ftp_action ftp_send
#elif ENABLE_FTPGET && !ENABLE_FTPPUT
# define ftp_action ftp_receive
#else
	int (*ftp_action)(const char *, char *) = ftp_send;

	/* Check to see if the command is ftpget or ftput */
	if (applet_name[3] == 'g') {
		ftp_action = ftp_receive;
	}
#endif

	INIT_G();
	/* Set default values */
	user = "anonymous";
	password = "busybox@";

	/*
	 * Decipher the command line
	 */
#if ENABLE_FEATURE_FTPGETPUT_LONG_OPTIONS
	applet_long_options = ftpgetput_longopts;
#endif
	opt_complementary = "=3:vv:cc"; /* must have 3 params; -v and -c count */
	opt = getopt32(argv, "cvu:p:P:", &user, &password, &port,
					&verbose_flag, &do_continue);
	argv += optind;

	/* We want to do exactly _one_ DNS lookup, since some
	 * sites (i.e. ftp.us.debian.org) use round-robin DNS
	 * and we want to connect to only one IP... */
	lsa = xhost2sockaddr(argv[0], bb_lookup_port(port, "tcp", 21));
	if (verbose_flag) {
		printf("Connecting to %s (%s)\n", argv[0],
			xmalloc_sockaddr2dotted(&lsa->u.sa));
	}

	ftp_login();
	return ftp_action(argv[1], argv[2]);
}
