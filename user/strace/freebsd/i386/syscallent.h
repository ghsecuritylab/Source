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
 * Automatically generated by ./../syscalls.pl on Thu Mar  8 18:14:07 2001
 */

  { 1,	0,	sys_syscall,	"syscall"	}, /* 0 */
  { 1,	0,	sys_exit,	"exit"	}, /* 1 */
  { 1,	TP,	sys_fork,	"fork"	}, /* 2 */
  { 3,	TD,	sys_read,	"read"	}, /* 3 */
  { 3,	TD,	sys_write,	"write"	}, /* 4 */
  { 3,	TD|TF,	sys_open,	"open"	}, /* 5 */
  { 1,	TF,	sys_close,	"close"	}, /* 6 */
  { 4,	TP,	sys_wait4,	"wait4"	}, /* 7 */
  { 2,	TD|TF,	sys_creat,	"creat?"	}, /* 8 */
  { 2,	TF,	sys_link,	"link"	}, /* 9 */
  { 1,	TF,	sys_unlink,	"unlink"	}, /* 10 */
  { -1,	0,	printargs,	"SYS_11"	}, /* 11 */
  { 1,	TF,	sys_chdir,	"chdir"	}, /* 12 */
  { 1,	TF,	sys_fchdir,	"fchdir"	}, /* 13 */
  { 3,	TF,	sys_mknod,	"mknod"	}, /* 14 */
  { 2,	TF,	sys_chmod,	"chmod"	}, /* 15 */
  { 3,	TF,	sys_chown,	"chown"	}, /* 16 */
  { 1,	0,	sys_break,	"break"	}, /* 17 */
  { 3,	0,	sys_getfsstat,	"getfsstat"	}, /* 18 */
  { 3,	TD,	sys_lseek,	"lseek?"	}, /* 19 */
  { 1,	0,	sys_getpid,	"getpid"	}, /* 20 */
  { 4,	TF,	sys_mount,	"mount"	}, /* 21 */
  { 2,	TF,	sys_unmount,	"unmount"	}, /* 22 */
  { 1,	0,	sys_setuid,	"setuid"	}, /* 23 */
  { 1,	0,	sys_getuid,	"getuid"	}, /* 24 */
  { 1,	0,	sys_geteuid,	"geteuid"	}, /* 25 */
  { 4,	0,	sys_ptrace,	"ptrace"	}, /* 26 */
  { 3,	TN,	sys_recvmsg,	"recvmsg"	}, /* 27 */
  { 3,	TN,	sys_sendmsg,	"sendmsg"	}, /* 28 */
  { 6,	TN,	sys_recvfrom,	"recvfrom"	}, /* 29 */
  { 3,	TN,	sys_accept,	"accept"	}, /* 30 */
  { 3,	TN,	sys_getpeername,	"getpeername"	}, /* 31 */
  { 3,	TN,	sys_getsockname,	"getsockname"	}, /* 32 */
  { 2,	TF,	sys_access,	"access"	}, /* 33 */
  { 2,	TF,	sys_chflags,	"chflags"	}, /* 34 */
  { 2,	TF,	sys_fchflags,	"fchflags"	}, /* 35 */
  { 1,	0,	sys_sync,	"sync"	}, /* 36 */
  { 2,	TS,	sys_kill,	"kill"	}, /* 37 */
  { 2,	TF,	sys_stat,	"stat?"	}, /* 38 */
  { 1,	0,	sys_getppid,	"getppid"	}, /* 39 */
  { 2,	TF,	sys_lstat,	"lstat?"	}, /* 40 */
  { 1,	TD,	sys_dup,	"dup"	}, /* 41 */
  { 1,	TD,	sys_pipe,	"pipe"	}, /* 42 */
  { 1,	0,	sys_getegid,	"getegid"	}, /* 43 */
  { 4,	0,	sys_profil,	"profil"	}, /* 44 */
  { 4,	0,	sys_ktrace,	"ktrace"	}, /* 45 */
  { 3,	TS,	sys_sigaction,	"sigaction?"	}, /* 46 */
  { 1,	0,	sys_getgid,	"getgid"	}, /* 47 */
  { 2,	TS,	sys_sigprocmask,	"sigprocmask?"	}, /* 48 */
  { 2,	0,	sys_getlogin,	"getlogin"	}, /* 49 */
  { 1,	0,	sys_setlogin,	"setlogin"	}, /* 50 */
  { 1,	TF,	sys_acct,	"acct"	}, /* 51 */
  { 1,	TS,	sys_sigpending,	"sigpending?"	}, /* 52 */
  { 2,	TS,	sys_sigaltstack,	"sigaltstack"	}, /* 53 */
  { 3,	TD,	sys_ioctl,	"ioctl"	}, /* 54 */
  { 1,	0,	sys_reboot,	"reboot"	}, /* 55 */
  { 1,	0,	sys_revoke,	"revoke"	}, /* 56 */
  { 2,	TF,	sys_symlink,	"symlink"	}, /* 57 */
  { 3,	TF,	sys_readlink,	"readlink"	}, /* 58 */
  { 3,	TF|TP,	sys_execve,	"execve"	}, /* 59 */
  { 1,	0,	sys_umask,	"umask"	}, /* 60 */
  { 1,	TF,	sys_chroot,	"chroot"	}, /* 61 */
  { 2,	TD,	sys_fstat,	"fstat?"	}, /* 62 */
  { 4,	0,	sys_getkerninfo,	"getkerninfo?"	}, /* 63 */
  { 1,	0,	sys_getpagesize,	"getpagesize?"	}, /* 64 */
  { 3,	0,	sys_msync,	"msync"	}, /* 65 */
  { 1,	TP,	sys_vfork,	"vfork"	}, /* 66 */
  { -1,	0,	printargs,	"SYS_67"	}, /* 67 */
  { -1,	0,	printargs,	"SYS_68"	}, /* 68 */
  { 1,	0,	sys_sbrk,	"sbrk"	}, /* 69 */
  { 1,	0,	sys_sstk,	"sstk"	}, /* 70 */
  { 6,	0,	sys_mmap,	"mmap?"	}, /* 71 */
  { 1,	0,	sys_vadvise,	"vadvise"	}, /* 72 */
  { 2,	0,	sys_munmap,	"munmap"	}, /* 73 */
  { 3,	0,	sys_mprotect,	"mprotect"	}, /* 74 */
  { 3,	0,	sys_madvise,	"madvise"	}, /* 75 */
  { -1,	0,	printargs,	"SYS_76"	}, /* 76 */
  { -1,	0,	printargs,	"SYS_77"	}, /* 77 */
  { 3,	0,	sys_mincore,	"mincore"	}, /* 78 */
  { 2,	0,	sys_getgroups,	"getgroups"	}, /* 79 */
  { 2,	0,	sys_setgroups,	"setgroups"	}, /* 80 */
  { 1,	0,	sys_getpgrp,	"getpgrp"	}, /* 81 */
  { 2,	0,	sys_setpgid,	"setpgid"	}, /* 82 */
  { 3,	0,	sys_setitimer,	"setitimer"	}, /* 83 */
  { 1,	TP,	sys_wait,	"wait?"	}, /* 84 */
  { 1,	TF,	sys_swapon,	"swapon"	}, /* 85 */
  { 2,	0,	sys_getitimer,	"getitimer"	}, /* 86 */
  { 2,	0,	sys_gethostname,	"gethostname?"	}, /* 87 */
  { 2,	0,	sys_sethostname,	"sethostname?"	}, /* 88 */
  { 1,	0,	sys_getdtablesize,	"getdtablesize"	}, /* 89 */
  { 2,	TD,	sys_dup2,	"dup2"	}, /* 90 */
  { -1,	0,	printargs,	"SYS_91"	}, /* 91 */
  { 3,	TD,	sys_fcntl,	"fcntl"	}, /* 92 */
  { 5,	TD,	sys_select,	"select"	}, /* 93 */
  { -1,	0,	printargs,	"SYS_94"	}, /* 94 */
  { 1,	TD,	sys_fsync,	"fsync"	}, /* 95 */
  { 3,	0,	sys_setpriority,	"setpriority"	}, /* 96 */
  { 3,	TN,	sys_socket,	"socket"	}, /* 97 */
  { 3,	TN,	sys_connect,	"connect"	}, /* 98 */
  { 3,	TN,	sys_accept,	"accept"	}, /* 99 */
  { 2,	0,	sys_getpriority,	"getpriority"	}, /* 100 */
  { 4,	TN,	sys_send,	"send?"	}, /* 101 */
  { 4,	TN,	sys_recv,	"recv?"	}, /* 102 */
  { 1,	TS,	sys_sigreturn,	"sigreturn?"	}, /* 103 */
  { 3,	TN,	sys_bind,	"bind"	}, /* 104 */
  { 5,	TN,	sys_setsockopt,	"setsockopt"	}, /* 105 */
  { 2,	TN,	sys_listen,	"listen"	}, /* 106 */
  { -1,	0,	printargs,	"SYS_107"	}, /* 107 */
  { 3,	TS,	sys_sigvec,	"sigvec?"	}, /* 108 */
  { 1,	TS,	sys_sigblock,	"sigblock?"	}, /* 109 */
  { 1,	TS,	sys_sigsetmask,	"sigsetmask?"	}, /* 110 */
  { 1,	TS,	sys_sigsuspend,	"sigsuspend?"	}, /* 111 */
  { 2,	TS,	sys_sigstack,	"sigstack?"	}, /* 112 */
  { 3,	TN,	sys_recvmsg,	"recvmsg?"	}, /* 113 */
  { 3,	TN,	sys_sendmsg,	"sendmsg?"	}, /* 114 */
  { -1,	0,	printargs,	"SYS_115"	}, /* 115 */
  { 2,	0,	sys_gettimeofday,	"gettimeofday"	}, /* 116 */
  { 2,	0,	sys_getrusage,	"getrusage"	}, /* 117 */
  { 5,	TN,	sys_getsockopt,	"getsockopt"	}, /* 118 */
  { -1,	0,	printargs,	"SYS_119"	}, /* 119 */
  { 3,	TD,	sys_readv,	"readv"	}, /* 120 */
  { 3,	TD,	sys_writev,	"writev"	}, /* 121 */
  { 2,	0,	sys_settimeofday,	"settimeofday"	}, /* 122 */
  { 3,	TD,	sys_fchown,	"fchown"	}, /* 123 */
  { 2,	TD,	sys_fchmod,	"fchmod"	}, /* 124 */
  { 6,	TN,	sys_recvfrom,	"recvfrom"	}, /* 125 */
  { 2,	0,	sys_setreuid,	"setreuid"	}, /* 126 */
  { 2,	0,	sys_setregid,	"setregid"	}, /* 127 */
  { 2,	TF,	sys_rename,	"rename"	}, /* 128 */
  { 2,	TF,	sys_truncate,	"truncate?"	}, /* 129 */
  { 2,	TD,	sys_ftruncate,	"ftruncate?"	}, /* 130 */
  { 2,	TD,	sys_flock,	"flock"	}, /* 131 */
  { 2,	0,	sys_mkfifo,	"mkfifo"	}, /* 132 */
  { 6,	TN,	sys_sendto,	"sendto"	}, /* 133 */
  { 2,	TN,	sys_shutdown,	"shutdown"	}, /* 134 */
  { 4,	TN,	sys_socketpair,	"socketpair"	}, /* 135 */
  { 2,	TF,	sys_mkdir,	"mkdir"	}, /* 136 */
  { 1,	TF,	sys_rmdir,	"rmdir"	}, /* 137 */
  { 2,	TF,	sys_utimes,	"utimes"	}, /* 138 */
  { -1,	0,	printargs,	"SYS_139"	}, /* 139 */
  { 2,	0,	sys_adjtime,	"adjtime"	}, /* 140 */
  { 3,	TN,	sys_getpeername,	"getpeername?"	}, /* 141 */
  { 1,	0,	sys_gethostid,	"gethostid?"	}, /* 142 */
  { 1,	0,	sys_sethostid,	"sethostid?"	}, /* 143 */
  { 2,	0,	sys_getrlimit,	"getrlimit?"	}, /* 144 */
  { 2,	0,	sys_setrlimit,	"setrlimit?"	}, /* 145 */
  { 2,	TS,	sys_killpg,	"killpg?"	}, /* 146 */
  { 1,	0,	sys_setsid,	"setsid"	}, /* 147 */
  { 4,	0,	sys_quotactl,	"quotactl"	}, /* 148 */
  { 1,	0,	sys_quota,	"quota?"	}, /* 149 */
  { 3,	TN,	sys_getsockname,	"getsockname"	}, /* 150 */
  { -1,	0,	printargs,	"SYS_151"	}, /* 151 */
  { -1,	0,	printargs,	"SYS_152"	}, /* 152 */
  { -1,	0,	printargs,	"SYS_153"	}, /* 153 */
  { -1,	0,	printargs,	"SYS_154"	}, /* 154 */
  { 2,	0,	sys_nfssvc,	"nfssvc"	}, /* 155 */
  { 4,	0,	sys_getdirentries,	"getdirentries?"	}, /* 156 */
  { 2,	TF,	sys_statfs,	"statfs"	}, /* 157 */
  { 2,	TD,	sys_fstatfs,	"fstatfs"	}, /* 158 */
  { -1,	0,	printargs,	"SYS_159"	}, /* 159 */
  { -1,	0,	printargs,	"SYS_160"	}, /* 160 */
  { 2,	0,	sys_getfh,	"getfh"	}, /* 161 */
  { 2,	0,	sys_getdomainname,	"getdomainname"	}, /* 162 */
  { 2,	0,	sys_setdomainname,	"setdomainname"	}, /* 163 */
  { 1,	0,	sys_uname,	"uname"	}, /* 164 */
  { 2,	0,	sys_sysarch,	"sysarch"	}, /* 165 */
  { 3,	0,	sys_rtprio,	"rtprio"	}, /* 166 */
  { -1,	0,	printargs,	"SYS_167"	}, /* 167 */
  { -1,	0,	printargs,	"SYS_168"	}, /* 168 */
  { 5,	TI,	sys_semsys,	"semsys"	}, /* 169 */
  { 6,	TI,	sys_msgsys,	"msgsys"	}, /* 170 */
  { 4,	TI,	sys_shmsys,	"shmsys"	}, /* 171 */
  { -1,	0,	printargs,	"SYS_172"	}, /* 172 */
  { 5,	TD,	sys_pread,	"pread"	}, /* 173 */
  { 5,	TD,	sys_pwrite,	"pwrite"	}, /* 174 */
  { -1,	0,	printargs,	"SYS_175"	}, /* 175 */
  { 1,	0,	sys_ntp_adjtime,	"ntp_adjtime"	}, /* 176 */
  { -1,	0,	printargs,	"SYS_177"	}, /* 177 */
  { -1,	0,	printargs,	"SYS_178"	}, /* 178 */
  { -1,	0,	printargs,	"SYS_179"	}, /* 179 */
  { -1,	0,	printargs,	"SYS_180"	}, /* 180 */
  { 1,	0,	sys_setgid,	"setgid"	}, /* 181 */
  { 1,	0,	sys_setegid,	"setegid"	}, /* 182 */
  { 1,	0,	sys_seteuid,	"seteuid"	}, /* 183 */
  { -1,	0,	printargs,	"SYS_184"	}, /* 184 */
  { -1,	0,	printargs,	"SYS_185"	}, /* 185 */
  { -1,	0,	printargs,	"SYS_186"	}, /* 186 */
  { -1,	0,	printargs,	"SYS_187"	}, /* 187 */
  { 2,	TF,	sys_stat,	"stat"	}, /* 188 */
  { 2,	TD,	sys_fstat,	"fstat"	}, /* 189 */
  { 2,	TF,	sys_lstat,	"lstat"	}, /* 190 */
  { 2,	TF,	sys_pathconf,	"pathconf"	}, /* 191 */
  { 2,	0,	sys_fpathconf,	"fpathconf"	}, /* 192 */
  { -1,	0,	printargs,	"SYS_193"	}, /* 193 */
  { 2,	0,	sys_getrlimit,	"getrlimit"	}, /* 194 */
  { 2,	0,	sys_setrlimit,	"setrlimit"	}, /* 195 */
  { 4,	0,	sys_getdirentries,	"getdirentries"	}, /* 196 */
  { 7,	0,	sys_mmap,	"mmap"	}, /* 197 */
  { 1,	0,	sys___syscall,	"__syscall"	}, /* 198 */
  { 4,	TD,	sys_lseek,	"lseek"	}, /* 199 */
  { 3,	TF,	sys_truncate,	"truncate"	}, /* 200 */
  { 3,	TD,	sys_ftruncate,	"ftruncate"	}, /* 201 */
  { 6,	0,	sys___sysctl,	"__sysctl"	}, /* 202 */
  { 2,	0,	sys_mlock,	"mlock"	}, /* 203 */
  { 2,	0,	sys_munlock,	"munlock"	}, /* 204 */
  { 1,	0,	sys_undelete,	"undelete"	}, /* 205 */
  { 2,	0,	sys_futimes,	"futimes"	}, /* 206 */
  { 1,	0,	sys_getpgid,	"getpgid"	}, /* 207 */
  { -1,	0,	printargs,	"SYS_208"	}, /* 208 */
  { 3,	TN,	sys_poll,	"poll"	}, /* 209 */
  { -1,	0,	printargs,	"SYS_210"	}, /* 210 */
  { -1,	0,	printargs,	"SYS_211"	}, /* 211 */
  { -1,	0,	printargs,	"SYS_212"	}, /* 212 */
  { -1,	0,	printargs,	"SYS_213"	}, /* 213 */
  { -1,	0,	printargs,	"SYS_214"	}, /* 214 */
  { -1,	0,	printargs,	"SYS_215"	}, /* 215 */
  { -1,	0,	printargs,	"SYS_216"	}, /* 216 */
  { -1,	0,	printargs,	"SYS_217"	}, /* 217 */
  { -1,	0,	printargs,	"SYS_218"	}, /* 218 */
  { -1,	0,	printargs,	"SYS_219"	}, /* 219 */
  { 4,	0,	sys___semctl,	"__semctl"	}, /* 220 */
  { 3,	TI,	sys_semget,	"semget"	}, /* 221 */
  { 3,	TI,	sys_semop,	"semop"	}, /* 222 */
  { -1,	0,	printargs,	"SYS_223"	}, /* 223 */
  { 3,	TI,	sys_msgctl,	"msgctl"	}, /* 224 */
  { 2,	TI,	sys_msgget,	"msgget"	}, /* 225 */
  { 4,	TI,	sys_msgsnd,	"msgsnd"	}, /* 226 */
  { 5,	TI,	sys_msgrcv,	"msgrcv"	}, /* 227 */
  { 3,	TI,	sys_shmat,	"shmat"	}, /* 228 */
  { 3,	TI,	sys_shmctl,	"shmctl"	}, /* 229 */
  { 1,	TI,	sys_shmdt,	"shmdt"	}, /* 230 */
  { 3,	TI,	sys_shmget,	"shmget"	}, /* 231 */
  { 2,	0,	sys_clock_gettime,	"clock_gettime"	}, /* 232 */
  { 2,	0,	sys_clock_settime,	"clock_settime"	}, /* 233 */
  { 2,	0,	sys_clock_getres,	"clock_getres"	}, /* 234 */
  { -1,	0,	printargs,	"SYS_235"	}, /* 235 */
  { -1,	0,	printargs,	"SYS_236"	}, /* 236 */
  { -1,	0,	printargs,	"SYS_237"	}, /* 237 */
  { -1,	0,	printargs,	"SYS_238"	}, /* 238 */
  { -1,	0,	printargs,	"SYS_239"	}, /* 239 */
  { 2,	0,	sys_nanosleep,	"nanosleep"	}, /* 240 */
  { -1,	0,	printargs,	"SYS_241"	}, /* 241 */
  { -1,	0,	printargs,	"SYS_242"	}, /* 242 */
  { -1,	0,	printargs,	"SYS_243"	}, /* 243 */
  { -1,	0,	printargs,	"SYS_244"	}, /* 244 */
  { -1,	0,	printargs,	"SYS_245"	}, /* 245 */
  { -1,	0,	printargs,	"SYS_246"	}, /* 246 */
  { -1,	0,	printargs,	"SYS_247"	}, /* 247 */
  { -1,	0,	printargs,	"SYS_248"	}, /* 248 */
  { -1,	0,	printargs,	"SYS_249"	}, /* 249 */
  { 3,	0,	sys_minherit,	"minherit"	}, /* 250 */
  { 1,	0,	sys_rfork,	"rfork"	}, /* 251 */
  { 3,	0,	sys_openbsd_poll,	"openbsd_poll"	}, /* 252 */
  { 1,	0,	sys_issetugid,	"issetugid"	}, /* 253 */
  { 3,	TF,	sys_lchown,	"lchown"	}, /* 254 */
  { -1,	0,	printargs,	"SYS_255"	}, /* 255 */
  { -1,	0,	printargs,	"SYS_256"	}, /* 256 */
  { -1,	0,	printargs,	"SYS_257"	}, /* 257 */
  { -1,	0,	printargs,	"SYS_258"	}, /* 258 */
  { -1,	0,	printargs,	"SYS_259"	}, /* 259 */
  { -1,	0,	printargs,	"SYS_260"	}, /* 260 */
  { -1,	0,	printargs,	"SYS_261"	}, /* 261 */
  { -1,	0,	printargs,	"SYS_262"	}, /* 262 */
  { -1,	0,	printargs,	"SYS_263"	}, /* 263 */
  { -1,	0,	printargs,	"SYS_264"	}, /* 264 */
  { -1,	0,	printargs,	"SYS_265"	}, /* 265 */
  { -1,	0,	printargs,	"SYS_266"	}, /* 266 */
  { -1,	0,	printargs,	"SYS_267"	}, /* 267 */
  { -1,	0,	printargs,	"SYS_268"	}, /* 268 */
  { -1,	0,	printargs,	"SYS_269"	}, /* 269 */
  { -1,	0,	printargs,	"SYS_270"	}, /* 270 */
  { -1,	0,	printargs,	"SYS_271"	}, /* 271 */
  { 3,	TD,	sys_getdents,	"getdents"	}, /* 272 */
  { -1,	0,	printargs,	"SYS_273"	}, /* 273 */
  { 2,	0,	sys_lchmod,	"lchmod"	}, /* 274 */
  { 3,	0,	sys_netbsd_lchown,	"netbsd_lchown"	}, /* 275 */
  { 2,	0,	sys_lutimes,	"lutimes"	}, /* 276 */
  { 3,	0,	sys_netbsd_msync,	"netbsd_msync"	}, /* 277 */
  { 2,	0,	sys_nstat,	"nstat"	}, /* 278 */
  { 2,	0,	sys_nfstat,	"nfstat"	}, /* 279 */
  { 2,	0,	sys_nlstat,	"nlstat"	}, /* 280 */
  { -1,	0,	printargs,	"SYS_281"	}, /* 281 */
  { -1,	0,	printargs,	"SYS_282"	}, /* 282 */
  { -1,	0,	printargs,	"SYS_283"	}, /* 283 */
  { -1,	0,	printargs,	"SYS_284"	}, /* 284 */
  { -1,	0,	printargs,	"SYS_285"	}, /* 285 */
  { -1,	0,	printargs,	"SYS_286"	}, /* 286 */
  { -1,	0,	printargs,	"SYS_287"	}, /* 287 */
  { -1,	0,	printargs,	"SYS_288"	}, /* 288 */
  { -1,	0,	printargs,	"SYS_289"	}, /* 289 */
  { -1,	0,	printargs,	"SYS_290"	}, /* 290 */
  { -1,	0,	printargs,	"SYS_291"	}, /* 291 */
  { -1,	0,	printargs,	"SYS_292"	}, /* 292 */
  { -1,	0,	printargs,	"SYS_293"	}, /* 293 */
  { -1,	0,	printargs,	"SYS_294"	}, /* 294 */
  { -1,	0,	printargs,	"SYS_295"	}, /* 295 */
  { -1,	0,	printargs,	"SYS_296"	}, /* 296 */
  { 2,	0,	sys_fhstatfs,	"fhstatfs"	}, /* 297 */
  { 2,	0,	sys_fhopen,	"fhopen"	}, /* 298 */
  { 2,	0,	sys_fhstat,	"fhstat"	}, /* 299 */
  { 1,	0,	sys_modnext,	"modnext"	}, /* 300 */
  { 2,	0,	sys_modstat,	"modstat"	}, /* 301 */
  { 1,	0,	sys_modfnext,	"modfnext"	}, /* 302 */
  { 1,	0,	sys_modfind,	"modfind"	}, /* 303 */
  { 1,	0,	sys_kldload,	"kldload"	}, /* 304 */
  { 1,	0,	sys_kldunload,	"kldunload"	}, /* 305 */
  { 1,	0,	sys_kldfind,	"kldfind"	}, /* 306 */
  { 1,	0,	sys_kldnext,	"kldnext"	}, /* 307 */
  { 2,	0,	sys_kldstat,	"kldstat"	}, /* 308 */
  { 1,	0,	sys_kldfirstmod,	"kldfirstmod"	}, /* 309 */
  { 1,	0,	sys_getsid,	"getsid"	}, /* 310 */
  { 3,	0,	sys_setresuid,	"setresuid"	}, /* 311 */
  { 3,	0,	sys_setresgid,	"setresgid"	}, /* 312 */
  { -1,	0,	printargs,	"SYS_313"	}, /* 313 */
  { 1,	0,	sys_aio_return,	"aio_return"	}, /* 314 */
  { 3,	0,	sys_aio_suspend,	"aio_suspend"	}, /* 315 */
  { 2,	0,	sys_aio_cancel,	"aio_cancel"	}, /* 316 */
  { 1,	0,	sys_aio_error,	"aio_error"	}, /* 317 */
  { 1,	0,	sys_aio_read,	"aio_read"	}, /* 318 */
  { 1,	0,	sys_aio_write,	"aio_write"	}, /* 319 */
  { 4,	0,	sys_lio_listio,	"lio_listio"	}, /* 320 */
  { 1,	0,	sys_yield,	"yield"	}, /* 321 */
  { 1,	0,	sys_thr_sleep,	"thr_sleep"	}, /* 322 */
  { 1,	0,	sys_thr_wakeup,	"thr_wakeup"	}, /* 323 */
  { 1,	0,	sys_mlockall,	"mlockall"	}, /* 324 */
  { 1,	0,	sys_munlockall,	"munlockall"	}, /* 325 */
  { 2,	0,	sys___getcwd,	"__getcwd"	}, /* 326 */
  { 2,	0,	sys_sched_setparam,	"sched_setparam"	}, /* 327 */
  { 2,	0,	sys_sched_getparam,	"sched_getparam"	}, /* 328 */
  { 3,	0,	sys_sched_setscheduler,	"sched_setscheduler"	}, /* 329 */
  { 1,	0,	sys_sched_getscheduler,	"sched_getscheduler"	}, /* 330 */
  { 1,	0,	sys_sched_yield,	"sched_yield"	}, /* 331 */
  { 1,	0,	sys_sched_get_priority_max,	"sched_get_priority_max"	}, /* 332 */
  { 1,	0,	sys_sched_get_priority_min,	"sched_get_priority_min"	}, /* 333 */
  { 2,	0,	sys_sched_rr_get_interval,	"sched_rr_get_interval"	}, /* 334 */
  { 2,	0,	sys_utrace,	"utrace"	}, /* 335 */
  { 7,	TD,	sys_sendfile,	"sendfile"	}, /* 336 */
  { 3,	0,	sys_kldsym,	"kldsym"	}, /* 337 */
  { 1,	0,	sys_jail,	"jail"	}, /* 338 */
  { -1,	0,	printargs,	"SYS_339"	}, /* 339 */
  { 3,	TS,	sys_sigprocmask,	"sigprocmask"	}, /* 340 */
  { 1,	TS,	sys_sigsuspend,	"sigsuspend"	}, /* 341 */
  { 3,	TS,	sys_sigaction,	"sigaction"	}, /* 342 */
  { 1,	TS,	sys_sigpending,	"sigpending"	}, /* 343 */
  { 1,	TS,	sys_sigreturn,	"sigreturn"	}, /* 344 */
  { -1,	0,	printargs,	"SYS_345"	}, /* 345 */
  { -1,	0,	printargs,	"SYS_346"	}, /* 346 */
  { 3,	0,	sys___acl_get_file,	"__acl_get_file"	}, /* 347 */
  { 3,	0,	sys___acl_set_file,	"__acl_set_file"	}, /* 348 */
  { 3,	0,	sys___acl_get_fd,	"__acl_get_fd"	}, /* 349 */
  { 3,	0,	sys___acl_set_fd,	"__acl_set_fd"	}, /* 350 */
  { 2,	0,	sys___acl_delete_file,	"__acl_delete_file"	}, /* 351 */
  { 2,	0,	sys___acl_delete_fd,	"__acl_delete_fd"	}, /* 352 */
  { 3,	0,	sys___acl_aclcheck_file,	"__acl_aclcheck_file"	}, /* 353 */
  { 3,	0,	sys___acl_aclcheck_fd,	"__acl_aclcheck_fd"	}, /* 354 */
  { 4,	0,	sys_extattrctl,	"extattrctl"	}, /* 355 */
  { 4,	0,	sys_extattr_set_file,	"extattr_set_file"	}, /* 356 */
  { 4,	0,	sys_extattr_get_file,	"extattr_get_file"	}, /* 357 */
  { 2,	0,	sys_extattr_delete_file,	"extattr_delete_file"	}, /* 358 */
  { 2,	0,	sys_aio_waitcomplete,	"aio_waitcomplete"	}, /* 359 */
  { 3,	0,	sys_getresuid,	"getresuid"	}, /* 360 */
  { 3,	0,	sys_getresgid,	"getresgid"	}, /* 361 */
  { 1,	0,	sys_kqueue,	"kqueue"	}, /* 362 */
  { 6,	0,	sys_kevent,	"kevent"	}, /* 363 */
