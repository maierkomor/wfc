/*
 *  Copyright (C) 2017-2021, Thomas Maier-Komor
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
, usesStringTypes(false)
, PaddedMsgSize(false)
, SinkToTemplate(false)
, WithComments(true)
, WithJson(false)
, EarlyDecode(false)
, inlineClear(true)
, inlineHas(true)
, inlineGet(true)
, inlineMaxSize(true)
, inlineSet(true)
, inlineSize(true)
, hasVarInt(false)
, hasVarSInt(false)
, hasInt(false)
, hasSInt(false)
, hasUInt(false)
, hasCStr(false)
, hasBool(false)
, hasFloat(false)
, hasFloats(false)
, hasDouble(false)
, hasDoubles(false)
, hasS8(false)
, hasS16(false)
, hasS32(false)
, hasS64(false)
, hasU8(false)
, hasU16(false)
, hasU32(false)
, hasU64(false)
, hasWT8(false)
, hasWT16(false)
, hasWT32(false)
, hasWT64(false)
, hasEnums(false)
, hasBytes(false)
, hasString(false)
, hasLenPfx(false)
, hasRBytes(false)
, hasRString(false)
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
		"/****************************************************************************\n";
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
		" ****************************************************************************/\n"
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
			error("IntSize (%ld) must be smaller or equal than VarIntBits (%u).",intsize,VarIntBits);
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
	if (optmode == optsize)
		EarlyDecode = true;
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
	PrintOut = (target->getIdentifier("toASCII") != 0);
	string toJSON = target->getOption("toJSON");
	WithJson = toIdentifier(toJSON);
	Asserts = target->getFlag("asserts");
	WithComments = target->getFlag("comments");
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
			KVPair *p = i->second;
			applyNodeOption(i->first.c_str(),p);
		}
		no = no->getParent();
	} while (no);
	const vector<string> &includes = target->getCodeLibs();
	for (size_t x = 0, n = includes.size(); x < n; ++x) 
		Lib.add(includes[x].c_str());
}


void CppGenerator::applyNodeOption(const char *nodepath, KVPair *kvp)
{
	diag("appyNodeOption(%s,[%s,%s,...])",nodepath,kvp->getKey().c_str(),kvp->getValue().c_str());
	if (nodepath[0] != '/')
		error("invalid node path '%s'",nodepath);
	const char *node = nodepath+1;
	Message *m = 0;	// needed for '/message' case
	do {
		char *slash = strchr((char*)node,'/');
		string msgname(node,slash ? slash-node : strlen(node));
		diag("appyNodeOption: msg %s",msgname.c_str());
		if (m == 0) {
			m = file->getMessage(msgname.c_str());
		} else if (Message *sm = m->getMessage(msgname.c_str())) {
			m = sm;
		} else {
			// this is a field or a enum
			break;
		}
		if (m == 0) {
			warn("Ignoring unknwon message '%s' in nodepath '%s'",msgname.c_str(),nodepath);
			return;
		}
		if (slash == 0)
			node = 0;
		else
			node = slash+1;
	} while (node);
	
	if (Enum *e = m ? m->getEnum(node) : file->getEnum(node)) {
		diag("appyNodeOption: enum %s",node);
		do {
			e->setOption(kvp->getKey(),kvp->getValue());
			kvp = kvp->getNext();
		} while (kvp);
	} else if (m == 0) {
		warn("Unable to resolve node path %s. Ignoring option '%s'.",nodepath,kvp->getKey().c_str());
	} else if (Field *f = m->getField(node)) {
		do {
			diag("appyNodeOption: field %s, key %s",node,kvp->getKey().c_str());
			f->setOption(kvp->getKey(),kvp->getValue());
			kvp = kvp->getNext();
		} while (kvp);
	} else {
		do {
			m->setOption(kvp->getKey().c_str(),kvp->getValue().c_str());
			kvp = kvp->getNext();
		} while (kvp);
	}
}


string CppGenerator::getValid(Field *f, bool invalid)
{
	string ret;
	if (mem_virtual == f->getStorage()) {
		if (invalid)
			ret = "!";
		ret += "$(field_has)";
		return ret;
	}
	uint32_t type = f->getType();
	if (type == ft_cptr) {
		if (invalid)
			ret += "m_$(fname) == 0";
		else
			ret += "m_$(fname) != 0";
		return ret;
	}
	int vbit = f->getValidBit();
	if (const char *invValue = f->getInvalidValue()) {
			assert(vbit < 0);
		if ((type == ft_string) && (!strcmp(invValue,"\"\""))) {
			ret += "!m_$(fname).empty()";
		} else {
			ret += "m_$(fname)";
			if (invalid)
				ret += " == ";
			else
				ret += " != ";
			ret += invValue;
		}
		return ret;
	}
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
		return "p_validbits &= ~(($(validtype))1U << $vbit);\n";
	else
		return "p_validbits[$($vbit/8)] &= ~(($(validtype))1U << $($vbit&7));\n";
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
	size_t vbit = 0;
	for (auto i : m->getFields()) {
		Field *f = i.second;
		if ((f == 0) || (!f->isUsed()) || f->isObsolete())
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


void CppGenerator::init()
{
	usesArrays = false;
	usesVectors = false;
	usesBytes = false;
	usesStringTypes = false;
	for (unsigned i = 0, n = file->numMessages(); i != n; ++i) {
		Message *m = file->getMessage(i);
		m->setOptions(target);
		initVBits(m);
		initNames(m,"");
	}
}


void CppGenerator::scanRequirements(Message *m)
{
	if (!m->getGenerate())
		return;
	if (optmode == optreview)
		hasVarInt = true;	// tags are then always generated as varint
	for (auto i : m->getFields()) {
		Field *f = i.second;
		if (f == 0)
			continue;
		if (!f->isUsed()) {
			hasUnused = true;
			continue;
		}
		int id = f->getId();
		if ((id == 0) && !target->getFlag("id0"))
			error("Use of id 0 requires special considerations. Enable support for it with option id0.");
		else if ((id << 3) >= 128)
			hasVarInt = true;
		uint32_t tc = f->getTypeClass();
		if ((tc == ft_enum) && WithJson) {
			Enum *e = Enum::id2enum(f->getType());
			const string &strfun = e->getStringFunction();
			if (strfun.empty())
				e->setStringFunction();
		}
		bool r = f->isRepeated();
		if (r)
			hasVarInt = true;
		switch (tc) {
		case ft_bytes:
			hasVarInt = true;
			hasLenPfx = true;
			hasBytes = true;
			if (r)
				hasRBytes = true;
			break;
		case ft_string:
			hasVarInt = true;
			hasString = true;
			if (r)
				hasRString = true;
			hasLenPfx = true;
			break;
		case ft_cptr:
			hasCStr = true;
			break;
		case ft_msg:
			hasLenPfx = true;
			hasVarInt = true;
			continue;
		case ft_enum:
			hasVarInt = true;
			hasEnums = true;
			break;
		case ft_bool:
			hasBool = true;
			hasVarInt = true;
			break;
		// uintxx
		case ft_unsigned:
			hasVarInt = true;
			hasUInt = true;
			break;
		case ft_uint8:
			hasU8 = true;
			hasVarInt = true;
			break;
		case ft_uint16:
			hasU16 = true;
			hasVarInt = true;
			break;
		case ft_uint32:
			hasU32 = true;
			hasVarInt = true;
			break;
		case ft_uint64:
			hasU64 = true;
			hasVarInt = true;
			break;
		// intxx
		case ft_int:
			hasVarInt = true;
			hasInt = true;
			break;
		case ft_int8:
			hasS8 = true;
			hasInt = true;
			hasVarInt = true;
			break;
		case ft_int16:
			hasS16 = true;
			hasInt = true;
			hasVarInt = true;
			break;
		case ft_int32:
			hasS32 = true;
			hasInt = true;
			hasVarInt = true;
			break;
		case ft_int64:
			hasS64 = true;
			hasInt = true;
			hasVarInt = true;
			break;
		// sintxx
		case ft_signed:
			hasVarSInt = true;
			hasSInt = true;
			break;
		case ft_sint8:
			hasS8 = true;
			hasVarSInt = true;
			break;
		case ft_sint16:
			hasS16 = true;
			hasVarSInt = true;
			break;
		case ft_sint32:
			hasS32 = true;
			hasVarSInt = true;
			break;
		case ft_sint64:
			hasS64 = true;
			hasVarSInt = true;
			break;
		// float/double
		case ft_float:
			hasFloat = true;
			if (f->isRepeated())
				hasFloats = true;
			break;
		case ft_double:
			hasDouble = true;
			if (f->isRepeated())
				hasDoubles = true;
			break;
		// fixed
		case ft_fixed8:
			hasU8 = true;
			hasWT8 = true;
			break;
		case ft_fixed16:
			hasU16 = true;
			hasWT16 = true;
			break;
		case ft_fixed32:
			hasU32 = true;
			hasWT32 = true;
			break;
		case ft_fixed64:
			hasU64 = true;
			hasWT64 = true;
			break;
		// sfixed
		case ft_sfixed8:
			hasS8 = true;
			hasWT8 = true;
			break;
		case ft_sfixed16:
			hasS16 = true;
			hasWT16 = true;
			break;
		case ft_sfixed32:
			hasS32 = true;
			hasWT32 = true;
			break;
		case ft_sfixed64:
			hasS64 = true;
			hasWT64 = true;
			break;
		default:
			abort();
		}

		if ((Endian == little_endian) && (f->isPacked()) && ((tc == ft_fixed8) || (tc == ft_fixed16) || (tc == ft_fixed32) || (tc == ft_fixed64) || (tc == ft_float) || (tc == ft_double)))
			hasLenPfx = true;
	}
	for (unsigned i = 0, e = m->numMessages(); i != e; ++i) {
		Message *sm = m->getMessage(i);
		hasVarInt = true;
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
	funcs.push_back(ct_decode_union);
	funcs.push_back(ct_decode_early);
	funcs.push_back(ct_read_varint);
	funcs.push_back(ct_read_u64);
	funcs.push_back(ct_read_u32);
	funcs.push_back(ct_read_u16);
	funcs.push_back(ct_read_double);
	funcs.push_back(ct_read_float);
	funcs.push_back(ct_skip_content);

	funcs.push_back(ct_ascii_indent);
	funcs.push_back(ct_ascii_bytes);
	funcs.push_back(ct_ascii_string);
	funcs.push_back(ct_ascii_numeric);

	funcs.push_back(ct_json_string);
	funcs.push_back(ct_json_indent);
	funcs.push_back(ct_json_cstr);
	funcs.push_back(ct_to_decstr);
	funcs.push_back(ct_to_dblstr);

	funcs.push_back(ct_parse_ascii_bool);
	funcs.push_back(ct_parse_ascii_bytes);
	funcs.push_back(ct_parse_ascii_dbl);
	funcs.push_back(ct_parse_ascii_flt);
	funcs.push_back(ct_parse_ascii_s8);
	funcs.push_back(ct_parse_ascii_s16);
	funcs.push_back(ct_parse_ascii_s32);
	funcs.push_back(ct_parse_ascii_s64);
	funcs.push_back(ct_parse_ascii_u8);
	funcs.push_back(ct_parse_ascii_u16);
	funcs.push_back(ct_parse_ascii_u32);
	funcs.push_back(ct_parse_ascii_u64);

	funcs.push_back(ct_decode_bytes);
	funcs.push_back(ct_decode_bytes_element);
	funcs.push_back(ct_encode_bytes);

	// gen wire section
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

	// gen string section
	funcs.push_back(gen_string);
	funcs.push_back(ct_send_u16);
	funcs.push_back(ct_send_u32);
	funcs.push_back(ct_send_u64);
	funcs.push_back(ct_send_varint);
	if (!VarIntBits64)
		funcs.push_back(ct_send_xvarint);
	funcs.push_back(ct_send_msg);

	// gen sink section
	// not generated, as sinks are user-defined
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
	lhg << "#ifndef _" << defname << "_H\n"
		"#define _" << defname << "_H\n\n\n";
	if (Asserts || (target->getOption("UnknownField") == "assert"))
		lhg << "#include <assert.h>\n";
	if (target->isId("toJSON") && (target->getOption("streamtype") == "std::ostream"))
		lhg << "#include <ostream>\n";
	else if (PrintOut)
		lhg <<	"#define OUTPUT_TO_ASCII 1\n"
			"#include <iosfwd>\n";
	if (target->StringSerialization() || usesStringTypes || PrintOut || WithJson)
		lhg <<	"#include <string>\n";
	else if (WithComments)
		lhg << "/* std::string support is not needed */\n";
	if (usesVectors)
		lhg << "#include <vector>\n";
	else if (WithComments)
		lhg << "/* std::vector support not needed */\n";
	if (usesArrays)
		lhg << "#include <array.h>\n";
	else if (WithComments)
		lhg << "/* array support not needed */\n";
	lhg <<	"#include <stddef.h>\n"
		"#include <stdint.h>\n"
		"#include <stdlib.h>\n";
	if (target->getOption("stringtype") == "std")
		lhg << "#include <string>\n";
	if (!target->getHeaders().empty()) {
		if (WithComments)
			lhg <<	"/* user requested header files */\n";
		for (auto &h : target->getHeaders()) 
			lhg <<	"#include " << h << '\n';
	}
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
	if ((target->getOption("stringtype")== "C") && !target->getOption("toString").empty())
		fatal("cannot generate method toString with stringtype C");
	for (unsigned i = 0, n = file->numMessages(); i != n; ++i)
		scanRequirements(file->getMessage(i));
	if (PrintOut) {
		if (hasBytes && (0 == target->getIdentifier("ascii_bytes")))
			error("need valid ascii_bytes option for toASCII");
	}
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
	for (unsigned i = 0, n = src.length(); i != n; ++i) {
		char c = ::toupper(src.at(i));
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
	if (!f->isUsed() || f->isObsolete()) {
		if (!f->isUsed())
			hasUnused = true;
		if (WithComments)
			G << "// omitted" << f->getUsage() << " member $(fname)\n";
		G.setField(0);
		return first;
	}
	mem_inst_t s = f->getStorage();
	if (s == mem_virtual) {
		if (WithComments) {
			if (def_not_init)
				G << "// no definition for virtual member $(fname)\n";
			else
				G << "// no initialization for virtual member $(fname)\n";
		}
		G.setField(0);
		return first;
	}
	if (s == mem_static) {
		if (!def_not_init) {
			if (WithComments)
				G << "// no initialization for static member $(fname)\n";
			G.setField(0);
			return first;
		}
		G << "static ";
	}
	if (def_not_init) {
		if (WithComments)
			G << "//! " << f->getWfcType() << " $(fname), id $(field_id)\n";
		switch (f->getQuantifier()) {
		case q_optional:
		case q_required:
			G << "$(typestr) m_";
			break;
		case q_repeated:
			G << f->getRepeatedType();
			G << " m_";
			break;
		default:
			abort();
		}
		G << "$(fname);\n";
	} else {
		if (first)
			G << ':';
		else
			G << ',';
		G << " m_$(fname)(";
		if (f->getQuantifier() == q_repeated)
			G << ")\n";
		else if (const char *defv = f->getDefaultValue()) 
			G << defv << ")\n";
		else if (const char *inv = f->getInvalidValue())
			G << inv << ")\n";
		else if (f->getEncoding() == wt_lenpfx)
			G << ")\n";
		else 
			G << "0)\n";
	}
	G.setField(0);
	return false;
}


void CppGenerator::writeStaticMember(Generator &G, Field *f, const char *mname)
{
	assert(f && f->isStatic());
	G.setField(f);
	if (!f->isUsed() || f->isObsolete()) {
		if (WithComments)
			G << "// omitted" << f->getUsage() << " member $(fname)\n";
		G.setField(0);
		return;
	}
	switch (f->getQuantifier()) {
	case q_optional:
	case q_required:
		G << "$(typestr) " << mname << "::m_";
		break;
	case q_repeated:
		if (unsigned s = f->getArraySize())
			G << "array<$(fulltype)," << s << "> m_";
		else
			G << "std::vector<$fulltype> m_";
		break;
	default:
		abort();
	}
	G << "$(fname)";
	if (const char *defv = f->getDefaultValue()) 
		G << " = " << defv;
	else if (const char *inv = f->getInvalidValue()) 
		G << " = " << inv;

	if (WithComments)
		G << ";\t// " << f->getWfcType() << " $(fname), id $(field_id)\n";
	else
		G << ";\n";
	G.setField(0);
}


static void writeFieldComment(Generator &G, Field *f, const char *x = "\n")
{
	G << "//" << f->getUsage();
	switch (f->getQuantifier()) {
	case q_optional:
		G << " optional ";
		break;
	case q_required:
		G << " required ";
		break;
	case q_repeated:
		G << " repeated ";
		break;
	default:
		abort();
	}
	G << f->getWfcType() << ' ' << f->getName() << ", id " << f->getId();
	G << x;
}


static void writeSetDecl(Generator &G, uint32_t type, uint8_t q, bool v, bool comments)
{
	if ((q_repeated != q) && ((type == ft_bytes) || (type == ft_string) || ((type & ft_filter) == ft_msg))) {
		if (comments) {
			if (type == ft_msg) {
				G <<	"/*!\n"
					" * Function for setting members of $fname using data from a serialized object.\n";
			} else {
				G <<	"/*!\n"
					" * Function for setting $fname using binary data.\n";
			}
			G <<	" * @param data pointer to binary data\n"
				" * @param s number of bytes at data pointer\n"
				" */\n";
		}
		G <<	"$(virtual)void $(field_set)(const void *data, size_t s)$(vimpl);\n";
	}
	if (!v || (q_repeated != q)) {
		if (comments)
			G << "//! Set $fname using a constant reference\n";
		G << "$(virtual)void $(field_set)";
		if (q_repeated == q)
			G << "(unsigned x, $(fullrtype)v)$(vimpl);\n";
		else
			G << "($(rtype)v)$(vimpl);\n";
		if ((type == ft_string) && (q != q_repeated)) {
			if (comments)
				G << "//! Set $fname using a pointer to a null-terminated C-string.\n";
			G << "$(virtual)void $(field_set)(const char *);\n";
		}
	}
}


static void writeProtDecl(Generator &G, Field *f, bool comments)
{
	if (!f->isDeprecated())
		return;
	G.setField(f);
	bool v = f->isVirtual();
	if (v) {
		G.addVariable("virtual","virtual ");
		G.addVariable("vimpl", " = 0");
	} else {
		G.addVariable("virtual","");
		G.addVariable("vimpl", "");
	}
	if (comments)
		G << "//" << f->getUsage() << ' ' << f->getWfcType() << " $(fname), id $(field_id)\n";
	writeSetDecl(G,f->getType(),f->getQuantifier(),f->isVirtual(),comments);
	G.clearVariable("virtual");
	G.clearVariable("vimpl");
	G.setField(0);
}


void CppGenerator::writeHeaderDecls(Generator &G, Field *f)
{
	if (f == 0)
		return;
	G.setField(f);
	if (WithComments)
		writeFieldComment(G,f);
	if (!f->isUsed() || f->isObsolete()) {
		G.setField(0);
		return;
	}
	bool v = f->isVirtual();
	if (v) {
		G.addVariable("virtual","virtual ");
		G.addVariable("vimpl", " = 0");
	} else {
		G.addVariable("virtual","");
		G.addVariable("vimpl", "");
	}
	if (f->isDeprecated() && target->getFlag("gnux"))
		G.addVariable("gnux"," __attribute__ ((deprecated))");
	else
		G.addVariable("gnux","");
	bool simpletype = f->hasSimpleType();
	uint32_t type = f->getType();
	uint8_t q = f->getQuantifier();
	if (q == q_optional) {
		if (WithComments)
			G <<	"/*!\n"
				" * Function for querying if $fname has been set.\n"
				" * @return true if $fname is set.\n"
				" */\n";
		G <<	"$(virtual)bool $(field_has)() const$(vimpl);\n";
	}
	if (q_repeated == q) {
		if (WithComments)
			G <<	"//! Function get const-access to the elements of $fname.\n";
		G <<	"const " << f->getRepeatedType() << " &$(field_get)() const$(gnux);\n";
		if (WithComments)
			G <<	"//! Function to get the number of elements in $fname.\n";
		G <<	"$(virtual)size_t $(fname)_size() const$(vimpl)$(gnux);\n";
		if (WithComments)
			G <<	"/*!\n"
				"* Function to append a element to $fname.\n"
				"* @return point to newly added element.\n"
				"*/\n";
		if (f->hasMessageType())
			G << "$(virtual)$(typestr)* $(field_add)()$(vimpl)$(gnux);\n";
		else if (simpletype)
			G << "$(virtual)void $(field_add)($(typestr) v)$(vimpl)$(gnux);\n";
		else
			G << "$(virtual)void $(field_add)(const $(typestr) &v)$(vimpl)$(gnux);\n";
		if (v && ((type == ft_string) || (type == ft_bytes) || ((type & ft_filter) == ft_msg))) {
			if (WithComments)
				G <<	"//! Function to append an element to $fname initialized by a buffer.\n";
			G <<	"virtual void $(field_add)(const void *, size_t) = 0;\n";
		}
		if (type == ft_string) {
			if (WithComments)
				G <<	"//! Function to append an element to $fname initialized by a C-string.\n";
			G <<	"$(virtual)void $(field_add)(const char*);\n";
		}
	}
	if ((q != q_required) || v) {
		if (WithComments)
			G <<	"//! Function to reset $fname to its default/unset value.\n";
		G << "$(virtual)void $(field_clear)()$(vimpl);\n";
	}
	if ((q == q_repeated) && v) {
		if (WithComments)
			G <<	"//! Function to access all elements of $fname.\n";
		G << "virtual const std::vector<$(typestr)> &$(field_get)() const = 0$(gnux);\n";
	}

	if (q_repeated == q) {
		G.addVariable("index","unsigned x");
		G.addVariable("idxvar","unsigned x, ");
		if (WithComments)
			G <<	"//! Get value of element x of $fname.\n";
	} else {
		G.addVariable("index","");
		G.addVariable("idxvar","");
		if (WithComments)
			G <<	"//! Get value of $fname.\n";
	}
	G << "$(virtual)$(fullrtype)$(field_get)($index) const$(vimpl)$(gnux);\n";
	G.clearVariable("index");
	G.clearVariable("idxvar");
	if (!f->isDeprecated())
		writeSetDecl(G,type,q,v,WithComments);
	if (!v && !f->isDeprecated()) {
		if (WithComments)
			G <<	"/*!\n"
				"* Provide mutable access to $fname.\n";
		if ((target->getOption("MutableType") == "pointer") || (target->getOption("MutableType") == "*")) {
			if (WithComments)
				G <<	"* @return pointer to member variable of $fname.\n"
					"*/\n";
			G << "$(virtual)$(ptype)$(field_mutable)";
		} else if ((target->getOption("MutableType") == "reference") || (target->getOption("MutableType") == "&")) {
			if (WithComments)
				G <<	"* @return reference to member variable of $fname.\n"
					"*/\n";
			G << "$(virtual)$(typestr) &$(field_mutable)";
		} else
			abort();
		if (q_repeated == q)
			G << "(unsigned x)$(vimpl);\n";
		else
			G << "()$(vimpl);\n";
		if (q_repeated == q) {
			if (WithComments)
				G << "//! Function to get mutable access to all elements of $fname.\n";
			G << "$(virtual)";
			if (unsigned s = f->getArraySize())
				G << "array<$(typestr)," << s << "> ";
			else
				G << "std::vector<$(typestr)> ";
			if ((target->getOption("MutableType") == "pointer") || (target->getOption("MutableType") == "*"))
				G << '*';
			else if ((target->getOption("MutableType") == "reference") || (target->getOption("MutableType") == "&"))
				G << '&';
			else
				abort();
			G << "$(field_mutable)()$(vimpl);\n";
		}
	} else if (v) {
		if (WithComments)
			G << "//! Function to check if $fname has no elements.\n";
		G << "virtual bool $(fname)_empty() const = 0;\n";
	}
	if (v && PrintOut && ((type & ft_filter) == ft_msg))
		G << "virtual void $(fname)_$(toASCII)($(streamtype) &o, size_t indent = 0) const = 0;\n";
	G.setField(0);
	G << '\n';
	G.clearVariable("virtual");
	G.clearVariable("vimpl");
	G.clearVariable("gnux");
}


void CppGenerator::writeEnumDecl(Generator &G, Enum *e, bool inclass)
{
	G.setEnum(e);
	G	<< "typedef enum {\n";
	const multimap<int64_t,string> &vn = e->getValueNamePairs();
	for (multimap<int64_t,string>::const_iterator i(vn.begin()),j(vn.end()); i != j; ++i) {
		diag("enum-value %s %ld",i->second.c_str(),i->first);
		G << i->second << " = " << i->first << ",\n";
	}
	G	<< "} $(ename);\n";
	const string &strfun = e->getStringFunction();
	if (!strfun.empty()) {
		if (WithComments)
			G << "//! Function to get an ASCII string from a value of a $ename.\n";
		if (inclass)
			G << "static ";
		G << "const char *$(strfun)($(ename) e);\n";
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
		"return 0;\n";
	const multimap<int64_t,string> &vn = en->getValueNamePairs();
	auto i(vn.begin()), e(vn.end()), x(i);
	while (i != e) {
		G << "case " << i->second << ":\n";
		x = i;
		++x;
		while ((x != e) && (x->first == i->first)) {
			if (WithComments)
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


struct sort_field_size
{
	bool operator () (const Field *l, const Field *r) const
	{
		return l->getMemberSize() > r->getMemberSize();
	}
};


void CppGenerator::writeMembers(Generator &G, Message *m, bool def_not_init)
{
	msg_sorting_t sorting = m->getSorting();
	if (sorting == sort_unset)
		sorting = sort_id;
	bool first = true;
	const std::map<unsigned,Field *> &fields = m->getFields();
	if (sorting == sort_none) {
		const vector<unsigned> &seq = m->getFieldSeq();
		for (auto i = seq.begin(), e = seq.end(); i != e; ++i) {
			Field *f = m->getFieldId(*i);
			assert(f);
			first = writeMember(G,f,def_not_init,first);
		}
	} else if (sorting == sort_id) {
		for (auto i : fields) {
			if (Field *f = i.second)
				first = writeMember(G,f,def_not_init,first);
		}
	} else if (sorting == sort_size) {
		vector<Field *> sorted;
		for (auto i : fields)
			sorted.push_back(i.second);
		sort(sorted.begin(),sorted.end(),sort_field_size());
		for (auto i : sorted)
			first = writeMember(G,i,def_not_init,first);
	} else if (sorting == sort_type) {
		set<string> tns;
		for (auto i : fields) {
			if (Field *f = i.second)
				tns.insert(f->getTypeName());
		}
		unsigned cnt = 0;
		for (auto ti = tns.begin(), te = tns.end(); ti != te; ++ti) {
			const string &tn = *ti;
			for (auto i : fields) {
				Field *f = i.second; 
				if ((f == 0) || (tn != f->getTypeName()))
					continue;
				first = writeMember(G,f,def_not_init,first);
				++cnt;
			}
		}
		assert(cnt == fields.size());
	} else if (sorting == sort_name) {
		map<string,Field*> names;
		for (auto i : fields) {
			if (Field *f = i.second)
				names.insert(make_pair(f->getName(),f));
		}
		unsigned cnt = 0;
		for (auto ni = names.begin(), ne = names.end(); ni != ne; ++ni) {
			first = writeMember(G,ni->second,def_not_init,first);
			++cnt;
		}
		assert(cnt == fields.size());
	} else
		abort();
}


void CppGenerator::writeStaticMembers(Generator &G, Message *m)
{
	bool had_output = false;
	const std::map<unsigned,Field *> &fields = m->getFields();
	const char *name = m->getFullname().c_str();
	for (auto i : fields) {
		Field *f = i.second; 
		if (f && f->isStatic()) {
			writeStaticMember(G,f,name);
			had_output = true;
		}
	}
	if (had_output)
		G << '\n';
}


void CppGenerator::writeClass(Generator &G, Message *m)
{
	if (!m->getGenerate()) {
		G.setMessage(m);
		if (m->isUsed())
			error("message %s is subpressed for generation, but is used",m->getName().c_str());
		if (WithComments)
			G << "// message $msg_name is not used\n";
		G.setMessage(0);
		return;
	}
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
		"$(msg_name)();\n\n";
	if (target->getFlag("withUnequal"))
		G << "bool operator != (const $(msg_name) &r) const;\n";
	if (target->getFlag("withEqual"))
		G << "bool operator == (const $(msg_name) &r) const;\n";
	G <<	"\n";
	if (WithComments)
		G << "//! Function for resetting all members to their default values.\n";
	G <<	"void $(msg_clear)();\n"
		"\n";
	if (WithComments)
		G <<	"/*!\n"
			" * Calculates the required number of bytes for serializing this object.\n"
			" * If member variables of the object are modified, the number of bytes\n"
			" * needed for serialization may change, too.\n"
			" * @return bytes needed for a serialized object representation\n"
			" */\n";
	G <<	"size_t $calcSize() const;\n\n";
	if (G.hasValue("fromMemory")) {
		if (WithComments)
			G <<	"/*!\n"
				" * Function for parsing serialized data and update this object accordingly.\n"
				" * Member variables that are not in the serialized data are not reset.\n"
				" * @param b buffer of serialized data\n"
				" * @param s number of bytes available in the buffer\n"
				" * @return number of bytes successfully parsed (can be < s)\n"
				" *         or a negative value indicating the error encountered\n"
				" */\n";
		G <<	"ssize_t $(fromMemory)(const void *b, ssize_t s);\n\n";
	}
	if (G.hasValue("toMemory")) {
		if (WithComments)
			G <<	"/*!\n"
				" * Function for serializing the object to memory.\n"
				" * @param b buffer to serialize the object to\n"
				" * @param s number of bytes available in the buffer\n"
				" * @return number of bytes successfully serialized\n"
				" */\n";
		G <<	"ssize_t $(toMemory)(uint8_t *, ssize_t) const;\n\n";
	}
	if (G.hasValue("toWire")) {
		G.setMode(gen_wire);
		if (WithComments)
			G <<	"/*!\n"
				"* Serialize the object using a function for transmitting individual bytes.\n"
				"* @param put function to put individual bytes for transmission on the wire\n"
				"*/\n";
		G <<	"void $(toWire)($putparam) const;\n\n";
	}
	if (G.hasValue("toSink")) {
		G.setMode(gen_sink);
		if (WithComments)
			G <<	"//! Function for serializing the object using the Sink class.\n";
		G <<	"$(sink_template)void $(toSink)($putparam) const;\n\n";
	}
	if (G.hasValue("toString")) {
		G.setMode(gen_string);
		if (WithComments)
			G <<	"//! Function for serializing the object to a string.\n";
		G <<	"void $(toString)($putparam) const;\n\n";
	}
	if (G.hasValue("toJSON")) {
		if (WithComments)
			G <<	"/*!\n"
				" * Function for writing a JSON representation of this object to a stream.\n"
				" * @param json stream object the JSON output shall be written to\n"
				" * @indLvl current indention level\n"
				" */\n";
		G <<	"void $(toJSON)($(streamtype) &json, unsigned indLvl = 0) const;\n\n";
	}
	if (PrintOut) {
		if (WithComments)
			G <<	"/*!\n"
				" * Function for writing an ASCII representation of this object to a stream.\n"
				" * @param o output stream\n"
				" * @param indent initial indention level\n"
				" */\n";
		G <<	"void $(toASCII)($(streamtype) &o, size_t indent = 0) const;\n\n";
	}
	if (G.hasValue("getMaxSize")) {
		if (WithComments)
			G <<	"/*!\n"
				" * Function for determining the maximum size that the object may need for\n"
				" * its serialized representation\n"
				" * @return maximum number of bytes or SIZE_MAX if no limit can be determined\n"
				"*/\n";
		G <<	"static size_t $getMaxSize();\n\n";
	}
	if (target->getOption("SetByName") != "") {
		if (WithComments)
			G <<	"//! Function for setting a parameter by its ASCII name using an ASCII representation of value.\n"
				"//! @param param parameter name\n"
				"//! @param value ASCII representation of the value\n"
				"//! @return number of bytes parsed from value or negative value if an error occurs\n";
		G <<	"int $(set_by_name)(const char *param, const char *value);\n\n";
	}
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
	for (auto i : m->getFields())
		writeHeaderDecls(G,i.second);
	G <<	"\n"
		"protected:\n";
	for (auto i : m->getFields())
		writeProtDecl(G,i.second,WithComments);
	writeMembers(G,m,true);
	unsigned numValid = m->getNumValid();
	if (numValid > 0) {
		G << "\nprivate:\n";
		if (numValid > VarIntBits) {
			G	<< "uint8_t p_validbits[$numvalidbytes];\n";
		} else if (numValid <= 8) {
			G	<< "uint8_t p_validbits;\n";
		} else if (numValid <= 16) {
			G	<< "uint16_t p_validbits;\n";
		} else if (numValid <= 32) {
			G	<< "uint32_t p_validbits;\n";
		} else if (numValid <= 64) {
			G	<< "uint64_t p_validbits;\n";
		} else {
			abort();
		}
	}
	G <<	"};\n\n\n";
	G.setMessage(0);
}


void CppGenerator::writeHas(Generator &G, Field *f)
{
	if (f->getQuantifier() != q_optional)
		return;
	G <<	"$(inline)bool $(prefix)$(msg_name)::$(field_has)() const\n"
		"{\n"
		"return " << getValid(f) << ";\n"
		"}\n\n";
}


void CppGenerator::writeSize(Generator &G, Field *f)
{
	if (f->getQuantifier() == q_repeated) {
		G <<	"$(inline)size_t $(prefix)$(msg_name)::$(fname)_size() const\n"
			"{\n"
			"return $(field_size);\n"
			"}\n\n";
	}
}


void CppGenerator::writeFunctions(Generator &G, Field *f)
{
	if (f->isVirtual() || f->isObsolete())
		return;
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
	if (f->isVirtual() || f->isObsolete())
		return;
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
	for (auto i : m->getFields()) {
		Field *f = i.second;
		if ((f != 0) && f->isUsed() && !f->isObsolete())
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
	if (!out)
		fatal("unable to create output file %s",fn.c_str());
	string ucfn;
	mangle_filename(ucfn,bn);
	writeInfos(out);
	out <<	"#ifndef " << ucfn << "_H\n"
		"#define " << ucfn << "_H\n\n";
	if (Asserts || (target->getOption("UnknownField") == "assert"))
		out << "#include <assert.h>\n";
	if (PrintOut) {
		out <<	"#define OUTPUT_TO_ASCII 1\n";
		if (target->getOption("streamtype") == "std::ostream") {
			out << "#include <iosfwd>\n";
		}
	}
	if (target->isId("toJSON") && (target->getOption("streamtype") == "std::ostream"))
		out << "#include <ostream>\n";
	if (target->StringSerialization() || usesStringTypes || PrintOut || WithJson)
		out <<	"#include <string>\n";
	else if (WithComments)
		out << "/* std::string support is not needed */\n";
	if (usesVectors)
		out << "#include <vector>\n";
	else if (WithComments)
		out << "/* std::vector support not needed */\n";
	if (usesArrays)
		out << "#include <array.h>\n";
	else if (WithComments)
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
		if (WithComments)
			out << "/* user requested header files */\n";
		for (auto i = headers.begin(), e = headers.end(); i != e; ++i)
			out << "#include " << *i << "\n";
	}
	const vector<string> &decls = target->getDeclarations();
	if (!decls.empty()) {
		if (WithComments)
			out << "/* user requested declarations */\n";
		for (auto i = headers.begin(), e = headers.end(); i != e; ++i)
			out << *i << "\n";
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
	if (usesBytes)
		G << "#include <bytes.h>\n";
	else if (WithComments)
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
			"\n"
			"	//! calculate the number ob bytes needed for a serialized representation\n"
			"	virtual size_t $calcSize() const = 0;\n";
		if (G.hasValue("toString"))
			G << 	"\n"
				"\t//! serialize the object to a string\n"
				"\tvirtual void $toString($(stringtype) &) const = 0;\n";
		if (G.hasValue("toSink") && !SinkToTemplate)
			G << 	"\n"
				"\t//! serialize the object to an object derived from class Sink\n"
				"\tvirtual void $toSink(Sink &) const = 0;\n";
		if (G.hasValue("toMemory")) {
			if (WithComments)
				G <<	"/*!\n"
					" * Function for serializing the object to memory.\n"
					" * @param b buffer to serialize the object to\n"
					" * @param s number of bytes available in the buffer\n"
					" * @return number of bytes successfully serialized\n"
					" */\n";
			G << 	"virtual ssize_t $toMemory(uint8_t *, ssize_t) const = 0;\n";
		}
		G <<	"};\n\n";
	}
	for (unsigned i = 0, n = file->numEnums(); i != n; ++i) {
		Enum *e = file->getEnum(i);
		writeEnumDecl(G,e);
	}
	for (unsigned i = 0, n = file->numMessages(); i != n; ++i) {
		Message *m = file->getMessage(i);
		if (m->isUsed() || m->getGenerate())
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
			if (m->isUsed() || m->getGenerate())
				writeInlines(G,m);
		}
	}
	if (!ns.empty())
		G << "} /* namespace " << ns << "*/\n\n";

	G << "#endif\n";
}


void CppGenerator::writeResetUnset(Generator &G, Field *f)
{
	int v = f->getValidBit();
	assert(v != -3);	// should be initialized now
	if ((v == -1) || (v == -2))
		return;
	G << "if (";
	writeGetValid(G,f,true);
	G << ") {\n";
	writeSetValid(G,v);
	const char *def = f->getDefaultValue();
	if (f->hasSimpleType()) {
		G <<	"m_$(fname) = " << (def?def:"0") << ";\n";
	} else if (def) {
		G <<	"m_$(fname) = " << def << ";\n";
	} else {
		G << 	"m_$(fname).clear();\n";
	}
	G <<	"}\n";
}


void CppGenerator::writeGet(Generator &G, Field *f)
{
	uint8_t q = f->getQuantifier();
	if (q != q_repeated) {
		G <<	"$(inline)$(fullrtype)$(prefix)$(msg_name)::$(field_get)() const\n"
			"{\n"
			"return m_$(fname);\n"
			"}\n\n";
	} else {
		G <<	"$(inline)$(fullrtype)$(prefix)$(msg_name)::$(field_get)(unsigned x) const\n"
			"{\n"
			"return m_$(fname)[x];\n"
			"}\n\n"
			"$(inline)const " << f->getRepeatedType(true) << " &$(prefix)$(msg_name)::$(field_get)() const\n"
			"{\n"
			"return m_$(fname);\n"
			"}\n\n";
	}
}


void CppGenerator::writeMutable(Generator &G, Field *f)
{
	if (f->isDeprecated())
		return;
	uint32_t t = f->getType();
	uint8_t q = f->getQuantifier();
	bool mut_ref = target->getOption("MutableType") == "reference" || target->getOption("MutableType") == "&";
	G.addVariable("index","");
	if (mut_ref) {
		G.addVariable("T","&");
		G.addVariable("R","");
	} else {
		G.addVariable("T","*");
		G.addVariable("R","&");
	}
	if (q == q_optional) {
		G <<	"$(inline)$(fulltype) $(T)$(prefix)$(msg_name)::$(field_mutable)()\n"
			"{\n";
		writeResetUnset(G,f);
		G <<	"return $(R)m_$(fname);\n"
			"}\n\n"
			;
	} else if (q == q_required) {
		G <<	"$(inline)$(fulltype) $(T)$(prefix)$(msg_name)::$(field_mutable)()\n"
			"{\n"
			"return $(R)m_$(fname);\n"
			"}\n\n"
			;
	} else if ((q == q_repeated) && (t != ft_bool)) {
		G.setVariable("index","[x]");
		G <<	"$(inline)$(fulltype) $(T)$(prefix)$(msg_name)::$(field_mutable)(unsigned x)\n"
			"{\n"
			"if (x >= $(field_size))\n";
		if (const char *def = f->getDefaultValue())
			G <<	"m_$(fname).resize(x+1," << def << ");\n";
		else
			G <<	"m_$(fname).resize(x+1);\n";
		G <<	"return $(R)m_$(fname)$(index);\n"
			"}\n\n"
			;
	}
	G.clearVariable("T");
	G.clearVariable("R");
	G.clearVariable("index");
	if (q == q_repeated) {
		if (unsigned s = f->getArraySize())
			G << "$(inline)array<$(fulltype)," << s << "> ";
		else
			G << "$(inline)std::vector<$(fulltype)> ";
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
	if (q_repeated != q) {
		if ((t == ft_bytes) || (t == ft_string)) {
			G <<	"$(inline)void $(prefix)$(msg_name)::$(field_set)(const void *data, size_t s)\n"
				"{\n"
				"m_$(fname).assign((const char *)data,s);\n"
			<<	setvalid
			<<	"}\n"
				"\n";
		} else if ((t & ft_filter) == ft_msg) {
			G <<	"$(inline)void $(prefix)$(msg_name)::$(field_set)(const void *data, size_t s)\n"
				"{\n"
				"m_$(fname).$(fromMemory)((const char *)data,s);\n"
			<<	setvalid
			<<	"}\n"
				"\n";
		}
		if (t == ft_string) {
			G <<	"$(inline)void $(prefix)$(msg_name)::$(field_set)(const char *data)\n"
				"{\n"
				"m_$(fname) = data;\n"
			<<	setvalid
			<<	"}\n"
				"\n";
		}
	}
	if (q == q_optional) {
		G <<	"$(inline)void $(prefix)$(msg_name)::$(field_set)($(fullrtype)v)\n"
			"{\n"
			"m_$(fname) = v;\n"
		<<	setvalid
		<<	"}\n"
			"\n";
	} else if (q == q_required) {
		G <<	"$(inline)void $(prefix)$(msg_name)::$(field_set)($(fullrtype)v)\n"
			"{\n"
			"m_$(fname) = v;\n"
			"}\n\n";
	} else if (q == q_repeated) {
		if (f->hasMessageType())
			G <<	"$(inline)$(fulltype) *$(prefix)$(msg_name)::$(field_add)()\n"
				"{\n"
				"m_$(fname).resize($(field_size)+1);\n"
				"return &m_$(fname).back();\n"
				"}\n\n";
		else if (f->hasSimpleType())
			G <<	"$(inline)void $(prefix)$(msg_name)::$(field_add)($(typestr) v)\n"
				"{\n"
				"m_$(fname).push_back(v);\n"
				"}\n\n";
		else
			G <<	"$(inline)void $(prefix)$(msg_name)::$(field_add)($(fullrtype)v)\n"
				"{\n"
				"m_$(fname).push_back(v);\n"
				"}\n\n";
		if (t == ft_string)
			G <<	"$(inline)void $(prefix)$(msg_name)::$(field_add)(const char *s)\n"
				"{\n"
				"m_$(fname).push_back(s);\n"
				"}\n\n";
		G <<	"$(inline)void $(prefix)$(msg_name)::$(field_set)(unsigned x, $(fullrtype)v)\n"
			"{\n";
		if (Asserts)
			G <<	"assert(x < $(field_size));\n";
		G <<	"m_$(fname)[x] = v;\n"
			"}\n\n";
	} else
		abort();
}


void CppGenerator::writeSetByNameR(Generator &G, Field *f)
{
	uint32_t t = f->getType();
	G <<	"if (0 == memcmp(name,\"$(fname)\",$fnamelen)) {\n"
		"if ((name[$fnamelen] == 0) && (value == 0)) {\n"
		"$field_clear();\n"
		"return 0;\n"
		"} else if (name[$fnamelen] == '[') {\n"
		"char *idxe;\n"
		"unsigned long x;\n"
		"if ((name[$($fnamelen+1)] == '+') && (name[$($fnamelen+2)] == ']')) {\n"
		"	x = m_$fname.size();\n"
		"	m_$fname.resize(x+1);\n"
		"	idxe = (char*)(name + $($fnamelen+2));\n"
		"	if (idxe[1] == 0)\n"
		"		return 0;\n"
		"} else {\n"
	       	"	x = strtoul(name+$($fnamelen+1),&idxe,0);\n"
		"	if (idxe[0] != ']')\n"
		"		$handle_error;\n"
		"	if (m_$(fname).size() <= x)\n"
		"		$handle_error;\n"
		"	if ((idxe[1] == 0) && (value == 0)) {\n"
		"		m_$fname.erase(m_$fname.begin()+x);\n"
		"		return 0;\n"
		"	}\n"
		"}\n";
	if ((t & ft_filter) == ft_msg) {
		G <<	"if (idxe[1] != '.')\n"
			"	$handle_error;\n"
			"return m_$(fname)[x].$(set_by_name)(idxe+2,value);\n"
			"}\n"
			"}\n";
		return;
	}
	if (!f->getParseAsciiFunction().empty()) {
		G <<	"return $parse_ascii(&m_$fname[x],value);\n"
			"}\n"
			"}\n";
		return;
	}
	G <<	"if (idxe[1] != 0)\n"
		"	$handle_error;\n";
	if ((t & ft_filter) == ft_enum) {
		if (target->getFlag("enumnames")) {
			G <<	"varint_t v;\n"
				"int r = parse_enum(&v,value);\n"
				"if (r > 0) {\n"
				"	$(field_set)(x,v);\n"
				"	return r;\n"
				"} else {"
				"	$handle_error;\n"
				"}\n"
				;
		} else {
			G <<	"char *eptr;\n"
				"long long ll = strtoll(value,&eptr,0);\n"
				"if (eptr == value)\n"
				"	$handle_error;\n"
				"m_$fname[x] = ($typestr) ll;\n";
			writeSetValid(G,f->getValidBit());
			G <<	"return eptr - value;\n"
				"}\n"
				"}\n";
		}
		return;
	}
	switch (t) {
	case ft_string:
		G <<	"m_$fname[x] = value;\n"
			"return m_$(fname)[x].size();\n";
		break;
	case ft_float:
		G <<	"return parse_ascii_flt(&m_$(fname)[x],value);\n";
		break;
	case ft_double:
		G <<	"return parse_ascii_dbl(&m_$(fname)[x],value);\n";
		break;
	case ft_int8:
	case ft_sint8:
	case ft_sfixed8:
		G <<	"return parse_ascii_s8(&m_$(fname)[x],value);\n";
		break;
	case ft_int16:
	case ft_sint16:
	case ft_sfixed16:
		G <<	"return parse_ascii_s16(&m_$(fname)[x],value);\n";
		break;
	case ft_int32:
	case ft_sint32:
	case ft_sfixed32:
		G <<	"return parse_ascii_s32(&m_$(fname)[x],value);\n";
		break;
	case ft_int64:
	case ft_sint64:
	case ft_sfixed64:
		G <<	"return parse_ascii_s64(&m_$(fname)[x],value);\n";
		break;
	case ft_fixed8:
	case ft_uint8:
		G <<	"return parse_ascii_u8(&m_$(fname)[x],value);\n";
		break;
	case ft_uint16:
	case ft_fixed16:
		G <<	"return parse_ascii_u16(&m_$(fname)[x],value);\n";
		break;
	case ft_uint32:
	case ft_fixed32:
		G <<	"return parse_ascii_u32(&m_$(fname)[x],value);\n";
		break;
	case ft_uint64:
	case ft_fixed64:
		G <<	"return parse_ascii_u64(&m_$(fname)[x],value);\n";
		break;
	case ft_bool:
		// vector<bool>::operator[] returns value instead of reference!
		G <<	"bool b;\n"
			"int r = parse_ascii_bool(&b,value);\n"
			"if (0 > r)\n"
			"	$handle_error;\n"
			"$(field_set)(x,b);\n"
			"return r;\n";
		break;
	case ft_signed:
	case ft_unsigned:
	default:
		abort();
	}
	G << "}\n}\n";
}


void CppGenerator::writeSetByName(Generator &G, Message *m)
{
	if (target->getOption("SetByName").empty())
		return;
	if (WithComments)
		G <<	"/*\n"
			" * Function for setting an element in dot notation with an ASCII value.\n"
			" * It will call the specified parse_ascii function for parsing the value.\n"
			" *\n"
			" * @return number of bytes successfully parsed or negative value indicating\n"
			" *         an error.\n"
			" */\n";
	G <<	"int $(prefix)$(msg_name)::$(set_by_name)(const char *name, const char *value)\n"
		"{\n";
	unsigned n = 0;
	for (auto i : m->getFields()) {
		Field *f = i.second;
		if ((f == 0) || !f->isUsed() || f->isObsolete())
			continue;
		uint32_t t = f->getType();
		if (t == ft_cptr)
			continue;
		++n;
		if (q_repeated == f->getQuantifier()) {
			if (t == ft_bytes)
				continue;
			G.setField(f);
			writeSetByNameR(G,f);
			G.setField(0);
			continue;
		}
		G.setField(f);
		if ((t & ft_filter) == ft_msg) {
			if (f->getValidBit() >= 0) {
				G <<	"if (0 == memcmp(name,\"$(fname)\",$fnamelen)) {\n"
					"if ((name[$fnamelen] == 0) && (value == 0)) {\n"
					"$field_clear();\n"
					"return 0;\n"
					"} else if (name[$fnamelen] == '.') {\n";
				writeSetValid(G,f->getValidBit());
			} else {
				G <<	"if (0 == memcmp(name,\"$(fname)\",$fnamelen)) {\n"
					"if ((name[$fnamelen] == 0) && (value == 0)) {\n"
					"m_$fname.clear();\n"
					"return 0;\n"
					"} else if (name[$fnamelen] == '.') {\n";

			}
			G <<	"return m_$(fname).$(set_by_name)(name+$($fnamelen+1),value);\n"
				"}\n"
				"}\n";
			G.setField(0);
			continue;
		}
		G <<	"if (0 == strcmp(name,\"$(fname)\")) {\n";
		if (q_required != f->getQuantifier()) {
			G <<	"if (value == 0) {\n"
				"$field_clear();\n"
				"return 0;\n"
				"}\n";
		}
		if (!f->getParseAsciiFunction().empty()) {
			G <<	"int r = $parse_ascii(&m_$fname,value);\n"
				"if (r > 0)\n";
			writeSetValid(G,f->getValidBit());
			G <<	"return r;\n"
				"}\n";
			G.setField(0);
			continue;
		}
		if ((t & ft_filter) == ft_enum) {
			if (target->getFlag("enumnames")) {
				G <<	"varint_t v;\n"
					"int r = parse_enum(&v,value);\n"
					"if (r > 0)\n"
					"	m_$(fname) = ($(fulltype))v;\n"
					"return r;\n"
					"}\n";
			} else {
				G <<	"char *eptr;\n"
					"long long ll = strtoll(value,&eptr,0);\n"
					"if (eptr == value)\n"
					"	$handle_error;\n";
				writeSetValid(G,f->getValidBit());
				G <<	"m_$fname = ($typestr) ll;\n"
					"return eptr - value;\n"
					"}\n";
				G.setField(0);
			}
			G.setField(0);
			continue;
		}
		switch (t) {
		case ft_string:
			G <<	"m_$fname = value;\n"
				"int r = m_$(fname).size();\n";
			break;
		case ft_bytes:
			G <<	"int r = parse_ascii_bytes(m_$(fname),value);\n";
			break;
		case ft_float:
			G <<	"int r = parse_ascii_flt(&m_$(fname),value);\n";
			break;
		case ft_double:
			G <<	"int r = parse_ascii_dbl(&m_$(fname),value);\n";
			break;
		case ft_int8:
		case ft_sint8:
		case ft_sfixed8:
			G <<	"int r = parse_ascii_s8(&m_$(fname),value);\n";
			break;
		case ft_int16:
		case ft_sint16:
		case ft_sfixed16:
			G <<	"int r = parse_ascii_s16(&m_$(fname),value);\n";
			break;
		case ft_int32:
		case ft_sint32:
		case ft_sfixed32:
			G <<	"int r = parse_ascii_s32(&m_$(fname),value);\n";
			break;
		case ft_int64:
		case ft_sint64:
		case ft_sfixed64:
			G <<	"int r = parse_ascii_s64(&m_$(fname),value);\n";
			break;
		case ft_fixed8:
		case ft_uint8:
			G <<	"int r = parse_ascii_u8(&m_$(fname),value);\n";
			break;
		case ft_uint16:
		case ft_fixed16:
			G <<	"int r = parse_ascii_u16(&m_$(fname),value);\n";
			break;
		case ft_uint32:
		case ft_fixed32:
			G <<	"int r = parse_ascii_u32(&m_$(fname),value);\n";
			break;
		case ft_uint64:
		case ft_fixed64:
			G <<	"int r = parse_ascii_u64(&m_$(fname),value);\n";
			break;
		case ft_bool:
			G <<	"int r = parse_ascii_bool(&m_$(fname),value);\n";
			break;
		case ft_signed:
		case ft_unsigned:
		default:
			abort();
		}
		int v = f->getValidBit();
		if (v >= 0) {
			G << "if (r > 0)\n";
			writeSetValid(G,v);
		}
		G <<	"return r;\n"
			"}\n";
		G.setField(0);
	}
	G <<	"$handle_error;\n}\n\n";
}


void CppGenerator::writeClear(Generator &G, Field *f)
{
	uint8_t q = f->getQuantifier();
	if (q == q_required)
		return;
	if (WithComments)
		G <<	"/*!\n"
			" * Function for clearing the associated member variable.\n"
			" * It will reset the value to the default value.\n"
			" */\n";
	G <<	"$(inline)void $(prefix)$(msg_name)::$(field_clear)()\n"
		"{\n";
	uint32_t type = f->getType();
	int vbit = f->getValidBit();
	if (vbit >= 0) {
		writeClearValid(G,vbit);
	}
	if ((q == q_repeated) || ((type & ft_filter) == ft_msg)) {
		G << "m_$(fname).clear();";
	} else if (const char *invStr = f->getInvalidValue()) {
		G << "m_$(fname) = " << invStr << ';';
	} else if (const char *defStr = f->getDefaultValue()) {
		G << "m_$(fname) = " << defStr << ';';
	} else if ((type == ft_string) || (type == ft_bytes)) {
		G << "m_$(fname).clear();";
	} else {
		G << "m_$(fname) = 0;";
	}
	G <<	"\n}\n\n";
}


void CppGenerator::writeCalcSize(Generator &G, Field *f)
{
	G.setField(f);
	if (WithComments)
		writeFieldComment(G,f);
	if (f->isDeprecated() || f->isObsolete() || !f->isUsed()) {
		G.setField(0);
		return;
	}
	uint32_t type = f->getType();
	uint8_t quan = f->getQuantifier();
	unsigned ts = f->getTagSize();
	string tstr,packedtagstr;
	const char *packed = "";
	if (quan != q_required) {
		char buf[32];
		sprintf(buf," + %d /* tag($fname) 0x%x */",ts,f->getId()<<3);
		tstr = buf;
	}
	if (quan == q_repeated) {
		bool v = f->isVirtual();
		if (f->hasMessageType()) {
			if (WithComments)
				G <<	"// repeated message $(fname)\n";
			G <<	"for (size_t x = 0, y = $(field_size); x < y; ++x) {\n";
			if (v)
				G <<	"size_t s = $(field_get)(x).$(calcSize)();\n";
			else
				G <<	"size_t s = m_$(fname)[x].$calcSize();\n";
			if (PaddedMsgSize)
				G << "r += sizeof(varint_t)*8/7+1;\n";
			else
				G << "r += $(wiresize_u)(s);\n";
			G <<	"r += s" << tstr << ";\n"
				"}\n";
			G.setField(0);
			return;
		}
		if (v)
			G << "if (!$(fname)_empty()) {\n";
		else
			G << "if (!m_$(fname).empty()) {\n";
		if (f->isPacked()) {
		// dynamic encoding cannot be used for packed arrays
			uint32_t t = type;
			signed shift = -1;
			G.addVariable("index","x");
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
				if (WithComments)
					G <<	"// $(fname): packed repeated $(typestr)\n";
				G <<	"size_t $(fname)_dl = 0;\n"
					"for (size_t x = 0, y = $(field_size); x < y; ++x)\n"
					"$(fname)_dl += $(wiresize_s)((int64_t)$(field_value));\n"
					"r += $(fname)_dl + $(wiresize_s)($(fname)_dl) /* data length */" << tstr << ";\n";
				break;
			default:
				assert((type & ft_filter) == ft_enum);
				/* FALLTHRU */
			case ft_uint8:
			case ft_uint16:
			case ft_uint32:
			case ft_uint64:
				if (WithComments)
					G <<	"// $(fname): packed repeated $(typestr)\n";
				G <<	"size_t $(fname)_dl = 0;\n"
					"for (size_t x = 0, y = $(field_size); x < y; ++x)\n"
					"$(fname)_dl += $(wiresize_u)((varint_t)$(field_value));\n"
					"r += $(fname)_dl + $(wiresize_u)($(fname)_dl) /* data length */" << tstr << ";\n";
				break;
			case ft_int:
			case ft_int8:
			case ft_int16:
			case ft_int32:
			case ft_int64:
				if (WithComments)
					G <<	"// $(fname): packed repeated $(typestr)\n";
				G <<	"size_t $(fname)_dl = 0;\n"
					"for (size_t x = 0, y = $(field_size); x < y; ++x)\n"
					"$(fname)_dl += $(wiresize_x)($field_value);\n"
					"r += $(fname)_dl + $(wiresize_x)($(fname)_dl) /* data length */" << tstr << ";\n";
				break;
			}
			if (shift == 0) {
				if (WithComments)
					G <<	"// $(fname): repeated packed $(typestr), with fixed element size of one byte\n";
				G <<	"size_t $(fname)_dl = $(field_size);\n"
					"r += $(fname)_dl + $(wiresize_u)($(fname)_dl)" << tstr << ";\n";
			} else if (shift > 0) {
				if (WithComments)
					G <<	"// $(fname): repeated packed $(typestr), with fixed element size\n";
				G <<	"size_t $(fname)_dl = $(field_size) << " << shift << ";\n"
					"r += $(fname)_dl + $(wiresize_u)($(fname)_dl)" << tstr << ";\n";
			}
			G.clearVariable("index");
			G.setField(0);
			G <<	"}\n";	// end of if (!m_(fname).empty())
			return;
		}
		G.addVariable("index","x");
		if (f->hasFixedSize()) {
			assert(!f->isPacked());
			if (WithComments)
				G <<	"// $(fname): non-packed, fixed size elements\n";
			G <<	"r += $(field_size) * " << f->getFixedSize() << ";\t// including tag\n";
		} else if (f->hasEnumType()) {
			if (WithComments)
				G <<	"// $(fname): repeated enum $(typestr)\n";
			G <<	"for (size_t x = 0, y = $(field_size); x < y; ++x)\n"
				"r += $(wiresize_u)($field_value);\n"
				"r += $(field_size) * $(tagsize);\t// tags\n";
		} else if (type == ft_cptr) {
			if (WithComments)
				G <<	"// $(fname): repeated $(typestr)\n";
			G <<	"for (size_t x = 0, y = $(field_size); x < y; ++x) {\n"
				"	size_t $(fname)_s = $(field_value) ? (strlen($field_value) + 1) : 1;\n"
				"	r += $(fname)_s + $(wiresize_u)($(fname)_s)" << tstr << ";\n"
				"}\n";
		} else if ((type == ft_bytes) || (type == ft_string)) {
			if (WithComments)
				G <<	"// $(fname): repeated $(typestr)\n";
			G <<	"for (size_t x = 0, y = $(field_size); x < y; ++x) {\n"
				"size_t s = $(field_value).size();\n"
				"r += $(wiresize_u)(s);\n"
				"r += s" << tstr << ";\n"
				"}\n";
		} else if ((type == ft_sint8) || (type == ft_sint16) || (type == ft_sint32) || (type == ft_sint64)) {
			if (WithComments)
				G <<	"// $(fname): " << packed << "repeated $(typestr)\n";
			G <<	"size_t $(fname)_dl = 0;\n"
				"for (size_t x = 0, y = $(field_size); x < y; ++x)\n"
				"$(fname)_dl += $(wiresize_s)((int64_t)$(field_value))" << tstr << ";\n"
				"r += $(fname)_dl;\n";
		} else if ((type == ft_int8) || (type == ft_int16) || (type == ft_int32) || (type == ft_int64)) {
			if (WithComments)
				G <<	"// $(fname): " << packed << "repeated $(typestr)\n";
			G <<	"size_t $(fname)_dl = 0;\n"
				"for (size_t x = 0, y = $(field_size); x < y; ++x)\n"
				"$(fname)_dl += $(wiresize_x)($field_value)" << tstr << ";\n"
				"r += $(fname)_dl;\n";
		} else if ((type == ft_uint8) || (type == ft_uint16) || (type == ft_uint32) || (type == ft_uint64)
			|| ((type & ft_filter) == ft_enum)) {
			if (WithComments)
				G <<	"// $(fname): " << packed << "repeated $(typestr)\n";
			G <<	"size_t $(fname)_dl = 0;\n"
				"for (size_t x = 0, y = $(field_size); x < y; ++x)\n"
				"$(fname)_dl += $(wiresize_u)((varint_t)$(field_value))" << tstr << ";\n"
				"r += $(fname)_dl;\n";
		} else
			abort();
		G.clearVariable("index");
		G	<< "}\n";	// end of if (!m_(fname).empty())
		G.setField(0);
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
		if (f->isVirtual())
			G <<	"size_t $(fname)_s = $(field_get)().$calcSize();\n"
				"r += $(fname)_s + $(wiresize_u)($(fname)_s)" << tstr << ";\n";
		else if (PaddedMsgSize)
			G <<	"size_t $(fname)_s = m_$(fname).$calcSize();\n"
				"r += $(fname)_s + sizeof(varint_t)*8/7+1" << tstr << ";\n";
		else
			G <<	"size_t $(fname)_s = m_$(fname).$calcSize();\n"
				"r += $(fname)_s + $(wiresize_u)($(fname)_s)" << tstr << ";\n";
	} else if ((type & ft_filter) == ft_enum) {
		if (f->isVirtual())
			G << "r += $(wiresize_u)($(field_get)())" << tstr << ";\n";
		else
			G << "r += $(wiresize_u)((varint_t)m_$(fname))" << tstr << ";\n";
	} else switch (type) {
	case ft_int8:
	case ft_int16:
	case ft_int32:
	case ft_int64:
		G << "r += $(wiresize_x)((varint_t)$(field_value))" << tstr << ";\n";
		break;
	case ft_uint8:
	case ft_uint16:
	case ft_uint32:
	case ft_uint64:
		G << "r += $(wiresize_u)((varint_t)$(field_value))" << tstr << ";\n";
		break;
	case ft_sint8:
	case ft_sint16:
	case ft_sint32:
	case ft_sint64:
		G << "r += $(wiresize_s)((varint_t)$(field_value))" << tstr << ";\n";
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
			G <<	"size_t $(fname)_s = $(field_value) ? (strlen($field_value) + 1) : 1;\n";
		else
			G <<	"size_t $(fname)_s = $(field_value) ? (strlen($field_value) + 1) : 0;\n";
		G <<	"r += $(fname)_s + $(wiresize_u)($(fname)_s)" << tstr << ";\n";
		break;
	case ft_bytes:
	case ft_string:
		G <<	"size_t $(fname)_s = $(field_value).size();\n"
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
	G <<	"size_t $(prefix)$(msg_name)::$calcSize() const\n"
		"{\n";
	size_t n = 0;
	for (auto i : m->getFields()) {
		Field *f = i.second;
		if ((f == 0) || (!f->isUsed()) || f->isObsolete())
			continue;
		if (f->getQuantifier() == 1) {
			size_t ts = f->getTagSize();
			G << "// " << f->getWfcType() << ' ' << f->getName() << ": tagsize " << ts;
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
	for (auto i : m->getFields()) {
		Field *f = i.second;
		if (f == 0)
			continue;
		if (!(f->hasFixedSize() && (f->getQuantifier() == 1)))
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
	G.fillField("*a++");
}


void CppGenerator::decode16bit(Generator &G, Field *f)
{
	G <<	"if ((a+1) >= e)\n"
		"	$handle_error;\n";
	switch (f->getTypeClass()) {
	case ft_uint16:
	case ft_enum:
		G.fillField("read_u16(a)");
		break;
	default:
		G.fillField("($typestr) read_u16(a)");
	}
	G <<	"a += 2;\n";
}


void CppGenerator::decode32bit(Generator &G, Field *f)
{
	G <<	"if ((a+3) >= e)\n"
		"	$handle_error;\n";
	switch (f->getTypeClass()) {
	case ft_float:
		G.fillField("read_float(a)");
		break;
	case ft_uint32:
	case ft_enum:
		G.fillField("read_u32(a)");
		break;
	default:
		G.fillField("($typestr) read_u32(a)");
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
		G.fillField("read_double(a)");
		break;
	case ft_enum:
	case ft_uint32:
		G.fillField("read_u64(a)");
		break;
	default:
		G.fillField("($typestr) read_u64(a)");
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
	G.fillField("v");
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
	G.fillField("varint_sint(v)");
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
	if ((f->getQuantifier() == q_repeated) && !f->isVirtual())
		G << "$(m_field).emplace_back();\n";
	G <<	"if (v != 0) {\n";
	G.fillField("(const uint8_t*)a,v");
	G <<	"if (n < 0)\n"
		"return n;\n"
		"if (n != (ssize_t)v)\n"
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
	if ((optmode == optspeed) || (target->getOption("stringtype") == "C")) {
		G <<	"{\n"
			"varint_t v;\n"
			"int n = read_varint(a,e-a,&v);\n"
			"a += n;\n"
			"if ((n <= 0) || ((a+v) > e))\n"
			"	$handle_error;\n";
		G.fillField("(const char*)a,v");
		G <<	"a += v;\n"
			"}\n";
	} else {
		G <<	"{\n";
		if (f->isRepeated())
			G << "int n = decode_bytes_element(m_$fname,a,e);\n";
		else
			G << "int n = decode_bytes(m_$fname,a,e);\n";
		G <<	"a += n;\n"
			"if ((n <= 0) || (a > e))\n"
			"	$handle_error;\n"
			"}\n";
	}
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
	if (!f->isUsed() || f->isObsolete()) {
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr\n";
		writeSkipContent(G,f->getEncoding());
		G.setField(0);
		return;
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
		default:
			abort();
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
			decodeVarint(G,f);
			break;
		case ft_signed:
		case ft_sint8:
		case ft_sint16:
		case ft_sint32:
		case ft_sint64:
			decodeSVarint(G,f);
			break;
		case ft_bool:
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
		G.fillField("(const char*)a");
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
	default:
		abort();
	}
	G << "break;\n";
	G.setField(0);
}


void CppGenerator::writeTagToMemory(Generator &G, Field *f, unsigned fixedlen)
{
	// fixedlen reserves space in memory for the field value
	// if the field value has a fixed length
	G << "// '$(fname)': id=$(field_id), encoding=$(field_enc), tag=$(field_tag)\n";
	if (optmode == optspeed) {
		unsigned id = f->getId();
		uint32_t encoding = f->getEncoding();
		unsigned xid = (id << 3) | encoding;
		char buf[32];
		if (f->isPacked())
			xid = (id << 3) | 2;
		if (fixedlen != 0) {
			G <<	"if (" << fixedlen << " > (e-a))\n"
				"	$handle_error;\n";
		} else if (xid < 0x80) {
			G <<	"if (a >= e)\n"
				"	$handle_error;\n";
		} else {
			unsigned es = wiresize_u64(xid);
			G <<	"if (" << es << " > (e-a))\n"
				"	$handle_error;\n";
		}
		unsigned bid = xid;
		while (bid&(~0x7f)) {
			sprintf(buf,"%x",(bid&0x7f)|0x80);
			G << "*a++ = 0x" << buf << ";\n";
			bid >>= 7;
		}
		sprintf(buf,"%x",bid&0x7f);
		G <<	"*a++ = 0x" << buf << ";\n";
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
		if (fixedlen != 0) {
			G <<	"if (" <<fixedlen << " > (e-a))\n"
				"	$handle_error;\n";
			if (xid < 0x80) {
				char buf[8];
				sprintf(buf,"%x",xid);
				G << "*a++ = 0x" << buf << ";\n";
			} else {
				G << "a += write_varint(a,e-a,$(field_tag));\t// '$(fname)': id=$(field_id)\n";
			}
		} else if (xid < 0x80) {
			char buf[8];
			sprintf(buf,"%x",xid);
			G <<	"if (a >= e)\n"
				"	$handle_error;\n"
				"*a++ = 0x" << buf << ";\n";
		} else if (xid < 0xc000) {
			char buf[32];
			unsigned es = wiresize_u64(xid);
			G <<	"if (" << (es) << " > (e-a))\n"
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
	G <<	"$(inline)size_t $(prefix)$(msg_name)::$getMaxSize()\n"
		"{\n";
	int64_t maxsize = 0;
	for (auto i : m->getFields()) {
		Field *f = i.second;
		if (f == 0)
			continue;
		if (WithComments)
			writeFieldComment(G,f,"");
		if (!f->isUsed() || f->isObsolete()) {
			if (WithComments)
				G << '\n';
			continue;
		}
		int64_t s = f->getMaxSize();
		if (s == -1) {
			maxsize = -1;
			if (WithComments)
				G << " has unlimited size\n";
		} else if (WithComments) {
			G << " has maximum size " << s << '\n';
		}
		if (maxsize != -1)
			maxsize += s;
	}
	G <<	"return ";
	if (-1 == maxsize)
		G << "SIZE_MAX";
	else
		G << maxsize;
	G <<	";\n"
		"}\n"
		"\n";
}


void CppGenerator::writeToMemory(Generator &G, Field *f)
{
	G.setField(f);
	if (f->isDeprecated() || f->isObsolete()) {
		if (WithComments)
			G << "// '$(fname)' is " << f->getUsage() << ". Therefore no data will be written.\n";
		G.setField(0);
		return;
	}
	uint32_t type = f->getType();
	uint8_t quan = f->getQuantifier();
	uint32_t encoding = f->getEncoding();
	switch (quan) {
	case q_optional:
		if ((optmode == optreview) || (mem_virtual == f->getStorage())) {
			G <<	"if ($(field_has)()) {\n";
		} else {
			if (WithComments)
				G <<	"// has $fname?\n";
			G <<	"if (";
			writeGetValid(G,f);
			G <<	") {\n";
		}
		break;
	case q_repeated:
		if (f->isPacked() && f->hasSimpleType()) {
			// packed encoding: tag,length,data
			G <<	"if (size_t $(fname)_ne = $(field_size)) {\n";
			G.setVariableHex("field_tag",f->getId()<<3|2);
			G.addVariable("index","x");
			writeTagToMemory(G,f);
			if (((type&ft_filter) == ft_enum) || (type == ft_uint8) ||  (type == ft_uint16) || (type == ft_uint32) || (type == ft_uint64) || (type == ft_int64)) {
				G <<	"ssize_t $(fname)_ws = 0;\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
					"	$(fname)_ws += $(wiresize_u)($(field_value));\n"
					"n = write_varint(a,e-a,$(fname)_ws);\n"
					"a += n;\n"
					"if ((n <= 0) || ($(fname)_ws > (e-a)))\n"
					"	$handle_error;\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
					"	a += write_varint(a,e-a,$(field_value));\n";
			} else if ((type == ft_int8) || (type == ft_int16) || (type == ft_int32)) {
				// negative numbers with varint_t < 64bit need padding
				G <<	"ssize_t $(fname)_ws = 0;\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
					"	$(fname)_ws += $(wiresize_u)($(field_value));\n"
					"n = write_varint(a,e-a,$(fname)_ws);\n"
					"a += n;\n"
					"if ((n <= 0) || ($(fname)_ws > (e-a)))\n"
					"	$handle_error;\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n";
				if (VarIntBits < 64)
					G<< "	a += write_xvarint(a,e-a,$(field_value));\n";
				else
					G<< "	a += write_varint(a,e-a,$(field_value));\n";
			} else if ((type == ft_sint8) || (type == ft_sint16) || (type == ft_sint32) || (type == ft_sint64)) {
				G <<	"ssize_t $(fname)_ws = 0;\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
					"	$(fname)_ws += $(wiresize_s)($(field_value));\n"
					"n = write_varint(a,e-a,$(fname)_ws);\n"
					"a += n;\n"
					"if ((n <= 0) || ($(fname)_ws > (e-a)))\n"
					"	$handle_error;\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
					"	a += write_varint(a,e-a,sint_varint($(field_value)));\n";
			} else if ((type == ft_bool) || (type == ft_fixed8) ||  (type == ft_sfixed8)) {
				G <<	"ssize_t $(fname)_ws = $(fname)_ne * sizeof($typestr);\n"
					"n = write_varint(a,e-a,$(fname)_ws);\n"
					"a += n;\n"
					"if ((n <= 0) || ($(fname)_ws > (e-a)))\n"
					"	$handle_error;\n";
				if (f->isVirtual())
					G <<	"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
						"*a++ = $(field_value);\n";
				else
					G <<	"memcpy(a,$(field_values),$(fname)_ws);\n";
				G <<	"a += $(fname)_ws;\n";
			} else if ((Endian == little_endian) && f->hasFixedSize() && !f->isVirtual()) {
				G <<	"ssize_t $(fname)_ws = $(fname)_ne * sizeof($typestr);\n"
					"n = write_varint(a,e-a,$(fname)_ws);\n"
					"a += n;\n"
					"if ((n <= 0) || ($(fname)_ws > (e-a)))\n"
					"	$handle_error;\n"
					"memcpy(a,$(field_values),$(fname)_ws);\n"
					"a += $(fname)_ws;\n";
			} else if ((type == ft_fixed16) || (type == ft_sfixed16)) {
				G <<	"ssize_t $(fname)_ws = $(fname)_ne << 1;\n"
					"n = write_varint(a,e-a,$(fname)_ws);\n"
					"a += n;\n"
					"if ((n <= 0) || ($(fname)_ws > (e-a)))\n"
					"	$handle_error;\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x, a+=2)\n"
					"	write_u16(a,$(field_value));\n";
			} else if ((type == ft_fixed32) || (type == ft_sfixed32)) {
				G <<	"ssize_t $(fname)_ws = $(fname)_ne << 2;\n"
					"n = write_varint(a,e-a,$(fname)_ws);\n"
					"a += n;\n"
					"if ((n <= 0) || ($(fname)_ws > (e-a)))\n"
					"	$handle_error;\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x, a+=4)\n"
					"	write_u32(a,$(field_value));\n";
			} else if (type == ft_float) {
				G <<	"ssize_t $(fname)_ws = $(fname)_ne << 2;\n"
					"n = write_varint(a,e-a,$(fname)_ws);\n"
					"a += n;\n"
					"if ((n <= 0) || ($(fname)_ws > (e-a)))\n"
					"	$handle_error;\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x, a+=4)\n"
					"	write_u32(a,mangle_float($(field_value)));\n";
			} else if ((type == ft_fixed64) || (type == ft_sfixed64)) {
				G <<	"ssize_t $(fname)_ws = $(fname)_ne << 3;\n"
					"n = write_varint(a,e-a,$(fname)_ws);\n"
					"a += n;\n"
					"if ((n <= 0) || ($(fname)_ws > (e-a)))\n"
					"	$handle_error;\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x, a+=8)\n"
					"	write_u64(a,$(field_value));\n";
			} else if (type == ft_double) {
				G <<	"ssize_t $(fname)_ws = $(fname)_ne << 3;\n"
					"n = write_varint(a,e-a,$(fname)_ws);\n"
					"a += n;\n"
					"if ((n <= 0) || ($(fname)_ws > (e-a)))\n"
					"	$handle_error;\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x, a+=8)\n"
					"	write_u64(a,mangle_double($(field_value)));\n";
			} else
				abort();
			G	<< "}\n";
			G.setField(0);
			G.clearVariable("index");
			return;
		}
		if (f->hasSimpleType()) {
			G <<	"for (auto x : m_$fname) {\n";
		} else {
			G <<	"for (const auto &x : m_$fname) {\n";
		}
		G.setVariable("field_value","x");
		break;
	case q_required:
		break;
	default:
		abort();
	}
	writeTagToMemory(G,f,f->hasFixedSize() ? f->getFixedSize() : 0);
	if ((type & ft_filter) == ft_msg)
		encoding = wt_msg;
	switch (encoding) {
	case wt_varint:	// varint
		if ((type == ft_sint8) || (type == ft_sint16) || (type == ft_sint32) || (type == ft_sint64))
			G <<	"n = write_varint(a,e-a,sint_varint($(field_value)));\n"
				"if (n <= 0)\n"
				"	$handle_error;\n"
				"a += n;\n";
		else if ((VarIntBits < 64) && ((type == ft_int8) || (type == ft_int16) || (type == ft_int32)))
			// these types may need padding when varint_t is
			// < 64bit to be compatible with 64bit systems
			G <<	"n = write_xvarint(a,e-a,$(field_value));\n"
				"if (n <= 0)\n"
				"	$handle_error;\n"
				"a += n;\n";
		else
			G <<	"n = write_varint(a,e-a,$(field_value));\n"
				"if (n <= 0)\n"
				"	$handle_error;\n"
				"a += n;\n";
		break;
	case wt_64bit:	// 64bit value
		if ((quan == q_repeated) || (optmode == optreview))
			G <<	"if ((a+8) > e)\n"
				"	$handle_error;\n";
		if (type == ft_double) {
			if ((Endian == little_endian) && (optmode == optspeed)) {
				G <<	"*(double*)a = $(field_value);\n";
			} else {
				G <<	"write_u64(a,mangle_double($(field_value)));\n";
			}
		} else {
			if ((Endian == little_endian) && (optmode == optspeed)) {
				G <<	"*(uint64_t*)a = $(field_value);\n";
			} else {
				G <<	"write_u64(a,(uint64_t)$(field_value));\n";
			}
		}
		G <<	"a += 8;\n";
		break;
	case wt_8bit:	// 8bit value
		if ((quan == q_repeated) || (optmode == optreview))
			G <<	"if (a >= e)\n"
				"	$handle_error;\n";
		G <<	"*a++ = $(field_value);\n";
		break;
	case wt_16bit:
		if ((quan == q_repeated) || (optmode == optreview))
			G <<	"if ((e-a) < 2)\n"
				"	$handle_error;\n";
		G <<	"write_u16(a,$(field_value));\n"
			"a += 2;\n";
		break;
	case wt_msg: 	// length delimited message
		if (PaddedMsgSize) {
			G <<	"if ((e-a) < (ssize_t)(sizeof(varint_t)*8/7+1))\n"
				"	$handle_error;\n"
				"n = $(field_value).$toMemory(a+(sizeof(varint_t)*8/7+1),e-a-(sizeof(varint_t)*8/7+1));\n"
				"if (n < 0)\n"	// == 0 is ok, could be empty message
				"	$handle_error;\n"
				"place_varint(a,n);\n"
				"a += sizeof(varint_t)*8/7+1;\n"
				"a += n;\n";
		} else {
			G <<	"ssize_t $(fname)_ws = $(field_value).$calcSize();\n"
				"n = write_varint(a,e-a,$(fname)_ws);\n"
				"a += n;\n"
				"if ((n <= 0) || ($(fname)_ws > (e-a)))\n"
				"	$handle_error;\n"
				"n = $(field_value).$toMemory(a,e-a);\n"
				"a += n;\n";
			if (Asserts)
				G << "assert(n == $(fname)_ws);\n";
		}
		break;
	case wt_lenpfx:	// length delimited byte array or string
		if (type == ft_cptr) {
			G <<	"if ($(field_value)) {\n"
				"	ssize_t $(fname)_s = strlen($field_value) + 1;\n"
				"	n = write_varint(a,e-a,$(fname)_s);\n"
				"	a += n;\n"
				"	if ((n <= 0) || ((e-a) < $(fname)_s))\n"
				"		$handle_error;\n"
				"	memcpy(a,$(field_value),$(fname)_s);\n"
				"	a += $(fname)_s;\n"
				"} else {\n"
				"	// transmit empty string of lenght 1\n"
				"	if (e-a < 2)\n"
				"		$handle_error;\n"
				"	*a++ = 1;\n"
				"	*a++ = 0;\n"
				"}\n";
		} else {
			if ((optmode == optspeed) || (target->getOption("stringtype") == "C")) {
				G <<	"ssize_t $(fname)_s = $(field_value).size();\n"
					"n = write_varint(a,e-a,$(fname)_s);\n"
					"a += n;\n"
					"if ((n <= 0) || ((e-a) < $(fname)_s))\n"
					"	$handle_error;\n"
					"memcpy(a,$(field_value).data(),$(fname)_s);\n"
					"a += $(fname)_s;\n";
			} else {
				G <<	"n = encode_bytes($field_value,a,e);\n"
					"if (n < 0)\n"
					"	$handle_error;\n"
					"a += n;\n";
			}
		}
		break;
	case wt_32bit:	// 32bit value
		G <<	"if ((e-a) < 4)\n"
			"	$handle_error;\n";
		if (type == ft_float) {
			G <<	"write_u32(a,mangle_float($(field_value)));\n";
		} else {
			G <<	"write_u32(a,(uint32_t)$(field_value));\n";
		}
		G <<	"a += 4;\n";
		break;
	default:
		abort();
	}
	if (quan != 1)
		G << "}\n";
	G.clearVariable("index");
	G.setField(0);
	
}


void CppGenerator::writeToX(Generator &G, Field *f)
{
	if (f->isDeprecated() || f->isObsolete()) {
		if (WithComments)
			G << "// '" << f->getName() << "' is deprecated. Therefore no data will be written.\n";
		return;
	}
	G.setField(f);
	uint32_t type = f->getType();
	uint8_t quan = f->getQuantifier();
	uint32_t encoding = f->getEncoding();
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
			G <<	"if (size_t $(fname)_ne = $(field_size)) {\n";
			G.setVariableHex("field_tag",f->getId()<<3|2);
			G.addVariable("index","x");
			writeTagToX(G,f);
			if (((type&ft_filter) == ft_enum) || (type == ft_int64) ||  (type == ft_uint8) ||  (type == ft_uint16) || (type == ft_uint32) || (type == ft_uint64)) {
				G <<	"size_t $(fname)_ws = 0;\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
					"$(fname)_ws += $(wiresize_u)($(field_value));\n"
					"$write_varint($(fname)_ws);\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
					"$write_varint($(field_value));\n";
			} else if ((type == ft_int8) || (type == ft_int16) || (type == ft_int32)) {
				G <<	"size_t $(fname)_ws = 0;\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
					"$(fname)_ws += $(wiresize_x)($(field_value));\n"
					"$write_varint($(fname)_ws);\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n";
				if (VarIntBits < 64)
					G << "	$write_xvarint($(field_value));\n";
				else
					G << "	$write_varint($(field_value));\n";
			} else if ((type == ft_sint8) || (type == ft_sint16) || (type == ft_sint32) || (type == ft_sint64)) {
				G <<	"size_t $(fname)_ws = 0;\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
					"$(fname)_ws += $(wiresize_s)($(field_value));\n"
					"$write_varint($(fname)_ws);\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
					"$write_varint(sint_varint($(field_value)));\n";
			} else if ((type == ft_bool) || (type == ft_fixed8) || (type == ft_sfixed8)) {
				G <<	"$write_varint($(fname)_ne);\n"
					"$write_bytes((const uint8_t *)$(field_values),$(fname)_ne);\n";
			} else if ((Endian == little_endian) && ((type == ft_fixed16) || (type == ft_fixed32) || (type == ft_fixed64) || (type == ft_float) || (type == ft_double))) {
				G <<	"size_t $(fname)_ws = $(fname)_ne * sizeof($(typestr));\n"
					"$write_varint($(fname)_ws);\n"
					"$write_bytes((const uint8_t *)$(field_values),$(fname)_ws);\n";
			} else if ((type == ft_fixed16) || (type == ft_sfixed16)) {
				G <<	"$write_varint($(fname)_ne << 1);\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
					"$(u16_wire((uint16_t)$(field_value)));\n";
			} else if ((type == ft_fixed32) || (type == ft_sfixed32)) {
				G <<	"$write_varint($(fname)_ne << 2);\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
					"$(u32_wire((uint32_t)$(field_value)));\n";
			} else if ((type == ft_fixed64) || (type == ft_sfixed64)) {
				G <<	"$write_varint($(fname)_ne << 3);\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
					"$(u64_wire((uint64_t)$(field_value)));\n";
			} else if (type == ft_float) {
				G <<	"$write_varint($(fname)_ne << 2);\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
					"$(u32_wire(mangle_float($(field_value))));\n";
			} else if (type == ft_double) {
				G <<	"$write_varint($(fname)_ne << 3);\n"
					"for (size_t x = 0; x != $(fname)_ne; ++x)\n"
					"$(u64_wire(mangle_double($(field_value))));\n";
			}
			else
				abort();
			G	<< "}\n";
			G.setField(0);
			G.clearVariable("index");
			return;
		}
		G <<	"for (size_t x = 0, y = $(field_size); x != y; ++x) {\n";
		G.addVariable("index","x");
		break;
	case q_required:
		break;
	default:
		abort();
	}
	writeTagToX(G,f);
	switch (encoding) {
	case wt_varint:	// varint
		if ((type == ft_sint8) || (type == ft_sint16) || (type == ft_sint32) || (type == ft_sint64))
			G <<	"$write_varint(sint_varint($(field_value)));\n";
		else if ((VarIntBits < 64) && ((type == ft_int8) || (type == ft_int16) || (type == ft_int32)))
			G <<	"$write_xvarint($(field_value));\n";
		else
			G <<	"$write_varint($(field_value));\n";
		break;
	case wt_64bit:	// 64bit value
		if (type == ft_double) {
			G <<	"$(u64_wire(mangle_double($(field_value))));\n";
		} else
			G <<	"$(u64_wire((uint64_t)$(field_value)));\n";
		break;
	case wt_8bit:	// 8bit value
		G <<	"$wireput($(field_value));\n";
		break;
	case wt_16bit:
		G <<	"$(u16_wire($(field_value)));\n";
		break;
	case wt_lenpfx:	// length delimited message, byte array or string
		if ((type & ft_filter) == ft_msg) {
			G <<	"$write_varint($(field_value).$calcSize());\n"
				"$(field_value).$(toX)($putarg);\n";
		} else if (type == ft_cptr) {
			G <<	"if ($(field_value)) {\n"
				"	size_t $(fname)_s = strlen($(field_value)) + 1;\n"
			 	"	$write_varint($(fname)_s);\n"
				"	uint8_t *$(fname)_d = (uint8_t*) $(field_value);\n"
				"	do {\n"
				"	$wireput(*$(fname)_d++);\n"
				"	} while (--$(fname)_s);\n"
				"} else {\n"
				"	// write 0x01 0x00 for empty C string\n"
				"	$wireput(1);\n"
				"	$wireput(0);\n"
				"}\n";
		} else {
			G <<	"size_t $(fname)_s = $(field_value).size();\n"
			 	"$write_varint($(fname)_s);\n"
				"$write_bytes((const uint8_t*) $(field_value).data(),$(fname)_s);\n";
		}
		break;
	case wt_32bit:	// 32bit value
		if (type == ft_float) {
			G <<	"$(u32_wire(mangle_float($(field_value))));\n";
		} else {
			G <<	"$(u32_wire((uint32_t)$(field_value)));\n";
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
		"if (fn <= 0)\n"
		"	$handle_error;\n"
		"a += fn;\n"
		"switch (fid) {\n";
	bool hasNullId = false;
	for (auto i : m->getFields()) {
		Field *f = i.second;
		if (f == 0)
			continue;
		if (!f->isUsed()) {
			hasUnused = true;
			continue;
		}
		if (f->isObsolete())
			continue;
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
		for (auto i : m->getFields()) {
			Field *f = i.second;
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
	if ((Terminator == "null") || (Terminator == "0x0") || (Terminator == "0")) {
		if (hasNullId) 
			error("request to handle null termination, but null-tag exists");
		G <<	"case 0:\t// terminate on null byte\n"
			"	break;\n";
	}
	G <<	"default:\n";
	if (target->getOption("UnknownField") == "assert") {
		if (WithComments)
			G << "// unknown field (option unknown=assert)\n";
		G << "assert(0);\n";
	} else if (target->getOption("UnknownField") == "skip") {
		if (WithComments)
			G << "// unknown field (option unknown=skip)\n";
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
	G <<	"ssize_t $(prefix)$(msg_name)::$toMemory(uint8_t *b, ssize_t s) const\n"
		"{\n";
	if (Debug)
		G << "std::cout << \"$(prefix)$(msg_name)::$toMemory(\" << (void*)b << \", \" << s << \")\\n\";\n";
	if (Asserts)
		G << "assert(s >= 0);\n";
	G <<	"uint8_t *a = b, *e = b + s;\n";
	bool needN = false;
	auto fields = m->getFields();
	if (optmode == optspeed) {
		for (auto i : fields) {
			Field *f = i.second;
			if ((f == 0) || (!f->isUsed()) || f->isDeprecated() || f->isObsolete())
				continue;
			if (!f->hasFixedSize() || (f->isRepeated() && f->isPacked())) {
				needN = true;
				break;
			}
		}
	} else if (optmode == optsize) {
		for (auto i : fields) {
			Field *f = i.second;
			if ((f == 0) || (!f->isUsed()) || f->isDeprecated() || f->isObsolete())
				continue;
			if ((f->getTagSize() > 2) || !f->hasFixedSize() || (f->isRepeated() && f->isPacked())) {
				needN = true;
				break;
			}
		}
	} else {
		for (auto i : fields) {
			Field *f = i.second;
			if ((f != 0) && f->isUsed() && !f->isDeprecated() && !f->isObsolete()) {
				needN = true;
				break;
			}
		}
	}
	if (needN)
		G <<	"signed n;\n";
	for (auto i : fields) {
		Field *f = i.second;
		if (f == 0)
			continue;
		if (!f->isUsed() || f->isDeprecated() || f->isObsolete()) {
			if (WithComments) {
				G.setField(f);
				G << "// '$(fname)' is" << f->getUsage() << ". Therefore no data will be written.\n";
				G.setField(0);
			}
		} else {
			writeToMemory(G,f);
		}
	}
	//if (Asserts)
		//G <<	"assert((a-b) == (signed)$calcSize());\n";
	const string &Terminator = target->getOption("Terminator");
	if ((Terminator == "ff") || (Terminator == "0xff")) {
		if (WithComments)
			G << "// write terminating ff byte\n";
		G << "*a++ = 0xff;\n";
	} else if ((Terminator == "null") || (Terminator == "0x0") || (Terminator == "0")) {
		if (WithComments)
			G <<"// write terminating null byte\n";
		G << "*a++ = 0;\n";
	}
	if (Asserts)
		G << "assert(a <= e);\n";
	G <<	"return a-b;\n"
		"}\n"
		"\n";
}


void CppGenerator::writeToJson(Generator &G, Field *f, char fsep)
{
	// fsepmode: 0 = '{', 1 = ',', 2 = fsep
	uint8_t quan = f->getQuantifier();
	uint32_t type = f->getType();
	const char *writevalue = 0;
	string av;
	if (!f->getJsonFunction().empty()) {
		av	= "json.put('\"');\n"
			+ f->getJsonFunction()
			+  "(json,$(field_value));\n"
			   "json.put('\"');\n";
		writevalue = av.c_str();
	} else if (f->isEnum()) {
		writevalue = 
			"if (const char *v = $(strfun)($(field_value))) {\n"
			"json.put('\"');\n"
			"json << v;\n"
			"json.put('\"');\n"
			"} else {\n"
			"json << $(field_value);\n"
			"}\n";
	} else if (type == ft_bool) {
		writevalue = "json << ($(field_value) ? \"true\" : \"false\");\n";
	} else if (f->isNumeric()) {
		if ((type == ft_float) || (type == ft_double)) {
			writevalue = "to_dblstr(json,$(field_value));\n";
		} else if ((type == ft_int8) || (type == ft_sint8) || (type == ft_sfixed8)) {
			if (optmode == optsize)
				writevalue = "to_decstr(json,(int) $(field_value));\n";
			else
				writevalue = "json << (int) $(field_value);\n";
		} else if ((type == ft_uint8) || (type == ft_fixed8)) {
			if (optmode == optsize)
				writevalue = "to_decstr(json,(unsigned) $(field_value));\n";
			else
				writevalue = "json << (unsigned) $(field_value);\n";
		} else {
			if (optmode == optsize)
				writevalue = "to_decstr(json,$field_value);\n";
			else
				writevalue = "json << $(field_value);\n";
		}
	} else if (f->isString()) {
		switch (f->getType()) {
		case ft_bytes:
			writevalue = "json_string(json,$(field_value));\n";
			break;
		case ft_string:
			writevalue = "json_cstr(json,$(field_value).c_str());\n";
			break;
		case ft_cptr:
			writevalue = "json_cstr(json,$(field_value));\n";
			break;
		default:
			abort();
		}
	} else if (f->isMessage()) {
		writevalue = "$(field_value).$(toJSON)(json,indLvl);\n";
	} else {
		abort();
	}

	switch (quan) {
		break;
	case q_optional:
		G <<	"if ($(field_has)()) {\n";
		if (fsep) {
			G <<	"$json_indent(json,indLvl,'" << fsep << "',\"$fname\");\n";
		} else {
			G <<	"fsep = $json_indent(json,indLvl,fsep,\"$fname\");\n";
		}
		G <<	writevalue
		<<	"}\n";
		break;
	case q_required:
		if (fsep) {
			G <<	"$json_indent(json,indLvl,'" << fsep << "',\"$fname\");\n";
		} else {
			G <<	"fsep = $json_indent(json,indLvl,fsep,\"$fname\");\n";
		}
		G <<	writevalue;
		break;
	case q_repeated:
		G.addVariable("index","i");
		G <<	"if (size_t s = $(field_size)) {\n";
		if (fsep) {
			G <<	"$json_indent(json,indLvl,'" << fsep << "');\n";
		} else {
			G <<	"fsep = $json_indent(json,indLvl,fsep);\n";
		}
		G <<	"indLvl += 2;\n"
			"json << \"\\\"$(fname)\\\":[\\n\";\n"
			"size_t i = 0;\n"
			"for (;;) {\n"
			"$json_indent(json,indLvl,0);\n"
			<< writevalue <<
			"++i;\n"
			"if (i == s)\n"
			"break;\n"
			"json << \",\\n\";\n"
			"}\n"
			"indLvl -= 2;\n"
			"json.put('\\n');\n"
			"$json_indent(json,indLvl,0);\n"
			"json.put(']');\n"
			"}\n";
		G.clearVariable("index");
		break;
	default:
		abort();
	}
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


void CppGenerator::writeToJson(Generator &G, Message *m)
{
	// commaFlag is needed if first element is optional and more
	// than one field is used in the message
	char fsep = 0;	// fsepmode: 0 = '{', 1 = ',', 2 = fsep
	const map<unsigned,Field *> &fields = m->getFields();
	for (auto i : fields) {
		Field *f = i.second;
		if ((f == 0) || (!f->isUsed()))
			continue;
		if (f->getQuantifier() == q_required)
			fsep = '{';
		break;
	}
	G.addVariable("json_indent",resolve_esc(target->getOption("json_indent")));
	G <<	"void $(prefix)$(msg_name)::$(toJSON)($(streamtype) &json, unsigned indLvl) const\n"
		"{\n";
	if (fsep == 0)
		G <<	"char fsep = '{';\n";
	G << "++indLvl;\n";

	for (auto i = fields.begin(), e = fields.end(); i != e; ++i) {
		Field *f = i->second;
		if ((f == 0) || (!f->isUsed()) || f->isObsolete())
			continue;
		G.setField(f);
		writeToJson(G,f,fsep);
		if (fsep)
			fsep = ',';
		G.setField(0);
	}
	if (fsep == 0)
		G <<	"if (fsep == '{')\n"
			"json.put('{');\n";
	G <<	"json.put('\\n');\n"
		"--indLvl;\n"
		"$json_indent(json,indLvl,0);\n"
		"json.put('}');\n"
		"if (indLvl == 0)\n"
		"	json.put('\\n');\n"
		"}\n"
		"\n";
	G.clearVariable("json_indent");
}


void CppGenerator::writeToX(Generator &G, Message *m)
{
	assert(G.hasValue("toX"));
	G <<	"$(sink_template)void $(prefix)$(msg_name)::$(toX)($putparam) const\n"
		"{\n";
	if (Debug)
		G << "std::cout << \"$(prefix)$(msg_name)::$(toX)($(putparam))\\n\";\n";
	for (auto i : m->getFields()) {
		Field *f = i.second;
		if ((f == 0) || (!f->isUsed()))
			continue;
		writeToX(G,f);
	}
	const string &Terminator = target->getOption("Terminator");
	if ((Terminator == "ff") || (Terminator == "0xff")) {
		if (WithComments)
			G << "// write terminating ff byte\n";
		G << "$wireput(0xff);\n";
	} else if ((Terminator == "null") || (Terminator == "0")) {
		if (WithComments)
			G << "// write terminating null byte\n";
		G << "$wireput(0);\n";
	}
	G <<	"}\n"
		"\n";
}


void CppGenerator::writeClear(Generator &G, Message *m)
{
	G <<	"void $(prefix)$(msg_name)::$(msg_clear)()\n"
		"{\n";
	for (auto i : m->getFields()) {
		Field *f = i.second;
		if ((f == 0) || (!f->isUsed()) || f->isObsolete())
			continue;
		G.setField(f);
		uint8_t q = f->getQuantifier();
		if (f->isVirtual())
			G << "$(field_clear)();\n";
		else if ((q == q_repeated) || f->isVirtual())
			G << "m_$(fname).$(msg_clear)();\n";
		else if (const char *inv = f->getInvalidValue())
			G << "m_$(fname) = " << inv << ";\n";
		else if (const char *def = f->getDefaultValue())
			G << "m_$(fname) = " << def << ";\n";
		else if (!f->hasSimpleType())
			G << "m_$(fname).$(msg_clear)();\n";
		else
			G << "m_$(fname) = 0;\n";
		G.setField(0);
	}
	G.clearVariable("fname");
	unsigned nv = m->getNumValid();
	diag("message %s: %u valid bits",m->getName().c_str(),nv);
	if (nv == 0);
	else if (nv <= VarIntBits)
		G << "p_validbits = 0;\n";
	else {
		for (unsigned n = 0; n < nv; n += 8)
			G << "p_validbits[" << n/8 << "] = 0;\n";
		if (nv%8)
			G << "p_validbits[" << (nv/8+1) << "] = 0;\n";
	}
	G <<	"}\n\n";
}


void CppGenerator::writeUnequal(Generator &G, Message *m)
{
	G <<	"bool $(prefix)$(msg_name)::operator != (const $(prefix)$(msg_name) &r) const\n"
		"{\n";
	if (optmode != optreview) {
		unsigned nv = m->getNumValid();
		if (nv == 0) {

		} else if (nv <= VarIntBits) {
			G <<	"if (p_validbits != r.p_validbits)\n"
				"	return true;\n";
		} else {
			G <<	"if (memcmp(p_validbits,r.p_validbits,sizeof(p_validbits)))\n"
				"	return true;\n";
		}
	}
	for (auto i : m->getFields()) {
		Field *f = i.second;
		if ((f == 0) || (!f->isUsed()) || f->isObsolete())
			continue;
		G.setField(f);
		quant_t q = f->getQuantifier();
		if (ft_cptr == f->getType()) {
			if (q == q_repeated) {
				G <<	"if ($(field_size) != r.$(field_size))\n"
					"	return true;\n"
					"for (size_t x = 0, y = $(field_size); x != y; ++x) {\n";
				G.addVariable("index","x");
			} else
				G.addVariable("index","");
			G <<	"if ($(field_value) != r.$(field_value)) {\n"
				"	if (($(field_value) == 0) || (r.$(field_value) == 0))\n"
				"		return true;\n"
				"	if (strcmp($(field_value),r.$(field_value)))\n"
				"		return true;\n"
				"}\n";
			if (q == q_repeated)
				G << "}\n";
			G.clearVariable("index");
		} else switch (q) {
		case q_required:
		case q_repeated:
			if (f->isVirtual()) {
				G.addVariable("index","");
				G <<	"if ($(field_value) != r.$(field_value))\n"
					"return true;\n";
				G.clearVariable("index");
			} else {
				G <<	"if (m_$(fname) != r.m_$(fname))\n"
					"return true;\n";
			}
			break;
		case q_optional:
			if (optmode == optreview) {
				G <<	"if ($(field_has)() ^ r.$(field_has)())\n"
					"return true;\n";
			}
			G <<	"if ($(field_has)() && ($(field_value) != r.$(field_value)))\n"
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


void CppGenerator::writeEqual(Generator &G, Message *m)
{
	G <<	"bool $(prefix)$(msg_name)::operator == (const $(prefix)$(msg_name) &r) const\n"
		"{\n";
	if (optmode != optreview) {
		unsigned nv = m->getNumValid();
		if (nv == 0) {

		} else if (nv <= VarIntBits) {
			G <<	"if (p_validbits != r.p_validbits)\n"
				"	return false;\n";
		} else {
			G <<	"if (memcmp(p_validbits,r.p_validbits,sizeof(p_validbits)))\n"
				"	return false;\n";
		}
	}
	for (auto i : m->getFields()) {
		Field *f = i.second;
		if ((f == 0) || (!f->isUsed()))
			continue;
		G.setField(f);
		if (f->isObsolete()) {
			if (WithComments)
				G << "// nothing to do for obsolete $fname\n";
			G.setField(0);
			continue;
		}
		quant_t q = f->getQuantifier();
		if (f->isVirtual()) {
			G <<	"if ($(field_value)() != r.$(field_value)())\n"
					"return false;\n";
		} else if (ft_cptr == f->getType()) {
			if (q == q_repeated) {
				G <<	"if ($(field_size) != r.$(field_size))\n"
					"	return false;\n"
					"for (size_t x = 0, y = $(field_size); x != y; ++x) {\n";
				G.addVariable("index","x");
			} else
				G.addVariable("index","");
			G <<	"if ($(field_value) != r.$(field_value)) {\n"
				"	if (($(field_value) == 0) || (r.$(field_value) == 0))\n"
				"		return false;\n"
				"	if (strcmp($(field_value),r.m_$(field_vlaue)))\n"
				"		return false;\n"
				"}\n";
			if (q == q_repeated)
				G << "}\n";
			G.clearVariable("index");
		} else switch (q) {
		case q_required:
		case q_repeated:
			G <<	"if (!(m_$(fname) == r.m_$(fname)))\n"
				"return false;\n";
			break;
		case q_optional:
			if (optmode == optreview) {
				G <<	"if ($(field_has)() ^ r.$(field_has)())\n"
					"return false;\n";
			}
			G <<	"if ($(field_has)() && (!(m_$(fname) == r.m_$(fname))))\n"
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


void CppGenerator::writeCmp(Generator &G, Message *m)
{
	bool wUE = target->getFlag("withUnequal");
	if (wUE)
		writeUnequal(G,m);

	if (target->getFlag("withEqual")) {
		if (wUE) {
			G <<	"bool $(prefix)$(msg_name)::operator == (const $(prefix)$(msg_name) &r) const\n"
				"{\n"
				"return !((*this) != r);\n"
				"}\n\n\n";
		} else {
			writeEqual(G,m);
		}
	}
}


void CppGenerator::writePrint(Generator &G, Field *f)
{
	G.setField(f);
	uint8_t quan = f->getQuantifier();
	uint32_t type = f->getType();
	const string &asciifun = f->getAsciiFunction();
	G.addVariable("index","");
	switch (quan) {
	case q_optional:
		if (!f->isInteger() || f->isVirtual() || !asciifun.empty())
			G <<	"$ascii_indent(o,indent,\"$fname\");\n";
		break;
	case q_required:
		if (!f->isInteger() || f->isVirtual() || !asciifun.empty())
			G <<	"$ascii_indent(o,indent,\"$fname\");\n";
		break;
	case q_repeated:
		G <<	"$ascii_indent(o,indent);\n"
			"size_t s_$(fname) = $field_size;\n"
			"o << \"$(fname)[\" << s_$fname << \"] = {\";\n"
			"++indent;\n"
			"for (size_t i = 0, e = s_$fname; i != e; ++i) {\n"
			"$ascii_indent(o,indent);\n"
			"o << i << \": \";\n"
			//"o << \"$(fname)[\" << i << \"] = \";\n"
			//"$ascii_indent(o,indent);\n"
			;
		G.setVariable("index","i");
		break;
	default:
		abort();
	}
	if (f->isVirtual()) {
		if (f->isMessage()) {
			G <<	"$(fname)_$(toASCII)(o,indent);\n"
				"o << ';';\n";
		} else
			G <<	"o << $(field_value) << ';';\n";
	} else if (!asciifun.empty()) {
		G << asciifun << "(o,$(field_value));\n";
		G << "o << ';';\n";
	} else switch (type) {
	case ft_uint8:
	case ft_fixed8:
		G << "ascii_numeric(o, indent, \"$fname\", (unsigned) $(field_value));\n";
		break;

	case ft_int8:
	case ft_sint8:
	case ft_sfixed8:
		G << "ascii_numeric(o, indent, \"$fname\", (signed) $(field_value));\n";
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
		G << "ascii_numeric(o, indent, \"$fname\", $(field_value));\n";
		break;

	case ft_bytes:
		G << target->getOption("ascii_bytes") << "(o,(const uint8_t*)$(field_value).data(),$(field_value).size(),indent+2);\no << ';';\n";
		break;
	case ft_string:
		{
			const string &filter = target->getOption("ascii_string");
			if (!filter.empty())
				G << filter << "(o,$field_value.data(),$field_value.size(),indent+2);\no << ';';\n";
			else
				G << "o << $field_value.c_str() << ';';\n";
		}
		break;
	case ft_cptr:
		{
			const string &filter = target->getOption("StringFilter");
			if (!filter.empty())
				G << "o << " << filter << "($(field_value),indent+2) << ';';\n";
			else
				G << "o << $(field_value) << ';';\n";
		}
		break;
	case ft_bool:
		G << "o << ($(field_value) ? \"true\" : \"false\") << ';';\n";
		break;

	default:
		if ((type & ft_filter) == ft_msg) {
			//if (quan != q_repeated)
				//G << ";\n";
			G << "$(field_value).$(toASCII)(o,indent);\n";
		} else if ((type & ft_filter) == ft_enum) {
			Enum *e = Enum::id2enum(type);
			assert(e);
			const string &strfun = e->getStringFunction();
			if (strfun.empty())
				G << "o << $(field_value) << ';';\n";
			else
				G << "if (const char *v = " << strfun << "($(field_value)))\n"
				       "o << v;\n"
				       "else\n"
				       "o << $field_value;\n"
				       "o << ';';\n";
		} else
			abort();
	}
	if (quan == q_repeated)
		G <<	"}\n"
			"--indent;\n"
			"$ascii_indent(o,indent);\n"
			"o << '}';\n"
			;
	G.clearVariable("index");
	G.setField(0);
}


void CppGenerator::writePrint(Generator &G, Message *m)
{
	if (hasBytes)
		G.addVariable("ascii_bytes",target->getIdentifier("ascii_bytes"));

	G <<	"void $(prefix)$(msg_name)::$(toASCII)($(streamtype) &o, size_t indent) const\n"
		"{\n"
		"o << \"$(msg_name) {\";\n"
		"++indent;\n";
	for (auto i : m->getFields()) {
		Field *f = i.second;
		if ((f == 0) || (!f->isUsed()) || f->isObsolete())
			continue;
		writePrint(G,f);
	}
	G <<	"--indent;\n"
		"$ascii_indent(o,indent);\n"
		"o << '}';\n"
		"}\n\n";

	if (hasBytes)
		G.clearVariable("ascii_bytes");
}


void CppGenerator::writeConstructor(Generator &G, Message *m)
{
	if (SubClasses)
		G << "$(prefix)$(msg_name)::$(msg_name)()\n";
	else
		G << "$(msg_name)::$(msg_name)()\n";
	writeMembers(G,m,false);
	unsigned nv = m->getNumValid();
	if (nv > VarIntBits)
		G << ", p_validbits {@}\n";
	else if (nv > 0)
		G << ", p_validbits(0)\n";
	G	<< "{\n}\n\n";
}


void CppGenerator::writeFunctions(Generator &G, Message *m)
{
	if (!m->getGenerate() && !m->isUsed())
		return;
	G.setMessage(m);
	writeStaticMembers(G,m);
	for (unsigned i = 0, n = m->numEnums(); i != n; ++i)
		writeEnumDefs(G,m->getEnum(i));
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
	
	for (auto i : m->getFields()) {
		if (Field *f = i.second)
			if (f->isUsed() && !f->isObsolete())
				writeFunctions(G,f);
	}
	writeCmp(G,m);
	writeSetByName(G,m);
	G.setMessage(0);
	for (unsigned i = 0, e = m->numMessages(); i != e; ++i)
		writeFunctions(G,m->getMessage(i));
}


void CppGenerator::writeHelpers(vector<unsigned> &funcs)
{
	// "inline" helpers
	unsigned is = target->IntSize();
	switch (is) {
	case 16:
		hasU16 |= hasUInt;
		hasS16 |= hasSInt | hasInt;
		break;
	case 32:
		hasU32 |= hasUInt;
		hasS32 |= hasSInt | hasInt;
		break;
	case 64:
		hasU64 |= hasUInt;
		hasS64 |= hasSInt | hasInt;
		break;
	default:
		abort();
	}

	// extra support functions: e.g. ascii/json
	if (WithJson) {
		funcs.push_back(ct_json_indent);
		funcs.push_back(ct_json_string);
		if (hasCStr || hasString)
			funcs.push_back(ct_json_cstr);
		funcs.push_back(ct_to_decstr);
		if (hasFloat|hasDouble)
			funcs.push_back(ct_to_dblstr);
	}

	if (PrintOut) {
		if (target->getOption("ascii_indent") == "ascii_indent")
			funcs.push_back(ct_ascii_indent);
		if (hasBytes && (target->getOption("ascii_bytes") == "ascii_bytes"))
			funcs.push_back(ct_ascii_bytes);
		if (hasString && (target->getOption("ascii_string") == "ascii_string"))
			funcs.push_back(ct_ascii_string);
		if (target->getOption("ascii_numeric") == "ascii_numeric")
			funcs.push_back(ct_ascii_numeric);
	}

	if (target->isId("SetByName")) {
		if (hasDouble)
			funcs.push_back(ct_parse_ascii_dbl);
		if (hasFloat)
			funcs.push_back(ct_parse_ascii_flt);
		if (hasU64)
			funcs.push_back(ct_parse_ascii_u64);
		if (hasU32)
			funcs.push_back(ct_parse_ascii_u32);
		if (hasU16)
			funcs.push_back(ct_parse_ascii_u16);
		if (hasU8)
			funcs.push_back(ct_parse_ascii_u8);
		if (hasS64)
			funcs.push_back(ct_parse_ascii_s64);
		if (hasS32)
			funcs.push_back(ct_parse_ascii_s32);
		if (hasS16)
			funcs.push_back(ct_parse_ascii_s16);
		if (hasS8)
			funcs.push_back(ct_parse_ascii_s8);
		if (hasBytes)
			funcs.push_back(ct_parse_ascii_bytes);
		if (hasBool)
			funcs.push_back(ct_parse_ascii_bool);
	}

	// generic functions - i.e. agnostic to wire/memory/sink mode
	if (hasVarInt || hasVarSInt)
		funcs.push_back(ct_wiresize);
	if (hasVarSInt) {
		funcs.push_back(ct_sint_varint);
		funcs.push_back(ct_varint_sint);
		funcs.push_back(ct_wiresize_s);
	}
	if (hasInt) {
		// sign extended varint
		funcs.push_back(ct_wiresize_x);
	}

	if (target->isId("toMemory")) {
		if (hasWT16)
			funcs.push_back(ct_write_u16);
		if (hasWT32 || hasFloat)
			funcs.push_back(ct_write_u32);
		if (hasWT64 || hasDouble)
			funcs.push_back(ct_write_u64);
		if (hasBytes || hasString)
			funcs.push_back(ct_encode_bytes);
	}

	if (target->isId("toMemory") || target->isId("toWire")) {
		if (hasFloat)
			funcs.push_back(ct_mangle_float);
		if (hasDouble)
			funcs.push_back(ct_mangle_double);
	}

	if (target->isId("fromMemory")) {
		funcs.push_back(ct_read_varint);
		if (((target->getOption("UnknownField") == "skip") || ((optmode != optspeed) && hasUnused)) && (!EarlyDecode))
			funcs.push_back(ct_skip_content);
		if (hasDouble && !EarlyDecode)
			funcs.push_back(ct_read_double);
		else if (hasDoubles)	// for repeated floats with early decode
			funcs.push_back(ct_read_double);
		if (hasFloat && !EarlyDecode)
			funcs.push_back(ct_read_float);
		else if (hasFloats)	// for repeated floats with early decode
			funcs.push_back(ct_read_float);
		if (hasWT64)
			funcs.push_back(ct_read_u64);
		if (hasWT32)
			funcs.push_back(ct_read_u32);
		if (hasWT16)
			funcs.push_back(ct_read_u16);
		if (EarlyDecode)
			funcs.push_back(ct_decode_early);
		if ((hasBytes || hasString) && (target->getOption("stringtype") != "C"))
			funcs.push_back(ct_decode_bytes);
		if ((hasRBytes || hasRString) && (target->getOption("stringtype") != "C"))
			funcs.push_back(ct_decode_bytes_element);
	}
	
	// mode specific implementations
	unsigned mode = 0;
	if (target->isId("toWire")) {
		funcs.push_back(gen_wire);
		mode = gen_wire;
		if (hasWT16)
			funcs.push_back(ct_send_u16);
		if (hasWT32 || hasFloat)
			funcs.push_back(ct_send_u32);
		if (hasWT64 || hasDouble)
			funcs.push_back(ct_send_u64);
	}
	if (target->isId("toString")) {
		funcs.push_back(gen_string);
		mode = gen_string;
		if (hasWT16)
			funcs.push_back(ct_send_u16);
		if (hasWT32 || hasFloat)
			funcs.push_back(ct_send_u32);
		if (hasWT64 || hasDouble)
			funcs.push_back(ct_send_u64);
		if (hasVarInt) {
			funcs.push_back(ct_send_varint);
			// send_msg after send_varint! depends on it
			funcs.push_back(ct_send_msg);
		}
		if (needSendVarSInt)
			funcs.push_back(ct_send_xvarint);
	}

	if (target->isId("toWire")) {
		if (mode != gen_wire) {
			funcs.push_back(gen_wire);
			mode = gen_wire;
		}
		if (hasVarInt)
			funcs.push_back(ct_send_varint);
		if (hasLenPfx)
			funcs.push_back(ct_send_bytes);
		if (needSendVarSInt)
			funcs.push_back(ct_send_xvarint);
	}
	if (target->isId("toMemory")) {
		if (mode != gen_wire) {
			funcs.push_back(gen_wire);
			mode = gen_wire;
		}
		if (hasVarInt)
			funcs.push_back(ct_write_varint);
		if (PaddedMsgSize)
			funcs.push_back(ct_place_varint);
		if (needSendVarSInt)
			funcs.push_back(ct_write_xvarint);
	}
}


static void writeEnumPairs(Generator &G, const Message *m)
{
	for (Enum *e : m->getEnums()) {
		for (const auto &nvp : e->getNameValuePairs())
			G << "{ \"" << nvp.first << "\", " << nvp.second << "},\n";
	}
	for (Message *sm : m->getMessages())
		writeEnumPairs(G,sm);
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
	else if ((PrintOut || target->isId("toJSON")) && (target->getOption("streamtype") == "std::ostream"))
		out << "#include <ostream>\n";
	if (hasEnums && target->getFlag("enumnames"))
		out << "#include <map>\n";
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
			"#endif\n\n\n";
	} else {
		vector<unsigned> funcs;
		writeHelpers(funcs);
		Lib.write_cpp(G,funcs,target);
	}

	if (hasEnums && target->getFlag("enumnames")) {
		// the '@' tells CompoundFold not to skip the next '{'
		G <<	"struct CStrLess {\n"
			"	bool operator () (const char *l, const char *r) const\n"
			"	{ return strcmp(l,r) < 0; }\n"
			"};\n"
			"\n"
			"static std::map<const char *,int, CStrLess> EnumsMap = {@\n";
		for (Enum *e : file->getEnums()) {
			for (const auto &nvp : e->getNameValuePairs())
				G << "{ \"" << nvp.first << "\", " << nvp.second << "},\n";
		}
		for (Message *m : file->getMessages())
			writeEnumPairs(G,m);
		G << 	"};\n"
			"\n";
		vector<unsigned> funcs;
		funcs.push_back(ct_parse_enum);
		Lib.write_h(G,funcs,target);
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
	for (Enum *e : file->getEnums())
		writeEnumDefs(G,e);
	for (unsigned i = 0, n = file->numMessages(); i != n; ++i) {
		Message *m = file->getMessage(i);
		if (m->isUsed()||m->getGenerate())
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
	if (!f->isUsed() || f->isObsolete()) {
		G << "case $(field_tag):";
		if (WithComments)
			G << "\t// $(fname) id $(field_id), type $typestr\n";
		else
			G << "\n";
		G.setField(0);
		return;
	}
	if (f->isPacked()) {
		G.setVariableHex("field_tag",(int64_t)id<<3|2);
		G <<	"case $(field_tag): {\t// $(fname) id $(field_id), packed $(typestr)[] coding 2\n"
			"varint_t v = ud.u64;\n"
			"const uint8_t *ae = a + v;\n"
			"do {\n";
		switch (type) {
		default:
		case ft_msg:
		case ft_bytes:
		case ft_string:
		case ft_cptr:
			abort();
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
			decodeVarint(G,f);
			break;
		case ft_signed:
		case ft_sint8:
		case ft_sint16:
		case ft_sint32:
		case ft_sint64:
			decodeSVarint(G,f);
			break;
		case ft_bool:
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
		G <<	"case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding byte[]\n";
		if (f->getQuantifier() == q_repeated)
			G << "$(m_field).emplace_back();\n";
		G <<	"if (((ssize_t)ud.vi > 0) && ((ssize_t)ud.vi <= (e-a))) {\n"
			"int n;\n";
		G.fillField("(const uint8_t*)a,ud.vi");
		G <<	"if (n != (ssize_t)ud.vi)\n"
			"	$handle_error;\n"
			"a += ud.vi;\n"
			"}\n";
		if (vbit != -1)
			writeSetValid(G,vbit);
		break;
	case ft_bytes:
	case ft_string:
		G <<	"case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding byte[]\n"
			"if ((ssize_t)ud.vi > e-a) {\n"
			"	$handle_error;\n"
			"}\n";
		G.fillField("(const char*)a,ud.vi");
		if (vbit != -1)
			writeSetValid(G,vbit);
		G <<	"a += ud.vi;\n";
		break;
	case ft_cptr:
		G <<	"case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding byte[]\n"
			"if ((ud.vi > 0) && (a[ud.vi-1] == 0)) {\n";
		G.fillField("(const char*)a");
		if (vbit != -1)
			writeSetValid(G,vbit);
		G <<	"a += ud.vi;\n"
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
		G.fillField("($(typestr))ud.u$varintbits");
		break;
	case ft_signed:
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding signed varint\n";
		G.fillField("varint_sint(ud.u$intsize)");
		break;
	case ft_sint8:
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding signed varint\n";
		G.fillField("varint_sint(ud.u8)");
		break;
	case ft_sint16:
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding signed varint\n";
		G.fillField("varint_sint(ud.u16)");
		break;
	case ft_sint32:
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding signed varint\n";
		G.fillField("varint_sint(ud.u32)");
		break;
	case ft_sint64:
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding signed varint\n";
		G.fillField("varint_sint(ud.u64)");
		break;
	case ft_bool:
	case ft_fixed8:
	case ft_sfixed8:
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding 8bit\n";
		G.fillField("ud.u8");
		break;
	case ft_fixed16:
	case ft_sfixed16:
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding 16bit\n";
		G.fillField("ud.u16");
		break;
	case ft_fixed32:
	case ft_sfixed32:
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding 32bit\n";
		G.fillField("ud.u32");
		break;
	case ft_float:
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding 32bit\n";
		G.fillField("ud.f");
		break;
	case ft_fixed64:
	case ft_sfixed64:
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding 64bit\n";
		G.fillField("ud.u64");
		break;
	case ft_double:
		G << "case $(field_tag):\t// $(fname) id $(field_id), type $typestr, coding 64bit\n";
		G.fillField("ud.d");
		break;
	default:
		abort();
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
	G <<	"union decode_union ud;\n"
		"ssize_t x = decode_early(a,e,&ud,&fid);\n"
		"if (x < 0)\n"
		"	$handle_error;\n"
		"a += x;\n"
		"switch (fid) {\n"
		;

	bool hasNullId = false;
	for (auto i : m->getFields()) {
		Field *f = i.second;
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
		for (auto i : m->getFields()) {
			Field *f = i.second;
			if ((f == 0) || f->isUsed() || f->isDeprecated())
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


