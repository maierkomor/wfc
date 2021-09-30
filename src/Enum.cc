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

#include "Message.h"
#include "Enum.h"
#include "Field.h"
#include "log.h"
#include "keywords.h"
#include "wirefuncs.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

using namespace std;


static map<string,unsigned> EnumName2Id;
static vector<Enum *> Enums;


Enum::Enum(const char *str, unsigned l)
: basename(str,l)
, name(basename)
, fullname(name)
, prefix("")
, toStringName()
, vmin(0)
, vmax(0)
, coding(enum_coding_varint)
, allow_alias(false)
{
	if (isKeyword(name.c_str()))
		error("keyword '%s' cannot be used as identifier for enum",name.c_str());

}


Enum::Enum(const string &n)
: basename(n)
, name(basename)
, fullname(name)
, prefix("")
, toStringName()
, vmin(0)
, vmax(0)
, coding(enum_coding_varint)
, allow_alias(false)
{

}


Enum *Enum::create(const char *str, unsigned l)
{
	string name(str,l);
	return create(name);
}


Enum *Enum::create(const string &name)
{
	Enum *e = new Enum(name);
	unsigned id = Enums.size() | ft_enum;
	Enums.push_back(e);
	auto r = EnumName2Id.insert(make_pair(name,id));
	if (!r.second)
		fatal("duplicate enum %s",name.c_str());
	return e;
}


Enum *Enum::id2enum(unsigned i)
{
	if ((i & ft_filter) != ft_enum)
		return 0;
	assert((i & ~ft_enum) < Enums.size());
	return Enums[i&~ft_enum];
}


const char *Enum::getTypeName(unsigned i, bool full)
{
	assert(((i & ft_filter) == ft_enum) && ((i & ~ft_enum) < Enums.size()));
	Enum *e = Enums[i&~ft_enum];
	if (full)
		return e->getFullName().c_str();
	return e->getName().c_str();
}


unsigned Enum::resolveId(const char *n, unsigned l)
{
	string name(n,l);
	auto i = EnumName2Id.find(name);
	if (i == EnumName2Id.end())
		return 0;
	return i->second;
}

const string &Enum::resolveId(unsigned id)
{
//	diag("resolvedId(%u)",i);
	assert(((id & ft_filter) == ft_enum) && ((id & ~ft_enum) < Enums.size()));
	return Enums[id&~ft_enum]->getName();
}


void Enum::setPrefix(const string &p)
{
	dbug("setPrefix: basename %s, name %s, p %s",basename.c_str(),name.c_str(),p.c_str());
	assert(prefix.empty());
	prefix = p;
	fullname = prefix + name;
}


void Enum::setNamePrefix(const string &p)
{
	dbug("setNamePrefix: basename %s, name %s, p %s",basename.c_str(),name.c_str(),p.c_str());
	assert(name == basename);
	name = p + basename;
	fullname = name;
}


static bool validFunctionName(const char *name, size_t l)
{
	const char *end = name + l;
	const char *n = name;
	while (n != end) {
		char c = *n++;
		if ((c >= 'a') && (c <= 'z'))
			continue;
		if ((c >= 'A') && (c <= 'Z'))
			continue;
		if ((c >= '0') && (c <= '9')) {
			if (name+1 == n)
				return false;
			continue;
		}
		switch (c) {
		case '_':
		case '$':
			continue;
		default:
			return false;
		}
	}
	return true;
}


void Enum::setOption(const string &o, const string &v)
{
	return setOption(o.c_str(),o.size(),v.c_str(),v.size());
}


void Enum::setOption(const char *o, size_t ol, const char *v, size_t vl)
{
	string option(o,ol), value(v,vl);
	if (option == "toString") {
		if ((value == "false") || (value == "\"\""))
			toStringName = "";
		else if ((v[0] == '"') && (v[vl-1] == '"')) {
			if (allow_alias)
				error("Option 'toString' cannot be used for enums with aliases.");
			else if (validFunctionName(v+1,vl-2)) {
				toStringName = string(v+1,vl-2);
				diag("enum %s: toString=%s",name.c_str(),toStringName.c_str());
			} else
				error("Invalid character in argument %s to option 'toString'",value.c_str());
		} else if (validFunctionName(v,vl)) {
			if (allow_alias)
				error("Option 'toString' cannot be used for enums with aliases.");
			else {
				toStringName = string(v,vl);
				diag("enum %s: toString=%s",name.c_str(),toStringName.c_str());
			}
		} else
			error("invalid argument '%s' to enum option '%s'",value.c_str(),option.c_str());
	} else if (option == "allow_alias") {
		if ((value == "true") || (value == "yes") || (value == "\"yes\"") || (value == "1")) {
			if (toStringName.empty())
				allow_alias = true;
			else
				error("Option 'allow_alias' cannot be used for enums with toString function.");
		} else if ((value == "false") || (value == "no") || (value == "\"no\"") || (value == "0")) {
			allow_alias = false;
		} else
			error("invalid argument '%s' to enum option '%s'",value.c_str(),option.c_str());
	} else if (option == "coding") {
		if (value == "dynamic")
			coding = enum_coding_dynamic;
		else if (value == "fixed8")
			coding = enum_coding_fixed8;
		else if (value == "fixed16")
			coding = enum_coding_fixed16;
		else if (value == "fixed32")
			coding = enum_coding_fixed32;
		else if (value == "fixed64")
			coding = enum_coding_fixed64;
		else if (value == "varint")
			coding = enum_coding_varint;
		else
			error("invalid argument '%s' to enum option '%s'",value.c_str(),option.c_str());
	} else
		error("unknown option '%s' for enum with value '%s'",value.c_str(),option.c_str());
}


void Enum::add(const char *str, unsigned l, const char *vstr, unsigned vl)
{
	int64_t v = strtoll(vstr,0,0);
	diag("add enum %s = %ld",str,v);
	add(str,l,v);
}


void Enum::add(const char *str, unsigned l, int64_t v)
{
	string name(str,l);
	if (nv.empty()) {
		vmin = v;
		vmax = v;
	} else {
		if (v < vmin)
			vmin = v;
		if (v > vmax)
			vmax = v;
	}
	auto x = nv.insert(make_pair(name,v));
	if (!x.second) {
		error("unable to add enumerator %s: identifier already used",name.c_str());
		return;
	}
	if (!allow_alias) {
		auto i = vn.find(v);
		if (i != vn.end()) {
			error("Unable to add enumerate '%s': enum value %ld already used by identifier '%s'.\nUse option allow_alias to allow multiple identifiers for one value.",name.c_str(),v,i->second.c_str());
			return;
		}
	}
	vn.insert(make_pair(v,name));
}


int64_t Enum::getValue(const string &str) const
{
	auto i = nv.find(str);
	assert(i != nv.end());
	return i->second;
}


bool Enum::hasValue(const string &str) const
{
	auto i = nv.find(str);
	return (i != nv.end());
}


const char *Enum::getName(int64_t v) const
{
	auto i = vn.find(v);
	if (i != vn.end())
		return i->second.c_str();
	return 0;
}


const char *Enum::getFirstName() const
{
	auto i = vn.begin();
	if (i != vn.end())
		return i->second.c_str();
	return 0;
}


wiretype_t Enum::getEncoding() const
{
	switch (coding) {
	default:
	case enum_coding_unset:
		abort();
	case enum_coding_varint:
		return wt_varint;
	case enum_coding_dynamic:
		return wt_dynamic;
	case enum_coding_fixed8:
		return wt_8bit;
	case enum_coding_fixed16:
		return wt_16bit;
	case enum_coding_fixed32:
		return wt_32bit;
	case enum_coding_fixed64:
		return wt_64bit;
	}
}


unsigned Enum::getMaximumSize() const
{
	unsigned lv = wiresize_u64(vmin);
	unsigned uv = wiresize_u64(vmax);
	return lv > uv ? lv : uv;
}


bool Enum::hasFixedSize() const
{
	// The return value of hasFixedSize is used for packed encoding.
	switch (coding) {
	default:
	case enum_coding_unset:
		abort();
	case enum_coding_varint:
	case enum_coding_dynamic:
		return false;
	case enum_coding_fixed8:
	case enum_coding_fixed16:
	case enum_coding_fixed32:
	case enum_coding_fixed64:
		return true;
	}
}


void Enum::setStringFunction()
{
	toStringName = basename + "_str";
}


void Enum::setStringValue(const char *str, const char *text, size_t l)
{
	int64_t v = strtoll(str,0,0);
	char *val = (char *) malloc(l+1);
	val[l] = 0;
	memcpy(val,text,l);
	if (!stringValues.insert(make_pair(v,val)).second) {
		error("enum value %ld has already a string literal set; ignoring %s",v,val);
		free(val);
	}
}


const char *Enum::getStringValue(int64_t v) const
{
	auto i = stringValues.find(v);
	if (i == stringValues.end())
		return 0;
	return i->second;
}
