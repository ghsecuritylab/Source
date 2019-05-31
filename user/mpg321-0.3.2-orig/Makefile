#
# Makefile for mpg321
#

INSTALLDIR = $(ROOTDIR)/romfs

EXEC = mpg321

CSRC = mpg321.c mad.c playlist.c network.c remote.c ao.c \
       options.c auth.c scrobbler.c fft.c fbuffer.c volume.c
OBJS = $(patsubst %.c,%.o, $(CSRC))

CFLAGS += -DHAVE_CONFIG_H -DHAVE_ALSA
CFLAGS += -I. -I$(ROOTDIR)/lib/include \
          -I$(ROOTDIR)/lib/libid3tag-0.15.1b \
          -I$(ROOTDIR)/lib/libmad-0.15.1b \
          -I$(ROOTDIR)/lib/alsa-lib-1.0.18/include \
          -I$(ROOTDIR)/lib/libao-1.1.0/include

LDFLAGS += -L$(ROOTDIR)/lib/lib -lasound -lpthread -lid3tag -lz -lmad -lao -ldl -lm -lc

all: $(OBJS)
	$(CC) -o $(EXEC) $(OBJS) $(LDFLAGS)
	$(STRIP) $(EXEC)

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

clean:
	rm -f *.o $(EXEC) .sgbuilt_user

romfs:
	cp -f $(EXEC) $(INSTALLDIR)/usr/sbin/
	cd $(INSTALLDIR)/usr/sbin && ln -sf $(EXEC) mpg123
