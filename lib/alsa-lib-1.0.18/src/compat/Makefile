#
# Makefile for alsa-lib
#

TOPDIR=../../../
include $(TOPDIR)Rules.mak

LIBCOMPAT=libcompat.a

CSRC=empty.c
OBJS=$(patsubst %.c,%.o, $(CSRC))

CFLAGS = -g -O2 -fPIC
CFLAGS += -D__mips__ -DHAVE_CONFIG_H -DPIC
CFLAGS += -I. -I../../include

all: $(LIBCOMPAT)

$(LIBCOMPAT) ar-target: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBCOMPAT) $(OBJS)

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

clean:
	$(RM) *.[oa] *~ core
