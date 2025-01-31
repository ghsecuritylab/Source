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
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995 Rick Sladkey <jrs@world.std.com>
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
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *	$Id: syscallent.h,v 1.1.1.1 2007-03-22 09:04:29 yy Exp $
 */

	{ 0,	0,	sys_setup,		"setup"		}, /* 0 */
	{ 1,	TP,	sys_exit,		"_exit"		}, /* 1 */
	{ 0,	TP,	sys_fork,		"fork"		}, /* 2 */
	{ 3,	TD,	sys_read,		"read"		}, /* 3 */
	{ 3,	TD,	sys_write,		"write"		}, /* 4 */
	{ 3,	TD|TF,	sys_open,		"open"		}, /* 5 */
	{ 1,	TD,	sys_close,		"close"		}, /* 6 */
	{ 3,	TP,	sys_waitpid,		"waitpid"	}, /* 7 */
	{ 2,	TD|TF,	sys_creat,		"creat"		}, /* 8 */
	{ 2,	TF,	sys_link,		"link"		}, /* 9 */
	{ 1,	TF,	sys_unlink,		"unlink"	}, /* 10 */
	{ 3,	TF|TP,	sys_execve,		"execve"	}, /* 11 */
	{ 1,	TF,	sys_chdir,		"chdir"		}, /* 12 */
	{ 1,	0,	sys_time,		"time"		}, /* 13 */
	{ 3,	TF,	sys_mknod,		"mknod"		}, /* 14 */
	{ 2,	TF,	sys_chmod,		"chmod"		}, /* 15 */
	{ 3,	TF,	sys_chown,		"lchown"	}, /* 16 */
	{ 0,	0,	sys_break,		"break"		}, /* 17 */
	{ 2,	TF,	sys_oldstat,		"oldstat"	}, /* 18 */
	{ 3,	TD,	sys_lseek,		"lseek"		}, /* 19 */
	{ 0,	0,	sys_getpid,		"getpid"	}, /* 20 */
	{ 5,	TF,	sys_mount,		"mount"		}, /* 21 */
	{ 1,	TF,	sys_umount,		"oldumount"	}, /* 22 */
	{ 1,	0,	sys_setuid,		"setuid"	}, /* 23 */
	{ 0,	0,	sys_getuid,		"getuid"	}, /* 24 */
	{ 1,	0,	sys_stime,		"stime"		}, /* 25 */
	{ 4,	0,	sys_ptrace,		"ptrace"	}, /* 26 */
	{ 1,	0,	sys_alarm,		"alarm"		}, /* 27 */
	{ 2,	TD,	sys_oldfstat,		"oldfstat"	}, /* 28 */
	{ 0,	TS,	sys_pause,		"pause"		}, /* 29 */
	{ 2,	TF,	sys_utime,		"utime"		}, /* 30 */
	{ 2,	0,	sys_stty,		"stty"		}, /* 31 */
	{ 2,	0,	sys_gtty,		"gtty"		}, /* 32 */
	{ 2,	TF,	sys_access,		"access"	}, /* 33 */
	{ 1,	0,	sys_nice,		"nice"		}, /* 34 */
	{ 0,	0,	sys_ftime,		"ftime"		}, /* 35 */
	{ 0,	0,	sys_sync,		"sync"		}, /* 36 */
	{ 2,	TS,	sys_kill,		"kill"		}, /* 37 */
	{ 2,	TF,	sys_rename,		"rename"	}, /* 38 */
	{ 2,	TF,	sys_mkdir,		"mkdir"		}, /* 39 */
	{ 1,	TF,	sys_rmdir,		"rmdir"		}, /* 40 */
	{ 1,	TD,	sys_dup,		"dup"		}, /* 41 */
	{ 1,	TD,	sys_pipe,		"pipe"		}, /* 42 */
	{ 1,	0,	sys_times,		"times"		}, /* 43 */
	{ 0,	0,	sys_prof,		"prof"		}, /* 44 */
	{ 1,	0,	sys_brk,		"brk"		}, /* 45 */
	{ 1,	0,	sys_setgid,		"setgid"	}, /* 46 */
	{ 0,	0,	sys_getgid,		"getgid"	}, /* 47 */
	{ 3,	TS,	sys_signal,		"signal"	}, /* 48 */
	{ 0,	0,	sys_geteuid,		"geteuid"	}, /* 49 */
	{ 0,	0,	sys_getegid,		"getegid"	}, /* 50 */
	{ 1,	TF,	sys_acct,		"acct"		}, /* 51 */
	{ 2,	TF,	sys_umount2,		"umount"	}, /* 52 */
	{ 0,	0,	sys_lock,		"lock"		}, /* 53 */
	{ 3,	TD,	sys_ioctl,		"ioctl"		}, /* 54 */
	{ 3,	TD,	sys_fcntl,		"fcntl"		}, /* 55 */
	{ 0,	0,	sys_mpx,		"mpx"		}, /* 56 */
	{ 2,	0,	sys_setpgid,		"setpgid"	}, /* 57 */
	{ 2,	0,	sys_ulimit,		"ulimit"	}, /* 58 */
	{ 1,	0,	sys_oldolduname,	"oldolduname"	}, /* 59 */
	{ 1,	0,	sys_umask,		"umask"		}, /* 60 */
	{ 1,	TF,	sys_chroot,		"chroot"	}, /* 61 */
	{ 2,	0,	sys_ustat,		"ustat"		}, /* 62 */
	{ 2,	TD,	sys_dup2,		"dup2"		}, /* 63 */
	{ 0,	0,	sys_getppid,		"getppid"	}, /* 64 */
	{ 0,	0,	sys_getpgrp,		"getpgrp"	}, /* 65 */
	{ 0,	0,	sys_setsid,		"setsid"	}, /* 66 */
	{ 3,	TS,	sys_sigaction,		"sigaction"	}, /* 67 */
	{ 0,	TS,	sys_siggetmask,		"siggetmask"	}, /* 68 */
	{ 1,	TS,	sys_sigsetmask,		"sigsetmask"	}, /* 69 */
	{ 2,	0,	sys_setreuid,		"setreuid"	}, /* 70 */
	{ 2,	0,	sys_setregid,		"setregid"	}, /* 71 */
	{ 3,	TS,	sys_sigsuspend,		"sigsuspend"	}, /* 72 */
	{ 1,	TS,	sys_sigpending,		"sigpending"	}, /* 73 */
	{ 2,	0,	sys_sethostname,	"sethostname"	}, /* 74 */
	{ 2,	0,	sys_setrlimit,		"setrlimit"	}, /* 75 */
	{ 2,	0,	sys_getrlimit,		"getrlimit"	}, /* 76 */
	{ 2,	0,	sys_getrusage,		"getrusage"	}, /* 77 */
	{ 2,	0,	sys_gettimeofday,	"gettimeofday"	}, /* 78 */
	{ 2,	0,	sys_settimeofday,	"settimeofday"	}, /* 79 */
	{ 2,	0,	sys_getgroups,		"getgroups"	}, /* 80 */
	{ 2,	0,	sys_setgroups,		"setgroups"	}, /* 81 */
	{ 1,	TD,	sys_oldselect,		"oldselect"	}, /* 82 */
	{ 2,	TF,	sys_symlink,		"symlink"	}, /* 83 */
	{ 2,	TF,	sys_oldlstat,		"oldlstat"	}, /* 84 */
	{ 3,	TF,	sys_readlink,		"readlink"	}, /* 85 */
	{ 1,	TF,	sys_uselib,		"uselib"	}, /* 86 */
	{ 1,	TF,	sys_swapon,		"swapon"	}, /* 87 */
	{ 3,	0,	sys_reboot,		"reboot"	}, /* 88 */
	{ 3,	TD,	sys_readdir,		"readdir"	}, /* 89 */
	{ 6,	TD,	sys_old_mmap,		"old_mmap"	}, /* 90 */
	{ 2,	0,	sys_munmap,		"munmap"	}, /* 91 */
	{ 2,	TF,	sys_truncate,		"truncate"	}, /* 92 */
	{ 2,	TD,	sys_ftruncate,		"ftruncate"	}, /* 93 */
	{ 2,	TD,	sys_fchmod,		"fchmod"	}, /* 94 */
	{ 3,	TD,	sys_fchown,		"fchown"	}, /* 95 */
	{ 2,	0,	sys_getpriority,	"getpriority"	}, /* 96 */
	{ 3,	0,	sys_setpriority,	"setpriority"	}, /* 97 */
	{ 4,	0,	sys_profil,		"profil"	}, /* 98 */
	{ 2,	TF,	sys_statfs,		"statfs"	}, /* 99 */
	{ 2,	TD,	sys_fstatfs,		"fstatfs"	}, /* 100 */
	{ 3,	0,	sys_ioperm,		"ioperm"	}, /* 101 */
	{ 2,	TD,	sys_socketcall,		"socketcall"	}, /* 102 */
	{ 3,	0,	sys_syslog,		"syslog"	}, /* 103 */
	{ 3,	0,	sys_setitimer,		"setitimer"	}, /* 104 */
	{ 2,	0,	sys_getitimer,		"getitimer"	}, /* 105 */
	{ 2,	TF,	sys_stat,		"stat"		}, /* 106 */
	{ 2,	TF,	sys_lstat,		"lstat"		}, /* 107 */
	{ 2,	TD,	sys_fstat,		"fstat"		}, /* 108 */
	{ 1,	0,	sys_olduname,		"olduname"	}, /* 109 */
	{ 1,	0,	sys_iopl,		"iopl"		}, /* 110 */
	{ 0,	0,	sys_vhangup,		"vhangup"	}, /* 111 */
	{ 0,	0,	sys_idle,		"idle"		}, /* 112 */
	{ 1,	0,	printargs,		"SYS_113"	}, /* 113 */
	{ 4,	TP,	sys_wait4,		"wait4"		}, /* 114 */
	{ 1,	0,	sys_swapoff,		"swapoff"	}, /* 115 */
	{ 1,	0,	sys_sysinfo,		"sysinfo"	}, /* 116 */
	{ 5,	0,	sys_ipc,		"ipc"		}, /* 117 */
	{ 1,	TD,	sys_fsync,		"fsync"		}, /* 118 */
	{ 1,	TS,	sys_sigreturn,		"sigreturn"	}, /* 119 */
	{ 5,	TP,	sys_clone,		"clone"		}, /* 120 */
	{ 2,	0,	sys_setdomainname,	"setdomainname"	}, /* 121 */
	{ 1,	0,	sys_uname,		"uname"		}, /* 122 */
	{ 3,	0,	printargs,		"SYS_123"	}, /* 123 */
	{ 1,	0,	sys_adjtimex,		"adjtimex"	}, /* 124 */
	{ 3,	0,	sys_mprotect,		"mprotect"	}, /* 125 */
	{ 3,	TS,	sys_sigprocmask,	"sigprocmask"	}, /* 126 */
	{ 2,	0,	sys_create_module,	"create_module"	}, /* 127 */
	{ 3,	0,	sys_init_module,	"init_module"	}, /* 128 */
	{ 1,	0,	sys_delete_module,	"delete_module"	}, /* 129 */
	{ 1,	0,	sys_get_kernel_syms,	"get_kernel_syms"}, /* 130 */
	{ 4,	0,	sys_quotactl,		"quotactl"	}, /* 131 */
	{ 1,	0,	sys_getpgid,		"getpgid"	}, /* 132 */
	{ 1,	TD,	sys_fchdir,		"fchdir"	}, /* 133 */
	{ 0,	0,	sys_bdflush,		"bdflush"	}, /* 134 */
	{ 3,	0,	sys_sysfs,		"sysfs"		}, /* 135 */
	{ 1,	0,	sys_personality,	"personality"	}, /* 136 */
	{ 5,	0,	sys_afs_syscall,	"afs_syscall"	}, /* 137 */
	{ 1,	0,	sys_setfsuid,		"setfsuid"	}, /* 138 */
	{ 1,	0,	sys_setfsgid,		"setfsgid"	}, /* 139 */
	{ 5,	TD,	sys_llseek,		"_llseek"	}, /* 140 */
	{ 3,	TD,	sys_getdents,		"getdents"	}, /* 141 */
	{ 5,	TD,	sys_select,		"select"	}, /* 142 */
	{ 2,	TD,	sys_flock,		"flock"		}, /* 143 */
	{ 3,	0,	sys_msync,		"msync"		}, /* 144 */
	{ 3,	TD,	sys_readv,		"readv"		}, /* 145 */
	{ 3,	TD,	sys_writev,		"writev"	}, /* 146 */
	{ 1,	0,	sys_getsid,		"getsid"	}, /* 147 */
	{ 1,	TD,	sys_fdatasync,		"fdatasync"	}, /* 148 */
	{ 1,	0,	sys_sysctl,		"_sysctl"	}, /* 149 */
	{ 1,	0,	sys_mlock,		"mlock"		}, /* 150 */
	{ 2,	0,	sys_munlock,		"munlock"	}, /* 151 */
	{ 2,	0,	sys_mlockall,		"mlockall"	}, /* 152 */
	{ 0,	0,	sys_munlockall,		"munlockall"	}, /* 153 */
	{ 0,	0,	sys_sched_setparam,	"sched_setparam"}, /* 154 */
	{ 2,	0,	sys_sched_getparam,	"sched_getparam"}, /* 155 */
	{ 3,	0,	sys_sched_setscheduler,	"sched_setscheduler"}, /* 156 */
	{ 1,	0,	sys_sched_getscheduler,	"sched_getscheduler"}, /* 157 */
	{ 0,	0,	sys_sched_yield,	"sched_yield"}, /* 158 */
	{ 1,	0,	sys_sched_get_priority_max,"sched_get_priority_max"}, /* 159 */
	{ 1,	0,	sys_sched_get_priority_min,"sched_get_priority_min"}, /* 160 */
	{ 2,	0,	sys_sched_rr_get_interval,"sched_rr_get_interval"}, /* 161 */
	{ 2,	0,	sys_nanosleep,		"nanosleep"	}, /* 162 */
	{ 4,	0,	sys_mremap,		"mremap"	}, /* 163 */
	{ 3,	0,	sys_setresuid,		"setresuid"	}, /* 164 */
	{ 3,	0,	sys_getresuid,		"getresuid"	}, /* 165 */
	{ 5,	0,	printargs,		"SYS_166"	}, /* 166 */
	{ 5,	0,	sys_query_module,	"query_module"	}, /* 167 */
	{ 3,	TD,	sys_poll,		"poll"		}, /* 168 */
	{ 3,	0,	printargs,		"nfsservctl"	}, /* 169 */
	{ 3,	0,	sys_setresgid,		"setresgid"	}, /* 170 */
	{ 3,	0,	sys_getresgid,		"getresgid"	}, /* 171 */
	{ 5,	0,	printargs,		"prctl"		}, /* 172 */
	{ 1,	TS,	printargs,		"rt_sigreturn"	}, /* 173 */
	{ 4,	TS,	sys_rt_sigaction,	"rt_sigaction"  }, /* 174 */
	{ 4,	TS,	sys_rt_sigprocmask,	"rt_sigprocmask"}, /* 175 */
	{ 2,	TS,	sys_rt_sigpending,	"rt_sigpending"	}, /* 176 */
	{ 4,	TS,	sys_rt_sigtimedwait,	"rt_sigtimedwait"}, /* 177 */
	{ 3,	TS,	sys_rt_sigqueueinfo,    "rt_sigqueueinfo"}, /* 178 */
	{ 2,	TS,	sys_rt_sigsuspend,	"rt_sigsuspend"	}, /* 179 */
	{ 5,	TD,	sys_pread,		"pread"		}, /* 180 */
	{ 5,	TD,	sys_pwrite,		"pwrite"	}, /* 181 */
	{ 3,	TF,	sys_chown,		"chown"		}, /* 182 */
	{ 2,	TF,	sys_getcwd,		"getcwd"	}, /* 183 */
	{ 2,	0,	sys_capget,		"capget"	}, /* 184 */
	{ 2,	0,	sys_capset,		"capset"	}, /* 185 */
	{ 2,	TS,	sys_sigaltstack,	"sigaltstack"	}, /* 186 */
	{ 4,	TD,	sys_sendfile,		"sendfile"	}, /* 187 */
	{ 5,	0,	printargs,		"SYS_188"	}, /* 188 */
	{ 5,	0,	printargs,		"SYS_189"	}, /* 189 */
	{ 0,	TP,	sys_vfork,		"vfork"		}, /* 190 */
	{ 2,	0,	printargs,		"getrlimit"	}, /* 191 */
	{ 6,	0,	sys_mmap,		"mmap2"		}, /* 192 */
	{ 2,	TF,	sys_truncate64,		"truncate64"	}, /* 193 */
	{ 2,	TD,	sys_ftruncate64,	"ftruncate64"	}, /* 194 */
	{ 2,	TF,	sys_stat64,		"stat64"	}, /* 195 */
	{ 2,	TF,	sys_lstat64,		"lstat64"	}, /* 196 */
	{ 2,	TD,	sys_fstat64,		"fstat64"	}, /* 197 */
	{ 3,	TF,	sys_chown,		"lchown32"	}, /* 198 */
	{ 0,	0,	sys_getuid,		"getuid32"	}, /* 199 */
	{ 0,	0,	printargs,		"getgid32"	}, /* 200 */
	{ 0,	0,	printargs,		"geteuid32"	}, /* 201 */
	{ 0,	0,	printargs,		"getegid32"	}, /* 202 */
	{ 2,	0,	printargs,		"setreuid32"	}, /* 203 */
	{ 2,	0,	printargs,		"setregid32"	}, /* 204 */
	{ 2,	0,	sys_getgroups32,	"getgroups32"	}, /* 205 */
	{ 2,	0,	sys_setgroups32,	"setgroups32"	}, /* 206 */
	{ 3,	0,	printargs,		"fchown32"	}, /* 207 */
	{ 3,	0,	printargs,		"setresuid32"	}, /* 208 */
	{ 3,	0,	printargs,		"getresuid32"	}, /* 209 */
	{ 3,	0,	printargs,		"setresgid32"	}, /* 210 */
	{ 3,	0,	printargs,		"getresgid32"	}, /* 211 */
	{ 3,	TF,	printargs,		"chown32"	}, /* 212 */
	{ 1,	0,	printargs,		"setuid32"	}, /* 213 */
	{ 1,	0,	printargs,		"setgid32"	}, /* 214 */
	{ 1,	0,	printargs,		"setfsuid32"	}, /* 215 */
	{ 1,	0,	printargs,		"setfsgid32"	}, /* 216 */
	{ 2,	TF,	sys_pivotroot,		"pivot_root"	}, /* 217 */
	{ 3,	0,	printargs,		"mincore"	}, /* 218 */
	{ 3,	0,	sys_madvise,		"madvise"	}, /* 219 */
	{ 3,	TN,	sys_socket,		"socket"	}, /* 220 */
	{ 3,	TN,	sys_bind,		"bind"		}, /* 221 */
	{ 3,	TN,	sys_connect,		"connect"	}, /* 222 */
	{ 2,	TN,	sys_listen,		"listen"	}, /* 223 */
	{ 3,	TN,	sys_accept,		"accept"	}, /* 224 */
	{ 3,	TN,	sys_getsockname,	"getsockname"	}, /* 225 */
	{ 3,	TN,	sys_getpeername,	"getpeername"	}, /* 226 */
	{ 4,	TN,	sys_socketpair,		"socketpair"	}, /* 227 */
	{ 4,	TN,	sys_send,		"send"		}, /* 228 */
	{ 6,	TN,	sys_sendto,		"sendto"	}, /* 229 */
	{ 4,	TN,	sys_recv,		"recv"		}, /* 230 */
	{ 6,	TN,	sys_recvfrom,		"recvfrom"	}, /* 231 */
	{ 2,	TN,	sys_shutdown,		"shutdown"	}, /* 232 */
	{ 5,	TN,	sys_setsockopt,		"setsockopt"	}, /* 233 */
	{ 5,	TN,	sys_getsockopt,		"getsockopt"	}, /* 234 */
	{ 5,	TN,	sys_sendmsg,		"sendmsg"	}, /* 235 */
	{ 5,	TN,	sys_recvmsg,		"recvmsg"	}, /* 236 */
	{ 4,	TI,	sys_semop,		"semop"		}, /* 237 */
	{ 4,	TI,	sys_semget,		"semget"	}, /* 238 */
	{ 4,	TI,	sys_semctl,		"semctl"	}, /* 239 */
	{ 4,	TI,	sys_msgsnd,		"msgsnd"	}, /* 240 */
	{ 4,	TI,	sys_msgrcv,		"msgrcv"	}, /* 241 */
	{ 4,	TI,	sys_msgget,		"msgget"	}, /* 242 */
	{ 4,	TI,	sys_msgctl,		"msgctl"	}, /* 243 */
	{ 4,	TI,	sys_shmat,		"shmat"		}, /* 244 */
	{ 4,	TI,	sys_shmdt,		"shmdt"		}, /* 245 */
	{ 4,	TI,	sys_shmget,		"shmget"	}, /* 246 */
	{ 4,	TI,	sys_shmctl,		"shmctl"	}, /* 247 */
	{ 5,	0,	printargs,		"SYS_248"	}, /* 248 */
	{ 5,	0,	printargs,		"SYS_249"	}, /* 249 */
	{ 5,	0,	printargs,		"SYS_250"	}, /* 250 */
	{ 5,	0,	printargs,		"SYS_251"	}, /* 251 */
	{ 5,	0,	printargs,		"SYS_252"	}, /* 252 */
	{ 5,	0,	printargs,		"SYS_253"	}, /* 253 */
	{ 5,	0,	printargs,		"SYS_254"	}, /* 254 */
	{ 5,	0,	printargs,		"SYS_255"	}, /* 255 */
	{ 5,	0,	printargs,		"SYS_256"	}, /* 256 */
	{ 5,	0,	printargs,		"SYS_257"	}, /* 257 */
	{ 5,	0,	printargs,		"SYS_258"	}, /* 258 */
	{ 5,	0,	printargs,		"SYS_259"	}, /* 259 */
	{ 5,	0,	printargs,		"SYS_260"	}, /* 260 */
	{ 5,	0,	printargs,		"SYS_261"	}, /* 261 */
	{ 5,	0,	printargs,		"SYS_262"	}, /* 262 */
	{ 5,	0,	printargs,		"SYS_263"	}, /* 263 */
	{ 5,	0,	printargs,		"SYS_264"	}, /* 264 */
	{ 5,	0,	printargs,		"SYS_265"	}, /* 265 */
	{ 5,	0,	printargs,		"SYS_266"	}, /* 266 */
	{ 5,	0,	printargs,		"SYS_267"	}, /* 267 */
	{ 5,	0,	printargs,		"SYS_268"	}, /* 268 */
	{ 5,	0,	printargs,		"SYS_269"	}, /* 269 */
	{ 5,	0,	printargs,		"SYS_270"	}, /* 270 */
	{ 5,	0,	printargs,		"SYS_271"	}, /* 271 */
	{ 5,	0,	printargs,		"SYS_272"	}, /* 272 */
	{ 5,	0,	printargs,		"SYS_273"	}, /* 273 */
	{ 5,	0,	printargs,		"SYS_274"	}, /* 274 */
	{ 5,	0,	printargs,		"SYS_275"	}, /* 275 */
	{ 5,	0,	printargs,		"SYS_276"	}, /* 276 */
	{ 5,	0,	printargs,		"SYS_277"	}, /* 277 */
	{ 5,	0,	printargs,		"SYS_278"	}, /* 278 */
	{ 5,	0,	printargs,		"SYS_279"	}, /* 279 */
	{ 5,	0,	printargs,		"SYS_280"	}, /* 280 */
	{ 5,	0,	printargs,		"SYS_281"	}, /* 281 */
	{ 5,	0,	printargs,		"SYS_282"	}, /* 282 */
	{ 5,	0,	printargs,		"SYS_283"	}, /* 283 */
	{ 5,	0,	printargs,		"SYS_284"	}, /* 284 */
	{ 5,	0,	printargs,		"SYS_285"	}, /* 285 */
	{ 5,	0,	printargs,		"SYS_286"	}, /* 286 */
	{ 5,	0,	printargs,		"SYS_287"	}, /* 287 */
	{ 5,	0,	printargs,		"SYS_288"	}, /* 288 */
	{ 5,	0,	printargs,		"SYS_289"	}, /* 289 */
	{ 5,	0,	printargs,		"SYS_290"	}, /* 290 */
	{ 5,	0,	printargs,		"SYS_291"	}, /* 291 */
	{ 5,	0,	printargs,		"SYS_292"	}, /* 292 */
	{ 5,	0,	printargs,		"SYS_293"	}, /* 293 */
	{ 5,	0,	printargs,		"SYS_294"	}, /* 294 */
	{ 5,	0,	printargs,		"SYS_295"	}, /* 295 */
	{ 5,	0,	printargs,		"SYS_296"	}, /* 296 */
	{ 5,	0,	printargs,		"SYS_297"	}, /* 297 */
	{ 5,	0,	printargs,		"SYS_298"	}, /* 298 */
	{ 5,	0,	printargs,		"SYS_299"	}, /* 299 */
	{ 5,	0,	printargs,		"SYS_300"	}, /* 300 */
	{ 5,	0,	printargs,		"SYS_301"	}, /* 301 */
	{ 5,	0,	printargs,		"SYS_302"	}, /* 302 */
	{ 5,	0,	printargs,		"SYS_303"	}, /* 303 */
	{ 5,	0,	printargs,		"SYS_304"	}, /* 304 */
	{ 5,	0,	printargs,		"SYS_305"	}, /* 305 */
	{ 5,	0,	printargs,		"SYS_306"	}, /* 306 */
	{ 5,	0,	printargs,		"SYS_307"	}, /* 307 */
	{ 5,	0,	printargs,		"SYS_308"	}, /* 308 */
	{ 5,	0,	printargs,		"SYS_309"	}, /* 309 */
	{ 5,	0,	printargs,		"SYS_310"	}, /* 310 */
	{ 5,	0,	printargs,		"SYS_311"	}, /* 311 */
	{ 5,	0,	printargs,		"SYS_312"	}, /* 312 */
	{ 5,	0,	printargs,		"SYS_313"	}, /* 313 */
	{ 5,	0,	printargs,		"SYS_314"	}, /* 314 */
	{ 5,	0,	printargs,		"SYS_315"	}, /* 315 */
	{ 5,	0,	printargs,		"SYS_316"	}, /* 316 */
	{ 5,	0,	printargs,		"SYS_317"	}, /* 317 */
	{ 5,	0,	printargs,		"SYS_318"	}, /* 318 */
	{ 5,	0,	printargs,		"SYS_319"	}, /* 319 */
	{ 5,	0,	printargs,		"SYS_320"	}, /* 320 */
	{ 5,	0,	printargs,		"SYS_321"	}, /* 321 */
	{ 5,	0,	printargs,		"SYS_322"	}, /* 322 */
	{ 5,	0,	printargs,		"SYS_323"	}, /* 323 */
	{ 5,	0,	printargs,		"SYS_324"	}, /* 324 */
	{ 5,	0,	printargs,		"SYS_325"	}, /* 325 */
	{ 5,	0,	printargs,		"SYS_326"	}, /* 326 */
	{ 5,	0,	printargs,		"SYS_327"	}, /* 327 */
	{ 5,	0,	printargs,		"SYS_328"	}, /* 328 */
	{ 5,	0,	printargs,		"SYS_329"	}, /* 329 */
	{ 5,	0,	printargs,		"SYS_330"	}, /* 330 */
	{ 5,	0,	printargs,		"SYS_331"	}, /* 331 */
	{ 5,	0,	printargs,		"SYS_332"	}, /* 332 */
	{ 5,	0,	printargs,		"SYS_333"	}, /* 333 */
	{ 5,	0,	printargs,		"SYS_334"	}, /* 334 */
	{ 5,	0,	printargs,		"SYS_335"	}, /* 335 */
	{ 5,	0,	printargs,		"SYS_336"	}, /* 336 */
	{ 5,	0,	printargs,		"SYS_337"	}, /* 337 */
	{ 5,	0,	printargs,		"SYS_338"	}, /* 338 */
	{ 5,	0,	printargs,		"SYS_339"	}, /* 339 */
	{ 5,	0,	printargs,		"SYS_340"	}, /* 340 */
	{ 5,	0,	printargs,		"SYS_341"	}, /* 341 */
	{ 5,	0,	printargs,		"SYS_342"	}, /* 342 */
	{ 5,	0,	printargs,		"SYS_343"	}, /* 343 */
	{ 5,	0,	printargs,		"SYS_344"	}, /* 344 */
	{ 5,	0,	printargs,		"SYS_345"	}, /* 345 */
	{ 5,	0,	printargs,		"SYS_346"	}, /* 346 */
	{ 5,	0,	printargs,		"SYS_347"	}, /* 347 */
	{ 5,	0,	printargs,		"SYS_348"	}, /* 348 */
	{ 5,	0,	printargs,		"SYS_349"	}, /* 349 */
	{ 5,	0,	printargs,		"SYS_350"	}, /* 350 */
	{ 5,	0,	printargs,		"SYS_351"	}, /* 351 */
	{ 5,	0,	printargs,		"SYS_352"	}, /* 352 */
	{ 5,	0,	printargs,		"SYS_353"	}, /* 353 */
	{ 5,	0,	printargs,		"SYS_354"	}, /* 354 */
	{ 5,	0,	printargs,		"SYS_355"	}, /* 355 */
	{ 5,	0,	printargs,		"SYS_356"	}, /* 356 */
	{ 5,	0,	printargs,		"SYS_357"	}, /* 357 */
	{ 5,	0,	printargs,		"SYS_358"	}, /* 358 */
	{ 5,	0,	printargs,		"SYS_359"	}, /* 359 */
	{ 5,	0,	printargs,		"SYS_360"	}, /* 360 */
	{ 5,	0,	printargs,		"SYS_361"	}, /* 361 */
	{ 5,	0,	printargs,		"SYS_362"	}, /* 362 */
	{ 5,	0,	printargs,		"SYS_363"	}, /* 363 */
	{ 5,	0,	printargs,		"SYS_364"	}, /* 364 */
	{ 5,	0,	printargs,		"SYS_365"	}, /* 365 */
	{ 5,	0,	printargs,		"SYS_366"	}, /* 366 */
	{ 5,	0,	printargs,		"SYS_367"	}, /* 367 */
	{ 5,	0,	printargs,		"SYS_368"	}, /* 368 */
	{ 5,	0,	printargs,		"SYS_369"	}, /* 369 */
	{ 5,	0,	printargs,		"SYS_370"	}, /* 370 */
	{ 5,	0,	printargs,		"SYS_371"	}, /* 371 */
	{ 5,	0,	printargs,		"SYS_372"	}, /* 372 */
	{ 5,	0,	printargs,		"SYS_373"	}, /* 373 */
	{ 5,	0,	printargs,		"SYS_374"	}, /* 374 */
	{ 5,	0,	printargs,		"SYS_375"	}, /* 375 */
	{ 5,	0,	printargs,		"SYS_376"	}, /* 376 */
	{ 5,	0,	printargs,		"SYS_377"	}, /* 377 */
	{ 5,	0,	printargs,		"SYS_378"	}, /* 378 */
	{ 5,	0,	printargs,		"SYS_379"	}, /* 379 */
	{ 5,	0,	printargs,		"SYS_380"	}, /* 380 */
	{ 5,	0,	printargs,		"SYS_381"	}, /* 381 */
	{ 5,	0,	printargs,		"SYS_382"	}, /* 382 */
	{ 5,	0,	printargs,		"SYS_383"	}, /* 383 */
	{ 5,	0,	printargs,		"SYS_384"	}, /* 384 */
	{ 5,	0,	printargs,		"SYS_385"	}, /* 385 */
	{ 5,	0,	printargs,		"SYS_386"	}, /* 386 */
	{ 5,	0,	printargs,		"SYS_387"	}, /* 387 */
	{ 5,	0,	printargs,		"SYS_388"	}, /* 388 */
	{ 5,	0,	printargs,		"SYS_389"	}, /* 389 */
	{ 5,	0,	printargs,		"SYS_390"	}, /* 390 */
	{ 5,	0,	printargs,		"SYS_391"	}, /* 391 */
	{ 5,	0,	printargs,		"SYS_392"	}, /* 392 */
	{ 5,	0,	printargs,		"SYS_393"	}, /* 393 */
	{ 5,	0,	printargs,		"SYS_394"	}, /* 394 */
	{ 5,	0,	printargs,		"SYS_395"	}, /* 395 */
	{ 5,	0,	printargs,		"SYS_396"	}, /* 396 */
	{ 5,	0,	printargs,		"SYS_397"	}, /* 397 */
	{ 5,	0,	printargs,		"SYS_398"	}, /* 398 */
	{ 5,	0,	printargs,		"SYS_399"	}, /* 399 */

#if SYS_ipc_subcall != 400
 #error fix me
#endif
	{ 8,	0,	printargs,		"socket_subcall"}, /* 400 */
	{ 3,	TN,	sys_socket,		"socket"	}, /* 401 */
	{ 3,	TN,	sys_bind,		"bind"		}, /* 402 */
	{ 3,	TN,	sys_connect,		"connect"	}, /* 403 */
	{ 2,	TN,	sys_listen,		"listen"	}, /* 404 */
	{ 3,	TN,	sys_accept,		"accept"	}, /* 405 */
	{ 3,	TN,	sys_getsockname,	"getsockname"	}, /* 406 */
	{ 3,	TN,	sys_getpeername,	"getpeername"	}, /* 407 */
	{ 4,	TN,	sys_socketpair,		"socketpair"	}, /* 408 */
	{ 4,	TN,	sys_send,		"send"		}, /* 409 */
	{ 4,	TN,	sys_recv,		"recv"		}, /* 410 */
	{ 6,	TN,	sys_sendto,		"sendto"	}, /* 411 */
	{ 6,	TN,	sys_recvfrom,		"recvfrom"	}, /* 412 */
	{ 2,	TN,	sys_shutdown,		"shutdown"	}, /* 413 */
	{ 5,	TN,	sys_setsockopt,		"setsockopt"	}, /* 414 */
	{ 5,	TN,	sys_getsockopt,		"getsockopt"	}, /* 415 */
	{ 5,	TN,	sys_sendmsg,		"sendmsg"	}, /* 416 */
	{ 5,	TN,	sys_recvmsg,		"recvmsg"	}, /* 417 */

#if SYS_ipc_subcall != 418
 #error fix me
#endif
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 418 */
