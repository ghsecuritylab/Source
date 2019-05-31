CFLAGS += -g -O2

all: tcpcheck

tcpcheck: tcpcheck.o
	$(CC) tcpcheck.o -o tcpcheck

clean:
	rm -f tcpcheck *~ *.o

romfs:
	$(STRIP) tcpcheck
	$(ROMFSINST) /usr/sbin/tcpcheck
