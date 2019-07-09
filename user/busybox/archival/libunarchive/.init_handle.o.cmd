cmd_archival/libunarchive/init_handle.o := /opt/buildroot-gcc342/bin/mipsel-linux-uclibc-gcc -Wp,-MD,archival/libunarchive/.init_handle.o.d   -std=gnu99 -Iinclude -Ilibbb  -include include/autoconf.h -D_GNU_SOURCE -DNDEBUG  -D"BB_VER=KBUILD_STR(1.12.1)" -DBB_BT=AUTOCONF_TIMESTAMP -O2 -fomit-frame-pointer -pipe  -Dlinux -D__linux__ -Dunix -DEMBED -I/home/prokascc/Source/lib/include -I/home/prokascc/Source -DMODEL_NAME=\"RP-N12\" -DOOB_SSID_2G=\"ASUS_RPN12\" -DOOB_SSID_5G=\"ASUS_RPN12_5G\" -Wall -Wshadow -Wwrite-strings -Wundef -Wstrict-prototypes -Wunused -Wunused-parameter -Wmissing-prototypes -Wmissing-declarations -Wdeclaration-after-statement  -fno-builtin-strlen -finline-limit=0 -fomit-frame-pointer -ffunction-sections -fdata-sections -fno-guess-branch-probability -funsigned-char -static-libgcc -falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1 -Os -Dlinux    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(init_handle)"  -D"KBUILD_MODNAME=KBUILD_STR(init_handle)" -c -o archival/libunarchive/init_handle.o archival/libunarchive/init_handle.c

deps_archival/libunarchive/init_handle.o := \
  archival/libunarchive/init_handle.c \
  include/libbb.h \
    $(wildcard include/config/selinux.h) \
    $(wildcard include/config/locale/support.h) \
    $(wildcard include/config/use/bb/pwd/grp.h) \
    $(wildcard include/config/feature/shadowpasswds.h) \
    $(wildcard include/config/use/bb/shadow.h) \
    $(wildcard include/config/lfs.h) \
    $(wildcard include/config/feature/buffers/go/on/stack.h) \
    $(wildcard include/config/buffer.h) \
    $(wildcard include/config/ubuffer.h) \
    $(wildcard include/config/feature/buffers/go/in/bss.h) \
    $(wildcard include/config/inux.h) \
    $(wildcard include/config/feature/ipv6.h) \
    $(wildcard include/config/feature/check/names.h) \
    $(wildcard include/config/feature/prefer/applets.h) \
    $(wildcard include/config/busybox/exec/path.h) \
    $(wildcard include/config/getopt/long.h) \
    $(wildcard include/config/feature/pidfile.h) \
    $(wildcard include/config/feature/syslog.h) \
    $(wildcard include/config/feature/individual.h) \
    $(wildcard include/config/o.h) \
    $(wildcard include/config/ntf.h) \
    $(wildcard include/config/t.h) \
    $(wildcard include/config/l.h) \
    $(wildcard include/config/wn.h) \
    $(wildcard include/config/.h) \
    $(wildcard include/config/ktop.h) \
    $(wildcard include/config/route.h) \
    $(wildcard include/config/feature/hwib.h) \
    $(wildcard include/config/debug/crond/option.h) \
    $(wildcard include/config/use/bb/crypt.h) \
    $(wildcard include/config/ioctl/hex2str/error.h) \
    $(wildcard include/config/feature/editing.h) \
    $(wildcard include/config/feature/editing/history.h) \
    $(wildcard include/config/ture/editing/savehistory.h) \
    $(wildcard include/config/feature/editing/savehistory.h) \
    $(wildcard include/config/feature/tab/completion.h) \
    $(wildcard include/config/feature/username/completion.h) \
    $(wildcard include/config/feature/editing/vi.h) \
    $(wildcard include/config/feature/topmem.h) \
    $(wildcard include/config/pgrep.h) \
    $(wildcard include/config/pkill.h) \
    $(wildcard include/config/pidof.h) \
    $(wildcard include/config/feature/devfs.h) \
  include/platform.h \
    $(wildcard include/config/werror.h) \
    $(wildcard include/config///.h) \
    $(wildcard include/config/nommu.h) \
    $(wildcard include/config//nommu.h) \
    $(wildcard include/config//mmu.h) \
  /home/prokascc/Source/lib/include/byteswap.h \
  /home/prokascc/Source/lib/include/bits/byteswap.h \
  /home/prokascc/Source/lib/include/endian.h \
  /home/prokascc/Source/lib/include/features.h \
    $(wildcard include/config/c99.h) \
    $(wildcard include/config/ix.h) \
    $(wildcard include/config/ix2.h) \
    $(wildcard include/config/ix199309.h) \
    $(wildcard include/config/ix199506.h) \
    $(wildcard include/config/en.h) \
    $(wildcard include/config/en/extended.h) \
    $(wildcard include/config/x98.h) \
    $(wildcard include/config/en2k.h) \
    $(wildcard include/config/gefile.h) \
    $(wildcard include/config/gefile64.h) \
    $(wildcard include/config/e/offset64.h) \
    $(wildcard include/config/d.h) \
    $(wildcard include/config/c.h) \
    $(wildcard include/config/ntrant.h) \
    $(wildcard include/config/i.h) \
    $(wildcard include/config/ern/inlines.h) \
  /home/prokascc/Source/lib/include/bits/uClibc_config.h \
    $(wildcard include/config/mips/isa/1//.h) \
    $(wildcard include/config/mips/isa/2//.h) \
    $(wildcard include/config/mips/isa/3//.h) \
    $(wildcard include/config/mips/isa/4//.h) \
    $(wildcard include/config/mips/isa/mips32//.h) \
    $(wildcard include/config/mips/isa/mips64//.h) \
    $(wildcard include/config//.h) \
  /home/prokascc/Source/lib/include/sys/cdefs.h \
    $(wildcard include/config/espaces.h) \
    $(wildcard include/config/tify/level.h) \
  /home/prokascc/Source/lib/include/bits/endian.h \
  /home/prokascc/Source/lib/include/arpa/inet.h \
  /home/prokascc/Source/lib/include/netinet/in.h \
  /home/prokascc/Source/lib/include/stdint.h \
  /home/prokascc/Source/lib/include/bits/wchar.h \
  /home/prokascc/Source/lib/include/bits/wordsize.h \
  /home/prokascc/Source/lib/include/bits/types.h \
  /opt/buildroot-gcc342/bin/../lib/gcc/mipsel-linux-uclibc/3.4.2/include/stddef.h \
  /home/prokascc/Source/lib/include/bits/kernel_types.h \
  /home/prokascc/Source/lib/include/bits/pthreadtypes.h \
  /home/prokascc/Source/lib/include/bits/sched.h \
  /home/prokascc/Source/lib/include/bits/socket.h \
  /home/prokascc/Source/lib/include/limits.h \
  /opt/buildroot-gcc342/bin/../lib/gcc/mipsel-linux-uclibc/3.4.2/include/limits.h \
  /home/prokascc/Source/lib/include/bits/posix1_lim.h \
  /home/prokascc/Source/lib/include/bits/local_lim.h \
  /home/prokascc/Source/lib/include/linux/limits.h \
  /home/prokascc/Source/lib/include/bits/posix2_lim.h \
  /home/prokascc/Source/lib/include/bits/xopen_lim.h \
  /home/prokascc/Source/lib/include/bits/stdio_lim.h \
  /home/prokascc/Source/lib/include/sys/types.h \
  /home/prokascc/Source/lib/include/time.h \
  /home/prokascc/Source/lib/include/sys/select.h \
  /home/prokascc/Source/lib/include/bits/select.h \
  /home/prokascc/Source/lib/include/bits/sigset.h \
  /home/prokascc/Source/lib/include/bits/time.h \
  /home/prokascc/Source/lib/include/sys/sysmacros.h \
  /home/prokascc/Source/lib/include/bits/sockaddr.h \
  /home/prokascc/Source/lib/include/asm/socket.h \
  /home/prokascc/Source/lib/include/asm/sockios.h \
  /home/prokascc/Source/lib/include/asm/ioctl.h \
  /home/prokascc/Source/lib/include/asm-generic/ioctl.h \
  /home/prokascc/Source/lib/include/bits/in.h \
  /opt/buildroot-gcc342/bin/../lib/gcc/mipsel-linux-uclibc/3.4.2/include/stdbool.h \
  /home/prokascc/Source/lib/include/sys/mount.h \
  /home/prokascc/Source/lib/include/sys/ioctl.h \
  /home/prokascc/Source/lib/include/bits/ioctls.h \
  /home/prokascc/Source/lib/include/asm/ioctls.h \
  /home/prokascc/Source/lib/include/bits/ioctl-types.h \
  /home/prokascc/Source/lib/include/sys/ttydefaults.h \
  /home/prokascc/Source/lib/include/ctype.h \
  /home/prokascc/Source/lib/include/bits/uClibc_touplow.h \
  /home/prokascc/Source/lib/include/dirent.h \
  /home/prokascc/Source/lib/include/bits/dirent.h \
  /home/prokascc/Source/lib/include/errno.h \
  /home/prokascc/Source/lib/include/bits/errno.h \
  /home/prokascc/Source/lib/include/bits/errno_values.h \
  /home/prokascc/Source/lib/include/fcntl.h \
  /home/prokascc/Source/lib/include/bits/fcntl.h \
  /home/prokascc/Source/lib/include/sys/stat.h \
  /home/prokascc/Source/lib/include/bits/stat.h \
  /home/prokascc/Source/lib/include/inttypes.h \
  /home/prokascc/Source/lib/include/netdb.h \
  /home/prokascc/Source/lib/include/rpc/netdb.h \
  /home/prokascc/Source/lib/include/bits/siginfo.h \
  /home/prokascc/Source/lib/include/bits/netdb.h \
  /home/prokascc/Source/lib/include/setjmp.h \
  /home/prokascc/Source/lib/include/bits/setjmp.h \
  /home/prokascc/Source/lib/include/signal.h \
  /home/prokascc/Source/lib/include/bits/signum.h \
  /home/prokascc/Source/lib/include/bits/sigaction.h \
  /home/prokascc/Source/lib/include/bits/sigcontext.h \
  /home/prokascc/Source/lib/include/asm/sigcontext.h \
  /home/prokascc/Source/lib/include/linux/types.h \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbdaf.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
    $(wildcard include/config/64bit.h) \
  /home/prokascc/Source/lib/include/asm/types.h \
    $(wildcard include/config/highmem.h) \
    $(wildcard include/config/64bit/phys/addr.h) \
  /home/prokascc/Source/lib/include/asm-generic/int-ll64.h \
  /home/prokascc/Source/lib/include/asm/bitsperlong.h \
  /home/prokascc/Source/lib/include/asm-generic/bitsperlong.h \
  /home/prokascc/Source/lib/include/linux/posix_types.h \
  /home/prokascc/Source/lib/include/linux/stddef.h \
  /home/prokascc/Source/lib/include/linux/compiler.h \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  /home/prokascc/Source/lib/include/asm/posix_types.h \
  /home/prokascc/Source/lib/include/asm/sgidefs.h \
  /home/prokascc/Source/lib/include/bits/sigstack.h \
  /home/prokascc/Source/lib/include/ucontext.h \
  /home/prokascc/Source/lib/include/sys/ucontext.h \
  /home/prokascc/Source/lib/include/sgidefs.h \
  /home/prokascc/Source/lib/include/bits/sigthread.h \
  /home/prokascc/Source/lib/include/stdio.h \
  /home/prokascc/Source/lib/include/bits/uClibc_stdio.h \
  /home/prokascc/Source/lib/include/wchar.h \
  /opt/buildroot-gcc342/bin/../lib/gcc/mipsel-linux-uclibc/3.4.2/include/stdarg.h \
  /home/prokascc/Source/lib/include/stdlib.h \
  /home/prokascc/Source/lib/include/bits/waitflags.h \
  /home/prokascc/Source/lib/include/bits/waitstatus.h \
  /home/prokascc/Source/lib/include/alloca.h \
  /home/prokascc/Source/lib/include/string.h \
  /home/prokascc/Source/lib/include/sys/poll.h \
  /home/prokascc/Source/lib/include/bits/poll.h \
  /home/prokascc/Source/lib/include/sys/mman.h \
  /home/prokascc/Source/lib/include/bits/mman.h \
  /home/prokascc/Source/lib/include/sys/socket.h \
  /home/prokascc/Source/lib/include/sys/uio.h \
  /home/prokascc/Source/lib/include/bits/uio.h \
  /home/prokascc/Source/lib/include/sys/time.h \
  /home/prokascc/Source/lib/include/sys/wait.h \
  /home/prokascc/Source/lib/include/sys/resource.h \
  /home/prokascc/Source/lib/include/bits/resource.h \
  /home/prokascc/Source/lib/include/termios.h \
  /home/prokascc/Source/lib/include/bits/termios.h \
  /home/prokascc/Source/lib/include/bits/uClibc_clk_tck.h \
  /home/prokascc/Source/lib/include/unistd.h \
  /home/prokascc/Source/lib/include/bits/posix_opt.h \
  /home/prokascc/Source/lib/include/bits/environments.h \
  /home/prokascc/Source/lib/include/bits/confname.h \
  /home/prokascc/Source/lib/include/bits/getopt.h \
  /home/prokascc/Source/lib/include/utime.h \
  /home/prokascc/Source/lib/include/sys/param.h \
  /home/prokascc/Source/lib/include/linux/param.h \
  /home/prokascc/Source/lib/include/asm/param.h \
  /home/prokascc/Source/lib/include/asm-generic/param.h \
    $(wildcard include/config/hz.h) \
  /home/prokascc/Source/lib/include/mntent.h \
  /home/prokascc/Source/lib/include/paths.h \
  /home/prokascc/Source/lib/include/sys/statfs.h \
  /home/prokascc/Source/lib/include/bits/statfs.h \
  /home/prokascc/Source/lib/include/pwd.h \
  /home/prokascc/Source/lib/include/grp.h \
  include/xatonum.h \
  include/unarchive.h \
    $(wildcard include/config/feature/tar/uname/gname.h) \
    $(wildcard include/config/dpkg.h) \
    $(wildcard include/config/dpkg/deb.h) \

archival/libunarchive/init_handle.o: $(deps_archival/libunarchive/init_handle.o)

$(deps_archival/libunarchive/init_handle.o):
