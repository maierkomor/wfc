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

#ifndef LOG_H
#define LOG_H

extern unsigned NumWarnings;
extern bool Diags;
#ifdef DEBUG
extern bool Debug;
void dbug(const char *, ...);
#else
#define dbug(...)
#endif

bool hadError();
void msg(const char *, ...) __attribute__((format(printf,1,2)));
void diag(const char *, ...) __attribute__((format(printf,1,2)));
void warn(const char *, ...) __attribute__((format(printf,1,2)));
void error(const char *, ...) __attribute__((format(printf,1,2)));
void fatal(const char *, ...) __attribute__((format(printf,1,2)));
void ICE(const char *, ...) __attribute__((format(printf,1,2)));

#endif
