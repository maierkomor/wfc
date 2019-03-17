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

#include "io.h"
#include "log.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;


char *readFile(const char *fn, struct stat *s)
{
	int fd = open(fn,O_RDONLY);
	if (fd == -1) {
		warn("unable to open file %s: %s",fn,strerror(errno));
		return 0;
	}
	struct stat st;
	if (s != 0) {
		st.st_size = s->st_size;
	} else if (-1 == fstat(fd,&st)) {
		close(fd);
		warn("unable to stat file %s: %s",fn,strerror(errno));
		return 0;
	}
	char *buf = (char *)malloc(st.st_size+1);
	buf[st.st_size] = 0;
	int n = read(fd,buf,st.st_size);
	close(fd);
	if (n == -1) {
		warn("error while reading file %s: %s",fn,strerror(errno));
		free(buf);
		return 0;
	}
	return buf;
}


#if defined __MINGW32__ || defined __MINGW64__
void *memrchr(void *p, char c, size_t l)
{
	char *x = (char*)p + l;
	while (l) {
		--l;
		--x;
		if (*x == c)
			return (void*)x;
	}
	return 0;
}
#endif


void foldPath(char *p)
{
	char *dd = strstr(p,"/../");
	while (dd) {
		if (dd == p) {
			memmove(p,p+3,strlen(p+3)+1);
		} else if (char *sl =  (char*)memrchr(p,'/',dd-p)) {
			memmove(sl,dd+3,strlen(dd+3)+1);
		} else {
			p[0] = '.';
			strcpy(p+1,dd+3);
		}
		dd = strstr(p,"/../");
		//printf("p: %s\n",p);
	}
	size_t pl = strlen(p);
	if (0 == memcmp(p+pl-3,"/..",3)) {
		p[pl-3] = 0;
		char *sl = strrchr(p,'/');
		if (sl) {
			if (sl == p)
				p[1] = 0;
			else
				*sl = 0;
		} else {
			p[0] = '/';
			p[1] = 0;
		}
	}
}


void foldPath(string &path)
{
	char p[path.size()+1];
	strcpy(p,path.c_str());
	//printf("foldPath(%s)\n",p);
	foldPath(p);
	path = p;
	//printf("foldPath()=%s\n",p);
}


bool isDir(const char *fname)
{
	struct stat st;
	return ((0 == stat(fname,&st) && (S_ISDIR(st.st_mode))));
}


#ifdef TESTMODULE
#include <stdio.h>
#include <limits.h>

const char *patterns[] = {
	"/../",
	"/",
	"abc/..",
	"abc/../",
	"/bla/bla/blu/../x/y/",
	"/bla/../blu/../x/y/",
	"/bla/bla/blu/../../x/y/",
	"/bla/bla/blu/../../x/y/..",
	"/..",
	"/bla/..",

};

int main()
{
	char p[PATH_MAX];
	for (int i = 0; i < sizeof(patterns)/sizeof(patterns[0]); ++i) {
		strcpy(p,patterns[i]);
		foldPath(p);
		printf("%s => %s\n",patterns[i],p);
	}
}
#endif
