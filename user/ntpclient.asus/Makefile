# Under Solaris, you need to 
#    CFLAGS += -xO2 -Xc
#    LDLIBS += -lnsl -lsocket
# Some versions of Linux may need
#    CFLAGS += -D_GNU_SOURCE
# To cross-compile
#    CC = arm-linux-gcc

CFLAGS += -Wall -O
CFLAGS += -D__USE_BSD -DASUS_NVRAM
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/include   # ASUS

SHDIR = $(ROOTDIR)/user/shared
LDFLAGS = -L. -L$(SHDIR) -lshared

all: ntpclient

test: ntpclient
	./ntpclient -d -r <test.dat

ntpclient: ntpclient.o phaselock.o

romfs: ntpclient
	$(STRIP) ntpclient
	$(ROMFSINST) /usr/sbin/ntpclient 

clean:
	rm -f ntpclient *.o
