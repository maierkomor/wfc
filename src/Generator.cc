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
	char c = *str++;
	if ((c >= 'a') && (c <= 'z'));
	else if ((c >= 'A') && (c <= 'Z'));
	else if (c == '_');
	else {
		error("'%s' is not a valid identifier",id.c_str());
		return false;
	}
	while (*str) {
		c = *str++;
		if ((c >= 'a') && (c <= 'z'));
		else if ((c >= 'A') && (c <= 'Z'));
		else if ((c >= '0') && (c <= '9'));
		else if (c == '_');
		else {
			error("'%s' is not a valid identifier",id.c_str());
			return false;
		}
	}
	return true;
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
, m_ind1line(false)
, m_afternl(false)
, m_inComment(false)
{
	if (o == 0)
		m_options = Options::getDefaults();
	m_skipAsserts = !m_options->getFlag("Asserts");
	m_skipComments = !m_options->getFlag("comments");
	m_folder = new FoldCompounds(!m_skipComments);
	const string &vib = m_options->getOption("VarIntBits");
	addVariable("VarIntBits",vib);
	addVariable("wiresize_s","wiresize_s");
	addVariable("wiresize_x","wiresize_x");
	addVariable("wiresize_u","wiresize");
	addVariable("bytestype",o->getOption("bytestype"));
	addVariable("stringtype",o->getOption("stringtype"));
	addVariable("varintbits",o->getOption("VarIntBits"));
	addVariable("set_by_name",o->getOption("SetByName"));
	addVariable("ascii_indent",o->getIdentifier("ascii_indent"));
	addVariable("sink_template","");

	addVariable("streamtype",o->getIdentifier("streamtype"));
	addVariable("ssize_t",o->getIdentifier("ssize_t"));
	addVariable("BaseClass",o->getIdentifier("BaseClass"));
	addVariable("toMemory",o->getIdentifier("toMemory"));
	addVariable("toSink",o->getIdentifier("toSink"));
	addVariable("toString",o->getIdentifier("toString"));
	addVariable("toWire",o->getIdentifier("toWire"));
	addVariable("toASCII",o->getIdentifier("toASCII"));
	addVariable("toJSON",o->getIdentifier("toJSON"));
	addVariable("fromMemory",o->getIdentifier("fromMemory"));
	addVariable("calcSize",o->getIdentifier("calcSize"));
	addVariable("getMaxSize",o->getIdentifier("getMaxSize"));
	addVariable("wireput","");
	addVariable("putarg","");
	addVariable("putparam","");
	addVariable("write_varint","");
	addVariable("write_xvarint","");
	addVariable("u16_wire","");
	addVariable("u32_wire","");
	addVariable("u64_wire","");
	addVariable("write_bytes","");
	addVariable("toX","");
	addVariable("inline","");

	setMode(gen_wire);
}


Generator::~Generator()
{
	indent_code(m_out,m_folder->getOutput().c_str());
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
		setVariable("wireput","put.push_back($1)");	// explicit function given to call
		setVariable("putarg","put");
		setVariable("putparam","$stringtype &put");
		setVariable("write_varint","send_varint(put,$1)");
		setVariable("write_xvarint","send_xvarint(put,$1)");
		setVariable("u16_wire","send_u16(put,$1)");
		setVariable("u32_wire","send_u32(put,$1)");
		setVariable("u64_wire","send_u64(put,$1)");
		setVariable("write_bytes","put.append((const char *)$1,$2)");
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


void Generator::addVariable(const string &n, const char *v)
{
	//diag("addVariable(%s,%s)",n.c_str(),v.c_str());
	auto r = m_vars.insert(make_pair(n,v ? v : ""));
	assert(r.second);
}


void Generator::clearVariable(const string &n)
{
	auto i = m_vars.find(n);
	if (i != m_vars.end())
		m_vars.erase(i);
}


void Generator::setVariable(const string &n, const char *v)
{
	if (v == 0)
		v = "";
	dbug("Generator::setVariable(%s,%s)",n.c_str(),v);
	auto r = m_vars.insert(pair<string,string>(n,v));
	//assert(r.second == false);
	if (r.second)
		cerr << n << ',' << v << endl;
	r.first->second = v;
}


void Generator::setVariable(const string &n, const string &v)
{
	dbug("Generator::setVariable(%s,%s)",n.c_str(),v.c_str());
	auto r = m_vars.insert(make_pair(n,v));
	assert(r.second == false);
	r.first->second = v;
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
		addVariable("msg_name", m->getName());
		addVariable("msg_fullname", m->getPrefix()+m->getName());
		addVariable("msg_clear", m_options->getOption("ClearName"));
		addVariable("prefix", m->getPrefix());
		addVariable("validtype",m->getValidType());
		setVariableDec("numvalidbytes",m->getNumValid()/8+((m->getNumValid()&7)?1:0));
	} else {
		clearVariable("msg_name");
		clearVariable("msg_fullname");
		clearVariable("msg_clear");
		clearVariable("prefix");
		clearVariable("validtype");
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


void Generator::fillField(const string &arg)
{
	const Field *f = m_field;
	assert(f);
	uint32_t tid = f->getTypeClass();
	bool vmember = f->isVirtual();

	if (vmember) {
		const char *cast = "";
		if (tid == ft_enum)
			cast = "($typestr)";
		switch (f->getQuantifier()) {
		case q_optional:
		case q_required:
			(*this) << "$(field_set)(" << cast << arg << ");\n";
			break;
		case q_repeated:
			(*this) << "add_$(fname)(" << cast << arg << ");\n";
			break;
		default:
			abort();
		}
		return;
	}
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
			(*this) << "$(m_field) = ($typestr) " << arg << ";\n";
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
			(*this) << "n = $(m_field).back().$(fromMemory)(" << arg << ");\n";
			break;
		case ft_bytes:
			(*this) << "$(m_field).emplace_back(" << arg << ");\n";
			break;
		case ft_string:
			(*this) << "$(m_field).emplace_back(" << arg << ");\n";
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
		assert(m_options);
		string fname = f->getName();
		uint32_t tid = f->getType();
		addVariable("fname",fname);
		char fnamelen[8];
		sprintf(fnamelen,"%lu",fname.size());
		addVariable("fnamelen",fnamelen);
		addVariable("m_field","m_"+fname);	// member reference
		quant_t q = f->getQuantifier();
		setVariable("bytestype",f->getOption("bytestype"));
		const string &st = f->getOption("stringtype");
		if (st == "pointer")
			setVariable("stringtype","char *");
		else
			setVariable("stringtype",st);
		addVariable("field_add",m_options->AddPrefix() + fname);
		addVariable("field_has",m_options->HasPrefix() + fname);
		addVariable("field_get",m_options->GetPrefix() + fname);
		addVariable("field_set",m_options->SetPrefix() + fname);
		addVariable("field_clear",m_options->ClearPrefix() + fname);
		addVariable("field_mutable",m_options->MutablePrefix() + fname);
		bool v = f->isVirtual();
		if (v && q == q_repeated) {
			addVariable("field_value",m_options->GetPrefix()+fname+"($index)");
			addVariable("field_size",fname+"_size()");
		} else if (v) {
			addVariable("field_value",m_options->GetPrefix()+fname+"()");
			if ((tid == ft_string) || (tid == ft_bytes))
				addVariable("field_size",fname+"_size()");
		} else if (q == q_repeated) {
			addVariable("field_value","m_"+fname+"[$index]");
			addVariable("field_size","m_"+fname+".size()");
		} else {
			addVariable("field_value","m_"+fname);
		}
		addVariable("field_values",v ? "$(field_get)().data()" : "m_$(fname).data()");

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
			addVariable("vbit",buf);
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
		addVariable("parse_ascii",f->getParseAsciiFunction());
		if (Enum *e = f->toEnum())
			setEnum(e);
	} else {
		clearVariable("fname");
		clearVariable("fnamelen");
		clearVariable("m_field");
		clearVariable("typestr");
		clearVariable("rtype");
		clearVariable("fulltype");
		clearVariable("fullrtype");
		clearVariable("ptype");
		clearVariable("vbit");
		clearVariable("field_add");
		clearVariable("field_clear");
		clearVariable("field_enc");
		clearVariable("field_get");
		clearVariable("field_has");
		clearVariable("field_id");
		clearVariable("field_mutable");
		clearVariable("field_push");
		clearVariable("field_set");
		clearVariable("field_size");
		clearVariable("field_tag");
		clearVariable("field_value");
		clearVariable("field_values");
		clearVariable("tagsize");
		clearVariable("parse_ascii");
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


