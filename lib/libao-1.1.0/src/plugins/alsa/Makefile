#
# Makefile for libao
#

TOPDIR=../../../../
include $(TOPDIR)Rules.mak

LIBALSA=libalsa.a
LIBALSA_SHARED=libalsa.so

CSRC=ao_alsa.c
OBJS=$(patsubst %.c,%.o, $(CSRC))

CFLAGS = -g -O2 -fPIC
CFLAGS += -I../../../include -I../../../include/ao \
          -I$(TOPDIR)alsa-lib-1.0.18/include
CFLAGS += -DAO_BUILDING_LIBAO -DPIC
CFLAGS += -DPACKAGE_NAME=\"libao\" -DPACKAGE_TARNAME=\"libao\" -DPACKAGE_VERSION=\"1.1.0\" -DPACKAGE_STRING=\"libao\ 1.1.0\" -DPACKAGE_BUGREPORT=\"monty@xiph.org\" -DPACKAGE_URL=\"\" -DPACKAGE=\"libao\" -DVERSION=\"1.1.0\" -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_DLFCN_H=1 -DLT_OBJDIR=\".libs/\" -DHAVE_DLFCN_H=1 -DHAVE_DLOPEN=1 -DHAVE_LIBPTHREAD=1 -DDLOPEN_FLAG=\(RTLD_NOW\ \|\ RTLD_GLOBAL\) -DSHARED_LIB_EXT=\".so\" -DSIZEOF_SHORT=2 -DSIZEOF_INT=4 -DSIZEOF_LONG=4 -DHAVE_SYS_SOUNDCARD_H=1

all: $(LIBALSA)

$(LIBALSA) ar-target: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBALSA) $(OBJS)
	$(INSTALL) -d $(TOPDIR)lib/ao/plugins-4
	$(RM) $(TOPDIR)lib/ao/plugins-4/$(LIBALSA)
	$(INSTALL) -m 644 $(LIBALSA) $(TOPDIR)lib/ao/plugins-4

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

shared: all
	$(LD) $(LDFLAGS) -soname=$(LIBALSA_SHARED) \
		-o $(LIBALSA_SHARED) --whole-archive $(LIBALSA) \
		--no-whole-archive $(TOPDIR)libc/misc/internals/interp.o \
		-L$(TOPDIR)lib -lc $(LDADD_LIBFLOAT) $(LIBGCC) \
		-lasound -lpthread -ldl -lao;
	$(INSTALL) -d $(TOPDIR)lib/ao/plugins-4
	$(RM) $(TOPDIR)lib/ao/plugins-4/$(LIBALSA_SHARED)
	$(INSTALL) -m 644 $(LIBALSA_SHARED) $(TOPDIR)lib/ao/plugins-4

clean:
	$(RM) *.[oa] *~ core $(LIBALSA_SHARED)*
