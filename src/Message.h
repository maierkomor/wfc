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

#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <vector>
#include <stdint.h>

class Enum;
class Field;
class Options;


class Message
{
	public:
	Message()
	: name()
	, basename()
	, prefix()
	, m_parent(0)
	, m_options(0)
	, m_msgid(0)
	, m_maxfid(0)
	, m_numvalid(0)
	, oneOf(false)
	, used(false)
	{ }
	
	explicit Message(const char *, unsigned, bool = false);

	~Message();
	
	static unsigned resolveId(const char *n, unsigned l);
	static const std::string &resolveId(unsigned i);
	static Message *id2msg(unsigned i);

	void addField(Field *f);
	void addMessage(Message *m);
	Message *getMessage(const char *n) const;
	void setParent(Message *m);
	void setName(const char *n, unsigned l);
	unsigned getId() const;
	unsigned getMaxSize() const;
	Enum *getEnum(unsigned i) const;
	Enum *getEnum(const char *e) const;

	const std::string &getName() const
	{ return name; }

	void setNamePrefix(const std::string &p);
	void setPrefix(const std::string &p);

	void setOptions(Options *o)
	{ m_options = o; }

	const std::string &getPrefix() const
	{ return prefix; }
	
	const std::string &getBasename() const
	{ return basename; }

	const std::string &getFullname() const
	{ return fullname; }

	size_t numFields() const
	{ return fields.size(); }

	Field *getField(unsigned n) const
	{ return fields[n]; }

	Field *getField(const char *n) const;

	size_t numMessages() const
	{ return msgs.size(); }

	Message *getMessage(unsigned i) const
	{ return msgs[i]; }

	unsigned getNumValid() const
	{ return m_numvalid; }

	Message *getParent() const
	{ return m_parent; }

	const char *getValidType() const
	{ return m_validtype; }

	void addEnum(Enum *e)
	{ enums.push_back(e); }

	unsigned numEnums() const
	{ return enums.size(); }

	bool isOneOf() const
	{ return oneOf; }

	void setUsed(bool u);

	bool isUsed() const
	{ return used; }

	std::string findROstring() const;

	void setNumValid(unsigned n);
	bool usesVectors() const;

	const std::string &getOption(const char *) const;
	bool getFlag(const char *) const;
	bool hasFixedSize() const;
	size_t getFixedSize() const;

	private:
	Message(const Message &);
	Message &operator = (const Message &);

	std::string name, basename, prefix, fullname;
	Message *m_parent;
	Options *m_options;
	unsigned m_msgid, m_maxfid, m_numvalid;
	const char *m_validtype;
	std::vector<Field*> fields;
	std::vector<Message*> msgs;
	std::vector<Enum*> enums;
	bool oneOf, used;
};

#endif
