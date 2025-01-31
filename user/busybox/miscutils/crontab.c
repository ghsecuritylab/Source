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
 * CRONTAB
 *
 * usually setuid root, -c option only works if getuid() == geteuid()
 *
 * Copyright 1994 Matthew Dillon (dillon@apollo.west.oic.com)
 * Vladimir Oleynik <dzo@simtreas.ru> (C) 2002
 *
 * Licensed under the GPL v2 or later, see the file LICENSE in this tarball.
 */

#include "libbb.h"

#ifndef CRONTABS
#define CRONTABS        "/var/spool/cron/crontabs"
#endif
#ifndef CRONUPDATE
#define CRONUPDATE      "cron.update"
#endif

static void change_user(const struct passwd *pas)
{
	setenv("USER", pas->pw_name, 1);
	setenv("HOME", pas->pw_dir, 1);
	setenv("SHELL", DEFAULT_SHELL, 1);

	/* initgroups, setgid, setuid */
	change_identity(pas);

	if (chdir(pas->pw_dir) < 0) {
		bb_perror_msg("chdir(%s) by %s failed",
				pas->pw_dir, pas->pw_name);
		xchdir("/tmp");
	}
}

static void edit_file(const struct passwd *pas, const char *file)
{
	const char *ptr;
	int pid = vfork();

	if (pid < 0) /* failure */
		bb_perror_msg_and_die("vfork");
	if (pid) { /* parent */
		wait4pid(pid);
		return;
	}

	/* CHILD - change user and run editor */
	change_user(pas);
	ptr = getenv("VISUAL");
	if (!ptr) {
		ptr = getenv("EDITOR");
		if (!ptr)
			ptr = "vi";
	}

	BB_EXECLP(ptr, ptr, file, NULL);
	bb_perror_msg_and_die("exec %s", ptr);
}

static int open_as_user(const struct passwd *pas, const char *file)
{
	pid_t pid;
	char c;

	pid = vfork();
	if (pid < 0) /* ERROR */
		bb_perror_msg_and_die("vfork");
	if (pid) { /* PARENT */
		if (wait4pid(pid) == 0) {
			/* exitcode 0: child says it can read */
			return open(file, O_RDONLY);
		}
		return -1;
	}

	/* CHILD */
	/* initgroups, setgid, setuid */
	change_identity(pas);
	/* We just try to read one byte. If it works, file is readable
	 * under this user. We signal that by exiting with 0. */
	_exit(safe_read(xopen(file, O_RDONLY), &c, 1) < 0);
}

int crontab_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int crontab_main(int argc UNUSED_PARAM, char **argv)
{
	const struct passwd *pas;
	const char *crontab_dir = CRONTABS;
	char *tmp_fname;
	char *new_fname;
	char *user_name;  /* -u USER */
	int fd;
	int src_fd;
	int opt_ler;

	/* file [opts]     Replace crontab from file
	 * - [opts]        Replace crontab from stdin
	 * -u user         User
	 * -c dir          Crontab directory
	 * -l              List crontab for user
	 * -e              Edit crontab for user
	 * -r              Delete crontab for user
	 * bbox also supports -d == -r, but most other crontab
	 * implementations do not. Deprecated.
	 */
	enum {
		OPT_u = (1 << 0),
		OPT_c = (1 << 1),
		OPT_l = (1 << 2),
		OPT_e = (1 << 3),
		OPT_r = (1 << 4),
		OPT_ler = OPT_l + OPT_e + OPT_r,
	};

	opt_complementary = "?1:dr"; /* max one argument; -d implies -r */
	opt_ler = getopt32(argv, "u:c:lerd", &user_name, &crontab_dir);
	argv += optind;

	if (sanitize_env_if_suid()) { /* Clears dangerous stuff, sets PATH */
		/* run by non-root? */
		if (opt_ler & (OPT_u|OPT_c))
			bb_error_msg_and_die("only root can use -c or -u");
	}

	if (opt_ler & OPT_u) {
		pas = getpwnam(user_name);
		if (!pas)
			bb_error_msg_and_die("user %s is not known", user_name);
	} else {
/* XXX: xgetpwuid */
		uid_t my_uid = getuid();
		pas = getpwuid(my_uid);
		if (!pas)
			bb_perror_msg_and_die("unknown uid %d", (int)my_uid);
	}

#define user_name DONT_USE_ME_BEYOND_THIS_POINT

	/* From now on, keep only -l, -e, -r bits */
	opt_ler &= OPT_ler;
	if ((opt_ler - 1) & opt_ler) /* more than one bit set? */
		bb_show_usage();

	/* Read replacement file under user's UID/GID/group vector */
	src_fd = STDIN_FILENO;
	if (!opt_ler) { /* Replace? */
		if (!argv[0])
			bb_show_usage();
		if (NOT_LONE_DASH(argv[0])) {
			src_fd = open_as_user(pas, argv[0]);
			if (src_fd < 0)
				bb_error_msg_and_die("user %s cannot read %s",
						pas->pw_name, argv[0]);
		}
	}

	/* cd to our crontab directory */
	xchdir(crontab_dir);

	tmp_fname = NULL;

	/* Handle requested operation */
	switch (opt_ler) {

	default: /* case OPT_r: Delete */
		unlink(pas->pw_name);
		break;

	case OPT_l: /* List */
		{
			char *args[2] = { pas->pw_name, NULL };
			return bb_cat(args);
			/* list exits,
			 * the rest go play with cron update file */
		}

	case OPT_e: /* Edit */
		tmp_fname = xasprintf("%s.%u", crontab_dir, (unsigned)getpid());
		/* No O_EXCL: we don't want to be stuck if earlier crontabs
		 * were killed, leaving stale temp file behind */
		src_fd = xopen3(tmp_fname, O_RDWR|O_CREAT|O_TRUNC, 0600);
		fchown(src_fd, pas->pw_uid, pas->pw_gid);
		fd = open(pas->pw_name, O_RDONLY);
		if (fd >= 0) {
			bb_copyfd_eof(fd, src_fd);
			close(fd);
			xlseek(src_fd, 0, SEEK_SET);
		}
		close_on_exec_on(src_fd); /* don't want editor to see this fd */
		edit_file(pas, tmp_fname);
		/* fall through */

	case 0: /* Replace (no -l, -e, or -r were given) */
		new_fname = xasprintf("%s.new", pas->pw_name);
		fd = open(new_fname, O_WRONLY|O_CREAT|O_TRUNC|O_APPEND, 0600);
		if (fd >= 0) {
			bb_copyfd_eof(src_fd, fd);
			close(fd);
			xrename(new_fname, pas->pw_name);
		} else {
			bb_error_msg("cannot create %s/%s",
					crontab_dir, new_fname);
		}
		if (tmp_fname)
			unlink(tmp_fname);
		/*free(tmp_fname);*/
		/*free(new_fname);*/

	} /* switch */

	/* Bump notification file.  Handle window where crond picks file up
	 * before we can write our entry out.
	 */
	while ((fd = open(CRONUPDATE, O_WRONLY|O_CREAT|O_APPEND, 0600)) >= 0) {
		struct stat st;

		fdprintf(fd, "%s\n", pas->pw_name);
		if (fstat(fd, &st) != 0 || st.st_nlink != 0) {
			/*close(fd);*/
			break;
		}
		/* st.st_nlink == 0:
		 * file was deleted, maybe crond missed our notification */
		close(fd);
		/* loop */
	}
	if (fd < 0) {
		bb_error_msg("cannot append to %s/%s",
				crontab_dir, CRONUPDATE);
	}
	return 0;
}
