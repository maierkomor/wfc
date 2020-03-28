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

#include "log.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <set>
#include <string>

using namespace std;


int MsgLog = STDOUT_FILENO;
int DiagLog = STDOUT_FILENO;
//int DiagLog = STDERR_FILENO;

bool Diags = false;
bool HadError = false;
unsigned NumWarnings = 0;
#ifdef DEBUG
bool Debug = false;
#endif

static set<string> Errors, Warnings;


bool hadError()
{ return HadError; }


void msg(const char *m, ...)
{
	char buf[1024];
	va_list val;
	va_start(val,m);
	int n = vsnprintf(buf,sizeof(buf)-1,m,val);
	va_end(val);
	if (n >= (int)sizeof(buf))
		n = sizeof(buf)-1;
	buf[n++] = '\n';
	if (-1 == write(MsgLog,buf,n))
		perror("writing to message log");
}


#ifdef DEBUG
void dbug(const char *m, ...)
{
	if (!Debug)
		return;
	char buf[1024];
	va_list val;
	va_start(val,m);
	int n = vsnprintf(buf,sizeof(buf)-1,m,val);
	va_end(val);
	if (n >= (int)sizeof(buf))
		n = sizeof(buf)-1;
	buf[n++] = '\n';
	if (-1 == write(DiagLog,buf,n))
		perror("writing to diagnostic log");
}
#endif


void diag(const char *m, ...)
{
	if (!Diags)
		return;
	char buf[1024];
	va_list val;
	va_start(val,m);
	int n = vsnprintf(buf,sizeof(buf)-1,m,val);
	va_end(val);
	if (n >= (int)sizeof(buf))
		n = sizeof(buf)-1;
	buf[n++] = '\n';
	if (-1 == write(DiagLog,buf,n))
		perror("writing to diagnostic log");
}


void warn(const char *m, ...)
{
	char prefix[] = "warning: ";
	char buf[1024];
	va_list val;
	va_start(val,m);
	memcpy(buf,prefix,sizeof(prefix));
	int n = vsnprintf(buf+sizeof(prefix)-1,sizeof(buf)-sizeof(prefix)-1,m,val) + sizeof(prefix) -1;
	va_end(val);
	if (n >= (int)sizeof(buf))
		n = sizeof(buf)-1;
	if (!Warnings.insert(buf).second)	// warning already displayed
		return;
	buf[n++] = '\n';
	++NumWarnings;
	if (-1 == write(DiagLog,buf,n))
		perror("writing to diagnostic log");
}


void error(const char *m, ...)
{
	char prefix[] = "error: ";
	char buf[1024];
	HadError = true;
	va_list val;
	va_start(val,m);
	memcpy(buf,prefix,sizeof(prefix));
	int n = vsnprintf(buf+sizeof(prefix)-1,sizeof(buf)-sizeof(prefix)-1,m,val) + sizeof(prefix) -1;
	va_end(val);
	if (n >= (int)sizeof(buf))
		n = sizeof(buf)-1;
	if (!Errors.insert(buf).second)		// error already displayed
		return;
	buf[n++] = '\n';
	if (-1 == write(DiagLog,buf,n))
		perror("writing to diagnostic log");
}


void ICE(const char *m, ...)
{
	char prefix[] = "ICE/bug: ";
	if (-1 == write(DiagLog,prefix,sizeof(prefix)))
		abort();
	char *buf;
	va_list val;
	va_start(val,m);
#if defined __MINGW32__ ||  defined __MINGW64__
	int n = vsnprintf(0,0,m,val);
	buf = (char *) malloc(n+1);
	vsprintf(buf,m,val);
#else
	int n = vasprintf(&buf,m,val);
#endif
	va_end(val);
	if (-1 == write(DiagLog,buf,n))
		abort();
	abort();
}


void fatal(const char *m, ...)
{
	char prefix[] = "fatal: ";
	char buf[1024];
	va_list val;
	va_start(val,m);
	strcpy(buf,prefix);
	int n = vsnprintf(buf+sizeof(prefix)-1,sizeof(buf)-sizeof(prefix)-1,m,val) + sizeof(prefix) -1;
	va_end(val);
	buf[n++] = '\n';
	if (n >= (int)sizeof(buf))
		n = sizeof(buf)-1;
	if (-1 == write(DiagLog,buf,n))
		perror("writing to diagnostic log");
	exit(EXIT_FAILURE);
}
