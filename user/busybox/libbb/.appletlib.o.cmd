cmd_libbb/appletlib.o := /opt/buildroot-gcc342/bin/mipsel-linux-uclibc-gcc -Wp,-MD,libbb/.appletlib.o.d   -std=gnu99 -Iinclude -Ilibbb  -include include/autoconf.h -D_GNU_SOURCE -DNDEBUG  -D"BB_VER=KBUILD_STR(1.12.1)" -DBB_BT=AUTOCONF_TIMESTAMP -O2 -fomit-frame-pointer -pipe  -Dlinux -D__linux__ -Dunix -DEMBED -I/home/prokascc/Source/lib/include -I/home/prokascc/Source -DMODEL_NAME=\"RP-N12\" -DOOB_SSID_2G=\"ASUS_RPN12\" -DOOB_SSID_5G=\"ASUS_RPN12_5G\" -Wall -Wshadow -Wwrite-strings -Wundef -Wstrict-prototypes -Wunused -Wunused-parameter -Wmissing-prototypes -Wmissing-declarations -Wdeclaration-after-statement  -fno-builtin-strlen -finline-limit=0 -fomit-frame-pointer -ffunction-sections -fdata-sections -fno-guess-branch-probability -funsigned-char -static-libgcc -falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1 -Os -Dlinux    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(appletlib)"  -D"KBUILD_MODNAME=KBUILD_STR(appletlib)" -c -o libbb/appletlib.o libbb/appletlib.c

deps_libbb/appletlib.o := \
  libbb/appletlib.c \
    $(wildcard include/config/show/usage.h) \
    $(wildcard include/config/feature/compress/usage.h) \
    $(wildcard include/config/feature/individual.h) \
    $(wildcard include/config/ture/individual.h) \
    $(wildcard include/config/locale/support.h) \
    $(wildcard include/config/test.h) \
    $(wildcard include/config/ture/suid.h) \
    $(wildcard include/config/feature/suid/config.h) \
    $(wildcard include/config/feature/suid.h) \
    $(wildcard include/config/feature/suid/config/quiet.h) \
    $(wildcard include/config/quiet.h) \
    $(wildcard include/config/feature/installer.h) \
    $(wildcard include/config/feature/autowidth.h) \
    $(wildcard include/config/build/libbusybox.h) \
  /home/prokascc/Source/lib/include/assert.h \
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
    $(wildcard include/config/.h) \
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
  include/busybox.h \
    $(wildcard include/config/feature/prefer/applets.h) \
    $(wildcard include/config/feature/shared/busybox.h) \
  include/libbb.h \
    $(wildcard include/config/selinux.h) \
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
    $(wildcard include/config/busybox/exec/path.h) \
    $(wildcard include/config/getopt/long.h) \
    $(wildcard include/config/feature/pidfile.h) \
    $(wildcard include/config/feature/syslog.h) \
    $(wildcard include/config/o.h) \
    $(wildcard include/config/ntf.h) \
    $(wildcard include/config/t.h) \
    $(wildcard include/config/l.h) \
    $(wildcard include/config/wn.h) \
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
  include/applets.h \
    $(wildcard include/config/feature/verbose/usage.h) \
    $(wildcard include/config/install/no/usr.h) \
    $(wildcard include/config/group.h) \
    $(wildcard include/config/user.h) \
    $(wildcard include/config/timex.h) \
    $(wildcard include/config/applet.h) \
    $(wildcard include/config/ing.h) \
    $(wildcard include/config/ename.h) \
    $(wildcard include/config/onfig.h) \
    $(wildcard include/config/h.h) \
    $(wildcard include/config/tl.h) \
    $(wildcard include/config/zip2.h) \
    $(wildcard include/config/p2.h) \
    $(wildcard include/config/v.h) \
    $(wildcard include/config/ttr.h) \
    $(wildcard include/config/on.h) \
    $(wildcard include/config/rp.h) \
    $(wildcard include/config/od.h) \
    $(wildcard include/config/asswd.h) \
    $(wildcard include/config/st.h) \
    $(wildcard include/config/oot.h) \
    $(wildcard include/config/um.h) \
    $(wildcard include/config/ar.h) \
    $(wildcard include/config/m.h) \
    $(wildcard include/config/applet/noexec.h) \
    $(wildcard include/config/nd.h) \
    $(wildcard include/config/ntab.h) \
    $(wildcard include/config/ptpw.h) \
    $(wildcard include/config/yhack.h) \
    $(wildcard include/config/e.h) \
    $(wildcard include/config/llocvt.h) \
    $(wildcard include/config/mod.h) \
    $(wildcard include/config/probe/small.h) \
    $(wildcard include/config/fsd.h) \
    $(wildcard include/config//dhcprelay.h) \
    $(wildcard include/config/f.h) \
    $(wildcard include/config/name.h) \
    $(wildcard include/config/sg.h) \
    $(wildcard include/config/2unix.h) \
    $(wildcard include/config/g.h) \
    $(wildcard include/config/g/deb.h) \
    $(wildcard include/config/pkmap.h) \
    $(wildcard include/config//dumpleases.h) \
    $(wildcard include/config/sck.h) \
    $(wildcard include/config/abel.h) \
    $(wildcard include/config/ture/grep/egrep/alias.h) \
    $(wildcard include/config/ct.h) \
    $(wildcard include/config/dir.h) \
    $(wildcard include/config/uidgid.h) \
    $(wildcard include/config/er/wake.h) \
    $(wildcard include/config/and.h) \
    $(wildcard include/config/r.h) \
    $(wildcard include/config/eidentd.h) \
    $(wildcard include/config/se.h) \
    $(wildcard include/config/et.h) \
    $(wildcard include/config/plash.h) \
    $(wildcard include/config/lush.h) \
    $(wildcard include/config/ormat.h) \
    $(wildcard include/config/sk.h) \
    $(wildcard include/config/chmail.h) \
    $(wildcard include/config/ture/grep/fgrep/alias.h) \
    $(wildcard include/config/dfs.h) \
    $(wildcard include/config/eramdisk.h) \
    $(wildcard include/config/k.h) \
    $(wildcard include/config/k/minix.h) \
    $(wildcard include/config/get.h) \
    $(wildcard include/config/put.h) \
    $(wildcard include/config/er.h) \
    $(wildcard include/config/enforce.h) \
    $(wildcard include/config/opt.h) \
    $(wildcard include/config/sebool.h) \
    $(wildcard include/config/ty.h) \
    $(wildcard include/config/p.h) \
    $(wildcard include/config/zip.h) \
    $(wildcard include/config/applet/oddname.h) \
    $(wildcard include/config/arm.h) \
    $(wildcard include/config/dump.h) \
    $(wildcard include/config/tid.h) \
    $(wildcard include/config/tname.h) \
    $(wildcard include/config/pd.h) \
    $(wildcard include/config/lock.h) \
    $(wildcard include/config/pdown.h) \
    $(wildcard include/config/nslave.h) \
    $(wildcard include/config/td.h) \
    $(wildcard include/config/tifyd.h) \
    $(wildcard include/config/tall.h) \
    $(wildcard include/config/feature/ip/address.h) \
    $(wildcard include/config/feature/ip/route.h) \
    $(wildcard include/config/feature/ip/link.h) \
    $(wildcard include/config/feature/ip/tunnel.h) \
    $(wildcard include/config/feature/ip/rule.h) \
    $(wildcard include/config/ddr.h) \
    $(wildcard include/config/alc.h) \
    $(wildcard include/config/rm.h) \
    $(wildcard include/config/s.h) \
    $(wildcard include/config/ink.h) \
    $(wildcard include/config/oute.h) \
    $(wildcard include/config/ule.h) \
    $(wildcard include/config/unnel.h) \
    $(wildcard include/config//mode.h) \
    $(wildcard include/config/lall.h) \
    $(wildcard include/config/lall5.h) \
    $(wildcard include/config/gd.h) \
    $(wildcard include/config/gth.h) \
    $(wildcard include/config/arch.h) \
    $(wildcard include/config/ture/initrd.h) \
    $(wildcard include/config/d/policy.h) \
    $(wildcard include/config/dfont.h) \
    $(wildcard include/config/dkmap.h) \
    $(wildcard include/config/ger.h) \
    $(wildcard include/config/in.h) \
    $(wildcard include/config/read.h) \
    $(wildcard include/config/etup.h) \
    $(wildcard include/config/zma.h) \
    $(wildcard include/config/edevs.h) \
    $(wildcard include/config/chpathcon.h) \
    $(wildcard include/config/sum.h) \
    $(wildcard include/config/rocom.h) \
    $(wildcard include/config/ir.h) \
    $(wildcard include/config/2fs.h) \
    $(wildcard include/config/ifo.h) \
    $(wildcard include/config/s/minix.h) \
    $(wildcard include/config/wap.h) \
    $(wildcard include/config/emp.h) \
    $(wildcard include/config/probe.h) \
    $(wildcard include/config/nt.h) \
    $(wildcard include/config/sktop.h) \
    $(wildcard include/config/ntpoint.h) \
    $(wildcard include/config/eif.h) \
    $(wildcard include/config/stat.h) \
    $(wildcard include/config/ter.h) \
    $(wildcard include/config/up.h) \
    $(wildcard include/config/ookup.h) \
    $(wildcard include/config/nvt.h) \
    $(wildcard include/config/swd.h) \
    $(wildcard include/config/ch.h) \
    $(wildcard include/config/ep.h) \
    $(wildcard include/config/of.h) \
    $(wildcard include/config/g6.h) \
    $(wildcard include/config/e/progress.h) \
    $(wildcard include/config/ot/root.h) \
    $(wildcard include/config/ll.h) \
    $(wildcard include/config/ntenv.h) \
    $(wildcard include/config/an.h) \
    $(wildcard include/config/dautorun.h) \
    $(wildcard include/config/te.h) \
    $(wildcard include/config/dahead.h) \
    $(wildcard include/config/dlink.h) \
    $(wildcard include/config/dprofile.h) \
    $(wildcard include/config/lpath.h) \
    $(wildcard include/config/ice.h) \
    $(wildcard include/config/ize.h) \
    $(wildcard include/config/torecon.h) \
    $(wildcard include/config/applet/nofork.h) \
    $(wildcard include/config/2cpio.h) \
    $(wildcard include/config/wake.h) \
    $(wildcard include/config//parts.h) \
    $(wildcard include/config/con.h) \
    $(wildcard include/config/level.h) \
    $(wildcard include/config/sv.h) \
    $(wildcard include/config/svdir.h) \
    $(wildcard include/config/ipt.h) \
    $(wildcard include/config/inuxenabled.h) \
    $(wildcard include/config/dmail.h) \
    $(wildcard include/config/tatus.h) \
    $(wildcard include/config/console.h) \
    $(wildcard include/config/files.h) \
    $(wildcard include/config/font.h) \
    $(wildcard include/config/keycodes.h) \
    $(wildcard include/config/logcons.h) \
    $(wildcard include/config/sid.h) \
    $(wildcard include/config/ture/sh/is/ash.h) \
    $(wildcard include/config/ture/sh/is/hush.h) \
    $(wildcard include/config/ture/sh/is/msh.h) \
    $(wildcard include/config/1sum.h) \
    $(wildcard include/config/wkey.h) \
    $(wildcard include/config/ttach.h) \
    $(wildcard include/config/tlimit.h) \
    $(wildcard include/config/it.h) \
    $(wildcard include/config/rt/stop/daemon.h) \
    $(wildcard include/config/ings.h) \
    $(wildcard include/config/y.h) \
    $(wildcard include/config/ogin.h) \
    $(wildcard include/config/ogd.h) \
    $(wildcard include/config/ponoff.h) \
    $(wildcard include/config/tch/root.h) \
    $(wildcard include/config/sysctl.h) \
    $(wildcard include/config/logd.h) \
    $(wildcard include/config/kset.h) \
    $(wildcard include/config/svd.h) \
    $(wildcard include/config/net.h) \
    $(wildcard include/config/netd.h) \
    $(wildcard include/config/feature/tftp/get.h) \
    $(wildcard include/config/feature/tftp/put.h) \
    $(wildcard include/config/ceroute.h) \
    $(wildcard include/config/size.h) \
    $(wildcard include/config/e2fs.h) \
    $(wildcard include/config//udhcpc.h) \
    $(wildcard include/config//udhcpd.h) \
    $(wildcard include/config/unt.h) \
    $(wildcard include/config/me.h) \
    $(wildcard include/config/ompress.h) \
    $(wildcard include/config/xpand.h) \
    $(wildcard include/config/q.h) \
    $(wildcard include/config/x2dos.h) \
    $(wildcard include/config/ip.h) \
    $(wildcard include/config/ime.h) \
    $(wildcard include/config/eep.h) \
    $(wildcard include/config/ecode.h) \
    $(wildcard include/config/ncode.h) \
    $(wildcard include/config/nfig.h) \
    $(wildcard include/config/ck.h) \
    $(wildcard include/config/chdog.h) \
    $(wildcard include/config/ami.h) \
    $(wildcard include/config/gs.h) \
  include/usage.h \
    $(wildcard include/config/ture/adduser/to/group.h) \
    $(wildcard include/config/ture/brctl/show.h) \
    $(wildcard include/config/ture/brctl/fancy.h) \
    $(wildcard include/config/ture/chcon/long/options.h) \
    $(wildcard include/config/ature/chcon/long/options.h) \
    $(wildcard include/config/ture/cpio/o.h) \
    $(wildcard include/config/ug/crond/option.h) \
    $(wildcard include/config/ture/date/isofmt.h) \
    $(wildcard include/config/ture/dd/ibs/obs.h) \
    $(wildcard include/config/ture/del/user/from/group.h) \
    $(wildcard include/config/fsd/fg/np.h) \
    $(wildcard include/config/feature/human/readable.h) \
    $(wildcard include/config/feature/df/inode.h) \
    $(wildcard include/config/ture/human/readable.h) \
    $(wildcard include/config/ture/df/inode.h) \
    $(wildcard include/config/ture/du/default/blocksize/1k.h) \
    $(wildcard include/config/ature/du/default/blocksize/1k.h) \
    $(wildcard include/config/opt/long.h) \
    $(wildcard include/config/topt/long.h) \
    $(wildcard include/config/ture/fancy/echo.h) \
    $(wildcard include/config/ture/eject/scsi.h) \
    $(wildcard include/config/ture/expand/long/options.h) \
    $(wildcard include/config/ature/expand/long/options.h) \
    $(wildcard include/config/feature/fdisk/blksize.h) \
    $(wildcard include/config/ture/fdisk/blksize.h) \
    $(wildcard include/config/ture/find/xdev.h) \
    $(wildcard include/config/ture/find/maxdepth.h) \
    $(wildcard include/config/ture/find/path.h) \
    $(wildcard include/config/ture/find/regex.h) \
    $(wildcard include/config/ture/find/type.h) \
    $(wildcard include/config/ture/find/perm.h) \
    $(wildcard include/config/ture/find/mtime.h) \
    $(wildcard include/config/ture/find/mmin.h) \
    $(wildcard include/config/ture/find/newer.h) \
    $(wildcard include/config/ture/find/inum.h) \
    $(wildcard include/config/ture/find/user.h) \
    $(wildcard include/config/ture/find/group.h) \
    $(wildcard include/config/ture/find/depth.h) \
    $(wildcard include/config/ture/find/size.h) \
    $(wildcard include/config/ture/find/print0.h) \
    $(wildcard include/config/ture/find/context.h) \
    $(wildcard include/config/ture/find/exec.h) \
    $(wildcard include/config/ture/find/prune.h) \
    $(wildcard include/config/ture/find/delete.h) \
    $(wildcard include/config/ture/find/paren.h) \
    $(wildcard include/config/ture/grep/context.h) \
    $(wildcard include/config/ture/wtmp.h) \
    $(wildcard include/config/ture/hdparm/hdio/getset/dma.h) \
    $(wildcard include/config/ture/hdparm/get/identity.h) \
    $(wildcard include/config/ture/hdparm/hdio/scan/hwif.h) \
    $(wildcard include/config/ture/hdparm/hdio/unregister/hwif.h) \
    $(wildcard include/config/ture/hdparm/hdio/drive/reset.h) \
    $(wildcard include/config/ture/hdparm/hdio/tristate/hwif.h) \
    $(wildcard include/config/ture/fancy/head.h) \
    $(wildcard include/config/ture/hexdump/reverse.h) \
    $(wildcard include/config/ture/httpd/setuid.h) \
    $(wildcard include/config/ture/httpd/basic/auth.h) \
    $(wildcard include/config/ture/httpd/auth/md5.h) \
    $(wildcard include/config/ture/ifconfig/status.h) \
    $(wildcard include/config/status.h) \
    $(wildcard include/config/ture/ipv6.h) \
    $(wildcard include/config/ture/ifconfig/slip.h) \
    $(wildcard include/config/slip.h) \
    $(wildcard include/config/ture/ifconfig/hw.h) \
    $(wildcard include/config/hw.h) \
    $(wildcard include/config/ture/hwib.h) \
    $(wildcard include/config/ture/ifconfig/memstart/ioaddr/irq.h) \
    $(wildcard include/config/memstart/ioaddr/irq.h) \
    $(wildcard include/config/ture/ifupdown/mapping.h) \
    $(wildcard include/config/ture/2/4/modules.h) \
    $(wildcard include/config/ture/insmod/load/map.h) \
    $(wildcard include/config/ture/ip/address.h) \
    $(wildcard include/config/ture/ip/route.h) \
    $(wildcard include/config/ture/ip/link.h) \
    $(wildcard include/config/ture/ip/tunnel.h) \
    $(wildcard include/config/ture/ip/rule.h) \
    $(wildcard include/config/ture/ipcalc/long/options.h) \
    $(wildcard include/config/ture/ipcalc/fancy.h) \
    $(wildcard include/config/ature/ipcalc/long/options.h) \
    $(wildcard include/config/ture/ls/timestamps.h) \
    $(wildcard include/config/ture/ls/filetypes.h) \
    $(wildcard include/config/ture/ls/followlinks.h) \
    $(wildcard include/config/ture/ls/recursive.h) \
    $(wildcard include/config/ture/ls/sortfiles.h) \
    $(wildcard include/config/ture/autowidth.h) \
    $(wildcard include/config/ture/ls/color.h) \
    $(wildcard include/config/feature/makedevs/leaf.h) \
    $(wildcard include/config/feature/makedevs/table.h) \
    $(wildcard include/config/ture/md5/sha1/sum/check.h) \
    $(wildcard include/config/ture/mdev/config.h) \
    $(wildcard include/config/ture/mdev/exec.h) \
    $(wildcard include/config/ture/mount/fake.h) \
    $(wildcard include/config/ture/mtab/support.h) \
    $(wildcard include/config/ture/mount/loop.h) \
    $(wildcard include/config/ture/mount/flags.h) \
    $(wildcard include/config/desktop.h) \
    $(wildcard include/config/nc/server.h) \
    $(wildcard include/config/nc/extra.h) \
    $(wildcard include/config/extra.h) \
    $(wildcard include/config/server.h) \
    $(wildcard include/config/ture/netstat/wide.h) \
    $(wildcard include/config/ture/netstat/prg.h) \
    $(wildcard include/config/feature/pidof/single.h) \
    $(wildcard include/config/feature/pidof/omit.h) \
    $(wildcard include/config/ture/pidof/single.h) \
    $(wildcard include/config/ture/pidof/omit.h) \
    $(wildcard include/config/feature/fancy/ping.h) \
    $(wildcard include/config/feature/ps/wide.h) \
    $(wildcard include/config/ture/ps/wide.h) \
    $(wildcard include/config/ture/readlink/follow.h) \
    $(wildcard include/config/ture/run/parts/fancy.h) \
    $(wildcard include/config/ture/setconsole/long/options.h) \
    $(wildcard include/config/ture/setfiles/check/option.h) \
    $(wildcard include/config/ture/last/fancy.h) \
    $(wildcard include/config/ture/fancy/sleep.h) \
    $(wildcard include/config/ature/fancy/sleep.h) \
    $(wildcard include/config/ture/sort/big.h) \
    $(wildcard include/config/ture/start/stop/daemon/fancy.h) \
    $(wildcard include/config/ture/stat/format.h) \
    $(wildcard include/config/ture/swapon/pri.h) \
    $(wildcard include/config/ture/rotate/logfile.h) \
    $(wildcard include/config/ture/remote/log.h) \
    $(wildcard include/config/ture/syslogd/dup.h) \
    $(wildcard include/config/ture/ipc/syslog.h) \
    $(wildcard include/config/ture/fancy/tail.h) \
    $(wildcard include/config/ture/tar/create.h) \
    $(wildcard include/config/ture/seamless/gz.h) \
    $(wildcard include/config/ture/seamless/bz2.h) \
    $(wildcard include/config/ture/seamless/lzma.h) \
    $(wildcard include/config/ture/seamless/z.h) \
    $(wildcard include/config/ture/tar/from.h) \
    $(wildcard include/config/feature/telnet/autologin.h) \
    $(wildcard include/config/ature/telnetd/standalone.h) \
    $(wildcard include/config/ture/telnetd/standalone.h) \
    $(wildcard include/config/ture/tftp/get.h) \
    $(wildcard include/config/ture/tftp/put.h) \
    $(wildcard include/config/ture/tftp/blocksize.h) \
    $(wildcard include/config/lude/susv2.h) \
    $(wildcard include/config/ture/udhcp/port.h) \
    $(wildcard include/config/dhcpc/default/script.h) \
    $(wildcard include/config/ture/udhcpc/arping.h) \
    $(wildcard include/config/ture/umount/all.h) \
    $(wildcard include/config/ture/unexpand/long/options.h) \
    $(wildcard include/config/ature/unexpand/long/options.h) \
    $(wildcard include/config/ture/vi/colon.h) \
    $(wildcard include/config/ture/vi/readonly.h) \
    $(wildcard include/config/ture/xargs/support/confirmation.h) \
    $(wildcard include/config/ture/xargs/support/zero/term.h) \
    $(wildcard include/config/ture/xargs/support/termopt.h) \
  include/applet_tables.h \

libbb/appletlib.o: $(deps_libbb/appletlib.o)

$(deps_libbb/appletlib.o):
