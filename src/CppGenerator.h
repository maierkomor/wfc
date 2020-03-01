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

#ifndef CPPGENERATOR_H
#define CPPGENERATOR_H

#include "CodeGenerator.h"
#include "codeid.h"
#include "Field.h"
#include <iosfwd>
#include <string>
#include <vector>

class Generator;
class Options;
class PBFile;
class KVPair;

class CppGenerator : public CodeGeneratorImpl
{
	public:
	CppGenerator(PBFile *p, Options *);

	void init(const std::vector<std::string> &);
	void setTarget(const char *t = "");
	void writeLib();
	void writeFiles(const char * = 0);
	void setLicense(const char *l);
	PBFile *getPBFile() const
	{ return file; }

	private:
	void applyNodeOption(const char *nodepath, KVPair *kvp);
	void initNames(Message *m, const std::string &prefix = "");
	void initVBits(Message *m);
	void scanRequirements(Message *m);
	void writeHeader(const std::string &fn);
	void writeBody(const std::string &fn);
	void writeInfos(std::ostream &);

	void writeClass(Generator &, Message *);
	void writeEnumDecl(Generator &, Enum *, bool = false);
	void writeEnumDefs(Generator &, Enum *);
	void writeHelpers(std::vector<unsigned> &);
	void writeInlineHelpers(Generator &, std::vector<unsigned> &);
	//void writeInlineHelpers(Generator &);

	bool writeMember(Generator &, Field *,bool,bool = false);
	void writeCalcSize(Generator &out, Field *f);
	void writeCalcSize(Generator &out, Message *m);
	void writeClear(Generator &out, Field *f);
	void writeClear(Generator &out, Message *m);
	void writeCmp(Generator &G, Message *m);
	void writeConstructor(Generator &G, Message *m);
	void writeEqual(Generator &G, Message *m);
	void writeFromMemory_early(Generator &out, Field *f);
	void writeFromMemory_early(Generator &out, Message *m);
	void writeFromMemory(Generator &out, Field *f);
	void writeFromMemory(Generator &out, Message *m);
	void writeFunctions(Generator &G, Message *m);
	void writeFunctions(Generator &out, Field *f);
	void writeGet(Generator &out, Field *f);
	void writeHas(Generator &, Field *f);
	void writeHeaderDecls(Generator &, Field *);
	void writeInlines(Generator &out, Field *f);
	void writeInlines(Generator &out, Message *m);
	void writeMaxSize(Generator &, Message *m);
	void writeMembers(Generator &G, Message *m,bool);
	void writeMutable(Generator &out, Field *f);
	void writePrint(Generator &out, Field *f);
	void writePrint(Generator &out, Message *m);
	void writeReaders(Generator &G, optmode_t optmode);
	void writeSet(Generator &out, Field *f);
	void writeSetByNameR(Generator &G, Field *f);
	void writeSetByName(Generator &G, Message *m);
	void writeSize(Generator &, Field *f);
	void writeStaticMember(Generator &G, Field *f, const char *n);
	void writeStaticMembers(Generator &G, Message *m);
	void writeTagToMemory(Generator &out, Field *f);
	void writeTagToX(Generator &out, Field *f);
	void writeToJson(Generator &out, Field *f, char fsep);
	void writeToJson(Generator &out, Message *m);
	void writeToMemory(Generator &out, Field *f);
	void writeToMemory(Generator &out, Message *m);
	void writeToX(Generator &out, Field *f);
	void writeToX(Generator &out, Message *m);
	void writeUnequal(Generator &G, Message *m);

	std::string getValid(Field *f, bool invalid = false);
	const char *setValid(int vbit, unsigned numvalid);
	const char *clearValid(int vbit, unsigned numvalid);
	void writeGetValid(Generator &out, Field *,bool = false);
	void writeSetValid(Generator &out, int vbit);
	void writeClearValid(Generator &out, int vbit);

	void decodeByteArray(Generator &G, Field *f);
	void decodeMessage(Generator &G, Field *f);
	void decodeSVarint(Generator &G, Field *f);
	void decodeVarint(Generator &G, Field *f);
	void decode64bit(Generator &G, Field *f);
	void decode32bit(Generator &G, Field *f);
	void decode16bit(Generator &G, Field *f);
	void decode8bit(Generator &G, Field *f);

	PBFile *file;
	Options *target;
	Options *clOptions;	// command line options to override target settings
	optmode_t optmode;
	endian_t Endian;
	bool usesArrays, usesVectors, usesCStrings, usesStringTypes, usesBytes,
	     Asserts, Debug, PrintOut, SubClasses, Checks, PaddedMsgSize, SinkToTemplate,
	     WithJson, EarlyDecode,
	     inlineClear, inlineHas, inlineGet, inlineMaxSize, inlineSet, inlineSize,
	     hasVarInt, hasVarSInt, hasInt, hasSInt, hasUInt, hasCStr,
	     hasBool, hasFloat, hasDouble, 
	     hasS8, hasS16, hasS32, hasS64, hasU8, hasU16, hasU32, hasU64,
	     hasWT8, hasWT16, hasWT32, hasWT64,
	     hasBytes, hasString, hasLenPfx, hasUnused,
	     needJsonString, needCalcSize, needSendVarSInt;
	unsigned VarIntBits, WireputArg;
	std::string ErrorHandling, license;
};

#endif

