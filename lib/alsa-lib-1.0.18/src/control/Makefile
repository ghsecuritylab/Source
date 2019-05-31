#
# Makefile for alsa-lib
#

TOPDIR=../../../
include $(TOPDIR)Rules.mak

LIBCONTROL=libcontrol.a

CSRC=cards.c tlv.c namehint.c hcontrol.c control.c control_hw.c \
     setup.c control_symbols.c control_shm.c control_ext.c
OBJS=$(patsubst %.c,%.o, $(CSRC))

CFLAGS = -g -O2 -fPIC
CFLAGS += -D__mips__ -DHAVE_CONFIG_H -DPIC
CFLAGS += -I. -I../../include

all: $(LIBCONTROL)

$(LIBCONTROL) ar-target: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBCONTROL) $(OBJS)

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

clean:
	$(RM) *.[oa] *~ core
