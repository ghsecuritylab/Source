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
 * Mini sulogin implementation for busybox
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

#include "libbb.h"
#include <syslog.h>

//static void catchalarm(int UNUSED_PARAM junk)
//{
//	exit(EXIT_FAILURE);
//}


int sulogin_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int sulogin_main(int argc UNUSED_PARAM, char **argv)
{
	char *cp;
	int timeout = 0;
	struct passwd *pwd;
	const char *shell;
#if ENABLE_FEATURE_SHADOWPASSWDS
	/* Using _r function to avoid pulling in static buffers */
	char buffer[256];
	struct spwd spw;
#endif

	logmode = LOGMODE_BOTH;
	openlog(applet_name, 0, LOG_AUTH);

	opt_complementary = "t+"; /* -t N */
	getopt32(argv, "t:", &timeout);

	if (argv[optind]) {
		close(0);
		close(1);
		dup(xopen(argv[optind], O_RDWR));
		close(2);
		dup(0);
	}

	/* Malicious use like "sulogin /dev/sda"? */
	if (!isatty(0) || !isatty(1) || !isatty(2)) {
		logmode = LOGMODE_SYSLOG;
		bb_error_msg_and_die("not a tty");
	}

	/* Clear dangerous stuff, set PATH */
	sanitize_env_if_suid();

// bb_askpass() already handles this
//	signal(SIGALRM, catchalarm);

	pwd = getpwuid(0);
	if (!pwd) {
		goto auth_error;
	}

#if ENABLE_FEATURE_SHADOWPASSWDS
	{
		/* getspnam_r may return 0 yet set result to NULL.
		 * At least glibc 2.4 does this. Be extra paranoid here. */
		struct spwd *result = NULL;
		int r = getspnam_r(pwd->pw_name, &spw, buffer, sizeof(buffer), &result);
		if (r || !result) {
			goto auth_error;
		}
		pwd->pw_passwd = result->sp_pwdp;
	}
#endif

	while (1) {
		char *encrypted;
		int r;

		/* cp points to a static buffer that is zeroed every time */
		cp = bb_askpass(timeout,
				"Give root password for system maintenance\n"
				"(or type Control-D for normal startup):");

		if (!cp || !*cp) {
			bb_info_msg("Normal startup");
			return 0;
		}
		encrypted = pw_encrypt(cp, pwd->pw_passwd, 1);
		r = strcmp(encrypted, pwd->pw_passwd);
		free(encrypted);
		if (r == 0) {
			break;
		}
		bb_do_delay(FAIL_DELAY);
		bb_error_msg("login incorrect");
	}
	memset(cp, 0, strlen(cp));
//	signal(SIGALRM, SIG_DFL);

	bb_info_msg("System Maintenance Mode");

	USE_SELINUX(renew_current_security_context());

	shell = getenv("SUSHELL");
	if (!shell)
		shell = getenv("sushell");
	if (!shell) {
		shell = "/bin/sh";
		if (pwd->pw_shell[0])
			shell = pwd->pw_shell;
	}
	/* Exec login shell with no additional parameters. Never returns. */
	run_shell(shell, 1, NULL, NULL);

 auth_error:
	bb_error_msg_and_die("no password entry for root");
}
