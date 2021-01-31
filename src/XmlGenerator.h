/*
 *  Copyright (C) 2017-2021, Thomas Maier-Komor
 *
 *  This source file belongs to Wire-Format-Compiler.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef XMLGENERATOR_H
#define XMLGENERATOR_H

#include "CodeGenerator.h"
#include <string>


class XmlGenerator : public CodeGeneratorImpl
{
	public:
	XmlGenerator(class PBFile *, class Options *);
	void init();
	void writeFiles(const char *basename = 0);
	void writeLib();
	void setTarget(const char *target);
	void setLicense(const char *);
	PBFile *getPBFile() const
	{ return file; }

	private:
	PBFile *file;
	Options *target;
	std::string license;
};

#endif
