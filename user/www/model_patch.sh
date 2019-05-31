#!/bin/sh
#
# Copyright 2013, ASUSTek Inc.
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of ASUSTek Inc.
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of ASUSTek Inc..
#

Usage() {
	echo "Usage: $0 TARGET_DIR PATCHLET_DIR"
	exit 1
}

do_patch() {
 local DIR_DST=$1
 local DIR_SRC=$2
 local DEPTH=$3
 local FNAME
 local BNAME

# declare -i DEPTH

 cd $DIR_DST
 for FNAME in $DIR_SRC/* ; do
  if [ -f $FNAME ]; then
   echo "patch -p${DEPTH} < $FNAME"
   patch -p${DEPTH} < $FNAME
  fi
 done
 DEPTH=$[DEPTH +1];
 for FNAME in $DIR_SRC/* ; do
  if [ -d $FNAME ]; then
   BNAME=`basename $FNAME`
   do_patch $DIR_DST/$BNAME $DIR_SRC/$BNAME $DEPTH
  fi
 done
}


if [ "${1}X" = "X" -o "${2}X" = "X" ]; then
	Usage
fi

PWD=`pwd`
TARGET_DIR=${PWD}/${1}
PATCH_DIR=${PWD}/${2}

if [ ! -d $TARGET_DIR -o ! -d $PATCH_DIR ]; then
	Usage
fi

do_patch $TARGET_DIR $PATCH_DIR 1
cd $PWD
exit 0
