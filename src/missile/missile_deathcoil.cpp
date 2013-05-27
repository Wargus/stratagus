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
/**@name missile_deathcoil.cpp - The missile DeathCoil. */
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

#include "missile.h"

#include "actions.h"
#include "map.h"
#include "unit.h"
#include "unit_find.h"

/**
**  Death-Coil class. Damages organic units and gives to the caster.
**
**  @todo  do it configurable.
*/
void MissileDeathCoil::Action()
{
	this->Wait = this->Type->Sleep;
	if (PointToPointMissile(*this) == false) {
		return;
	}
	Assert(this->SourceUnit != NULL);
	CUnit &source = *this->SourceUnit;

	if (source.Destroyed) {
		return;
	}
	// source unit still exists
	//
	// Target unit still exists and casted on a special target
	//
	if (this->TargetUnit && !this->TargetUnit->Destroyed
		&& this->TargetUnit->CurrentAction() == UnitActionDie) {
		HitUnit(&source, *this->TargetUnit, this->Damage);
		if (source.CurrentAction() != UnitActionDie) {
			source.Variable[HP_INDEX].Value += this->Damage;
			source.Variable[HP_INDEX].Value = std::min(source.Variable[HP_INDEX].Max, source.Variable[HP_INDEX].Value);
		}
	} else {
		//
		// No target unit -- try enemies in range 5x5 // Must be parametrable
		//
		std::vector<CUnit *> table;
		const Vec2i destPos = Map.MapPixelPosToTilePos(this->destination);
		const Vec2i range(2, 2);
		Select(destPos - range, destPos + range, table, IsEnemyWith(*source.Player));

		if (table.empty()) {
			return;
		}
		const size_t n = table.size();  // enemy count
		const int damage = std::min<int>(1, this->Damage / n);

		// disperse damage between them
		for (size_t i = 0; i != n; ++i) {
			HitUnit(&source, *table[i], damage);
		}
		if (source.CurrentAction() != UnitActionDie) {
			source.Variable[HP_INDEX].Value += this->Damage;
			source.Variable[HP_INDEX].Value = std::min(source.Variable[HP_INDEX].Max, source.Variable[HP_INDEX].Value);
		}
	}
	this->TTL = 0;
}

//@}
