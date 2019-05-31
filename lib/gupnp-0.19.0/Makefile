#
# Makefile for glib
#

MYROOT=$(CURDIR)
GUPNPROOT=$(MYROOT)
TOPDIR=$(CURDIR)/..

GLIBROOT=$(TOPDIR)/glib-2.35.1
LIBSOUPROOT=$(TOPDIR)/libsoup-2.41.2
GSSDPROOT=$(TOPDIR)/gssdp-0.13.0
XML2ROOT=$(TOPDIR)/libxml2-2.8.0
UUIDROOT=$(TOPDIR)/libuuid

SUBDIRS = libgupnp
export NM=$(CONFIG_CROSS_COMPILER_PATH)/mipsel-linux-nm
export TOPDIR GLIBROOT LIBSOUPROOT GSSDPROOT GUPNPROOT XML2ROOT UUIDROOT

all shared install: checkbuild
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
	rm -f checkfile

ifeq ($(wildcard ./checkfile),)
checkbuild:
	@touch checkfile;
	@touch config.status;
	@for X in $(SUBDIRS) ; do \
		find ./$$X -name Makefile | xargs touch ; done
else
checkbuild:
endif

.PHONY: dummy all clean checkbuild
