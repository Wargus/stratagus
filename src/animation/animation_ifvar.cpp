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

#include "animation.h"
#include "unit.h"

//IfVar compare types
#define IF_GREATER_EQUAL 1
#define IF_GREATER 2
#define IF_LESS_EQUAL 3
#define IF_LESS 4
#define IF_EQUAL 5
#define IF_NOT_EQUAL 6

/* virtual */ void CAnimation_IfVar::Action(CUnit& unit, int &/*move*/, int /*scale*/) const
{
	Assert(unit.Anim.Anim == this);

	const int lop = ParseAnimInt(&unit, this->leftVar.c_str());
	const int rop = ParseAnimInt(&unit, this->rightVar.c_str());
	bool go = false;

	switch (this->type) {
		case IF_GREATER_EQUAL:
			go = (lop >= rop);
			break;
		case IF_GREATER:
			go = (lop > rop);
			break;
		case IF_LESS_EQUAL:
			go = (lop <= rop);
			break;
		case IF_LESS:
			go = (lop < rop);
			break;
		case IF_EQUAL:
			go = (lop == rop);
			break;
		case IF_NOT_EQUAL:
			go = (lop != rop);
		break;
	}
	if (go) {
		unit.Anim.Anim = this->gotoLabel;
	}
}

/* virtual */ void CAnimation_IfVar::Init(const char* s)
{
	char *op2 = const_cast<char*>(s);
	char *next = strchr(op2, ' ');
	if (next) {
		while (*next == ' ') {
			*next++ = '\0';
		}
	}
	this->leftVar = op2;
	op2 = next;
	next = strchr(op2, ' ');
	if (next) {
		while (*next == ' ') {
			*next++ = '\0';
		}
	}
	this->rightVar = op2;
	op2 = next;
	next = strchr(op2, ' ');
	if (next) {
		while (*next == ' ') {
			*next++ = '\0';
		}
	}
	if (!strcmp(op2,">=")) {
		this->type = 1;
	} else if (!strcmp(op2,">")) {
		this->type = 2;
	} else if (!strcmp(op2,"<=")) {
		this->type = 3;
	} else if (!strcmp(op2,"<")) {
		this->type = 4;
	} else if (!strcmp(op2,"==")) {
		this->type = 5;
	} else if (!strcmp(op2,"!=")) {
		this->type = 6;
	} else {
		this->type = atoi(op2);
	}
	op2 = next;
	while (*op2 == ' ') {
		++op2;
	}
	FindLabelLater(&this->gotoLabel, op2);
}

//@}
