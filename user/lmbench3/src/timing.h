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
 * $Id: timing.h,v 1.1.1.1 2006-11-23 11:25:34 steven Exp $
 */
#ifndef _TIMING_H
#define _TIMING_H

char	*p64(uint64 big);
char	*p64sz(uint64 big);
double	Delta(void);
double	Now(void);
void	adjust(int usec);
void	bandwidth(uint64 bytes, uint64 times, int verbose);
uint64	bytes(char *s);
void	context(uint64 xfers);
uint64	delta(void);
int	get_enough(int);
uint64	get_n(void);
void	kb(uint64 bytes);
double	l_overhead(void);
char	last(char *s);
void	latency(uint64 xfers, uint64 size);
void	mb(uint64 bytes);
void	micro(char *s, uint64 n);
void	micromb(uint64 mb, uint64 n);
void	milli(char *s, uint64 n);
void	morefds(void);
void	nano(char *s, uint64 n);
uint64	now(void);
void	ptime(uint64 n);
void	rusage(void);
void	save_n(uint64);
void	settime(uint64 usecs);
void	start(struct timeval *tv);
uint64	stop(struct timeval *begin, struct timeval *end);
uint64	t_overhead(void);
double	timespent(void);
void	timing(FILE *out);
uint64	tvdelta(struct timeval *, struct timeval *);
void	tvsub(struct timeval *tdiff, struct timeval *t1, struct timeval *t0);
void	use_int(int result);
void	use_pointer(void *result);
uint64	usecs_spent(void);
void	touch(char *buf, int size);
size_t*	permutation(int max, int scale);
int	cp(char* src, char* dst, mode_t mode);
long	bread(void* src, long count);

#if defined(hpux) || defined(__hpux)
int	getpagesize();
#endif

#endif /* _TIMING_H */
