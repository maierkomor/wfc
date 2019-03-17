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

#include "PBFile.h"
#include "ProtoDriver.h"
#include <assert.h>
#include <iostream>

using namespace std;

ProtoDriver::ProtoDriver(bool d)
: scanner(0)
, file(0)
, had_error(false)
, debug(d)
{ }


void yy::ProtoParser::error(const yy::ProtoParser::location_type &yylloc_param, const string &m)
{
	if (yylloc_param.begin.column != yylloc_param.end.column)
		cerr << "parser error at or before line " << (yylloc_param.begin.line) << '[' << (yylloc_param.begin.column) << ".." << (yylloc_param.end.column+1) << "]: " << m << endl;
	else
		cerr << "parser error at or before line " << (yylloc_param.begin.line) << '[' << (yylloc_param.begin.column) << "]: " << m << endl;
	drv.setError();
}


/*
void yy::ProtoParser::error(const string &m)
{
	cerr << "parser error: " << m << endl;
	drv.setError();
}
*/


PBFile *ProtoDriver::getFile() const
{
	assert(file);
	return file;
}


protov_t ProtoDriver::getProtocol() const
{
	return file->ProtocolVersion();
}
