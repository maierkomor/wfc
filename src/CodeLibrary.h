/*
 *  Copyright (C) 2017-2018, Thomas Maier-Komor
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

#ifndef CODELIBRARY_H
#define CODELIBRARY_H

#include <map>
#include <set>
#include <string>
#include <vector>
#include "codeid.h"


class CodeTemplate;
class Generator;
class Options;

class CodeLibrary
{
	public:
	CodeLibrary();

	void add(const char *fn);
	void addBuf(char *, const char *fn = "<internal>");
	//std::string getComment(const char *function, const Options *o) const;
	//const std::string &getFunction(const char *function, const Options *o) const;
	//const std::string &getFunction(codeid_t f, const Options *o) const;
	void write_includes(Generator &G, const std::vector<unsigned> &funcs, const Options *options) const;
	void write_cpp(Generator &G, const std::vector<unsigned> &funcs, const Options *options) const;
	void write_h(Generator &G, const std::vector<unsigned> &funcs, const Options *options) const;

	private:
	void addFile(const char *fn, struct stat * = 0);
	CodeTemplate *getTemplate(codeid_t, const Options *o) const;

	std::multimap<codeid_t, CodeTemplate *> m_templates;
	std::set<std::string> m_files;
	std::map<std::string,std::string> m_variants;
};

extern CodeLibrary Lib;

#endif
