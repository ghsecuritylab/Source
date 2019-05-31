#
# Makefile for alsa-lib
#

TOPDIR=../../../
include $(TOPDIR)Rules.mak

LIBSEQ=libseq.a

CSRC=seq_hw.c seq.c seq_event.c seqmid.c \
     seq_midi_event.c seq_symbols.c seq_old.c
OBJS=$(patsubst %.c,%.o, $(CSRC))

CFLAGS = -g -O2 -fPIC
CFLAGS += -D__mips__ -DHAVE_CONFIG_H -DPIC
CFLAGS += -I. -I../../include

all: $(LIBSEQ)

$(LIBSEQ) ar-target: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBSEQ) $(OBJS)

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

clean:
	$(RM) *.[oa] *~ core
