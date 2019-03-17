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

#ifndef PROTODRIVER_H
#define PROTODRIVER_H

#include <stdio.h>
#include <string>
#include <stdint.h>

#include "Field.h"
#include "PBFile.h"
#include "xproto.h"

class ProtoDriver;
class PBFile;

int yylex(yy::ProtoParser::semantic_type *yylval_param, yy::ProtoParser::location_type *yylloc_param, ProtoDriver &drv);

class ProtoDriver
{
	public:
	explicit ProtoDriver(bool d);

	std::string filename;
	int parse(const char *fn);

	//void error(const yy::location &, const std::string &);
	//void error(const std::string &);
	int lex(yy::ProtoParser::semantic_type *yylval_param, yy::ProtoParser::location_type *yylloc_param);

	void setFile(PBFile *f)
	{ file = f; }

	PBFile *getFile() const;

	bool hadError() const
	{ return had_error; }

	void setError()
	{ had_error = true; }

	protov_t getProtocol() const;

	private:
	ProtoDriver(const ProtoDriver &);
	ProtoDriver& operator = (const ProtoDriver &);

	void *scanner;
	PBFile *file;
	bool had_error;
	bool debug;
};


#endif
