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

#ifndef SPELL_ADJUSTVARIABLE_H
#define SPELL_ADJUSTVARIABLE_H

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "spells.h"

class SpellActionTypeAdjustVariable
{
public:
	SpellActionTypeAdjustVariable() : Enable(0), Value(0), Max(0), Increase(0),
		ModifEnable(0), ModifValue(0), ModifMax(0), ModifIncrease(0),
		InvertEnable(0), AddValue(0), AddMax(0), AddIncrease(0), IncreaseTime(0),
		TargetIsCaster(0) {};

	int Enable;                 /// Value to affect to this field.
	int Value;                  /// Value to affect to this field.
	int Max;                    /// Value to affect to this field.
	int Increase;               /// Value to affect to this field.

	char ModifEnable;           /// true if we modify this field.
	char ModifValue;            /// true if we modify this field.
	char ModifMax;              /// true if we modify this field.
	char ModifIncrease;         /// true if we modify this field.

	char InvertEnable;          /// true if we invert this field.
	int AddValue;               /// Add this value to this field.
	int AddMax;                 /// Add this value to this field.
	int AddIncrease;            /// Add this value to this field.
	int IncreaseTime;           /// How many time increase the Value field.
	char TargetIsCaster;        /// true if the target is the caster.
};


class Spell_AdjustVariable : public SpellActionType
{
public:
	Spell_AdjustVariable() : Var(NULL) {};
	~Spell_AdjustVariable() { delete [](this->Var); };
	virtual int Cast(CUnit &caster, const SpellType &spell,
					 CUnit *target, const Vec2i &goalPos);
	virtual void Parse(lua_State *l, int startIndex, int endIndex);

private:
	SpellActionTypeAdjustVariable *Var;
};

//@}

#endif // SPELL_ADJUSTVARIABLE_H
