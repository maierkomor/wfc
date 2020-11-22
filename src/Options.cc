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

#include "Options.h"
#include "KVPair.h"
#include "PBFile.h"
#include "log.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <set>
#include <vector>


using namespace std;

extern bool toIdentifier(string &);

static bool init_module();
static bool initialized = false;

static map<const char *,const char *,CStrCaseCmp> BinOptionList,TextOptionList;
static map<const char *,set<string>,CStrCaseCmp> ValidOptions;
static Options *Defaults = 0, *FieldDefaults = 0;

static bool init_module()
{
	BinOptionList["comments"] = "generate comments";
	BinOptionList["debug"] = "generate debugging/logging interface";
	BinOptionList["asserts"] = "include asserts";
	BinOptionList["gnux"] = "allow GNU extensions in generated code";
	BinOptionList["id0"] = "allow use of field ID 0";
	BinOptionList["enumnames"] = "allow use of enum names in setByName functions";

	TextOptionList["author"] = "author of source file";
	TextOptionList["copyright"] = "year of copyright of source file";
	TextOptionList["email"] = "e-mail address of copyright holder or author";
	TextOptionList["BaseClass"] = "name of Base Class for common virtual interface";
	BinOptionList["SubClasses"] = "generate nested classes instead of mangled names";
	BinOptionList["withEqual"] = "generate operator == for object comparison";
	BinOptionList["withUnequal"] = "generate operator != for object comparison";
	TextOptionList["endian"] = "target endian to use (unknown,little,big)";
	TextOptionList["stringtype"] = "implementation type for 'string': std::string (default), C (const char *), <typename>";
	TextOptionList["bytestype"] = "implementation type for 'bytes': std::string (default), <typename>";
	TextOptionList["arraysize"] = "use array of size <arraysize> instead of vector for repeated fields";
	TextOptionList["UnknownField"] = "skip/assert unknown fields while parsing (default: skip)";
	TextOptionList["namespace"] = "namespace of generated code (default: <none>)";
	BinOptionList["genlib"] = "generate wfc library";
	TextOptionList["codelib"] = "add code library file or directory";

	TextOptionList["toMemory"] = "name of function for serializing to memory; \"\" to omit generation";
	TextOptionList["toString"] = "name of function for serializing to std::string; \"\" to omit generation";
	TextOptionList["toWire"] = "name of function for serializing via function 'wireput'; \"\" to omit generation";
	TextOptionList["calcSize"] = "set name of function for calculating size on wire; \"\" to omit generation";
	TextOptionList["fromMemory"] = "produce/omit code for parsing memory";
	TextOptionList["AddPrefix"] = "prefix to use for add methods";
	TextOptionList["ClearPrefix"] = "prefix to use for clear methods";
	TextOptionList["ClearName"] = "name of clear methods";
	TextOptionList["GetPrefix"] = "prefix to use for get methods";
	TextOptionList["HasPrefix"] = "prefix to use for has methods";
	TextOptionList["MutablePrefix"] = "prefix to use for mutable methods";
	TextOptionList["SetPrefix"] = "prefix to use for set methods";
	TextOptionList["toSink"] = "name of function for serializing with sink interface; \"\" to omit generation";
	TextOptionList["ascii_string"] = "function for printing strings in toASCII (default: ascii_string)";
	TextOptionList["ascii_bytes"] = "function for printing bytes in toASCII (default: ascii_bytes)";
	TextOptionList["ascii_indent"] = "function to create indentions in toASCII (default: ascii_indent)";
	TextOptionList["storage"] = "storage type of message fields";
	TextOptionList["SetByName"] = "name of function for setting by name with ASCII input (default: setByName)";

	TextOptionList["VarIntBits"] = "data type for varint_t (default: 64)";
	ValidOptions["VarIntBits"].insert("64");
	ValidOptions["VarIntBits"].insert("32");
	ValidOptions["VarIntBits"].insert("16");
	TextOptionList["intsize"] = "number of bits of default integer types (field option)";
	ValidOptions["IntSize"].insert("64");
	ValidOptions["IntSize"].insert("32");
	ValidOptions["IntSize"].insert("16");

	TextOptionList["Optimize"] = "optimization target (review,speed,size)";
	TextOptionList["Optimize_for"] = "alias for option 'Optimize' kept for compatibility reasons";
	TextOptionList["wireput"] = "function for puting data on the wire";
	TextOptionList["SortMembers"] = "sort member variables by: id (default), name, type, size, unsorted";
	TextOptionList["ErrorHandling"] = "error handling concept: assert, cancel (default), throw";
	TextOptionList["wiresize"] = "library function to use as 'wiresize' function";
	TextOptionList["declare"] = "add custom declaration to the generated .h file";
	TextOptionList["header"] = "#include header file <file.h> or \"file.h\" in generated .h file";
	TextOptionList["unset"] = "in-range value that marks the field as unset (omits valid bit generation)";
	TextOptionList["inline"] = "comma separated list of methods to inline: has, get, set";
	TextOptionList["lang"] = "output language type: C++, XML";
	TextOptionList["toJSON"] = "name of function for generating JSON output";
	TextOptionList["json_indent"] = "statement for JSON indention";
	/*
	 * inline: all methods and helper functions are inline in the header
	 * static: all methods with dependencies in .cpp, helpers are static in same .cpp
	 * extern: methods can be put as needed, helpers are shared in <libfile>.{cpp|h}
	 * 	   => need to make sure that shared helpers are compatible wrt config
	 * 	   (e.g. endian, wordwith)
	 *
	 * template based Sink functions need forward declarations of all functions.
	 * => Sink is incompatible with static
	 */
	TextOptionList["wfclib"] = "wfc library functions scope: inline, static, extern";
	TextOptionList["libfile"] = "basename of files for extern library functions";
	TextOptionList["toASCII"] = "name of function for generating human readable ASCII output";

	ValidOptions["MutableType"].insert("pointer");
	ValidOptions["MutableType"].insert("reference");
	ValidOptions["MutableType"].insert("*");
	ValidOptions["MutableType"].insert("&");
	ValidOptions["ErrorHandling"].insert("assert");
	ValidOptions["ErrorHandling"].insert("cancel");
	ValidOptions["ErrorHandling"].insert("throw");
	ValidOptions["UnknownField"].insert("assert");
	ValidOptions["UnknownField"].insert("skip");
	ValidOptions["SortMembers"].insert("name");
	ValidOptions["SortMembers"].insert("id");
	ValidOptions["SortMembers"].insert("type");
	ValidOptions["SortMembers"].insert("size");
	ValidOptions["SortMembers"].insert("unsorted");
	ValidOptions["SortMembers"].insert("none");

	ValidOptions["endian"].insert("unknown");
	ValidOptions["endian"].insert("little");
	ValidOptions["endian"].insert("big");
	ValidOptions["Terminator"].insert("");
	ValidOptions["Terminator"].insert("none");
	ValidOptions["Terminator"].insert("null");
	ValidOptions["Terminator"].insert("0");
	ValidOptions["Terminator"].insert("0x0");
	ValidOptions["Terminator"].insert("ff");
	ValidOptions["Terminator"].insert("0xff");
	ValidOptions["Terminator"].insert("FF");
	ValidOptions["Terminator"].insert("0xFF");

	ValidOptions["storage"].insert("regular");
	ValidOptions["storage"].insert("static");
	ValidOptions["storage"].insert("virtual");
	ValidOptions["Optimize"].insert("review");
	ValidOptions["Optimize"].insert("size");
	ValidOptions["Optimize"].insert("speed");
	ValidOptions["wfclib"].insert("static");
	ValidOptions["wfclib"].insert("inline");
	ValidOptions["wfclib"].insert("extern");
	ValidOptions["lang"].insert("C++");
	ValidOptions["lang"].insert("c++");
	ValidOptions["lang"].insert("XML");
	ValidOptions["lang"].insert("xml");
	ValidOptions["usage"].insert("regular");
	ValidOptions["usage"].insert("deprecated");
	ValidOptions["usage"].insert("obsolete");
	return true;
}


Options *Options::getDefaults()
{
	if (!initialized)
		initialized = init_module();
	if (0 == Defaults) {
		Defaults = new Options("defaults");
		Defaults->initDefaults();
	}
	return Defaults;
}


Options *Options::getFieldDefaults()
{
	if (!initialized)
		initialized = init_module();
	if (0 == FieldDefaults) {
		FieldDefaults = new Options("field_defaults");
		FieldDefaults->initFieldDefaults();
	}
	return FieldDefaults;
}


Options::Options(const string &src, const Options *p)
: m_parent(p)
, m_name(src)
{
	if (!initialized)
		initialized = init_module();
}


void Options::setParent(const Options *p)
{
	assert((m_parent == 0) ^ (p == 0));
	m_parent = p;
}


void Options::initDefaults()
{
	m_TextOptions["author"] = "";
	m_TextOptions["copyright"] = "";
	m_TextOptions["email"] = "";

	m_TextOptions["AddPrefix"] = "add_";
	m_TextOptions["BaseClass"] = "";
	m_TextOptions["lang"] = "c++";
	m_TextOptions["calcSize"] = "calcSize";
	m_TextOptions["getMaxSize"] = "getMaxSize";
	m_TextOptions["ClearName"] = "clear";
	m_TextOptions["ClearPrefix"] = "clear_";
	m_TextOptions["GetPrefix"] = "";
	m_TextOptions["HasPrefix"] = "has_";
	m_TextOptions["MutablePrefix"] = "mutable_";
	m_TextOptions["SetPrefix"] = "set_";
	m_TextOptions["VarIntBits"] = "64";
	m_TextOptions["endian"] = "unknown";
	m_TextOptions["stringtype"] = "std::string";
	m_TextOptions["bytestype"] = "std::string";
	m_TextOptions["syntax"] = "xprotov1";
	m_TextOptions["IntSize"] = "64";	// default size if no size is given (i.e. int, unsigned, signed)
	m_TextOptions["UnknownField"] = "skip";
	m_TextOptions["namespace"] = "";
	m_TextOptions["toASCII"] = "toASCII";
	m_TextOptions["ascii_bytes"] = "ascii_bytes";
	m_TextOptions["ascii_string"] = "ascii_string";
	m_TextOptions["ascii_indent"] = "ascii_indent";
	m_TextOptions["toMemory"] = "toMemory";
	m_TextOptions["toSink"] = "";
	m_TextOptions["toString"] = "toString";
	m_TextOptions["toWire"] = "toWire";
	m_TextOptions["toJSON"] = "toJSON";
	m_TextOptions["fromMemory"] = "fromMemory";
	m_TextOptions["wireput"] = "";
	m_TextOptions["SortMembers"] = "id";
	m_TextOptions["ErrorHandling"] = "cancel";
	m_TextOptions["MutableType"] = "pointer";
	m_TextOptions["Terminator"] = "";
	m_TextOptions["wiresize"] = "";
	m_TextOptions["wiresize_s"] = "";
	m_TextOptions["sint_varint"] = "";
	m_TextOptions["varint_sint"] = "";
	m_TextOptions["send_varint"] = "";		// use wiresend function
	m_TextOptions["parse_ascii_bool"] = "";
	m_TextOptions["parse_ascii_flt"] = "";
	m_TextOptions["parse_ascii_dbl"] = "";
	m_TextOptions["parse_ascii_s8"] = "";
	m_TextOptions["parse_ascii_s16"] = "";
	m_TextOptions["parse_ascii_s32"] = "";
	m_TextOptions["parse_ascii_s64"] = "";
	m_TextOptions["parse_ascii_u8"] = "";
	m_TextOptions["parse_ascii_u16"] = "";
	m_TextOptions["parse_ascii_u32"] = "";
	m_TextOptions["parse_ascii_u64"] = "";
	m_TextOptions["read_u64"] = "";
	m_TextOptions["read_u32"] = "";
	m_TextOptions["read_u16"] = "";
	m_TextOptions["read_bytes"] = "";
	m_TextOptions["read_varint"] = "";
	m_TextOptions["send_bytes"] = "";
	m_TextOptions["send_msg"] = "";
	m_TextOptions["send_u16"] = "";
	m_TextOptions["send_u32"] = "";
	m_TextOptions["send_u64"] = "";
	m_TextOptions["write_u64"] = "";
	m_TextOptions["write_u32"] = "";
	m_TextOptions["write_u16"] = "";
	m_TextOptions["write_bytes"] = "";
	m_TextOptions["write_varint"] = "";
	m_TextOptions["json_cstr"] = "";
	m_TextOptions["json_indent"] = "json_indent";
	m_TextOptions["json_string"] = "";
	m_TextOptions["to_decstr"] = "";
	m_TextOptions["to_dblstr"] = "";
	m_TextOptions["Optimize"] = "review";
	m_TextOptions["Optimize_for"] = "review";
	m_TextOptions["inline"] = "";
	m_TextOptions["ssize_t"] = "";
	m_TextOptions["wfclib"] = "static";
	m_TextOptions["libname"] = "wfccore";
	m_TextOptions["streamtype"] = "std::ostream";
	m_TextOptions["SetByName"] = "setByName";

	m_TextOptions["mangle_double"] = "";
	m_TextOptions["mangle_float"] = "";

	m_BinOptions["SubClasses"] = false;
	m_BinOptions["asserts"] = true;
	m_BinOptions["comments"] = true;
	m_BinOptions["debug"] = false;
	m_BinOptions["genlib"] = true;
	m_BinOptions["gnux"] = true;
	m_BinOptions["withEqual"] = true;
	m_BinOptions["withUnequal"] = true;
	m_BinOptions["FlexDecoding"] = false;
	m_BinOptions["SinkToTemplate"] = false;
	m_BinOptions["devel"] = false;
	m_BinOptions["PaddedMessageSize"] = false;
	m_BinOptions["enumnames"] = false;
}


void Options::initFieldDefaults()
{
	m_TextOptions["arraysize"] = "0";
	m_TextOptions["stringtype"] = "std::string";
	m_TextOptions["bytestype"] = "std::string";
	m_TextOptions["intsize"] = "64";
	m_TextOptions["unset"] = "";
	m_TextOptions["default"] = "";
	m_TextOptions["storage"] = "regular";
	m_TextOptions["parse_ascii"] = "";
	m_TextOptions["to_ascii"] = "";
	m_TextOptions["to_json"] = "";
	m_TextOptions["usage"] = "regular";
	// TODO, valid values could be: default, fixed, variable, dynamic
	//m_TextOptions["encoding"] = "default";

	m_BinOptions["devel"] = false;
	m_BinOptions["packed"] = false;
	m_BinOptions["used"] = true;
}


bool Options::hasFlag(const char *option) const
{
	auto b = m_BinOptions.find(option);
	return (b != m_BinOptions.end());
}


bool Options::hasOption(const char *option) const
{
	const Options *o = this;
	do {
		auto t = o->m_TextOptions.find(option);
		if (t != o->m_TextOptions.end())
			return true;
		o = o->m_parent;
	} while (o);
	return false;
}


const char *Options::GetPrefix() const
{
	auto i = m_TextOptions.find("GetPrefix");
	if (i != m_TextOptions.end())
		return i->second.c_str();
	return "";
}


const char *Options::SetPrefix() const
{
	auto i = m_TextOptions.find("SetPrefix");
	if (i != m_TextOptions.end())
		return i->second.c_str();
	return "set_";
}


const char *Options::MutablePrefix() const
{
	auto i = m_TextOptions.find("MutablePrefix");
	if (i != m_TextOptions.end())
		return i->second.c_str();
	return "mutable_";
}


const char *Options::AddPrefix() const
{
	auto i = m_TextOptions.find("AddPrefix");
	if (i != m_TextOptions.end())
		return i->second.c_str();
	return "add_";
}


const char *Options::ClearName() const
{
	auto i = m_TextOptions.find("ClearName");
	if (i != m_TextOptions.end())
		return i->second.c_str();
	return "clear";
}


const char *Options::ClearPrefix() const
{
	auto i = m_TextOptions.find("ClearPrefix");
	if (i != m_TextOptions.end())
		return i->second.c_str();
	return "clear_";
}


const char *Options::HasPrefix() const
{
	auto i = m_TextOptions.find("HasPrefix");
	if (i != m_TextOptions.end())
		return i->second.c_str();
	return "has_";

}


void Options::printDefines(ostream &out) const
{
	switch (Endian()) {
	case unknown_endian:
		break;
	case little_endian:
		out <<	"#if !defined(__LITTLE_ENDIAN) && !defined(__LITTLE_ENDIAN__) && (__BYTE_ORDER__ != 1234)\n"
			"#error code generated for little endian systems only\n"
			"#endif\n\n";
		break;
	case big_endian:
		out <<	"#if !defined(__BIG_ENDIAN) && !defined(__BIG_ENDIAN__)\n"
			"#error code generated for big endian systems only\n"
			"#endif\n\n";
		break;
	}
	out <<	"#ifdef WFC_ENDIAN\n"
		"#if WFC_ENDIAN != " << Endian() << "\n"
		"#error WFC generated code incompatible due to endian\n"
		"#endif\n"
		"#else\n"
		"#define WFC_ENDIAN     " << Endian() << " // " << getOption("endian") << " endian\n"
		"#endif\n\n";
	if (getFlag("SubClasses"))
		out << "#define SUBCLASSES 1\n";
	if (isId("toMemory"))
		out << "#define HAVE_TO_MEMORY 1\n";
	if (isId("toString"))
		out << "#define HAVE_TO_STRING 1\n";
	if (isId("toSink"))
		out << "#define HAVE_TO_SINK 1\n";
	if (isId("toWire"))
		out << "#define HAVE_TO_WIRE 1\n";
	if (isId("toASCII"))
		out << "#define HAVE_TO_ASCII 1\n";
	if (isId("toJSON"))
		out << "#define HAVE_TO_JSON 1\n";
	if (isId("fromMemory"))
		out << "#define HAVE_FROM_MEMORY 1\n";
	const string &errh = getOption("ErrorHandling");
	if (errh == "cancel")
		out << "#define ON_ERROR_CANCEL 1\n";
	else if (errh == "assert")
		out << "#define ON_ERROR_ABORT 1\n";
	else if (errh == "throw")
		out << "#define ON_ERROR_THROW 1\n";
	out << '\n';
}


void Options::printTo(ostream &out, const string &prefix) const
{
	const Options *o = this;
	set<string,StrCaseCmp> printed;
	bool devel = getFlag("devel");
	do {
		out << prefix << "options from " << o->m_name << ":\n";
		for (auto i(o->m_TextOptions.begin()), e(o->m_TextOptions.end()); i != e; ++i) {
			if (!devel && (TextOptionList.find(i->first.c_str()) == TextOptionList.end()))
				continue;
			if ((i->first == "Optimize") &&  (!printed.insert("Optimize_for").second))
				continue;
			if ((i->first == "Optimize_for") &&  (!printed.insert("Optimize").second))
				continue;
			if (!printed.insert(i->first).second)
				continue;
			char buf[70];
			const char *arg = i->second.c_str();
			int n;
			if ((arg[0] >= '0') && (arg[0] <= '9'))
				n = snprintf(buf,sizeof(buf),"%-16s: %s",i->first.c_str(),arg);
			else if ((arg[0] == '"') || (arg[0] == '\''))
				n = snprintf(buf,sizeof(buf),"%-16s: %s",i->first.c_str(),arg);
			else
				n = snprintf(buf,sizeof(buf),"%-16s: \"%s\"",i->first.c_str(),arg);
			out << prefix << buf;
			if (n >= (int)sizeof(buf))
				out << "\"...\n";
			else
				out << '\n';
		}
		out << prefix << '\n';
		o = o->m_parent;
	} while (o);
	o = this;
	printed.clear();
	do {
		bool first = true;
		for (auto i(o->m_BinOptions.begin()), e(o->m_BinOptions.end()); i != e; ++i) {
			if (!devel && (BinOptionList.find(i->first.c_str()) == BinOptionList.end()))
				continue;
			if (printed.find(i->first) != printed.end())
				continue;
			if (i->second)
				continue;
			printed.insert(i->first);
			if (first) {
				out << prefix << "disabled flags from " << o->m_name << ":\n";
				out << prefix << '\t';
				first = false;
			} else
				out << ", ";
			out << i->first;
		}
		if (!first)
			out << '\n';
		first = true;
		for (auto i(o->m_BinOptions.begin()), e(o->m_BinOptions.end()); i != e; ++i) {
			if (!devel && (BinOptionList.find(i->first.c_str()) == BinOptionList.end()))
				continue;
			if (printed.find(i->first) != printed.end())
				continue;
			if (!i->second)
				continue;
			printed.insert(i->first);
			if (first) {
				out << prefix << "enabled flags from " << o->m_name << ":\n";
				out << prefix << '\t';
				first = false;
			} else
				out << ", ";
			out << i->first;
		}
		if (!first)
			out << '\n';
		o = o->m_parent;
	} while (o);
	//out << prefix << "optmode " << OptimizationMode() << "\n";
}


bool isBinaryArg(const char *value, bool &binValue)
{
	if (!strcasecmp(value,"off"))
		binValue = false;
	else if (!strcasecmp(value,"false"))
		binValue = false;
	else if (!strcasecmp(value,"disable"))
		binValue = false;
	else if (!strcasecmp(value,"disabled"))
		binValue = false;
	else if (!strcasecmp(value,"0"))
		binValue = false;
	else if (!strcasecmp(value,"on"))
		binValue = true;
	else if (!strcasecmp(value,"true"))
		binValue = true;
	else if (!strcasecmp(value,"enable"))
		binValue = true;
	else if (!strcasecmp(value,"enabled"))
		binValue = true;
	else if (!strcasecmp(value,"1"))
		binValue = true;
	else
		return false;
	return true;
}


void Options::addOption(const char *arg, bool documented)
{
	if (0 == strncmp(arg,"no-",3)) {
		if (strchr(arg+3,'=')) {
			error("invalid combination of no- option prefix with argument in %s",arg);
			return;
		}
		addOption(arg+3,false,documented);
		return;
	}
	if (0 == strncmp(arg,"with-",5)) {
		if (strchr(arg+5,'=')) {
			error("invalid combination of with- option prefix with argument in %s",arg);
			return;
		}
		addOption(arg+5,true,documented);
		return;
	}
	char option[64];
	const char *value;
	const char *equal = strchr(arg,'=');
	if (equal) {
		size_t n = (size_t)(equal-arg) > sizeof(option) ? sizeof(option) : (equal-arg);
		strncpy(option,arg,n);
		value = equal + 1;
		option[n] = 0;
	} else {
		strncpy(option,arg,sizeof(option));
		value = 0;
	}
	if (value) {
		bool binValue;
		if (isBinaryArg(value,binValue))
			addOption(option,binValue,documented);
		else
			addOption(option,value,documented);
	} else {
		addOption(option,true,documented);
	}
}


static bool isValidOption(const char *k, const char *v)
{
	auto x(ValidOptions.find(k));
	if (x == ValidOptions.end()) // unrestricted option
		return true;
	const set<string> &allowed = x->second;
	if (allowed.find(v) == allowed.end())
		return false;
	return true;
}


void Options::setOption(const char *k, const char *v)
{
	/*
	if (!getFlag("devel")) {
		auto f = TextOptionList.find(k);
		assert(f != TextOptionList.end());
		if (f == TextOptionList.end())
			fatal("unknown option %s",k);
	}
	*/
	if (!isValidOption(k,v)) {
		error("option %s cannot be set to '%s'",k,v);
		return;
	}
	dbug("setting option %s to %s",k,v);
	auto i = m_TextOptions.insert(make_pair(k,v));
	if (!i.second)
		i.first->second = v;
}


void Options::add(KVPair *pairs)
{
	KVPair *p = pairs;
	do {
		addOption(p->getKey().c_str(),p->getValue().c_str(),false);
		p = p->getNext();
	} while (p);
	delete pairs;
}


void Options::add(const string &n, KVPair *p)
{
	m_NodeOptions.insert(make_pair(n,p));
}


void Options::addOption(const char *option, bool flag, bool documented)
{
	if (0 == strcasecmp(option,"cc_generic_services")) {
		if (flag != false) {
			error("cc_generic_services is unsupported and can only be set to false");
			flag = false;
		}
	} 
	auto x(m_BinOptions.find(option));
	if (m_BinOptions.end() != x) {
		x->second = flag;
		return;
	}
	const Options *p = m_parent;
	while (p) {
		if (p->hasFlag(option)) {
			m_BinOptions.insert(make_pair(option,flag));
			return;
		}
		p = p->m_parent;
	}
	if (flag == false) {
		auto i(TextOptionList.find(option));
		if (TextOptionList.end() != i) {
			setOption(option,"");
			return;
		}
	}
	if (documented && (BinOptionList.find(option) == BinOptionList.end())) {
		// search TextOptionList to figure out if this is a
		// published interface for option -f. Internal functions
		// should use setOption or command line option -x.
		error("unknown flag '%s'",option);
		return;
	}
	auto i(BinOptionList.find(option));
	if (BinOptionList.end() != i) {
		m_BinOptions[option] = flag;
		return;
	}
	error("unknown flag '%s'",option);
}


void Options::addOption(const char *option, const char *v, bool documented)
{
	string value = v;
	if (v[0] == '"') {
		size_t l = strlen(v+1);
		assert(v[l] == '"');
		value = string(v+1,l-1);
	}
	if (0 == strcasecmp(option,"default")) {
		// special case, as default can any kind of argument (id,int,bool,string)
		m_TextOptions["default"] = value;
		return;
	}
	bool binValue;
	if (isBinaryArg(value.c_str(),binValue)) {
		addOption(option,binValue,documented);
		return;
	}
	if (0 == strcasecmp(option,"codelib")) {
		m_CodeLibs.push_back(value);
		return;
	}
	if (0 == strcasecmp(option,"header")) {
		size_t vl = strlen(v);
		if ((v[0] == '<') && (v[vl-1] == '>'))
			m_Headers.push_back(v);
		else if ((v[0] == '"') && (v[vl-1] == '"'))
			m_Headers.push_back(v);
		else
			error("invalid argument for option header: %s",v);
		return;
	}
	if (0 == strcasecmp(option,"Optimize")) {
		if (value == "size")
			m_TextOptions["Optimize"] = value;
		else if (value == "speed")
			m_TextOptions["Optimize"] = value;
		else if (value == "review")
			m_TextOptions["Optimize"] = value;
		else if (value == "lite")
			m_TextOptions["Optimize"] = value;
		else if (value == "lite_runtime")
			m_TextOptions["Optimize"] = value;
		else
			error("invalid argument for option %s: %s",option,value.c_str());
		return;
	}
	if (documented && (TextOptionList.find(option) == TextOptionList.end())) {
		// search TextOptionList to figure out if this is a
		// published interface for option -f. Internal functions
		// should use setOption or command line option -x.
		auto os = ValidOptions.find(option);
		if ((os != ValidOptions.end()) && (os->second.find(v) != os->second.end())) {
			m_TextOptions.insert(make_pair(option,value));
			return;
		}
		error("invalid option/value pair %s/%s",option,v);
		return;
	}
	auto x(m_TextOptions.find(option));
	if (m_TextOptions.end() != x) {
		setOption(option,value.c_str());
		return;
	}
	const Options *p = m_parent;
	while (p) {
		if (p->hasOption(option)) {
			const char *v = value.c_str();
			if (!isValidOption(option,v)) {
				error("invalid option/value combination %s=%s",option,v);
				return;
			}
			m_TextOptions.insert(make_pair(option,value));
			return;
		}
		p = p->m_parent;
	}
	error("undefined option '%s'",option);
}


bool Options::getFlag(const char *option) const
{
	const Options *o = this;
	do {
		auto i = o->m_BinOptions.find(option);
		if (i != o->m_BinOptions.end())
			return i->second;
		o = o->m_parent;
	} while (o);
	//ICE("unknown flag '%s'",option);
	return false;
}


vector<string> Options::getCodeLibs() const
{
	vector<string> codelibs;
	const Options *o = this;
	do {
		codelibs.insert(codelibs.end(),o->m_CodeLibs.begin(),o->m_CodeLibs.end());
		o = o->m_parent;
	} while (o);
	return codelibs;
}


vector<string> Options::getDeclarations() const
{
	vector<string> declarations;
	const Options *o = this;
	do {
		declarations.insert(declarations.end(),o->m_Declarations.begin(),o->m_Declarations.end());
		o = o->m_parent;
	} while (o);
	return declarations;
}


vector<string> Options::getHeaders() const
{
	vector<string> headers;
	const Options *o = this;
	do {
		headers.insert(headers.end(),o->m_Headers.begin(),o->m_Headers.end());
		o = o->m_parent;
	} while (o);
	return headers;
}


const string &Options::getOption(const char *option) const
{
	static string empty("");
	const Options *o = this;
	do {
		auto i = o->m_TextOptions.find(option);
		if (i != o->m_TextOptions.end())
			return i->second;
		o = o->m_parent;
	} while (o);
	return empty;
}


bool Options::isId(const char *o) const
{
	const string &id = getOption(o);
	if (id.empty())
		return false;
	const char *str = id.c_str();
	while (isalnum(*str) || (*str == '_'))
		++str;
	return *str == 0;
}


string Options::getId(const char *o) const
{
	if (isId(o))
		return getOption(o);
	return "";
}


const char *Options::getSource(const char *option) const
{
	auto i = m_TextOptions.find(option);
	if (i != m_TextOptions.end())
		return m_name.c_str();
	auto j = m_BinOptions.find(option);
	if (j != m_BinOptions.end())
		return m_name.c_str();
	if (m_parent)
		return m_parent->getSource(option);
	return "<none>";
}


optmode_t Options::OptimizationMode() const
{
	auto i = m_TextOptions.find("Optimize");	// Optimize has priority over Optimize_for
	auto j = m_TextOptions.find("Optimize_for");
	auto e = m_TextOptions.end();
	if ((i == e) && (j == e)) {
		if (m_parent)
			return m_parent->OptimizationMode();
		return optreview;
	}
	if ((i != e) && (j != e)) {
		if (i->second != j->second)
			error("inconsistent setting for optimization: Optimmize: '%s', Optimize_for: '%s'",i->second.c_str(),j->second.c_str());
	}
	if (i == e)
		i = j;
	const char *value = i->second.c_str();
	if (0 == strcasecmp(value,"size")) 
		return optsize;
	else if (0 == strcasecmp(value,"speed")) 
		return optspeed;
	else if (0 == strcasecmp(value,"review")) 
		return optreview;
	else if (0 == strcasecmp(value,"lite")) 
		return optsize;
	else if (0 == strcasecmp(value,"lite_runtime")) 
		return optsize;
	abort();
	return optreview;
}


/*
protov_t Options::ProtocolVersion() const
{
	auto i(m_TextOptions.find("syntax"));
	if (i == m_TextOptions.end()) {
		if (m_parent)
			return m_parent->ProtocolVersion();
		return xprotov1;
	}
	const string &mode = i->second;
	if (mode == "gprotov2")
		return gprotov2;
	if (mode == "gprotov3")
		return gprotov3;
	if (mode == "xprotov1")
		return xprotov1;
	return xprotov1;
}
*/


endian_t Options::Endian() const
{
	auto i(m_TextOptions.find("endian"));
	if (i == m_TextOptions.end())
		return unknown_endian;
	const string &mode = i->second;
	if (mode == "unknown")
		return unknown_endian;
	if (mode == "little")
		return little_endian;
	if (mode == "big")
		return big_endian;
	error("Unknown endian mode '%s'. Using mode unknown.",mode.c_str());
	return unknown_endian;
}


unsigned Options::VarIntBits() const
{
	const string &v = getOption("VarIntBits");
	const char *value = v.c_str();
	assert(value);
	long l = strtol(value,0,0);
	switch (l) {
	case 16:
	case 32:
	case 64:
		return l;
	default:
		error("Invalid setting for varint bits size of '%s'",value);
		return 64;
	}
}


unsigned Options::IntSize() const
{
	const string &v = getOption("IntSize");
	const char *value = v.c_str();
	assert(value);
	long l = strtol(value,0,0);
	switch (l) {
	case 16:
	case 32:
	case 64:
		return l;
	default:
		error("Invalid setting for varint bits size of '%s'",value);
		return 64;
	}
}


void Options::merge(const Options &o) 
{
	for (auto i(o.m_TextOptions.begin()), e(o.m_TextOptions.end()); i != e; ++i) {
		if (i->second != "")
			m_TextOptions[i->first] = i->second;
	}
	
	for (auto i(o.m_BinOptions.begin()), e(o.m_BinOptions.end()); i != e; ++i) {
		m_BinOptions[i->first] = i->second;
	}
}


void Options::printHelp(ostream &out)
{
	out.setf(ios_base::left);
	for (auto i(TextOptionList.begin()), e(TextOptionList.end()); i != e; ++i) {
		out << "-f";
		out.width(15);
		out << i->first << ": " << i->second << '\n';
	}

	for (auto i(BinOptionList.begin()), e(BinOptionList.end()); i != e; ++i) {
		out << "-f";
		out.width(15);
		out << i->first << ": " << i->second << '\n';
	}
}


bool Options::StringSerialization() const
{
	return !getOption("toString").empty();
}


bool Options::isEnabled(const char *k) const
{
	string id = getOption(k);
	return toIdentifier(id);
}


const char *Options::getIdentifier(const char *option) const
{
	const Options *o = this;
	auto i = o->m_TextOptions.find(option);
	while (i == o->m_TextOptions.end()) {
		o = o->m_parent;
		if (o == 0)
			ICE("unable to find well-known option");
		i = o->m_TextOptions.find(option);
	}
	string &r = const_cast<string&>(i->second);
	if (r.empty())
		return 0;
	const char *rs = r.c_str();
	if ((rs[0] == '"') && (rs[r.size()-1] == '"')) {
		r.erase(0,1);
		r.resize(r.size()-1);
	}
	rs = r.c_str();
	if (!isIdentifier(rs)) {
		warn("option %s requires identifier, but is set to '%s'",option,rs);
		r.clear();
		return 0;
	}
	return rs;
}
	


KVPair::~KVPair()
{
	delete next;
}


void KVPair::setNext(KVPair *n)
{
	assert(n->next == 0);
	n->next = next;
	next = n;
}


bool isIdentifier(const char *id)
{
	size_t l = strlen(id);
	if (l == 0)
		return false;
	char c = *id++;
	if ((c >= 'a') && (c <= 'z'));
	else if ((c >= 'A') && (c <= 'Z'));
	else if (c == '_');
	else if (c == ':') {
		// this is for e.g. ::sometype
		if ((id[0] != ':') || (id[1] == ':') || (id[1] == 0))
			return false;
		++id;
		--l;
	} else
		return false;
	while (--l) {
		c = *id++;
		if ((c >= 'a') && (c <= 'z'));
		else if ((c >= 'A') && (c <= 'Z'));
		else if ((c >= '0') && (c <= '9'));
		else if (c == '_');
		else if (c == ':') {
			// this is for e.g. std::string
			if ((id[0] != ':') || (id[1] == ':') || (id[1] == 0))
				return false;
			++id;
			--l;
		} else
			return false;
	}
	return true;
}


