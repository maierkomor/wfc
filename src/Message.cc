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

#include "Field.h"
#include "Message.h"
#include "Enum.h"
#include "log.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <map>

using namespace std;


static map<string,unsigned> MessageName2Id;
static vector<Message *> Messages;


Message::Message(const char *n, unsigned l, bool o)
: m_parent(0)
, m_msgid(0)
, m_maxfid(0)
, m_numvalid(0)
, m_validtype(0)
, oneOf(o)
{
	setName(n,l);
	ParsingMessage = string(n,l);
//	diag("message %s",name.c_str());
}


Message::~Message()
{
	for (auto i(fields.begin()), e(fields.end()); i != e; ++i)
		delete *i;
	for (auto i(enums.begin()), e(enums.end()); i != e; ++i)
		delete *i;
	for (auto i(msgs.begin()), e(msgs.end()); i != e; ++i)
		delete *i;

}


void Message::addMessage(Message *m)
{
	msgs.push_back(m);
	m->setParent(this);
}


Field *Message::getField(const char *n) const
{
	for (auto i(fields.begin()), e(fields.end()); i != e; ++i) {
		if (*i == 0)
			continue;
		if (0 == strcmp(n, (*i)->getName()))
			return *i;
	}
	return 0;
}

unsigned Message::getMaxSize() const
{
	size_t n = 0;
	for (auto i(fields.begin()), e(fields.end()); i != e; ++i) {
		Field *f = *i;
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
	for (auto i(msgs.begin()), e(msgs.end()); i != e; ++i)
		if ((*i)->name == n)
			return *i;
	return 0;
}


void Message::setParent(Message *m)
{
	m_parent = m;
}


void Message::setNamePrefix(const string &p)
{
	name = p + basename;
	fullname = name;
	//diag("message %s: name set to '%s', fullname %s",basename.c_str(),prefix.c_str(),fullname.c_str());
}


void Message::setPrefix(const string &p)
{
	prefix = p;
	fullname = p + basename;
	//diag("message %s: prefix set to '%s', name '%s', fullname '%s'",basename.c_str(),prefix.c_str(),name.c_str(),fullname.c_str());
}



void Message::setNumValid(unsigned n)
{
	assert(m_numvalid == 0);
	m_numvalid = n;
	if ((n <= 8) || (n > 64))
		m_validtype = "uint8_t";
	else if (n <= 16)
		m_validtype = "uint16_t";
	else if (n <= 32)
		m_validtype = "uint32_t";
	else
		m_validtype = "uint64_t";
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
	const char *name = f->getName();
	unsigned id = f->getId();
	for (auto i(fields.begin()), e(fields.end()); i != e; ++i) {
		Field *f2 = *i;
		if (f2 == 0)
			continue;
		if (0 == strcmp(name,f2->getName()))
			error("duplicate field name '%s' (ids: %u and %u)",name,id,f2->getId());
	}
//	diag("adding field %s with id %u, type %x",f->getName(),id,f->getType());
	if (id >= fields.size())
		fields.resize(id+1);
	if (fields[id] != 0) {
		error("duplicate id %d for field %s",id,f->getName());
		return;
	}
	fields[id] = f;
	if (id > m_maxfid)
		m_maxfid = id;
}


Enum *Message::getEnum(unsigned i) const
{
	assert(i < enums.size());
	return enums[i];
}


Enum *Message::getEnum(const char *n) const
{
	for (auto i(enums.begin()), e(enums.end()); i != e; ++i) {
		Enum *en = *i;
		if (en->getName() == n)
			return en;
	}
	return 0;
}


unsigned Message::resolveId(const char *n, unsigned l)
{
	string name(n,l);
//	diag("resolveId('%s')",name.c_str());
	auto i(MessageName2Id.find(name));
	if (i != MessageName2Id.end()) {
//		diag("resolveId('%s') = %u",name.c_str(),i->second);
		return i->second;
	}
//	diag("resolveId('%s') = 0",name.c_str());
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
	name = string(n,l);
	basename = name;
	fullname = name;
	auto x = MessageName2Id.insert(make_pair(name,m_msgid));
	if (!x.second)
		fatal("message %s already defined",n);
//	diag("message %s has id %i",name.c_str(),id);
}


bool Message::usesVectors() const
{
	for (auto i(fields.begin()), e(fields.end()); i != e; ++i) {
		Field *f = *i;
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
	for (auto i(fields.begin()), e(fields.end()); i != e; ++i) {
		Field *f = *i;
		if ((f == 0) || !f->isUsed())
			continue;
		uint32_t tid = f->getType();
		if (tid == ft_cptr)
			return fullname + '/' + f->getName();
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
	for (auto i(fields.begin()), e(fields.end()); i != e; ++i) {
		Field *f = *i;
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
	for (auto i(fields.begin()), e(fields.end()); i != e; ++i) {
		Field *f = *i;
		if ((f == 0) || !f->isUsed())
			continue;
		assert(f->hasFixedSize());
		s += f->getFixedSize();
	}
	return s;
}


void Message::setUsed(bool u)
{
	used = u;
	if (u) {
		for (size_t i = 0, n = numFields(); i != n; ++i) {
			Field *f = getField(i);
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
		for (size_t i = 0, n = msgs.size(); i != n; ++i) {
			msgs[i]->setUsed(false);
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
