#
# Makefile for glib
#

MYROOT=$(CURDIR)
LIBSOUPROOT=$(MYROOT)
TOPDIR=$(CURDIR)/..
GLIBROOT=$(TOPDIR)/glib-2.35.1
XML2ROOT=$(TOPDIR)/libxml2-2.8.0

SUBDIRS = libsoup
export NM=$(CONFIG_CROSS_COMPILER_PATH)/mipsel-linux-nm
export TOPDIR GLIBROOT LIBSOUPROOT XML2ROOT

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
