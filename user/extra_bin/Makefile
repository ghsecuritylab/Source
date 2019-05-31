ROMFS_DIR = $(ROOTDIR)/romfs

clean:

romfs:
	mkdir -p $(ROMFS_DIR)/usr/lib
	cp webs_update.sh $(ROMFS_DIR)/sbin/webs_update.sh
ifeq ($(CONFIG_ASUS_XXX),y)
	cp webs_upgrade.sh $(ROMFS_DIR)/sbin/webs_upgrade.sh
endif
	cp webs_upgrade_radio.sh $(ROMFS_DIR)/sbin/webs_upgrade_radio.sh
ifeq ($(CONFIG_ASUS_PLAYER),y)
	cp -a testmp3 $(ROMFS_DIR)/usr/lib
endif
