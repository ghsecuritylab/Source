cmd_libbb/lineedit_ptr_hack.o := /opt/buildroot-gcc342/bin/mipsel-linux-uclibc-gcc -Wp,-MD,libbb/.lineedit_ptr_hack.o.d   -std=gnu99 -Iinclude -Ilibbb  -include include/autoconf.h -D_GNU_SOURCE -DNDEBUG  -D"BB_VER=KBUILD_STR(1.12.1)" -DBB_BT=AUTOCONF_TIMESTAMP -O2 -fomit-frame-pointer -pipe  -Dlinux -D__linux__ -Dunix -DEMBED -I/home/prokascc/Source/lib/include -I/home/prokascc/Source -DMODEL_NAME=\"RP-N12\" -DOOB_SSID_2G=\"ASUS_RPN12\" -DOOB_SSID_5G=\"ASUS_RPN12_5G\" -Wall -Wshadow -Wwrite-strings -Wundef -Wstrict-prototypes -Wunused -Wunused-parameter -Wmissing-prototypes -Wmissing-declarations -Wdeclaration-after-statement  -fno-builtin-strlen -finline-limit=0 -fomit-frame-pointer -ffunction-sections -fdata-sections -fno-guess-branch-probability -funsigned-char -static-libgcc -falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1 -Os -Dlinux    -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(lineedit_ptr_hack)"  -D"KBUILD_MODNAME=KBUILD_STR(lineedit_ptr_hack)" -c -o libbb/lineedit_ptr_hack.o libbb/lineedit_ptr_hack.c

deps_libbb/lineedit_ptr_hack.o := \
  libbb/lineedit_ptr_hack.c \

libbb/lineedit_ptr_hack.o: $(deps_libbb/lineedit_ptr_hack.o)

$(deps_libbb/lineedit_ptr_hack.o):