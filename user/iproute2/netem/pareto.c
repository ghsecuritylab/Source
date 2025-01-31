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
 * Pareto distribution table generator
 * Taken from the uncopyrighted NISTnet code.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

#include <linux/types.h>
#include <linux/pkt_sched.h>

static const double a=3.0;
#define TABLESIZE	16384
#define TABLEFACTOR	NETEM_DIST_SCALE

int
main(int argc, char **argv)
{
	int i, n;
	double dvalue;

	printf("# This is the distribution table for the pareto distribution.\n");

	for (i = 65536, n = 0; i > 0; i -= 16) {
		dvalue = (double)i/(double)65536;
		dvalue = 1.0/pow(dvalue, 1.0/a);
		dvalue -= 1.5;
		dvalue *= (4.0/3.0)*(double)TABLEFACTOR;
		if (dvalue > 32767)
			dvalue = 32767;

		printf(" %d", (int)rint(dvalue));
		if (++n == 8) {
			putchar('\n');
			n = 0;
		}
	}
	
	return 0;
}	
