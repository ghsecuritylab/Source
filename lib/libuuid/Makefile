#
# Makefile for libao
#

TOPDIR=..
include $(TOPDIR)/Rules.mak

MAJOR_VERSION=1
MINOR_VERSION=3
SUBLEVEL=0

LIBTARGET_SHARED=libuuid.so
LIBTARGET_SHARED_FULLNAME=$(LIBTARGET_SHARED).$(MAJOR_VERSION).$(MINOR_VERSION).$(SUBLEVEL)

CSRC=clear.c compare.c copy.c gen_uuid.c isnull.c pack.c parse.c unpack.c unparse.c uuid_time.c
OBJS=$(patsubst %.c,%.o, $(CSRC))

CFLAGS = -O2 -fPIC -DPIC -include ./config.h
LDFLAGS = -shared -s -fPIC

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
