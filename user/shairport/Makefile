#
# Makefile for libao
#

SHAIRDIR = $(ROOTDIR)/user/shairport
INSTALLDIR = $(ROOTDIR)/romfs
USRDIR = $(ROOTDIR)/user
LIBDIR = $(ROOTDIR)/lib
GLIBDIR = $(LIBDIR)/glib-2.35.1

CFLAGS = -g -O2 -Wall -I$(LIBDIR)/alsa-lib-1.0.18/include -I$(USRDIR)/openssl-0.9.8e/include -I$(LIBDIR)/libao-1.1.0/include

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

LDFLAGS = -L$(LIBDIR)/lib -lgcc -L$(USRDIR)/openssl-0.9.8e -L$(USRDIR)/shared -lshared


MY_CFLAGS= -DASUS_NVRAM
MY_LDFLAGS=-lao -lssl -lcrypto

CFLAGS += $(MY_CFLAGS)
LDFLAGS+=-lm -lpthread $(MY_LDFLAGS)

OBJS=socketlib.o shairport.o alac.o hairtunes.o 
all: hairtunes shairport 

hairtunes: hairtunes.c alac.o
	$(CC) $(CFLAGS) -DHAIRTUNES_STANDALONE hairtunes.c alac.o -o $@ $(LDFLAGS)

shairport: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)
	$(STRIP) $@

clean:
	-@rm -rf hairtunes shairport $(OBJS)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


prefix=/usr/local

romfs:
	chmod 755 $(SHAIRDIR)/shairport
	cp -f $(SHAIRDIR)/shairport  $(INSTALLDIR)/usr/sbin

.PHONY: all clean install

.SILENT: clean

