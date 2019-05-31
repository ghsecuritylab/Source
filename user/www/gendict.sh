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
#
# replace model name/web title
#

Usage(){
	echo "Usage: $0 MODEL_NAME"
	exit 1
}

if [ "$1" = "" ]; then
	echo "***** You MUST input model name *****"
	Usage
fi

SRC=tmp.str
DICT=`find www/* -name "*.dict"`
YEAR=`date +%Y`

cat $1.str > $SRC
echo "ZVYEARVZ=$YEAR" >> $SRC

for dict in $DICT
do
	#sed -i s/ZVMODELVZ/$MODEL/g $dict
	total_line=`wc $SRC | awk '{print $1}'`
	curline=1
	while [ $curline -le $total_line ]; do
		TITLE=`awk -v lll=$curline -F = 'FNR==lll {print $1}' $SRC`
		CONTENT=`awk -v lll=$curline -F = 'FNR==lll {print $2}' $SRC`
		sed -i s/"$TITLE"/"$CONTENT"/g $dict
		curline=`expr $curline + 1`
	done
done

rm -f $SRC
