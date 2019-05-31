#
# Makefile for glib
#

MYROOT=$(CURDIR)
GST_PLUGFFMPEG_DIR=$(MYROOT)
TOPDIR=$(CURDIR)/..

GLIBROOT=$(GPLTOPDIR)/glib-2.35.1
LIBSOUPROOT=$(GPLTOPDIR)/libsoup-2.41.2
XML2ROOT=$(GPLTOPDIR)/libxml2-2.8.0
GST_DIR=$(TOPDIR)/gstreamer-0.10.36
LIBMMSDIR=$(GPLTOPDIR)/libmms-0.6.2
GST_PLUGBASE_DIR=$(TOPDIR)/gst-plugins-base-0.10.36
ASOUNDROOT=$(GPLTOPDIR)/alsa-lib-1.0.18
RUNTIMELIB=$(GPLTOPDIR)/lib
LIBFDKAAC=$(TOPDIR)/fdk-aac-0.1.3

GST_PLUG_INSTALL_DIR=$(RUNTIMELIB)/gstreamer-0.10
SUBDIRS = gst-libs ext
LIBPLUGS = ext/ffmpeg

export NM=$(CONFIG_CROSS_COMPILER_PATH)/mipsel-linux-nm
export TOPDIR GLIBROOT LIBSOUPROOT GST_DIR LIBMMSDIR GST_PLUGBASE_DIR XML2ROOT GST_PLUGFFMPEG_DIR ASOUNDROOT RUNTIMELIB LIBFDKAAC

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
