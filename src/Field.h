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

#ifndef FIELD_H
#define FIELD_H

#include <string>
#include <stdint.h>
#include "Options.h"
#include "wiretypes.h"


typedef enum {
	ft_invalid 	= 0,
	ft_filter	= 0xf0000000,
	ft_enum		= 0x10000000,
	// range for enums
	ft_msg		= 0x40000000,
	// range for user-defined messages
	ft_native	= 0x80000000,
	ft_bytes	= 0x80000001,
	ft_string	= 0x80000002,
	ft_cptr		= 0x80000003,
	// reserved for more native complex types
	// varint size
	ft_int32	= 0x80000020,
	ft_uint32	= 0x80000021,
	ft_sint32	= 0x80000022,
	ft_int64	= 0x80000030,
	ft_uint64	= 0x80000031,
	ft_sint64	= 0x80000032,
	ft_int16	= 0x80000040,
	ft_uint16	= 0x80000041,
	ft_sint16	= 0x80000042,
	// 8bit size
	ft_bool		= 0x80000050,
	ft_int8		= 0x80000051,
	ft_uint8	= 0x80000052,
	ft_sint8	= 0x80000053,
	ft_fixed8	= 0x80000054,
	ft_sfixed8	= 0x80000055,
	// 16bit size
	ft_fixed16	= 0x80000060,
	ft_sfixed16	= 0x80000061,
	// 32bit size
	ft_fixed32	= 0x80000070,
	ft_sfixed32	= 0x80000071,
	ft_float	= 0x80000072,
	// 64bit size
	ft_fixed64	= 0x80000080,
	ft_sfixed64	= 0x80000081,
	ft_double	= 0x80000082,
	// impelmentation-defined types
	ft_unsigned	= 0x80000090,
	ft_signed	= 0x80000091,
	ft_int		= 0x80000092,
	// reserved for more simple types
} fieldtype_t;


typedef enum {
	q_optional	= 0,
	q_required	= 1,
	q_repeated	= 2,
	q_unspecified	= 3
} quant_t;


typedef enum {
	st_std,
	st_class,
	st_pointer
} stringtype_t;


class Enum;
class Message;


extern std::string ParsingMessage;


class Field
{
	public:
	Field(const char *n, unsigned l, quant_t q, uint32_t t, long long i);
	~Field();

	unsigned getId() const
	{ return id; }

	unsigned getTagSize() const;

	const char *getName() const
	{ return name.c_str(); }

	quant_t getQuantifier() const
	{ return quan; }

	bool isRepeated() const
	{ return quan == q_repeated; }

	uint32_t getType() const;
	uint32_t getTypeClass() const;
	const char *getTypeName(bool full = false) const;

	void setDefaultValue(const char *d, size_t l)
	{ defvalue = std::string(d,l); }

	const char *getDefaultValue() const;
	bool setValidBit(int v);

	int getValidBit() const
	{ return valid_bit; }

	bool hasNativeType() const
	{ return (type & ft_filter) == ft_native; }

	bool hasSimpleType() const;
//	{ return ((type & ft_filter) == ft_enum) || (((type & ft_filter) == ft_native) && (type != ft_string) && (type != ft_bytes)); }

	bool hasEnumType() const
	{ return (type & ft_filter) == ft_enum; }

	bool hasMessageType() const
	{ return (type & ft_filter) == ft_msg; }

	bool isUsed() const
	{ return used; }

	bool isVirtual() const
	{ return vmember; }

	void setVirtual(bool v)
	{ vmember = v; }

	bool hasFixedSize() const;
	int64_t getMaxSize() const;
	unsigned getMemberSize() const;
	unsigned getFixedSize(bool tag = true) const;
	unsigned getIntSize() const
	{ return intsize; }
	void setIntSize(unsigned i)
	{ intsize = i; }
	static unsigned getVarintSize(uint64_t);
	wiretype_t getEncoding() const;
	wiretype_t getElementEncoding() const;
	void addOptions(class KVPair *);
	void setOption(const std::string &option, const std::string &value);
	bool isPacked() const;
	unsigned getArraySize() const;
	void setParent(Message *m);
	void setTarget(Options *o = 0);
	bool isEnum() const;
	bool isMessage() const;
	bool isNumeric() const;
	bool isString() const;
	bool needsValidbit() const;

	Enum *toEnum() const;

	Message *getParent() const
	{ return parent; }

	const std::string &getOption(const char *) const;

	const std::string &getInvalidValue() const
	{ return invvalue; }

	private:
	Field(const Field &);
	Field &operator = (const Field &);

	Message *parent;
	std::string name, defvalue, invvalue;
	Options *options;
	uint32_t type;
	unsigned id;
	unsigned arraysize;
	unsigned intsize;
	stringtype_t stringtype;

	// -3: uninitialized,
	// -2: invvalue,
	// -1: vector/char*/repeated/requried,
	// >=0: bit0, ...
	int valid_bit;
	quant_t quan;
	bool packed,used,vmember;
};


#endif
