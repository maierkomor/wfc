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

#ifndef INDENTER_H
#define INDENTER_H

#include <stack>
#include <string>


class Indenter
{
	public:
	explicit Indenter()
	: out()
	, indent(0)
	, indNext(false)
	{ }

	void process(const char *);

	const std::string &getOutput() const
	{ return out; }
	
	private:
	std::string out;
	int indent;
	bool indNext;
};


inline Indenter &operator << (Indenter &c, const char *s)
{
	c.process(s);
	return c;
}

#endif
