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
 * Copyright 1989 - 1991, Julianne Frances Haugh <jockgrrl@austin.rr.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Julianne F. Haugh nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY JULIE HAUGH AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL JULIE HAUGH OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "libbb.h"
#if ENABLE_SELINUX
#include <selinux/selinux.h>  /* for setexeccon  */
#endif

#if ENABLE_SELINUX
static security_context_t current_sid;

void FAST_FUNC renew_current_security_context(void)
{
	freecon(current_sid);  /* Release old context  */
	getcon(&current_sid);  /* update */
}
void FAST_FUNC set_current_security_context(security_context_t sid)
{
	freecon(current_sid);  /* Release old context  */
	current_sid = sid;
}

#endif

/* Run SHELL, or DEFAULT_SHELL if SHELL is empty.
   If COMMAND is nonzero, pass it to the shell with the -c option.
   If ADDITIONAL_ARGS is nonzero, pass it to the shell as more
   arguments.  */

void FAST_FUNC run_shell(const char *shell, int loginshell, const char *command, const char **additional_args)
{
	const char **args;
	int argno = 1;
	int additional_args_cnt = 0;

	for (args = additional_args; args && *args; args++)
		additional_args_cnt++;

	args = xmalloc(sizeof(char*) * (4 + additional_args_cnt));

	args[0] = bb_get_last_path_component_nostrip(xstrdup(shell));

	if (loginshell)
		args[0] = xasprintf("-%s", args[0]);

	if (command) {
		args[argno++] = "-c";
		args[argno++] = command;
	}
	if (additional_args) {
		for (; *additional_args; ++additional_args)
			args[argno++] = *additional_args;
	}
	args[argno] = NULL;
#if ENABLE_SELINUX
	if (current_sid)
		setexeccon(current_sid);
	if (ENABLE_FEATURE_CLEAN_UP)
		freecon(current_sid);
#endif
	execv(shell, (char **) args);
	bb_perror_msg_and_die("cannot run %s", shell);
}
