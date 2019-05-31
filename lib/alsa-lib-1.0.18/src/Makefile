#
# Makefile for alsa-lib
#

TOPDIR=../../
include $(TOPDIR)Rules.mak

MAJOR_VERSION=2
MINOR_VERSION=0
SUBLEVEL=0

LIBASOUND=libasound.a
LIBASOUND_SHARED=libasound.so
LIBASOUND_SHARED_FULLNAME=libasound.so.$(MAJOR_VERSION).$(MINOR_VERSION).$(SUBLEVEL)

SUBDIRS=control mixer pcm timer rawmidi hwdep seq alisp compat
EXT_LIBA_DIR=$(patsubst %,%/, $(SUBDIRS))
EXT_LIBA_FILE=$(patsubst %,lib%.a, $(SUBDIRS))
EXT_LIBA=$(join $(EXT_LIBA_DIR), $(EXT_LIBA_FILE))

CSRC=conf.c confmisc.c input.c output.c async.c error.c \
     dlmisc.c socket.c shmarea.c userfile.c names.c
OBJS=$(patsubst %.c,%.o, $(CSRC))

CFLAGS = -g -O2 -fPIC
CFLAGS += -D__mips__ -DHAVE_CONFIG_H -DPIC
CFLAGS += -I. -I../include

all: $(LIBASOUND)

$(LIBASOUND) ar-target: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBASOUND) $(OBJS)
	$(INSTALL) -d $(TOPDIR)lib
	$(RM) $(TOPDIR)lib/$(LIBASOUND)
	$(INSTALL) -m 644 $(LIBASOUND) $(TOPDIR)lib/
	@for dir in $(SUBDIRS) conf; \
	do \
		make -C $$dir all; \
	done;

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

shared: all
	$(LD) $(LDFLAGS) -soname=$(LIBASOUND_SHARED).$(MAJOR_VERSION) \
		-o $(LIBASOUND_SHARED_FULLNAME) \
		--whole-archive $(LIBASOUND) $(EXT_LIBA)\
		--no-whole-archive $(TOPDIR)libc/misc/internals/interp.o \
		-L$(TOPDIR)lib -lc $(LDADD_LIBFLOAT) $(LIBGCC) \
		-lm -lpthread -ldl -lrt;
	$(INSTALL) -d $(TOPDIR)lib
	$(RM) $(TOPDIR)lib/$(LIBASOUND_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBASOUND_SHARED).$(MAJOR_VERSION)
	$(INSTALL) -m 644 $(LIBASOUND_SHARED_FULLNAME) $(TOPDIR)lib
	$(LN) -sf $(LIBASOUND_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBASOUND_SHARED)
	$(LN) -sf $(LIBASOUND_SHARED_FULLNAME) $(TOPDIR)lib/$(LIBASOUND_SHARED).$(MAJOR_VERSION)

clean:
	$(RM) *.[oa] *~ core $(LIBASOUND_SHARED)* $(LIBASOUND_SHARED_FULLNAME)*
	@for dir in $(SUBDIRS) conf; \
	do \
		make -C $$dir clean; \
	done;
