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
/**@name action_research.c - The research action. */
//
//      (c) Copyright 1998-2004 by Lutz Sammer
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
#include "video.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "actions.h"
#include "upgrade_structs.h"
#include "upgrade.h"
#include "interface.h"
#include "ai.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Unit researches!
**
**  @param unit  Pointer of researching unit.
*/
void HandleActionResearch(Unit* unit)
{
	const Upgrade* upgrade;

	if (!unit->SubAction) { // first entry
		upgrade = unit->Data.Research.Upgrade = unit->Orders[0].Arg1.Upgrade;
#if 0
		// FIXME: I want to support both, but with network we need this check
		//  but if want combined upgrades this is worse

		//
		// Check if an other building has already started?
		//
		if (unit->Player->UpgradeTimers.Upgrades[upgrade - Upgrades]) {
			DebugPrint("Two researches running\n");
			PlayerAddCosts(unit->Player, upgrade->Costs);

			unit->Reset = unit->Wait = 1;
			unit->Orders[0].Action = UnitActionStill;
			unit->SubAction = 0;
			return;
		}
#endif
		unit->SubAction = 1;
	} else {
		upgrade = unit->Data.Research.Upgrade;
	}

	if (unit->Type->NewAnimations) {
		UnitShowNewAnimation(unit, unit->Type->NewAnimations->Research);
		if (unit->Wait) {
			unit->Wait--;
			return;
		}
	}

	unit->Player->UpgradeTimers.Upgrades[upgrade - Upgrades] += SpeedResearch;
	if (unit->Player->UpgradeTimers.Upgrades[upgrade-Upgrades] >=
			upgrade->Costs[TimeCost]) {

		NotifyPlayer(unit->Player, NotifyGreen, unit->X, unit->Y,
			"%s: complete", unit->Type->Name);
		if (unit->Player->AiEnabled) {
			AiResearchComplete(unit, upgrade);
		}
		UpgradeAcquire(unit->Player, upgrade);

		unit->Reset = unit->Wait = 1;
		unit->Orders[0].Action = UnitActionStill;
		unit->SubAction = 0;

		// Upgrade can change all
		SelectedUnitChanged();

		return;
	}

	unit->Reset = 1;
	unit->Wait = CYCLES_PER_SECOND / 6;

	// FIXME: should be animations here?
}

//@}
