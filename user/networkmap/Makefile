SHDIR = $(ROOTDIR)/user/shared

CFLAGS += -s \
	-Wall -O2 \
	-DASUS_NVRAM

ifeq ($(filter y, $(CONFIG_DEFAULTS_ASUS_EAN66)),y)
CFLAGS += -DSWMODE_ADAPTER_SUPPORT
else
ifeq ($(filter y, $(CONFIG_DEFAULTS_ASUS_RPN12) $(CONFIG_DEFAULTS_ASUS_RPN14) $(CONFIG_DEFAULTS_ASUS_RPN53) $(CONFIG_DEFAULTS_ASUS_RPAC52) $(CONFIG_DEFAULTS_ASUS_RPAC56) $(CONFIG_DEFAULTS_ASUS_WMPN12)),y)
CFLAGS +=
else
CFLAGS += -DSWMODE_ADAPTER_SUPPORT \
	-DSCAN_PRINTER_SERVER \
	-DSCAN_ITUNE_SERVER \
	-DSCAN_UPNP_DEVICE
endif
endif

CFLAGS += -I. \
	-I$(SHDIR) \
	-I$(ROOTDIR)/$(LINUXDIR)/include

LDFLAGS += -L$(SHDIR) -lshared \
	-lpthread

EXEC = networkmap

OBJS = function.o networkmap.o

all: $(EXEC)
	$(STRIP) $(EXEC)

romfs:
	$(ROMFSINST) /usr/sbin/$(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -f *.o $(EXEC)
