#!/bin/bash
#
#  Copyright (C) 2019, Thomas Maier-Komor
#
#  This file belongs to Wire-Format-Compiler.
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

export WFC=`pwd`/../bin/wfc
MAKE=`which gmake||which make`
#echo make is $MAKE

declare -A flagsets
flagsets[fs_OsLs]="-Os -fwfclib=static -g"
flagsets[fs_OsLi]="-Os -fwfclib=inline -g"
flagsets[fs_OsLe]="-Os -fwfclib=extern -g"
flagsets[fs_O2Ls]="-O2 -fwfclib=static -g"
flagsets[fs_O2Li]="-O2 -fwfclib=inline -g"
flagsets[fs_O2Le]="-O2 -fwfclib=extern -g"
flagsets[fs_OrLs]="-Or -fwfclib=static -g"
flagsets[fs_OrLi]="-Or -fwfclib=inline -g"
flagsets[fs_OrLe]="-Or -fwfclib=extern -g"
flagsets[fs_O2le]="-O2 -fendian=little -g"
flagsets[fs_O2s]="-O2 -s"
flagsets[fs_Ors]="-Or -s"
flagsets[fs_Oss]="-Os -s"
flagsets[fs_O2C]="-O2 -fStringType=C"
flagsets[fs_OsC]="-Os -fStringType=C"
flagsets[fs_OrC]="-Or -fStringType=C"

CXXFLAGS0=$CXXFLAGS
if [ "" == "$CXXFLAGS0" ]; then
	CXXFLAGS0="-Os -g -Wall --std=gnu++11"
fi


for flagset in "${!flagsets[@]}"; do
	echo testing with flagset $flagset

	#ODIR=`pwd`/"$flagset"
	ODIR="$flagset"
	WFCFLAGS="${flagsets["$flagset"]}"
	CXXFLAGS="$CXXFLAGS0 -I../include -I. -I$ODIR"
	if [ -d "$ODIR" ]; then
		rm -r "$ODIR"
	fi
	mkdir "$ODIR"
	echo "$WFCFLAGS"|grep "wfclib=extern" > /dev/null
	if [ "0" == "$?" ]; then
		echo generating library into $ODIR
		"$WFC" $WFCFLAGS -l -o $ODIR
		WFCOBJS=$ODIR/wfccore.o
	else
		WFCOBJS=
	fi
	export ODIR WFCFLAGS CXXFLAGS WFCOBJS
	echo compiling wfc files
	echo MAKEFLAGS=$MAKEFLAGS
	$MAKE -f genwfc.mk -e || exit
	echo compiling test-cases
	$MAKE -e -f runtests.mk || exit
done

# report sizes
cd testcases
wfcs=`ls *.wfc | sed 's/\.wfc/.o/g'`
cd ..
tmpfile=`mktemp`
for flagset in "${!flagsets[@]}"; do
	cd $flagset
	size $wfcs 2>/dev/null | grep -v filename | awk "{printf(\"%s(%s): %d\n\",\$6,\"$flagset\",\$1);}" >> $tmpfile
	cd ..
done
sort < $tmpfile
rm $tmpfile
