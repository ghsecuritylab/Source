#
# Makefile for libao
#

USERDIR = $(ROOTDIR)/user
LIBDIR = $(ROOTDIR)/lib
GLIBDIR = $(LIBDIR)/glib-2.35.1
INSTALLDIR = $(ROOTDIR)/romfs

CFLAGS += -I$(USERDIR)/shared \
	-I$(ROOTDIR)/$(LINUXDIR)/include

CFLAGS += -DASUS_NVRAM
ifeq ($(filter y, $(CONFIG_DEFAULTS_ASUS_RPN14) $(CONFIG_DEFAULTS_ASUS_RPN53) $(CONFIG_DEFAULTS_ASUS_RPAC52)),y)
CFLAGS += -DI2C_TO_ALC5627
else
ifeq ($(filter y, $(CONFIG_DEFAULTS_ASUS_RPAC56)),y)
CFLAGS += -DXXXXX
else
ifeq ($(filter y, $(CONFIG_DEFAULTS_ASUS_WMPN12)),y)
CFLAGS += -DUSB_TO_CM6510 \
	-DLED_INDICATE_AUDIO_PLAY
endif
endif
endif

CFLAGS += -O2 -I$(GLIBDIR)/include -I$(LIBDIR)/gupnp-av-0.10.0 -I$(LIBDIR)/gupnp-0.19.0 -I$(LIBDIR)/gssdp-0.13.0 -I$(LIBDIR)/libxml2-2.8.0/include -I$(LIBDIR)/libsoup-2.41.2 -I$(LIBDIR)/alsa-lib-1.0.18/include

CSRC=avtransport.c renderingcontrol.c connectionmanager.c m3player.c gstreamer.c last_change.c
OBJS=$(patsubst %.c,%.o, $(CSRC))
TARGET=m3player

LDFLAGS = -Wl,--export-dynamic -L$(LIBDIR)/lib -lgobject-2.0 -lglib-2.0 -lgio-2.0 -lgupnp-av-1.0 -lgupnp-1.0 -lgssdp-1.0 -lxml2 -lsoup-2.4 -lgthread-2.0 -lgmodule-2.0 -luuid -lffi -lz -lc -lgcc -L$(USERDIR)/shared -lshared 

all: $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)
	$(STRIP) $(TARGET)

romfs:
	$(ROMFSINST) /bin/$(TARGET)
	mkdir -p $(INSTALLDIR)/m3player
	cp -f *.xml $(INSTALLDIR)/m3player
	cp -f icons/$(MODEL_NAME).png $(INSTALLDIR)/m3player/model.png
	#replace model name
	sed -i 's/ZVMODELVZ/$(MODEL_NAME)/g' $(INSTALLDIR)/m3player/MediaRendererV1.xml
	sed -i 's/ZVMODELVZ/$(MODEL_NAME)/g' $(INSTALLDIR)/m3player/MediaRendererV2.xml

$(OBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o *~ core $(TARGET)
