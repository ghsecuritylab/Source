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
 * LICENSE NOTICE.
 *
 * Use of the Microsoft Windows Rally Development Kit is covered under
 * the Microsoft Windows Rally Development Kit License Agreement,
 * which is provided within the Microsoft Windows Rally Development
 * Kit or at http://www.microsoft.com/whdc/rally/rallykit.mspx. If you
 * want a license from Microsoft to use the software in the Microsoft
 * Windows Rally Development Kit, you must (1) complete the designated
 * "licensee" information in the Windows Rally Development Kit License
 * Agreement, and (2) sign and return the Agreement AS IS to Microsoft
 * at the address provided in the Agreement.
 */

/*
 * Copyright (c) Microsoft Corporation 2005.  All rights reserved.
 * This software is provided with NO WARRANTY.
 */

#include <stdio.h>
#include <assert.h>

#include "globals.h"

#include "statemachines.h"
#include "band.h"
#include "bandfuncs.h"

void
band_ResetNi(void)
{
    g_band.Ni = BAND_NMAX;
}


void
band_InitStats(void)
{
    band_t *b = &g_band;
    struct timeval now;

    b->Ni = BAND_NMAX;
    b->r = BAND_BLOCK_TIME;
    b->begun = FALSE;

    if (TRACE(TRC_BAND))
	dbgprintf("InitStats: Ni=%u r=%u\n",
		b->Ni, b->r);

    gettimeofday(&now, NULL);
    timeval_add_ms(&now, BAND_BLOCK_TIME);

    assert(g_block_timer == NULL);
    g_block_timer = event_add(&now, state_block_timeout, /*state:*/NULL);
}


void
band_UpdateStats(void)
{
    band_t *b = &g_band;
    struct timeval now;
    
    /* TODO: assumes platform timers are perfect!  Should really measure the time
     * now and calculate the actual time waited for. */
    uint32_t Ta = BAND_BLOCK_TIME;

    uint32_t value = BAND_MUL_FRAME(b->r * b->Ni) / (Ta * 1000);
    uint32_t bound = ((b->Ni * BAND_GAMMA) + (BAND_ALPHA * BAND_BETA -1))
      / (BAND_ALPHA * BAND_BETA);
    b->Ni = TOPOMIN(100*BAND_NMAX, TOPOMAX(bound, value));

    if (b->begun) {
	if (b->Ni < BAND_NMAX/2) {
	    b->Ni *= 2;
        } else if (b->Ni < BAND_NMAX) {
	    b->Ni = BAND_NMAX;
	}
    }
    b->begun = FALSE;

    if (TRACE(TRC_BAND))
	dbgprintf("UpdateStats: Ni=%u r=%u\n",
		b->Ni, b->r);

    if (!g_block_timer)
    {
	gettimeofday(&now, NULL);
	timeval_add_ms(&now, BAND_BLOCK_TIME);
	g_block_timer = event_add(&now, state_block_timeout, /*state:*/NULL);
    }
    else
    {
	warn("UpdateStats: block timer still running?!?\n");
    }
}

void
band_ChooseHelloTime(void)
{
    band_t *b = &g_band;

    uint32_t delay = random_uniform(b->Ni);

    if (TRACE(TRC_BAND))
	dbgprintf("ChooseHelloTime: delay=%u; ", delay);

    /* if the delay falls within this block, set a timer to send the Hello */
    if (delay < BAND_ALPHA && !g_hello_timer)
    {
	struct timeval now;
	gettimeofday(&now, NULL);
	timeval_add_ms(&now, BAND_MUL_FRAME(delay));
	g_hello_timer = event_add(&now, state_hello_delay_timeout, /*state:*/NULL);
	if (TRACE(TRC_BAND))
	    dbgprintf("will hello at %ld.%06ld\n", now.tv_sec, now.tv_usec);
    }
    else
    {
	if (TRACE(TRC_BAND))
	    dbgprintf("idle this block, so no Hello\n");
    }
}


void
band_IncreaseLoad(bool_t mapbegin)
{
    band_t *b = &g_band;

    b->r++;
    if (mapbegin)
	b->begun = TRUE;

    if (TRACE(TRC_BAND))
	dbgprintf("IncreaseLoad: Ni=%u r=%u\n",
		b->Ni, b->r);
}
