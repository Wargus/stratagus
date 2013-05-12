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
/**@name animation_spawnmissile.h - The animation SpawnMissile headerfile. */
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

#ifndef ANIMATION_SPAWNMISSILE_H
#define ANIMATION_SPAWNMISSILE_H

//@{

#include <string>
#include "animation.h"

//SpawnMissile flags
enum SpawnMissile_Flags {
	SM_None = 0,           /// Clears all flags
	SM_Damage = 1,         /// Missile deals damage to units
	SM_ToTarget = 2,       /// Missile is directed to unit's target
	SM_Pixel = 4,          /// Missile's offsets are calculated in pixels rather than tiles
	SM_RelTarget = 8,      /// All calculations are relative to unit's target
	SM_Ranged = 16,        /// Missile can't be shot if current range between unit and it's target
	                       /// is bigger than unit's attack range
	SM_SetDirection = 32   /// Missile takes the same direction as spawner
};

class CAnimation_SpawnMissile : public CAnimation
{
public:
	CAnimation_SpawnMissile() : CAnimation(AnimationSpawnMissile) {}

	virtual void Action(CUnit &unit, int &move, int scale) const;
	virtual void Init(const char *s, lua_State *l);

private:
	std::string missileTypeStr;
	std::string startXStr;
	std::string startYStr;
	std::string destXStr;
	std::string destYStr;
	std::string flagsStr;
	std::string offsetNumStr;
};

//@}

#endif // ANIMATION_SPAWNMISSILE_H
