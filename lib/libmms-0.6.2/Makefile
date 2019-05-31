#
# Makefile for libmms
#

TOPDIR=../
include $(TOPDIR)Rules.mak

GLIBROOT=$(GPLTOPDIR)/glib-2.35.1
RUNTIMELIB=$(GPLTOPDIR)/lib

MAJOR_VERSION=0
MINOR_VERSION=0
SUBLEVEL=2

LIBMAD=libmms.a
LIBMAD_SHARED=libmms.so
LIBMAD_SHARED_FULLNAME=libmms.so.$(MAJOR_VERSION).$(MINOR_VERSION).$(SUBLEVEL)

CSRC=mms.c mmsh.c mmsx.c uri.c
OBJS=$(patsubst %.c,%.o, $(CSRC))

CFLAGS = -Wall -g -O -fPIC -DPIC
CFLAGS += -DHAVE_CONFIG_H -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
CFLAGS += -I. -I$(GLIBROOT)/include
LFLAGS = -L$(RUNTIMELIB) -lglib-2.0 -lc

all: $(LIBMAD)

$(LIBMAD) ar-target: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBMAD) $(OBJS)
	$(INSTALL) -d $(TOPDIR)lib
	$(RM) $(TOPDIR)lib/$(LIBMAD)
	$(INSTALL) -m 644 $(LIBMAD) $(TOPDIR)lib/

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

shared: all
	$(LD) $(LDFLAGS) -soname=$(LIBMAD_SHARED).$(MAJOR_VERSION) \
		-o $(LIBMAD_SHARED_FULLNAME) --whole-archive $(LIBMAD) \
		--no-whole-archive $(TOPDIR)libc/misc/internals/interp.o \
		$(LFLAGS) $(LDADD_LIBFLOAT) $(LIBGCC);
	$(INSTALL) -d $(TOPDIR)lib
	$(RM) $(TOPDIR)lib/$(LIBMAD_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBMAD_SHARED).$(MAJOR_VERSION)
	$(INSTALL) -m 644 $(LIBMAD_SHARED_FULLNAME) $(TOPDIR)lib
	$(LN) -sf $(LIBMAD_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBMAD_SHARED)
	$(LN) -sf $(LIBMAD_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBMAD_SHARED).$(MAJOR_VERSION)

clean:
	$(RM) *.[oa] *~ core $(LIBMAD_SHARED)* $(LIBMAD_SHARED_FULLNAME)*
