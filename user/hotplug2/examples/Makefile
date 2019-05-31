# vim:set sw=8 nosta:

all:

romfs:
	case "$(CONFIG_LINUXDIR)" in \
		*3.*) \
		$(ROMFSINST) hotplug2.rules-2.6kernel /etc_ro/hotplug2.rules \
		;; \
		*2.6.*) \
		$(ROMFSINST) hotplug2.rules-2.6kernel /etc_ro/hotplug2.rules \
		;; \
		*) \
		$(ROMFSINST) hotplug2.rules-2.4kernel /etc_ro/hotplug2.rules \
		;; \
	esac
	# remove comments, compress whitespace
	@sed -i 's/#.*//g;s/[ \t]\+/ /g;s/^[ \t]*//;s/[ \t]*$$//;/^$$/d' $(ROOTDIR)/romfs/etc_ro/hotplug2.rules

clean:

