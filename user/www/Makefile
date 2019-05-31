# Copyright 2013, ASUSTek Inc.
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of ASUSTek Inc.
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of ASUSTek Inc..
#

ROMFS_DIR = $(ROOTDIR)/romfs

ifeq ($(MODEL_NAME),RP-N12)
NOUSE_FILES=www/images/New_ui/RPN53_LED.gif
else
NOUSE_FILES=xxx
endif

all:
romfs:
	echo "make www romfs"
	rm -rf $(ROMFS_DIR)/www
	cp -r universal_www www
	@if [ -d www/sysdep/$(MODEL_NAME) ] ; then \
		cp -prLf www/sysdep/$(MODEL_NAME)/* www; \
	fi
	rm -rf $(NOUSE_FILES)
	@rm -rf www/sysdep
	./gendict.sh $(MODEL_NAME)
	mv www $(ROMFS_DIR)/www
	touch $(ROMFS_DIR)/tmp/settings
	cd $(ROMFS_DIR)/www && ln -sf ../tmp/settings Settings_$(MODEL_NAME).CFG && cd ../..
	rm -f $(ROMFS_DIR)/tmp/settings
