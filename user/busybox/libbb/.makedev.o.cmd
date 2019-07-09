cmd_libbb/makedev.o := /opt/buildroot-gcc342/bin/mipsel-linux-uclibc-gcc -Wp,-MD,libbb/.makedev.o.d   -std=gnu99 -Iinclude -Ilibbb  -include include/autoconf.h -D_GNU_SOURCE -DNDEBUG  -D"BB_VER=KBUILD_STR(1.12.1)" -DBB_BT=AUTOCONF_TIMESTAMP -O2 -fomit-frame-pointer -pipe  -Dlinux -D__linux__ -Dunix -DEMBED -I/home/prokascc/Source/lib/include -I/home/prokascc/Source -DMODEL_NAME=\"RP-N12\" -DOOB_SSID_2G=\"ASUS_RPN12\" -DOOB_SSID_5G=\"ASUS_RPN12_5G\" -Wall -Wshadow -Wwrite-strings -Wundef -Wstrict-prototypes -Wunused -Wunused-parameter -Wmissing-prototypes -Wmissing-declarations -Wdeclaration-after-statement  -fno-builtin-strlen -finline-limit=0 -fomit-frame-pointer -ffunction-sections -fdata-sections -fno-guess-branch-probability -funsigned-char -static-libgcc -falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1 -Os -Dlinux    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(makedev)"  -D"KBUILD_MODNAME=KBUILD_STR(makedev)" -c -o libbb/makedev.o libbb/makedev.c

deps_libbb/makedev.o := \
  libbb/makedev.c \
  include/platform.h \
    $(wildcard include/config/werror.h) \
    $(wildcard include/config///.h) \
    $(wildcard include/config/nommu.h) \
    $(wildcard include/config//nommu.h) \
    $(wildcard include/config//mmu.h) \
  /home/prokascc/Source/lib/include/byteswap.h \
  /home/prokascc/Source/lib/include/bits/byteswap.h \
  /home/prokascc/Source/lib/include/endian.h \
    $(wildcard include/config/.h) \
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

libbb/makedev.o: $(deps_libbb/makedev.o)

$(deps_libbb/makedev.o):
