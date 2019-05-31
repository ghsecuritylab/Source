#
# Makefile for alsa-lib
#

TOPDIR=../../../
include $(TOPDIR)Rules.mak

all:
	$(INSTALL) -d $(TOPDIR)lib/alsa
	$(INSTALL) -m 644 alsa.conf $(TOPDIR)lib/alsa
	$(INSTALL) -d $(TOPDIR)lib/alsa/cards
	$(INSTALL) -m 644 cards/aliases.conf $(TOPDIR)lib/alsa/cards
	$(INSTALL) -d $(TOPDIR)lib/alsa/pcm
	$(INSTALL) -m 644 pcm/default.conf $(TOPDIR)lib/alsa/pcm
	$(INSTALL) -m 644 pcm/dmix.conf $(TOPDIR)lib/alsa/pcm
	$(INSTALL) -m 644 pcm/dsnoop.conf $(TOPDIR)lib/alsa/pcm

clean:
	@echo do nothing ...
