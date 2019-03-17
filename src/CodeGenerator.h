/*
 *  Copyright (C) 2017-2019, Thomas Maier-Komor
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

#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include <string>
#include <vector>


class PBFile;
class Options;

class CodeGeneratorImpl
{
	public:
	virtual ~CodeGeneratorImpl();
	virtual void init(const std::vector<std::string> &) = 0;
	virtual void writeFiles(const char *basename = 0) = 0;
	virtual void writeLib() = 0;
	virtual void setTarget(const char *target) = 0;
	virtual void setLicense(const char *) = 0;
	virtual PBFile *getPBFile() const = 0;
};


typedef enum { cg_none, cg_cpp, cg_xml } codegen_t;


class CodeGenerator
{
	public:
	CodeGenerator(PBFile *, Options *);

	void init(const std::vector<std::string> &);
	void writeFiles(const char *basename);
	void writeLib(const char *basename = 0);
	void setTarget(const char *);

	private:
	CodeGenerator(const CodeGenerator &);
	CodeGenerator &operator = (const CodeGenerator &);

	CodeGeneratorImpl *impl;
};


void PostProcess(const char *fn, class Options *);


#endif
