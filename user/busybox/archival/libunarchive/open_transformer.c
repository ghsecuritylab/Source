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
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

#include "libbb.h"
#include "unarchive.h"

/* transformer(), more than meets the eye */
/*
 * On MMU machine, the transform_prog is removed by macro magic
 * in include/unarchive.h. On NOMMU, transformer is removed.
 */
void FAST_FUNC open_transformer(int fd,
	USE_DESKTOP(long long) int FAST_FUNC (*transformer)(int src_fd, int dst_fd),
	const char *transform_prog)
{
	struct fd_pair fd_pipe;
	int pid;

	xpiped_pair(fd_pipe);

#if BB_MMU
	pid = fork();
	if (pid == -1)
		bb_perror_msg_and_die("vfork" + 1);
#else
	pid = vfork();
	if (pid == -1)
		bb_perror_msg_and_die("vfork");
#endif

	if (pid == 0) {
		/* child process */
		close(fd_pipe.rd); /* we don't want to read from the parent */
		// FIXME: error check?
#if BB_MMU
		transformer(fd, fd_pipe.wr);
		if (ENABLE_FEATURE_CLEAN_UP) {
			close(fd_pipe.wr); /* send EOF */
			close(fd);
		}
		/* must be _exit! bug was actually seen here */
		_exit(EXIT_SUCCESS);
#else
		{
			char *argv[4];
			xmove_fd(fd, 0);
			xmove_fd(fd_pipe.wr, 1);
			argv[0] = (char*)transform_prog;
			argv[1] = (char*)"-cf";
			argv[2] = (char*)"-";
			argv[3] = NULL;
			BB_EXECVP(transform_prog, argv);
			bb_perror_msg_and_die("can't exec %s", transform_prog);
		}
#endif
		/* notreached */
	}

	/* parent process */
	close(fd_pipe.wr); /* don't want to write to the child */
	xmove_fd(fd_pipe.rd, fd);
}
