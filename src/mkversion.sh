#!/bin/bash

if [ -f ../.hg_archival.txt ]; then
	# Gather version information from .hg_archival.txt for archives.
	TAG=`awk '/^tag:/ {printf("%s",$2);}' ../.hg_archival.txt`
	LTAG=`awk '/^latesttag:/ {printf("%s",$2);}' ../.hg_archival.txt`
	TAGD=`awk '/^latesttagdistance:/ {printf("%s",$2);}' ../.hg_archival.txt`
	if [ "" != "$TAG" ]; then
		VER=$TAG
	elif [ "" == "$LTAG" ]; then
		echo archive has no tag or latesttag
		exit 1
	elif [ "0" != $TAGD ]; then
		VER=$LTAG.$TAGD
	else
		VER=$LTAG
	fi
	echo "#define VERSION \"$VER\"" > version.h.new
	BR=`awk '/^branch:/ {printf("%s",$2);}' ../.hg_archival.txt`
	echo "#define HG_BRANCH \"$BR\"" >> version.h.new
elif [ -d ../.hg ]; then
	# Gather version information from repository and sandbox.
	VER=`hg log -r. -T'{tags}'`
	if [ "tip" == "$VER" ]; then
		# Then create version based on latest tag, tag distance, and modification indicator.
		VER=`hg log -r. -T "{latesttag}{if(latesttagdistance,'.{latesttagdistance}')}"`
	fi
	# Check if we have modified, removed, added or deleted files.
	if [ `hg st -mard | wc -l` != "0" ]; then
		# add delta indicator
		VER+="+"
	fi
	VER+=`hg log -r. -T" (hg:{rev}/{node|short})"`
	HG_BRANCH=`hg id -b`
	echo "#define VERSION \"$VER\"" > version.h.new
	HG_ID=`hg id -i`
	echo "#define HG_ID \"$HG_ID\"" >> version.h.new
	HG_REV=`hg log -r. -T{rev}`
	echo "#define HG_REV \"$HG_REV\"" >> version.h.new
	HG_BRANCH=`hg log -r. -T{branch}`
	echo "#define HG_BRANCH \"$HG_BRANCH\"" >> version.h.new
else
	# Bail out with an error, if no version information an be gathered.
	echo no version information available
	exit 1
fi

echo version $VER

if [ ! -f version.h ]; then
	echo creating version.h
	mv version.h.new version.h
	exit
fi

cmp version.h version.h.new 2>&1 > /dev/null

if [ "0" == "$?" ]; then
	echo version.h is up-to-date
	rm version.h.new
else
	echo updating version.h
	mv -f version.h.new version.h
fi
