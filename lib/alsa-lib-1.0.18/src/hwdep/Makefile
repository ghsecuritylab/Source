#
# Makefile for alsa-lib
#

TOPDIR=../../../
include $(TOPDIR)Rules.mak

LIBHWDEP=libhwdep.a

CSRC=hwdep.c hwdep_hw.c hwdep_symbols.c
OBJS=$(patsubst %.c,%.o, $(CSRC))

CFLAGS = -g -O2 -fPIC
CFLAGS += -D__mips__ -DHAVE_CONFIG_H -DPIC
CFLAGS += -I. -I../../include

all: $(LIBHWDEP)

$(LIBHWDEP) ar-target: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBHWDEP) $(OBJS)

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

clean:
	$(RM) *.[oa] *~ core
