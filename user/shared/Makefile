#
# Linux router shared code Makefile
#
# Copyright 2013, ASUSTek Inc.
# All Rights Reserved.
# 
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#

USERDIR = $(ROOTDIR)/user

CFLAGS += -s \
	-Wall \
	-DASUS_NVRAM \
	-DWPS_SUPPORT \
	-DASUS_ATE \
	-DLED_DRV_SUPPORT

# SquashFS
include $(ROOTDIR)/$(LINUXDIR)/.config
ifeq ($(filter y, $(CONFIG_RT2880_ROOTFS_IN_FLASH)),y)
	CFLAGS += -DUSE_SQUASHFS
endif

ifeq ($(filter y, $(CONFIG_DEFAULTS_ASUS_EAN66)),y)
CFLAGS += -DEXTERNAL_PHY_SUPPORT \
	-DDUAL_BAND_NONCONCURRENT \
	-DWPA_SUPPLICANT_SUPPORT
else
ifeq ($(filter y, $(CONFIG_DEFAULTS_ASUS_RPN12)),y)
CFLAGS +=
else
ifeq ($(filter y, $(CONFIG_DEFAULTS_ASUS_RPN14) $(CONFIG_DEFAULTS_ASUS_RPN53)),y)
CFLAGS += -DTOUCH_SUPPORT \
	-DAUDIO_JACK_SUPPORT \
	-DINTERNET_RADIO
else
ifeq ($(filter y, $(CONFIG_DEFAULTS_ASUS_RPAC52)),y)
CFLAGS += -DTOUCH_SUPPORT \
	-DAUDIO_JACK_SUPPORT \
	-DINTERNET_RADIO \
	-DIEEE802_11AC
else
ifeq ($(filter y, $(CONFIG_DEFAULTS_ASUS_RPAC56)),y)
CFLAGS += -DIEEE802_11AC
else
ifeq ($(filter y, $(CONFIG_DEFAULTS_ASUS_WMPN12)),y)
CFLAGS += -DINTERNET_RADIO
else
CFLAGS += -D__CONFIG_NAT__ \
	-DEXTERNAL_PHY_SUPPORT \
	-DDUAL_BAND_NONCONCURRENT \
	-DTOUCH_SUPPORT \
	-DAUDIO_JACK_SUPPORT \
	-DVLAN \
	-DDLM \
	-DINTERNET_RADIO \
	-DWPA_SUPPLICANT_SUPPORT \
	-DIEEE802_11AC
endif
endif
endif
endif
endif
endif

CFLAGS += -I. \
	-I./include \
	-I./chipdeps \
	-I$(USERDIR)/wireless_tools \
	-I$(ROOTDIR)/$(LINUXDIR)/include \
	-I$(ROOTDIR)/$(LINUXDIR)/drivers/net/raeth

LDFLAGS = -L.

OBJS := shutils.o \
	process.o \
	http.o \
	defaults.o \
	nvram_linux.o \
	linux_timer.o \
	nvparse.o \
	notify_rc.o \
	semaphore.o \
	flash_mtd.o \
	ralink.o
OBJS += $(ROOTDIR)/user/wireless_tools/iwlib.so

ifeq ($(CONFIG_DEFAULTS_RALINK_MT7620),y)
OBJS += mt7620.o
else
ifeq ($(CONFIG_DEFAULTS_RALINK_MT7621),y)
OBJS += mt7621.o
else
ifeq ($(CONFIG_DEFAULTS_RALINK_MT7628),y)
OBJS += mt7628.o
else
ifeq ($(CONFIG_DEFAULTS_RALINK_RT3883),y)
OBJS += rt3883.o
endif
endif
endif
endif

SRCS := $(OBJS:%.o=%.c)
HEADERS := mtd-abi.h flash_mtd.h notify_rc.h semaphore_mfp.h

vpath %.c chipdeps

all: libshared.so libshared.a

romfs:
	$(ROMFSINST) /lib/libshared.so

clean:
	rm -f *.o libshared.so

libshared.so: $(OBJS) $(SRCS) $(HEADERS)
	$(LD) -shared -o $@ $(OBJS)
	$(STRIP) --strip-debug $@

libshared.a: $(OBJS) $(SRCS) $(HEADERS)
	$(AR) cr $@ $(OBJS)
