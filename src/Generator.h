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

#ifndef GENERATOR_H
#define GENERATOR_H

#include <iosfwd>
#include <map>
#include <string>
#include <stdint.h>
#include <sstream>

class Enum;
class Field;
class Message;

typedef enum { gen_wire = 0x10000, gen_sink = 0x10001, gen_string = 0x10002 } genmode_t;

class Indentor;

class Generator
{
	public:
	explicit Generator(std::ostream &o, const class Options * = 0);
	~Generator();

	void addVariable(const std::string &n, const std::string &v);
	void addVariable(const std::string &n, const char *v);
	const char *getVariable(const std::string &n);
	bool hasVariable(const std::string &n) const;
	bool hasValue(const std::string &n) const;
	void indentingWrite(const char *t, const char *e);
	void setEnum(const Enum *e);
	void setField(const Field *f);
	void setMessage(const Message *m);
	void setVariable(const std::string &n, const std::string &v);
	void setVariable(const std::string &n, const char *v);
	void setVariableDec(const std::string &n, int64_t v);
	void setVariableHex(const std::string &n, uint64_t v);
	void clearVariable(const std::string &n);
	void fillField(const std::string &arg);
	void setMode(genmode_t);

	const Message *getMessage() const
	{ return m_message; }

	const Enum *getEnum() const
	{ return m_enum; }

	const Field *getField() const
	{ return m_field; }

	void sync()
	{ m_out.flush(); }

	bool skipComments() const
	{ return m_skipComments; }

	private:
	std::string evaluate(const std::string &p);

	std::ostream &m_out;
	std::map<std::string,std::string> m_vars;
	int m_indent, m_errid;
	unsigned m_skipBrace;
	const class Options *m_options;
	const Message *m_message;
	const Field *m_field;
	const Enum *m_enum;
	std::string m_linebuf, m_errstr;
	bool m_skipAsserts, m_ind1line, m_afternl, m_skipComments, m_inComment;
	class FoldCompounds *m_folder;
};


Generator &operator << (Generator &, const char *);
Generator &operator << (Generator &, const std::string &);
//Generator &operator << (Generator &g, uint64_t u);
//Generator &operator << (Generator &g, int64_t u);
//Generator &operator << (Generator &g, uint32_t u);
//Generator &operator << (Generator &g, int32_t u);

template <typename T>
Generator &operator << (Generator &g, T u)
{
	std::stringstream ss;
	ss << u;
	const std::string &s = ss.str();
	g.indentingWrite(s.c_str(),s.c_str()+s.size());
	return g;
}

bool toIdentifier(std::string &id);

#endif
