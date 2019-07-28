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

#ifndef MESSAGE_H
#define MESSAGE_H

#include <map>
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
	: m_name()
	, m_basename()
	, m_prefix()
	, m_parent(0)
	, m_options(0)
	, m_msgid(0)
	, m_maxfid(0)
	, m_numvalid(0)
	, m_used(false)
	, m_vdata(false)
	{ }
	
	explicit Message(const char *, unsigned, bool = false);

	~Message();
	
	static unsigned resolveId(const char *n, unsigned l);
	static const std::string &resolveId(unsigned i);
	static Message *id2msg(unsigned i);

	void addField(Field *f);
	void addMessage(Message *m);
	Message *getMessage(const char *n) const;
	const char *getValidType() const;
	void setOption(const std::string &option, const std::string &value);
	void setParent(Message *m);
	void setName(const char *n, unsigned l);
	unsigned getId() const;
	unsigned getMaxSize() const;
	Enum *getEnum(unsigned i) const;
	Enum *getEnum(const char *e) const;

	const std::string &getName() const
	{ return m_name; }

	void setNamePrefix(const std::string &p);
	void setPrefix(const std::string &p);

	void setOptions(Options *o)
	{ m_options = o; }

	const std::string &getPrefix() const
	{ return m_prefix; }
	
	const std::string &getBasename() const
	{ return m_basename; }

	const std::string &getFullname() const
	{ return m_fullname; }

	size_t numFields() const
	{ return m_fields.size(); }

	//Field *getField(unsigned n) const
	//{ return m_fields[n]; }

	Field *getFieldId(unsigned n) const;
	Field *getField(const char *n) const;

	const std::map<unsigned,Field *> &getFields() const
	{ return m_fields; }

	const std::vector<unsigned> getFieldSeq() const
	{ return m_fieldseq; }

	size_t numMessages() const
	{ return m_msgs.size(); }

	Message *getMessage(unsigned i) const
	{ return m_msgs[i]; }

	unsigned getNumValid() const
	{ return m_numvalid; }

	Message *getParent() const
	{ return m_parent; }

	void addEnum(Enum *e)
	{ m_enums.push_back(e); }

	unsigned numEnums() const
	{ return m_enums.size(); }

	void setUsed(bool u);

	bool isUsed() const
	{ return m_used; }

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

	std::string m_name, m_basename, m_prefix, m_fullname;
	Message *m_parent;
	Options *m_options;
	unsigned m_msgid, m_maxfid, m_numvalid;
	std::map<unsigned,Field*> m_fields;
	std::vector<Message*> m_msgs;
	std::vector<Enum*> m_enums;
	std::vector<unsigned> m_fieldseq;
	bool m_used, m_vdata;
};

#endif
