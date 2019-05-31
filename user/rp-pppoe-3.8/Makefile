.EXPORT_ALL_VARIABLES:

DIRS = src 

all:
	for i in $(DIRS) ; do make -C $$i $@ || exit $?; done

romfs: all
	cp src/pppoe-relay ./
	$(STRIP) pppoe-relay
	$(ROMFSINST) /usr/sbin/pppoe-relay

clean:
	for i in $(DIRS) ; do make -C $$i clean ; done

