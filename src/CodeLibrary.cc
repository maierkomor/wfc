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

#include "CodeLibrary.h"
#include "CodeTemplate.h"
#include "Generator.h"
#include "Options.h"
#include "log.h"
#include "io.h"

#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include <sstream>

using namespace std;

CodeLibrary Lib;


static inline char *skip_ws(char *x)
{
	while ((*x == ' ') || (*x == '\t') || (*x == '\n') || (*x == '\r'))
		++x;
	return x;
}


static bool isValidName(const char *n)
{
	if (!isalpha(*n))
		return false;
	while (isalnum(*++n)||(*n == '_'));
	while ((*n == ' ') || (*n == '\t'))
		++n;
	if (*n == '\n')
		return true;
	return false;
}


CodeLibrary::CodeLibrary()
{
}


void CodeLibrary::add(const char *entry)
{
	struct stat st;
	if (-1 == stat(entry,&st)) {
		warn("unable to include %s: %s",entry,strerror(errno));
		return;
	}
	if (!S_ISDIR(st.st_mode)) {
		addFile(entry,&st);
		return;
	}
	diag("including directory %s",entry);
	DIR *d = opendir(entry);
	if (d == 0) {
		warn("unable to include directory %s: %s",entry,strerror(errno));
		return;
	}
	struct dirent *de = readdir(d);
	while (de) {
		size_t l = strlen(de->d_name);
		if (memcmp(".cct",de->d_name+l-4,4) && memcmp(".cc",de->d_name+l-3,3)) {
			de = readdir(d);
			continue;
		}
		string fn = entry;
		fn += '/';
		fn += de->d_name;
		addFile(fn.c_str(),0);
		de = readdir(d);
	}
}


void CodeLibrary::addFile(const char *filename, struct stat *st)
{
	size_t fnl = strlen(filename)+1;
	char fn[fnl];
	memcpy(fn,filename,fnl);
	foldPath(fn);
	if (!m_files.insert(fn).second) {
		diag("ignoring duplicate include file %s",fn);
		return;
	}
	diag("including %s",fn);
	int fd = open(fn,O_RDONLY);
	if (fd == -1) {
		fatal("unable to open file %s: %s",fn,strerror(errno));
		return;
	}
	size_t size;
	if (st == 0) {
		struct stat s;
		if (-1 == fstat(fd,&s)) {
			warn("unable to stat file %s: %s",fn,strerror(errno));
			close(fd);
			return;
		}
		size = s.st_size;
	} else
		size = st->st_size;
	char *buf = (char *)malloc(size+1);
	buf[size] = 0;
	if (-1 == read(fd,buf,size)) {
		warn("unable to read file %s: %s",fn,strerror(errno));
		close(fd);
		return;
	}
	addBuf(buf,fn);
	free(buf);
	close(fd);
}


void CodeLibrary::addBuf(char *buf, const char *fn)
{
	char *at = buf;
	while (at) {
		char *slash = strchr(at,'/');
		while (slash) {
			if (slash[1] == '*')
				break;
			slash = strchr(slash+1,'/');
		}
		if (slash == 0)
			return;
		char *x = skip_ws(slash + 2);
		if (memcmp(x,"wfc-template:",13)) {
			at = x;
			continue;
		}
		char *eoc = strstr(x+14,"*/");
		if (eoc == 0) {
			warn("file %s: unterminated comment",fn);
			return;
		}
		*eoc = 0;
		char *f = strstr(slash,"function:");
		*eoc = '*';
		if (f == 0) {
			warn("file %s: code template missing function name",fn);
			at = eoc + 2;
			continue;
		}
		f = skip_ws(f+9);
		if (!isValidName(f)) {
			char *sp = strchr(f,' ');
			if (sp)
				*sp = 0;
			char *nl = strchr(f,'\n');
			if (nl)
				*nl = 0;
			warn("file %s: code template has invalid function name %s",fn,f);
			at = eoc + 2;
			continue;
		}
		char *z = eoc + 2;
		char *define = strstr(eoc,"#define");
		char *ifdef = strstr(eoc,"#if");
		char *c = strchr(z,'{');
		if (ifdef && c) {
			if (ifdef < c)
				c = 0;
			else
				ifdef = 0;
		}
		if (define && c) {
			if (define < c)
				c = 0;
			else
				define = 0;
		}
		if (ifdef && define) {
			if (define < ifdef)
				ifdef = 0;
			else
				define = 0;
		}
		char *eofunc = 0;
		char *p = strchr(eoc+2,'(');
		if (ifdef) {
			char *endif = strstr(ifdef+3,"#endif");
			if (endif == 0) {
				warn("#if missing #endif");
				return;
			}
			eofunc = strchr(endif,'\n');
			if (eofunc == 0)
				eofunc = endif+6;
		} else if (define) {
			eofunc = define+5;
			do {
				eofunc = strchr(eofunc,'\n');
			} while (eofunc && (eofunc[-1] == '\\'));
		} else if (c) {
			while (p && p[-1] == '$')	// skip over leading $(inline)
				p = strchr(p+1,'(');
			/*
			if ((p == 0) || (p > c)) {
				warn("file %s: template without parameter list",fn);
				at = eoc + 2;
				continue;
			}
			*/
			unsigned br = 1;
			do {
				++c;
				if (*c == '{')
					++br;
				else if (*c == '}')
					--br;
				else if (*c == 0) {
					warn("file %s: template with incomplete body",fn);
					return;
				}
			} while (br);
			eofunc = c + 1;
		}
		if (eofunc == 0) {
			warn("file %s: template without body",fn);
			return;
		}

		CodeTemplate *ct = new CodeTemplate(f,slash,eoc,eofunc);
		if (ct->getVariant().empty()) {
			error("parser error for function %s: tag not found");
			delete ct;
			at = eofunc;
			continue;
		}
		ct->setFilename(fn);
		codeid_t id = ct->getFunctionId();
		if (id != ct_invalid) {
			while (isspace(p[-1]))
				--p;
			char *v = p-1;
			while (isalnum(v[-1])||(v[-1] == '_'))
				--v;
			string variant = string(v,p);
			auto i = m_variants.insert(make_pair(variant,fn));
			if (i.second) {
				m_templates.insert(make_pair(id,ct));
			} else {
				warn("ignorin duplicate variant %s of function %s from file %s, taking %s",variant.c_str(),Functions[id],fn,i.first->second.c_str());
				delete ct;
			}
		} else {
			warn("ignoring template for unknown function %s in file %s",ct->getVariant().c_str(),fn);
			delete ct;
		}
		at = eofunc;
	}
}


/*
string CodeLibrary::getComment(const char *f, const Options *o) const
{
	//auto p = m_templates.equal_range(f);
	auto p = m_templates.equal_range((codeid_t)(f-Functions[0]));	// BUG!!!

	while (p.first != p.second) {
		CodeTemplate *t = p.first->second;
		if (t->fitsOptions(o)) 
			return t->getComment();
		++p.first;
	}
	return "";
}
*/


CodeTemplate *CodeLibrary::getTemplate(codeid_t f, const Options *o) const
{
	auto p = m_templates.equal_range(f);

	while (p.first != p.second) {
		CodeTemplate *t = p.first->second;
		dbug("trying variant %s",t->getVariant().c_str());
		if (t->fitsOptions(o)) {
			diag("using variant %s for function %s",t->getVariant().c_str(),Functions[f]);
			return t;
		}
		++p.first;
	}
	diag("no library variant available for function %s",Functions[f]);
	return 0;
}


void CodeLibrary::write_includes(Generator &G, const vector<unsigned> &funcs, const Options *options) const
{
	set<string> includes, sysincludes;
	for (auto i = funcs.begin(), e = funcs.end(); i != e; ++i) {
		codeid_t id = (codeid_t)*i;
		if (id & 0x10000)
			continue;
		CodeTemplate *t = getTemplate(id,options);
		if (t == 0)
			continue;
		const vector<string> &c_inc = t->getIncludes();
		includes.insert(c_inc.begin(),c_inc.end());
		const vector<string> &c_sinc = t->getSysincludes();
		sysincludes.insert(c_sinc.begin(),c_sinc.end());
	}
	for (auto i = sysincludes.begin(), e = sysincludes.end(); i != e; ++i)
		G << "#include <" << *i << ">\n";
	if (!sysincludes.empty())
		G << '\n';
	for (auto i = includes.begin(), e = includes.end(); i != e; ++i)
		G << "#include \"" << *i << "\"\n";
	if (!includes.empty())
		G << '\n';
}


void CodeLibrary::write_h(Generator &G, const vector<unsigned> &funcs, const Options *options) const
{
	const string &wfclib = options->getOption("wfclib");
	libmode_t lm;
	if (wfclib == "inline")
		lm = libinline;
	else if (wfclib == "static")
		lm = libstatic;
	else if (wfclib == "extern")
		lm = libextern;
	else
		abort();
	diag("writing header of helper functions");
	for (auto i = funcs.begin(), e = funcs.end(); i != e; ++i) {
		unsigned f = *i;
		if (f & 0x10000) {
			dbug("switching generation mode to %x",f);
			G.setMode((genmode_t)f);
			continue;
		}
		CodeTemplate *t = getTemplate((codeid_t)f,options);
		dbug("generating declaration for %s",Functions[f]);
		if (t == 0) {
			G << "/* unable to find implementation for function " << Functions[f] << "*/\n";
			warn("unable to find implementation for function %s",Functions[f]);
			continue;
		}
		t->write_h(G,lm);
	}
}


void CodeLibrary::write_cpp(Generator &G, const vector<unsigned> &funcs, const Options *options) const
{
	write_includes(G,funcs,options);
	const string &wfclib = options->getOption("wfclib");
	libmode_t lm;
	if (wfclib == "inline")
		lm = libinline;
	else if (wfclib == "static")
		lm = libstatic;
	else if (wfclib == "extern")
		lm = libextern;
	else
		abort();
	diag("writing body of helper functions");
	for (auto i = funcs.begin(), e = funcs.end(); i != e; ++i) {
		unsigned f = *i;
		if (f & 0x10000) {
			G.setMode((genmode_t)f);
			continue;
		}
		CodeTemplate *t = getTemplate((codeid_t)f,options);
		if (t == 0) {
			G << "/* unable to find implementation for function " << Functions[f] << " */\n";
			error("unable to find implementation for function %s",Functions[f]);
			continue;
		}
		t->write_cpp(G,lm);
	}
}


