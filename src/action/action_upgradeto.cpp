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
/**@name action_upgradeto.c - The unit upgrading to new action. */
//
//      (c) Copyright 1998-2005 by Lutz Sammer and Jimmy Salmon
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
//      $Id$

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "player.h"
#include "unittype.h"
#include "unit.h"
#include "actions.h"
#include "ai.h"
#include "interface.h"
#include "map.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Unit upgrades unit!
**
**  @param unit  Pointer to unit.
*/
void HandleActionUpgradeTo(Unit* unit)
{
	Player* player;
	UnitType* type;
	const UnitStats* stats;

	if (!unit->SubAction) { // first entry
		unit->Data.UpgradeTo.Ticks = 0;
		unit->SubAction = 1;
	}

	if (unit->Type->NewAnimations) {
		UnitShowNewAnimation(unit, unit->Type->NewAnimations->Upgrade);
		if (unit->Wait) {
			unit->Wait--;
			return;
		}
	}

	player = unit->Player;
	type = unit->Orders[0].Type;
	stats = &type->Stats[player->Player];

	// FIXME: Should count down here
	unit->Data.UpgradeTo.Ticks += SpeedUpgrade;
	if (unit->Data.UpgradeTo.Ticks >= stats->Costs[TimeCost]) {

		unit->HP += stats->HitPoints - unit->Type->Stats[player->Player].HitPoints;
		// don't have such unit now
		player->UnitTypesCount[unit->Type->Slot]--;

		Assert(unit->Type->TileWidth == type->TileWidth &&
			unit->Type->TileHeight == type->TileHeight);
		unit->Type = type;
		unit->Stats = (UnitStats*)stats;
		// and we have new one...
		player->UnitTypesCount[unit->Type->Slot]++;
		UpdateForNewUnit(unit, 1);

		NotifyPlayer(player, NotifyGreen, unit->X, unit->Y,
			"Upgrade to %s complete", unit->Type->Name);
		if (unit->Player->AiEnabled) {
			AiUpgradeToComplete(unit, type);
		}
		if (!unit->Type->NewAnimations) {
			unit->Reset = unit->Wait = 1;
		}
		unit->Orders[0].Action = UnitActionStill;
		unit->SubAction = 0;

		// Update Possible sight range change
		MapUnmarkUnitSight(unit);
		unit->CurrentSightRange = unit->Stats->SightRange;
		MapMarkUnitSight(unit);

		//
		// Update possible changed buttons.
		//
		if (IsOnlySelected(unit) || player == ThisPlayer) {
			// could affect the buttons of any selected unit
			SelectedUnitChanged();
		}

		return;
	}

	if (!unit->Type->NewAnimations) {
		unit->Reset = 1;
	}
	unit->Wait = CYCLES_PER_SECOND / 6;
}

//@}
