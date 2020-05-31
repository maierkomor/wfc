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

#ifndef PBFILE_H
#define PBFILE_H

#include <map>
#include <string>
#include <vector>

class Message;
class Enum;
class Options;
class Target;

typedef enum {
	gprotov2, gprotov3, xprotov1
} protov_t;


class PBFile
{
	public:
	PBFile(const char *);
	~PBFile();

	void addMessage(Message *m);
	Message *getMessage(const char *n) const;
	void setOption(const char *o, unsigned, const char *v, unsigned);
	const std::string &getOption(const char *) const;

	const std::vector<Message *> &getMessages() const
	{ return messages; }

	const std::string &getFilename() const
	{ return filename; }

	void addEnum(Enum *e)
	{ enums.push_back(e); }

	unsigned numEnums() const
	{ return enums.size(); }

	Enum *getEnum(unsigned i) const
	{ return enums[i]; }

	Enum *getEnum(const char *) const;

	const std::vector<Enum *> &getEnums() const
	{ return enums; }

	unsigned numMessages() const
	{ return messages.size(); }

	Message *getMessage(unsigned i) const
	{ return messages[i]; }

	void needArrays()
	{ withArrays = true; }

	void needVectors()
	{ withVectors = true; }

	bool usesVectors() const
	{ return withVectors; }

	bool usesArrays() const
	{ return withArrays; }

	Options *getOptions(const std::string &n = "");

	void addOptions(Options *o);
	//void addTarget(Target *t);
	Options *getTarget(const char *t, unsigned tl) const;

	protov_t ProtocolVersion() const
	{ return protocolVersion; }

	private:
	PBFile(const PBFile &);
	PBFile& operator = (const PBFile &);

	std::string filename;
	std::vector<Message *> messages;
	std::vector<Options *> options;
	std::vector<Enum *> enums;
	std::string target;
	protov_t protocolVersion;
	bool withArrays, withVectors;
};


#endif
