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
/***********************************************************************
*
* radattr.c
*
* A plugin which is stacked on top of radius.so.  This plugin writes
* all RADIUS attributes from the server's authentication confirmation
* into /var/run/radattr.pppN.  These attributes are available for
* consumption by /etc/ppp/ip-{up,down} scripts.
*
* Copyright (C) 2002 Roaring Penguin Software Inc.
*
* This plugin may be distributed according to the terms of the GNU
* General Public License, version 2 or (at your option) any later version.
*
***********************************************************************/

static char const RCSID[] =
"$Id: radattr.c,v 1.1 2002/01/22 16:03:00 dfs Exp $";

#include "pppd.h"
#include "radiusclient.h"
#include <stdio.h>

extern void (*radius_attributes_hook)(VALUE_PAIR *);
static void print_attributes(VALUE_PAIR *);
static void cleanup(void *opaque, int arg);

char pppd_version[] = VERSION;

/**********************************************************************
* %FUNCTION: plugin_init
* %ARGUMENTS:
*  None
* %RETURNS:
*  Nothing
* %DESCRIPTION:
*  Initializes radattr plugin.
***********************************************************************/
void
plugin_init(void)
{
    radius_attributes_hook = print_attributes;

    add_notifier(&link_down_notifier, cleanup, NULL);

    /* Just in case... */
    add_notifier(&exitnotify, cleanup, NULL);
    info("RADATTR plugin initialized.");
}

/**********************************************************************
* %FUNCTION: print_attributes
* %ARGUMENTS:
*  vp -- linked-list of RADIUS attribute-value pairs
* %RETURNS:
*  Nothing
* %DESCRIPTION:
*  Prints the attribute pairs to /var/run/radattr.pppN.  Each line of the
*  file contains "name value" pairs.
***********************************************************************/
static void
print_attributes(VALUE_PAIR *vp)
{
    FILE *fp;
    char fname[512];
    char name[2048];
    char value[2048];

    slprintf(fname, sizeof(fname), "/var/run/radattr.%s", ifname);
    fp = fopen(fname, "w");
    if (!fp) {
	warn("radattr plugin: Could not open %s for writing: %m", fname);
	return;
    }

    for (; vp; vp=vp->next) {
	if (rc_avpair_tostr(vp, name, sizeof(name), value, sizeof(value)) < 0) {
	    continue;
	}
	fprintf(fp, "%s %s\n", name, value);
    }
    fclose(fp);
}

/**********************************************************************
* %FUNCTION: cleanup
* %ARGUMENTS:
*  opaque -- not used
*  arg -- not used
* %RETURNS:
*  Nothing
* %DESCRIPTION:
*  Deletes /var/run/radattr.pppN
***********************************************************************/
static void
cleanup(void *opaque, int arg)
{
    char fname[512];

    slprintf(fname, sizeof(fname), "/var/run/radattr.%s", ifname);
    (void) remove(fname);
}
