#
# infosvr Makefile
#
# Copyright (C) 2013 ASUSTek Inc.
#

SHDIR = $(ROOTDIR)/user/shared

CFLAGS += -Wall \
	-Dlinux \
	-DASUS_NVRAM \
	-DNO_PARALLEL \
	-DPRNINFO

CFLAGS += -I. \
	-I$(SHDIR) \
	-I$(SHDIR)/include \
	-I$(ROOTDIR)/$(LINUXDIR)/include

LDFLAGS = -L$(SHDIR) -lshared

EXEC = infosvr

OBJS = infosvr.o \
	wlutils.o \
	common.o \
	packet.o \
	service.o \
	interface.o \
	storage.o

all: $(OBJS)
	$(CC) -o $(EXEC) $(OBJS) $(LDFLAGS)
	$(STRIP) $(EXEC)

clean:
	rm -f *.o *~ $(EXEC)

romfs:
	$(ROMFSINST) /usr/sbin/$(EXEC)
