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

#include "Field.h"
#include "Message.h"
#include "Enum.h"
#include "log.h"
#include "keywords.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <map>

using namespace std;


static map<string,unsigned> MessageName2Id;
static vector<Message *> Messages;


Message::Message(const char *n, unsigned l, bool o)
: m_parent(0)
, m_options(0)
, m_msgid(0)
, m_maxfid(0)
, m_numvalid(0)
, m_used(false)
, m_storage(mem_regular)
, m_sorting(sort_unset)
{
	if (o)
		error("oneof is unsupported");
	setName(n,l);
	ParsingMessage = string(n,l);
//	diag("message %s",m_name.c_str());
}


Message::~Message()
{
	for (auto i(m_fields.begin()), e(m_fields.end()); i != e; ++i)
		delete i->second;
	for (auto i(m_enums.begin()), e(m_enums.end()); i != e; ++i)
		delete *i;
	for (auto i(m_msgs.begin()), e(m_msgs.end()); i != e; ++i)
		delete *i;

}


void Message::addMessage(Message *m)
{
	m_msgs.push_back(m);
	m->setParent(this);
}


Field *Message::getField(const char *n) const
{
	for (auto i(m_fields.begin()), e(m_fields.end()); i != e; ++i) {
		Field *f = i->second;
		if ((f != 0) && (0 == strcmp(n, f->getName())))
			return i->second;
	}
	return 0;
}


Field *Message::getFieldId(unsigned n) const
{
	auto i = m_fields.find(n);
	if (i == m_fields.end())
		return 0;
	return i->second;
}


unsigned Message::getMaxSize() const
{
	size_t n = 0;
	for (auto i(m_fields.begin()), e(m_fields.end()); i != e; ++i) {
		Field *f = i->second;
		if (f == 0)
			continue;
		unsigned fs = f->getMaxSize();
		if (fs == 0)
			return 0;
		n += fs;
	}
	return n;
}


Message *Message::getMessage(const char *n) const
{
	for (auto i(m_msgs.begin()), e(m_msgs.end()); i != e; ++i)
		if ((*i)->m_name == n)
			return *i;
	return 0;
}


void Message::setParent(Message *m)
{
	m_parent = m;
}


void Message::setNamePrefix(const string &p)
{
	m_name = p + m_basename;
	m_fullname = m_name;
	//diag("message %s: m_name set to '%s', m_fullname %s",m_basename.c_str(),m_prefix.c_str(),m_fullname.c_str());
}


void Message::setOption(const char *option, const char *value)
{
#ifdef BETA_FEATURES
	if (!strcmp(option,"members")) {
		if (!strcmp(value, "virtual"))
			m_storage = mem_virtual;
		else if (!strcmp(value,"static"))
			m_storage = mem_static;
		else if (!strcmp(value,"regular"))
			m_storage = mem_regular;
		else
			error("invalid value '%s' for option '%s'",value,option);
	} else
#endif
	if (!strcmp(option,"SortMembers")) {
		if (!strcmp(value,"id"))
			m_sorting = sort_id;
		else if (!strcmp(value,"name"))
			m_sorting = sort_name;
		else if (!strcmp(value,"size"))
			m_sorting = sort_size;
		else if (!strcmp(value,"type"))
			m_sorting = sort_type;
		else if (!strcmp(value,"none"))
			m_sorting = sort_none;
		else if (!strcmp(value,"unsorted"))
			m_sorting = sort_none;
		else
			error("invalid sorting type %s",value);
	} else {
		error("invalid message option '%s'",option);
	}
}


void Message::setPrefix(const string &p)
{
	m_prefix = p;
	m_fullname = p + m_basename;
	//diag("message %s: m_prefix set to '%s', m_name '%s', m_fullname '%s'",m_basename.c_str(),m_prefix.c_str(),m_name.c_str(),m_fullname.c_str());
}



void Message::setNumValid(unsigned n)
{
	assert(m_numvalid == 0);
	m_numvalid = n;
}


const char *Message::getValidType() const
{
	if ((m_numvalid <= 8) || (m_numvalid > 64))
		return "uint8_t";
	else if (m_numvalid <= 16)
		return "uint16_t";
	else if (m_numvalid <= 32)
		return "uint32_t";
	else
		return "uint64_t";
}


Message *Message::id2msg(unsigned id)
{
	assert(((id & ft_filter) == ft_msg) && ((id & ~ft_msg) < Messages.size()));
	return Messages[id&~ft_msg];
}


void Message::addField(Field *f)
{
	assert(f);
	f->setParent(this);
	const char *m_name = f->getName();
	unsigned id = f->getId();
	for (auto i(m_fields.begin()), e(m_fields.end()); i != e; ++i) {
		Field *f2 = i->second;
		if (f2 == 0)
			continue;
		if (0 == strcmp(m_name,f2->getName()))
			error("duplicate field m_name '%s' (ids: %u and %u)",m_name,id,f2->getId());
	}
//	diag("adding field %s with id %u, type %x",f->getName(),id,f->getType());
	if (!m_fields.insert(make_pair(id,f)).second) {
		error("duplicate id %d for field %s",id,f->getName());
		delete f;
	}
	if (id > m_maxfid)
		m_maxfid = id;
	m_fieldseq.push_back(id);
}


Enum *Message::getEnum(unsigned i) const
{
	assert(i < m_enums.size());
	return m_enums[i];
}


Enum *Message::getEnum(const char *n) const
{
	for (auto i(m_enums.begin()), e(m_enums.end()); i != e; ++i) {
		Enum *en = *i;
		if (en->getName() == n)
			return en;
	}
	return 0;
}


unsigned Message::resolveId(const char *n, unsigned l)
{
	string m_name(n,l);
//	diag("resolveId('%s')",m_name.c_str());
	auto i(MessageName2Id.find(m_name));
	if (i != MessageName2Id.end()) {
//		diag("resolveId('%s') = %u",m_name.c_str(),i->second);
		return i->second;
	}
//	diag("resolveId('%s') = 0",m_name.c_str());
	return 0;
}


const string &Message::resolveId(unsigned id)
{
//	diag("resolvedId(%u)",i);
	assert(((id & ft_filter) == ft_msg) && ((id & ~ft_msg) < Messages.size()));
	return Messages[id&~ft_msg]->getName();
}


void Message::setName(const char *n, unsigned l)
{
	assert(m_msgid == 0);
	m_msgid = Messages.size() | ft_msg;
	Messages.push_back(this);
	m_name = string(n,l);
	if (isKeyword(m_name.c_str()))
		error("keyword '%s' cannot be used as identifier for messsage",m_name.c_str());
	m_basename = m_name;
	m_fullname = m_name;
	auto x = MessageName2Id.insert(make_pair(m_name,m_msgid));
	if (!x.second)
		fatal("message %s already defined",n);
//	diag("message %s has id %i",m_name.c_str(),id);
}


bool Message::usesVectors() const
{
	for (auto i(m_fields.begin()), e(m_fields.end()); i != e; ++i) {
		Field *f = i->second;
		if (f == 0)
			continue;
		if ((f->getQuantifier() == 2) && (f->getArraySize() == 0))
			return true;
	}
	return false;
}


const string &Message::getOption(const char *o) const
{
	const Message *m = this;
	while (m->m_parent)
		m = m->m_parent;
	assert(m->m_options);
	return m->m_options->getOption(o);
}


bool Message::getFlag(const char *o) const
{
	const Message *m = this;
	while (m->m_parent)
		m = m->m_parent;
	assert(m->m_options);
	return m->m_options->getFlag(o);
}

string Message::findROstring() const
{
	string ret;
	for (auto i(m_fields.begin()), e(m_fields.end()); i != e; ++i) {
		Field *f = i->second;
		if ((f == 0) || !f->isUsed())
			continue;
		uint32_t tid = f->getType();
		if (tid == ft_cptr)
			return m_fullname + '/' + f->getName();
		if ((tid & ft_filter) == ft_msg) {
			Message *sm = Message::id2msg(tid);
			ret = sm->findROstring();
			if (ret != "")
				return ret;
		}
	}
	return "";
}


bool Message::hasFixedSize() const
{
	for (auto i(m_fields.begin()), e(m_fields.end()); i != e; ++i) {
		Field *f = i->second;
		if ((f == 0) || !f->isUsed())
			continue;
		if (q_required != f->getQuantifier())
			return false;
		if (!f->hasFixedSize())
			return false;
	}
	return true;
}


size_t Message::getFixedSize() const
{
	size_t s = 0;
	for (auto i(m_fields.begin()), e(m_fields.end()); i != e; ++i) {
		Field *f = i->second;
		if ((f == 0) || !f->isUsed())
			continue;
		assert(f->hasFixedSize());
		s += f->getFixedSize();
	}
	return s;
}


void Message::setOptions(Options *o)
{
	m_options = o;
	if (m_sorting == sort_unset) {
		// not explicitly set, derive from target
		setOption("SortMembers",o->getOption("SortMembers").c_str());
	}
}


void Message::setUsed(bool u)
{
	m_used = u;
	if (u) {
		for (auto i(m_fields.begin()), e(m_fields.end()); i != e; ++i) {
			Field *f = i->second;
			if ((f == 0) || (!f->isUsed()))
				continue;
			unsigned type = f->getType();
			if (ft_msg != (type & ft_filter))
				continue;
			Message *sm = Message::id2msg(type);
			if (sm != this)
				sm->setUsed(true);
		}
	} else {
		for (size_t i = 0, n = m_msgs.size(); i != n; ++i) {
			m_msgs[i]->setUsed(false);
		}
	}
}


/*
const string &Message::getOption(string &path, const char *o) const
{
	if (path.c_str()[0] == '/') {

	}
	path = 
}
*/
