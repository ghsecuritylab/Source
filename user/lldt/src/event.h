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

#ifndef EVENT_H
#define EVENT_H

/* socket and timeout event dispatcher */
#include <sys/time.h>

/* Initialise the eventing system */
extern void event_init(void);

/* Add a new event to the list, calling "function(state)" at "firetime". */
extern event_t *event_add(struct timeval *firetime, event_fn_t function, void *state);

/* Cancel a previously requested event; return TRUE if successful, FALSE if the
 * event wasn't found. */
extern bool_t event_cancel(event_t *event);

/* You can register a handler function to deal with IO on a file descriptor: */
/* NB: Limited to one fd of interest only! */
typedef void (*event_io_fn_t)(int fd, void *state);
extern void event_add_io(int fd, event_io_fn_t function, void *state);
extern void event_remove_io(int fd);

/* Capture the current thread, and run event and IO handlers forever.
 * Does not return. */
extern void event_mainloop(void);

#endif /* EVENT_H */
