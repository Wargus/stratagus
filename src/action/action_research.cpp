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

#include "stratagus.h"

#include "action/action_research.h"

#include "ai.h"
#include "animation.h"
#include "iolib.h"
#include "script.h"
#include "sound.h"
#include "player.h"
#include "translate.h"
#include "unit.h"
#include "unitsound.h"
#include "unittype.h"
#include "upgrade_structs.h"
#include "upgrade.h"

/// How many resources the player gets back if canceling research
#define CancelResearchCostsFactor  100


/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/* static */ COrder *COrder::NewActionResearch(CUnit &unit, CUpgrade &upgrade)
{
	COrder_Research *order = new COrder_Research();

	// FIXME: if you give quick an other order, the resources are lost!
	unit.Player->SubCosts(upgrade.Costs);

	order->SetUpgrade(upgrade);
	return order;
}

/* virtual */ void COrder_Research::Save(CFile &file, const CUnit &unit) const
{
	file.printf("{\"action-research\"");

	if (this->Finished) {
		file.printf(" \"finished\", ");
	}
	if (this->Upgrade) {
		file.printf(", \"upgrade\", \"%s\"", this->Upgrade->Ident.c_str());
	}
	file.printf("}");
}

/* virtual */ bool COrder_Research::ParseSpecificData(lua_State *l, int &j, const char *value, const CUnit &unit)
{
	if (!strcmp(value, "upgrade")) {
		++j;
		this->Upgrade = CUpgrade::Get(LuaToString(l, -1, j + 1));
	} else {
		return false;
	}
	return true;
}

/* virtual */ bool COrder_Research::IsValid() const
{
	return true;
}

/* virtual */ PixelPos COrder_Research::Show(const CViewport & , const PixelPos &lastScreenPos) const
{
	return lastScreenPos;
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
/* virtual */ void COrder_Research::Execute(CUnit &unit)
{
	const CUpgrade &upgrade = this->GetUpgrade();
	const CUnitType &type = *unit.Type;


	UnitShowAnimation(unit, type.Animations->Research ? type.Animations->Research : type.Animations->Still);
	if (unit.Wait) {
		unit.Wait--;
		return ;
	}
#if 0
	if (unit.Anim.Unbreakable) {
		return ;
	}
#endif
	CPlayer &player = *unit.Player;
	player.UpgradeTimers.Upgrades[upgrade.ID] += std::max(1, player.SpeedResearch / SPEEDUP_FACTOR);
	if (player.UpgradeTimers.Upgrades[upgrade.ID] >= upgrade.Costs[TimeCost]) {
		if (upgrade.Name.empty()) {
			player.Notify(NotifyGreen, unit.tilePos, _("%s: research complete"), type.Name.c_str());
		} else {
			player.Notify(NotifyGreen, unit.tilePos, _("%s: research complete"), upgrade.Name.c_str());
		}
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
		this->Finished = true;
		return ;
	}
	unit.Wait = CYCLES_PER_SECOND / 6;
}

/* virtual */ void COrder_Research::Cancel(CUnit &unit)
{
	const CUpgrade &upgrade = this->GetUpgrade();
	unit.Player->UpgradeTimers.Upgrades[upgrade.ID] = 0;

	unit.Player->AddCostsFactor(upgrade.Costs, CancelResearchCostsFactor);
}

//@}
