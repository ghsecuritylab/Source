#
# Makefile for libao
#

SUBDIRS=alsa

all:
	@for dir in $(SUBDIRS); \
	do \
		make -C $$dir all; \
	done;

shared:
	@for dir in $(SUBDIRS); \
	do \
		make -C $$dir shared; \
	done;

clean:
	@for dir in $(SUBDIRS); \
	do \
		make -C $$dir clean; \
	done;
