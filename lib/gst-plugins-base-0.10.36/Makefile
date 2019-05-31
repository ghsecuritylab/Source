#
# Makefile for glib
#

MYROOT=$(CURDIR)
GST_PLUGBASE_DIR=$(MYROOT)
TOPDIR=$(CURDIR)/..

GLIBROOT=$(GPLTOPDIR)/glib-2.35.1
XML2ROOT=$(GPLTOPDIR)/libxml2-2.8.0
GST_DIR=$(TOPDIR)/gstreamer-0.10.36
ASOUNDROOT=$(GPLTOPDIR)/alsa-lib-1.0.18
RUNTIMELIB=$(GPLTOPDIR)/lib

GST_PLUG_INSTALL_DIR=$(RUNTIMELIB)/gstreamer-0.10
SUBDIRS = gst-libs gst ext #sys common
LIBDIRS = gst-libs/gst/interfaces gst-libs/gst/pbutils gst-libs/gst/audio gst-libs/gst/tag gst-libs/gst/rtp gst-libs/gst/rtsp gst-libs/gst/riff gst-libs/gst/sdp gst-libs/gst/app
LIBPLUGS = ext/alsa gst/audioconvert gst/typefind gst/app
SPECIAL_SO = gst/playback/.libs/libgstdecodebin2.so

export NM=$(CONFIG_CROSS_COMPILER_PATH)/mipsel-linux-nm
export TOPDIR GLIBROOT GST_DIR XML2ROOT GST_PLUGBASE_DIR ASOUNDROOT RUNTIMELIB

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
	@for X in $(SPECIAL_SO) ; do \
		$(STRIP) $$X; \
		cp -af $$X $(GST_PLUG_INSTALL_DIR);\
		done

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
