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

#include "Indenter.h"
#include "log.h"

#include <assert.h>
#include <string.h>
#include <iostream>

using namespace std;


struct lineinfo
{
	unsigned bro = 0, brc = 0;
	bool has_case = false, has_default = false
		, has_do = false, has_else = false, has_for = false, has_if = false
		, has_switch = false, has_while = false, has_semi = false;
};


class Indenter
{
	public:
	Indenter(ostream &o, const char *str)
	: out(o)
	, input(str)
	, line(str)
	, at(str)
	, nl(0)
	{ }

	void process();

	private:
	void scan_line(lineinfo &);

	ostream &out;
	const char *input, *line, *at, *nl;
	size_t len = 0;
};


static bool end_of_word(const char *at)
{
	switch (*at) {
	case ' ':
	case '\n':
	case '\t':
	case '(':
	case '{':
	case ':':
	case ';':
		return true;
	default:
		return false;
	}
}


void Indenter::scan_line(lineinfo &li)
{
	bool has_char = false, has_arg = false;
	//const char *at = line;
	unsigned plvl = 0;	// level of parantheses
	for (;;) {
		char c = *at++;
		switch (c) {
		case 'c':
			if (!has_char) {
				if ((0 == memcmp(at,"ase",3)) && end_of_word(at+3)) {
					li.has_case = true;
					at += 3;
				}
				has_char = true;
			}
			break;
		case 'd':
			if (!has_char) {
				if ((at[0] == 'o') && end_of_word(at+1)) {
					li.has_do = true;
					++at;
				} else if ((0 == memcmp(at,"efault",6)) && end_of_word(at+6)) {
					li.has_default = true;
					at += 6;
				}
				has_char = true;
			}
			break;
		case 'e':
			if (!has_char) {
				if ((0 == memcmp(at,"lse",3)) && end_of_word(at+3)) {
					li.has_else = true;
					at += 3;
				}
				has_char = true;
			}
			break;
		case 'f':
			if (!has_char) {
				if ((at[0] == 'o') && (at[1] == 'r') && end_of_word(at+2)) {
					li.has_for = true;
					at += 2;
				}
			}
			has_char = true;
			break;
		case 'i':
			if (!has_char && (at[0] == 'f') && end_of_word(at+1)) {
				li.has_if = true;
				++at;
			}
			has_char = true;
			break;
		case 's':
			if (!has_char) {
				if ((0 == memcmp(at,"witch",5)) && end_of_word(at+5)) {
					li.has_switch = true;
					at += 5;
				}
				has_char = true;
			}
			break;
		case 'w':
			if (!has_char) {
				if ((0 == memcmp(at,"hile",4)) && end_of_word(at+4)) {
					li.has_while = true;
					at += 4;
				}
				has_char = true;
			}
			break;
		case '"':
			while (*at != '"') {
				if (*at == '\\')
					++at;
				else if (*at == '\n')
					break;
				++at;
			}
			++at;
			break;
		case '\'':
			if (*at == '\\')
				++at;
#ifdef DEVEL
			++at;
			assert(*at == '\\');
			++at;
#else
			at += 2;
#endif
			break;
		case '(':
			++plvl;
			break;
		case ')':
			if (0 == --plvl)
				has_arg = true;
			break;
		case ';':
			if (has_arg) {
				if (li.has_for)
					li.has_for = false;
				else if (li.has_if)
					li.has_if = false;
				if (li.has_while)
					li.has_semi = true;
			}
			break;
		case '{':
			++li.bro;
			break;
		case '}':
			if (li.bro)
				--li.bro;
			else
				++li.brc;
			break;
		case 0:
		case '\n':
			nl = at - 1;
			len = nl - line;
			return;
		default:
			;
		}
	}
}

static void writeIndent(ostream &out, int l)
{
	if (l < 0) {
#if defined TEST 
		abort();
#elif defined DEVEL
		ICE("indention failure on:\n>>>\n%s\n<<<",str);
#else
		warn("indention error (level %d)",l);
		return;
#endif
	}
	while (l--)
		out << '\t';
}


void Indenter::process()
{
	int indLvl = 0, indOnce = 0;
	bool had_do = false;
	for (;;) {
		// skip leading white-space
		if (indLvl > 0)
			while ((*at == ' ') || (*at == '\t'))
				++at;
		line = at;
		lineinfo li;
		scan_line(li);

		// adjust this line
		int adjust = 0;
		if (li.has_while && had_do)
			adjust = -1;
		else if (li.has_default||li.has_case)
			adjust = -1;
		if (li.brc)
			indLvl -= li.brc;
#if defined TEST || defined DEBUG
		assert(indLvl >= 0);	
#else
		if (indLvl < 0)
			indLvl = 0;
#endif

		writeIndent(out,indLvl+indOnce+adjust);
		if (*nl == 0) {
			out.write(line,nl-line);
			return;
		}
		out.write(line,nl-line+1);

		// preperation for next line
		if (li.bro) {
			indLvl += li.bro;
		} else if (li.has_do | li.has_else | li.has_if | li.has_for) {
			++indOnce;
		} else if (li.has_while && !had_do && !li.has_semi) {
			++indOnce;
		} else
			indOnce = 0;
		had_do = li.has_do;

		// start next line
	}
}


void indent_code(ostream &out, const char *str)
{
	Indenter I(out,str);
	I.process();
}


#ifdef TEST
#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

const char *text = 
	"while(1);\n"
	"void func()\n"
	"{\n"
	"unsigned i;\n"
	"while (d != e) {\n"
	"for (unsigned i = 0; i < n; ++i)\n"
	"put((uint8_t)*d++);\n"
	"}\n"
	"}\n"
	"\n"
	"void a()\n"
	"{\n"
	"switch(x) {\n"
	"case 'a':\n"
	"sth();\n"
	"more();\n"
	"break;\n"
	"case 'b':\n"
	"nsth();\n"
	"anymore();\n"
	"strcmp(bla,blub);\n"
	"break;\n"
	"default:\n"
	"null();\n"
	"}\n"
	"if (x) {\n"
	"xx \n"
	"} else {\n"
	"xy\n"
	"}\n"
	"while (a)\n"
	"++a;\n"
	"while (b) {\n"
	"++b;\n"
	"}\n"
	"if (bla)\n"
	"blub();\n"
	"if (f()) {\n"
	"nothing();\n"
	"} else\n"
	"ready();\n"
	"do{\n"
	"do something();\n"
	"while(anything);\n"
	"++x;\n"
	"}while(y());\n"
	"if (a)\n"
	"if (b)\n"
	"if (c)\n"
	"++x;\n"
	"else\n"
	"++y;\n"
	"if (x) {\n"
	" int x;\n"
	"if (y){\n" 
	" xy\n"
	"}\n"
	"}\n"
	"}\n";

int main(int argc, char *argv[])
{
	if (argc == 2) {
		int fd = open(argv[1],O_RDONLY);
		struct stat st;
		fstat(fd,&st);
		char *buf = (char*)malloc(st.st_size+1);
		buf[st.st_size] = 0;
		int n = read(fd,buf,st.st_size);
		close(fd);
		indent_code(cout,buf);
		return 0;
	}
	string str = text;
	cout << "in:\n" << text << "\n\n";
	cout << "out:\n";
	indent_code(cout,str.c_str());
}
#endif
