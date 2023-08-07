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
/**@name animation_ifvar.h - The animation IfVar headerfile. */
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

#ifndef ANIMATION_IFVAR_H
#define ANIMATION_IFVAR_H

//@{

#include <string>
#include "animation.h"

class CAnimation_IfVar : public CAnimation
{
public:
	CAnimation_IfVar() : CAnimation(AnimationIfVar), binOpFunc(nullptr), gotoLabel(nullptr) {}

	virtual void Action(CUnit &unit, int &move, int scale) const;
	virtual void Init(const char *s, lua_State *l);

private:
	using BinOpFunc = bool (int lhs, int rhs);

private:
	std::string leftVar;
	std::string rightVar;
	BinOpFunc *binOpFunc;
	CAnimation *gotoLabel;
};

//@}

#endif // ANIMATION_IFVAR_H
