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

#ifndef ENUMS_H
#define ENUMS_H

#include <map>
#include <string>
#include <inttypes.h>
#include "wiretypes.h"

typedef enum {
	enum_coding_unset = 0,
	enum_coding_varint = 1,
	enum_coding_fixed8 = 2,
	enum_coding_fixed16 = 3,
	enum_coding_fixed32 = 4,
	enum_coding_fixed64 = 5,
	enum_coding_dynamic = 6,
} enum_coding_t;


class Enum
{
	public:
	static Enum *create(const std::string &n);
	static Enum *create(const char *n, unsigned l);
	static unsigned resolveId(const char *n, unsigned l);
	static const std::string &resolveId(unsigned i);
	static Enum *id2enum(unsigned i);
	static const char *getTypeName(unsigned i, bool full);

	void add(const char *str, unsigned l, const char *vstr, unsigned vl);
	void add(const char *str, unsigned l, int64_t v);
	int64_t getValue(const std::string &str) const;
	wiretype_t getEncoding() const;
	unsigned getMaximumSize() const;
	bool hasValue(const std::string &str) const;
	const char *getName(int64_t v) const;
	const char *getFirstName() const;

	const std::string &getName() const
	{ return name; }

	const std::string &getFullName() const
	{ return fullname; }

	const std::string &getPrefix() const
	{ return prefix; }

	const std::map<std::string,int64_t> &getNameValuePairs() const
	{ return nv; }

	const std::multimap<int64_t,std::string> &getValueNamePairs() const
	{ return vn; }

	void setPrefix(const std::string &prefix);
	void setNamePrefix(const std::string &prefix);

	void setOption(const char *o, size_t ol, const char *v, size_t vl);
	void setOption(const std::string &o, const std::string &v);

	const std::string &getStringFunction() const
	{ return toStringName; }

	const char *getStringValue(int64_t) const;
	void setStringValue(const char *str, const char *text, size_t l);

	void setStringFunction();

	bool hasFixedSize() const;

	private:
	explicit Enum(const std::string &);
	Enum(const char *str,unsigned l);
	std::string basename,name,fullname,prefix;
	std::map<std::string,int64_t> nv;
	std::multimap<int64_t,std::string> vn;
	std::map<int64_t,const char *> stringValues;
	std::string toStringName;
	int64_t vmin, vmax;
	enum_coding_t coding;
	bool allow_alias;
};


#endif
