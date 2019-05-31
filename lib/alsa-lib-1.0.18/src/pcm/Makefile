#
# Makefile for alsa-lib
#

TOPDIR=../../../
include $(TOPDIR)Rules.mak

LIBPCM=libpcm.a

CSRC=atomic.c mask.c interval.c pcm.c \
     pcm_params.c pcm_simple.c pcm_hw.c pcm_misc.c pcm_mmap.c \
     pcm_symbols.c pcm_generic.c pcm_plugin.c pcm_copy.c \
     pcm_linear.c pcm_route.c pcm_mulaw.c pcm_alaw.c pcm_adpcm.c \
     pcm_rate.c pcm_rate_linear.c pcm_plug.c pcm_multi.c pcm_shm.c \
     pcm_file.c pcm_null.c pcm_empty.c pcm_share.c pcm_meter.c \
     pcm_hooks.c pcm_lfloat.c pcm_ladspa.c pcm_dmix.c pcm_dshare.c \
     pcm_dsnoop.c pcm_direct.c pcm_asym.c pcm_iec958.c \
     pcm_softvol.c pcm_extplug.c pcm_ioplug.c pcm_mmap_emul.c
OBJS=$(patsubst %.c,%.o, $(CSRC))

CFLAGS = -g -O2 -fPIC
CFLAGS += -D__mips__ -DHAVE_CONFIG_H -DPIC
CFLAGS += -I. -I../../include

all: $(LIBPCM)

$(LIBPCM) ar-target: $(OBJS)
	$(AR) $(ARFLAGS) $(LIBPCM) $(OBJS)

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

clean:
	$(RM) *.[oa] *~ core
