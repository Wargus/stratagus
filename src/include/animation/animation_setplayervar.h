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
/**@name animation_setplayervar.h - The animation SetPlayerVar headerfile. */
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

#ifndef ANIMATION_SETPLAYERVAR_H
#define ANIMATION_SETPLAYERVAR_H

//@{

#include <string>
#include "animation.h"

class CAnimation_SetPlayerVar : public CAnimation
{
public:
	void Action(CUnit &unit, int &move, int scale) const override;
	void Init(const char *s, lua_State *l) override;

private:
	SetVar_ModifyTypes mod;
	std::string playerStr;
	std::string varStr;
	std::string argStr;
	std::string valueStr;
};

extern int GetPlayerData(int player, std::string_view prop, std::string_view arg);

//@}

#endif // ANIMATION_SETPLAYERVAR_H
