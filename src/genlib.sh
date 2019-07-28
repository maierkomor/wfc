#!/bin/bash
#
#  Copyright (C) 2017-2019, Thomas Maier-Komor
#
#  This source file belongs to Wire-Format-Compiler.
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


if [ -f template_lib.h.new ]; then
	rm template_lib.h.new
fi

if [ -f template_lib.cc.new ]; then
	rm template_lib.cc.new
fi

if [ "$CCTSRC" == "" ]; then
	cd src
	if [ -d ../.hg ]; then
		CCTSRC=`hg st -a -c -m -n ../share`
	else
		CCTSRC=`ls -1 ../share/*.cc*`
	fi
fi

echo '#include "template_lib.h"' > template_lib.cc.new
echo >> template_lib.cc.new
echo "char TemplateLib[] = {" >> template_lib.cc.new

cat $CCTSRC | xxd -i >> template_lib.cc.new

echo ", 0x00 };" >> template_lib.cc.new

echo "#ifndef TEMPLATE_LIB_H" > template_lib.h.new
echo "#define TEMPLATE_LIB_H" >> template_lib.h.new
echo  >> template_lib.h.new
echo "extern char TemplateLib[];" >> template_lib.h.new
echo >> template_lib.h.new
echo "#endif" >> template_lib.h.new

if [ -f template_lib.cc ]; then
	diff template_lib.cc template_lib.cc.new 2>&1 > /dev/null
	if [ "$?" == "0" ]; then
		echo template_lib.cc is up-to-date
		rm template_lib.cc.new
	else
		echo updating template_lib.cc
		mv template_lib.cc.new template_lib.cc
	fi
else
	mv template_lib.cc.new template_lib.cc
fi

if [ -f template_lib.h ]; then
	diff template_lib.h template_lib.h.new 2>&1 > /dev/null
	if [ "$?" == "0" ]; then
		echo template_lib.h is up-to-date
		rm template_lib.h.new
	else
		echo updating template_lib.h
		mv template_lib.h.new template_lib.h
	fi
else
	mv template_lib.h.new template_lib.h
fi
