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
 * lat_rand.c - random number generation
 *
 * usage: lat_rand [-P <parallelism>] [-W <warmup>] [-N <repetitions>]
 *
 * Copyright (c) 2002 Carl Staelin.  Distributed under the FSF GPL with
 * additional restriction that results may published only if
 * (1) the benchmark is unmodified, and
 * (2) the version in the sccsid below is included in the report.
 * Support for this development by Hewlett-Packard is gratefully acknowledged.
 */
char	*id = "$Id: lat_rand.c,v 1.1.1.1 2006-11-23 11:25:34 steven Exp $\n";

#include "bench.h"

#ifdef HAVE_DRAND48
void bench_drand48(iter_t iterations, void *cookie);
void bench_lrand48(iter_t iterations, void *cookie);
#endif
#ifdef HAVE_RAND
void bench_rand(iter_t iterations, void *cookie);
#endif
#ifdef HAVE_RANDOM
void bench_random(iter_t iterations, void *cookie);
#endif
int 
main(int ac, char **av)
{
	int parallel = 1;
	int warmup = 0;
	int repetitions = TRIES;
	int c;
	char* usage = "[-P <parallelism>] [-W <warmup>] [-N <repetitions>]\n";

	while (( c = getopt(ac, av, "P:W:N:")) != EOF) {
		switch(c) {
		case 'P':
			parallel = atoi(optarg);
			if (parallel <= 0) lmbench_usage(ac, av, usage);
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
	if (optind < ac) {
		lmbench_usage(ac, av, usage);
	}

#ifdef HAVE_DRAND48
	benchmp(NULL, bench_drand48, NULL,
		0, parallel, warmup, repetitions, NULL);
	nano("drand48 latency", get_n());

	benchmp(NULL, bench_lrand48, NULL,
		0, parallel, warmup, repetitions, NULL);
	nano("lrand48 latency", get_n());
#endif
#ifdef HAVE_RAND
	benchmp(NULL, bench_rand, NULL,
		0, parallel, warmup, repetitions, NULL);
	nano("rand latency", get_n());
#endif
#ifdef HAVE_RANDOM
	benchmp(NULL, bench_random, NULL,
		0, parallel, warmup, repetitions, NULL);
	nano("random latency", get_n());
#endif
	return (0);
}

#ifdef HAVE_DRAND48
void 
bench_drand48(register iter_t iterations, void *cookie)
{
	register double v = 0.0;
	while (iterations-- > 0) {
		v += drand48();
	}
	use_int((int)v);
}

void 
bench_lrand48(register iter_t iterations, void *cookie)
{
	register long v = 0.0;
	while (iterations-- > 0) {
		v += lrand48();
	}
	use_int((int)v);
}
#endif /* HAVE_DRAND48 */
#ifdef HAVE_RAND
void 
bench_rand(register iter_t iterations, void *cookie)
{
	register int v = 0.0;
	while (iterations-- > 0) {
		v += rand();
	}
	use_int((int)v);
}
#endif /* HAVE_RAND */
#ifdef HAVE_RANDOM
void 
bench_random(register iter_t iterations, void *cookie)
{
	register int v = 0.0;
	while (iterations-- > 0) {
		v += random();
	}
	use_int((int)v);
}
#endif /* HAVE_RANDOM */
