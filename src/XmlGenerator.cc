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

#include "XmlGenerator.h"
#include "PBFile.h"
#include "Options.h"
#include "Enum.h"
#include "Field.h"
#include "Message.h"
#include "log.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fstream>

using namespace std;


XmlGenerator::XmlGenerator(class PBFile *f, class Options *o)
: CodeGeneratorImpl()
, file(f)
, target(o)
{

}


void XmlGenerator::init()
{
}

void XmlGenerator::setTarget(const char *target)
{

}


void XmlGenerator::setLicense(const char *t)
{
	license =
		"<!--\n";
	const char *nl = strchr(t,'\n');
	while (nl) {
		license += " -- ";
		license += string(t,nl+1);
		t = nl + 1;
		nl = strchr(t,'\n');
	}
	if (*t) {
		license += " -- ";
		license += t;
	}
	license +=
		" -->\n"
		"\n"
		"\n";

}


static string indent(unsigned i)
{
	static const char Indent[] = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
	if (i <= sizeof(Indent))
		return string(Indent,i);
	string ret = Indent;
	i -= sizeof(Indent);
	do {
		ret += '\t';
	} while (--i);
	return ret;
}


static void writeEnum(fstream &out, Enum *e, unsigned ind)
{
	out << indent(ind) << "<Enum name=\"" << e->getName() << "\">\n";
	++ind;
	const map<string,int64_t> &nvp = e->getNameValuePairs();
	for (auto i = nvp.begin(), e = nvp.end(); i != e; ++i)
		out << indent(ind) << "<EnumValue name=\"" << i->first << "\" value=\"" << i->second << "\"/>\n";
	--ind;
	out << indent(ind) << "</Enum>\n";
}


static void writeField(fstream &out, Field *f, unsigned ind)
{
	if (f == 0)
		return;
	quant_t q = f->getQuantifier();
	const char *quant = 0;
	switch (q) {
	case q_optional:
		quant = "optional";
		break;
	case q_required:
		quant = "required";
		break;
	case q_repeated:
		quant = "repeated";
		break;
	default:
		abort();
	}
	const char *defV = f->getDefaultValue();
	const char *tn = f->getTypeName();
	assert(tn);
	out	<< indent(ind) << "<Field" 
		<< " name=\"" << f->getName() << "\""
		<< " quantifier=\"" << quant << "\""
		<< " type=\"" << tn << "\""
		<< " id=\"" << f->getId() << "\"";
	if (defV && defV[0])
		out << " default=\"" << defV << "\"";
	out	<< "/>\n";
}


static void writeMessage(fstream &out, Message *m, unsigned ind)
{
	out << indent(ind) << "<Message name=\"" << m->getName() << "\">\n";
	++ind;
	for (unsigned i = 0, ne = m->numEnums(); i < ne; ++i)
		writeEnum(out,m->getEnum(i),ind);
	for (unsigned i = 0, n = m->numMessages(); i != n; ++i) {
		out << '\n';
		writeMessage(out,m->getMessage(i),ind);
	}
	const map<unsigned,Field *> &fields = m->getFields();
	for (auto i = fields.begin(), e = fields.end(); i != e; ++i)
		writeField(out,i->second,ind);
	--ind;
	out << indent(ind) << "</Message>\n";
}


void XmlGenerator::writeFiles(const char *basename)
{
	string fn = basename;
	fn += ".xml";
	msg("generating %s",fn.c_str());
	fstream out;
	out.open(fn.c_str(),fstream::out);
	out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	out << license;
	// TODO: add XML-Schema
	for (unsigned i = 0, n = file->numEnums(); i != n; ++i)
		writeEnum(out,file->getEnum(i),0);
	for (unsigned i = 0, n = file->numMessages(); i != n; ++i)
		writeMessage(out,file->getMessage(i),0);
	
	if (hadError())
		unlink(fn.c_str());
}


void XmlGenerator::writeLib()
{

}
