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
#include "Enum.h"
#include "Message.h"
#include "KVPair.h"
#include "Options.h"
#include "log.h"
#include <cassert>
#include <stdlib.h>
#include "wirefuncs.h"
#include <set>

using namespace std;

string ParsingMessage;

static const char *KeywordsA[] = {
	"alignas",
	"alignof",
	"and",
	"asm",
	"auto",
	"break",
	"case",
	"catch",
	"char",
	"class",
	"const",
	"const_cast",
	"constexpr",
	"continue",
	"decltype",
	"default",
	"delete",
	"do",
	"double",
	"dynamic_cast",
	"else",
	"explicit",
	"extern",
	"false",
	"float",
	"for",
	"friend",
	"goto",
	"if",
	"inline",
	"long",
	"mutable",
	"namespace",
	"new",
	"nullptr",
	"or",
	"private",
	"protected",
	"public",
	"register",
	"reinterpret_cast",
	"restrict",
	"return",
	"short",
	"sizeof",
	"static",
	"static_assert",
	"static_cast",
	"struct",
	"switch",
	"template",
	"this",
	"throw",
	"true",
	"typedef",
	"typeid",
	"typename",
	"union",
	"using",
	"virtual",
	"void",
	"volatile",
	"while",
	"xor",
};


static set<string> Keywords;


Field::Field(const char *n, unsigned l, quant_t q, uint32_t t, long long i)
: parent(0)
, name(n,l)
, defvalue()
, invvalue()
, type(t)
, id(i)
, arraysize(0)
, intsize(64)
, valid_bit(-3)
, quan(q)
, packed(false)
, used(true)
{
	options = new Options((string)"field:"+n,Options::getFieldDefaults());
	assert(q != q_unspecified);
	if (id == 0)
		warn("In message %s: field %s has invalid id %lld: id should be positive integer",ParsingMessage.c_str(),name.c_str(),i);
	else if (i < 0)
		fatal("In message %s: field %s has invalid id %lld: id must not be negative integer",ParsingMessage.c_str(),name.c_str(),i);
	if (Keywords.empty()) {
		for (size_t i = 0; i < sizeof(KeywordsA)/sizeof(KeywordsA[0]); ++i)
			Keywords.insert(KeywordsA[i]);
	}
	if (Keywords.find(name) != Keywords.end())
		error("invalid identifier %s",name.c_str());
}


Field::~Field()
{
	delete options;
}


bool Field::hasSimpleType() const
{
	switch (type & ft_filter) {
	case ft_msg:
		return false;
	case ft_enum:
		return true;
	default:;
	}
	switch (getType()) {	// to get correct resolution of stringtype
	case ft_bytes:
	case ft_string:
		return false;
	case ft_cptr:
	case ft_bool:
	case ft_int8:
	case ft_uint8:
	case ft_sint8:
	case ft_fixed8:
	case ft_sfixed8:
	case ft_int16:
	case ft_uint16:
	case ft_sint16:
	case ft_fixed16:
	case ft_sfixed16:
	case ft_int32:
	case ft_uint32:
	case ft_sint32:
	case ft_fixed32:
	case ft_sfixed32:
	case ft_int64:
	case ft_uint64:
	case ft_sint64:
	case ft_fixed64:
	case ft_sfixed64:
	case ft_float:
	case ft_double:
		return true;
	default:
		abort();
	}
}


unsigned Field::getMemberSize() const
{
	switch (type & ft_filter) {
	case ft_msg:
		return 0;
	case ft_enum:
		return intsize/8;
	default:;
	}
	switch (getType()) {	// to get correct resolution of stringtype
	case ft_bytes:
	case ft_string:
		return 16;
	case ft_cptr:
		return intsize/8;
	case ft_bool:
	case ft_int8:
	case ft_uint8:
	case ft_sint8:
	case ft_fixed8:
	case ft_sfixed8:
		return 1;
	case ft_int16:
	case ft_uint16:
	case ft_sint16:
	case ft_fixed16:
	case ft_sfixed16:
		return 2;
	case ft_int32:
	case ft_uint32:
	case ft_sint32:
	case ft_fixed32:
	case ft_sfixed32:
	case ft_float:
		return 4;
	case ft_int64:
	case ft_uint64:
	case ft_sint64:
	case ft_fixed64:
	case ft_sfixed64:
	case ft_double:
		return 8;
	default:
		abort();
	}
	return 0;
}


int64_t Field::getMaxSize() const
{
	if ((quan == q_repeated) && (arraysize == 0))
		return 0;
	unsigned mult = arraysize;
	if (mult == 0)
		mult = 1;
	unsigned ms = 0;
	unsigned type = getType();
	wiretype_t enc = getEncoding();
	if ((type & ft_filter) == ft_enum) {
		enc = Enum::id2enum(type)->getEncoding();
	} else if ((type & ft_filter) == ft_msg) {
		enc = wt_msg;
		ms = Message::id2msg(type)->getMaxSize();
	}
	switch (enc) {
	case wt_lenpfx:
		switch(type) {
		case ft_msg:
			{
				Message *m = Message::id2msg(type);
				ms = m->getMaxSize();
			}
			break;
		case ft_string:
		case ft_bytes:
		case ft_cptr:
			{
				const string &maxlen = options->getOption("maxlength");
				if (maxlen == "")
					return 0;
				ms = strtol(maxlen.c_str(),0,0);
			}
			break;
		case ft_int8:
		case ft_uint8:
		case ft_sint8:
		case ft_int16:
		case ft_uint16:
		case ft_sint16:
		case ft_int32:
		case ft_uint32:
		case ft_sint32:
		case ft_int64:
		case ft_uint64:
		case ft_sint64:
			ms = options->VarIntBits() / 7 + 1;
			break;
		default:
			abort();
		}
		if (ms == 0)
			return 0;
		ms += getVarintSize(ms);
		break;
	case wt_dynamic:
		// invalid assumption: varint >= max(used fixed types)
		// so make sure that uint64_t can be transfered even
		// if varint_t is uint16_t
		ms = options->VarIntBits() / 7 + 1;
		if (ms < 8)
			ms = 8;
		break;
	case wt_varint:
		ms = options->VarIntBits() / 7 + 1;
		break;
	case wt_8bit:
		ms = 1;
		break;
	case wt_16bit:
		ms = 2;
		break;
	case wt_32bit:
		ms = 4;
		break;
	case wt_64bit:
		ms = 8;
		break;
	case wt_msg:
		break;
	default:
		abort();
	}
	unsigned ts = getTagSize();
	if (isPacked()) {
		assert(quan == q_repeated);
		return ts + getVarintSize(arraysize) + ms * arraysize;
	} else {
		return (ts + ms) * arraysize;
	}
	return ms;
}


void Field::setParent(Message *p)
{
	assert(parent == 0);
	parent = p;
}


const char *Field::getDefaultValue() const
{
	if (hasEnumType()) {
		Enum *en = Enum::id2enum(type);
		if (!defvalue.empty()) {
			if (!en->hasValue(defvalue))
				warn("default value %s of enum %s is not defined. Code may not compile.",defvalue.c_str(),en->getName().c_str());
			return defvalue.c_str();
		}
		const char *nullname = en->getName(0);
		if (nullname)
			return nullname;
		warn("enum %s has no value for 0 and no default value",en->getName().c_str());
		return en->getFirstName();
	}
	if (!defvalue.empty())
		return defvalue.c_str();
	if ((quan == q_required) && (getType() == ft_cptr))
		return "\"\"";
	return 0;
}


uint32_t Field::getType() const
{
	switch (type) {
	case ft_signed:
		switch (intsize) {
		case 8: return ft_sint8;
		case 16: return ft_sint16;
		case 32: return ft_sint32;
		case 64: return ft_sint64;
		default: abort();
		}
	case ft_int:
		switch (intsize) {
		case 8: return ft_int8;
		case 16: return ft_int16;
		case 32: return ft_int32;
		case 64: return ft_int64;
		default: abort();
		}
	case ft_unsigned:
		switch (intsize) {
		case 8: return ft_uint8;
		case 16: return ft_uint16;
		case 32: return ft_uint32;
		case 64: return ft_uint64;
		default: abort();
		}
	case ft_bytes:
		return ft_bytes;
	case ft_cptr:
	case ft_string: {
		const string &stringtype = getOption("stringtype");
		if ("C" == stringtype)
			return ft_cptr;
		if ("pointer" == stringtype)
			return ft_cptr;
		return ft_string;
		}
	}
	return type;
}


uint32_t Field::getTypeClass() const
{
	switch (type&ft_filter) {
	case ft_msg:
		return ft_msg;
	case ft_enum:
		return ft_enum;
	default:
		return getType();
	}
}


const char *Field::getTypeName(bool full) const
{
	if ((type & ft_filter) == ft_enum) 
		return Enum::getTypeName(type,full);
	if ((type & ft_filter) == ft_msg) 
		return Message::resolveId(type).c_str();
	switch (type) {
	case ft_signed:
	case ft_int: {
		const string &intsize = getOption("intsize");
		if (intsize == "8") return "int8_t";
		if (intsize == "16") return "int16_t";
		if (intsize == "32") return "int32_t";
		if (intsize == "64") return "int64_t";
		error("invalid value %s for option intsize",intsize.c_str());
		return "int64_t";
		}
	case ft_unsigned: {
		const string &intsize = getOption("intsize");
		if (intsize == "8") return "uint8_t";
		if (intsize == "16") return "uint16_t";
		if (intsize == "32") return "uint32_t";
		if (intsize == "64") return "uint64_t";
		error("invalid value %s for option intsize",intsize.c_str());
		return "uint64_t";
		}
	case ft_bool:		return "bool";
	case ft_int8:		return "int8_t";
	case ft_uint8:		return "uint8_t";
	case ft_sint8:		return "int8_t";
	case ft_fixed8:		return "uint8_t";
	case ft_sfixed8:	return "int8_t";
	case ft_int16:		return "int16_t";
	case ft_uint16:		return "uint16_t";
	case ft_sint16:		return "int16_t";
	case ft_fixed16:	return "uint16_t";
	case ft_sfixed16:	return "int16_t";
	case ft_int32:		return "int32_t";
	case ft_uint32:		return "uint32_t";
	case ft_sint32:		return "int32_t";
	case ft_fixed32:	return "uint32_t";
	case ft_sfixed32:	return "int32_t";
	case ft_int64:		return "int64_t";
	case ft_uint64:		return "uint64_t";
	case ft_sint64:		return "int64_t";
	case ft_fixed64:	return "uint64_t";
	case ft_sfixed64:	return "int64_t";
	case ft_float:		return "float";
	case ft_double:		return "double";
	case ft_bytes: {
		const string &bytestype = getOption("bytestype");
		if (bytestype.empty())
			return "std::string";
		return bytestype.c_str();
		}
	case ft_cptr:
	case ft_string:	{
		const string &stringtype = getOption("stringtype");
		if ("std" == stringtype )
			return "std::string";
		if ("pointer" == stringtype)
			return "const char *";
		if ("C" == stringtype)
			return "const char *";
		return stringtype.c_str();
		}
	default:
		ICE("unable to resolve type name for type %x",type);
		return 0;
	}
}


wiretype_t Field::getEncoding() const
{
	if ((type & ft_filter) == ft_msg)
		return wt_lenpfx;
	if (isPacked())
		return wt_lenpfx;
	if ((type & ft_filter) == ft_enum)
		return Enum::id2enum(type)->getEncoding();
	switch (type) {
	case ft_int8:
	case ft_uint8:
	case ft_sint8:
	case ft_int16:
	case ft_uint16:
	case ft_sint16:
	case ft_int32:
	case ft_uint32:
	case ft_sint32:
	case ft_int64:
	case ft_uint64:
	case ft_sint64:
		return wt_varint;
	case ft_bool:
	case ft_fixed8:
	case ft_sfixed8:
		return wt_8bit;
	case ft_fixed16:
	case ft_sfixed16:
		return wt_16bit;
	case ft_double:
	case ft_fixed64:
	case ft_sfixed64:
		return wt_64bit;
	case ft_float:
	case ft_fixed32:
	case ft_sfixed32:
		return wt_32bit;
	case ft_bytes:
	case ft_string:
	case ft_cptr:
		return wt_lenpfx;
	case ft_signed:
	case ft_int:
	case ft_unsigned: {
		const string &intsize = getOption("intsize");
		if (intsize == "8")
			return wt_8bit;
		if (intsize == "16")
			return wt_varint;
		if (intsize == "32")
			return wt_varint;
		if (intsize == "64")
			return wt_varint;
		fatal("invalid value for option intsize: %s",intsize.c_str());
		} break;
	default:
		;
	}
	ICE("missing encoding type 0x%x",type);
	return wt_lenpfx;	// to suppress warning
}


wiretype_t Field::getElementEncoding() const
{
	assert(quan == q_repeated);
	assert((type & ft_filter) != ft_msg);
	if ((type & ft_filter) == ft_enum)
		return Enum::id2enum(type)->getEncoding();
	switch (type) {
	case ft_int8:
	case ft_uint8:
	case ft_sint8:
	case ft_int16:
	case ft_uint16:
	case ft_sint16:
	case ft_int32:
	case ft_uint32:
	case ft_sint32:
	case ft_int64:
	case ft_uint64:
	case ft_sint64:
		return wt_varint;
	case ft_bool:
	case ft_fixed8:
	case ft_sfixed8:
		return wt_8bit;
	case ft_fixed16:
	case ft_sfixed16:
		return wt_16bit;
	case ft_double:
	case ft_fixed64:
	case ft_sfixed64:
		return wt_64bit;
	case ft_float:
	case ft_fixed32:
	case ft_sfixed32:
		return wt_32bit;
	case ft_bytes:
	case ft_string:
	case ft_cptr:
		return wt_lenpfx;
	case ft_signed:
	case ft_int:
	case ft_unsigned: {
		const string &intsize = getOption("intsize");
		if (intsize == "8")
			return wt_8bit;
		if (intsize == "16")
			return wt_varint;
		if (intsize == "32")
			return wt_varint;
		if (intsize == "64")
			return wt_varint;
		fatal("invalid value for option intsize: %s",intsize.c_str());
		} break;
	default:
		;
	}
	ICE("missing encoding type 0x%x",type);
	return wt_lenpfx;	// to suppress warning
}


bool Field::isEnum() const
{
	return ((type & ft_filter) == ft_enum);
}


bool Field::isMessage() const
{
	return ((type & ft_filter) == ft_msg);
}


bool Field::isNumeric() const
{
	switch (type & ft_filter) {
	case ft_msg:
		return false;
	case ft_enum:
		return true;
	default:;
	}
	return !isString();
}


bool Field::isString() const
{
	switch (type) {
	case ft_string:
	case ft_bytes:
	case ft_cptr:
		return true;
	default:
		return false;
	}
}


bool Field::isPacked() const
{
	switch (type & ft_filter) {
	case ft_msg:
		assert(packed == false);
		return false;
	case ft_enum:
		return packed;
	case ft_native:
		switch (type) {
		case ft_string:
		case ft_bytes:
		case ft_cptr:
			assert(packed == false);
			return false;
		case ft_unsigned:
		case ft_signed:
		case ft_int:
		case ft_int8:
		case ft_uint8:
		case ft_sint8:
		case ft_int16:
		case ft_uint16:
		case ft_sint16:
		case ft_int32:
		case ft_uint32:
		case ft_sint32:
		case ft_int64:
		case ft_uint64:
		case ft_sint64:
		case ft_bool:
		case ft_fixed8:
		case ft_sfixed8:
		case ft_fixed16:
		case ft_sfixed16:
		case ft_fixed32:
		case ft_sfixed32:
		case ft_float:
		case ft_fixed64:
		case ft_sfixed64:
		case ft_double:
			return packed;
		default:
			abort();
		}
	default:
		abort();
	}
	assert(packed == false);
	return false;
}

unsigned Field::getVarintSize(uint64_t v)
{
	return wiresize_u64(v);
}


unsigned Field::getTagSize() const
{
	return getVarintSize(id << 3);
}


bool Field::hasFixedSize() const
{
	if ((type & ft_filter) == ft_enum) {
		Enum *e = Enum::id2enum(type);
		assert(e);
		return e->hasFixedSize();
	}
	if ((type & ft_filter) == ft_msg) {
		Message *m = Message::id2msg(type);
		assert(m);
		return m->hasFixedSize();
	}
	switch (getType()) {
	case ft_bytes:
	case ft_string:
	case ft_cptr:
	case ft_unsigned:
	case ft_signed:
	case ft_int:
	case ft_int8:
	case ft_sint8:
	case ft_uint8:
	case ft_int16:
	case ft_sint16:
	case ft_uint16:
	case ft_int32:
	case ft_sint32:
	case ft_uint32:
	case ft_int64:
	case ft_sint64:
	case ft_uint64:
		return false;
	case ft_bool:
	case ft_fixed8:
	case ft_sfixed8:
	case ft_fixed16:
	case ft_sfixed16:
	case ft_fixed32:
	case ft_sfixed32:
	case ft_float:
	case ft_fixed64:
	case ft_sfixed64:
	case ft_double:
		return true;
	default:
		abort();
	}
	return false;
}


unsigned Field::getFixedSize(bool tag) const
{
	unsigned n;
	if ((type & ft_filter) == ft_enum) {
		Enum *e = Enum::id2enum(type);
		assert(e);
		n = 1;
		if (tag)
			n += getVarintSize(id << 3);
		return n;
	}
	if ((type & ft_filter) == ft_msg) {
		Message *m = Message::id2msg(type);
		assert(m);
		n = m->getFixedSize();
		if (tag) {
			n += getVarintSize(n);	// encoded length after tag
			n += getVarintSize(id << 3);
		}
		return n;
	}
	switch (type) {
	case ft_unsigned:
	case ft_signed:
	case ft_int: {
		const string &intsize = getOption("intsize");
		if (intsize == "64")
			n = 8;
		else if (intsize == "32")
			n = 4;
		else if (intsize == "16")
			n = 2;
		else if (intsize == "8")
			n = 1;
		else
			abort();
		} break;
	case ft_bool:
	case ft_int8:
	case ft_sint8:
	case ft_uint8:
	case ft_fixed8:
	case ft_sfixed8:
		n = 1;
		break;
	case ft_fixed16:
	case ft_sfixed16:
		n = 2;
		break;
	case ft_fixed32:
	case ft_sfixed32:
	case ft_float:
		n = 4;
		break;
	case ft_fixed64:
	case ft_sfixed64:
	case ft_double:
		n = 8;
		break;
	default:
		abort();
	}
	if (tag)
		n += getVarintSize(id << 3);
	return n;
}


void Field::addOptions(KVPair *kvl)
{
	KVPair *kv = kvl;
	do {
		setOption(kv->getKey(),kv->getValue());
		options->addOption(kv->getKey().c_str(),kv->getValue().c_str(),false);
		kv = kv->getNext();
	} while (kv);
	options->add(kvl);
}


void Field::setTarget(Options *o)
{
	options->setParent(o);
}


void Field::setOption(const string &option, const string &value)
{
	if (option == "default") {
		defvalue = value;
	} else if (option == "unset") {
		invvalue = value;
	} else if (option == "packed") {
		if (quan != q_repeated)
			error("invalid option 'packed' for non-repated type");
		if ((type & ft_filter) == ft_msg)
			error("invalid option 'packed' for message type");
		if (value == "true")
			packed = true;
		else if (value == "false")
			packed = false;
		else
			error("invalid value '%s' for option packed",value.c_str());
	} else if (option == "used") {
		if (value == "true")
			used = true;
		else if (value == "false")
			used = false;
		else
			error("invalid value '%s' for option used",value.c_str());
	} else if ((option == "array") || (option == "arraysize")) {
		const char *v = value.c_str();
		if ((v[0] >= '0') && (v[0] <= '9')) {
			long s = strtol(value.c_str(),0,0);
			arraysize = s;
			options->addOption("arraysize",v,false);
		} else {
			error("invalid array size %s",value.c_str());
		}
	} else if (option == "stringtype") {
		if ((type == ft_string) || (type == ft_cptr)) {
			if ((value == "pointer") || (value == "C")) {
				stringtype = st_pointer;
				type = ft_cptr;
			} else {
				stringtype = st_class;
				type = ft_string;
			}
		} else
			error("invalid option stringtype for field '%s' of type '%s'",name.c_str(),getTypeName());
	} else if (option == "bytestype") {
		if (type == ft_bytes) {
			type = ft_bytes;
		} else
			error("invalid option bytestype for field '%s' of type '%s'",name.c_str(),getTypeName());
	} else if (option == "intsize") {
		if (value == "64")
			intsize = 64;
		else if (value == "32")
			intsize = 32;
		else if (value == "16")
			intsize = 16;
		else if (value == "8")
			intsize = 8;
		else
			error("invalid setting for option intsize: %s",value.c_str());
	//} else if (option == "encoding") {	// TODO
	} else 
		error("unknown option '%s'",option.c_str());
}


bool Field::needsValidbit() const
{
	if (quan != q_optional)
		return false;
	if (invvalue.empty()) {
		if (getType() == ft_cptr)
			return false;
		return true;
	}
	return false;
}


bool Field::setValidBit(int v)
{
	assert(valid_bit == -3);
	if (quan != q_optional) {
		valid_bit = -1;
		return false;
	}
	if (ft_cptr == getType()) {
		valid_bit = -1;
		return false;
	}
	if (!invvalue.empty()) {
		valid_bit = -2;
		return false;
	}
	valid_bit = v;
	return true;
}


const string &Field::getOption(const char *o) const
{
	return options->getOption(o);
}


unsigned Field::getArraySize() const
{
	const string &as = getOption("arraysize");
	if (as == "")
		return arraysize;
	long long a = strtoll(as.c_str(),0,0);
	return a;
}


Enum *Field::toEnum() const
{
	if (!hasEnumType())
		return 0;
	return Enum::id2enum(type);
}
