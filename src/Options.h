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

#ifndef OPTIONS_H
#define OPTIONS_H

#include <iosfwd>
#include <map>
#include <string>
#include <vector>
#include <strings.h>

typedef enum {
	optreview, optspeed, optsize,
} optmode_t;

typedef enum {
	unknown_endian, little_endian, big_endian
} endian_t;

typedef enum {
	bo_unset = 0,
	bo_false = 1,
	bo_true = 2,
} boolopt_t;


// member instantiation type
typedef enum {
	mem_unset = 0,
	mem_regular = 1,
	mem_virtual = 2,
	mem_static = 3,
} mem_inst_t;


struct StrCaseCmp : public std::binary_function<std::string,std::string,bool>
{
	bool operator()(const std::string &lhs, const std::string &rhs) const
	{
		return strcasecmp(lhs.c_str(), rhs.c_str()) < 0 ;
	}
};


struct CStrCaseCmp //: public std::binary_function<const char *,const char *,bool>
{
	bool operator()(const char *lhs, const char *rhs) const
	{
		return strcasecmp(lhs, rhs) < 0 ;
	}
};


bool isBinaryArg(const char *value, bool &binValue);

class KVPair;


/*
 * options hierarchy
 * field->message->..->message->target->command line->file->defaults
 */


class Options
{
	public:
	explicit Options(const std::string &src, const Options *p = 0);

	void initFieldDefaults();
	void merge(const Options &);
	void printTo(std::ostream &out, const std::string &prefix = "") const;
	void printDefines(std::ostream &out) const;
	void addOption(const char *, bool documented);
	void addOption(const char *o, const char *v, bool documented);
	void addOption(const char *o, bool f, bool documented);
	void add(KVPair *);
	void add(const std::string &, KVPair *);
	void setParent(const Options *p);
	const Options *getParent() const
	{ return m_parent; }

	bool hasFlag(const char *) const;
	bool hasOption(const char *) const;
	bool isEnabled(const char *) const;
	optmode_t OptimizationMode() const;
	endian_t Endian() const;
	const char *ClearName() const;
	const char *AddPrefix() const;
	const char *ClearPrefix() const;
	const char *GetPrefix() const;
	const char *HasPrefix() const;
	const char *MutablePrefix() const;
	const char *SetPrefix() const;
	unsigned VarIntBits() const;
	unsigned IntSize() const;
	bool StringSerialization() const;
	bool getFlag(const char *) const;
	const std::string &getOption(const char *) const;
	const char *getIdentifier(const char *) const;
	bool isId(const char *) const;
	std::string getId(const char *) const;
	const char *getSource(const char *) const;
	void setOption(const char *, const char *);

	static void printHelp(std::ostream &out);
	static Options *getDefaults();
	static Options *getFieldDefaults();

	const std::string &getName() const
	{ return m_name; }

	const std::map<std::string,KVPair*> &getNodeOptions() const
	{ return m_NodeOptions; }

	std::vector<std::string> getDeclarations() const;
	std::vector<std::string> getHeaders() const;
	std::vector<std::string> getCodeLibs() const;

	private:
	void initDefaults();
	const Options *m_parent;
	std::string m_name;
	std::map<std::string,bool,StrCaseCmp> m_BinOptions;
	std::map<std::string,std::string,StrCaseCmp> m_TextOptions;
	std::map<std::string,KVPair*> m_NodeOptions;
	std::vector<std::string> m_CodeLibs, m_Declarations, m_Headers;
};

bool isIdentifier(const char *id);

#endif
