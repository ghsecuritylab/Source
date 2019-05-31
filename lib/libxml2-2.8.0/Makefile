#
# Makefile for libao
#

TOPDIR=..
include $(TOPDIR)/Rules.mak

MAJOR_VERSION=2
MINOR_VERSION=0
SUBLEVEL=28

LIBTARGET_SHARED=libxml2.so
LIBTARGET_SHARED_FULLNAME=$(LIBTARGET_SHARED).$(MAJOR_VERSION).$(MINOR_VERSION).$(SUBLEVEL)

CSRC=SAX.c entities.c encoding.c error.c parserInternals.c parser.c tree.c hash.c list.c xmlIO.c xmlmemory.c uri.c valid.c xlink.c HTMLparser.c HTMLtree.c debugXML.c xpath.c xpointer.c xinclude.c nanohttp.c nanoftp.c DOCBparser.c catalog.c globals.c threads.c c14n.c xmlstring.c xmlregexp.c xmlschemas.c xmlschemastypes.c xmlunicode.c xmlreader.c relaxng.c dict.c SAX2.c xmlwriter.c legacy.c chvalid.c pattern.c xmlsave.c xmlmodule.c schematron.c xzlib.c
OBJS=$(patsubst %.c,%.o, $(CSRC))

CFLAGS = -DHAVE_CONFIG_H -I. -I./include -I./include -D_REENTRANT -O2 -pedantic -W -Wformat -Wunused -Wimplicit -Wreturn-type -Wswitch -Wcomment -Wtrigraphs -Wformat -Wchar-subscripts -Wuninitialized -Wparentheses -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Waggregate-return -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs -Winline -Wredundant-decls -Wno-long-long -fPIC -DPIC
LDFLAGS = -shared  -fPIC -ldl -lm -O2

all shared: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -soname=$(LIBTARGET_SHARED).$(MAJOR_VERSION) -o $(LIBTARGET_SHARED_FULLNAME)
	$(STRIP) $(LIBTARGET_SHARED_FULLNAME)
	$(INSTALL) -d $(TOPDIR)/lib
	$(RM) $(TOPDIR)/lib/$(LIBTARGET_SHARED_FULLNAME) $(TOPDIR)/lib/$(LIBTARGET_SHARED).$(MAJOR_VERSION)
	$(INSTALL) -m 644 $(LIBTARGET_SHARED_FULLNAME) $(TOPDIR)/lib
	$(LN) -sf $(LIBTARGET_SHARED_FULLNAME) $(TOPDIR)/lib/$(LIBTARGET_SHARED)
	$(LN) -sf $(LIBTARGET_SHARED_FULLNAME) $(TOPDIR)/lib/$(LIBTARGET_SHARED).$(MAJOR_VERSION)

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.[oa] *~ core $(LIBTARGET_SHARED)* $(LIBTARGET_SHARED_FULLNAME)*
