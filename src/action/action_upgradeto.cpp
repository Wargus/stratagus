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
/**@name action_upgradeto.cpp - The unit upgrading to new action. */
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "player.h"
#include "unittype.h"
#include "animation.h"
#include "unit.h"
#include "actions.h"
#include "ai.h"
#include "interface.h"
#include "map.h"
#include "spells.h"

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
static int TransformUnitIntoType(CUnit &unit, CUnitType &newtype)
{
	CUnitType *oldtype = unit.Type;
	if (oldtype == &newtype) { // nothing to do
		return 1;
	}
	const Vec2i pos = unit.tilePos + oldtype->GetHalfTileSize() - newtype.GetHalfTileSize();
	CUnit *container = unit.Container;

	if (container) {
		MapUnmarkUnitSight(unit);
	} else {
		SaveSelection();
		unit.Remove(NULL);
		if (!UnitTypeCanBeAt(newtype, pos)) {
			unit.Place(unit.tilePos);
			RestoreSelection();
			// FIXME unit is not modified, try later ?
			return 0;
		}
	}
	CPlayer &player = *unit.Player;
	player.UnitTypesCount[oldtype->Slot]--;
	player.UnitTypesCount[newtype.Slot]++;

	player.Demand += newtype.Demand - oldtype->Demand;
	player.Supply += newtype.Supply - oldtype->Supply;

	// Change resource limit
	for (int i = 0; i < MaxCosts; ++i) {
		if (player.MaxResources[i] != -1) {
			player.MaxResources[i] += newtype._Storing[i] - oldtype->_Storing[i];
			player.SetResource(i, player.Resources[i]);
		}
	}

	//  adjust Variables with percent.
	const CUnitStats &newstats = newtype.Stats[player.Index];

	for (unsigned int i = 0; i < UnitTypeVar.GetNumberVariable(); ++i) {
		if (unit.Variable[i].Max) {
			unit.Variable[i].Value = newstats.Variables[i].Max *
				unit.Variable[i].Value / unit.Variable[i].Max;
		} else {
			unit.Variable[i].Value = newstats.Variables[i].Value;
		}
		unit.Variable[i].Max = newstats.Variables[i].Max;
		unit.Variable[i].Increase = newstats.Variables[i].Increase;
		unit.Variable[i].Enable = newstats.Variables[i].Enable;
	}

	unit.Type = &newtype;
	unit.Stats = &newtype.Stats[player.Index];

	if (newtype.CanCastSpell && !unit.AutoCastSpell) {
		unit.AutoCastSpell = new char[SpellTypeTable.size()];
		memset(unit.AutoCastSpell, 0, SpellTypeTable.size() * sizeof(char));
	}

	UpdateForNewUnit(unit, 1);
	//  Update Possible sight range change
	UpdateUnitSightRange(unit);
	if (!container) {
		unit.Place(pos);
		RestoreSelection();
	} else {
		MapMarkUnitSight(unit);
	}
	//
	// Update possible changed buttons.
	//
	if (IsOnlySelected(unit) || &player == ThisPlayer) {
		// could affect the buttons of any selected unit
		SelectedUnitChanged();
	}
	return 1;
}

/**
**  Unit transform into unit.
**
**  @param unit  Pointer to unit.
*/
void HandleActionTransformInto(COrder& order, CUnit &unit)
{
	// What to do if an error occurs ?
	TransformUnitIntoType(unit, *order.Arg1.Type);
}

/**
**  Unit upgrades unit!
**
**  @param unit  Pointer to unit.
*/
void HandleActionUpgradeTo(COrder& order, CUnit &unit)
{
	if (!unit.SubAction) { // first entry
		order.Data.UpgradeTo.Ticks = 0;
		unit.SubAction = 1;
	}
	unit.Type->Animations->Upgrade ?
		UnitShowAnimation(unit, unit.Type->Animations->Upgrade) :
		UnitShowAnimation(unit, unit.Type->Animations->Still);
	if (unit.Wait) {
		unit.Wait--;
		return;
	}
	CPlayer *player = unit.Player;
	CUnitType &newtype = *order.Arg1.Type;
	const CUnitStats *newstats = &newtype.Stats[player->Index];

	// FIXME: Should count down here
	order.Data.UpgradeTo.Ticks += SpeedUpgrade;
	if (order.Data.UpgradeTo.Ticks < newstats->Costs[TimeCost]) {
		unit.Wait = CYCLES_PER_SECOND / 6;
		return;
	}

	unit.ClearAction();
	unit.State = 0;

	if (TransformUnitIntoType(unit, newtype) == 0) {
		player->Notify(NotifyGreen, unit.tilePos.x, unit.tilePos.y,
			_("Upgrade to %s canceled"), newtype.Name.c_str());
		return ;
	}
	//  Warn AI.
	if (player->AiEnabled) {
		AiUpgradeToComplete(unit, newtype);
	}
	player->Notify(NotifyGreen, unit.tilePos.x, unit.tilePos.y,
		_("Upgrade to %s complete"), unit.Type->Name.c_str());
}

//@}
