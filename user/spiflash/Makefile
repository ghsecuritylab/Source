# Linux NVRAM Makefile
#
# Copyright 2004, ASUSTek Inc.
# All Rights Reserved.
# 
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: Makefile,v 1.1 2007/06/08 10:22:42 arthur Exp $
#

#.PHONY: all clean tags dep romfs

SHDIR = $(ROOTDIR)/user/shared

CFLAGS	+= -Wall -I. -I$(ROOTDIR)/$(LINUXDIR)/include 
CFLAGS	+= -I$(SHDIR)/include
CFLAGS	+= -s
LDFLAGS	+= -L.

SOURCES = spiflash.c
OBJS = spiflash.o
TARGETS = spiflash

all: $(TARGETS)

spiflash: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)
	$(STRIP) $@

clean:
	rm -f $(TARGETS) *.o *.so

romfs:
	$(ROMFSINST) /usr/sbin/spiflash

