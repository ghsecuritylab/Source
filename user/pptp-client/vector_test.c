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
/* vector_test.c .... Test the vector package.
 *                    C. Scott Ananian <cananian@alumni.princeton.edu>
 *
 * $Id: vector_test.c,v 1.1.1.1 2008/07/21 09:19:28 james26_jang Exp $
 */

#include <stdlib.h>
#include <assert.h>
#include "vector.h"

#define MAX 25

void main(void) {
    int i, j, retval;

    VECTOR *v = vector_create();
    assert(v != NULL);
    assert(vector_size(v)==0);
    for (i=0; i<=MAX; i++) {
        assert(!vector_contains(v, i));
        assert(!vector_remove(v, i));
        assert(!vector_search(v, i, (PPTP_CALL **)&j));
        retval = vector_scan(v, i, MAX*2, &j);
        assert(retval);
        assert(j==i);
    }

    for (i=1; i<=MAX; i++) {
        retval = vector_insert(v, i, (PPTP_CALL *)i);
        assert(retval);
        assert(vector_size(v)==i);
    }
    for (i=1; i<MAX; i++) {
        retval = vector_search(v, i, (PPTP_CALL **)&j);
        assert(retval);
        assert(j==i);
        retval = vector_contains(v, i);
        assert(retval);
    }
    assert(vector_size(v)==MAX);
    retval = vector_contains(v, MAX+1);
    assert(!retval);
    retval = vector_search(v, MAX+1, (PPTP_CALL **)&j);
    assert(!retval);

    retval = vector_scan(v, 0, MAX, &j);
    assert(retval);
    assert(j==0);
    retval = vector_scan(v, 1, MAX, &j);
    assert(!retval);
    retval = vector_scan(v, 1, MAX+1, &j);
    assert(retval);
    assert(j==MAX+1);
    retval = vector_scan(v, 1, MAX+MAX, &j);
    assert(retval);
    assert(j==MAX+1);

    for (i=0; i<(MAX*10); i++) {
        int k = (random() % MAX) + 1;
        assert(vector_contains(v, k));
        assert(!vector_scan(v, 1, k, &j));
        assert(!vector_scan(v, k, MAX, &j));
        retval = vector_remove(v, k);
        assert(retval);
        assert(vector_size(v)==MAX-1);
        assert(!vector_contains(v, k));
        assert(!vector_search(v, k, (PPTP_CALL **) &j));
        retval = vector_scan(v, 1, MAX, &j);
        assert(retval);
        assert(j==k);
        retval = vector_insert(v, k, (PPTP_CALL *) k);
        assert(retval);
        assert(vector_size(v)==MAX);
        assert(vector_contains(v, k));
        assert(!vector_scan(v, 1, MAX, &j));
        retval = vector_search(v, k, (PPTP_CALL **) &j);
        assert(retval);
        assert(j==k);
    }

    for (i=1; i<=MAX; i++) {
        assert(vector_size(v)==MAX-(i-1));
        vector_remove(v, i);
        assert(vector_size(v)==MAX-i);
        assert(!vector_contains(v, i));
        retval = vector_search(v, i, (PPTP_CALL **) &j);
        assert(!retval);
        retval = vector_scan(v, 1, MAX, &j);
        assert(retval);
        assert(j==1);
    }
    assert(vector_size(v)==0);
    vector_destroy(v);
}
