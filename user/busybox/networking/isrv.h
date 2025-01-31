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
 * Generic non-forking server infrastructure.
 * Intended to make writing telnetd-type servers easier.
 *
 * Copyright (C) 2007 Denys Vlasenko
 *
 * Licensed under GPL version 2, see file LICENSE in this tarball for details.
 */

#if __GNUC_PREREQ(4,1)
# pragma GCC visibility push(hidden)
#endif

/* opaque structure */
struct isrv_state_t;
typedef struct isrv_state_t isrv_state_t;

/* callbacks */
void isrv_want_rd(isrv_state_t *state, int fd);
void isrv_want_wr(isrv_state_t *state, int fd);
void isrv_dont_want_rd(isrv_state_t *state, int fd);
void isrv_dont_want_wr(isrv_state_t *state, int fd);
int isrv_register_fd(isrv_state_t *state, int peer, int fd);
void isrv_close_fd(isrv_state_t *state, int fd);
int isrv_register_peer(isrv_state_t *state, void *param);

/* driver */
void isrv_run(
	int listen_fd,
	int (*new_peer)(isrv_state_t *state, int fd),
	int (*do_rd)(int fd, void **),
	int (*do_wr)(int fd, void **),
	int (*do_timeout)(void **),
	int timeout,
	int linger_timeout
);

#if __GNUC_PREREQ(4,1)
# pragma GCC visibility pop
#endif
