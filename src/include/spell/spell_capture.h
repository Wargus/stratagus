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

#ifndef SPELL_CAPTURE_H
#define SPELL_CAPTURE_H

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "spells.h"

class Spell_Capture : public SpellActionType
{
public:
	Spell_Capture() : SacrificeEnable(false), JoinToAIForce(false), Damage(0), DamagePercent(0) {};
	virtual int Cast(CUnit &caster, const SpellType &spell,
					 CUnit *target, const Vec2i &goalPos);
	virtual void Parse(lua_State *l, int startIndex, int endIndex);

private:
	bool SacrificeEnable; /// true if the caster dies after casting.
	bool JoinToAIForce;   /// if true, captured unit is joined into caster's AI force, if available
	int Damage;           /// damage the spell does if unable to caputre
	int DamagePercent;    /// percent the target must be damaged for a
	/// capture to succeed.
};

//@}

#endif // SPELL_CAPTURE_H
