#
# Makefile for alsa-utils
#

SUBDIRS=amixer

all:
	@for dir in $(SUBDIRS); \
	do \
		make -C $$dir all; \
	done;

clean:
	@for dir in $(SUBDIRS); \
	do \
		make -C $$dir clean; \
	done;

romfs:
	@for dir in $(SUBDIRS); \
	do \
		make -C $$dir romfs; \
	done;
