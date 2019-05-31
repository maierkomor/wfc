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

#include "Evaluator.h"
#include "log.h"

#include <iostream>
#include <string>
#include <string.h>
#include <map>
#include <vector>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

using namespace std;

static bool VarChar[256];
static bool initialized = false;


Evaluator::Evaluator(const map<string,string> &vars, int &errid, errmode_t e)
: m_vars(vars)
, m_errid(errid)
, m_errmode(e)
{
	if (!initialized) {
		memset(VarChar,0,sizeof(VarChar));
		for (int i = '0'; i <= '9'; ++i)
			VarChar[i] = true;
		for (int i = 'a'; i <= 'z'; ++i)
			VarChar[i] = true;
		for (int i = 'A'; i <= 'Z'; ++i)
			VarChar[i] = true;
		VarChar[(unsigned)'_'] = true;
	}
}


/*
static string makeVar(const char *text, size_t &l)
{
	string v;
	const char *at = text;
	assert(*at == '$');
	++at;
	if (*at == '(') {
		++at;
		do {
			v.push_back(*at);
			++at;
		} while ((*at != '(') && (*at != ')') && (*at != 0));
		// for '(' length will be recalculated by resolveFunction
		++at;
	} else {
		while (VarChar[(unsigned)*at])
			v.push_back(*at++);
	}
	if (0 == strcmp(at,"(,)"))
		at += 3;
	l = at-text;
	return v;
}
*/


string Evaluator::getVariable(const string &v)
{
	map<string,string>::const_iterator i = m_vars.find(v);
	if (i != m_vars.end())
		return i->second;
	if (v == "handle_error") {
		char buf[16];
		switch (m_errmode) {
		case em_ret_errid:
			--m_errid;
			snprintf(buf,sizeof(buf),"return %d",m_errid);
			return buf;
		case em_abort_exe:
			return "abort()";
		case em_throw_errid:
			--m_errid;
			snprintf(buf,sizeof(buf),"throw %d",m_errid);
			return buf;
		default:
			abort();
		}
	}
	ICE("undefined variable $(%s)",v.c_str());
	return "<missing var $" +v + '>';
}


void Evaluator::resolveArithmetic(string &text, size_t off)
{
	assert(off < text.size());
	const char *d = text.c_str()+off;
	assert(d[0] == '$');
	assert(d[1] == '(');
	assert(isdigit(d[2]));
	assert(0 == strchr(d+1,'$'));
	long long v0,v1;
	char op,rp;
	int n = sscanf(d+2,"%lld%c%lld%c",&v0,&op,&v1,&rp);
	assert((n == 4) && (rp == ')'));
	switch (op) {
		case '+':
			v0 += v1;
			break;
		case '-':
			v0 -= v1;
			break;
		case '*':
			v0 *= v1;
			break;
		case '/':
			v0 /= v1;
			break;
		case '%':
			v0 %= v1;
			break;
		case '&':
			v0 &= v1;
			break;
		case '^':
			v0 ^= v1;
			break;
		default:
			abort();
	}
	char tmp[64];
	sprintf(tmp,"%lld",v0);
	const char *p = strchr(d,')');
	dbug("arith: replace '%s' with '%s'",string(d,p-d+1).c_str(),tmp);
	text.replace(off,p-d+1,tmp);
}


void Evaluator::resolveVariable(string &text, size_t off)
{
	assert(off < text.size());
	const char *at = text.c_str()+off;
	const char *t = at;
	assert(strchr(t+1,'$') == 0);
	assert(*t == '$');
	assert(!isdigit(t[1]));
	bool optional_comma = true;
	int p = 0;
	++t;
	if (*t == '(') {
		++p;
		++t;
		optional_comma = false;	// do not handle "," for $(xx) syntax
		assert(!isdigit(*t));
	}
	const char *vb = t;
	while (isalnum(*t) || (*t == '_'))
		++t;
	const char *ve = t;
	while (p) {
		if (*t == '(')
			++p;
		else if (*t == ')')
			--p;
		else
			assert(*t);
		++t;
	}
	const string &ret = getVariable(string(vb,ve));
	if (optional_comma)
		optional_comma = (*t == ',');
	size_t l = t-at;
	if (optional_comma && ret.empty())
		++l;
	/*
	{
		char buf[256];
		snprintf(buf,sizeof(buf),"resolveVariable '%%-%lus': '%%s' %s comma, var:%s %lu",l,optional_comma?"with":"without",string(vb,ve).c_str(),l);
		msg(buf,text,ret.c_str());
	}
	*/
	dbug("resolveVariable %s,%u",at,l);
	text.replace(off,l,ret);
}


bool Evaluator::isFunction(const char *d)
{
	string var;
	assert(d[0] == '$');
	++d;
	if (*d == '(')
		++d;
	if ((*d >= '0') && (*d <= '9')) {
		dbug("isFuncion(%s) = parm",d);
		return false;
	}
	while (isalnum(*d) || (*d == '_'))
		var.push_back(*d++);
	dbug("isFuncion(%s)",var.c_str());
	map<string,string>::const_iterator i = m_vars.find(var);
	if (i == m_vars.end()) {
		dbug("isFuncion(%s) = nx",var.c_str());
		return false;
	}
	const string &v = i->second;
	bool r = (0 != strstr(v.c_str(),"$1"));
	dbug("isFuncion(%s) = %d",var.c_str(),r);
	return r;
}


void Evaluator::resolveFunction(string &text, size_t off)
{
	assert(off < text.size());
	const char *d = text.c_str()+off;
	assert(d[0] == '$');
	assert(0 == strchr(d+1,'$'));
	const char *at = text.c_str()+off;
	const char *t = at;
	dbug("resolveFunction %s",t);
	assert(*t == '$');
	++t;
	if (*t == '(')
		++t;
	string fname;
	while (VarChar[(unsigned)*t])
		fname.push_back(*t++);
	assert(!fname.empty());
	/*
	if (text[1] == '(') {
		assert(*t == ')');
		++t;
		assert(*t == '(');
	} else {
	*/
	assert(*t == '(');
	int paran = 1;
	++t;
	string param;
	vector<string> params;
	do {
		if (*t == '(') {
			++paran;
			param.push_back(*t);
		} else if (*t == ')') {
			--paran;
			if (paran != 0) {
				param.push_back(*t);
			} else {
				dbug("param %s",param.c_str());
				params.push_back(param);
				param.clear();
			}
		} else if ((*t == ',') && (paran == 1)) {
			dbug("param %s",param.c_str());
			params.push_back(param);
			param.clear();
			while (t[1] == ' ')
				++t;
		} else
			param.push_back(*t);
		++t;
	} while (paran != 0);
	if (text[1] == '(') {
		assert(*t == ')');
		++t;
	}
	string ret = getVariable(fname);
	for (unsigned p = 0; p < params.size(); ++p) {
		char buf[8];
		snprintf(buf,sizeof(buf),"$%u",p+1);
		const char *at = strstr(ret.c_str(),buf);
		evaluate(params[p]);
		dbug("param %u = %s",p+1,params[p].c_str());
		while (at) {
			ret.replace(at-ret.c_str(),strlen(buf),params[p]);
			at = strstr(ret.c_str(),buf);
			dbug("ret = %s",ret.c_str());
		}
	}
	dbug("ret = %s",ret.c_str());
	text.replace(off,t-at,ret);
}

/*
static const char *exprLen(const char *e)
{
	assert(*e == '$');
	if (e[1] == '(') {
		int p = 1;
		e += 2;
		do {
			if (*e == '(')
				++p;
			else if (*e == ')')
				--p;
			else 
				assert((*e != 0) && (*e != '\n'));
			++e;
		} while (p);
	} else {
		while ((*e >= 'a') && (*e <= 'z')
			|| ((*e >= 'A') && (*e <= 'Z'))
			|| ((*e >= '0') && (*e <= '9'))
			|| (*e == '_'))
			++e;
	}
	return e;
}
*/


void Evaluator::evaluate(string &text)
{
	const char *t = text.c_str();
	dbug("resolveVariables0(\"%s\")",t);
	// important to search backward:
	// otherwise one might get partly resolve $() expressions
	// that fail to resolve
	const char *d = strrchr(t,'$');
	while (d) {
		dbug("dollar %s",d);
		if ((d[1] == '(') && ((d[2] >= '0') && (d[2] <= '9'))) {
			resolveArithmetic(text,d-t);
		} else if (isFunction(d)) {
			resolveFunction(text,d-t);
		} else {
			resolveVariable(text,d-t);
		}
		t = text.c_str();
		d = strrchr(t,'$');
	}
}


#ifdef TEST_MODULE
int main()
{
	const char *text = 
	"$u32 * $(u32) = $(a($b,$c)) $a(1,x)\n"
	"$a($u32, 2)";

	Vars["a"] = "$1/$2";
	Vars["b"] = "var_b";
	Vars["c"] = "var_c";
	Vars["u32"] = "var_32U";
	Vars["u16"] = "var_16U";
	
	string result = text;
	resolveVariables(result);
	cout << result << endl;
}
#endif
