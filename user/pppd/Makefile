
PPPD_DIR = $(ROOTDIR)/user/pppd
INSTALLDIR = $(ROOTDIR)/romfs

all: pppd

pppd:
	$(MAKE) -C pppd_src

romfs:	pppd_src pppd_src/plugins
	$(MAKE) -C pppd_src/plugins
	cp pppd_src/pppd ./
	$(STRIP) pppd
	$(ROMFSINST) /usr/sbin/pppd
	cp pppd_src/plugins/rp-pppoe/rp-pppoe.so ./
	$(STRIP) rp-pppoe.so
	mkdir -p $(ROMFSDIR)/usr/lib/pppd/
	$(ROMFSINST) /usr/lib/pppd/rp-pppoe.so
	cp pppd_src/plugins/pppol2tp.so ./
	$(STRIP) pppol2tp.so
	$(ROMFSINST) /usr/lib/pppd/pppol2tp.so
	cp pppd_src/plugins/pptp/pptp.so ./
	$(STRIP) pptp.so
	$(ROMFSINST) /usr/lib/pppd/pptp.so	

clean:
	rm -f pppd rp-pppoe.so pppol2tp.so pptp.so
	$(MAKE) -C pppd_src clean
