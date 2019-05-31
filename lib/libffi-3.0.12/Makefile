#
# Makefile for glib
#

MYROOT=$(CURDIR)
LIBFFIROOT=$(MYROOT)
TOPDIR=$(CURDIR)/..
CCAS=$(CC)

SUBDIRS = mipsel-unknown-linux-gnu
export NM=$(CONFIG_CROSS_COMPILER_PATH)/mipsel-linux-nm
export TOPDIR LIBFFIROOT CCAS

all shared install:
	@for X in $(SUBDIRS) ; do \
		$(MAKE) -C $$X; done
	@for X in $(SUBDIRS) ; do \
		cd $$X/.libs; \
		$(STRIP) *.so*; \
		cp -af *.so* $(TOPDIR)/lib; \
		cd $(MYROOT); done

clean:
	@for X in $(SUBDIRS) ; do \
		$(MAKE) -C $$X clean; done

.PHONY: dummy all clean
