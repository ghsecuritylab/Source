SHDIR = $(ROOTDIR)/user/shared

EXEC = mtd_write
OBJS = mtd.o

CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)

ifeq ($(filter y, $(CONFIG_DEFAULTS_ASUS_RPN12) $(CONFIG_DEFAULTS_ASUS_WMPN12)),y)
CFLAGS += -DASUS_NVRAM \
	-DUPGRADE_PROGRESS_SUPPORT

CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/include

LDFLAGS += -L$(SHDIR) -lshared
endif

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o
