#
# Makefile for glib
#

MYROOT=$(CURDIR)
GST_DIR=$(MYROOT)
TOPDIR=$(CURDIR)/..

GLIBROOT=$(GPLTOPDIR)/glib-2.35.1
RUNTIMELIB=$(GPLTOPDIR)/lib

GST_PLUG_INSTALL_DIR=$(RUNTIMELIB)/gstreamer-0.10
SUBDIRS = gst libs plugins
LIBDIRS = gst libs/gst/base libs/gst/controller libs/gst/dataprotocol
LIBPLUGS = plugins/elements plugins/indexers
# only install libgstreamer-0.10.so libgstbase-0.10.so

export NM=$(CONFIG_CROSS_COMPILER_PATH)/mipsel-linux-nm
export TOPDIR GLIBROOT GST_DIR

all shared install: checkbuild
	@for X in $(SUBDIRS) ; do \
		$(MAKE) -C $$X; done
	@for X in $(LIBDIRS) ; do \
		cd $$X/.libs; \
		$(STRIP) *.so*; \
		cp -af *.so* $(TOPDIR)/lib; \
		cd $(MYROOT); done
	@mkdir -p $(GST_PLUG_INSTALL_DIR)
	@for X in $(LIBPLUGS) ; do \
		cd $$X/.libs; \
		$(STRIP) *.so*; \
		cp -af *.so* $(GST_PLUG_INSTALL_DIR);\
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
