%pointer
%e 150
%p 400
%n 80
%k 80
%a 250
%o 500
%option bison-bridge
%option bison-locations
%x COMMENT

%{

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

#include "ProtoDriver.h"
#include "PBFile.h"
#include "xproto.h"
#include "Message.h"
#include "Enum.h"
#include "Options.h"
#include "log.h"
#include <assert.h>
#include <string>

using namespace std;

#define YYSTYPE yy::ProtoParser::semantic_type
#define YYLTYPE yy::ProtoParser::location_type
#define YY_EXTRA_TYPE ProtoDriver*
yy::ProtoParser::token_type lookupId(char *yytxt, unsigned yylen, YYSTYPE *param);

%}


OPTION		"option"
MESSAGE		"message"
ONEOF		"oneof"
OPTIONAL	"optional"
REQUIRED	"required"
REPEATED	"repeated"
RESERVE		"reserve"
ENUM		"enum"
STRING		"string"
CSTR		"cstr"
BYTES		"bytes"
BOOL		"bool"
INT8		"int8"
UINT8		"uint8"
SINT8		"sint8"
FIXED8		"fixed8"
SFIXED8		"sfixed8"
FIXED16		"fixed16"
SFIXED16	"sfixed16"
INT16		"int16"
UINT16		"uint16"
SINT16		"sint16"
FIXED32		"fixed32"
SFIXED32	"sfixed32"
INT32		"int32"
UINT32		"uint32"
SINT32		"sint32"
FIXED64		"fixed64"
SFIXED64	"sfixed64"
INT64		"int64"
UINT64		"uint64"
SINT64		"sint64"
FLOAT		"float"
DOUBLE		"double"
UNSIGNED	"unsigned"
INT		"int"
SIGNED		"signed"

EQUAL		"="
OPTSTART	"["
OPTEND		"]"
LBRACE		"{"
RBRACE		"}"
SEMI		";"
COMMA		","
COLON		":"
RANGE		".."

LCOMMENT	"//"[^\n]*\n
BCOMMENT	"/*"
WHITESP		[ \t]+
NEWLINE		\n

IDENTIFIER	[a-zA-Z_][a-zA-Z0-9_]*
IDREF		[a-zA-Z_][a-zA-Z0-9_:]*
NODEREF		("/"[a-zA-Z_][a-zA-Z0-9_]*)+
SYSINC		"<"[a-zA-Z0-9_+-.]*">"

DECLIT		-?[1-9][0-9]*
OCTLIT		[0-7]*
HEXLIT		0x[0-9a-fA-F]+

BACKSLASH	"\\"
ESC		{BACKSLASH}([abfnrtv\?'"\\])
DQSTR		(\"([^\"\n\\]|{ESC})*\")
SQSTR		(\'([^\'\n\\]|{ESC})*\')
STRINGLIT	({SQSTR}|{DQSTR})

/*
SQSTRING	"'"[^\n']*"'"
DQSTRING	"\""[^\n"]*"\""
STRINGLIT	{DQSTRING}|{SQSTRING}
*/
UNKNOWN		.



%%
{OPTION}	{ return yy::ProtoParser::token::OPTION; }
{OPTIONAL}	{ return yy::ProtoParser::token::OPTIONAL; }
{REQUIRED}	{ return yy::ProtoParser::token::REQUIRED; }
{REPEATED}	{ return yy::ProtoParser::token::REPEATED; }
{MESSAGE}	{ return yy::ProtoParser::token::MESSAGE; }
{RESERVE}	{ return yy::ProtoParser::token::RESERVE; }
{ONEOF}		{ return yy::ProtoParser::token::ONEOF; }
{ENUM}		{ return yy::ProtoParser::token::ENUM; }
{BOOL}		{ return yy::ProtoParser::token::BOOL; }
{INT8}		{ if (yyextra->getProtocol() == xprotov1)
			return yy::ProtoParser::token::INT8;
		  return lookupId(yytext,yyleng,yylval_param);
		}
{UINT8}		{ if (yyextra->getProtocol() == xprotov1)
			return yy::ProtoParser::token::UINT8;
		  return lookupId(yytext,yyleng,yylval_param);
		}
{SINT8}		{ if (yyextra->getProtocol() == xprotov1)
			return yy::ProtoParser::token::SINT8;
		  return lookupId(yytext,yyleng,yylval_param);
		}
{FIXED8}	{ if (yyextra->getProtocol() == xprotov1)
			return yy::ProtoParser::token::FIXED8;
		  return lookupId(yytext,yyleng,yylval_param);
		}
{SFIXED8}	{ if (yyextra->getProtocol() == xprotov1)
			return yy::ProtoParser::token::SFIXED8;
		  return lookupId(yytext,yyleng,yylval_param);
		}
{FIXED16}	{ if (yyextra->getProtocol() == xprotov1)
			return yy::ProtoParser::token::FIXED16;
		  return lookupId(yytext,yyleng,yylval_param);
		}
{SFIXED16}	{ if (yyextra->getProtocol() == xprotov1)
			return yy::ProtoParser::token::SFIXED16;
		  return lookupId(yytext,yyleng,yylval_param);
		}
{INT16}		{ if (yyextra->getProtocol() == xprotov1)
			return yy::ProtoParser::token::INT16;
		  return lookupId(yytext,yyleng,yylval_param);
		}
{UINT16}	{ if (yyextra->getProtocol() == xprotov1)
			return yy::ProtoParser::token::UINT16;
		  return lookupId(yytext,yyleng,yylval_param);
		}
{SINT16}	{ if (yyextra->getProtocol() == xprotov1)
			return yy::ProtoParser::token::SINT16;
		  return lookupId(yytext,yyleng,yylval_param);
		}
{UNSIGNED}	{ if (yyextra->getProtocol() == xprotov1)
			return yy::ProtoParser::token::UNSIGNED;
		  return lookupId(yytext,yyleng,yylval_param);
		}
{SIGNED}	{ if (yyextra->getProtocol() == xprotov1)
			return yy::ProtoParser::token::SIGNED;
		  return lookupId(yytext,yyleng,yylval_param);
		}
{INT}		{ if (yyextra->getProtocol() == xprotov1)
			return yy::ProtoParser::token::INT;
		  return lookupId(yytext,yyleng,yylval_param);
		}
{FIXED32}	{ return yy::ProtoParser::token::FIXED32; }
{SFIXED32}	{ return yy::ProtoParser::token::SFIXED32; }
{INT32}		{ return yy::ProtoParser::token::INT32; }
{UINT32}	{ return yy::ProtoParser::token::UINT32; }
{SINT32}	{ return yy::ProtoParser::token::SINT32; }
{FIXED64}	{ return yy::ProtoParser::token::FIXED64; }
{SFIXED64}	{ return yy::ProtoParser::token::SFIXED64; }
{INT64}		{ return yy::ProtoParser::token::INT64; }
{UINT64}	{ return yy::ProtoParser::token::UINT64; }
{SINT64}	{ return yy::ProtoParser::token::SINT64; }
{FLOAT}		{ return yy::ProtoParser::token::FLOAT; }
{DOUBLE}	{ return yy::ProtoParser::token::DOUBLE; }
{BYTES}		{ return yy::ProtoParser::token::BYTES; }
{STRING}	{ return yy::ProtoParser::token::STRING; }
{CSTR}		{ return yy::ProtoParser::token::CSTR; }
{SEMI}		{ return yy::ProtoParser::token::SEMI; }
{COMMA}		{ return yy::ProtoParser::token::COMMA; }
{RANGE}		{ return yy::ProtoParser::token::RANGE; }
{COLON}		{ return yy::ProtoParser::token::COLON; }
{EQUAL}		{ return yy::ProtoParser::token::EQUAL; }

{LBRACE}	{ return yy::ProtoParser::token::LBRACE; }
{RBRACE}	{ return yy::ProtoParser::token::RBRACE; }
{OPTSTART}	{ return yy::ProtoParser::token::OPTSTART; }
{OPTEND}	{ return yy::ProtoParser::token::OPTEND; }

{IDENTIFIER}	{ return lookupId(yytext,yyleng,yylval_param); }
{IDREF}		{ return lookupId(yytext,yyleng,yylval_param); }
{SYSINC}	{ return lookupId(yytext,yyleng,yylval_param); }
{NODEREF}	{	yylval_param->id.str = yytext;
			yylval_param->id.len = yyleng;
			return yy::ProtoParser::token::NODEREF;
		}

{DECLIT}	{	yylval_param->id.str = yytext;
			yylval_param->id.len = yyleng;
			return yy::ProtoParser::token::DECLIT;
		}
{OCTLIT}	{	yylval_param->id.str = yytext;
			yylval_param->id.len = yyleng;
			return yy::ProtoParser::token::OCTLIT;
		}
{HEXLIT}	{	yylval_param->id.str = yytext;
			yylval_param->id.len = yyleng;
			return yy::ProtoParser::token::HEXLIT;
		}
{STRINGLIT}	{	yylval_param->id.str = yytext;
			yylval_param->id.len = yyleng;
			return yy::ProtoParser::token::STRINGLIT;
		}
{NEWLINE}	{ yylloc->begin.lines(1); }
{WHITESP}	{ }
{LCOMMENT}	{ yylloc->begin.lines(1); }
{BCOMMENT}	BEGIN(COMMENT);
{UNKNOWN}	{ error("invalid character in input '%s'",string(yytext,yyleng).c_str()); }

<COMMENT>"*/"		BEGIN(INITIAL);
<COMMENT>[^*\n]+	{ }
<COMMENT>"*"		{ }
<COMMENT>{NEWLINE}	{ yylloc->begin.lines(1); }

%%

int ProtoDriver::parse(const char *fn)
{
	assert(file == 0);
	file = new PBFile(fn);
	FILE *in = fopen(fn,"r");
	if (in == 0)
		fatal("unable to open input file %s: %s",fn,strerror(errno));
	yylex_init(&scanner);
	yyset_in(in,scanner);
	yyset_extra(this,scanner);
	yy::ProtoParser parser(*this);
	if (debug)
		parser.set_debug_level(1);
	int r = parser.parse();
	yylex_destroy(scanner);
	fclose(in);
	return r;
}


int ProtoDriver::lex(yy::ProtoParser::semantic_type *yylval_param, yy::ProtoParser::location_type *yylloc_param)
{
	return yylex(yylval_param,yylloc_param,scanner);
}


int yylex(yy::ProtoParser::semantic_type *yylval_param, yy::ProtoParser::location_type *yylloc_param, ProtoDriver &drv)
{
	return drv.lex(yylval_param,yylloc_param);
}

extern "C"
int yywrap(yyscan_t scanner)
{
	return 1;
}


yy::ProtoParser::token_type lookupId(char *yytxt, unsigned yylen, YYSTYPE *param)
{
/*
	if (drv.syntax == xprotov1) {
		if (0 == strcmp(yytxt,"fixed8")
			return yy::ProtoParser::token::FIXED8;
		if (0 == strcmp(yytxt,"sfixed8")
			return yy::ProtoParser::token::SFIXED8;
		if (0 == strcmp(yytxt,"fixed16")
			return yy::ProtoParser::token::FIXED16;
		if (0 == strcmp(yytxt,"sfixed16")
			return yy::ProtoParser::token::SFIXED16;

	}
	*/
	if (unsigned n = Message::resolveId(yytxt,yylen)) {
		param->msgid = n;
		return yy::ProtoParser::token::MSGNAME;
	}
	if (unsigned n = Enum::resolveId(yytxt,yylen)) {
		param->enumid = n;
		return yy::ProtoParser::token::ENUMNAME;
	}
	param->id.str = yytxt;
	param->id.len = yylen;
	return yy::ProtoParser::token::IDENTIFIER;
}

