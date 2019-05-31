#
# Makefile for glib
#

MYROOT=$(CURDIR)
GLIBROOT=$(MYROOT)
TOPDIR=$(CURDIR)/..
RUNTIMELIB=$(TOPDIR)/lib
LIBFFIROOT=$(TOPDIR)/libffi-3.0.12/mipsel-unknown-linux-gnu/include

SUBDIRS = glib gmodule gthread gobject gio 
export NM=$(CONFIG_CROSS_COMPILER_PATH)/mipsel-linux-nm
export TOPDIR GLIBROOT RUNTIMELIB LIBFFIROOT

all shared install:
	@for X in $(SUBDIRS) ; do \
		$(MAKE) -C $$X; done
	@for X in $(SUBDIRS) ; do \
		cd $$X/.libs; \
		$(STRIP) *2.0.so*; \
		cp -af *2.0.so* $(TOPDIR)/lib; \
		cd $(MYROOT); done

clean:
	@for X in $(SUBDIRS) ; do \
		$(MAKE) -C $$X clean; done

.PHONY: dummy all clean
