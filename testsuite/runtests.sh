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
flagsets[fs_O2Ls]="-O2 -fwfclib=static"
flagsets[fs_O2Li]="-O2 -fwfclib=inline"
flagsets[fs_O2Le]="-O2 -fwfclib=extern"
flagsets[fs_OsLs]="-Os -fwfclib=static"
flagsets[fs_OsLi]="-Os -fwfclib=inline"
flagsets[fs_OsLe]="-Os -fwfclib=extern"
flagsets[fs_OrLs]="-Or -fwfclib=static"
flagsets[fs_OrLi]="-Or -fwfclib=inline"
flagsets[fs_OrLe]="-Or -fwfclib=extern"
flagsets[fs_O2le]="-O2 -fendian=little"


for flagset in "${!flagsets[@]}"; do
	echo testing with flagset $flagset

	#ODIR=`pwd`/"$flagset"
	ODIR="$flagset"
	WFCFLAGS="${flagsets["$flagset"]}"
	CXXFLAGS="-Os -g -I../include -I. -I$ODIR"
	if [ -d $ODIR ]; then
		rm -r $ODIR
	fi
	mkdir $ODIR
	echo "$WFCFLAGS"|grep "wfclib=extern" > /dev/null
	if [ "0" == "$?" ]; then
		echo generating library into $ODIR
		$WFC $WFCFLAGS -l -o $ODIR
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
