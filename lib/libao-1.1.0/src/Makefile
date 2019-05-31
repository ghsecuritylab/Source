#
# Makefile for libao
#

TOPDIR=../../
include $(TOPDIR)Rules.mak

MAJOR_VERSION=4
MINOR_VERSION=0
SUBLEVEL=0

LIBAO=libao.a
LIBAO_SHARED=libao.so
LIBAO_SHARED_FULLNAME=libao.so.$(MAJOR_VERSION).$(MINOR_VERSION).$(SUBLEVEL)

CSRC=audio_out.c config.c ao_null.c ao_wav.c ao_au.c ao_raw.c ao_aixs.c 
OBJS=$(patsubst %.c,%.o, $(CSRC))

CFLAGS = -g -O2 -fPIC
CFLAGS += -I../include -I../include/ao
CFLAGS += -DAO_BUILDING_LIBAO -DPIC
CFLAGS += -DPACKAGE_NAME=\"libao\" -DPACKAGE_TARNAME=\"libao\" -DPACKAGE_VERSION=\"1.1.0\" -DPACKAGE_STRING=\"libao\ 1.1.0\" -DPACKAGE_BUGREPORT=\"monty@xiph.org\" -DPACKAGE_URL=\"\" -DPACKAGE=\"libao\" -DVERSION=\"1.1.0\" -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_DLFCN_H=1 -DLT_OBJDIR=\".libs/\" -DHAVE_DLFCN_H=1 -DHAVE_DLOPEN=1 -DHAVE_LIBPTHREAD=1 -DDLOPEN_FLAG=\(RTLD_NOW\ \|\ RTLD_GLOBAL\) -DSHARED_LIB_EXT=\".so\" -DSIZEOF_SHORT=2 -DSIZEOF_INT=4 -DSIZEOF_LONG=4 -DHAVE_SYS_SOUNDCARD_H=1
CFLAGS += -DAO_PLUGIN_PATH=\"/lib/ao/plugins-4\"

all: $(LIBAO)

$(LIBAO) ar-target: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBAO) $(OBJS)
	$(INSTALL) -d $(TOPDIR)lib
	$(RM) $(TOPDIR)lib/$(LIBAO)
	$(INSTALL) -m 644 $(LIBAO) $(TOPDIR)lib/
	@make -C plugins all

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

shared: all
	$(LD) $(LDFLAGS) -soname=$(LIBAO_SHARED).$(MAJOR_VERSION) \
		-o $(LIBAO_SHARED_FULLNAME) --whole-archive $(LIBAO) \
		--no-whole-archive $(TOPDIR)libc/misc/internals/interp.o \
		-L$(TOPDIR)lib -lc $(LDADD_LIBFLOAT) $(LIBGCC) \
		-lpthread -ldl;
	$(INSTALL) -d $(TOPDIR)lib
	$(RM) $(TOPDIR)lib/$(LIBAO_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBAO_SHARED).$(MAJOR_VERSION)
	$(INSTALL) -m 644 $(LIBAO_SHARED_FULLNAME) $(TOPDIR)lib
	$(LN) -sf $(LIBAO_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBAO_SHARED)
	$(LN) -sf $(LIBAO_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBAO_SHARED).$(MAJOR_VERSION)
	@make -C plugins shared

clean:
	$(RM) *.[oa] *~ core $(LIBAO_SHARED)* $(LIBAO_SHARED_FULLNAME)*
	@make -C plugins clean
