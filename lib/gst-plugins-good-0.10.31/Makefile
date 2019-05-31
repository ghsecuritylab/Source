#
# Makefile for glib
#

MYROOT=$(CURDIR)
GST_PLUGGOOD_DIR=$(MYROOT)
TOPDIR=$(CURDIR)/..

GLIBROOT=$(GPLTOPDIR)/glib-2.35.1
LIBSOUPROOT=$(GPLTOPDIR)/libsoup-2.41.2
XML2ROOT=$(GPLTOPDIR)/libxml2-2.8.0
GST_DIR=$(TOPDIR)/gstreamer-0.10.36
GST_PLUGBASE_DIR=$(TOPDIR)/gst-plugins-base-0.10.36
ASOUNDROOT=$(GPLTOPDIR)/alsa-lib-1.0.18
RUNTIMELIB=$(GPLTOPDIR)/lib

GST_PLUG_INSTALL_DIR=$(RUNTIMELIB)/gstreamer-0.10
SUBDIRS = ext gst
LIBPLUGS = ext/soup gst/audioparsers gst/id3demux gst/asfdemux gst/wavparse gst/apetag gst/isomp4

export NM=$(CONFIG_CROSS_COMPILER_PATH)/mipsel-linux-nm
export TOPDIR GLIBROOT LIBSOUPROOT GST_DIR GST_PLUGBASE_DIR XML2ROOT GST_PLUGGOOD_DIR ASOUNDROOT RUNTIMELIB

all shared install: checkbuild
	@for X in $(SUBDIRS) ; do \
		$(MAKE) -C $$X; done
	@mkdir -p $(GST_PLUG_INSTALL_DIR)
	@for X in $(LIBPLUGS) ; do \
		cd $$X/.libs; \
		$(STRIP) *.so*; \
		cp -af *.so* $(GST_PLUG_INSTALL_DIR);\
		cd $(MYROOT); done

clean:
	@for X in $(SUBDIRS) ; do \
		$(MAKE) -C $$X clean; done
	@cd $(MYROOT)
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
