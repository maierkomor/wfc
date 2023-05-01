/*
 *  Copyright (C) 2017-2020, Thomas Maier-Komor
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

#ifndef CODETEMPLATE_H
#define CODETEMPLATE_H

#include <string>
#include <map>
#include <vector>

#include "codeid.h"

class Generator;
class Options;

extern const char *Functions[];


class CodeTemplate
{
	public:
	CodeTemplate(char *f, char *sl, char *eoc, char *eofunc);

	bool fitsOptions(const Options *) const;
	bool isDefine() const;
	bool isFunction() const;
	bool isTemplate() const;
	bool isUnion() const;
	bool isStruct() const;

	static codeid_t getFunctionId(const std::string &);
	codeid_t getFunctionId() const;

	void setFilename(const char *f)
	{ filename = f; }

	const std::string &getVariant() const
	{ return variant; }

	void write_h(Generator &G, libmode_t);
	void write_cpp(Generator &G, libmode_t);

	const std::vector<std::string> &getDependencies() const
	{ return dependencies; }

	const std::vector<std::string> &getIncludes() const
	{ return includes; }

	const std::vector<std::string> &getSysincludes() const
	{ return sysincludes; }

	private:
	void writeComment(Generator &G) const;
	void addDependencies(char *);
	std::string getDeclaration() const;

	std::string function, variant, code, comment, filename;
	std::vector<std::string> includes, sysincludes, dependencies;
	std::map<std::string,std::string> requirements, metadata;
	bool forceinline = false, omitdecl = false;

	/* valid requirements:
	 * varintbits
	 * endian
	 * Optimize
	 * errorhandling
	 * intsize
	 * stringtype
	 */
};


#endif
