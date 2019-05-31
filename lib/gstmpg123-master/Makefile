MYROOT=$(CURDIR)
TOPDIR=$(ROOTDIR)/lib

GLIBROOT=$(TOPDIR)/glib-2.35.1
XML2ROOT=$(TOPDIR)/libxml2-2.8.0
GST_DIR=$(TOPDIR)/gstreamer-0.10.36
GST_PLUGBASE_DIR=$(TOPDIR)/gst-plugins-base-0.10.36
MPG123_DIR=$(TOPDIR)/mpg123-1.14.4
RUNTIMELIB=$(TOPDIR)/lib

GST_PLUG_INSTALL_DIR=$(RUNTIMELIB)/gstreamer-0.10
SUBDIRS = src
LIBPLUGS = src

export NM=$(CONFIG_CROSS_COMPILER_PATH)/mipsel-linux-nm
export GLIBROOT GST_DIR XML2ROOT GST_PLUGBASE_DIR MPG123_DIR RUNTIMELIB

all shared install:
	@for X in $(SUBDIRS) ; do \
		$(MAKE) -C $$X; done
	@mkdir -p $(GST_PLUG_INSTALL_DIR)
	@for X in $(LIBPLUGS) ; do \
		cd $$X; \
		$(STRIP) *.so*; \
		cp -af *.so* $(GST_PLUG_INSTALL_DIR); \
		cd $(MYROOT); done

clean:
	@for X in $(SUBDIRS) ; do \
		$(MAKE) -C $$X clean; done
	@cd $(MYROOT)

.PHONY: dummy all clean
