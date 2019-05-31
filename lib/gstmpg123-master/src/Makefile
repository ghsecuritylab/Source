LIBTARGET_SHARED=libgstmpg123.so

CSRC=gstmpg123-0_10.c
OBJS=$(patsubst %.c,%.o, $(CSRC))

CFLAGS = -O2 -std=c99 -fPIC -DPIC -I./ -I$(GST_DIR) -I$(GST_DIR)/libs -I$(GLIBROOT)/include -I$(GST_PLUGBASE_DIR)/gst-libs -I$(XML2ROOT)/include -I$(MPG123_DIR)/src/libmpg123
LDFLAGS = -shared -fPIC -DPIC

all shared: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -L$(RUNTIMELIB) -lgstreamer-0.10 -lgobject-2.0 -lgmodule-2.0 -lxml2 -lgthread-2.0 -lglib-2.0 -lgstbase-0.10 -lgstaudio-0.10 -lgstinterfaces-0.10 -lmpg123 -Wl,-soname -Wl,$(LIBTARGET_SHARED) -o $(LIBTARGET_SHARED)
	#$(STRIP) $(LIBTARGET_SHARED)

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.[oa] *~ core $(LIBTARGET_SHARED)*
