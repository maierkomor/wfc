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

#include "Generator.h"
#include "Enum.h"
#include "Evaluator.h"
#include "Field.h"
#include "Message.h"
#include "Options.h"
#include "Indenter.h"
#include "log.h"
#include "FoldCompounds.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


using namespace std;


bool toIdentifier(string &id)
{
	if (id.empty())
		return false;
	const char *str = id.c_str();
	if (str[0] == '"') {
		if (str[strlen(str)-1] != '"')
			return false;
		id = string(str+1,strlen(str)-2);
		str = id.c_str();
		if (*str == 0)
			return false;
	}
	while (*str) {
		char c = *str++;
		if ((c >= 'a') && (c <= 'z'));
		else if ((c >= 'A') && (c <= 'Z'));
		else if (c == '_');
		else {
			error("'%s' is not a valid identifier",id.c_str());
			return false;
		}
	}
	return true;
}


static string resolve_esc(const string &s)
{
	string r;
	const char *str = s.c_str();
	while (char c = *str++) {
		if (c == '\\') {
			switch (char e = *str++) {
			case 't':
				r += '\t';
				break;
			case 'n':
				r += '\n';
				break;
			case 'r':
				r += '\r';
				break;
			case '\\':
				r += '\\';
				break;
			default:
				r += '\\';
				r += e;
			}
		} else {
			r += c;
		}
	}
	return r;
}


Generator::Generator(ostream &str, const Options *o)
: m_out(str)
, m_indent(0)
, m_errid(0)
, m_skipBrace(0)
, m_options(o)
, m_message(0)
, m_field(0)
, m_enum(0)
, m_skipAsserts(false)
, m_ind1line(false)
, m_afternl(false)
, m_folder(new FoldCompounds)
{
	if (o == 0)
		m_options = Options::getDefaults();
	m_skipAsserts = !m_options->getFlag("Asserts");
	const string &vib = m_options->getOption("VarIntBits");
	addVariable("VarIntBits",vib);
	//addVariable("wiresize_s",(string)"wiresize_s"+vib);
	//addVariable("wiresize_u",(string)"wiresize_u"+vib);
	addVariable("wiresize_s",(string)"wiresize_s");
	addVariable("wiresize_x",(string)"wiresize_x");
	addVariable("wiresize_u",(string)"wiresize");
	addVariable("bytestype",m_options->getOption("bytestype"));
	addVariable("stringtype",m_options->getOption("stringtype"));
	addVariable("json_indent",resolve_esc(m_options->getOption("json_indent")));
	addVariable("streamtype",resolve_esc(m_options->getOption("streamtype")));
	addVariable("varintbits",m_options->getOption("VarIntBits"));

	string sizeBaseType = o->getOption("ssize_t");
	if (toIdentifier(sizeBaseType))
		setVariable("ssize_t",sizeBaseType);
	else
		setVariable("ssize_t","");

	setVariable("BaseClass","BaseClass");
	string BaseClass = o->getOption("BaseClass");
	if (toIdentifier(BaseClass))
		setVariable("BaseClass",BaseClass);
	else if (BaseClass == "")
		setVariable("BaseClass","");
	else
		warn("ignoring invalid identifier name '%s' for method BaseClass",BaseClass.c_str());

	setVariable("toMemory","toMemory");
	string toMemory = o->getOption("toMemory");
	if (toIdentifier(toMemory))
		setVariable("toMemory",toMemory);
	else if (toMemory == "")
		setVariable("toMemory","");
	else
		warn("ignoring invalid identifier name '%s' for method toMemory",toMemory.c_str());

	setVariable("toSink","toSink");
	string toSink = o->getOption("toSink");
	if (toIdentifier(toSink))
		setVariable("toSink",toSink);
	else if (toSink == "")
		setVariable("toSink","");
	else
		warn("ignoring invalid identifier name '%s' for method toSink",toSink.c_str());

	setVariable("toString","toString");
	string toString = o->getOption("toString");
	if (toIdentifier(toString))
		setVariable("toString",toString);
	else if (toString == "")
		setVariable("toString","");
	else
		warn("ignoring invalid identifier name '%s' for method toString",toString.c_str());

	setVariable("toWire","toWire");
	string toWire = o->getOption("toWire");
	if (toIdentifier(toWire))
		setVariable("toWire",toWire);
	else if (toWire == "")
		setVariable("toWire","");
	else
		warn("ignoring invalid identifier name '%s' for method toWire",toWire.c_str());

	setVariable("toASCII","toASCII");
	string toASCII = o->getOption("toASCII");
	if (toIdentifier(toASCII))
		setVariable("toASCII",toASCII);
	else if (toASCII == "")
		setVariable("toASCII","");
	else
		warn("ignoring invalid identifier name '%s' for method toASCII",toASCII.c_str());

	setVariable("toJSON","toJSON");
	string toJSON = o->getOption("toJSON");
	if (toIdentifier(toJSON))
		setVariable("toJSON",toJSON);
	else if (toJSON == "")
		setVariable("toJSON","");
	else
		warn("ignoring invalid identifier name '%s' for method toJSON",toJSON.c_str());

	setVariable("fromMemory","fromMemory");
	string fromMemory = o->getOption("fromMemory");
	if (toIdentifier(fromMemory))
		setVariable("fromMemory",fromMemory);
	else if (fromMemory == "")
		setVariable("fromMemory","");
	else 
		warn("ignoring invalid identifier name '%s' for method formMemory",fromMemory.c_str());

	setVariable("calcSize","calcSize");
	string calcSize = o->getOption("calcSize");
	if (toIdentifier(calcSize))
		setVariable("calcSize",calcSize);
	else if (calcSize == "")
		setVariable("calcSize","");
	else
		warn("ignoring invalid identifier name '%s' for method calcSize",calcSize.c_str());

	setVariable("getMaxSize","getMaxSize");
	string getMaxSize = o->getOption("getMaxSize");
	if (toIdentifier(getMaxSize))
		setVariable("getMaxSize",getMaxSize);
	else if (getMaxSize == "")
		setVariable("getMaxSize","");
	else
		warn("ignoring invalid identifier name '%s' for method getMaxSize",getMaxSize.c_str());

	setMode(gen_wire);
}


Generator::~Generator()
{
	Indenter ind;
	ind.process(m_folder->getOutput().c_str());
	m_out << ind.getOutput();
}


void Generator::setMode(genmode_t g)
{
	string wireput = m_options->getOption("wireput");
	setVariable("sink_template","");
	if (g == gen_sink) {
		setVariable("wireput","s.sink8($1)");	// explicit function given to call
		setVariable("putarg","s");
		setVariable("putparam","Sink &s");
		setVariable("write_varint","s.sink_vi($1)");
		setVariable("write_xvarint","s.sink_vi((uint64_t)(int64_t)$1)");
		setVariable("u16_wire","s.sink16($1)");
		setVariable("u32_wire","s.sink32($1)");
		setVariable("u64_wire","s.sink64($1)");
		setVariable("write_bytes","s.sink_bytes($1,$2)");
		setVariable("toX",getVariable("toSink"));
		if (m_options->getFlag("SinkToTemplate"))
			setVariable("sink_template","template <class Sink> ");
	} else if (g == gen_string) {
		setVariable("wireput","s.push_back($1)");	// explicit function given to call
		setVariable("putarg","s");
		setVariable("putparam","std::string &s");
		setVariable("write_varint","send_varint(s,$1)");
		setVariable("write_xvarint","send_xvarint(s,$1)");
		setVariable("u16_wire","send_u16(s,$1)");
		setVariable("u32_wire","send_u32(s,$1)");
		setVariable("u64_wire","send_u64(s,$1)");
		setVariable("write_bytes","s.append((const char *)$1,$2)");
		setVariable("toX",getVariable("toString"));
	} else if (toIdentifier(wireput)) {
		setVariable("wireput",wireput);	// explicit function given to call
		setVariable("putarg","");
		setVariable("putparam","");
		setVariable("write_varint","send_varint($1)");
		setVariable("write_xvarint","send_xvarint($1)");
		setVariable("u16_wire","send_u16($1)");
		setVariable("u32_wire","send_u32($1)");
		setVariable("u64_wire","send_u64($1)");
		setVariable("write_bytes","send_bytes($1,$2)");
		setVariable("toX",m_options->getOption("toWire"));
	} else {
		setVariable("wireput","put");	// name of function argument
		setVariable("putarg","put");
		setVariable("putparam","void (*put)(uint8_t)");
		setVariable("write_varint","send_varint($wireput,$1)");
		setVariable("write_xvarint","send_xvarint($wireput,$1)");
		setVariable("u16_wire","send_u16($wireput,$1)");
		setVariable("u32_wire","send_u32($wireput,$1)");
		setVariable("u64_wire","send_u64($wireput,$1)");
		setVariable("write_bytes","send_bytes($wireput,$1,$2)");
		setVariable("toX",m_options->getOption("toWire"));
	}
}


void Generator::addVariable(const string &n, const string &v)
{
	//diag("addVariable(%s,%s)",n.c_str(),v.c_str());
	auto r = m_vars.insert(make_pair(n,v));
	assert(r.second);
}


void Generator::clearVariable(const string &n)
{
	auto i = m_vars.find(n);
	if (i != m_vars.end())
		m_vars.erase(i);
}


void Generator::setVariable(const string &n, const string &v)
{
	dbug("Generator::setVariable(%s,%s)",n.c_str(),v.c_str());
	auto r = m_vars.insert(make_pair(n,v));
	r.first->second = v;
	/*
	auto i = m_vars.find(n);
	assert(i != m_vars.end());
	i->second = v;
	*/
}


void Generator::setVariableHex(const string &n, uint64_t v)
{
	dbug("Generator::setVariable(%s,%ld)",n.c_str(),v);
	char buf[64];
	int l = snprintf(buf,sizeof(buf),"0x%llx",(unsigned long long)v);
	auto r = m_vars.insert(pair<string,string>(n,string(buf,l)));
	r.first->second = string(buf,l);
}


void Generator::setVariableDec(const string &n, int64_t v)
{
	dbug("Generator::setVariable(%s,%ld)",n.c_str(),v);
	char buf[64];
	int l = snprintf(buf,sizeof(buf),"%lld",(long long)v);
	auto r = m_vars.insert(pair<string,string>(n,string(buf,l)));
	r.first->second = string(buf,l);
}


void Generator::setMessage(const Message *m)
{
	if (m) {
		assert(m_message == 0);
		setVariable("msg_name", m->getName());
		setVariable("msg_fullname", m->getPrefix()+m->getName());
		setVariable("msg_clear", m_options->getOption("ClearName"));
		setVariable("prefix", m->getPrefix());
		setVariable("validtype",m->getValidType());
		setVariableDec("numvalidbytes",m->getNumValid()/8+((m->getNumValid()&7)?1:0));
	} else {
		clearVariable("msg_name");
		clearVariable("msg_fullname");
		clearVariable("msg_clear");
		clearVariable("prefix");
	}
	m_message = m;
}


void Generator::setEnum(const Enum *e)
{
	if (e) {
		assert(m_enum == 0);
		addVariable("ename",e->getName());
		addVariable("efullname",e->getFullName());
		addVariable("eprefix",e->getPrefix());
		addVariable("strfun",e->getStringFunction());
	} else {
		clearVariable("ename");
		clearVariable("efullname");
		clearVariable("eprefix");
		clearVariable("strfun");
	}
	m_enum = e;
}


/*
const char *Generator::getTypeName(uint32_t type) const
{
	if ((type & ft_filter) == ft_msg) 
		return Message::resolveId(type).c_str();
	ICE("unable to resolve type %x",type);
	return 0;
}
*/


void Generator::field_fill(const string &arg)
{
	const Field *f = m_field;
	assert(f);
	uint32_t tid = f->getTypeClass();

	switch (f->getQuantifier()) {
	case q_optional:
		switch (tid) {
		case ft_cptr:
			(*this) << "$(field_set)(" << arg << ");\n";
			break;
		case ft_msg:
			(*this) << "n = $(m_field).$(fromMemory)(" << arg << ");\n";
			break;
		case ft_bytes:
		case ft_string:
			(*this) << "$(m_field).assign(" << arg << ");\n";
			break;
		case ft_unsigned:
		case ft_int:
		case ft_int16:
		case ft_uint16:
		case ft_int32:
		case ft_uint32:
		case ft_int64:
		case ft_uint64:
		case ft_signed:
		case ft_sint16:
		case ft_sint32:
		case ft_sint64:
		case ft_bool:
		case ft_int8:
		case ft_uint8:
		case ft_sint8:
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
			(*this) << "$(field_set)(" << arg << ");\n";
			break;
		case ft_enum:
			(*this) << "$(field_set)(($(typestr)) " << arg << ");\n";
			break;
		default:
			ICE("missing field type 0x%x",tid);
		}
		break;
	case q_required:
		switch (tid) {
		case ft_cptr:
			(*this) << "$(m_field) = " << arg << ";\n";
			break;
		case ft_msg:
			(*this) << "n = $(m_field).$(fromMemory)(" << arg << ");\n";
			break;
		case ft_bytes:
		case ft_string:
			(*this) << "$(m_field).assign(" << arg << ");\n";
			break;
		case ft_unsigned:
		case ft_int:
		case ft_int16:
		case ft_uint16:
		case ft_int32:
		case ft_uint32:
		case ft_int64:
		case ft_uint64:
		case ft_signed:
		case ft_sint16:
		case ft_sint32:
		case ft_sint64:
		case ft_bool:
		case ft_int8:
		case ft_uint8:
		case ft_sint8:
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
			(*this) << "$(m_field) = " << arg << ";\n";
			break;
		case ft_enum:
			(*this) << "$(field_set)(($typestr) " << arg << ");\n";
			break;
		default:
			ICE("missing field type 0x%x",tid);
		}
		break;
	case q_repeated:
		switch (tid) {
		case ft_cptr:
			(*this) << "$(m_field).push_back(" << arg << ");\n";
			break;
		case ft_msg:
			(*this) <<	"n = $(m_field).back().$(fromMemory)(" << arg << ");\n";
			break;
		case ft_bytes:
			(*this) << "$(m_field).push_back($(bytestype)(" << arg << "));\n";
			break;
		case ft_string:
			(*this) << "$(m_field).push_back($(stringtype)(" << arg << "));\n";
			break;
		case ft_unsigned:
		case ft_int:
		case ft_int16:
		case ft_uint16:
		case ft_int32:
		case ft_uint32:
		case ft_int64:
		case ft_uint64:
		case ft_signed:
		case ft_sint16:
		case ft_sint32:
		case ft_sint64:
		case ft_bool:
		case ft_int8:
		case ft_uint8:
		case ft_sint8:
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
			(*this) << "$(m_field).push_back(" << arg << ");\n";
			break;
		case ft_enum:
			(*this) << "$(m_field).push_back(($(typestr))" << arg << ");\n";
			break;
		default:
			ICE("missing field type 0x%x",tid);
		}
		break;
	default:
		abort();
	}
}


void Generator::setField(const Field *f)
{
	m_field = f;
	if (f) {
		string fname = f->getName();
		uint32_t tid = f->getType();
		assert(m_options);
		addVariable("fname",fname);
		addVariable("m_field","m_"+fname);	// member reference
		addVariable("p_field","&m_"+fname);	// member pointer
		switch (f->getQuantifier()) {
		case q_required:
			if ((tid == ft_string) || (tid == ft_bytes))
				addVariable("field_push","m_"+fname+".assign");	// added member pointer
			else
				addVariable("field_push","m_"+fname+".set");
			break;
		case q_optional:
			if ((tid == ft_string) || (tid == ft_bytes))
				addVariable("field_push","m_"+fname+".assign");	// added member pointer
			else
				addVariable("field_push","m_"+fname+".set");
			break;
		case q_repeated:
			if ((tid == ft_string) || (tid == ft_bytes))
				addVariable("field_push","m_"+fname+".assign");	// added member pointer
			else
				addVariable("field_push","m_"+fname+".push_back");
			break;
		default:
			abort();
		}
		setVariable("bytestype",f->getOption("bytestype"));
		setVariable("stringtype",f->getOption("stringtype"));
		addVariable("field_add",m_options->AddPrefix() + fname);
		addVariable("field_has",m_options->HasPrefix() + fname);
		addVariable("field_get",m_options->GetPrefix() + fname);
		addVariable("field_set",m_options->SetPrefix() + fname);
		addVariable("field_clear",m_options->ClearPrefix() + fname);
		addVariable("field_mutable",m_options->MutablePrefix() + fname);
		/*
		if (m_message->getNumValid() <= VarIntBits)
			addVariable("field_valid","p_validbits |= (($(validtype))1U << $(vbit));\n");
		else
			addVairable("field_valid","p_validbits[$(vbit/8)] |= (($(validtype))1U << ($(vbit)&0x7));\n");
			*/
		char tmp[64];
		unsigned id = f->getId();
		sprintf(tmp,"%u",id);
		addVariable("field_id",tmp);
		unsigned enc = f->getEncoding();
		sprintf(tmp,"%u",enc);
		addVariable("field_enc",tmp);
		sprintf(tmp,"0x%x",(id<<3)|enc);
		addVariable("field_tag",tmp);
		const char *typestr = f->getTypeName();
		assert(typestr);
		const char *fulltype = typestr;
		if ((tid & ft_filter) == ft_msg) {
			assert((tid & ft_filter) == ft_msg);
			Message *m = Message::id2msg(tid);
			assert(m);
			typestr = m->getName().c_str();
			fulltype = m->getFullname().c_str();
			assert(m->getFullname().size()>0);
			//dbug("fulltype-name %s",fulltype);
		} else if ((tid & ft_filter) == ft_enum) {
			Enum *e = Enum::id2enum(tid);
			typestr = e->getName().c_str();
			fulltype = e->getFullName().c_str();
			//dbug("fulltype-name0 %s",fulltype);
		}
		addVariable("typestr", typestr);
		addVariable("fulltype", fulltype);
		int vbit = f->getValidBit();
		if (vbit >= 0) {
			char buf[16];
			sprintf(buf,"%d",vbit);
			setVariable("vbit",buf);
		} else
			clearVariable("vbit");
		string rtype,fullrtype;
		if (m_field->hasEnumType()) {
			rtype = typestr;
			fullrtype = fulltype;
			//dbug("fulltype-name %s",fulltype);
			rtype += ' ';
			fullrtype += ' ';
		} else if (m_field->hasSimpleType()) {
			rtype = typestr;
			fullrtype = typestr;
			rtype += ' ';
			fullrtype += ' ';
		} else if (0 == strcmp(typestr,"const char *")) {
			rtype = typestr;
			fullrtype = typestr;
		} else {
			rtype = "const ";
			rtype += typestr;
			rtype += " &";
			fullrtype = "const ";
			fullrtype += fulltype;
			fullrtype += " &";
			//diag("fullrtype: %s",fullrtype.c_str());
		}
		addVariable("rtype", rtype);
		addVariable("fullrtype", fullrtype);
		string ptype = typestr;
		ptype += " *";
		addVariable("ptype",ptype);
		sprintf(tmp,"%u",f->getTagSize());
		addVariable("tagsize",tmp);
		if (Enum *e = f->toEnum())
			setEnum(e);
	} else {
		clearVariable("fname");
		clearVariable("m_field");
		clearVariable("p_field");
		clearVariable("typestr");
		clearVariable("rtype");
		clearVariable("fulltype");
		clearVariable("fullrtype");
		clearVariable("ptype");
		clearVariable("vbit");
		clearVariable("field_push");
		clearVariable("field_add");
		clearVariable("field_has");
		clearVariable("field_get");
		clearVariable("field_set");
		clearVariable("field_clear");
		clearVariable("field_mutable");
		clearVariable("field_id");
		clearVariable("field_enc");
		clearVariable("field_tag");
		clearVariable("tagsize");
		setVariable("bytestype",m_options->getOption("bytestype"));
		setVariable("stringtype",m_options->getOption("stringtype"));
		setEnum(0);
	}
}


bool Generator::hasVariable(const string &n) const
{
	auto i = m_vars.find(n);
	return (i != m_vars.end());
}


bool Generator::hasValue(const string &n) const
{
	auto i = m_vars.find(n);
	if (i == m_vars.end())
		return false;
	return i->second != "";
}


const char *Generator::getVariable(const string &n)
{
	const char *nstr = n.c_str();
	if ((nstr[0] >= '0') && (nstr[0] <= '9'))
		return nstr;
	auto i = m_vars.find(n);
	if (i != m_vars.end()) {
		//assert(i->second.size() > 0);
		return i->second.c_str();
	}
	/*
	if (n == "handle_error") {
		switch (errmode) {
		case ret_errid:
			--m_errid;
			char buf[16];
			snprintf(buf,sizeof(buf),"return %d",m_errid);
			m_errstr = buf;
			return m_errstr.c_str();
		case abort_exe:
			return "abort()";
		case throw_errid:
			--m_errid;
			char buf[16];
			snprintf(buf,sizeof(buf),"throw %d",m_errid);
			m_errstr = buf;
			return m_errstr.c_str();
		default:
			abort();
	}
	*/
	ICE("undefined variable $(%s)",n.c_str());
	return n.c_str();
}


static const char *opfind(const char *arg)
{
	const char *plus = strchr(arg,'+');
	const char *minus = strchr(arg,'-');
	const char *mult = strchr(arg,'*');
	const char *div = strchr(arg,'/');
	const char *mod = strchr(arg,'%');
	const char *b_and = strchr(arg,'&');
	if (plus) {
		assert((minus == 0) && (mult == 0) && (div == 0) && (mod == 0) && (b_and == 0));
		return plus;
	}
	if (minus) {
		assert((plus == 0) && (mult == 0) && (div == 0) && (mod == 0) && (b_and == 0));
		return minus;
	}
	if (mult) {
		assert((plus == 0) && (minus == 0) && (div == 0) && (mod == 0) && (b_and == 0));
		return mult;
	}
	if (div) {
		assert((plus == 0) && (minus == 0) && (mult == 0) && (mod == 0) && (b_and == 0));
		return div;
	}
	if (mod) {
		assert((plus == 0) && (minus == 0) && (mult == 0) && (div == 0) && (b_and == 0));
		return mod;
	}
	if (b_and) {
		assert((plus == 0) && (minus == 0) && (mult == 0) && (div == 0) && (mod == 0));
		return b_and;
	}
	return 0;
}


string Generator::evaluate(const string &a)
{
	dbug("Generator::evaluate(%s)",a.c_str());
	const char *p0 = a.c_str();
	const char *op = opfind(p0);
	if (op) {
		string a0(p0,op);
		string a1(op+1);
		long long v0 = strtoll(getVariable(a0),0,0);
		long long v1 = strtoll(getVariable(a1),0,0);
		long long r = 0;
		switch (*op) {
		case '+':
			r = v0 + v1;
			break;
		case '-':
			r = v0 - v1;
			break;
		case '*':
			r = v0 * v1;
			break;
		case '/':
			r = v0 / v1;
			break;
		case '%':
			r = v0 % v1;
			break;
		case '&':
			r = v0 & v1;
			break;
		default:
			ICE("invalid operator %c",*op);
		}
		char tmp[64];
		sprintf(tmp,"%lld",r);
		return string(tmp);
	}
	return getVariable(a);
}


/*
static const char *unfilteredEnd(const char *t)
{
	const char *e = t + strlen(t);
	const char *d = strchr(t,'$');
	const char *l = strchr(t,'{');
	const char *r = strchr(t,'}');
	const char *n = strchr(t,'\n');
	if (d && (d < e))
		e = d;
	if (l && (l < e))
		e = l;
	if (r && (r < e))
		e = r;
	if (n && (n < e))
		e = n;
	return e;
}
*/


void Generator::indentingWrite(const char *t, const char *e)
{
	errmode_t em = em_invalid;
	const string &errmode = m_options->getOption("ErrorHandling");
	if (errmode == "cancel")
		em = em_ret_errid;
	else if (errmode == "throw")
		em = em_throw_errid;
	else if (errmode == "assert")
		em = em_abort_exe;
	else
		abort();
	Evaluator eval(m_vars,m_errid,em);
	while (t < e) {
		const char *d = (const char *) memchr(t,'$',e-t);
		if (d && (d != t) && (d[-1] == '\\')) {
			// skip over \$, needed for $(WFC_ROOT)

		}
		const char *n = (const char *) memchr(t,'\n',e-t);
		if (d && ((d < n) || (n == 0))) {
			m_linebuf += string(t,d);
			//dbug("linebuf0: '%s'",m_linebuf.c_str());
			string rol;
			if (n)
				rol = string(d,n);
			else
				rol = d;
			eval.evaluate(rol);
			m_linebuf += rol;
			if (n)
				t = n;
			else
				t = e;
			//dbug("linebuf1: '%s'",m_linebuf.c_str());
			continue;
		} else if (n == 0) {
			m_linebuf += string(t,e);
			//dbug("linebuf2: '%s'",m_linebuf.c_str());
			return;
		} else if (n < d) {
			m_linebuf += string(t,n+1);
			//dbug("linebuf4: '%s'",m_linebuf.c_str());
		} else {
			m_linebuf += string(t,n+1);
			//dbug("linebuf3: '%s'",m_linebuf.c_str());
		}
		const char *line = m_linebuf.c_str();
		assert(line[strlen(line)-1] == '\n');
		if (m_skipAsserts && (0 == strncmp(line,"assert(",7))) {
			assert(m_ind1line == false);
			m_linebuf.clear();
			t = n + 1;
			continue;
		}
		//m_out << m_linebuf;
		m_folder->process(m_linebuf.c_str());
		m_linebuf.clear();
		t = n + 1;
		m_afternl = true;
	}

}


/*
static unsigned chrcnt(const char *str, const char *end, char c)
{
	unsigned n = 0;
	str = (const char *) memchr(str,c,end-str);
	while (str) {
		++n;
		++str;
		str = (const char *) memchr(str,c,end-str);
	}
	return n;
}


void Generator::indentingWrite(const char *t, const char *e)
{
	Evaluator eval(m_vars);
	string rol;
	while (t < e) {
		if (m_afternl) {
			while (*t == '\t')
				++t;
			m_afternl = false;
		}
		const char *d = (const char *) memchr(t,'$',e-t);
		const char *n = (const char *) memchr(t,'\n',e-t);
		if (d && ((d < n) || (n == 0))) {
			m_linebuf += string(t,d);
			dbug("linebuf0: '%s'",m_linebuf.c_str());
			if (n)
				rol = string(d,n);
			else
				rol = d;
			eval.evaluate(rol);
			m_linebuf += rol;
			if (n)
				t = n;
			else
				t = e;
			dbug("linebuf1: '%s'",m_linebuf.c_str());
			continue;
		} else if (n == 0) {
			m_linebuf += string(t,e);
			dbug("linebuf2: '%s'",m_linebuf.c_str());
			return;
		} else if (n < d) {
			m_linebuf += string(t,n+1);
			dbug("linebuf4: '%s'",m_linebuf.c_str());
		} else {
			m_linebuf += string(t,n+1);
			dbug("linebuf3: '%s'",m_linebuf.c_str());
		}
		const char *line = m_linebuf.c_str();
		const char *eol = line + m_linebuf.size();
		assert(line[strlen(line)-1] == '\n');
		if (m_skipAsserts && (0 == strncmp(line,"assert(",7))) {
			assert(m_ind1line == false);
			m_linebuf.clear();
			t = n + 1;
			continue;
		}
		unsigned rb = chrcnt(line,eol,'}');
		//assert(rb < 2);
		unsigned lb = chrcnt(line,eol,'{');
		assert(lb < 2);
		if ((rb == 1) && (lb == 1))
			rb = lb = 0;
		m_indent -= rb;
		if (m_indent < 0) {
			m_out.flush();
			error("unmatched '}'");
			m_indent = 0;
		}
		int indent = m_indent;
		if (0 == strncmp(line,"case ",5))
			--indent;
		else if (0 == strncmp(line,"default:",8))
			--indent;
		else if (0 == strncmp(line,"} else {",8))
			--indent;
		if (indent < 0)
			indent = 0;
		if (m_ind1line) {
			++indent;
			m_ind1line = false;
		}
		if (m_indent <= 16)
			m_out.write("\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t",indent);
		else 
			for (int i = 0; i < indent; ++i)
				m_out.put('\t');
		if ((0 == lb) && ((0 == strncmp(line,"if (",4)) || (0 == strncmp(line,"while (",7)) || (0 == strncmp(line,"for (",5))))
			m_ind1line = true;
		else if (strstr(line,"if ("))
			diag("unmatched if(: '%s'",line);
		m_indent += lb;
		if (lb || rb)
			dbug("m_indent = %d",m_indent);
		m_out << m_linebuf;
		m_linebuf.clear();
		t = n + 1;
		m_afternl = true;
	}
}
*/


Generator &operator << (Generator &g, const char *t)
{
	g.indentingWrite(t,t+strlen(t));
	return g;
}


Generator &operator << (Generator &g, const string &s)
{
	const char *cstr = s.c_str();
	g.indentingWrite(cstr,cstr+s.size());
	return g;
}


/*
Generator &operator << (Generator &g, uint32_t u)
{
	char buf[32];
	int n = sprintf(buf,"%u",u);
	g.indentingWrite(buf,buf+n);
	return g;
}


Generator &operator << (Generator &g, int32_t u)
{
	char buf[32];
	int n = sprintf(buf,"%d",u);
	g.indentingWrite(buf,buf+n);
	return g;
}

Generator &operator << (Generator &g, uint64_t u)
{
	char buf[32];
	int n = sprintf(buf,"%ld",u);
	g.indentingWrite(buf,buf+n);
	return g;
}

Generator &operator << (Generator &g, int64_t u)
{
	char buf[32];
	int n = sprintf(buf,"%ld",u);
	g.indentingWrite(buf,buf+n);
	return g;
}

*/
