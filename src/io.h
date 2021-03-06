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

#ifndef IO_H
#define IO_H

#include <string>

#if defined __MINGW32__ || defined __MINGW64__
#define FILESEP '\\'
#else
#define FILESEP '/'
#endif

char *readFile(const char *, struct stat *s = 0);
void foldPath(char *p);
void foldPath(std::string &path);
bool isDir(const char *fname);

#endif
