//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name animation_ifvar.cpp - The animation IfVar. */
//
//      (c) Copyright 2012 by Joris Dauphin
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "animation/animation_ifvar.h"

#include "unit.h"

//IfVar compare types
enum EIfVarBinOp {
	IF_GREATER_EQUAL = 1,
	IF_GREATER,
	IF_LESS_EQUAL,
	IF_LESS,
	IF_EQUAL,
	IF_NOT_EQUAL,
};

bool binOpGreaterEqual(int lhs, int rhs) { return lhs >= rhs; }
bool binOpGreater(int lhs, int rhs) { return lhs > rhs; }
bool binOpLessEqual(int lhs, int rhs) { return lhs <= rhs; }
bool binOpLess(int lhs, int rhs) { return lhs < rhs; }
bool binOpEqual(int lhs, int rhs) { return lhs == rhs; }
bool binOpNotEqual(int lhs, int rhs) { return lhs != rhs; }
bool returnFalse(int , int) { return false; }

/* virtual */ void CAnimation_IfVar::Action(CUnit &unit, int &/*move*/, int /*scale*/) const
{
	Assert(unit.Anim.Anim == this);

	const int lop = ParseAnimInt(&unit, this->leftVar.c_str());
	const int rop = ParseAnimInt(&unit, this->rightVar.c_str());
	const bool cond = this->binOpFunc(lop, rop);

	if (cond) {
		unit.Anim.Anim = this->gotoLabel;
	}
}

/*
** s = "leftOp Op rigthOp gotoLabel"
*/
/* virtual */ void CAnimation_IfVar::Init(const char *s)
{
	const std::string str(s);
	const size_t len = str.size();

	size_t begin = 0;
	size_t end = std::min(len, str.find(' ', begin));
	this->leftVar.assign(str, begin, end - begin);

	begin = std::min(len, str.find_first_not_of(' ', end));
	end = std::min(len, str.find(' ', begin));
	std::string op(str, begin, end - begin);

	if (op == ">=") {
		this->binOpFunc = binOpGreaterEqual;
	} else if (op == ">") {
		this->binOpFunc = binOpGreater;
	} else if (op == "<=") {
		this->binOpFunc = binOpLessEqual;
	} else if (op == "<") {
		this->binOpFunc = binOpLess;
	} else if (op == "==") {
		this->binOpFunc = binOpEqual;
	} else if (op == "!=") {
		this->binOpFunc = binOpNotEqual;
	} else {
		EIfVarBinOp type = static_cast<EIfVarBinOp>(atoi(op.c_str()));

		switch (type) {
			case IF_GREATER_EQUAL: this->binOpFunc = binOpGreaterEqual; break;
			case IF_GREATER: this->binOpFunc = binOpGreater; break;
			case IF_LESS_EQUAL: this->binOpFunc = binOpLessEqual; break;
			case IF_LESS: this->binOpFunc = binOpLess; break;
			case IF_EQUAL: this->binOpFunc = binOpEqual; break;
			case IF_NOT_EQUAL: this->binOpFunc = binOpNotEqual; break;
			default: this->binOpFunc = returnFalse; break;
		}
	}

	begin = std::min(len, str.find_first_not_of(' ', end));
	end = std::min(len, str.find(' ', begin));
	this->rightVar.assign(str, begin, end - begin);

	begin = std::min(len, str.find_first_not_of(' ', end));
	end = std::min(len, str.find(' ', begin));
	std::string label(str, begin, end - begin);

	FindLabelLater(&this->gotoLabel, label);
}

//@}
