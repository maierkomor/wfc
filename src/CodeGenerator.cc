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

#include "CodeGenerator.h"
#include "CodeLibrary.h"
#include "Generator.h"
#include "CppGenerator.h"
#include "PBFile.h"
#include "XmlGenerator.h"
#include "Options.h"
#include "log.h"
#include "io.h"
#include "version.h"

#include <assert.h>
#include <errno.h>
#include <fstream>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>


using namespace std;

static string License;


CodeGenerator::CodeGenerator(PBFile *p, Options *o)
: impl(0)
{
	const string &lang = o->getOption("lang");
	diag("lang=%s",lang.c_str());
	if ((lang == "c++") || (lang == "C++")) {
		impl = new CppGenerator(p,o);
	} else if ((lang == "xml") || (lang == "XML")) {
		impl = new XmlGenerator(p,o);
	} else
		abort();
	License = "\nCode generated with Wire-Format-Compiler " VERSION "\n\n";
	if (p) {
		License += "Source Information:\n===================\n";
		License += "Filename : ";
		License += p->getFilename();
		License += '\n';
	}
	const string &copyright = o->getOption("copyright");
	const string &author = o->getOption("author");
	const string &email = o->getOption("email");
	if (!copyright.empty()) {
		License += "Copyright: ";
		License += copyright;
		License += "\n";
	}
	if (!author.empty()) {
		License += "Author   : ";
		License += author;
		License += "\n";
	}
	if (!email.empty()) {
		License += "E-Mail   : ";
		License += email;
		License += "\n";
	}
	License += "\nCode generated on ";
	time_t t = time(0);
	char buf[64];
	strftime(buf,sizeof(buf),"%F, %H:%M:%S (",localtime(&t));
	License += buf;
	License += tzname[0];
	License += 
		").\n\n"
		"This program is free software: you can redistribute it and/or modify\n"
		"it under the terms of the GNU General Public License as published by\n"
		"the Free Software Foundation, either version 3 of the License, or\n"
		"(at your option) any later version.\n"
		"\n"
		"This program is distributed in the hope that it will be useful,\n"
		"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		"GNU General Public License for more details.\n"
		"\n"
		"You should have received a copy of the GNU General Public License\n"
		"along with this program.  If not, see <http://www.gnu.org/licenses/>.\n"
		"\n";
	assert(impl);
	impl->setLicense(License.c_str());
}


CodeGeneratorImpl::~CodeGeneratorImpl()
{

}


void CodeGenerator::init()
{
	assert(impl);
	impl->init();
}


void CodeGenerator::writeLib(const char *basename)
{
	assert(impl);
	if (basename == 0)
		impl->writeLib();
	else if (isDir(basename)) {
		char cwd[PATH_MAX];
		if (0 == getcwd(cwd,sizeof(cwd))) {
			error("unable to determine current work directory: %s",strerror(errno));
			return;
		}
		if (0 == chdir(basename))
			impl->writeLib();
		else
			error("unable to cd to %s: %s",basename,strerror(errno));
		if (-1 == chdir(cwd))
			warn("unable to change directory back to %s: %s",cwd,strerror(errno));
	} else
		error("Unable to change the name of the core library. Target directory must exist.");
}


void CodeGenerator::writeFiles(const char *basename)
{
	assert(impl);
	if (isDir(basename)) {
		string bn = basename;
		if (bn[bn.size()-1] != FILESEP)
			bn += FILESEP;
		const char *pbname = impl->getPBFile()->getFilename().c_str();
		const char *lfs = strrchr(pbname,FILESEP);
		if (lfs)
			pbname = lfs+1;
		bn += pbname;
		bn.resize(bn.size()-4);
		impl->writeFiles(bn.c_str());
	} else {
		impl->writeFiles(basename);
	}
}


void CodeGenerator::setTarget(const char *target)
{
	assert(impl);
	impl->setTarget(target);
}


void PostProcess(const char *ifn, class Options *o)
{
	size_t l = strlen(ifn);
	if (strcmp(ifn+l-4,".cct")) {
		error("file for post processing is expected to have .cct suffix");
		return;
	}
	char *buf = readFile(ifn);
	if (buf == 0)
		return;
	string ofn = ifn;
	ofn.resize(l-1);
	fstream out;
	out.open(ofn.c_str(),fstream::out);
	const vector<string> &includes = o->getCodeLibs();
	for (size_t x = 0, n = includes.size(); x < n; ++x) 
		Lib.add(includes[x].c_str());
	Generator G(out,o);
	G.setVariable("inline","");
	string wireput = o->getOption("wireput");
	if (!toIdentifier(wireput)) {
		G.setVariable("putparam","void (*put)(uint8_t)");
		G.setVariable("put","put");
	} else {
		G.setVariable("putparam","");
		G.setVariable("put",o->getOption("wireput"));
	}
	G << buf;
	free(buf);
}

