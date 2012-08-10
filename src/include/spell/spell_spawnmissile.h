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
//
//      (c) Copyright 1999-2012 by Vladi Belperchinov-Shabanski,
//                                 Joris DAUPHIN, and Jimmy Salmon
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

#ifndef SPELL_SPAWNMISSILE_H
#define SPELL_SPAWNMISSILE_H

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "spells.h"

/**
**  Different targets.
*/
enum LocBaseType {
	LocBaseCaster,
	LocBaseTarget
};

/**
**  This struct is used for defining a missile start/stop location.
**
**  It's evaluated like this, and should be more or less flexible.:
**  base coordinates(caster or target) + (AddX,AddY) + (rand()%AddRandX,rand()%AddRandY)
*/
class SpellActionMissileLocation
{
public:
	SpellActionMissileLocation(LocBaseType base) : Base(base), AddX(0), AddY(0),
		AddRandX(0), AddRandY(0) {} ;

	LocBaseType Base;   /// The base for the location (caster/target)
	int AddX;           /// Add to the X coordinate
	int AddY;           /// Add to the X coordinate
	int AddRandX;       /// Random add to the X coordinate
	int AddRandY;       /// Random add to the X coordinate
};

class Spell_SpawnMissile : public SpellActionType
{
public:
	Spell_SpawnMissile() : Damage(0), TTL(-1), Delay(0), UseUnitVar(false),
		StartPoint(LocBaseCaster), EndPoint(LocBaseTarget), Missile(0) {}
	virtual int Cast(CUnit &caster, const SpellType &spell,
					 CUnit *target, const Vec2i &goalPos);
	virtual void Parse(lua_State *lua, int startIndex, int endIndex);

private:
	int Damage;                             /// Missile damage.
	int TTL;                                /// Missile TTL.
	int Delay;                              /// Missile original delay.
	bool UseUnitVar;                        /// Use the caster's damage parameters
	SpellActionMissileLocation StartPoint;  /// Start point description.
	SpellActionMissileLocation EndPoint;    /// Start point description.
	MissileType *Missile;                   /// Missile fired on cast
};


//@}

#endif // SPELL_SPAWNMISSILE_H
