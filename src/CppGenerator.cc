/*
 *  Copyright (C) 2017-2019, Thomas Maier-Komor
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

#include "CppGenerator.h"
#include "CodeLibrary.h"
#include "Generator.h"
#include "Enum.h"
#include "Field.h"
#include "KVPair.h"
#include "PBFile.h"
#include "Message.h"
#include "Options.h"
#include "io.h"
#include "log.h"
#include "wirefuncs.h"

#include <assert.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <set>
#include <string>
#include <string.h>
#include <unistd.h>

using namespace std;


CppGenerator::CppGenerator(PBFile *p, Options *o)
: file(p)
, clOptions(o)
, usesArrays(false)
, usesVectors(false)
, usesCStrings(false)
, usesStringTypes(false)
, PaddedMsgSize(false)
, SinkToTemplate(false)
, WithJson(false)
, EarlyDecode(false)	// TODO: bind to -Os or other option
, inlineClear(true)
, inlineHas(true)
, inlineGet(true)
, inlineMaxSize(true)
, inlineSet(true)
, inlineSize(true)
, hasU16(false)
, hasU32(false)
, hasU64(false)
, hasFloat(false)
, hasDouble(false)
, hasInt(false)
, hasSInt(false)
, hasUInt(false)
, hasCStr(false)
, hasLenPfx(false)
, hasUnused(false)
, needJsonString(false)
, needCalcSize(false)
, needSendVarSInt(false)
, license()
{
	setTarget();
}


void CppGenerator::setLicense(const char *t)
{
	license =
		"/********************************************************************************\n";
	const char *nl = strchr(t,'\n');
	while (nl) {
		license += " * ";
		license += string(t,nl+1);
		t = nl + 1;
		nl = strchr(t,'\n');
	}
	if (*t) {
		license += " * ";
		license += t;
	}
	license +=
		" ********************************************************************************/\n"
		"\n"
		"\n";
}


void CppGenerator::setTarget(const char *t)
{
	if (t[0] != 0)
		diag("setting target to %s",t);
	usesStringTypes = false;
	if (file == 0)
		file = new PBFile("<none>");
	target = file->getOptions(t);
	VarIntBits = target->VarIntBits();
	Options *fo = Options::getFieldDefaults();
	fo->initFieldDefaults();
	if (target->hasOption("stringtype"))
		fo->addOption("stringtype",target->getOption("stringtype").c_str(),false);
	if (target->hasOption("bytestype"))
		fo->addOption("bytestype",target->getOption("bytestype").c_str(),false);
	if (target->hasOption("intsize")) {
		long intsize = strtol(target->getOption("intsize").c_str(),0,0);
		if (intsize > (long)VarIntBits)
			error("IntSize (%u) must be smaller or equal than VarIntBits (%u).",intsize,VarIntBits);
		else
			fo->addOption("intsize",target->getOption("intsize").c_str(),false);
	} else if (VarIntBits < 64) {
		fo->addOption("intsize",target->getOption("VarIntBits").c_str(),false);
	}
	clOptions->setParent(0);
	clOptions->setParent(target);
	target = clOptions;
	if (fo->getParent())
		fo->setParent(0);
	fo->setParent(target);

	SinkToTemplate = target->getFlag("SinkToTemplate") && target->isId("toSink");
	if (SinkToTemplate && (target->getOption("wfclib") == "static"))
		fatal("static library support is incompatible with template based Sink methods");
	optmode = target->OptimizationMode();
	if (optmode == optreview) {
		//if (SinkToTemplate)
			//fatal("SinkToTemplate cannot be used with -Or");
		inlineClear = false;
		inlineHas = false;
		inlineGet = false;
		inlineMaxSize = false;
		inlineSet = false;
		inlineSize = false;
	} else {
		// needed as during initialization target might be set twice
		// if option -t is passed on the command line
		inlineClear = true;
		inlineHas = true;
		inlineGet = true;
		inlineMaxSize = true;
		inlineSet = true;
		inlineSize = true;
	}
#ifdef BETA_FEATURES
	if (optmode == optsize)
		EarlyDecode = true;
#endif
	PaddedMsgSize = target->getFlag("padded_message_size");
	const char *inlopt = target->getOption("inline").c_str();
	if (strstr(inlopt,"!has"))
		inlineHas = false;
	else if (strstr(inlopt,"has"))
		inlineHas = true;
	if (strstr(inlopt,"!get"))
		inlineGet = false;
	else if (strstr(inlopt,"get"))
		inlineGet = true;
	if (strstr(inlopt,"!set"))
		inlineSet = false;
	else if (strstr(inlopt,"set"))
		inlineSet = true;
	if (strstr(inlopt,"!size"))
		inlineSize = false;
	else if (strstr(inlopt,"size"))
		inlineSize = true;
	if (strstr(inlopt,"!clear"))
		inlineClear = false;
	else if (strstr(inlopt,"clear"))
		inlineClear = true;
	if (strstr(inlopt,"!maxsize"))
		inlineMaxSize = false;
	else if (strstr(inlopt,"maxsize"))
		inlineMaxSize = true;
	Endian = target->Endian();
	string toASCII = target->getOption("toASCII");
	PrintOut = toIdentifier(toASCII);
	string toJSON = target->getOption("toJSON");
	WithJson = toIdentifier(toJSON);
	Asserts = target->getFlag("asserts");
	ErrorHandling = target->getOption("ErrorHandling");

	if (ErrorHandling == "") ErrorHandling = "cancel";
	else if (ErrorHandling == "cancel");
	else if (ErrorHandling == "throw");
	else if (ErrorHandling == "assert");
	else fatal("invalid value for error handling: %s",ErrorHandling.c_str());

	Debug = target->getFlag("debug");
	SubClasses = target->getFlag("SubClasses");
	string wireput = target->getOption("wireput");
	WireputArg = !toIdentifier(wireput);
	const Options *no = target;
	do {
		const map<string,KVPair*> &nodeopts = no->getNodeOptions();
		for (map<string,KVPair*>::const_iterator i(nodeopts.begin()),e(nodeopts.end()); i != e; ++i) {
			applyNodeOption(i->first.c_str(),i->second);
		}
		no = no->getParent();
	} while (no);
	const vector<string> &includes = target->getCodeLibs();
	for (size_t x = 0, n = includes.size(); x < n; ++x) 
		Lib.add(includes[x].c_str());
}


void CppGenerator::applyNodeOption(const char *nodepath, KVPair *kvp)
{
	diag("appyNodeOption(%s,[%s,%s])",nodepath,kvp->getKey().c_str(),kvp->getValue().c_str());
	if (nodepath[0] != '/')
		error("invalid node path '%s'",nodepath);
	const char *node = nodepath+1;
	Message *m = 0;
	const char *slash = strchr(node,'/');
	while (slash) {
		string msgname(node,slash);
		diag("appyNodeOption: msg %s",msgname.c_str());
		if (m) {
			m = m->getMessage(msgname.c_str());
		} else {
			m = file->getMessage(msgname.c_str());
		}
		if (m == 0) {
			warn("Ignoring unknwon message '%s' in nodepath '%s'",msgname.c_str(),nodepath);
			return;
		}
		node = slash+1;
		slash = strchr(slash+1,'/');
	}
	Enum *e = m ? m->getEnum(node) : file->getEnum(node);
	if (e) {
		diag("appyNodeOption: enum %s",node);
		e->setOption(kvp->getKey(),kvp->getValue());
		return;
	}
	if ((m == 0) && (e == 0)) {
		warn("Unable to resolve node path %s. Ignoring option %s",nodepath,kvp->getKey().c_str());
		return;
	}
	if (Field *f = m->getField(node)) {
		diag("appyNodeOption: field %s",node);
		f->setOption(kvp->getKey(),kvp->getValue());
		return;
	}
	warn("Ignoring unknwon node '%s' in nodepath '%s'",node,nodepath);
}


string CppGenerator::getValid(Field *f, bool invalid)
{
	string ret;
	uint32_t type = f->getType();
	if (type == ft_cptr) {
		if (invalid)
			ret += "m_$(fname) == 0";
		else
			ret += "m_$(fname) != 0";
		return ret;
	}
	int vbit = f->getValidBit();
	const string &invValue = f->getInvalidValue();
	if (!invValue.empty()) {
		assert(vbit < 0);
		if (invalid)
			ret += "m_$(fname) == " + invValue;
		else
			ret += "m_$(fname) != " + invValue;
		return ret;
	}
	assert(invValue == "");
	assert(vbit >= 0);
	if (invalid)
		ret += "0 == ";
	else
		ret += "0 != ";
	if (f->getParent()->getNumValid() <= VarIntBits)
		ret += "(p_validbits & (($(validtype))1U << $vbit))";
	else
		ret += "(p_validbits[$($vbit/8)] & (($(validtype))1U << $($vbit&7)))";
	return ret;
}


void CppGenerator::writeGetValid(Generator &G, Field *f, bool invalid)
{
	G << getValid(f,invalid);
}


const char *CppGenerator::setValid(int vbit, unsigned numvalid)
{
	if (vbit < 0)
		return "";
	if (numvalid <= VarIntBits)
		return "p_validbits |= (($(validtype))1U << $vbit);\n";
	else
		return "p_validbits[$($vbit/8)] |= (($(validtype))1U << $($vbit&7));\n";
}


void CppGenerator::writeSetValid(Generator &G, int vbit)
{
	G << setValid(vbit,G.getMessage()->getNumValid());
}


const char *CppGenerator::clearValid(int vbit, unsigned numvalid)
{
	if (vbit < 0)
		return "";
	if (numvalid <= VarIntBits)
		return "p_validbits &= ~(($(validtype))1U << $vbit);";
	else
		return "p_validbits[$($vbit/8)] &= ~(($(validtype))1U << $($vbit&7));";
}


void CppGenerator::writeClearValid(Generator &G, int vbit)
{
	G << clearValid(vbit,G.getMessage()->getNumValid());
}


void CppGenerator::initNames(Message *m, const string &prefix)
{
	//diag("initNames(%s,%s)",m->getName().c_str(),prefix.c_str());
	string subprefix = prefix + m->getName();
	if (SubClasses) {
		subprefix += "::";
		for (unsigned i = 0, e = m->numEnums(); i != e; ++i) {
			Enum *en = m->getEnum(i);
			en->setPrefix(subprefix);
		}
		for (unsigned i = 0, e = m->numMessages(); i != e; ++i) {
			Message *sm = m->getMessage(i);
			sm->setPrefix(subprefix);
			initNames(sm,subprefix);
		}
	} else {
		subprefix += '_';
		for (unsigned i = 0, e = m->numEnums(); i != e; ++i) {
			Enum *en = m->getEnum(i);
			en->setNamePrefix(subprefix);
		}
		for (unsigned i = 0, e = m->numMessages(); i != e; ++i) {
			Message *sm = m->getMessage(i);
			initNames(sm,subprefix);
			sm->setNamePrefix(subprefix);
		}
	}
}


void CppGenerator::initVBits(Message *m)
{
	// valid bits are necessary for optional fields
	// validbit=explicit: all types but ft_cstr get a valid-bit
	// validbit=implicit: following types
	if (m->isOneOf())
		fatal("one-of messages are unsupported");
	size_t vbit = 0;
	for (size_t i = 0, n = m->numFields(); i != n; ++i) {
		Field *f = m->getField(i);
		if ((f == 0) || (!f->isUsed()))
			continue;
		quant_t q = f->getQuantifier();
		if (q == q_repeated) {
			if (f->getArraySize() != 0)
				usesArrays = true;
			else
				usesVectors = true;
		}
		if (f->getIntSize() > VarIntBits)	// implementation int size to be used for signed/int/unsigned
			f->setIntSize(VarIntBits);
		uint32_t t = f->getType();
		if (VarIntBits < 64) {
			if ((t == ft_int8) || (t == ft_int16) || (t == ft_int32)) {
				needSendVarSInt = true;
			}
			if ((t == ft_int64) || (t == ft_uint64) || (t == ft_sint64))
				error("Conflict: use of int64 type in %s/%s when varint is smaller than 64bit.",m->getName().c_str(),f->getName());
			if (VarIntBits < 32)
				if ((t == ft_int32) || (t == ft_uint32) || (t == ft_sint32))
					error("Conflict: use of int32 types in %s/%s when varint is smaller than 32bit.",m->getName().c_str(),f->getName());
			if (VarIntBits < 16)
				if ((t == ft_int16) || (t == ft_uint16) || (t == ft_sint16))
					error("Conflict: use of int16 types in %s/%s when varint is smaller than 16bit.",m->getName().c_str(),f->getName());
		}
		if (f->setValidBit(vbit))
			++vbit;
		switch (t) {
		case ft_string:
		case ft_bytes:
			usesStringTypes = true;
			break;
		default:;
		}
		/*
		// valid bit
		if (t == ft_string) {
			const string &stringtype = f->getOption("stringtype");
			if (stringtype == "std")
				usesStringTypes = true;
			f->setValidBit(-1);
		} else if (t == ft_bytes) {
			f->setValidBit(-1);
			const string &bytestype = f->getOption("bytestype");
			if (bytestype == "std")
				usesStringTypes = true;
			else if (bytestype == "class")
				usesBytes = true;
			else
				error("invalid value for option bytestype: %s",bytestype.c_str());
		} else if (t == ft_cptr) {
			f->setValidBit(-1);
		} else if (q_optional == q)
			f->setValidBit(vbit++);		// assign valid bit id
		else if ((q_repeated == q) ||  (q_required == q))
			f->setValidBit(-1);
		else
			abort();
		*/
	}
	m->setNumValid(vbit);
	for (unsigned i = 0, e = m->numMessages(); i != e; ++i) {
		Message *sm = m->getMessage(i);
		initVBits(sm);
	}
}


void CppGenerator::init(const vector<string> &msgs)
{
	bool all = msgs.empty();
	usesArrays = false;
	usesVectors = false;
	usesBytes = false;
	usesCStrings = false;
	usesStringTypes = false;
	for (unsigned i = 0, n = file->numMessages(); i != n; ++i) {
		Message *m = file->getMessage(i);
		m->setUsed(all);
		m->setOptions(target);
		initVBits(m);
		initNames(m,"");
	}
	for (auto i = msgs.begin(), e = msgs.end(); i != e; ++i) {
		const char *name = i->c_str();
		if (Message *m = file->getMessage(name))
			m->setUsed(true);
		else
			error("unable to select message %s for code generation: no such message",name);
	}
}


void CppGenerator::scanRequirements(Message *m)
{
	for (unsigned i = 0, e = m->numFields(); i != e; ++i) {
		Field *f = m->getField(i);
		if (f == 0)
			continue;
		if (!f->isUsed()) {
			hasUnused = true;
			continue;
		}
		uint32_t tc = f->getTypeClass();
		if ((tc == ft_enum) && WithJson) {
			Enum *e = Enum::id2enum(f->getType());
			const string &strfun = e->getStringFunction();
			if (strfun.empty())
				e->setStringFunction();
		}
		if (f->isRepeated() || (tc == ft_enum) || (tc == ft_string) || (tc == ft_bytes) || (tc == ft_msg))
			hasUInt = true;
		if (tc == ft_msg)
			continue;
		if ((tc == ft_int) || (tc == ft_int8)| (tc == ft_int16) || (tc == ft_int32) || (tc == ft_int64)) {
			hasInt = true;
			continue;
		}
		if ((tc == ft_unsigned) || (tc == ft_uint16) || (tc == ft_uint32) || (tc == ft_uint64)) {
			hasUInt = true;
			continue;
		}
		if ((tc == ft_signed) || (tc == ft_sint8) || (tc == ft_sint16) || (tc == ft_sint32) || (tc == ft_sint64)) {
			hasSInt = true;
			continue;
		}
		if (tc == ft_cptr) {
			hasCStr = true;
			continue;
		}
		if ((tc == ft_string) || (tc == ft_bytes)) {
			hasLenPfx = true;
			continue;
		}
		if ((Endian == little_endian) && (f->isPacked()) && ((tc == ft_fixed8) || (tc == ft_fixed16) || (tc == ft_fixed32) || (tc == ft_fixed64) || (tc == ft_float) || (tc == ft_double)))
			hasLenPfx = true;
		wiretype_t w;
		if (f->isRepeated())
			w = f->getElementEncoding();
		else
			w = f->getEncoding();
		switch (w) {
		case wt_16bit:
			hasU16 = true;
			break;
		case wt_32bit:
			if (tc == ft_float)
				hasFloat = true;
			else
				hasU32 = true;
			break;
		case wt_64bit:
			if (tc == ft_double)
				hasDouble = true;
			else
				hasU64 = true;
			break;
		case wt_lenpfx:
		case wt_8bit:
		case wt_varint:
			break;
		default:
			abort();
		}
	}
	for (unsigned i = 0, e = m->numMessages(); i != e; ++i) {
		Message *sm = m->getMessage(i);
		scanRequirements(sm);
	}
}


static void allHelpers(vector<unsigned> &funcs, bool VarIntBits64)
{
	funcs.push_back(ct_mangle_float);
	funcs.push_back(ct_mangle_double);
	funcs.push_back(ct_wiresize);
	funcs.push_back(ct_wiresize_s);
	funcs.push_back(ct_wiresize_x);
	funcs.push_back(ct_sint_varint);
	funcs.push_back(ct_varint_sint);
	funcs.push_back(ct_write_u16);
	funcs.push_back(ct_write_u32);
	funcs.push_back(ct_write_u64);
	funcs.push_back(ct_read_varint);
	funcs.push_back(ct_read_u64);
	funcs.push_back(ct_read_u32);
	funcs.push_back(ct_read_u16);
	funcs.push_back(ct_read_double);
	funcs.push_back(ct_read_float);
	funcs.push_back(ct_skip_content);

	funcs.push_back(gen_wire);
	funcs.push_back(ct_send_u16);
	funcs.push_back(ct_send_u32);
	funcs.push_back(ct_send_u64);
	funcs.push_back(ct_send_varint);
	if (!VarIntBits64)
		funcs.push_back(ct_send_xvarint);
	funcs.push_back(ct_send_bytes);
	funcs.push_back(ct_write_varint);
	if (!VarIntBits64)
		funcs.push_back(ct_write_xvarint);
	funcs.push_back(ct_place_varint);

	funcs.push_back(gen_string);
	funcs.push_back(ct_send_u16);
	funcs.push_back(ct_send_u32);
	funcs.push_back(ct_send_u64);
	funcs.push_back(ct_send_varint);
	if (!VarIntBits64)
		funcs.push_back(ct_send_xvarint);
	funcs.push_back(ct_send_msg);


	funcs.push_back(ct_json_string);
	funcs.push_back(ct_json_indent);
	funcs.push_back(ct_json_cstr);
	funcs.push_back(ct_to_decstr);
	funcs.push_back(ct_to_dblstr);
}


void CppGenerator::writeLib()
{
	vector<unsigned> allfuncs;
	allHelpers(allfuncs,VarIntBits==64);
	const string &libname = target->getOption("libname");
	msg("generating %s.cpp...",libname.c_str());
	fstream lc;
	lc.open((libname + ".cpp").c_str(),fstream::out);
	writeInfos(lc);
	lc << "#include \"" << libname << ".h\"\n\n";
	Generator lcg(lc,target);
	Lib.write_cpp(lcg,allfuncs,target);

	fstream lh;
	msg("generating %s.h...",libname.c_str());
	lh.open((libname + ".h").c_str(),fstream::out);
	writeInfos(lh);
	string defname = libname;
	transform(libname.begin(),libname.end(),defname.begin(),::toupper);
	Generator lhg(lh,target);
	lhg << "#ifndef " << defname << "_H\n"
		"#define " << defname << "_H\n\n\n";
	if (Asserts || (target->getOption("UnknownField") == "assert"))
		lhg << "#include <assert.h>\n";
	if (PrintOut)
		lhg <<	"#define OUTPUT_TO_ASCII 1\n"
			"#include <iosfwd>\n";
	if (target->StringSerialization() || usesStringTypes || PrintOut || WithJson)
		lhg <<	"#include <string>\n";
	else
		lhg << "/* std::string support is not needed */\n";
	if (usesVectors)
		lhg << "#include <vector>\n";
	else
		lhg << "/* std::vector support not needed */\n";
	if (usesArrays)
		lhg << "#include <array.h>\n";
	else
		lhg << "/* array support not needed */\n";
	lhg <<	"#include <stddef.h>\n"
		"#include <stdint.h>\n"
		"#include <stdlib.h>\n";
	if (target->getOption("stringtype") == "std")
		lhg << "#include <string>\n";
	Lib.write_includes(lhg,allfuncs,target);
	target->printDefines(lh);
	lhg <<	"\n\n"
		"typedef uint$(VarIntBits)_t varint_t;\n"
		"typedef int$(VarIntBits)_t varsint_t;\n";
	if (lhg.hasValue("ssize_t"))
		lhg << "typedef $ssize_t ssize_t;\n";
	lhg <<	"\n\n";
	Lib.write_h(lhg,allfuncs,target);
	lhg << "#endif\n";
}


void CppGenerator::writeFiles(const char *basename)
{
	for (unsigned i = 0, n = file->numMessages(); i != n; ++i)
		scanRequirements(file->getMessage(i));
	const string &wfclib = target->getOption("wfclib");
	if ((wfclib == "extern") && target->getFlag("genlib"))
		writeLib();
	writeBody(basename);
	if (hadError()) {
		string fn;
		fn = basename;
		fn += ".cpp";
		unlink(fn.c_str());
		return;
	}
	writeHeader(basename);
	if (hadError()) {
		string fn;
		fn = basename;
		fn += ".cpp";
		unlink(fn.c_str());
		fn = basename;
		fn += ".h";
		unlink(fn.c_str());
	}
}


static void mangle_filename(string &dst, const string src)
{
	locale loc;
	for (unsigned i = 0, n = src.length(); i != n; ++i) {
		char c = toupper(src.at(i),loc);
		if (c == '-')
			c = '_';
		else if (c == '.')
			c = '_';
		if (c == '/')
			dst.clear();
		else
			dst += c;
	}
}


bool CppGenerator::writeMember(Generator &G, Field *f, bool def_not_init, bool first)
{
	assert(f != 0);
	G.setField(f);
	if (!f->isUsed()) {
		hasUnused = true;
		G << "// omitted unused member $(fname)\n";
		G.setField(0);
		return first;
	}
	if (def_not_init) {
		switch (f->getQuantifier()) {
		case q_optional:
		case q_required:
			G << "$(typestr) m_";
			break;
		case q_repeated:
			if (unsigned s = f->getArraySize())
				G << "array<$(typestr)," << s << "> m_";
			else
				G << "std::vector<$(typestr)> m_";
			break;
		default:
			abort();
		}
		G << "$(fname);\n";
	} else {
		if (first)
			G << ": ";
		else
			G << ", ";
		G << "m_$(fname)(";
		const char *defv = f->getDefaultValue();
		if (defv) 
			G << defv << ")\n";
		else if (f->getEncoding() == wt_lenpfx)
			G << ")\n";
		else 
			G << "0)\n";
	}
	G.setField(0);
	return false;
}


void CppGenerator::writeHeaderDecls(Generator &G, Field *f)
{
	if (f == 0)
		return;
	if (!f->isUsed())
		return;
	G.setField(f);
	bool simpletype = f->hasSimpleType();
	uint8_t q = f->getQuantifier();
	if (q != q_required)
		G	<< "bool $(field_has)() const;\n";
	if (q_repeated == q) {
		G	<< "size_t $(fname)_size() const;\n";
		if (f->hasMessageType())
			G << "$(typestr)* $(field_add)();\n";
		else if (simpletype)
			G << "void $(field_add)($(typestr) v);\n";
		else
			G << "void $(field_add)(const $(typestr) &v);\n";
	}
	if (q != q_required)
		G << "void $(field_clear)();\n";
	if (q_repeated == q)
		G << "$(fullrtype)$(field_get)(unsigned x) const;\n";
	else
		G << "$(fullrtype)$(field_get)() const;\n";
	uint32_t type = f->getType();
	if (type == ft_bytes)
		G <<	"void $(field_set)(const void *data, size_t s);\n";
	G << "void $(field_set)";
	if (q_repeated == q)
		G << "(unsigned x, $(fullrtype) v);\n";
	else
		G << "($(rtype) v);\n";
	if (target->getOption("MutableType") == "pointer")
		G << "$(ptype)$(field_mutable)";
	else if (target->getOption("MutableType") == "reference")
		G << "$(typestr) &$(field_mutable)";
	else
		abort();
	if (q_repeated == q)
		G << "(unsigned x);\n";
	else
		G << "();\n";
	if (q_repeated == q) {
		if (unsigned s = f->getArraySize())
			G << "array<$(typestr)," << s << "> ";
		else
			G << "std::vector<$(typestr)> ";
		if (target->getOption("MutableType") == "pointer")
			G << '*';
		else if (target->getOption("MutableType") == "reference")
			G << '&';
		else
			abort();
		G << "$(field_mutable)();\n";
	}
	G << '\n';
	G.setField(0);
}


void CppGenerator::writeEnumDecl(Generator &G, Enum *e, bool inclass)
{
	G.setEnum(e);
	G	<< "typedef enum {\n";
	const multimap<int64_t,string> &vn = e->getValueNamePairs();
	for (multimap<int64_t,string>::const_iterator i(vn.begin()),j(vn.end()); i != j; ++i) {
		diag("enum-value %s %lld",i->second.c_str(),i->first);
		G << i->second << " = " << i->first << ",\n";
	}
	G	<< "} $(ename);\n\n";
	const string &strfun = e->getStringFunction();
	if (!strfun.empty()) {
		if (inclass)
			G << "static ";
		G << "const char *$(strfun)($(ename) e);\n\n";
	}
	G.setEnum(0);
}


void CppGenerator::writeEnumDefs(Generator &G, Enum *en)
{
	const string &strfun = en->getStringFunction();
	if (strfun.empty())
		return;
	G.setEnum(en);
	G <<	"const char *$(eprefix)$(strfun)($(ename) e)\n"
		"{\n"
		"switch (e) {\n"
		"default:\n"
		"return \"<invalid $(ename)>\";\n";
	const multimap<int64_t,string> &vn = en->getValueNamePairs();
	auto i(vn.begin()), e(vn.end()), x(i);
	while (i != e) {
		G << "case " << i->second << ":\n";
		x = i;
		++x;
		while ((x != e) && (x->first == i->first)) {
			G << "// alias " << x->second << '\n';
			++x;
		}
		if (const char *str = en->getStringValue(i->first))
			G << "return " << str << ";\n";
		else
			G << "return \"" << i->second << "\";\n";
		i = x;
	}
	G <<	"}\n"
		"}\n\n";
	G.setEnum(0);
}


void CppGenerator::writeMembers(Generator &G, Message *m, bool def_not_init)
{
	const string &sorting = m->getOption("SortMembers");
	if ((sorting != "id") && (sorting != "type"))
		error("Invalid argument '%s' for option SortMember. Valid values are: id, type",sorting.c_str());
	unsigned nf = m->numFields();
	if (sorting == "id") {
		bool first = true;
		for (unsigned i = 0; i != nf; ++i) {
			Field *f = m->getField(i);
			if (f)
				first = writeMember(G,f,def_not_init,first);
		}
		return;
	}
	assert(sorting == "type");
	unsigned size = 32;
	bool first = true;
	do {
		size >>= 1;
		for (unsigned i = 0; i != nf; ++i) {
			Field *f = m->getField(i);
			if ((f == 0) || (!f->isRepeated()))
				continue;
			if (f->getMemberSize() == size)
				first = writeMember(G,f,def_not_init,first);
		}
	} while (size);
	size = 32;
	do {
		size >>= 1;
		for (unsigned i = 0; i != nf; ++i) {
			Field *f = m->getField(i);
			if ((f == 0) || (f->isRepeated()))
				continue;
			if (f->getMemberSize() == size)
				first = writeMember(G,f,def_not_init,first);
		}
	} while (size);
}


void CppGenerator::writeClass(Generator &G, Message *m)
{
	if (!SubClasses) {
		for (unsigned i = 0, e = m->numEnums(); i != e; ++i) {
			Enum *en = m->getEnum(i);
			writeEnumDecl(G,en);
		}
		for (unsigned i = 0, e = m->numMessages(); i != e; ++i) {
			Message *sm = m->getMessage(i);
			writeClass(G,sm);
		}
	}
	G.setMessage(m);
	G <<	"class $(msg_name)";
	if (G.hasValue("BaseClass"))
		G << " : public $BaseClass";
	G <<	"\n"
	 	"{\n"
		"public:\n"
		"$(msg_name)();\n\n"
		"bool operator != (const $(msg_name) &r) const;\n"
		"bool operator == (const $(msg_name) &r) const;\n"
		"void $(msg_clear)();\n"
		"size_t $calcSize() const;\n";
	if (G.hasValue("getMaxSize"))
		G << "size_t $getMaxSize() const;\n";
	if (G.hasValue("fromMemory"))
		G << "ssize_t $(fromMemory)(const void *b, ssize_t s);\n";
	if (G.hasValue("toMemory"))
		G << "size_t $(toMemory)(uint8_t *, ssize_t) const;\n";
	if (G.hasValue("toWire")) {
		G.setMode(gen_wire);
		G <<	"void $(toWire)($putparam) const;\n";
	}
	if (G.hasValue("toSink")) {
		G.setMode(gen_sink);
		G <<	"$(sink_template)void $(toSink)($putparam) const;\n";
	}
	if (G.hasValue("toString")) {
		G.setMode(gen_string);
		G <<	"void $(toString)($putparam) const;\n";
	}
	if (G.hasValue("toJSON"))
		G << "void $(toJSON)($(streamtype) &json, unsigned indLvl = 0) const;\n";
	if (PrintOut)
		G << "void $(toASCII)($(streamtype) &o, const std::string &p = \"\") const;\n";
	G << '\n';
	if (SubClasses) {
		for (unsigned i = 0, e = m->numEnums(); i != e; ++i) {
			Enum *en = m->getEnum(i);
			writeEnumDecl(G,en,true);
		}
		G.setMessage(0);
		for (unsigned i = 0, e = m->numMessages(); i != e; ++i) {
			writeClass(G,m->getMessage(i));
			G << "\n";
		}
		G.setMessage(m);
	}
	for (unsigned i = 0, e = m->numFields(); i != e; ++i)
		writeHeaderDecls(G,m->getField(i));
	G <<	"\n"
		"protected:\n";
	writeMembers(G,m,true);
	unsigned numValid = m->getNumValid();
	if (numValid > 0) {
		G << "\nprivate:\n";
		if (numValid <= 8) {
			G	<< "uint8_t p_validbits;\n";
		} else if (numValid <= 16) {
			G	<< "uint16_t p_validbits;\n";
		} else if (numValid <= 32) {
			G	<< "uint32_t p_validbits;\n";
		} else if (numValid <= 64) {
			G	<< "uint64_t p_validbits;\n";
		} else {
			G	<< "uint8_t p_validbits[$(numvalidbytes)];\n";
		}
	}
	G <<	"};\n\n\n";
	G.setMessage(0);
}


void CppGenerator::writeHas(Generator &G, Field *f)
{
	quant_t q = f->getQuantifier();
	if (q == q_required)
		return;
	//uint32_t t = f->getType();
	G <<	"$(inline)bool $(prefix)$(msg_name)::$(field_has)() const\n"
		"{\n"
		"return ";
	switch (q) {
	case q_unspecified:
	case q_optional:
		writeGetValid(G,f);
		G << ";\n}\n\n";
		break;
	case q_required:
		abort();
		/*
		if ((t == ft_bytes) || (t == ft_string))
			G << "!m_$(fname).empty();\n}\n\n";
		else if (t == ft_cptr)
			G << "0 != m_$(fname)[0];\n}\n\n";
		else
			G << "true;\n}\n\n";
		*/
		break;
	case q_repeated:
		G << "!m_$(fname).empty();\n}\n\n";
		break;
	}
}


void CppGenerator::writeSize(Generator &G, Field *f)
{
	if (f->getQuantifier() == q_repeated) {
		G <<	"$(inline)size_t $(prefix)$(msg_name)::$(fname)_size() const\n"
			"{\n"
			"return m_$(fname).size();\n"
			"}\n\n";
	}
}


void CppGenerator::writeFunctions(Generator &G, Field *f)
{
	G.setField(f);
	if (!inlineHas)
		writeHas(G,f);
	if (!inlineGet)
		writeGet(G,f);
	if (!inlineClear)
		writeClear(G,f);
	if (!inlineSet)
		writeSet(G,f);
	if (!inlineSize)
		writeSize(G,f);
	G.setField(0);
}


void CppGenerator::writeInlines(Generator &G, Field *f)
{
	G.setField(f);
	if (inlineGet)
		writeGet(G,f);
	if (inlineHas)
		writeHas(G,f);
	if (inlineClear)
		writeClear(G,f);
	if (inlineSet)
		writeSet(G,f);
	if (inlineSize)
		writeSize(G,f);
	G.setField(0);
}


void CppGenerator::writeInlines(Generator &G, Message *m)
{
	string subprefix;
	for (unsigned i = 0, e = m->numMessages(); i != e; ++i) {
		writeInlines(G,m->getMessage(i));
	}
	G.setMessage(m);
	if (G.hasValue("toSink") && SinkToTemplate) {
		G.setMode(gen_sink);
		G.setVariable("toX",G.getVariable("toSink"));
		writeToX(G,m);
	}
	if (inlineMaxSize)
		writeMaxSize(G,m);
	for (unsigned i = 0, e = m->numFields(); i != e; ++i) {
		Field *f = m->getField(i);
		if ((f != 0) && (f->isUsed()))
			writeInlines(G,f);
	}
	G.setMessage(0);
}


void CppGenerator::writeInfos(ostream &out)
{
	out	<< license;
	out	<< "/*\n";
	target->printTo(out," * ");
	out	<< " */\n\n";
}


void CppGenerator::writeHeader(const string &bn)
{
	string fn = bn + ".h";
	msg("generating %s...",fn.c_str());
	fstream out;
	out.open(fn.c_str(),fstream::out);
	string ucfn;
	mangle_filename(ucfn,bn);
	writeInfos(out);
	out <<	"#ifndef " << ucfn << "_H\n"
		"#define " << ucfn << "_H\n\n";
	if (Asserts || (target->getOption("UnknownField") == "assert"))
		out << "#include <assert.h>\n";
	if (PrintOut)
		out <<	"#define OUTPUT_TO_ASCII 1\n"
			"#include <iosfwd>\n";
	if (target->StringSerialization() || usesStringTypes || PrintOut || WithJson)
		out <<	"#include <string>\n";
	else
		out << "/* std::string support is not needed */\n";
	if (usesVectors)
		out << "#include <vector>\n";
	else
		out << "/* std::vector support not needed */\n";
	if (usesArrays)
		out << "#include <array.h>\n";
	else
		out << "/* array support not needed */\n";
	out <<	"#include <stddef.h>\n"
		"#include <stdlib.h>\n"
		"#include <stdint.h>\n"
		//"#include <sys/types.h>\n"	 -- incompatible with AVR, not needed otherwise
		"\n";
	const string &wfclib = target->getOption("wfclib");
	if (SinkToTemplate && (wfclib == "extern"))
		out << "#include \"" << target->getOption("libname") << ".h\"\n";
	const vector<string> &headers = target->getHeaders();
	if (!headers.empty()) {
		out << "/* user requested header files */\n";
		for (auto i = headers.begin(), e = headers.end(); i != e; ++i)
			out << "#include " << *i << "\n";
	}
	Generator G(out,target);
	if (target->getOption("wfclib") != "extern") {
		vector<unsigned> funcs;
		writeHelpers(funcs);
		Lib.write_includes(G,funcs,target);
	}

	if (G.hasValue("toSink")) {
		G << "#define WITH_SINK\n";
		if (SinkToTemplate)
			G << "#include \"" << target->getOption("libname") << ".h\"\n";
		else
			G << "#include <sink.h>\n";
	}
	if (usesCStrings)
		G << "#include <cstring.h>\n";
	if (usesBytes)
		G << "#include <bytes.h>\n";
	if (PrintOut)
		G << "#include <wfc_support.h>\n";
	else
		G << "/* wfc support functions not needed */\n";
	G << '\n';

	const string &ns = target->getOption("namespace");
	if (!ns.empty())
		G << "namespace " << ns << " {\n\n";
	target->printDefines(out);
	G.setVariable("inline","inline ");
	if (G.hasValue("ssize_t"))
		G << "typedef $ssize_t ssize_t;\n";
	G << "typedef uint$(VarIntBits)_t varint_t;\n\n";
	G << "typedef int$(VarIntBits)_t varsint_t;\n\n";
	if (!WireputArg) {
		G <<	"#define WIREPUT_FUNCTION $wireput\n"
			"void $wireput(uint8_t);\n\n";
	}
	G << "\n";
	string bcn = target->getOption("BaseClass");
	if (toIdentifier(bcn)) {
		G.setVariable("stringtype",target->getOption("stringtype"));
		G << 	"class " << bcn << "\n"
			"{\n"
			"	public:\n"
			"	virtual ~" << bcn << "() = 0;\n"
			"	virtual size_t $calcSize() const = 0;\n";
		if (G.hasValue("toString"))
			G << 	"\tvirtual void $toString($(stringtype) &) const = 0;\n";
		if (G.hasValue("toSink") && !SinkToTemplate)
			G << 	"\tvirtual void $toSink(Sink &) const = 0;\n";
		if (G.hasValue("toMemory"))
			G << 	"\tvirtual size_t $toMemory(uint8_t *, ssize_t) const = 0;\n";
		G <<	"};\n\n";
	}
	for (unsigned i = 0, n = file->numEnums(); i != n; ++i) {
		Enum *e = file->getEnum(i);
		writeEnumDecl(G,e);
	}
	for (unsigned i = 0, n = file->numMessages(); i != n; ++i) {
		Message *m = file->getMessage(i);
		if (m->isUsed())
			writeClass(G,m);
	}
	if (target->getOption("wfclib") != "extern") {
		vector<unsigned> funcs;
		writeHelpers(funcs);
		Lib.write_h(G,funcs,target);
	}

	if (inlineGet || inlineHas || inlineClear || inlineSet || inlineSize || SinkToTemplate) {
		for (unsigned i = 0, n = file->numMessages(); i != n; ++i) {
			Message *m = file->getMessage(i);
			if (m->isUsed())
				writeInlines(G,m);
		}
	}
	if (!ns.empty())
		G << "} /* namespace " << ns << "*/\n\n";

	G << "#endif\n";
}



void CppGenerator::writeGet(Generator &G, Field *f)
{
	uint8_t q = f->getQuantifier();
	if (q != q_repeated) {
		G <<	"$(inline)$(fullrtype) $(prefix)$(msg_name)::$(field_get)() const\n"
			"{\n"
			"return m_$(fname);\n"
			"}\n\n";
	} else {
		G <<	"$(inline)$(fullrtype) $(prefix)$(msg_name)::$(field_get)(unsigned x) const\n"
			"{\n"
			"return m_$(fname)[x];\n"
			"}\n\n";
	}
}


void CppGenerator::writeMutable(Generator &G, Field *f)
{
	bool simpletype = f->hasSimpleType();
	uint32_t t = f->getType();
	uint8_t q = f->getQuantifier();
	bool mut_ref = target->getOption("MutableType") == "reference";
	G.setVariable("index","");
	if (mut_ref)
		G <<	"$(inline)$(fulltype) &";
	else
		G <<	"$(inline)$(fulltype) *";
	if (q == q_optional) {
		int vbit = f->getValidBit();
		if (-1 == vbit) {
			G <<	"$(prefix)$(msg_name)::$(field_mutable)()\n"
				"{\n";
		} else {
			G <<	"$(prefix)$(msg_name)::$(field_mutable)()\n"
				"{\n";
			if (simpletype) {
				const char *def = f->getDefaultValue();
				G <<	"if (" << getValid(f,true) << ") {\n"
					"m_$(fname) = " << (def?def:"0") << ";\n";
				writeSetValid(G,vbit);
				G <<	"}\n";
			} else if ((t & ft_filter) == ft_msg) {
				G << 	"if (" << getValid(f,true) << ") {\n"
					"m_$(fname).clear();\n";
				writeSetValid(G,vbit);
				G <<	"}\n";
			} else {
				writeSetValid(G,vbit);
			}
		}
	} else if (q == q_required) {
		G <<	"$(prefix)$(msg_name)::$(field_mutable)()\n"
			"{\n";
	} else if (q == q_repeated) {
		G.setVariable("index","[x]");
		G <<	"$(prefix)$(msg_name)::$(field_mutable)(unsigned x)\n"
			"{\n"
			"if (x >= m_$(fname).size())\n"
			"m_$(fname).resize(x+1);\n";
	} else
		abort();
	if (mut_ref)
		G <<	"return m_$(fname)$(index);\n"
			"}\n\n"
			;
	else
		G <<	"return &m_$(fname)$(index);\n"
			"}\n\n"
			;
	G.clearVariable("index");
	if (q == q_repeated) {
		if (unsigned s = f->getArraySize())
			G << "inline array<$(fulltype)," << s << "> ";
		else
			G << "inline std::vector<$(fulltype)> ";
		if (mut_ref)
			G << '&';
		else
			G << '*';
		G << "$(prefix)$(msg_name)::$(field_mutable)()\n"
			"{\n";
		if (mut_ref)
			G << "return m_$(fname);\n";
		else
			G << "return &m_$(fname);\n";
		G << "}\n\n";
	}
}


void CppGenerator::writeSet(Generator &G, Field *f)
{
	writeMutable(G,f);
	uint32_t t = f->getType();
	uint8_t q = f->getQuantifier();
	int vbit = f->getValidBit();
	string setvalid = setValid(vbit,f->getParent()->getNumValid());
	if (q == q_optional) {
		if (t == ft_bytes) {
			G <<	"$(inline)void $(prefix)$(msg_name)::$(field_set)(const void *data, size_t s)\n"
				"{\n"
				"m_$(fname).assign((const char *)data,s);\n"
			<<	setvalid
			<<	"}\n"
				"\n";
		} else {
			G <<	"$(inline)void $(prefix)$(msg_name)::$(field_set)($(rtype) v)\n"
				"{\n"
				"m_$(fname) = v;\n"
			<<	setvalid
			<<	"}\n"
				"\n";
		}
	} else if (q == q_required) {
		if (t == ft_bytes)
			G <<	"$(inline)void $(prefix)$(msg_name)::$(field_set)(const void *data, size_t s)\n"
				"{\n"
				"m_$(fname).assign((const char *)data,s);\n"
				"}\n\n";
		G <<	"$(inline)void $(prefix)$(msg_name)::$(field_set)($(fullrtype) v)\n"
			"{\n"
			"m_$(fname) = v;\n"
			"}\n\n";
	} else if (q == q_repeated) {
		if (f->hasMessageType())
			G	<< "$(inline)$(fulltype) *$(prefix)$(msg_name)::$(field_add)()\n"
				   "{\n"
				   "m_$(fname).resize(m_$(fname).size()+1);\n"
				   "return &m_$(fname).back();\n"
				   "}\n\n";
		else if (f->hasSimpleType())
			G	<< "$(inline)void $(prefix)$(msg_name)::$(field_add)($(typestr) v)\n"
				   "{\n"
				   "m_$(fname).push_back(v);\n"
				   "}\n\n";
		else
			G	<< "$(inline)void $(prefix)$(msg_name)::$(field_add)($(fullrtype)v)\n"
				   "{\n"
				   "m_$(fname).push_back(v);\n"
				   "}\n\n";
		G <<	"$(inline)void $(prefix)$(msg_name)::$(field_set)(unsigned x, $(fullrtype)v)\n"
			"{\n";
		if (Asserts)
			G <<	"assert(x < m_$(fname).size());\n";
		G <<	"m_$(fname)[x] = v;\n"
			"}\n\n";
	} else
		abort();
}


void CppGenerator::writeClear(Generator &G, Field *f)
{
	uint8_t q = f->getQuantifier();
	if (q == q_required)
		return;
	G	<< "$(inline)void $(prefix)$(msg_name)::$(field_clear)()\n"
		   "{\n";
	uint32_t type = f->getType();
	const string &invStr = f->getInvalidValue();
	int vbit = f->getValidBit();
	if (vbit >= 0) {
		writeClearValid(G,vbit);
	} else if ((q == q_repeated) || ((type & ft_filter) == ft_msg)) {
		G << "m_$(fname).clear();";
	} else if (!invStr.empty()) {
		G << "m_$(fname) = " << invStr << ';';
	} else if ((type == ft_string) || (type == ft_bytes)) {
		G << "m_$(fname).clear();";
	} else if (type == ft_cptr) {
		G << "m_$(fname) = 0;";
	} else {
		abort();
	}
	G <<	"\n}\n\n";
}


void CppGenerator::writeCalcSize(Generator &G, Field *f)
{
	G.setField(f);
	uint32_t type = f->getType();
	uint8_t quan = f->getQuantifier();
	unsigned ts = f->getTagSize();
	string tstr,packedtagstr;
	const char *packed = "";
	G << "// $fname\n";
	if (quan != q_required) {
		char buf[32];
		sprintf(buf," + %d /* tag(id) 0x%x */",ts,f->getId()<<3);
		tstr = buf;
	}
	if (quan == q_repeated) {
		if (f->hasMessageType()) {
			G <<	"// repeated message $(fname)\n"
				"for (size_t x = 0, y = m_$(fname).size(); x < y; ++x) {\n"
				"size_t s = m_$(fname)[x].$calcSize();\n"
				;
			if (PaddedMsgSize)
				G << "r += sizeof(varint_t)*8/7+1;\n";
			else
				G << "r += $(wiresize_u)(s);\n";
			G <<	"r += s" << tstr << ";\n"
				"}\n";
			G.setField(0);
			return;
		}
		G << "if (!m_$(fname).empty()) {\n";
		if (f->isPacked()) {
		// dynamic encoding cannot be used for packed arrays
			uint32_t t = type;
			unsigned shift;
			if ((type & ft_filter) == ft_enum) {
				wiretype_t ee = Enum::id2enum(type)->getEncoding();
				if (ee != wt_dynamic)
					t = ee;
			}
			switch (t) {
			case ft_bool:
			case ft_fixed8:
			case ft_sfixed8:
				shift = 0;
				break;
			case ft_fixed16:
			case ft_sfixed16:
				shift = 1;
				break;
			case ft_fixed32:
			case ft_sfixed32:
			case ft_float:
				shift = 2;
				break;
			case ft_fixed64:
			case ft_sfixed64:
			case ft_double:
				shift = 3;
				break;
			case ft_sint8:
			case ft_sint16:
			case ft_sint32:
			case ft_sint64:
				G <<	"// $(fname): packed repeated $(typestr)\n"
					"size_t $(fname)_dl = 0;\n"
					"for (size_t x = 0, y = m_$(fname).size(); x < y; ++x)\n"
					"$(fname)_dl += $(wiresize_s)((int64_t)m_$(fname)[x]);\n"
					"r += $(fname)_dl + $(wiresize_s)($(fname)_dl) /* data length */" << tstr << ";\n";
				G.setField(0);
				G <<	"}\n";	// end of if (!m_(fname).empty())
				return;
			default:
				assert((type & ft_filter) == ft_enum);
				/* FALLTHRU */
			case ft_uint8:
			case ft_uint16:
			case ft_uint32:
			case ft_uint64:
				G <<	"// $(fname): packed repeated $(typestr)\n"
					"size_t $(fname)_dl = 0;\n"
					"for (size_t x = 0, y = m_$(fname).size(); x < y; ++x)\n"
					"$(fname)_dl += $(wiresize_u)((varint_t)m_$(fname)[x]);\n"
					"r += $(fname)_dl + $(wiresize_u)($(fname)_dl) /* data length */" << tstr << ";\n";
				G.setField(0);
				G <<	"}\n";	// end of if (!m_(fname).empty())
				return;
			case ft_int:
			case ft_int8:
			case ft_int16:
			case ft_int32:
			case ft_int64:
				G <<	"// $(fname): packed repeated $(typestr)\n"
					"size_t $(fname)_dl = 0;\n"
					"for (size_t x = 0, y = m_$(fname).size(); x < y; ++x)\n"
					"$(fname)_dl += $(wiresize_x)(m_$(fname)[x]);\n"
					"r += $(fname)_dl + $(wiresize_x)($(fname)_dl) /* data length */" << tstr << ";\n";
				G.setField(0);
				G <<	"}\n";	// end of if (!m_(fname).empty())
				return;
			}
			if (shift == 0) {
				G <<	"// $(fname): repeated packed $(typestr), with fixed element size of one byte\n"
					"size_t $(fname)_dl = m_$(fname).size();\n"
					"r += $(fname)_dl + $(wiresize_u)($(fname)_dl)" << tstr << ";\n";
				G.setField(0);
				G <<	"}\n";	// end of if (!m_(fname).empty())
				return;
			} else if (shift > 0) {
				G <<	"// $(fname): repeated packed $(typestr), with fixed element size\n"
					"size_t $(fname)_dl = m_$(fname).size() << " << shift << ";\n"
					"r += $(fname)_dl + $(wiresize_u)($(fname)_dl)" << tstr << ";\n";
				G.setField(0);
				G <<	"}\n";	// end of if (!m_(fname).empty())
				return;
			}
			abort();
		}
		if (f->hasFixedSize()) {
			assert(!f->isPacked());
			G <<	"// $(fname): non-packed, fixed size elements\n"
				"r += m_$(fname).size() * " << f->getFixedSize() << ";\t// including tag\n";
		} else if (f->hasEnumType())
			G <<	"// $(fname): repeated enum $(typestr)\n"
				"for (size_t x = 0, y = m_$(fname).size(); x < y; ++x)\n"
				"r += $(wiresize_u)(m_$(fname)[x]);\n"
				"r += m_$(fname).size() * $(tagsize);\t// tags\n";
		else if (type == ft_cptr)
			G <<	"// $(fname): repeated $(typestr)\n"
				"for (size_t x = 0, y = m_$(fname).size(); x < y; ++x) {\n"
				"	size_t $(fname)_s = m_$(fname)[x] ? (strlen(m_$(fname)[x]) + 1) : 1;\n"
				"	r += $(fname)_s + $(wiresize_u)($(fname)_s)" << tstr << ";\n"
				"}\n";
		else if ((type == ft_bytes) || (type == ft_string))
			G <<	"// $(fname): repeated $(typestr)\n"
				"for (size_t x = 0, y = m_$(fname).size(); x < y; ++x) {\n"
				"size_t s = m_$(fname)[x].size();\n"
				"r += $(wiresize_u)(s);\n"
				"r += s" << tstr << ";\n"
				"}\n";
		else if ((type == ft_sint8) || (type == ft_sint16) || (type == ft_sint32) || (type == ft_sint64))
			G <<	"// $(fname): " << packed << "repeated $(typestr)\n"
				"size_t $(fname)_dl = 0;\n"
				"for (size_t x = 0, y = m_$(fname).size(); x < y; ++x)\n"
				"$(fname)_dl += $(wiresize_s)((int64_t)m_$(fname)[x])" << tstr << ";\n"
				"r += $(fname)_dl;\n";
		else if ((type == ft_int8) || (type == ft_int16) || (type == ft_int32) || (type == ft_int64))
			G <<	"// $(fname): " << packed << "repeated $(typestr)\n"
				"size_t $(fname)_dl = 0;\n"
				"for (size_t x = 0, y = m_$(fname).size(); x < y; ++x)\n"
				"$(fname)_dl += $(wiresize_x)(m_$(fname)[x])" << tstr << ";\n"
				"r += $(fname)_dl;\n";
		else if ((type == ft_uint8) || (type == ft_uint16) || (type == ft_uint32) || (type == ft_uint64)
			|| ((type & ft_filter) == ft_enum))
			G <<	"// $(fname): " << packed << "repeated $(typestr)\n"
				"size_t $(fname)_dl = 0;\n"
				"for (size_t x = 0, y = m_$(fname).size(); x < y; ++x)\n"
				"$(fname)_dl += $(wiresize_u)((varint_t)m_$(fname)[x])" << tstr << ";\n"
				"r += $(fname)_dl;\n";
		else
			abort();
		G.setField(0);
		G	<< "}\n";	// end of if (!m_(fname).empty())
		return;
	} // end of repeated 
	if (quan == q_optional) {
		if (optmode == optspeed) {
			G << "if (";
			writeGetValid(G,f);
			G << ") {\n";
		} else
			G << "if ($(field_has)()) {\n";
	}
	if ((type & ft_filter) == ft_msg) {
		if (PaddedMsgSize)
			G <<	"size_t $(fname)_s = m_$(fname).$calcSize();\n"
				"r += $(fname)_s + sizeof(varint_t)*8/7+1" << tstr << ";\n";
		else
			G <<	"size_t $(fname)_s = m_$(fname).$calcSize();\n"
				"r += $(fname)_s + $(wiresize_u)($(fname)_s)" << tstr << ";\n";
	} else if ((type & ft_filter) == ft_enum) {
		G << "r += $(wiresize_u)((varint_t)m_$(fname))" << tstr << ";\n";
	} else switch (type) {
	case ft_int8:
	case ft_int16:
	case ft_int32:
	case ft_int64:
		G << "r += $(wiresize_x)((varint_t)m_$(fname))" << tstr << ";\n";
		break;
	case ft_uint8:
	case ft_uint16:
	case ft_uint32:
	case ft_uint64:
		G << "r += $(wiresize_u)((varint_t)m_$(fname))" << tstr << ";\n";
		break;
	case ft_sint8:
	case ft_sint16:
	case ft_sint32:
	case ft_sint64:
		G << "r += $(wiresize_s)((varint_t)m_$(fname))" << tstr << ";\n";
		break;
	case ft_bool:
	case ft_fixed8:
	case ft_sfixed8:
		if (quan == q_optional)
			G << "r += " << (ts+1) << ";\n";
		break;
	case ft_fixed16:
	case ft_sfixed16:
		if (quan == q_optional)
			G << "r += " << (ts+2) << ";\n";
		break;
	case ft_fixed32:
	case ft_sfixed32:
	case ft_float:
		if (quan == q_optional)
			G << "r += " << (ts+4) << ";\n";
		break;
	case ft_fixed64:
	case ft_sfixed64:
	case ft_double:
		if (quan == q_optional)
			G << "r += " << (ts+8) << ";\n";
		break;
	case ft_cptr:
		if (quan == q_required)
			G <<	"size_t $(fname)_s = m_$(fname) ? (strlen(m_$(fname)) + 1) : 1;\n";
		else
			G <<	"size_t $(fname)_s = m_$(fname) ? (strlen(m_$(fname)) + 1) : 0;\n";
		G <<	"r += $(fname)_s + $(wiresize_u)($(fname)_s)" << tstr << ";\n";
		break;
	case ft_bytes:
	case ft_string:
		G <<	"size_t $(fname)_s = m_$(fname).size();\n"
			"r += $(fname)_s + $(wiresize_u)($(fname)_s)" << tstr << ";\n";
		break;
	default:
		abort();
	}
	if (quan == q_optional)
		G << "}\n";
	G.setField(0);
}


void CppGenerator::writeCalcSize(Generator &G, Message *m)
{
	G	<< "size_t $(prefix)$(msg_name)::$calcSize() const\n"
		<< "{\n";
	size_t n = 0;
	for (unsigned i = 0, e = m->numFields(); i != e; ++i) {
		Field *f = m->getField(i);
		if (f == 0)
			continue;
		if (f->getQuantifier() == 1) {
			size_t ts = f->getTagSize();
			G << "// " << f->getName() << ": tagsize " << ts;
			n += ts;
			if (f->hasFixedSize()) {
				size_t fs = f->getFixedSize(false);
				G << ", data size " << fs;
				n += fs;
				if (f->hasMessageType()) {
					size_t li = wiresize_u64(fs);
					G << ", length info " << li;
					n += li;
				}
			}
			G << "\n";
		}
	}
	G << "size_t r = " << n << ";\t// required size, default is fixed length\n";
	for (unsigned i = 0, e = m->numFields(); i != e; ++i) {
		Field *f = m->getField(i);
		if (f == 0)
			continue;
		if (!(f->hasFixedSize() && (f->getQuantifier() == 1)) && f->isUsed())
			writeCalcSize(G,f);
	}
	G <<	"return r;\n"
		"}\n"
		"\n";
}



void CppGenerator::decode8bit(Generator &G, Field *f)
{
	G <<	"if (a >= e)\n"
		"	$handle_error;\n";
	G.field_fill("*a++");
}


void CppGenerator::decode16bit(Generator &G, Field *f)
{
	G <<	"if ((a+1) >= e)\n"
		"	$handle_error;\n";
	switch (f->getTypeClass()) {
	case ft_uint16:
	case ft_enum:
		G.field_fill("read_u16(a)");
		break;
	default:
		G.field_fill("($typestr) read_u16(a)");
	}
	G <<	"a += 2;\n";
}


void CppGenerator::decode32bit(Generator &G, Field *f)
{
	G <<	"if ((a+3) >= e)\n"
		"	$handle_error;\n";
	switch (f->getTypeClass()) {
	case ft_float:
		G.field_fill("read_float(a)");
		break;
	case ft_uint32:
	case ft_enum:
		G.field_fill("read_u32(a)");
		break;
	default:
		G.field_fill("($typestr) read_u32(a)");
		break;
	}
	G <<	"a += 4;\n";
}


void CppGenerator::decode64bit(Generator &G, Field *f)
{
	G <<	"if ((a+7) >= e)\n"
		"	$handle_error;\n";
	switch (f->getTypeClass()) {
	case ft_double:
		G.field_fill("read_double(a)");
		break;
	case ft_enum:
	case ft_uint32:
		G.field_fill("read_u64(a)");
		break;
	default:
		G.field_fill("($typestr) read_u64(a)");
		break;
	}
	G <<	"a += 8;\n";
}


void CppGenerator::decodeVarint(Generator &G, Field *f)
{
	G <<	"{\n"
		"varint_t v;\n"
		"int n = read_varint(a,e-a,&v);\n"
		"if (n <= 0)\n"
		"	$handle_error;\n"
		"a += n;\n";
	G.field_fill("v");
	G <<	"}\n";
}


void CppGenerator::decodeSVarint(Generator &G, Field *f)
{
	G <<	"{\n"
		"varint_t v;\n"
		"int n = read_varint(a,e-a,&v);\n"
		"if (n <= 0)\n"
		"	$handle_error;\n"
		"a += n;\n";
	G.field_fill("varint_sint(v)");
	G <<	"}\n";
}


void CppGenerator::decodeMessage(Generator &G, Field *f)
{
	G <<	"{\n"
		"varint_t v;\n"
		"int n = read_varint(a,e-a,&v);\n"
		"a += n;\n"
		"if ((n <= 0) || ((a+v) > e))\n"
		"	$handle_error;\n";
	if (f->getQuantifier() == q_repeated)
		G << "$(m_field).emplace_back();\n";
	G <<	"if (v != 0) {\n";
	G.field_fill("(const uint8_t*)a,v");
	G <<	"if (n != (ssize_t)v)\n"
		"	$handle_error;\n"
		"a += v;\n"
		"}\n"
		"}\n";
	/*	msg: 	field.from(a,v)
	 *	msg[]:	field.push_back(string(a,v))
	 */
}


void CppGenerator::decodeByteArray(Generator &G, Field *f)
{
	G <<	"{\n"
		"varint_t v;\n"
		"int n = read_varint(a,e-a,&v);\n"
		"a += n;\n"
		"if ((n <= 0) || ((a+v) > e))\n"
		"	$handle_error;\n";
	G.field_fill("(const char*)a,v");
	G <<	"a += v;\n"
		"}\n";
	/*	string: 	field.assign
	 *	string[]:	field.push_back(string(a,v))
	 */
}


static void writeSkipContent(Generator &G, wiretype_t t)
{
	switch (t) {
	case wt_varint:
		G <<	"while ((*a & 0x80) && (a <= e))\n"
			"	++a;\n";
		break;
	case wt_64bit:
		G << "a += 8;\n";
		break;
	case wt_32bit:
		G << "a += 4;\n";
		break;
	case wt_16bit:
		G << "a += 2;\n";
		break;
	case wt_8bit:
		G << "++a;\n";
		break;
	default:
		G << "{\n"
			"varint_t v;\n"
			"unsigned l = read_varint(a,e-a,&v);\n"
			"if (0 == l)\n"
			"	$handle_error;\n"
			"a += l + v;\n"
			"}\n";
	}
	G <<	"if (a >= e)\n"
		"	$handle_error;\n"
		"break;\n";
}


void CppGenerator::writeFromMemory(Generator &G, Field *f)
{
	G.setField(f);
	uint32_t type = f->getTypeClass();
	uint32_t id = f->getId();
	if (!f->isUsed()) {
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr\n";
		writeSkipContent(G,f->getEncoding());
	}
	if (f->isPacked()) {
		G.setVariableHex("field_tag",(int64_t)id<<3|2);
		G <<	"case $(field_tag): {\t// $(fname) id $(field_id), packed $(typestr)[] coding 2\n"
			"varint_t v;\n"
			"int n = read_varint(a,e-a,&v);\t// length of packed\n"
			"if (n <= 0)\n"
			"	$handle_error;\n"
			"a += n;\n"
			"const uint8_t *ae = a + v;\n"
			"do {\n";
		switch (type) {
		case ft_msg:
		case ft_bytes:
		case ft_string:
		case ft_cptr:
			abort();
			break;
		case ft_unsigned:
		case ft_int:
		case ft_enum:
		case ft_int16:
		case ft_uint16:
		case ft_int32:
		case ft_uint32:
		case ft_int64:
		case ft_uint64:
			decodeVarint(G,f);
			break;
		case ft_signed:
		case ft_sint16:
		case ft_sint32:
		case ft_sint64:
			decodeSVarint(G,f);
			break;
		case ft_bool:
		case ft_int8:
		case ft_uint8:
		case ft_sint8:
		case ft_fixed8:
		case ft_sfixed8:
			decode8bit(G,f);
			break;
		case ft_fixed16:
		case ft_sfixed16:
			decode16bit(G,f);
			break;
		case ft_fixed32:
		case ft_sfixed32:
		case ft_float:
			decode32bit(G,f);
			break;
		case ft_fixed64:
		case ft_sfixed64:
		case ft_double:
			decode64bit(G,f);
			break;
		}
		G <<	"} while (a < ae);\n"
			"} break;\n";
		uint32_t enc = f->getEncoding();
		if (enc == wt_lenpfx) {
			G.setField(0);
			return;
		}
		G.setVariableHex("field_tag",(int64_t)id<<3|enc);
	}
	int vbit = f->getValidBit();
	switch (type) {
	case ft_msg:
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding byte[]\n";
		decodeMessage(G,f);
		if (vbit != -1)
			writeSetValid(G,vbit);
		break;
	case ft_bytes:
	case ft_string:
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding byte[]\n";
		decodeByteArray(G,f);
		if (vbit != -1)
			writeSetValid(G,vbit);
		break;
	case ft_cptr:
		G <<	"case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding byte[]\n"
			"{\n"
			"varint_t v;\n"
			"int n = read_varint(a,e-a,&v);\n"
			"a += n;\n"
			"if ((n <= 0) || (a+v > e))\n"
			"	$handle_error;\n"
			"if ((v > 0) && (a[v-1] == 0))\n";
		G.field_fill("(const char*)a");
		if (vbit != -1)
			writeSetValid(G,vbit);
		G <<	"a += v;\n"
			"}\n";
		break;
	case ft_unsigned:
	case ft_int:
	case ft_enum:
	case ft_int8:
	case ft_uint8:
	case ft_int16:
	case ft_uint16:
	case ft_int32:
	case ft_uint32:
	case ft_int64:
	case ft_uint64:
		G.setVariableHex("field_tag",(int64_t)id<<3|wt_varint);
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding varint\n";
		decodeVarint(G,f);
		if (target->getFlag("FlexDecoding")) {
			G << "break;\n";
			G.setVariableHex("field_tag",(int64_t)id<<3|wt_8bit);
			G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding 8bit\n";
			decode8bit(G,f);
			G << "break;\n";
			G.setVariableHex("field_tag",(int64_t)id<<3|wt_16bit);
			G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding 16bit\n";
			decode16bit(G,f);
			G << "break;\n";
			G.setVariableHex("field_tag",(int64_t)id<<3|wt_32bit);
			G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding: 32bit\n";
			decode32bit(G,f);
			G << "break;\n";
			G.setVariableHex("field_tag",(int64_t)id<<3|wt_64bit);
			G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding: 64bit\n";
			decode64bit(G,f);
		}
		break;
	case ft_signed:
	case ft_sint8:
	case ft_sint16:
	case ft_sint32:
	case ft_sint64:
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding signed varint\n";
		decodeSVarint(G,f);
		break;
	case ft_bool:
	case ft_fixed8:
	case ft_sfixed8:
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding 8bit\n";
		decode8bit(G,f);
		break;
	case ft_fixed16:
	case ft_sfixed16:
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding 16bit\n";
		decode16bit(G,f);
		break;
	case ft_fixed32:
	case ft_sfixed32:
	case ft_float:
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding 32bit\n";
		decode32bit(G,f);
		break;
	case ft_fixed64:
	case ft_sfixed64:
	case ft_double:
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding 64bit\n";
		decode64bit(G,f);
		break;
	}
	G << "break;\n";
	G.setField(0);
}


void CppGenerator::writeTagToMemory(Generator &G, Field *f)
{
	G << "// '$(fname)': id=$(field_id), encoding=$(field_enc), tag=$(field_tag) - " << optmode << "\n";
	if (optmode == optspeed) {
		unsigned id = f->getId();
		uint32_t encoding = f->getEncoding();
		unsigned xid = (id << 3) | encoding;
		char buf[32];
		if (f->isPacked())
			xid = (id << 3) | 2;
		if (xid < 0x80) {
			sprintf(buf,"%x",xid);
			G <<	"if (a >= e)\n"
				"	$handle_error;\n"
				"*a++ = 0x" << buf << ";\n";
		} else {
			unsigned es = wiresize_u64(xid);
			G <<	"if (" << es << " > (e-a))\n"
				"	$handle_error;\n";
			unsigned bid = xid;
			while (bid&(~0x7f)) {
				sprintf(buf,"%x",(bid&0x7f)|0x80);
				G << "*a++ = 0x" << buf << ";\n";
				bid >>= 7;
			}
			sprintf(buf,"%x",bid&0x7f);
			G <<	"*a++ = 0x" << buf << ";\n";
		}
	} else if (optmode == optreview) {
		G <<	"n = write_varint(a,e-a,$(field_tag));\t// '$(fname)': id=$(field_id)\n"
			"if (n == 0)\n"
			"	$handle_error;\n"
			"a += n;\n";
	} else {
		unsigned id = f->getId();
		uint32_t encoding = f->getEncoding();
		unsigned xid = (id << 3) | encoding;
		if (f->isPacked())
			xid = (id << 3) | 2;
		if (xid < 0x80) {
			char buf[8];
			sprintf(buf,"%x",xid);
			G <<	"if (a >= e)\n"
				"	$handle_error;\n"
				"*a++ = 0x" << buf << ";\n";
		} else if (xid < 0xc000) {
			char buf[32];
			unsigned es = wiresize_u64(xid);
			G <<	"if (" << es << " > (e-a))\n"
				"	$handle_error;\n";
			unsigned bid = xid;
			while (bid&(~0x7f)) {
				sprintf(buf,"%x",(bid&0x7f)|0x80);
				G << "*a++ = 0x" << buf << ";\n";
				bid >>= 7;
			}
			sprintf(buf,"%x",bid&0x7f);
			G <<	"*a++ = 0x" << buf << ";\n";
		} else {
			G <<	"n = write_varint(a,e-a,$(field_tag));\t// '$(fname)': id=$(field_id)\n"
				"if (n <= 0)\n"
				"	$handle_error;\n"
				"a += n;\n";
		}
	}
}


void CppGenerator::writeTagToX(Generator &G, Field *f)
{
	G << "// '$(fname)': id=$(field_id), encoding=$(field_enc), tag=$(field_tag)\n";
	unsigned id = f->getId();
	if (optmode == optspeed) {
		uint32_t encoding = f->getEncoding();
		unsigned xid = (id << 3) | encoding;
		if (f->isPacked())
			xid = (id << 3) | 2;
		if (xid < 0x80) {
			G <<	"$wireput($field_tag);\t// '$(fname)': id=$(field_id)\n";
		} else {
			unsigned bid = xid;
			do {
				G.setVariableHex("subtag",(bid&0x7f)|0x80);
				G << "$wireput($subtag);\n";
				bid >>= 7;
			} while (bid&(~0x7f));
			G.setVariableHex("subtag",bid&0x7f);
			G <<	"$wireput($subtag);\n";
		}
	} else if ((id << 3) < 128)
		G << "$wireput($(field_tag));\t// '$(fname)': id=$(field_id)\n";
	else
		G << "$write_varint($(field_tag));\t// '$(fname)': id=$(field_id)\n";
}


void CppGenerator::writeMaxSize(Generator &G, Message *m)
{
	if (!G.hasValue("getMaxSize"))
		return;
	int64_t maxsize = 0;
	for (unsigned i = 0, e = m->numFields(); i != e; ++i) {
		Field *f = m->getField(i);
		if ((f == 0) || (!f->isUsed()))
			continue;
		int64_t s = f->getMaxSize();
		if (s == 0) {
			maxsize = 0;
			break;
		}
		maxsize += s;
	}
	G.setVariableDec("maxsize",maxsize);
	G <<	"$(inline)size_t $(prefix)$(msg_name)::$getMaxSize() const\n"
		"{\n"
		"	return $maxsize;\n"
		"}\n"
		"\n";
}


void CppGenerator::writeToMemory(Generator &G, Field *f)
{
	G.setField(f);
	uint32_t type = f->getType();
	uint8_t quan = f->getQuantifier();
	uint32_t encoding = f->getEncoding();
	G.addVariable("index","");
	switch (quan) {
	case q_optional:
		if (optmode == optreview) {
			G <<	"if ($(field_has)()) {\n";
		} else {
			G <<	"// has $fname?\n"
				"if (";
			writeGetValid(G,f);
			G <<	") {\n";
		}
		break;
	case q_repeated:
		if (f->isPacked() && f->hasSimpleType()) {
			// packed encoding: tag,length,data
			G <<	"if (size_t $(fname)_ne = m_$(fname).size()) {\n";
			G.setVariableHex("field_tag",f->getId()<<3|2);
			writeTagToMemory(G,f);
			if (((type&ft_filter) == ft_enum) ||  (type == ft_uint16) || (type == ft_uint32) || (type == ft_uint64) || (type == ft_int64)) {
				G <<	"ssize_t $(fname)_ws = 0;\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
					"	$(fname)_ws += $(wiresize_u)(m_$(fname)[x]);\n"
					"n = write_varint(a,e-a,$(fname)_ws);\n"
					"a += n;\n"
					"if ((n <= 0) || ($(fname)_ws > (e-a)))\n"
					"	$handle_error;\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
					"	a += write_varint(a,e-a,m_$(fname)[x]);\n";
			} else if ((type == ft_int8) || (type == ft_int16) || (type == ft_int32)) {
				// negative numbers with varint_t < 64bit need padding
				G <<	"ssize_t $(fname)_ws = 0;\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
					"	$(fname)_ws += $(wiresize_u)(m_$(fname)[x]);\n"
					"n = write_varint(a,e-a,$(fname)_ws);\n"
					"a += n;\n"
					"if ((n <= 0) || ($(fname)_ws > (e-a)))\n"
					"	$handle_error;\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n";
				if (VarIntBits < 64)
					G<< "	a += write_xvarint(a,e-a,m_$(fname)[x]);\n";
				else
					G<< "	a += write_varint(a,e-a,m_$(fname)[x]);\n";
			} else if ((type == ft_sint8) || (type == ft_sint16) || (type == ft_sint32) || (type == ft_sint64)) {
				G <<	"ssize_t $(fname)_ws = 0;\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
					"	$(fname)_ws += $(wiresize_s)(m_$(fname)[x]);\n"
					"n = write_varint(a,e-a,$(fname)_ws);\n"
					"a += n;\n"
					"if ((n <= 0) || ($(fname)_ws > (e-a)))\n"
					"	$handle_error;\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
					"	a += write_varint(a,e-a,sint_varint(m_$(fname)[x]));\n";
			} else if ((type == ft_bool) || (type == ft_fixed8) ||  (type == ft_sfixed8) || (type == ft_uint8)) {
				G <<	"ssize_t $(fname)_ws = $(fname)_ne * sizeof($(typestr));\n"
					"n = write_varint(a,e-a,$(fname)_ws);\n"
					"a += n;\n"
					"if ((n <= 0) || ($(fname)_ws > (e-a)))\n"
					"	$handle_error;\n"
					"memcpy(a,m_$(fname).data(),$(fname)_ws);\n"
					"a += $(fname)_ws;\n";
			} else if ((Endian == little_endian) && f->hasFixedSize()) {
				G <<	"ssize_t $(fname)_ws = $(fname)_ne * sizeof($(typestr));\n"
					"n = write_varint(a,e-a,$(fname)_ws);\n"
					"a += n;\n"
					"if ((n <= 0) || ($(fname)_ws > (e-a)))\n"
					"	$handle_error;\n"
					"memcpy(a,m_$(fname).data(),$(fname)_ws);\n"
					"a += $(fname)_ws;\n";
			} else if ((type == ft_fixed16) || (type == ft_sfixed16)) {
				G <<	"ssize_t $(fname)_ws = $(fname)_ne << 1;\n"
					"n = write_varint(a,e-a,$(fname)_ws);\n"
					"a += n;\n"
					"if ((n <= 0) || ($(fname)_ws > (e-a)))\n"
					"	$handle_error;\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x, a+=2)\n"
					"	write_u16(a,*(uint16_t*)&m_$(fname)[x]);\n";
			} else if ((type == ft_fixed32) || (type == ft_sfixed32)) {
				G <<	"ssize_t $(fname)_ws = $(fname)_ne << 2;\n"
					"n = write_varint(a,e-a,$(fname)_ws);\n"
					"a += n;\n"
					"if ((n <= 0) || ($(fname)_ws > (e-a)))\n"
					"	$handle_error;\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x, a+=4)\n"
					"	write_u32(a,m_$(fname)[x]);\n";
			} else if (type == ft_float) {
				G <<	"ssize_t $(fname)_ws = $(fname)_ne << 2;\n"
					"n = write_varint(a,e-a,$(fname)_ws);\n"
					"a += n;\n"
					"if ((n <= 0) || ($(fname)_ws > (e-a)))\n"
					"	$handle_error;\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x, a+=4)\n"
					"	write_u32(a,mangle_float(m_$(fname)[x]));\n";
			} else if ((type == ft_fixed64) || (type == ft_sfixed64)) {
				G <<	"ssize_t $(fname)_ws = $(fname)_ne << 3;\n"
					"n = write_varint(a,e-a,$(fname)_ws);\n"
					"a += n;\n"
					"if ((n <= 0) || ($(fname)_ws > (e-a)))\n"
					"	$handle_error;\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x, a+=8)\n"
					"	write_u64(a,m_$(fname)[x]);\n";
			} else if (type == ft_double) {
				G <<	"ssize_t $(fname)_ws = $(fname)_ne << 3;\n"
					"n = write_varint(a,e-a,$(fname)_ws);\n"
					"a += n;\n"
					"if ((n <= 0) || ($(fname)_ws > (e-a)))\n"
					"	$handle_error;\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x, a+=8)\n"
					"	write_u64(a,mangle_double(m_$(fname)[x]));\n";
			} else
				abort();
			G	<< "}\n";
			G.setField(0);
			G.clearVariable("index");
			return;
		}
		G <<	"for (size_t x = 0, y = m_$(fname).size(); x != y; ++x) {\n";
		G.setVariable("index","[x]");
		break;
	case q_required:
		break;
	default:
		abort();
	}
	writeTagToMemory(G,f);
	if (encoding == wt_lenpfx) {
		if ((type == ft_string) || (type == ft_bytes) || (type == ft_cptr))
			encoding = 0x12;	// string or byte array
	}
	switch (encoding) {
	case wt_varint:	// varint
		if ((type == ft_sint8) || (type == ft_sint16) || (type == ft_sint32) || (type == ft_sint64))
			G <<	"n = write_varint(a,e-a,sint_varint(m_$(fname)$(index)));\n"
				"if (n <= 0)\n"
				"	$handle_error;\n"
				"a += n;\n";
		else if ((VarIntBits < 64) && ((type == ft_int8) || (type == ft_int16) || (type == ft_int32)))
			// these types may need padding when varint_t is
			// < 64bit to be compatible with 64bit systems
			G <<	"n = write_xvarint(a,e-a,m_$(fname)$(index));\n"
				"if (n <= 0)\n"
				"	$handle_error;\n"
				"a += n;\n";
		else
			G <<	"n = write_varint(a,e-a,m_$(fname)$(index));\n"
				"if (n <= 0)\n"
				"	$handle_error;\n"
				"a += n;\n";
		break;
	case wt_64bit:	// 64bit value
		G <<	"if ((a+8) > e)\n"
			"	$handle_error;\n";
		if (type == ft_double) {
			if ((Endian == little_endian) && (optmode == optspeed)) {
				G <<	"*(double*)a = m_$(fname)$(index);\n";
			} else {
				G <<	"write_u64(a,mangle_double(m_$(fname)$(index)));\n";
			}
		} else {
			if ((Endian == little_endian) && (optmode == optspeed)) {
				G <<	"*(uint64_t*)a = m_$(fname)$(index);\n";
			} else {
				G <<	"write_u64(a,(uint64_t)m_$(fname)$(index));\n";
			}
		}
		G <<	"a += 8;\n";
		break;
	case wt_lenpfx: 	// length delimited message
		if (PaddedMsgSize) {
			G <<	"if ((e-a) < (ssize_t)(sizeof(varint_t)*8/7+1))\n"
				"	$handle_error;\n"
				"n = m_$(fname)$(index).$toMemory(a+(sizeof(varint_t)*8/7+1),e-a-(sizeof(varint_t)*8/7+1));\n"
				"if (n < 0)\n"	// == 0 is ok, could be empty message
				"	$handle_error;\n"
				"place_varint(a,n);\n"
				"a += sizeof(varint_t)*8/7+1;\n"
				"a += n;\n";
		} else {
			G <<	"ssize_t $(fname)_ws = m_$(fname)$(index).$calcSize();\n"
				"n = write_varint(a,e-a,$(fname)_ws);\n"
				"a += n;\n"
				"if ((n <= 0) || ($(fname)_ws > (e-a)))\n"
				"	$handle_error;\n"
				"n = m_$(fname)$(index).$toMemory(a,e-a);\n"
				"a += n;\n";
			if (Asserts)
				G << "assert(n == $(fname)_ws);\n";
		}
		break;
	case wt_8bit:	// 8bit value
		G <<	"if (a >= e)\n"
			"	$handle_error;\n"
			"*a++ = m_$(fname)$(index);\n";
		break;
	case wt_16bit:
		G <<	"if ((e-a) < 2)\n"
			"	$handle_error;\n";
		G <<	"write_u16(a,m_$(fname)$(index));\n";
		G <<	"a += 2;\n";
		break;
	case 0x12:	// length delimited byte array or string
		if (type == ft_cptr) 
			G <<	"if (m_$(fname)$(index)) {\n"
				"	ssize_t $(fname)_s = strlen(m_$(fname)$(index)) + 1;\n"
				"	n = write_varint(a,e-a,$(fname)_s);\n"
				"	a += n;\n"
				"	if ((n <= 0) || ((e-a) < $(fname)_s))\n"
				"		$handle_error;\n"
				"	memcpy(a,m_$(fname)$(index),$(fname)_s);\n"
				"	a += $(fname)_s;\n"
				"} else {\n"
				"	// transmit empty string of lenght 1\n"
				"	if (e-a < 2)\n"
				"		$handle_error;\n"
				"	*a++ = 1;\n"
				"	*a++ = 0;\n"
				"}\n";
		else
			G <<	"ssize_t $(fname)_s = m_$(fname)$(index).size();\n"
				"n = write_varint(a,e-a,$(fname)_s);\n"
				"a += n;\n"
				"if ((n <= 0) || ((e-a) < $(fname)_s))\n"
				"	$handle_error;\n"
				"memcpy(a,m_$(fname)$(index).data(),$(fname)_s);\n"
				"a += $(fname)_s;\n";
		break;
	case wt_32bit:	// 32bit value
		G <<	"if ((e-a) < 4)\n"
			"	$handle_error;\n";
		if (type == ft_float) {
			G <<	"write_u32(a,mangle_float(m_$(fname)$(index)));\n";
		} else {
			G <<	"write_u32(a,(uint32_t)m_$(fname)$(index));\n";
		}
		G <<	"a += 4;\n";
		break;
	default:
		abort();
	}
	if (quan != 1) {
		G << "}\n";
	}
	G.clearVariable("index");
	G.setField(0);
	
}


void CppGenerator::writeToX(Generator &G, Field *f)
{
	G.setField(f);
	uint32_t type = f->getType();
	uint8_t quan = f->getQuantifier();
	uint32_t encoding = f->getEncoding();
	G.addVariable("index","");
	switch (quan) {
	case q_optional:
		if (optmode == optspeed) {
			G << "if (";
			writeGetValid(G,f);
			G << ") {\n";
		} else
			G << "if ($(field_has)()) {\n";
		break;
	case q_repeated:
		if (f->isPacked() && f->hasSimpleType()) {
			// packed encoding: tag,length,data
			G <<	"if (size_t $(fname)_ne = m_$(fname).size()) {\n";
			G.setVariableHex("field_tag",f->getId()<<3|2);
			writeTagToX(G,f);
			if (((type&ft_filter) == ft_enum) || (type == ft_int64) ||  (type == ft_uint8) ||  (type == ft_uint16) || (type == ft_uint32) || (type == ft_uint64)) {
				G <<	"size_t $(fname)_ws = 0;\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
					"$(fname)_ws += $(wiresize_u)(m_$(fname)[x]);\n"
					"$write_varint($(fname)_ws);\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
					"$write_varint(m_$(fname)[x]);\n";
			} else if ((type == ft_int8) || (type == ft_int16) || (type == ft_int32)) {
				G <<	"size_t $(fname)_ws = 0;\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
					"$(fname)_ws += $(wiresize_x)(m_$(fname)[x]);\n"
					"$write_varint($(fname)_ws);\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n";
				if (VarIntBits < 64)
					G << "	$write_xvarint(m_$(fname)[x]);\n";
				else
					G << "	$write_varint(m_$(fname)[x]);\n";
			} else if ((type == ft_sint8) || (type == ft_sint16) || (type == ft_sint32) || (type == ft_sint64)) {
				G <<	"size_t $(fname)_ws = 0;\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
					"$(fname)_ws += $(wiresize_s)(m_$(fname)[x]);\n"
					"$write_varint($(fname)_ws);\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
					"$write_varint(sint_varint(m_$(fname)[x]));\n";
			} else if ((type == ft_bool) || (type == ft_fixed8) || (type == ft_sfixed8)) {
				G <<	"$write_varint($(fname)_ne);\n"
					"$write_bytes((const uint8_t *)m_$(fname).data(),$(fname)_ne);\n";
			} else if ((Endian == little_endian) && ((type == ft_fixed16) || (type == ft_fixed32) || (type == ft_fixed64) || (type == ft_float) || (type == ft_double))) {
				G <<	"size_t $(fname)_ws = $(fname)_ne * sizeof($(typestr));\n"
					"$write_varint($(fname)_ws);\n"
					"$write_bytes((const uint8_t *)m_$(fname).data(),$(fname)_ws);\n";
			} else if ((type == ft_fixed16) || (type == ft_sfixed16)) {
				G <<	"$write_varint($(fname)_ne << 1);\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
					"$(u16_wire(*(uint16_t*)&m_$(fname)[x]));\n";
			} else if ((type == ft_fixed32) || (type == ft_sfixed32) || (type == ft_float)) {
				G <<	"$write_varint($(fname)_ne << 2);\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
					"$(u32_wire(*(uint32_t*)&m_$(fname)[x]));\n";
			} else if ((type == ft_fixed64) || (type == ft_sfixed64) || (type == ft_double)) {
				G <<	"$write_varint($(fname)_ne << 3);\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
					"$(u64_wire(*(uint64_t*)&m_$(fname)[x]));\n";
			}
			else
				abort();
			G	<< "}\n";
			G.setField(0);
			G.clearVariable("index");
			return;
		}
		G <<	"for (size_t x = 0, y = m_$(fname).size(); x != y; ++x) {\n";
		G.setVariable("index","[x]");
		break;
	case q_required:
		break;
	default:
		abort();
	}
	writeTagToX(G,f);
	if (encoding == wt_lenpfx) {
		if ((type == ft_string) || (type == ft_bytes) || (type == ft_cptr))
			encoding = wt_msg;	// string or byte array
	}
	switch (encoding) {
	case wt_varint:	// varint
		if ((type == ft_sint8) || (type == ft_sint16) || (type == ft_sint32) || (type == ft_sint64))
			G <<	"$write_varint(sint_varint(m_$(fname)$(index)));\n";
		else if ((VarIntBits < 64) && ((type == ft_int8) || (type == ft_int16) || (type == ft_int32)))
			G <<	"$write_xvarint(m_$(fname)$(index));\n";
		else
			G <<	"$write_varint(m_$(fname)$(index));\n";
		break;
	case wt_64bit:	// 64bit value
		if (type == ft_double) {
			G <<	"$(u64_wire(mangle_double(m_$(fname)$(index))));\n";
		} else
			G <<	"$(u64_wire((uint64_t)m_$(fname)$(index)));\n";
		break;
	case wt_lenpfx: 	// length delimited message
		G <<	"$write_varint(m_$(fname)$(index).$calcSize());\n"
			"m_$(fname)$(index).$(toX)($putarg);\n";
		break;
	case wt_8bit:	// 8bit value
		G <<	"$wireput(m_$(fname)$(index));\n";
		break;
	case wt_16bit:
		G <<	"$(u16_wire(m_$(fname)$(index)));\n";
		break;
	case wt_msg:	// length delimited byte array or string
		if (type == ft_cptr) {
			G <<	"if (m_$(fname)$(index)) {\n"
				"	size_t $(fname)_s = strlen(m_$(fname)$(index)) + 1;\n"
			 	"	$write_varint($(fname)_s);\n"
				"	uint8_t *$(fname)_d = (uint8_t*) m_$(fname)$(index);\n"
				"	do {\n"
				"	$wireput(*$(fname)_d++);\n"
				"	} while (--$(fname)_s);\n"
				"} else {\n"
				"	// write 0x01 0x00 for empty C string\n"
				"	$wireput(1);\n"
				"	$wireput(0);\n"
				"}\n";
		} else {
			G <<	"size_t $(fname)_s = m_$(fname)$(index).size();\n"
			 	"$write_varint($(fname)_s);\n"
				"$write_bytes((const uint8_t*) m_$(fname)$(index).data(),$(fname)_s);\n";
		}
		break;
	case wt_32bit:	// 32bit value
		if (type == ft_float) {
			G <<	"$(u32_wire(mangle_float(m_$(fname)$(index))));\n";
		} else {
			G <<	"$(u32_wire((uint32_t)m_$(fname)$(index)));\n";
		}
		break;
	default:
		abort();
	}
	if (quan != 1)
		G << "}\n";
	G.clearVariable("index");
	G.setField(0);
	
}


void CppGenerator::writeFromMemory(Generator &G, Message *m)
{
	G <<	"ssize_t $(prefix)$(msg_name)::$(fromMemory)(const void *b, ssize_t s)\n"
		"{\n"
		"const uint8_t *a = (const uint8_t *)b;\n"
		"const uint8_t *e = a + s;\n";
	if (Debug)
		G << "std::cout << \"$(msg_fullname)::$(fromMemory)(\" << (void*)b << \", \" << s << \")\\n\";\n";
	G <<	"while (a < e) {\n"
		"varint_t fid;\n";
	const string &Terminator = target->getOption("Terminator");
	if ((Terminator == "ff") || (Terminator == "0xff"))
		G <<	"if (*a == 0xff)\t// 0xff terminator\n"
			"break;\n";
	G <<	"int fn = read_varint(a,e-a,&fid);\n"
		"a += fn;\n"
		"if (fn <= 0)\n"
		"	$handle_error;\n"
		"switch (fid) {\n";
	bool hasNullId = false;
	for (unsigned i = 0, e = m->numFields(); i != e; ++i) {
		Field *f = m->getField(i);
		if (f == 0)
			continue;
		if (!f->isUsed()) {
			hasUnused = true;
			continue;
		}
		if (f->getId() == 0) {
			wiretype_t e = f->getEncoding();
			if ((e == wt_varint) || (e == wt_dynamic))
				hasNullId = true;
		}
		writeFromMemory(G,f);
	}
	if (target->getOption("UnknownField") == "assert") {
		// need to skip known but unused fields to avoid asserts
		bool unused = false;
		for (unsigned i = 0, e = m->numFields(); i != e; ++i) {
			Field *f = m->getField(i);
			if ((f == 0) || (f->isUsed()))
				continue;
			unused = true;
			G.setField(f);
			if (f->isPacked())
				G.setVariableHex("field_tag",(int64_t)f->getId()<<3|2);
			G <<	"case $(field_tag):\t// $(fname) id $(field_id), unused\n";
			if (optmode == optspeed)
				writeSkipContent(G,f->getEncoding());
			G.setField(0);
		}
		if (unused && (optmode != optspeed)) {
			// Unused fields should not be assert(0)'ed
			// as unknown are. Therefore we need a dedicated
			// handling for the if unkown=assert
			// TODO: optimize speed:
			// dedicated skipping according to field type?
			G <<	"{\n"
				"	ssize_t s = skip_content(a,e-a,fid&7);\n"
				"	if (s <= 0)\n"
				"		$handle_error;\n"
				"	a += s;\n"
				"	break;\n"
				"}\n";
		}
	}
	if ((Terminator == "null") || (Terminator == "0")) {
		if (hasNullId) 
			error("request to handle null termination, but null-tag exists");
		G <<	"case 0:\t// terminate on null byte\n"
			"	break;\n";
	}
	G <<	"default:\n";
	if (target->getOption("UnknownField") == "assert") {
		G << "assert(0);	// unknown field (option unknwon=assert)\n";
	} else if (target->getOption("UnknownField") == "skip") {
		G <<	"{\n"
			"	ssize_t s = skip_content(a,e-a,fid&7);\n"
			"	if (s <= 0)\n"
			"		$handle_error;\n"
			"	a += s;\n"
			"	break;\n"
			"}\n";
	} else 
		fatal("unable to handle option unknown with value %s",target->getOption("UnknownField").c_str());
	G <<	"}\n"
		"}\n";
	if (Asserts)
		G << "assert((a-(const uint8_t *)b) == s);\n";
	G <<	"if (a > e)\n"
		"	$handle_error;\n"
		"return a-(const uint8_t *)b;\n"
		"}\n"
		"\n";
}


void CppGenerator::writeToMemory(Generator &G, Message *m)
{
	G <<	"size_t $(prefix)$(msg_name)::$toMemory(uint8_t *b, ssize_t s) const\n"
		"{\n";
	if (Debug)
		G << "std::cout << \"$(prefix)$(msg_name)::$toMemory(\" << (void*)b << \", \" << s << \")\\n\";\n";
	if (Asserts)
		G << "assert(s >= 0);\n";
	G <<	"uint8_t *a = b, *e = b + s;\n";
	bool needN = false;
	if (optmode == optspeed) {
		for (unsigned i = 0, e = m->numFields(); i != e; ++i) {
			Field *f = m->getField(i);
			if ((f == 0) || (!f->isUsed()))
				continue;
			if (f->isRepeated() || !f->hasFixedSize() || (f->getTagSize() > 1)) {
				needN = true;
				break;
			}
		}
	} else if (optmode == optsize) {
		for (unsigned i = 0, e = m->numFields(); i != e; ++i) {
			Field *f = m->getField(i);
			if ((f == 0) || (!f->isUsed()))
				continue;
			if (f->isRepeated() || !f->hasFixedSize() || (f->getTagSize() > 2)) {
				needN = true;
				break;
			}
		}
	} else {
		needN = true;
	}
	if (needN)
		G <<	"signed n;\n";
	//else
		//G <<	"// temporary signed n is not needed\n";
	for (unsigned i = 0, e = m->numFields(); i != e; ++i) {
		Field *f = m->getField(i);
		if ((f == 0) || (!f->isUsed()))
			continue;
		writeToMemory(G,f);
	}
	//if (Asserts)
		//G <<	"assert((a-b) == (signed)$calcSize());\n";
	const string &Terminator = target->getOption("Terminator");
	if ((Terminator == "ff") || (Terminator == "0xff"))
		G <<	"// write terminating ff byte\n"
			"*a++ = 0xff;\n";
	else if ((Terminator == "null") || (Terminator == "0"))
		G <<	"// write terminating null byte\n"
			"*a++ = 0;\n";
	if (Asserts)
		G << "assert(a <= e);\n";
	G <<	"return a-b;\n"
		"}\n"
		"\n";
}


void CppGenerator::writeToJson(Generator &G, Field *f, unsigned comma, bool last)
{
	// comma: 0 = none, 1 = optional, 2 = always
	uint8_t quan = f->getQuantifier();
	uint32_t type = f->getType();
	const char *writevalue = 0;
	if (f->isEnum())
		writevalue = "json.put('\"');\n"
			"json << $(strfun)(m_$(fname)$(index));\n"
			"json.put('\"');\n";
	else if (type == ft_bool)
		writevalue = "json << (m_$(fname)$(index) ? \"true\" : \"false\");\n";
	else if (f->isNumeric())
		if ((type == ft_float) || (type == ft_double))
			writevalue = "to_dblstr(json,m_$(fname)$(index));\n";
		else
			writevalue = "to_decstr(json,m_$(fname)$(index));\n";
	else if (f->isString())
		switch (f->getType()) {
		case ft_bytes:
		case ft_string:
			writevalue = "json_string(json,m_$(fname)$(index));\n";
			break;
		case ft_cptr:
			writevalue = "json_cstr(json,m_$(fname)$(index));\n";
			break;
		default:
			abort();
		}
	else if (f->isMessage())
		writevalue = "m_$(fname)$(index).$(toJSON)(json,indLvl);\n";
	else
		abort();

	G.setVariable("index","");
	G.setVariable("sep",comma ? "," : "");
	switch (quan) {
		break;
	case q_optional:
		G <<	"if ($(field_has)()) {\n";
		if (comma == 1)
			G <<	"if (needComma)\n";
		if (comma > 0)
			G <<	"json << \",\\n\";\n";
		G <<	"$json_indent(json,indLvl);\n"
			"json << \"\\\"$(fname)\\\":\";\n"
			<< writevalue;
		if ((comma < 2) && !last)
			G <<	"needComma = true;\n";
		G <<	"}\n";
		break;
	case q_required:
		if (comma == 1)
			G <<	"if (needComma)\n";
		if (comma > 0)
			G <<	"json << \",\\n\";\n";
		G <<	"$json_indent(json,indLvl);\n"
			"json << \"\\\"$(fname)\\\":\";\n";
		G <<	writevalue;
		break;
	case q_repeated:
		G.setVariable("index","[i]");
		G <<	"if (size_t n = m_$(fname).size()) {\n";
		if (comma == 1)
			G <<	"if (needComma)\n";
		if (comma > 0)
			G <<	"json << \",\\n\";\n";
		G <<	"$json_indent(json,indLvl);\n"
			"indLvl += 2;\n"
			"json << \"\\\"$(fname)\\\":[\\n\";\n"
			"for (size_t i = 0; i < n; ++i) {\n"
			"$json_indent(json,indLvl);\n"
			<< writevalue <<
			"if ((i+1) != n)\n"
			"json.put(',');\n"
			"json.put('\\n');\n"
			"}\n"
			"indLvl -= 2;\n"
			"$json_indent(json,indLvl);\n"
			"json.put(']');\n";
		if ((comma < 2) && !last)
			G << "needComma = true;\n";
		G <<	"}\n";
		break;
	default:
		abort();
	}
	G.clearVariable("index");
	G.clearVariable("sep");
}


void CppGenerator::writeToJson(Generator &G, Message *m)
{
	// commaFlag is needed if first element is optional and more
	// than one field is used in the message
	bool needCommaFlag = false;
	unsigned numUsed = 0;
	for (unsigned i = 0, e = m->numFields(); i != e; ++i) {
		Field *f = m->getField(i);
		if ((f == 0) || (!f->isUsed()))
			continue;
		++numUsed;
		if ((numUsed == 1) && (f->getQuantifier() == q_required))
			break;
		if (f->getQuantifier() != q_required)
			needCommaFlag = true;
	}
	G <<	"void $(prefix)$(msg_name)::$(toJSON)($(streamtype) &json, unsigned indLvl) const\n"
		"{\n";
	if (numUsed == 1)
		needCommaFlag = false;
	if (needCommaFlag)
		G <<	"bool needComma = false;\n";
	G <<	"json << \"{\\n\";\n";
	G <<	"++indLvl;\n"
		;

	unsigned comma = 0;	// 0: none, 1: testflag, 2: always
	for (unsigned i = 0, e = m->numFields(); i != e; ++i) {
		Field *f = m->getField(i);
		if ((f == 0) || (!f->isUsed()))
			continue;
		G.setField(f);
		writeToJson(G,f,comma,i == (e-1));
		G.setField(0);
		quant_t quan = f->getQuantifier();
		switch (quan) {
		case q_optional:
		case q_repeated:
			if (comma == 0)
				comma = 1;
			break;
		case q_required:
			comma = 2;
			break;
		default:
			abort();
		}
	}
	G <<	"json.put('\\n');\n"
		"--indLvl;\n"
		"$json_indent(json,indLvl);\n"
		"json.put('}');\n"
		"if (indLvl == 0)\n"
		"	json.put('\\n');\n"
		"}\n"
		"\n";
}


void CppGenerator::writeToX(Generator &G, Message *m)
{
	assert(G.hasValue("toX"));
	G <<	"$(sink_template)void $(prefix)$(msg_name)::$(toX)($putparam) const\n"
		"{\n";
	if (Debug)
		G << "std::cout << \"$(prefix)$(msg_name)::$(toX)($(putparam))\\n\";\n";
	for (unsigned i = 0, e = m->numFields(); i != e; ++i) {
		Field *f = m->getField(i);
		if ((f == 0) || (!f->isUsed()))
			continue;
		writeToX(G,f);
	}
	const string &Terminator = target->getOption("Terminator");
	if ((Terminator == "ff") || (Terminator == "0xff"))
		G <<	"// write terminating ff byte\n"
			"$wireput(0xff);\n";
	else if ((Terminator == "null") || (Terminator == "0"))
		G <<	"// write terminating null byte\n"
			"$wireput(0);\n";
	G <<	"}\n"
		"\n";
}


void CppGenerator::writeClear(Generator &G, Message *m)
{
	G <<	"void $(prefix)$(msg_name)::$(msg_clear)()\n"
		"{\n";
	for (unsigned i = 0, e = m->numFields(); i != e; ++i) {
		Field *f = m->getField(i);
		if ((f == 0) || (!f->isUsed()))
			continue;
		G.setVariable("fname",f->getName());
		uint8_t q = f->getQuantifier();
		if ((q == q_repeated) || (!f->hasSimpleType()))
			G << "m_$(fname).$(msg_clear)();\n";
		else if (const char *def = f->getDefaultValue())
			G << "m_$(fname) = " << def << ";\n";
		else
			G << "m_$(fname) = 0;\n";
	}
	G.clearVariable("fname");
	unsigned nv = m->getNumValid();
	diag("message %s: %u valid bits",m->getName().c_str(),nv);
	if (nv == 0);
	else if (nv <= 64)
		G << "p_validbits = 0;\n";
	else {
		for (unsigned n = 0; n < nv; n += 8)
			G << "p_validbits[" << n/8 << "] = 0;\n";
		if (nv%8)
			G << "p_validbits[" << (nv/8+1) << "] = 0;\n";
	}
	G <<	"}\n\n";
}


void CppGenerator::writeCmp(Generator &G, Message *m)
{
	bool wUE = target->getFlag("withUnequal");
	if (wUE) {
		G <<	"bool $(prefix)$(msg_name)::operator != (const $(prefix)$(msg_name) &r) const\n"
			"{\n";
		for (unsigned i = 0, e = m->numFields(); i != e; ++i) {
			Field *f = m->getField(i);
			if ((f == 0) || (!f->isUsed()))
				continue;
			G.setField(f);
			quant_t q = f->getQuantifier();
			if (ft_cptr == f->getType()) {
				if (q == q_repeated) {
					G <<	"if (m_$(fname).size() != r.m_$(fname).size())\n"
						"	return true;\n"
						"for (size_t x = 0, y = m_$(fname).size(); x != y; ++x) {\n";
					G.setVariable("index","[x]");
				} else
					G.setVariable("index","");
				G <<	"if (m_$(fname)$(index) != r.m_$(fname)$(index)) {\n"
					"	if ((m_$(fname)$(index) == 0) || (r.m_$(fname)$(index) == 0))\n"
					"		return true;\n"
					"	if (strcmp(m_$(fname)$(index),r.m_$(fname)$(index)))\n"
					"		return true;\n"
					"}\n";
				if (q == q_repeated)
					G << "}\n";
				G.clearVariable("index");
				G.setField(0);
				continue;
			}
			switch (q) {
			case q_required:
			case q_repeated:
				G <<	"if (m_$(fname) != r.m_$(fname))\n"
					"return true;\n";
				break;
			case q_optional:
				G <<	"if ($(field_has)() ^ r.$(field_has)())\n"
					"return true;\n"
					"if ($(field_has)() && (m_$(fname) != r.m_$(fname)))\n"
					"return true;\n";
				break;
			default:
				abort();
			}
			G.setField(0);
		}
		G <<	"return false;\n"
			"}\n\n\n";
	}

	if (target->getFlag("withEqual")) {
		if (wUE) {
			G <<	"bool $(prefix)$(msg_name)::operator == (const $(prefix)$(msg_name) &r) const\n"
				"{\n"
				"return !((*this) != r);\n"
				"}\n\n\n";
			return;
		}
		G <<	"bool $(prefix)$(msg_name)::operator == (const $(prefix)$(msg_name) &r) const\n"
			"{\n";
		for (unsigned i = 0, e = m->numFields(); i != e; ++i) {
			Field *f = m->getField(i);
			if ((f == 0) || (!f->isUsed()))
				continue;
			G.setField(f);
			quant_t q = f->getQuantifier();
			if (ft_cptr == f->getType()) {
				if (q == q_repeated) {
					G <<	"if (m_$(fname).size() != r.m_$(fname).size())\n"
						"	return false;\n"
						"for (size_t x = 0, y = m_$(fname).size(); x != y; ++x) {\n";
					G.setVariable("index","[x]");
				} else
					G.setVariable("index","");
				G <<	"if (m_$(fname)$(index) != r.m_$(fname)$(index)) {\n"
					"	if ((m_$(fname)$(index) == 0) || (r.m_$(fname)$(index) == 0))\n"
					"		return false;\n"
					"	if (strcmp(m_$(fname)$(index),r.m_$(fname)$(index)))\n"
					"		return false;\n"
					"}\n";
				if (q == q_repeated)
					G << "}\n";
				G.clearVariable("index");
				G.setField(0);
				continue;
			}
			switch (q) {
			case q_required:
			case q_repeated:
				if (wUE)
					G <<	"if (m_$(fname) != r.m_$(fname))\n"
						"return false;\n";
				else
					G <<	"if (!(m_$(fname) == r.m_$(fname)))\n"
						"return false;\n";
				break;
			case q_optional:
				if (wUE)
					G <<	"if ($(field_has)() ^ r.$(field_has)())\n"
						"return false;\n"
						"if ($(field_has)() && (m_$(fname) != r.m_$(fname)))\n"
						"return false;\n";
				else
					G <<	"if ($(field_has)() ^ r.$(field_has)())\n"
						"return false;\n"
						"if ($(field_has)() && (!(m_$(fname) == r.m_$(fname))))\n"
						"return false;\n";
				break;
			default:
				abort();
			}
			G.setField(0);
		}
		G <<	"return true;\n"
			"}\n\n\n";
	}
}


void CppGenerator::writePrint(Generator &G, Field *f)
{
	G.setField(f);
	uint8_t quan = f->getQuantifier();
	uint32_t type = f->getType();
	G.addVariable("index","");
	switch (quan) {
	case q_optional:
		if ((type & ft_filter) == ft_msg)
			G << "if ($(field_has)()) {\n";
		else
			G << "if ($(field_has)())\n";
		/* FALLTHRU */
	case q_required:
		G << "o << p << \"\\t$(fname) = \"";
		break;
	case q_repeated:
		G << "for (size_t i = 0, e = m_$(fname).size(); i != e; ++i) {\n";
		G << "o << p << \"\\t$(fname)[\" << i << \"] = \"";
		G.setVariable("index","[i]");
		break;
	default:
		abort();
	}
	switch (type) {
	case ft_uint8:
	case ft_fixed8:
		G << " << (unsigned) m_$(fname)$(index) << \";\\n\";\n";
		break;

	case ft_int8:
	case ft_sint8:
	case ft_sfixed8:
		G << " << (signed) m_$(fname)$(index) << \";\\n\";\n";
		break;

	case ft_int32:
	case ft_uint32:
	case ft_sint32:
	case ft_int64:
	case ft_uint64:
	case ft_sint64:
	case ft_int16:
	case ft_uint16:
	case ft_sint16:
	case ft_fixed16:
	case ft_sfixed16:
	case ft_fixed32:
	case ft_sfixed32:
	case ft_float:
	case ft_fixed64:
	case ft_sfixed64:
	case ft_double:
		G << " << m_$(fname)$(index) << \";\\n\";\n";
		break;

	case ft_bytes:
		G << " << HexBytes(m_$(fname)$(index).data(),m_$(fname)$(index).size(),p+\"\\t\\t\") << \";\\n\";\n";
		break;
	case ft_string:
		G << " << C_ASCII(m_$(fname)$(index).c_str(),p+\"\\t\\t\") << \";\\n\";\n";
		break;
	case ft_cptr:
		G << " << C_ASCII(m_$(fname)$(index),p+\"\\t\\t\") << \";\\n\";\n";
		break;
	case ft_bool:
		G << " << (m_$(fname)$(index) ? \"true\" : \"false\") << \";\\n\";\n";
		break;

	default:
		if ((type & ft_filter) == ft_msg) {
			G << ";\n";
			if (PrintOut)
				G << "m_$(fname)$(index).$(toASCII)(o,p+'\\t');\n";
		} else if ((type & ft_filter) == ft_enum) {
			Enum *e = Enum::id2enum(type);
			assert(e);
			const string &strfun = e->getStringFunction();
			if (strfun.empty())
				G << " << m_$(fname)$(index) << \";\\n\";\n";
			else
				G << " << " << strfun << "(m_$(fname)$(index)) << \";\\n\";\n";
		} else
			abort();
	}
	if ((quan == q_optional) && ((type & ft_filter) == ft_msg))
		G << "}\n";
	else if (quan == q_repeated)
		G << "}\n";
	G.clearVariable("index");
	G.setField(0);
}


void CppGenerator::writePrint(Generator &G, Message *m)
{
	G <<	"void $(prefix)$(msg_name)::$(toASCII)($(streamtype) &o, const std::string &p) const\n"
		"{\n"
		"o << \"message $(msg_name) {\\n\";\n";
	for (unsigned i = 0, e = m->numFields(); i != e; ++i) {
		Field *f = m->getField(i);
		if ((f == 0) || (!f->isUsed()))
			continue;
		writePrint(G,f);
	}
	G <<	"o << p << \"}\\n\";\n"
		"}\n\n";
}


void CppGenerator::writeConstructor(Generator &G, Message *m)
{
	if (SubClasses)
		G << "$(prefix)$(msg_name)::$(msg_name)()\n";
	else
		G << "$(msg_name)::$(msg_name)()\n";
	writeMembers(G,m,false);
	unsigned nv = m->getNumValid();
	if (nv > 0)
		G << ", p_validbits(" << ((nv <= 64) ? "0)\n" : ")\n");
	G	<< "{\n}\n\n";
}


void CppGenerator::writeFunctions(Generator &G, Message *m)
{
	G.setMessage(m);
	writeConstructor(G,m);
	writeClear(G,m);
	// ::calcSize needed in all sender functions for writing message/field size into stream
	if (!inlineMaxSize)
		writeMaxSize(G,m);
	if (PrintOut)
		writePrint(G,m);
	if (G.hasValue("fromMemory")) {
		if (EarlyDecode)
			writeFromMemory_early(G,m);
		else
			writeFromMemory(G,m);
	}
	if (G.hasValue("toMemory"))
		writeToMemory(G,m);
	if (G.hasValue("toWire")) {
		G.setMode(gen_wire);
		G.setVariable("toX",G.getVariable("toWire"));
		writeToX(G,m);
	}
	if (G.hasValue("toSink") && !SinkToTemplate) {
		G.setMode(gen_sink);
		G.setVariable("toX",G.getVariable("toSink"));
		writeToX(G,m);
	}
	if (G.hasValue("toString")) {
		G.setMode(gen_string);
		G.setVariable("toX",G.getVariable("toString"));
		writeToX(G,m);
	}
	if (G.hasValue("toJSON"))
		writeToJson(G,m);
	if (needCalcSize)
		writeCalcSize(G,m);
	
	for (unsigned i = 0, e = m->numFields(); i != e; ++i) {
		if (Field *f = m->getField(i))
			if (f->isUsed())
				writeFunctions(G,f);
	}
	writeCmp(G,m);
	G.setMessage(0);
	for (unsigned i = 0, e = m->numMessages(); i != e; ++i) {
		writeFunctions(G,m->getMessage(i));
	}
	for (unsigned i = 0, n = m->numEnums(); i != n; ++i)
		writeEnumDefs(G,m->getEnum(i));
}


void CppGenerator::writeHelpers(vector<unsigned> &funcs)
{
	// "inline" helpers
	if (hasUInt || hasSInt) {
		// wiresize_s depends on wiresize
		funcs.push_back(ct_wiresize);
	}
	if (hasSInt) {
		funcs.push_back(ct_sint_varint);
		funcs.push_back(ct_varint_sint);
		funcs.push_back(ct_wiresize_s);
	}
	if (hasInt)
		funcs.push_back(ct_wiresize_x);

	if (target->isId("toMemory")) {
		if (hasU16)
			funcs.push_back(ct_write_u16);
		if (hasU32 || hasFloat)
			funcs.push_back(ct_write_u32);
		if (hasU64 || hasDouble)
			funcs.push_back(ct_write_u64);
	}

	if (target->isId("toMemory") || target->isId("toWire")) {
		if (hasFloat)
			funcs.push_back(ct_mangle_float);
		if (hasDouble)
			funcs.push_back(ct_mangle_double);
	}

	if (target->isId("toWire")) {
		funcs.push_back(gen_wire);
		if (hasU16)
			funcs.push_back(ct_send_u16);
		if (hasU32 || hasFloat)
			funcs.push_back(ct_send_u32);
		if (hasU64 || hasDouble)
			funcs.push_back(ct_send_u64);
	}
	if (target->isId("toString")) {
		funcs.push_back(gen_string);
		if (hasU16)
			funcs.push_back(ct_send_u16);
		if (hasU32 || hasFloat)
			funcs.push_back(ct_send_u32);
		if (hasU64 || hasDouble)
			funcs.push_back(ct_send_u64);
	}

	// "function" helpers
	unsigned mode = 0;
	if (target->isId("toWire")) {
		funcs.push_back(gen_wire);
		mode = gen_wire;
		funcs.push_back(ct_send_varint);
		if (hasLenPfx)
			funcs.push_back(ct_send_bytes);
		if (needSendVarSInt)
			funcs.push_back(ct_send_xvarint);
	}
	if (target->isId("toMemory")) {
		if (mode != gen_wire)
			funcs.push_back(gen_wire);
		funcs.push_back(ct_write_varint);
		if (PaddedMsgSize)
			funcs.push_back(ct_place_varint);
		if (needSendVarSInt)
			funcs.push_back(ct_write_xvarint);
	}

	if (target->isId("fromMemory")) {
		funcs.push_back(ct_read_varint);
		if (((target->getOption("UnknownField") == "skip") || ((optmode != optspeed) && hasUnused)) && (!EarlyDecode))
			funcs.push_back(ct_skip_content);
		if (hasDouble)
			funcs.push_back(ct_read_double);
		if (hasFloat)
			funcs.push_back(ct_read_float);
		if (hasU64 || EarlyDecode)
			funcs.push_back(ct_read_u64);
		if (hasU32 || EarlyDecode)
			funcs.push_back(ct_read_u32);
		if (hasU16 || EarlyDecode)
			funcs.push_back(ct_read_u16);
	}

	if (target->isId("toString")) {
		mode = gen_string;
		funcs.push_back(gen_string);
		funcs.push_back(ct_send_varint);
		funcs.push_back(ct_send_msg);
		if (needSendVarSInt)
			funcs.push_back(ct_send_xvarint);
	}
	if (WithJson) {
		funcs.push_back(ct_json_indent);
		funcs.push_back(ct_json_string);
		if (hasCStr)
			funcs.push_back(ct_json_cstr);
		funcs.push_back(ct_to_decstr);
		if (hasFloat|hasDouble)
			funcs.push_back(ct_to_dblstr);
	}
}


void CppGenerator::writeInlineHelpers(Generator &G, vector<unsigned> &funcs)
{
}


void CppGenerator::writeBody(const string &bn)
{
	string fn = bn + ".cpp";
	msg("generating %s...",fn.c_str());
	fstream out;
	out.open(fn.c_str(),fstream::out);
	writeInfos(out);
	if (Debug)
		out << "#include <iostream>\n";
	if (PrintOut || target->isId("toJSON"))
		out << "#include <ostream>\n";
	string hname;
	if (const char *lfs = strrchr(bn.c_str(),FILESEP))
		hname = lfs+1;
	else
		hname = bn;
	out <<	"#include <stdlib.h>\n"
		"#include <string.h>\n"
		"#include \"" << hname << ".h\"\n\n";	// must be before wirefuncs.h to define VARINTBITS
	Generator G(out,target);
	const string &ns = target->getOption("namespace");
	if (!ns.empty())
		G << "namespace " << ns << " {\n\n";
	const string &wfclib = target->getOption("wfclib");
	if (wfclib == "extern") {
		const string &libname = target->getOption("libname");
		out <<	"#include \"" << libname << ".h\"\n\n"
			"#if !defined(WFC_ENDIAN) || (WFC_ENDIAN != " << target->Endian() << ")\n"
			"#error wfc: incompatible settings concerning endian\n"
			"#endif\n";
	} else {
		vector<unsigned> funcs;
		writeHelpers(funcs);
		Lib.write_cpp(G,funcs,target);
	}
	needCalcSize = (target->isId("toSink") || target->isId("toMemory") || target->isId("toWire") || target->isId("toString"));
	G.setVariable("inline","");

	if (G.hasValue("BaseClass")) {
		G <<	"$BaseClass::~$BaseClass()\n"
			"{\n"
			"\n"
			"}\n"
			"\n"
			;
	}
	for (unsigned i = 0, n = file->numEnums(); i != n; ++i)
		writeEnumDefs(G,file->getEnum(i));
	for (unsigned i = 0, n = file->numMessages(); i != n; ++i) {
		Message *m = file->getMessage(i);
		if (m->isUsed())
			writeFunctions(G,m);
	}
	if (!ns.empty())
		G << "} /* namespace " << ns << " */\n";
}


void CppGenerator::writeFromMemory_early(Generator &G, Field *f)
{
	G.setField(f);
	uint32_t type = f->getTypeClass();
	uint32_t id = f->getId();
	if (!f->isUsed()) {
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr\n";
		writeSkipContent(G,f->getEncoding());
	}
	if (f->isPacked()) {
		G.setVariableHex("field_tag",(int64_t)id<<3|2);
		G <<	"case $(field_tag): {\t// $(fname) id $(field_id), packed $(typestr)[] coding 2\n"
			"varint_t v = ud.u64;\n"
			"const uint8_t *ae = a + v;\n"
			"do {\n";
		switch (type) {
		case ft_msg:
		case ft_bytes:
		case ft_string:
		case ft_cptr:
			abort();
			break;
		case ft_unsigned:
		case ft_int:
		case ft_enum:
		case ft_int16:
		case ft_uint16:
		case ft_int32:
		case ft_uint32:
		case ft_int64:
		case ft_uint64:
			decodeVarint(G,f);
			break;
		case ft_signed:
		case ft_sint16:
		case ft_sint32:
		case ft_sint64:
			decodeSVarint(G,f);
			break;
		case ft_bool:
		case ft_int8:
		case ft_uint8:
		case ft_sint8:
		case ft_fixed8:
		case ft_sfixed8:
			decode8bit(G,f);
			break;
		case ft_fixed16:
		case ft_sfixed16:
			decode16bit(G,f);
			break;
		case ft_fixed32:
		case ft_sfixed32:
		case ft_float:
			decode32bit(G,f);
			break;
		case ft_fixed64:
		case ft_sfixed64:
		case ft_double:
			decode64bit(G,f);
			break;
		}
		G <<	"} while (a < ae);\n"
			"break;\n"
			"}\n";
		uint32_t enc = f->getEncoding();
		if (enc == wt_lenpfx) {
			G.setField(0);
			return;
		}
		G.setVariableHex("field_tag",(int64_t)id<<3|enc);
	}
	int vbit = f->getValidBit();
	switch (type) {
	case ft_msg:
		G <<	"case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding byte[]\n"
			"{\n";
		if (f->getQuantifier() == q_repeated)
			G << "$(m_field).emplace_back();\n";
		G <<	"if (ud.vi != 0) {\n"
			"int n;\n";
		G.field_fill("(const uint8_t*)a,ud.vi");
		G <<	"if (n != (ssize_t)ud.vi)\n"
			"	$handle_error;\n"
			"a += ud.vi;\n"
			"}\n"
			"}\n";
		if (vbit != -1)
			writeSetValid(G,vbit);
		break;
	case ft_bytes:
	case ft_string:
		G <<	"case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding byte[]\n"
			"{\n"
			"if (ud.vi > 0) {\n";
		G.field_fill("(const char*)a,ud.vi");
		if (vbit != -1)
			writeSetValid(G,vbit);
		G <<	"a += ud.vi;\n"
			"}\n}\n";
		break;
	case ft_cptr:
		G <<	"case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding byte[]\n"
			"{\n"
			"if ((ud.vi > 0) && (a[ud.vi-1] == 0))\n";
		G.field_fill("(const char*)a");
		if (vbit != -1)
			writeSetValid(G,vbit);
		G <<	"a += ud.vi;\n"
			"}\n";
		break;
	case ft_unsigned:
	case ft_int:
	case ft_enum:
	case ft_int16:
	case ft_uint16:
	case ft_int32:
	case ft_uint32:
	case ft_int64:
	case ft_uint64:
		G.setVariableHex("field_tag",(int64_t)id<<3|wt_varint);
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding varint\n";
		G.field_fill("($(typestr))ud.u$varintbits");
		break;
	case ft_signed:
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding signed varint\n";
		G.field_fill("varint_sint(ud.u$intsize)");
		break;
	case ft_sint16:
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding signed varint\n";
		G.field_fill("varint_sint(ud.u16)");
		break;
	case ft_sint32:
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding signed varint\n";
		G.field_fill("varint_sint(ud.u32)");
		break;
	case ft_sint64:
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding signed varint\n";
		G.field_fill("varint_sint(ud.u64)");
		break;
	case ft_bool:
	case ft_int8:
	case ft_uint8:
	case ft_sint8:
	case ft_fixed8:
	case ft_sfixed8:
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding 8bit\n";
		G.field_fill("ud.u8");
		break;
	case ft_fixed16:
	case ft_sfixed16:
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding 16bit\n";
		G.field_fill("ud.u16");
		break;
	case ft_fixed32:
	case ft_sfixed32:
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding 32bit\n";
		G.field_fill("ud.u32");
		break;
	case ft_float:
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding 32bit\n";
		G.field_fill("ud.f");
		break;
	case ft_fixed64:
	case ft_sfixed64:
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding 64bit\n";
		G.field_fill("ud.u64");
		break;
	case ft_double:
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding 64bit\n";
		G.field_fill("ud.d");
		break;
	}
	G << "break;\n";
	G.setField(0);
}


void CppGenerator::writeFromMemory_early(Generator &G, Message *m)
{
	G <<	"ssize_t $(prefix)$(msg_name)::$(fromMemory)(const void *b, ssize_t s)\n"
		"{\n"
		"const uint8_t *a = (const uint8_t *)b;\n"
		"const uint8_t *e = a + s;\n";
	if (Debug)
		G << "std::cout << \"$(msg_fullname)::$(fromMemory)(\" << (void*)b << \", \" << s << \")\\n\";\n";
	G <<	"while (a < e) {\n"
		"varint_t fid;\n";
	const string &Terminator = target->getOption("Terminator");
	if ((Terminator == "ff") || (Terminator == "0xff"))
		G <<	"if (*a == 0xff)\t// 0xff terminator\n"
			"break;\n";
	G <<	"int fn = read_varint(a,e-a,&fid);\n"
		"a += fn;\n"
		"if (fn <= 0)\n"
		"	$handle_error;\n"
		"union {\n"
		"	varint_t vi;\n"
		"	uint64_t u64;\n"
		"	double d;\n"
		"	uint32_t u32;\n"
		"	float f;\n"
		"	uint16_t u16;\n"
		"	uint8_t u8;\n"
		"} ud;\n"
		"switch (fid&7) {\n"
		"case 0x0: // varint\n"
		"case 0x2: // varint of len pfx array\n"
		"	fn = read_varint(a,e-a,&ud.vi);\n"
		"	if (fn <= 0)\n"
		"		$handle_error;\n"
		"	a += fn;\n"
		"	break;\n"
		;
	if (VarIntBits == 64)
		G <<
		"case 0x1: // 64-bit\n"
		"	if (a+8 > e)\n"
		"		$handle_error;\n"
		"	ud.u64 = read_u64(a);\n"
		"	a += 8;\n"
		"	break;\n"
		;
	G <<	"case 0x3: // 8-bit\n"
		"	if (a+1 > e)\n"
		"		$handle_error;\n"
		"	ud.u64 = 0;\n"
		"	ud.u8 = *a++;\n"
		"	break;\n"
		"case 0x4: // 16-bit\n"
		"	if (a+2 > e)\n"
		"		$handle_error;\n"
		"	ud.u$varintbits = 0;\n"
		"	ud.u16 = read_u16(a);\n"
		"	a += 2;\n"
		"	break;\n"
		;
	if (VarIntBits >= 32)
		G <<
		"case 0x5: // 32-bit\n"
		"	if (a+4 > e)\n"
		"		$handle_error;\n"
		"	ud.u$varintbits = 0;\n"
		"	ud.u32 = read_u32(a);\n"
		"	a += 4;\n"
		"	break;\n"
		;
	G <<	"default:\n"
		"	$handle_error;\n"
		"}\n"
		"switch (fid) {\n"
		;

	bool hasNullId = false;
	for (unsigned i = 0, e = m->numFields(); i != e; ++i) {
		Field *f = m->getField(i);
		if (f == 0)
			continue;
		if (!f->isUsed()) {
			hasUnused = true;
			continue;
		}
		if (f->getId() == 0) {
			wiretype_t e = f->getEncoding();
			if ((e == wt_varint) || (e == wt_dynamic))
				hasNullId = true;
		}
		writeFromMemory_early(G,f);
	}
	if (target->getOption("UnknownField") == "assert") {
		// need to skip known but unused fields to avoid asserts
		for (unsigned i = 0, e = m->numFields(); i != e; ++i) {
			Field *f = m->getField(i);
			if ((f == 0) || (f->isUsed()))
				continue;
			G.setField(f);
			if (f->isPacked())
				G.setVariableHex("field_tag",(int64_t)f->getId()<<3|2);
			G <<	"case $(field_tag):\t// $(fname) id $(field_id), unused\n";
			if (optmode == optspeed)
				writeSkipContent(G,f->getEncoding());
			G.setField(0);
		}
	}
	if ((Terminator == "null") || (Terminator == "0")) {
		if (hasNullId) 
			error("request to handle null termination, but null-tag exists");
		G <<	"case 0:\t// terminate on null byte\n"
			"	break;\n";
	}
	G <<	"default:\n";
	if (target->getOption("UnknownField") == "assert") {
		G << "assert(0);	// unknown field (option unknwon=assert)\n";
	} else if (target->getOption("UnknownField") == "skip") {
		G <<	"if ((fid & 7) == 2) {\n"
			"	// need only to skip len prefixed data\n"
			"	a += ud.vi;\n"
			"	if (a > e)\n"
			"		$handle_error;\n"
			"}\n";
	} else 
		fatal("unable to handle option unknown with value %s",target->getOption("UnknownField").c_str());
	G <<	"}\n"
		"}\n";
	if (Asserts)
		G << "assert((a-(const uint8_t *)b) == s);\n";
	G <<	"if (a > e)\n"
		"	$handle_error;\n"
		"return a-(const uint8_t *)b;\n"
		"}\n"
		"\n";
}


