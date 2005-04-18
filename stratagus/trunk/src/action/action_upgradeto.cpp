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
**  Transform an unit in another.
**
**  @param unit     unit to transform.
**  @param newtype  new type of the unit.
**
**  @return 0 on error, 1 if nothing happens, 2 else.
*/
static int TransformUnitIntoType(Unit* unit, UnitType* newtype)
{
	Player* player;
	UnitType* oldtype;
	const UnitStats* newstats;
	const UnitStats* oldstats;
	int x;
	int y;
	int i;

	Assert(unit);
	Assert(newtype);

	oldtype = unit->Type;
	if (oldtype == newtype) { // nothing to do
		return 1;
	}
	x = unit->X + (oldtype->TileWidth - newtype->TileWidth) / 2;
	y = unit->Y + (oldtype->TileHeight - newtype->TileHeight) / 2;

	SaveSelection();
	RemoveUnit(unit, NULL);
	if (!UnitTypeCanBeAt(newtype, x, y)) {
		PlaceUnit(unit, unit->X, unit->Y);
		RestoreSelection();
		return 0;
	}
	player = unit->Player;
	player->UnitTypesCount[oldtype->Slot]--;
	player->UnitTypesCount[newtype->Slot]++;

	player->Demand += newtype->Demand - oldtype->Demand;
	player->Supply += newtype->Supply - oldtype->Supply;

	//  Update Possible sight range change
	unit->CurrentSightRange = unit->Stats->Variables[SIGHTRANGE_INDEX].Max;

	unit->Orders[0].Action = UnitActionStill;
	unit->SubAction = 0;

	//  adjust Variables with percent.
	newstats = &newtype->Stats[player->Player];
	oldstats = &oldtype->Stats[player->Player];

	unit->HP = newstats->Variables[HP_INDEX].Max * unit->HP / oldstats->Variables[HP_INDEX].Max;
	if (oldstats->Variables[MANA_INDEX].Max) {
		unit->Variable[MANA_INDEX].Value = newstats->Variables[MANA_INDEX].Max *
			unit->Variable[MANA_INDEX].Value / oldstats->Variables[MANA_INDEX].Max;
	} else {
		unit->Variable[MANA_INDEX].Value = newstats->Variables[MANA_INDEX].Max;
	}
	for (i = 0; i < UnitTypeVar.NumberVariable; i++) {
		if (unit->Variable[i].Max) {
			unit->Variable[i].Value = newstats->Variables[i].Max *
				unit->Variable[i].Value / unit->Variable[i].Max;
		} else {
			unit->Variable[i].Value = newstats->Variables[i].Value;
		}
		unit->Variable[i].Max = newstats->Variables[i].Max;
		unit->Variable[i].Increase = newstats->Variables[i].Increase;
	}

	unit->Type = newtype;
	unit->Stats = &newtype->Stats[player->Player];
	UpdateForNewUnit(unit, 1);
	PlaceUnit(unit, x, y);
	RestoreSelection();
	//
	// Update possible changed buttons.
	//
	if (IsOnlySelected(unit) || unit->Player == ThisPlayer) {
		// could affect the buttons of any selected unit
		SelectedUnitChanged();
	}
	return 1;
}

/**
**  Unit upgrades unit!
**
**  @param unit  Pointer to unit.
*/
void HandleActionUpgradeTo(Unit* unit)
{
	Player* player;
	UnitType* newtype;
	const UnitStats* newstats;

	Assert(unit);

	if (!unit->SubAction) { // first entry
		unit->Data.UpgradeTo.Ticks = 0;
		unit->SubAction = 1;
	}

	unit->Type->NewAnimations->Upgrade ?
		UnitShowNewAnimation(unit, unit->Type->NewAnimations->Upgrade) :
		UnitShowNewAnimation(unit, unit->Type->NewAnimations->Still);
	if (unit->Wait) {
		unit->Wait--;
		return;
	}

	player = unit->Player;
	newtype = unit->Orders[0].Type;
	newstats = &newtype->Stats[player->Player];

	// FIXME: Should count down here
	unit->Data.UpgradeTo.Ticks += SpeedUpgrade;
	if (unit->Data.UpgradeTo.Ticks < newstats->Costs[TimeCost]) {
		unit->Wait = CYCLES_PER_SECOND / 6;
		return;
	}

	if (TransformUnitIntoType(unit, newtype) == 0) {
		NotifyPlayer(player, NotifyGreen, unit->X, unit->Y,
			"Upgrade to %s canceled", newtype->Name);
		// FIXME :
		return ;
	}
	//  Warn AI.
	if (player->AiEnabled) {
		AiUpgradeToComplete(unit, newtype);
	}
	NotifyPlayer(player, NotifyGreen, unit->X, unit->Y,
		"Upgrade to %s complete", unit->Type->Name);
}

//@}
