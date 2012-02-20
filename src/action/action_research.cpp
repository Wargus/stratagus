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
/**@name action_research.cpp - The research action. */
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
#include "sound.h"
#include "unitsound.h"
#include "unittype.h"
#include "animation.h"
#include "player.h"
#include "unit.h"
#include "actions.h"
#include "upgrade_structs.h"
#include "upgrade.h"
#include "ai.h"
#include "iolib.h"
#include "script.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/* virtual */ COrder_Research *COrder_Research::Clone() const
{
	return new COrder_Research(*this);
}

/* virtual */ void COrder_Research::Save(CFile &file, const CUnit &unit) const
{
	file.printf("{\"action-research\"");

	if (this->Upgrade) {
		file.printf(", \"upgrade\", \"%s\"", this->Upgrade->Ident.c_str());
	}
	file.printf("}");
}

/* virtual */ bool COrder_Research::ParseSpecificData(lua_State *l, int &j, const char *value, const CUnit &unit)
{
	if (!strcmp(value, "upgrade")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		this->Upgrade = CUpgrade::Get(LuaToString(l, -1));
		lua_pop(l, 1);
	} else {
		return false;
	}
	return true;
}


/* virtual */ void COrder_Research::UpdateUnitVariables(CUnit &unit) const
{
	unit.Variable[RESEARCH_INDEX].Value = unit.Player->UpgradeTimers.Upgrades[this->Upgrade->ID];
	unit.Variable[RESEARCH_INDEX].Max = this->Upgrade->Costs[TimeCost];
}

/**
**  Research upgrade.
**
**  @return true when finished.
*/
/* virtual */ bool COrder_Research::Execute(CUnit &unit)
{
	const CUpgrade &upgrade = this->GetUpgrade();
	const CUnitType &type = *unit.Type;

	type.Animations->Research ?
		UnitShowAnimation(unit, type.Animations->Research) :
		UnitShowAnimation(unit, type.Animations->Still);
	if (unit.Wait) {
		unit.Wait--;
		return false;
	}
#if 0
	if (unit.Anim.Unbreakable) {
		return false;
	}
#endif
	CPlayer &player = *unit.Player;
	player.UpgradeTimers.Upgrades[upgrade.ID] += SpeedResearch;
	if (player.UpgradeTimers.Upgrades[upgrade.ID] >= upgrade.Costs[TimeCost]) {
		player.Notify(NotifyGreen, unit.tilePos.x, unit.tilePos.y,
			_("%s: research complete"), type.Name.c_str());
		if (&player == ThisPlayer) {
			CSound *sound = GameSounds.ResearchComplete[player.Race].Sound;

			if (sound) {
				PlayGameSound(sound, MaxSampleVolume);
			}
		}
		if (player.AiEnabled) {
			AiResearchComplete(unit, &upgrade);
		}
		UpgradeAcquire(player, &upgrade);
		return true;
	}
	unit.Wait = CYCLES_PER_SECOND / 6;
	return false;
}

/* virtual */ void COrder_Research::Cancel(CUnit &unit)
{
	const CUpgrade &upgrade = this->GetUpgrade();
	unit.Player->UpgradeTimers.Upgrades[upgrade.ID] = 0;

	unit.Player->AddCostsFactor(upgrade.Costs, CancelResearchCostsFactor);
}


/**
**  Unit researches!
**
**  @param unit  Pointer of researching unit.
*/
void HandleActionResearch(COrder& order, CUnit &unit)
{
	Assert(order.Action == UnitActionResearch);

	if (order.Execute(unit)) {
		unit.ClearAction();
	}
}

//@}
