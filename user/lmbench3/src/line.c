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
 * line.c - guess the cache line size
 *
 * usage: line
 *
 * Copyright (c) 2000 Carl Staelin.
 * Copyright (c) 1994 Larry McVoy.  Distributed under the FSF GPL with
 * additional restriction that results may published only if
 * (1) the benchmark is unmodified, and
 * (2) the version in the sccsid below is included in the report.
 * Support for this development by Sun Microsystems is gratefully acknowledged.
 */
char	*id = "$Id: line.c,v 1.1.1.1 2006-11-23 11:25:34 steven Exp $\n";

#include "bench.h"

/*
 * Assumptions:
 *
 * 1) Cache lines are a multiple of pointer-size words
 * 2) Cache lines are no larger than 1/4 a page size
 * 3) Pages are an even multiple of cache lines
 */
int
main(int ac, char **av)
{
	int	i, j, l;
	int	verbose = 0;
	int	warmup = 0;
	int	repetitions = TRIES;
	int	c;
	size_t	maxlen = 64 * 1024 * 1024;
	struct mem_state state;
	char   *usage = "[-v] [-W <warmup>] [-N <repetitions>][-M len[K|M]]\n";

	state.line = sizeof(char*);
	state.pagesize = getpagesize();

	while (( c = getopt(ac, av, "avM:W:N:")) != EOF) {
		switch(c) {
		case 'v':
			verbose = 1;
			break;
		case 'M':
			maxlen = bytes(optarg);
			break;
		case 'W':
			warmup = atoi(optarg);
			break;
		case 'N':
			repetitions = atoi(optarg);
			break;
		default:
			lmbench_usage(ac, av, usage);
			break;
		}
	}

	if ((l = line_find(maxlen, warmup, repetitions, &state)) > 0) {
		if (verbose) {
			printf("cache line size: %d bytes\n", l);
		} else {
			printf("%d\n", l);
		}
	}

	return (0);
}
