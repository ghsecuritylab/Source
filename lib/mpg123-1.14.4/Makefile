#
# Makefile for glib
#

MYROOT=$(CURDIR)
MPG123ROOT=$(MYROOT)
TOPDIR=$(ROOTDIR)/lib

ASOUNDROOT=$(TOPDIR)/alsa-lib-1.0.18
RUNTIMELIB=$(TOPDIR)/lib

SUBDIRS = src
export NM=$(CONFIG_CROSS_COMPILER_PATH)/mipsel-linux-nm
export TOPDIR MPG123ROOT ASOUNDROOT RUNTIMELIB
BINFILE = src/.libs/mpg123
INSTALLDIR = $(ROOTDIR)/romfs

all shared install: checkbuild
	@for X in $(SUBDIRS) ; do \
		$(MAKE) -C $$X; done
	@cd $(SUBDIRS)/libmpg123/.libs; \
		$(STRIP) *.so*; \
		cp -af *.so* $(RUNTIMELIB); \
		cd $(MYROOT);
	@cd $(SUBDIRS)/.libs; \
		$(STRIP) mpg123; \
		cd $(MYROOT);

clean:
	@for X in $(SUBDIRS) ; do \
		$(MAKE) -C $$X clean; done
	rm -f checkfile

#romfs:
#	test -f $(BINFILE) || exit 1
#	cp -f $(BINFILE) $(INSTALLDIR)/usr/sbin/

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
