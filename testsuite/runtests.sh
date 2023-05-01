#!/bin/bash
#
#  Copyright (C) 2019-2020, Thomas Maier-Komor
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
SIZE=`which gsize||which size`
#echo make is $MAKE

declare -A flagsets cxxflags
flagsets[fs_OsLs]='-Os -fwfclib=static -g'
flagsets[fs_OsLi]='-Os -fwfclib=inline -g'
flagsets[fs_OsLe]='-Os -fwfclib=extern -g'
flagsets[fs_O2Ls]='-O2 -fwfclib=static -g'
flagsets[fs_O2Li]='-O2 -fwfclib=inline -g'
flagsets[fs_O2Le]='-O2 -fwfclib=extern -g'
flagsets[fs_OrLs]='-Or -fwfclib=static -g'
flagsets[fs_OrLi]='-Or -fwfclib=inline -g'
flagsets[fs_OrLe]='-Or -fwfclib=extern -g'
flagsets[fs_O2s]='-O2 -s'
flagsets[fs_O2sp]='-O2 -fpadded_message_size'
flagsets[fs_Ors]='-Or -s'
flagsets[fs_Oss]='-Os -s'
flagsets[fs_O2B]='-O2 -fwfclib=extern -fBaseClass="Message"'
flagsets[fs_OrB]='-Or -fwfclib=extern -fBaseClass="Message"'
flagsets[fs_OsB]='-Os -fwfclib=extern -fBaseClass="Message"'
flagsets[fs_O2C]='-O2 -fStringType=C -ftoString=""'
flagsets[fs_OsC]='-Os -fStringType=C -ftoString=""'
flagsets[fs_OrC]='-Or -fStringType=C -ftoString=""'
flagsets[fs_O2A]='-O2 -fStringType=AString -fheader=\"astring.h\"'
flagsets[fs_OsA]='-O2 -fStringType=AString -fheader=\"astring.h\"'
flagsets[fs_OrA]='-O2 -fStringType=AString -fheader=\"astring.h\"'
flagsets[fs_O2ALe]='-O2 -fStringType=AString -fheader=\"astring.h\"'
flagsets[fs_OsALe]='-O2 -fStringType=AString -fheader=\"astring.h\"'
flagsets[fs_OrALe]='-O2 -fStringType=AString -fheader=\"astring.h\"'
if [ `uname -m` != "armv7l" ]; then
	flagsets[fs_O2le]="-O2 -fendian=little -g"
fi

cxxflags[fs_OsLs]="-Dstringtype=string"
cxxflags[fs_OsLi]="-Dstringtype=string"
cxxflags[fs_OsLe]="-Dstringtype=string"
cxxflags[fs_O2Ls]="-Dstringtype=string"
cxxflags[fs_O2Li]="-Dstringtype=string"
cxxflags[fs_O2Le]="-Dstringtype=string"
cxxflags[fs_OrLs]="-Dstringtype=string"
cxxflags[fs_OrLi]="-Dstringtype=string"
cxxflags[fs_OrLe]="-Dstringtype=string"
cxxflags[fs_O2s]="-Dstringtype=string"
cxxflags[fs_Ors]="-Dstringtype=string"
cxxflags[fs_Oss]="-Dstringtype=string"
cxxflags[fs_O2C]="-Dstringtype=string"
cxxflags[fs_OsC]="-Dstringtype=string"
cxxflags[fs_OrC]="-Dstringtype=string"
cxxflags[fs_O2A]="-Dstringtype=AString"
cxxflags[fs_OsA]="-Dstringtype=AString"
cxxflags[fs_OrA]="-Dstringtype=AString"
cxxflags[fs_O2ALe]="-Dstringtype=AString"
cxxflags[fs_OsALe]="-Dstringtype=AString"
cxxflags[fs_OrALe]="-Dstringtype=AString"
cxxflags[fs_O2le]="-Dstringtype=string"

defaulttests="initests corruption enumtest empty_test tttest \
	vbittest stringtest recursion json_hs lt1 skiptest reftest \
	vbittest2 tttest fixed_test novi_test pack_test comp_test \
	ref_byname byname_test inv_def_test arraycheck reftestv2"

# tests that need special settings
# TODO: cstrtest
extratests="cstrtest xvarint xint"

testcases[fs_OsLs]="$defaulttests xvarint xint"
testcases[fs_OsLi]="$defaulttests xint"
testcases[fs_OsLe]="$defaulttests"
testcases[fs_O2Ls]="$defaulttests xvarint xint"
testcases[fs_O2Li]="$defaulttests xint"
testcases[fs_O2Le]="$defaulttests"
testcases[fs_OrLs]="$defaulttests xvarint xint"
testcases[fs_OrLi]="$defaulttests xint"
testcases[fs_OrLe]="$defaulttests"
testcases[fs_O2s]="$defaulttests xint"
testcases[fs_Ors]="$defaulttests xint"
testcases[fs_Oss]="$defaulttests xint"
testcases[fs_O2C]="$defaulttests xint"
testcases[fs_OsC]="$defaulttests xint"
testcases[fs_OrC]="$defaulttests xint"
testcases[fs_O2A]="$defaulttests xint"
testcases[fs_OsA]="$defaulttests xint"
testcases[fs_OrA]="$defaulttests xint"
testcases[fs_O2ALe]="$defaulttests"
testcases[fs_OsALe]="$defaulttests"
testcases[fs_OrALe]="$defaulttests"
testcases[fs_O2le]="$defaulttests"


CXXFLAGS0=$CXXFLAGS
if [ "" == "$CXXFLAGS0" ]; then
	#CXXFLAGS0="-Os -g -Wall --std=gnu++11"
	CXXFLAGS0="-g -Wall --std=gnu++11"
fi


for flagset in "${!flagsets[@]}"; do
	echo testing with flagset $flagset
	echo CXXFLAGS=${cxxflags[$flagset]}
	echo TESTCASES=${testcases[$flagset]}

	#ODIR=`pwd`/"$flagset"
	ODIR="$flagset"
	WFCFLAGS="${flagsets[$flagset]}"
	TESTCASES="${testcases[$flagset]}"
	CXXFLAGS="$CXXFLAGS0 -I../include -I. -I$ODIR -I/usr/pkg/include ${cxxflags[$flagset]}"
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
	export ODIR WFCFLAGS CXXFLAGS WFCOBJS TESTCASES
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
	$SIZE $wfcs 2>/dev/null | grep -v filename | awk "{printf(\"%s(%s): %d\n\",\$6,\"$flagset\",\$1);}" >> $tmpfile
	cd ..
done
sort < $tmpfile
rm $tmpfile
