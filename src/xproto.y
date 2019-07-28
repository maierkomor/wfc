%{
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

#define YYDEBUG 1
#include "Message.h"
#include "Options.h"
#include "KVPair.h"
#include "PBFile.h"
#include "ProtoDriver.h"
#include "Field.h"
#include "Enum.h"
#include "log.h"
#include <set>
#include <string>


using namespace std;

%}

%locations
%define parser_class_name {ProtoParser}
%define parse.trace
%define parse.error verbose

%code requires
{
class ProtoDriver;
}

%param { ProtoDriver &drv }

%locations
%initial-action
{ @$.begin.filename = @$.end.filename = &drv.filename; }

%code {
#include "ProtoDriver.h"
}

%union {
	struct { char *str; unsigned len; } id;
	struct { char *str; unsigned len; } intlit;
	uint32_t msgid;
	uint32_t enumid;
	uint32_t type;
	quant_t quant;
	class Field *field;
	class Message *msg;
	class PBFile *file;
	class Enum *edef;
	class KVPair *kvpair;
	class Options *options;
}

%token OPTION MESSAGE OPTIONAL REQUIRED REPEATED ENUM ONEOF
	STRING BYTES BOOL CSTR
	INT8 UINT8 SINT8 FIXED8 SFIXED8
	INT16 UINT16 SINT16 FIXED16 SFIXED16
	INT32 UINT32 SINT32 FIXED32 SFIXED64
	INT64 UINT64 SINT64 FIXED64 SFIXED32
	INT UNSIGNED SIGNED
	FLOAT DOUBLE
	LBRACE RBRACE OPTSTART OPTEND SEMI EQUAL COMMA COLON

%token<msgid> MSGNAME
%token<enumid> ENUMNAME
%token<id> IDENTIFIER OCTLIT DECLIT HEXLIT STRINGLIT NODEREF SYSINC IDREF

%type<edef> EnumDef_P Enum_P
%type<quant> Quantifier_P
%type<type> Type_P
%type<field> FieldSpec_P Field_P
%type<msg> Message_P MessageBody_P
%type<file> File_P
%type<intlit> IntLit_P
%type<kvpair> Option_P OptionList_P
%type<id> OptionArg_P
%type<options> Target_P TargetOptions_P

%start File_P

%%
Quantifier_P
	: OPTIONAL	{ $$ = q_optional; }
	| REQUIRED	{ $$ = q_required; }
	| REPEATED	{ $$ = q_repeated; }
	|		{ $$ = q_unspecified; }
	;

Type_P	: INT32		{ $$ = ft_int32; }
	| UINT32	{ $$ = ft_uint32; }
	| SINT32	{ $$ = ft_sint32; }
	| INT64		{ $$ = ft_int64; }
	| UINT64	{ $$ = ft_uint64; }
	| SINT64	{ $$ = ft_sint64; }
	| INT16		{ $$ = ft_int16; }
	| UINT16	{ $$ = ft_uint16; }
	| SINT16	{ $$ = ft_sint16; }
	| INT8		{ $$ = ft_int8; }
	| UINT8		{ $$ = ft_uint8; }
	| SINT8		{ $$ = ft_sint8; }
	| FLOAT		{ $$ = ft_float; }
	| DOUBLE	{ $$ = ft_double; }
	| STRING	{ $$ = ft_string; }
	| BYTES		{ $$ = ft_bytes; }
	| BOOL		{ $$ = ft_bool; }
	| FIXED8	{ $$ = ft_fixed8; }
	| SFIXED8	{ $$ = ft_sfixed8; }
	| FIXED16	{ $$ = ft_fixed16; }
	| SFIXED16	{ $$ = ft_sfixed16; }
	| FIXED32	{ $$ = ft_fixed32; }
	| SFIXED32	{ $$ = ft_sfixed32; }
	| FIXED64	{ $$ = ft_fixed64; }
	| SFIXED64	{ $$ = ft_sfixed64; }
	| UNSIGNED	{ $$ = ft_unsigned; }
	| SIGNED	{ $$ = ft_signed; }
	| INT		{ $$ = ft_int; }
	| MSGNAME	{ $$ = $1; }
	| ENUMNAME	{ $$ = $1; }
	;

IntLit_P: DECLIT
	{ $$.str = $1.str; $$.len = $1.len; }
	| HEXLIT
	{ $$.str = $1.str; $$.len = $1.len; }
	| OCTLIT
	{ $$.str = $1.str; $$.len = $1.len; }
	;

Option_P: IDENTIFIER EQUAL OptionArg_P
	{ $$ = new KVPair($1.str,$1.len,$3.str,$3.len); }
	;

OptionList_P
	: Option_P
	{ $$ = $1; }
	| OptionList_P COMMA Option_P
	{ $1->setNext($3); $$ = $1; }
	;

OptionArg_P
	: IDENTIFIER
	{ $$ = $1; }
	| IDREF
	{ $$ = $1; }
	| SYSINC
	{ $$ = $1; }
	| STRINGLIT
	{ $$ = $1; }
	| DECLIT
	{ $$ = $1; }
	| HEXLIT
	{ $$ = $1; }
	| OCTLIT
	{ $$ = $1; }
	;

/*
OptionName_P
	: OPTION IDENTIFIER
	{ $$ = $2; }
	;
*/

TargetOptions_P
	: OPTION IDENTIFIER LBRACE
	{ $$ = new Options(string($2.str,$2.len),Options::getDefaults()); }
	| OPTION IDENTIFIER COLON IDENTIFIER LBRACE
	{ $$ = new Options(string($2.str,$2.len),drv.getFile()->getTarget($4.str,$4.len)); }
	| TargetOptions_P Option_P SEMI
	{ $$ = $1; $$->add($2); }
	| TargetOptions_P NODEREF COLON OptionList_P SEMI
	{ $$ = $1; $$->add(string($2.str,$2.len),$4); }
	;

Target_P: TargetOptions_P RBRACE
	{ $$ = $1; }
	;

EnumDef_P
	: ENUM IDENTIFIER LBRACE 
	{ $$ = Enum::create($2.str,$2.len); }
	| ENUM ENUMNAME LBRACE 
	{ $$ = Enum::create(Enum::resolveId($2)); }
	| EnumDef_P IDENTIFIER EQUAL IntLit_P SEMI
	{ $1->add($2.str,$2.len,$4.str,$4.len); $$ = $1; }
	| EnumDef_P IDENTIFIER EQUAL IntLit_P OPTSTART STRING EQUAL STRINGLIT OPTEND SEMI
	{	$1->add($2.str,$2.len,$4.str,$4.len);
		$1->setStringValue($4.str,$8.str,$8.len); $$ = $1; }
	| EnumDef_P OPTION IDENTIFIER EQUAL OptionArg_P SEMI
	{ $$ = $1; $$->setOption($3.str,$3.len,$5.str,$5.len); }
	;

Enum_P 	: EnumDef_P RBRACE
	{ $$ = $1; }
	;

FieldSpec_P
	: Quantifier_P Type_P IDENTIFIER EQUAL IntLit_P
	{
		switch (drv.getProtocol()) {
		default:
			abort();
		case gprotov2:
			if ($1 == q_unspecified) {
				::error("missing quantifier");
				$1 = q_optional;
			}
			break;
		case gprotov3:
			if (($1 == q_optional) || ($1 == q_required)) {
				::error("keywords required/optional invalid for protoV3");
				$1 = q_optional;
			}
			break;
		case xprotov1:
			if ($1 == q_unspecified)
				$1 = q_optional;
			break;
		}
		$$ = new Field($3.str,$3.len,$1,(fieldtype_t)$2,strtoll($5.str,0,0));
	}
	;

Field_P	: FieldSpec_P SEMI
	{ $$ = $1; }
	| FieldSpec_P OPTSTART OptionList_P OPTEND SEMI
	{ $$ = $1; $$->addOptions($3); }
	;

MessageBody_P
	: MESSAGE IDENTIFIER LBRACE
	{ $$ = new Message($2.str,$2.len); }
	| MessageBody_P OPTION IDENTIFIER EQUAL OptionArg_P SEMI
	{ $$ = $1; $$->setOption(string($3.str,$3.len),string($5.str,$5.len)); }
	| ONEOF IDENTIFIER LBRACE
	{ $$ = new Message($2.str,$2.len,true); }
	| MessageBody_P Field_P
	{ $$ = $1; $1->addField($2); }
	| MessageBody_P Message_P
	{ $$ = $1; $1->addMessage($2); ParsingMessage = $1->getName(); }
	| MessageBody_P Enum_P
	{ $$ = $1; $1->addEnum($2); }
	| MessageBody_P SEMI
	{ $$ = $1; ParsingMessage = ""; }
	;

Message_P: MessageBody_P RBRACE
	{ $$ = $1; }
	;

File_P	: Message_P
	{ $$ = drv.getFile(); $$->addMessage($1); }
	| Target_P
	{ $$ = drv.getFile(); $$->addOptions($1); }
	| OPTION IDENTIFIER EQUAL OptionArg_P SEMI
	{ $$ = drv.getFile(); $$->setOption($2.str,$2.len,$4.str,$4.len); }
	| OPTION NODEREF COLON OptionList_P SEMI
	{ $$ = drv.getFile(); $$->getOptions()->add(string($2.str,$2.len),$4); }
	| Enum_P
	{ $$ = drv.getFile(); $$->addEnum($1); drv.setFile($$); }
	| File_P Message_P
	{ $1->addMessage($2); $$ = $1; }
	| File_P Enum_P
	{ $1->addEnum($2); }
	| File_P Target_P
	{ $1->addOptions($2); }
	| File_P OPTION IDENTIFIER EQUAL OptionArg_P SEMI
	{ $1->setOption($3.str,$3.len,$5.str,$5.len); $$ = $1; }
	| File_P OPTION NODEREF COLON OptionList_P SEMI
	{ $1->getOptions()->add(string($3.str,$3.len),$5); $$ = $1; }
	;

%%

