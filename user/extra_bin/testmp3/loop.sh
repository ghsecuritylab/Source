#!/bin/sh
RET=0;
FPATH=/usr/lib/testmp3
FNAME=1
while [ "$RET" = "0" ]; do
	asplayer ${FPATH}/${FNAME}.mp3
	RET=$?
        if [ $FNAME = "1" ]; then
		FNAME=2
	elif [ $FNAME = "2" ]; then
		FNAME=3
	elif [ $FNAME = "3" ]; then
		FNAME=1
	fi
done
