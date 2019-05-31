/*
 * Copyright 2003, 2004 Porchdog Software. All rights reserved.
 *
 *	Redistribution and use in source and binary forms, with or without modification,
 *	are permitted provided that the following conditions are met:
 *
 *		1. Redistributions of source code must retain the above copyright notice,
 *		   this list of conditions and the following disclaimer.   
 *		2. Redistributions in binary form must reproduce the above copyright notice,
 *		   this list of conditions and the following disclaimer in the documentation
 *		   and/or other materials provided with the distribution.
 *
 *	THIS SOFTWARE IS PROVIDED BY PORCHDOG SOFTWARE ``AS IS'' AND ANY
 *	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *	IN NO EVENT SHALL THE HOWL PROJECT OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 *	INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *	BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 *	OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 *	OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 *	OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *	The views and conclusions contained in the software and documentation are those
 *	of the authors and should not be interpreted as representing official policies,
 *	either expressed or implied, of Porchdog Software.
 */

#include <howl.h>
#include <stdio.h>


static sw_result
socket_handler(
	sw_socket_handler handler,
	sw_salt				salt,
	sw_socket			socket,
	sw_socket_event	events,
	sw_opaque			extra)
{
	sw_assert(0);
}


int
main()
{
	sw_salt		salt;
	sw_socket	socket;
	sw_uint32		msecs = 1000;
	int			i;

	sw_salt_init(&salt, 0, NULL);

	sw_tcp_socket_init(&socket);

	sw_socket_bind(socket, sw_ipv4_address_any(), 0);

	sw_socket_listen(socket, 5);

	sw_salt_register_socket(salt, socket, SW_SOCKET_READ, NULL, socket_handler, NULL);

	for (i = 0; i < 10; i++)
	{
		sw_log(SW_LOG_INFO, "main()", "going into step\n");
		sw_salt_step(salt, &msecs);
		sw_log(SW_LOG_INFO, "main()", "coming out of step\n");
	}

	sw_salt_unregister_socket(salt, socket);

	sw_socket_fina(socket);

	sw_salt_fina(salt);
}
