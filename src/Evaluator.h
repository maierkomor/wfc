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

#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <map>
#include <string>

typedef enum {
	em_invalid = 0,
	em_ret_errid = 1,
	em_throw_errid = 2,
	em_abort_exe = 3,
} errmode_t;

class Evaluator
{
	public:
	Evaluator(const std::map<std::string,std::string> &vars, int &errid, errmode_t e);

	void evaluate(std::string &text);

	private:
	std::string getVariable(const std::string &v);
	bool isFunction(const char *d);
	void resolveArithmetic(std::string &text, size_t off);
	void resolveFunction(std::string &text, size_t off);
	void resolveVariable(std::string &text, size_t off);

	const std::map<std::string,std::string> &m_vars;
	int &m_errid;
	errmode_t m_errmode;
};

#endif
