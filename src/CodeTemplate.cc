/*
 *  Copyright (C) 2017-2022, Thomas Maier-Komor
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

#include "CodeTemplate.h"
#include "Generator.h"
#include "Options.h"
#include "log.h"

#include <sstream>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

using namespace std;


const char *Functions[] = {
	"$invalid_template",
	"ascii_bool",
	"ascii_bytes",
	"ascii_indent",
	"ascii_numeric",
	"ascii_string",
	"json_cstr",
	"json_indent",
	"json_string",
	"mangle_double",
	"mangle_float",
	"decode_bytes",
	"decode_bytes_element",
	"decode_early",
	"decode_union",
	"parse_ascii_bool",
	"parse_ascii_bytes",
	"parse_ascii_dbl",
	"parse_ascii_flt",
	"parse_ascii_s16",
	"parse_ascii_s32",
	"parse_ascii_s64",
	"parse_ascii_s8",
	"parse_ascii_u16",
	"parse_ascii_u32",
	"parse_ascii_u64",
	"parse_ascii_u8",
	"parse_enum",
	"place_varint",
	"read_double",
	"read_float",
	"read_u16",
	"read_u32",
	"read_u64",
	"read_varint",
	"send_bytes",
	"send_msg",
	"send_u16",
	"send_u32",
	"send_u64",
	"send_varint",
	"send_xvarint",
	"sint_varint",
	"skip_content",
	"to_dblstr",
	"to_decstr",
	"varint_sint",
	"wiresize",
	"wiresize_s",
	"wiresize_x",
	"write_u16",
	"write_u32",
	"write_u64",
	"write_varint",
	"write_xvarint",
	"CStrLess",
	"encode_bytes",
	0
};


static const char *Parameters[] = {
	"asserts",
	"checks",
	"debug",
	"endian",
	"ErrorHandling",
	"intsize",
	"Optimize",
	"stringtype",
	"syntax",
	"UnknownField",
	"varintbits",
	0
};


static const char *MetaData[] = {
	"copyright",
	"description",
	"comment",
	0
};


extern const char *InstallDir;


static bool isOf(const char *p, const char **list)
{
	do {
		if (0 == strcasecmp(*list,p))
			return true;
		++list;
	} while (*list);
	return false;
}


CodeTemplate::CodeTemplate(char *f, char *sl, char *eoc, char *eofunc)
: function()
, variant()
, code()
, comment()
, filename()
{
	char *eof = f;
	do {
		++eof;
	} while (isalnum(*eof)||(*eof == '_'));
	function.assign(f,eof);
	char *t;
	char *p = strchr(eoc,'(');
	char *b = strchr(eoc,'{');
	if (b < p) {
		// this must be a union: '{' before '('
		char *u = strstr(eoc,"union");
		if (u && isspace(u[5])) {
			u += 6;
		} else {
			u = strstr(eoc,"struct");
			assert(u && isspace(u[6]));
			u += 7;
		}
		while (isspace(*u))
			++u;
		t = u;
		while (isalnum(*u)||(*u == '_'))
			++u;
		p = u;
	} else {
		assert(p);
		while (p[-1] == '$')
			p = strchr(p+1,'(');
		while (isspace(p[-1]))
			--p;
		t = p-1;
		while (isalnum(t[-1])||(t[-1] == '_'))
			--t;
	}
	variant = string(t,p);
	eoc += 2;
	comment = string(sl,eoc);
	comment += '\n';
	while (isspace(*eoc))
		++eoc;
	code = string(eoc,t);
	code += function;
	code += string(p,eofunc);
	*eoc = 0;
	diag("adding variant %s of function %s",variant.c_str(),function.c_str());
	char *nl = strchr(sl,'\n');
	while (nl) {
		char *line = nl + 1;
		char *eol = strchr(line,'\n');
		if (eol)
			*eol = 0;
		char *colon = strchr(line,':');
		if (colon == 0) {
			if (eol)
				*eol = '\n';
			nl = eol;
			continue;
		}
		while ((*line == ' ') || (*line == '*'))
			++line;
		char param[64];
		char value[256];
		if (2 == sscanf(line,"%63[A-Za-z_]:%255s",param,value)) {
			//diag("template %s parameter: %s = %s",variant.c_str(),param,value);
			if (isOf(param,Parameters))
				requirements.insert(make_pair(param,value));
			else if (isOf(param,MetaData))
				metadata.insert(make_pair(param,value));
			else if (!strcmp(param,"include"))
				includes.push_back(value);
			else if (!strcmp(param,"sysinclude"))
				sysincludes.push_back(value);
			else if (!strcmp(param,"requires"))
				addDependencies(value);
			else if (!strcmp(param,"function"));
			else if (!strcmp(param,"description"));
			else if (!strcmp(param,"include"));
			else if (!strcmp(param,"sysinclude"));
			else
				warn("ignoring unknown requirement %s of function %s",param,variant.c_str());
		}
		nl = eol;
	}
}

void CodeTemplate::writeComment(Generator &G) const
{
	if (G.skipComments())
		return;
	string fn;
	size_t il = strlen(InstallDir);
	if (0 == strncmp(filename.c_str(),InstallDir,il)) {
		fn = "(WFC_ROOT)/";
		fn += filename.c_str()+il;
	} else 
		fn = filename;
	G <<	"/* included from: " << fn << "\n"
		" * function:      " << function << "\n"
		" * variant:       " << variant << "\n";
	for (auto i = requirements.begin(), e = requirements.end(); i != e; ++i)
		G << " * " << i->first << ": " << i->second << '\n';
	for (auto i = metadata.begin(), e = metadata.end(); i != e; ++i)
		G << " * " << i->first << ": " << i->second << '\n';
	G << " */\n";
}


static const char *end_of_decl(const char *code)
{
	unsigned p = 0;
	const char *at = strchr(code,'(');
	if (at == 0)
		return 0;
	do {
		if (*at == ')')
			--p;
		else if (*at == '(')
			++p;
		else if (*at == 0)
			fatal("unable to find end of declaration in code template:\n%s",code);
		++at;
	} while (p != 0);
	return at;
}


string CodeTemplate::getDeclaration() const
{
	assert(isFunction());
	const char *c = code.c_str();
	string ret(c,end_of_decl(c));
	if (isFunction())
		ret += ';';
	ret += '\n';
	return ret;
}


codeid_t CodeTemplate::getFunctionId(const string &f)
{
	const char **fun = Functions;
	++fun;
	do {
		if (*fun == f)
			return (codeid_t)(fun-Functions);
		++fun;
	} while (*fun);
	return ct_invalid;
}


codeid_t CodeTemplate::getFunctionId() const
{
	return getFunctionId(function);
}


bool CodeTemplate::fitsOptions(const Options *o) const
{
	const string &v = o->getOption(function.c_str());
	if (v != "") {
		diag("fit functionname %s with %s",v.c_str(),variant.c_str());
		return (v == variant);
	}
	for (auto i(requirements.begin()),e(requirements.end()); i != e; ++i) {
		const string &param = i->first;
		const string &value = i->second;
		const string &setting = o->getOption(param.c_str());
		// TODO comma separated requirements
		assert(strchr(value.c_str(),',') == 0);
		if ((setting != "") && (setting != value)) {
			diag("match function %s: mismatch %s != %s",variant.c_str(),setting.c_str(),value.c_str());
			return false;
		}
	}
	return true;
}


bool CodeTemplate::isDefine() const
{
	const char *c = code.c_str();
	if (*c != '#')
		return false;
	++c;
	while ((*c == ' ') || (*c == '\t'))
		++c;
	if (strncmp(c,"define",6))
		return false;
	c += 6;
	if ((*c == '(') || (*c == ' ') || (*c == '\t'))
		return true;
	return false;
}


bool CodeTemplate::isTemplate() const
{
	const char *c = code.c_str();
	if (!strncmp(c,"template",8) && (isspace(c[8]) || (c[8] == '<')))
		return true;
	return false;
}


bool CodeTemplate::isUnion() const
{
	const char *c = code.c_str();
	if (!strncmp(c,"union",5) && isspace(c[5]))
		return true;
	return false;
}


bool CodeTemplate::isStruct() const
{
	const char *c = code.c_str();
	if (!strncmp(c,"struct",6) && isspace(c[6]))
		return true;
	return false;
}


bool CodeTemplate::isFunction() const
{
	if (isDefine())
		return false;
	if (isUnion())
		return false;
	if (isStruct())
		return false;
	if (isTemplate())
		return false;
	return true;
}


void CodeTemplate::write_h(Generator &G, libmode_t m)
{
	diag("write_h(%s)",function.c_str());
	if (isUnion() || isStruct()) {
		if (m != libstatic) {
			writeComment(G);
			G << code << ";\n\n\n";
		}
		return;
	}
	if (isDefine() || isTemplate()) {
		if (m != libstatic) {
			writeComment(G);
			G << code << "\n\n\n";
		}
		return;
	}
	assert(isFunction());

	const char *c = code.c_str();
	switch (m) {
	case libinline:
		writeComment(G);
		if (!isTemplate())
			G << "inline ";
		G << code << "\n\n\n";
		break;
	case libstatic:
		break;
	case libextern:
		if (omitdecl == false) {
			writeComment(G);
			if (isTemplate())
				G << code << "\n\n\n";
			else
				G << "extern " << string(c,end_of_decl(c)) << ";\n\n\n";
		}
		break;
	default:
		abort();
	}
}


static string getDefHeader(const char *dec, const char *eod)
{
	string ret(dec,eod);
	const char *str = ret.c_str();
	const char *e = strchr(str,'=');
	while (e) {
		while ((e != str) && ((*e == ' ') || (*e == '\t')))
			--e;
		const char *at = e;
		++at;
		char c = *at;
		while ((c != ',') && (c != ')') && (c != 0)) {
			++at;
			c = *at;
		}
		if (c == 0) {
			dbug("invalid default argument in template: '%s'",str);
			fatal("error handling default argument of template");
		}
		ret.erase(e-str,at-e);
		str = ret.c_str();
		e = strchr(str,'=');
	}
	return ret;
}


void CodeTemplate::write_cpp(Generator &G, libmode_t m)
{
	//dbug("write_cpp(%s)",function.c_str());
	if (isUnion() || isStruct()) {
		if (m == libstatic) {
			writeComment(G);
			G << code << ";\n\n\n";
		}
		return;
	}
	if (isDefine() || isTemplate()) {
		if (m == libstatic) {
			writeComment(G);
			G << code << "\n\n\n";
		}
		return;
	}
	assert(isFunction());
	if ((m == libinline) || ((m == libextern) && forceinline))
		return;
	writeComment(G);
	const char *c = code.c_str();
	const char *eod = end_of_decl(c);
	switch (m) {
	case libstatic:
		if ((!isTemplate()) && (omitdecl == false))
			G << "static ";
		G << code;
		break;
	case libextern:
		G << getDefHeader(c,eod);
		G << eod;
		break;
	default:
		abort();
	}
	G << "\n\n\n";
}


void CodeTemplate::addDependencies(char *deps)
{
	string dep;
	char c = *deps++;
	do {
		while (isspace(c) || (c == ',') || (c == ';'))
			c = *deps++;
		while (isalnum(c) || (c == '_')) {
			dep += c;
			c = *deps++;
		}
		if (!dep.empty()) {
			dbug("%s depends on %s",function.c_str(),dep.c_str());
			dependencies.push_back(dep);
			dep.clear();
		}
	} while (isspace(c) || isalnum(c) || (c == '_') || (c == ',') || (c == ';'));
	if ((c != '\n') && (c != '/') && (c != 0))
		warn("invalid argument for requirements: %s",deps);
}
