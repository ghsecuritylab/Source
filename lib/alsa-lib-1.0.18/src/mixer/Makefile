#
# Makefile for alsa-lib
#

TOPDIR=../../../
include $(TOPDIR)Rules.mak

LIBMIXER=libmixer.a

CSRC=bag.c mixer.c simple.c simple_none.c simple_abst.c
OBJS=$(patsubst %.c,%.o, $(CSRC))

CFLAGS = -g -O2 -fPIC
CFLAGS += -D__mips__ -DHAVE_CONFIG_H -DPIC
CFLAGS += -I. -I../../include

all: $(LIBMIXER)

$(LIBMIXER) ar-target: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBMIXER) $(OBJS)

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

clean:
	$(RM) *.[oa] *~ core
