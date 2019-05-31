#
# Makefile for alsa-utils
#

INSTALLDIR = $(ROOTDIR)/romfs

EXEC = aplay

CSRC = aplay.c
OBJS = $(patsubst %.c,%.o, $(CSRC))

CFLAGS += -DHAVE_CONFIG_H
CFLAGS += -I. -I../include -I$(ROOTDIR)/lib/include \
          -I$(ROOTDIR)/lib/alsa-lib-1.0.18/include

LDFLAGS += -L$(ROOTDIR)/lib/lib -lasound -lpthread -ldl -lm -lc

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
	cd $(INSTALLDIR)/usr/sbin
