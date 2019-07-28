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

#include "keywords.h"

#include <set>
#include <string.h>

using namespace std;

static const char *KeywordsA[] = {
	"alignas",
	"alignof",
	"and",
	"asm",
	"auto",
	"break",
	"case",
	"catch",
	"char",
	"class",
	"const",
	"const_cast",
	"constexpr",
	"continue",
	"decltype",
	"default",
	"delete",
	"do",
	"double",
	"dynamic_cast",
	"else",
	"explicit",
	"extern",
	"false",
	"float",
	"for",
	"friend",
	"goto",
	"if",
	"inline",
	"long",
	"mutable",
	"namespace",
	"new",
	"nullptr",
	"or",
	"private",
	"protected",
	"public",
	"register",
	"reinterpret_cast",
	"restrict",
	"return",
	"short",
	"sizeof",
	"static",
	"static_assert",
	"static_cast",
	"struct",
	"switch",
	"template",
	"this",
	"throw",
	"true",
	"typedef",
	"typeid",
	"typename",
	"union",
	"using",
	"virtual",
	"void",
	"volatile",
	"while",
	"xor",
};


struct CStrLess
{
	bool operator () (const char *l, const char *r)
	{ return 0 > strcmp(l,r); }
};


static set<const char *, CStrLess> Keywords;
static set<const char *, CStrLess>::const_iterator init_keywords();
static set<const char *, CStrLess>::const_iterator KWE = init_keywords();


static set<const char *, CStrLess>::const_iterator init_keywords()
{
	for (size_t i = 0; i < sizeof(KeywordsA)/sizeof(KeywordsA[0]); ++i)
		Keywords.insert(KeywordsA[i]);
	return Keywords.end();
}


bool isKeyword(const char *s)
{
	return (Keywords.find(s) != KWE);
}


