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

#ifndef MESSAGE_H
#define MESSAGE_H

#include <map>
#include <string>
#include <vector>
#include <stdint.h>
#include "Options.h"

class Enum;
class Field;
class Options;

typedef enum { sort_unset, sort_id, sort_name, sort_size, sort_type, sort_none } msg_sorting_t;


class Message
{
	public:
	Message()
	: m_name()
	, m_basename()
	, m_prefix()
	, m_parent(0)
	, m_msgid(0)
	, m_maxfid(0)
	, m_numvalid(0)
	, m_used(false)
	, m_generate(false)
	, m_storage(mem_regular)
	, m_sorting(sort_unset)
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
	void setOption(const char *o, const char *v);
	void setParent(Message *m);
	void setName(const char *n, unsigned l);
	unsigned getId() const;
	unsigned getMaxSize() const;
	Enum *getEnum(unsigned i) const;
	Enum *getEnum(const char *e) const;

	const std::vector<Enum*> &getEnums() const
	{ return m_enums; }

	const std::vector<Message*> &getMessages() const
	{ return m_msgs; }

	const std::string &getName() const
	{ return m_name; }

	void setNamePrefix(const std::string &p);
	void setPrefix(const std::string &p);

	void setOptions(Options *o);

	const std::string &getPrefix() const
	{ return m_prefix; }
	
	const std::string &getBasename() const
	{ return m_basename; }

	const std::string &getFullname() const
	{ return m_fullname; }

	size_t numFields() const
	{ return m_fields.size(); }

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

	msg_sorting_t getSorting() const
	{ return m_sorting; }

	void setUsed(bool u);

	bool isUsed() const
	{ return m_used; }

	bool getGenerate() const
	{ return m_generate; }

	void setGenerate(bool g);

	mem_inst_t getStorage() const
	{ return m_storage; }

	std::string findROstring() const;

	void setNumValid(unsigned n);
	bool usesVectors() const;
	bool hasFixedSize() const;
	size_t getFixedSize() const;
	void addReservation(unsigned lb, unsigned ub);

	private:
	Message(const Message &);
	Message &operator = (const Message &);

	std::string m_name, m_basename, m_prefix, m_fullname;
	Message *m_parent;
	unsigned m_msgid, m_maxfid, m_numvalid;
	std::map<unsigned,Field*> m_fields;
	std::vector<Message*> m_msgs;
	std::vector<Enum*> m_enums;
	std::vector<unsigned> m_fieldseq;
	std::vector< std::pair<unsigned,unsigned> > m_reservations;
	bool m_used, m_generate;	// m_used is calculated by dependencies, m_generate is option/command-line setting
	mem_inst_t m_storage;
	msg_sorting_t m_sorting;
};

#endif
