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
Copyright (c) 2001-2006, Gerrit Pape
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
   3. The name of the author may not be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* Busyboxed by Denys Vlasenko <vda.linux@googlemail.com> */
/* TODO: depends on runit_lib.c - review and reduce/eliminate */

#include <sys/poll.h>
#include <sys/file.h>
#include "libbb.h"
#include "runit_lib.h"

#if ENABLE_MONOTONIC_SYSCALL
#include <sys/syscall.h>

/* libc has incredibly messy way of doing this,
 * typically requiring -lrt. We just skip all this mess */
static void gettimeofday_ns(struct timespec *ts)
{
	syscall(__NR_clock_gettime, CLOCK_REALTIME, ts);
}
#else
static void gettimeofday_ns(struct timespec *ts)
{
	if (sizeof(struct timeval) == sizeof(struct timespec)
	 && sizeof(((struct timeval*)ts)->tv_usec) == sizeof(ts->tv_nsec)
	) {
		/* Cheat */
		gettimeofday((void*)ts, NULL);
		ts->tv_nsec *= 1000;
	} else {
		extern void BUG_need_to_implement_gettimeofday_ns(void);
		BUG_need_to_implement_gettimeofday_ns();
	}
}
#endif

/* Compare possibly overflowing unsigned counters */
#define LESS(a,b) ((int)((unsigned)(b) - (unsigned)(a)) > 0)

/* state */
#define S_DOWN 0
#define S_RUN 1
#define S_FINISH 2
/* ctrl */
#define C_NOOP 0
#define C_TERM 1
#define C_PAUSE 2
/* want */
#define W_UP 0
#define W_DOWN 1
#define W_EXIT 2

struct svdir {
	int pid;
	smallint state;
	smallint ctrl;
	smallint want;
	smallint islog;
	struct timespec start;
	int fdlock;
	int fdcontrol;
	int fdcontrolwrite;
};

struct globals {
	smallint haslog;
	smallint sigterm;
	smallint pidchanged;
	struct fd_pair selfpipe;
	struct fd_pair logpipe;
	char *dir;
	struct svdir svd[2];
};
#define G (*(struct globals*)&bb_common_bufsiz1)
#define haslog       (G.haslog      )
#define sigterm      (G.sigterm     )
#define pidchanged   (G.pidchanged  )
#define selfpipe     (G.selfpipe    )
#define logpipe      (G.logpipe     )
#define dir          (G.dir         )
#define svd          (G.svd         )
#define INIT_G() do { \
	pidchanged = 1; \
} while (0)

static void fatal2_cannot(const char *m1, const char *m2)
{
	bb_perror_msg_and_die("%s: fatal: cannot %s%s", dir, m1, m2);
	/* was exiting 111 */
}
static void fatal_cannot(const char *m)
{
	fatal2_cannot(m, "");
	/* was exiting 111 */
}
static void fatal2x_cannot(const char *m1, const char *m2)
{
	bb_error_msg_and_die("%s: fatal: cannot %s%s", dir, m1, m2);
	/* was exiting 111 */
}
static void warn_cannot(const char *m)
{
	bb_perror_msg("%s: warning: cannot %s", dir, m);
}

static void s_child(int sig_no UNUSED_PARAM)
{
	write(selfpipe.wr, "", 1);
}

static void s_term(int sig_no UNUSED_PARAM)
{
	sigterm = 1;
	write(selfpipe.wr, "", 1); /* XXX */
}

static char *add_str(char *p, const char *to_add)
{
	while ((*p = *to_add) != '\0') {
		p++;
		to_add++;
	}
	return p;
}

static int open_trunc_or_warn(const char *name)
{
	int fd = open_trunc(name);
	if (fd < 0)
		bb_perror_msg("%s: warning: cannot open %s",
				dir, name);
	return fd;
}

static void update_status(struct svdir *s)
{
	ssize_t sz;
	int fd;
	svstatus_t status;

	/* pid */
	if (pidchanged) {
		fd = open_trunc_or_warn("supervise/pid.new");
		if (fd < 0)
			return;
		if (s->pid) {
			char spid[sizeof(int)*3 + 2];
			int size = sprintf(spid, "%u\n", (unsigned)s->pid);
			write(fd, spid, size);
		}
		close(fd);
		if (rename_or_warn("supervise/pid.new",
		    s->islog ? "log/supervise/pid" : "log/supervise/pid"+4))
			return;
		pidchanged = 0;
	}

	/* stat */
	fd = open_trunc_or_warn("supervise/stat.new");
	if (fd < -1)
		return;

	{
		char stat_buf[sizeof("finish, paused, got TERM, want down\n")];
		char *p = stat_buf;
		switch (s->state) {
		case S_DOWN:
			p = add_str(p, "down");
			break;
		case S_RUN:
			p = add_str(p, "run");
			break;
		case S_FINISH:
			p = add_str(p, "finish");
			break;
		}
		if (s->ctrl & C_PAUSE) p = add_str(p, ", paused");
		if (s->ctrl & C_TERM) p = add_str(p, ", got TERM");
		if (s->state != S_DOWN)
			switch (s->want) {
			case W_DOWN:
				p = add_str(p, ", want down");
				break;
			case W_EXIT:
				p = add_str(p, ", want exit");
				break;
			}
		*p++ = '\n';
		write(fd, stat_buf, p - stat_buf);
		close(fd);
	}

	rename_or_warn("supervise/stat.new",
		s->islog ? "log/supervise/stat" : "log/supervise/stat"+4);

	/* supervise compatibility */
	memset(&status, 0, sizeof(status));
	status.time_be64 = SWAP_BE64(s->start.tv_sec + 0x400000000000000aULL);
	status.time_nsec_be32 = SWAP_BE32(s->start.tv_nsec);
	status.pid_le32 = SWAP_LE32(s->pid);
	if (s->ctrl & C_PAUSE)
		status.paused = 1;
	if (s->want == W_UP)
		status.want = 'u';
	else
		status.want = 'd';
	if (s->ctrl & C_TERM)
		status.got_term = 1;
	status.run_or_finish = s->state;
	fd = open_trunc_or_warn("supervise/status.new");
	if (fd < 0)
		return;
	sz = write(fd, &status, sizeof(status));
	close(fd);
	if (sz != sizeof(status)) {
		warn_cannot("write supervise/status.new");
		unlink("supervise/status.new");
		return;
	}
	rename_or_warn("supervise/status.new",
		s->islog ? "log/supervise/status" : "log/supervise/status"+4);
}

static unsigned custom(struct svdir *s, char c)
{
	int pid;
	int w;
	char a[10];
	struct stat st;
	char *prog[2];

	if (s->islog) return 0;
	strcpy(a, "control/?");
	a[8] = c; /* replace '?' */
	if (stat(a, &st) == 0) {
		if (st.st_mode & S_IXUSR) {
			pid = vfork();
			if (pid == -1) {
				warn_cannot("vfork for control/?");
				return 0;
			}
			if (!pid) {
				/* child */
				if (haslog && dup2(logpipe.wr, 1) == -1)
					warn_cannot("setup stdout for control/?");
				prog[0] = a;
				prog[1] = NULL;
				execv(a, prog);
				fatal_cannot("run control/?");
			}
			/* parent */
			while (safe_waitpid(pid, &w, 0) == -1) {
				warn_cannot("wait for child control/?");
				return 0;
			}
			return !wait_exitcode(w);
		}
	} else {
		if (errno != ENOENT)
			warn_cannot("stat control/?");
	}
	return 0;
}

static void stopservice(struct svdir *s)
{
	if (s->pid && !custom(s, 't')) {
		kill(s->pid, SIGTERM);
		s->ctrl |= C_TERM;
		update_status(s);
	}
	if (s->want == W_DOWN) {
		kill(s->pid, SIGCONT);
		custom(s, 'd');
		return;
	}
	if (s->want == W_EXIT) {
		kill(s->pid, SIGCONT);
		custom(s, 'x');
	}
}

static void startservice(struct svdir *s)
{
	int p;
	char *run[2];

	if (s->state == S_FINISH)
		run[0] = (char*)"./finish";
	else {
		run[0] = (char*)"./run";
		custom(s, 'u');
	}
	run[1] = NULL;

	if (s->pid != 0)
		stopservice(s); /* should never happen */
	while ((p = vfork()) == -1) {
		warn_cannot("vfork, sleeping");
		sleep(5);
	}
	if (p == 0) {
		/* child */
		if (haslog) {
			/* NB: bug alert! right order is close, then dup2 */
			if (s->islog) {
				xchdir("./log");
				close(logpipe.wr);
				xdup2(logpipe.rd, 0);
			} else {
				close(logpipe.rd);
				xdup2(logpipe.wr, 1);
			}
		}
		bb_signals(0
			+ (1 << SIGCHLD)
			+ (1 << SIGTERM)
			, SIG_DFL);
		sig_unblock(SIGCHLD);
		sig_unblock(SIGTERM);
		execvp(*run, run);
		fatal2_cannot(s->islog ? "start log/" : "start ", *run);
	}
	/* parent */
	if (s->state != S_FINISH) {
		gettimeofday_ns(&s->start);
		s->state = S_RUN;
	}
	s->pid = p;
	pidchanged = 1;
	s->ctrl = C_NOOP;
	update_status(s);
}

static int ctrl(struct svdir *s, char c)
{
	int sig;

	switch (c) {
	case 'd': /* down */
		s->want = W_DOWN;
		update_status(s);
		if (s->pid && s->state != S_FINISH)
			stopservice(s);
		break;
	case 'u': /* up */
		s->want = W_UP;
		update_status(s);
		if (s->pid == 0)
			startservice(s);
		break;
	case 'x': /* exit */
		if (s->islog)
			break;
		s->want = W_EXIT;
		update_status(s);
		/* FALLTHROUGH */
	case 't': /* sig term */
		if (s->pid && s->state != S_FINISH)
			stopservice(s);
		break;
	case 'k': /* sig kill */
		if (s->pid && !custom(s, c))
			kill(s->pid, SIGKILL);
		s->state = S_DOWN;
		break;
	case 'p': /* sig pause */
		if (s->pid && !custom(s, c))
			kill(s->pid, SIGSTOP);
		s->ctrl |= C_PAUSE;
		update_status(s);
		break;
	case 'c': /* sig cont */
		if (s->pid && !custom(s, c))
			kill(s->pid, SIGCONT);
		if (s->ctrl & C_PAUSE)
			s->ctrl &= ~C_PAUSE;
		update_status(s);
		break;
	case 'o': /* once */
		s->want = W_DOWN;
		update_status(s);
		if (!s->pid)
			startservice(s);
		break;
	case 'a': /* sig alarm */
		sig = SIGALRM;
		goto sendsig;
	case 'h': /* sig hup */
		sig = SIGHUP;
		goto sendsig;
	case 'i': /* sig int */
		sig = SIGINT;
		goto sendsig;
	case 'q': /* sig quit */
		sig = SIGQUIT;
		goto sendsig;
	case '1': /* sig usr1 */
		sig = SIGUSR1;
		goto sendsig;
	case '2': /* sig usr2 */
		sig = SIGUSR2;
		goto sendsig;
	}
	return 1;
 sendsig:
	if (s->pid && !custom(s, c))
		kill(s->pid, sig);
	return 1;
}

int runsv_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int runsv_main(int argc UNUSED_PARAM, char **argv)
{
	struct stat s;
	int fd;
	int r;
	char buf[256];

	INIT_G();

	if (!argv[1] || argv[2])
		bb_show_usage();
	dir = argv[1];

	xpiped_pair(selfpipe);
	close_on_exec_on(selfpipe.rd);
	close_on_exec_on(selfpipe.wr);
	ndelay_on(selfpipe.rd);
	ndelay_on(selfpipe.wr);

	sig_block(SIGCHLD);
	bb_signals_recursive(1 << SIGCHLD, s_child);
	sig_block(SIGTERM);
	bb_signals_recursive(1 << SIGTERM, s_term);

	xchdir(dir);
	/* bss: svd[0].pid = 0; */
	if (S_DOWN) svd[0].state = S_DOWN; /* otherwise already 0 (bss) */
	if (C_NOOP) svd[0].ctrl = C_NOOP;
	if (W_UP) svd[0].want = W_UP;
	/* bss: svd[0].islog = 0; */
	/* bss: svd[1].pid = 0; */
	gettimeofday_ns(&svd[0].start);
	if (stat("down", &s) != -1) svd[0].want = W_DOWN;

	if (stat("log", &s) == -1) {
		if (errno != ENOENT)
			warn_cannot("stat ./log");
	} else {
		if (!S_ISDIR(s.st_mode)) {
			errno = 0;
			warn_cannot("stat log/down: log is not a directory");
		} else {
			haslog = 1;
			svd[1].state = S_DOWN;
			svd[1].ctrl = C_NOOP;
			svd[1].want = W_UP;
			svd[1].islog = 1;
			gettimeofday_ns(&svd[1].start);
			if (stat("log/down", &s) != -1)
				svd[1].want = W_DOWN;
			xpiped_pair(logpipe);
			close_on_exec_on(logpipe.rd);
			close_on_exec_on(logpipe.wr);
		}
	}

	if (mkdir("supervise", 0700) == -1) {
		r = readlink("supervise", buf, sizeof(buf));
		if (r != -1) {
			if (r == sizeof(buf))
				fatal2x_cannot("readlink ./supervise", ": name too long");
			buf[r] = 0;
			mkdir(buf, 0700);
		} else {
			if ((errno != ENOENT) && (errno != EINVAL))
				fatal_cannot("readlink ./supervise");
		}
	}
	svd[0].fdlock = xopen3("log/supervise/lock"+4,
			O_WRONLY|O_NDELAY|O_APPEND|O_CREAT, 0600);
	if (lock_exnb(svd[0].fdlock) == -1)
		fatal_cannot("lock supervise/lock");
	close_on_exec_on(svd[0].fdlock);
	if (haslog) {
		if (mkdir("log/supervise", 0700) == -1) {
			r = readlink("log/supervise", buf, 256);
			if (r != -1) {
				if (r == 256)
					fatal2x_cannot("readlink ./log/supervise", ": name too long");
				buf[r] = 0;
				fd = xopen(".", O_RDONLY|O_NDELAY);
				xchdir("./log");
				mkdir(buf, 0700);
				if (fchdir(fd) == -1)
					fatal_cannot("change back to service directory");
				close(fd);
			}
			else {
				if ((errno != ENOENT) && (errno != EINVAL))
					fatal_cannot("readlink ./log/supervise");
			}
		}
		svd[1].fdlock = xopen3("log/supervise/lock",
				O_WRONLY|O_NDELAY|O_APPEND|O_CREAT, 0600);
		if (lock_ex(svd[1].fdlock) == -1)
			fatal_cannot("lock log/supervise/lock");
		close_on_exec_on(svd[1].fdlock);
	}

	mkfifo("log/supervise/control"+4, 0600);
	svd[0].fdcontrol = xopen("log/supervise/control"+4, O_RDONLY|O_NDELAY);
	close_on_exec_on(svd[0].fdcontrol);
	svd[0].fdcontrolwrite = xopen("log/supervise/control"+4, O_WRONLY|O_NDELAY);
	close_on_exec_on(svd[0].fdcontrolwrite);
	update_status(&svd[0]);
	if (haslog) {
		mkfifo("log/supervise/control", 0600);
		svd[1].fdcontrol = xopen("log/supervise/control", O_RDONLY|O_NDELAY);
		close_on_exec_on(svd[1].fdcontrol);
		svd[1].fdcontrolwrite = xopen("log/supervise/control", O_WRONLY|O_NDELAY);
		close_on_exec_on(svd[1].fdcontrolwrite);
		update_status(&svd[1]);
	}
	mkfifo("log/supervise/ok"+4, 0600);
	fd = xopen("log/supervise/ok"+4, O_RDONLY|O_NDELAY);
	close_on_exec_on(fd);
	if (haslog) {
		mkfifo("log/supervise/ok", 0600);
		fd = xopen("log/supervise/ok", O_RDONLY|O_NDELAY);
		close_on_exec_on(fd);
	}
	for (;;) {
		struct pollfd x[3];
		unsigned deadline;
		char ch;

		if (haslog)
			if (!svd[1].pid && svd[1].want == W_UP)
				startservice(&svd[1]);
		if (!svd[0].pid)
			if (svd[0].want == W_UP || svd[0].state == S_FINISH)
				startservice(&svd[0]);

		x[0].fd = selfpipe.rd;
		x[0].events = POLLIN;
		x[1].fd = svd[0].fdcontrol;
		x[1].events = POLLIN;
		/* x[2] is used only if haslog == 1 */
		x[2].fd = svd[1].fdcontrol;
		x[2].events = POLLIN;
		sig_unblock(SIGTERM);
		sig_unblock(SIGCHLD);
		poll(x, 2 + haslog, 3600*1000);
		sig_block(SIGTERM);
		sig_block(SIGCHLD);

		while (read(selfpipe.rd, &ch, 1) == 1)
			continue;

		for (;;) {
			int child;
			int wstat;

			child = wait_any_nohang(&wstat);
			if (!child)
				break;
			if ((child == -1) && (errno != EINTR))
				break;
			if (child == svd[0].pid) {
				svd[0].pid = 0;
				pidchanged = 1;
				svd[0].ctrl &=~ C_TERM;
				if (svd[0].state != S_FINISH) {
					fd = open_read("finish");
					if (fd != -1) {
						close(fd);
						svd[0].state = S_FINISH;
						update_status(&svd[0]);
						continue;
					}
				}
				svd[0].state = S_DOWN;
				deadline = svd[0].start.tv_sec + 1;
				gettimeofday_ns(&svd[0].start);
				update_status(&svd[0]);
				if (LESS(svd[0].start.tv_sec, deadline))
					sleep(1);
			}
			if (haslog) {
				if (child == svd[1].pid) {
					svd[1].pid = 0;
					pidchanged = 1;
					svd[1].state = S_DOWN;
					svd[1].ctrl &= ~C_TERM;
					deadline = svd[1].start.tv_sec + 1;
					gettimeofday_ns(&svd[1].start);
					update_status(&svd[1]);
					if (LESS(svd[1].start.tv_sec, deadline))
						sleep(1);
				}
			}
		} /* for (;;) */
		if (read(svd[0].fdcontrol, &ch, 1) == 1)
			ctrl(&svd[0], ch);
		if (haslog)
			if (read(svd[1].fdcontrol, &ch, 1) == 1)
				ctrl(&svd[1], ch);

		if (sigterm) {
			ctrl(&svd[0], 'x');
			sigterm = 0;
		}

		if (svd[0].want == W_EXIT && svd[0].state == S_DOWN) {
			if (svd[1].pid == 0)
				_exit(EXIT_SUCCESS);
			if (svd[1].want != W_EXIT) {
				svd[1].want = W_EXIT;
				/* stopservice(&svd[1]); */
				update_status(&svd[1]);
				close(logpipe.wr);
				close(logpipe.rd);
			}
		}
	} /* for (;;) */
	/* not reached */
	return 0;
}
