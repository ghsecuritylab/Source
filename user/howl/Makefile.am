SUBDIRS = include src samples test docs etc

HOWLDIR = $(ROOTDIR)/user/howl
INSTALLDIR = $(ROOTDIR)/romfs

pkgconfigdir = $(prefix)/lib/pkgconfig
pkgconfig_DATA = howl.pc
EXTRA_DIST	= howl.sln

romfs:
	cp -f $(HOWLDIR)/src/mDNSResponder/.libs/mDNSResponder   $(INSTALLDIR)/sbin/
	cp -f $(HOWLDIR)/samples/console/publish/.libs/mDNSPublish $(INSTALLDIR)/sbin/
	cp -f $(HOWLDIR)/src/lib/mDNSResponder/.libs/libmDNSResponder.so.0 $(INSTALLDIR)/lib/
	cp -f $(HOWLDIR)/src/lib/howl/.libs/libhowl.so.0 $(INSTALLDIR)/lib/
	cp -f $(HOWLDIR)/src/lib/libs/libgcc_s.so.1  $(INSTALLDIR)/lib/

