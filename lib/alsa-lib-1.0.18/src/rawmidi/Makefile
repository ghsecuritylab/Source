#
# Makefile for alsa-lib
#

TOPDIR=../../../
include $(TOPDIR)Rules.mak

LIBRAWMIDI=librawmidi.a

CSRC=rawmidi.c rawmidi_hw.c rawmidi_symbols.c rawmidi_virt.c
OBJS=$(patsubst %.c,%.o, $(CSRC))

CFLAGS = -g -O2 -fPIC
CFLAGS += -D__mips__ -DHAVE_CONFIG_H -DPIC
CFLAGS += -I. -I../../include

all: $(LIBRAWMIDI)

$(LIBRAWMIDI) ar-target: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBRAWMIDI) $(OBJS)

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

clean:
	$(RM) *.[oa] *~ core
