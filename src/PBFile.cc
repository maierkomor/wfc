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

#include "PBFile.h"
#include "Enum.h"
#include "Message.h"
#include "Options.h"
#include "log.h"
#include <string.h>

#include <cassert>

using namespace std;


PBFile::PBFile(const char *fn)
: filename(fn)
, protocolVersion(xprotov1)
, withArrays(false)
, withVectors(false)
{ 
	options.push_back(new Options(string("file:")+fn,Options::getDefaults()));
}


PBFile::~PBFile()
{
	for (size_t i = 0, s = options.size(); i != s; ++i)
		delete options[i];
}


void PBFile::addMessage(Message *m)
{
	const char *name = m->getName().c_str();
	diag("%s: adding message %s",filename.c_str(),name);
	for (size_t x = 0, y = messages.size(); x != y; ++x) {
		if (0 == strcmp(name,messages[x]->getName().c_str()))
			error("duplicate message name '%s'",name);
	}
	messages.push_back(m);
	if (!withVectors)
		withVectors = m->usesVectors();
}


Enum *PBFile::getEnum(const char *n) const
{
	for (vector<Enum*>::const_iterator i(enums.begin()), e(enums.end()); i != e; ++i) {
		Enum *en = *i;
		if (en->getName() == n)
			return en;
	}
	return 0;
}


Message *PBFile::getMessage(const char *n) const
{
	for (size_t x = 0, y = messages.size(); x != y; ++x) {
		if (0 == strcmp(n,messages[x]->getName().c_str()))
			return messages[x];
	}
	return 0;
}


Options *PBFile::getOptions(const string &n)
{
	if (n == "")
		return options[0];
	for (size_t i = 1, s = options.size(); i != s; ++i)
		if (options[i]->getName() == n)
			return options[i];
	error("unknown target options '%s'",n.c_str());
	Options *o = new Options(n,Options::getDefaults());
	options.push_back(o);
	return o;
}


/*
const string &PBFile::getOption(const char *opt) const
{
	return options->getOption(opt);
}


bool PBFile::usesStringSerialization() const
{
	return options->getFlag("StringSerialization");
}
*/


void PBFile::setOption(const char *k, unsigned kl, const char *v, unsigned vl)
{
	string key(k,k+kl), value(v,v+vl);
	options[0]->addOption(key.c_str(),value.c_str(),false);
}


void PBFile::addOptions(Options *o)
{
	options.push_back(o);
}


Options *PBFile::getTarget(const char *t, unsigned tl) const
{
	string target(t,tl);
	for (size_t i = 1, s = options.size(); i != s; ++i)
		if (options[i]->getName() == target)
			return options[i];
	error("request for unknown target %s",target.c_str());
	return options[0];
}
