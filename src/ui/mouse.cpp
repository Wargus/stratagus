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
/**@name mouse.cpp - The mouse handling. */
//
//      (c) Copyright 1998-2015 by Lutz Sammer, Jimmy Salmon and Andrettin
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

#define ICON_SIZE_X (UI.ButtonPanel.Buttons[0].Style->Width)
#define ICON_SIZE_Y (UI.ButtonPanel.Buttons[0].Style->Height)

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <ctype.h>
#include <stdexcept>

#include "stratagus.h"

#include "ui.h"

#include "action/action_build.h"
#include "action/action_train.h"
#include "actions.h"
#include "commands.h"
#include "cursor.h"
#include "font.h"
#include "interface.h"
#include "map.h"
#include "menus.h"
#include "minimap.h"
#include "missile.h"
#include "network.h"
#include "player.h"
#include "sound.h"
#include "spells.h"
#include "translate.h"
#include "unit.h"
#include "unit_find.h"
#include "unitsound.h"
#include "unittype.h"
#include "video.h"
#include "widgets.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

int MouseButtons;                            /// Current pressed mouse buttons

int KeyModifiers;                            /// Current keyboard modifiers

CUnit *UnitUnderCursor;                      /// Unit under cursor
int ButtonAreaUnderCursor = -1;              /// Button area under cursor
int ButtonUnderCursor = -1;                  /// Button under cursor
int OldButtonUnderCursor = -1;               /// Button under cursor
bool GameMenuButtonClicked;                  /// Menu button was clicked
bool GameDiplomacyButtonClicked;             /// Diplomacy button was clicked
bool LeaveStops;                             /// Mouse leaves windows stops scroll

enum _cursor_on_ CursorOn = CursorOnUnknown; /// Cursor on field

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/
static void HandlePieMenuMouseSelection();

CUnit *GetUnitUnderCursor()
{
	return UnitUnderCursor;
}

/**
**  Cancel building cursor mode.
*/
void CancelBuildingMode()
{
	CursorBuilding = NULL;
	UI.StatusLine.Clear();
	UI.StatusLine.ClearCosts();
	CurrentButtonLevel = 0;
	UI.ButtonPanel.Update();
}

static bool CanBuildOnArea(const CUnit &unit, const Vec2i &pos)
{
	for (int j = 0; j < unit.Type->TileHeight; ++j) {
		for (int i = 0; i < unit.Type->TileWidth; ++i) {
			const Vec2i tempPos(i, j);
			if (!Map.Field(pos + tempPos)->playerInfo.IsExplored(*ThisPlayer)) {
				return false;
			}
		}
	}
	return true;
}

static void DoRightButton_ForForeignUnit(CUnit *dest)
{
	CUnit &unit = *Selected[0];

	if (unit.Player->Index != PlayerNumNeutral || dest == NULL
		|| !(dest->Player == ThisPlayer || dest->IsTeamed(*ThisPlayer))) {
		return;
	}
	// tell to go and harvest from a unit
	const int res = unit.Type->GivesResource;

	if (res
		&& dest->Type->BoolFlag[HARVESTER_INDEX].value
		&& dest->Type->ResInfo[res]
		&& dest->ResourcesHeld < dest->Type->ResInfo[res]->ResourceCapacity
		&& unit.Type->BoolFlag[CANHARVEST_INDEX].value) {
		unit.Blink = 4;
		//  Right mouse with SHIFT appends command to old commands.
		const int flush = !(KeyModifiers & ModifierShift);
		SendCommandResource(*dest, unit, flush);
	}
}

static bool DoRightButton_Transporter(CUnit &unit, CUnit *dest, int flush, int &acknowledged)
{
	//  Enter transporters ?
	if (dest == NULL) {
		return false;
	}
	// dest is the transporter
	if (dest->Type->CanTransport()) {
		// Let the transporter move to the unit. And QUEUE!!!
		if (dest->CanMove() && CanTransport(*dest, unit)) {
			DebugPrint("Send command follow\n");
			// is flush value correct ?
			if (!acknowledged) {
				PlayUnitSound(unit, VoiceAcknowledging);
				acknowledged = 1;
			}
			SendCommandFollow(*dest, unit, 0);
		}
		// FIXME : manage correctly production units.
		if (!unit.CanMove() || CanTransport(*dest, unit)) {
			dest->Blink = 4;
			DebugPrint("Board transporter\n");
			if (!acknowledged) {
				PlayUnitSound(unit, VoiceAcknowledging);
				acknowledged = 1;
			}
			SendCommandBoard(unit, *dest, flush);
			return true;
		}
	}
	//  unit is the transporter
	//  FIXME : Make it more configurable ? NumSelect == 1, lua option
	if (CanTransport(unit, *dest)) {
		// Let the transporter move to the unit. And QUEUE!!!
		if (unit.CanMove()) {
			DebugPrint("Send command follow\n");
			// is flush value correct ?
			if (!acknowledged) {
				PlayUnitSound(unit, VoiceAcknowledging);
				acknowledged = 1;
			}
			SendCommandFollow(unit, *dest, 0);
		} else if (!dest->CanMove()) {
			DebugPrint("Want to transport but no unit can move\n");
			return true;
		}
		dest->Blink = 4;
		DebugPrint("Board transporter\n");
		if (!acknowledged) {
			PlayUnitSound(unit, VoiceAcknowledging);
			acknowledged = 1;
		}
		SendCommandBoard(*dest, unit, flush);
		return true;
	}
	return false;
}

static bool DoRightButton_Harvest_Unit(CUnit &unit, CUnit &dest, int flush, int &acknowledged)
{
	// Return a loaded harvester to deposit
	if (unit.ResourcesHeld > 0
		&& dest.Type->CanStore[unit.CurrentResource]
		&& (dest.Player == unit.Player
			|| (dest.Player->IsAllied(*unit.Player) && unit.Player->IsAllied(*dest.Player)))) {
		const ResourceInfo &resinfo = *unit.Type->ResInfo[unit.CurrentResource];
		dest.Blink = 4;
		if (!acknowledged) {
			PlayUnitSound(unit, VoiceAcknowledging);
			acknowledged = 1;
		}
		SendCommandReturnGoods(unit, &dest, flush);
		return true;
	}
	// Go and harvest from a unit
	const int res = dest.Type->GivesResource;
	const CUnitType &type = *unit.Type;
	if (res && type.ResInfo[res] && dest.Type->BoolFlag[CANHARVEST_INDEX].value
		&& (dest.Player == unit.Player || dest.Player->Index == PlayerNumNeutral)) {
			if (unit.CurrentResource != res || unit.ResourcesHeld < type.ResInfo[res]->ResourceCapacity) {
				dest.Blink = 4;
				SendCommandResource(unit, dest, flush);
				if (!acknowledged) {
					PlayUnitSound(unit, VoiceHarvesting);
					acknowledged = 1;
				}
				return true;
			} else {
				CUnit *depot = FindDeposit(unit, 1000, unit.CurrentResource);
				if (depot) {
					dest.Blink = 4;
					if (!acknowledged) {
						PlayUnitSound(unit, VoiceAcknowledging);
						acknowledged = 1;
					}
					SendCommandReturnGoods(unit, depot, flush);
					return true;
				}
			}
	}
	return false;
}

static bool DoRightButton_Harvest_Pos(CUnit &unit, const Vec2i &pos, int flush, int &acknowledged)
{
	if (!Map.Field(pos)->playerInfo.IsExplored(*unit.Player)) {
		return false;
	}
	const CUnitType &type = *unit.Type;
	// FIXME: support harvesting more types of terrain.
	for (int res = 0; res < MaxCosts; ++res) {
		if (type.ResInfo[res]
			&& type.ResInfo[res]->TerrainHarvester
			&& Map.Field(pos)->IsTerrainResourceOnMap(res)) {
			if (unit.CurrentResource != res || unit.ResourcesHeld < type.ResInfo[res]->ResourceCapacity) {
				SendCommandResourceLoc(unit, pos, flush);
				if (!acknowledged) {
					PlayUnitSound(unit, VoiceHarvesting);
					acknowledged = 1;
				}
				return true;
			} else {
				CUnit *depot = FindDeposit(unit, 1000, unit.CurrentResource);
				if (depot) {
					if (!acknowledged) {
						PlayUnitSound(unit, VoiceAcknowledging);
						acknowledged = 1;
					}
					SendCommandReturnGoods(unit, depot, flush);
					return true;
				}
			}
		}
	}
	return false;
}

static bool DoRightButton_Worker(CUnit &unit, CUnit *dest, const Vec2i &pos, int flush, int &acknowledged)
{
	const CUnitType &type = *unit.Type;

	// Go and repair
	if (type.RepairRange && dest != NULL
		&& dest->Type->RepairHP
		&& dest->Variable[HP_INDEX].Value < dest->Variable[HP_INDEX].Max
		&& (dest->Player == unit.Player || unit.IsAllied(*dest))) {
		dest->Blink = 4;
		if (!acknowledged) {
			PlayUnitSound(unit, VoiceRepairing);
			acknowledged = 1;
		}
		SendCommandRepair(unit, pos, dest, flush);
		return true;
	}
	// Harvest
	if (type.BoolFlag[HARVESTER_INDEX].value) {
		if (dest != NULL) {
			if (DoRightButton_Harvest_Unit(unit, *dest, flush, acknowledged)) {
				return true;
			}
		} else {
			if (DoRightButton_Harvest_Pos(unit, pos, flush, acknowledged)) {
				return true;
			}
		}
	}
	// Follow another unit
	if (UnitUnderCursor != NULL && dest != NULL && dest != &unit
		&& (dest->Player == unit.Player || unit.IsAllied(*dest) || dest->Player->Index == PlayerNumNeutral)) {
		dest->Blink = 4;
		if (!acknowledged) {
			PlayUnitSound(unit, VoiceAcknowledging);
			acknowledged = 1;
		}
		if (dest->Type->CanMove() == false && !dest->Type->BoolFlag[TELEPORTER_INDEX].value) {
			SendCommandMove(unit, pos, flush);
		} else {
			SendCommandFollow(unit, *dest, flush);
		}
		return true;
	}
	// Move
	if (!acknowledged) {
		PlayUnitSound(unit, VoiceAcknowledging);
		acknowledged = 1;
	}
	SendCommandMove(unit, pos, flush);
	return true;
}

static bool DoRightButton_AttackUnit(CUnit &unit, CUnit &dest, const Vec2i &pos, int flush, int &acknowledged)
{
	const CUnitType &type = *unit.Type;
	const int action = type.MouseAction;

	if (action == MouseActionSpellCast || unit.IsEnemy(dest)) {
		dest.Blink = 4;
		if (!acknowledged) {
			PlayUnitSound(unit, VoiceAttack);
			acknowledged = 1;
		}
		if (action == MouseActionSpellCast) {
			// This is for demolition squads and such
			Assert(unit.Type->CanCastSpell);
			size_t spellnum;
			for (spellnum = 0; !type.CanCastSpell[spellnum] && spellnum < SpellTypeTable.size() ; spellnum++) {
			}
			SendCommandSpellCast(unit, pos, &dest, spellnum, flush);
		} else {
			if (CanTarget(type, *dest.Type)) {
				SendCommandAttack(unit, pos, &dest, flush);
			} else { // No valid target
				SendCommandAttack(unit, pos, NoUnitP, flush);
			}
		}
		return true;
	}
	if ((dest.Player == unit.Player || unit.IsAllied(dest) || dest.Player->Index == PlayerNumNeutral) && &dest != &unit) {
		dest.Blink = 4;
		if (!acknowledged) {
			PlayUnitSound(unit, VoiceAcknowledging);
			acknowledged = 1;
		}
		if (!dest.Type->CanMove() && !dest.Type->BoolFlag[TELEPORTER_INDEX].value) {
			SendCommandMove(unit, pos, flush);
		} else {
			SendCommandFollow(unit, dest, flush);
		}
		return true;
	}
	return false;
}

static void DoRightButton_Attack(CUnit &unit, CUnit *dest, const Vec2i &pos, int flush, int &acknowledged)
{
	if (dest != NULL && unit.CurrentAction() != UnitActionBuilt) {
		if (DoRightButton_AttackUnit(unit, *dest, pos, flush, acknowledged)) {
			return;
		}
	}
	if (Map.WallOnMap(pos)) {
		// XXX: hardcoded for humans and orcs
		if (PlayerRaces.Name[unit.Player->Race] == "human" && Map.OrcWallOnMap(pos)) {
			SendCommandAttack(unit, pos, NoUnitP, flush);
			return;
		}
		if (PlayerRaces.Name[unit.Player->Race] == "orc" && Map.HumanWallOnMap(pos)) {
			SendCommandAttack(unit, pos, NoUnitP, flush);
			return;
		}
	}
	// empty space
	if ((KeyModifiers & ModifierControl)) {
		if (RightButtonAttacks) {
			SendCommandMove(unit, pos, flush);
			if (!acknowledged) {
				PlayUnitSound(unit, VoiceAcknowledging);
				acknowledged = 1;
			}
		} else {
			if (!acknowledged) {
				PlayUnitSound(unit, VoiceAttack);
				acknowledged = 1;
			}
			SendCommandAttack(unit, pos, NoUnitP, flush);
		}
	} else {
		if (RightButtonAttacks) {
			if (!acknowledged) {
				PlayUnitSound(unit, VoiceAttack);
				acknowledged = 1;
			}
			SendCommandAttack(unit, pos, NoUnitP, flush);
		} else {
			// Note: move is correct here, right default is move
			if (!acknowledged) {
				PlayUnitSound(unit, VoiceAcknowledging);
				acknowledged = 1;
			}
			SendCommandMove(unit, pos, flush);
		}
	}
	// FIXME: ALT-RIGHT-CLICK, move but fight back if attacked.
}

static bool DoRightButton_Follow(CUnit &unit, CUnit &dest, int flush, int &acknowledged)
{
	if (dest.Player == unit.Player || unit.IsAllied(dest) || dest.Player->Index == PlayerNumNeutral) {
		dest.Blink = 4;
		if (!acknowledged) {
			PlayUnitSound(unit, VoiceAcknowledging);
			acknowledged = 1;
		}
		if (dest.Type->CanMove() == false && !dest.Type->BoolFlag[TELEPORTER_INDEX].value) {
			SendCommandMove(unit, dest.tilePos, flush);
		} else {
			SendCommandFollow(unit, dest, flush);
		}
		return true;
	}
	return false;
}

static bool DoRightButton_Harvest_Reverse(CUnit &unit, CUnit &dest, int flush, int &acknowledged)
{
	const CUnitType &type = *unit.Type;

	// tell to return a loaded harvester to deposit
	if (dest.ResourcesHeld > 0
		&& type.CanStore[dest.CurrentResource]
		&& dest.Player == unit.Player) {
		dest.Blink = 4;
		SendCommandReturnGoods(dest, &unit, flush);
		if (!acknowledged) {
			PlayUnitSound(unit, VoiceAcknowledging);
			acknowledged = 1;
		}
		return true;
	}
	// tell to go and harvest from a building
	const int res = type.GivesResource;
	if (res
		&& dest.Type->ResInfo[res]
		&& dest.ResourcesHeld < dest.Type->ResInfo[res]->ResourceCapacity
		&& type.BoolFlag[CANHARVEST_INDEX].value
		&& dest.Player == unit.Player) {
		unit.Blink = 4;
		SendCommandResource(dest, unit, flush);
		return true;
	}
	return false;
}

static bool DoRightButton_NewOrder(CUnit &unit, CUnit *dest, const Vec2i &pos, int flush, int &acknowledged)
{
	// Go and harvest from a unit
	if (dest != NULL && dest->Type->GivesResource && dest->Type->BoolFlag[CANHARVEST_INDEX].value
		&& (dest->Player == unit.Player || dest->Player->Index == PlayerNumNeutral)) {
		dest->Blink = 4;
		if (!acknowledged) {
			PlayUnitSound(unit, VoiceAcknowledging);
			acknowledged = 1;
		}
		SendCommandResource(unit, *dest, flush);
		return true;
	}
	// FIXME: support harvesting more types of terrain.
	const CMapField &mf = *Map.Field(pos);
	if (mf.playerInfo.IsExplored(*unit.Player) && mf.IsTerrainResourceOnMap()) {
		if (!acknowledged) {
			PlayUnitSound(unit, VoiceAcknowledging);
			acknowledged = 1;
		}
		SendCommandResourceLoc(unit, pos, flush);
		return true;
	}
	return false;
}

static void DoRightButton_ForSelectedUnit(CUnit &unit, CUnit *dest, const Vec2i &pos, int &acknowledged)
{
	// don't self targeting.
	if (dest == &unit) {
		return;
	}
	if (unit.Removed) {
		return;
	}
	const CUnitType &type = *unit.Type;
	const int action = type.MouseAction;
	//  Right mouse with SHIFT appends command to old commands.
	const int flush = !(KeyModifiers & ModifierShift);

	//  Control + alt click - ground attack
	if ((KeyModifiers & ModifierControl) && (KeyModifiers & ModifierAlt)) {
		if (unit.Type->BoolFlag[GROUNDATTACK_INDEX].value) {
			if (!acknowledged) {
				PlayUnitSound(unit, VoiceAttack);
				acknowledged = 1;
			}
			SendCommandAttackGround(unit, pos, flush);
			return;
		}
	}
	//  Control + right click on unit is follow anything.
	if ((KeyModifiers & ModifierControl) && dest) {
		dest->Blink = 4;
		if (!acknowledged) {
			PlayUnitSound(unit, VoiceAcknowledging);
			acknowledged = 1;
		}
		SendCommandFollow(unit, *dest, flush);
		return;
	}

	//  Alt + right click on unit is defend anything.
	if ((KeyModifiers & ModifierAlt) && dest) {
		dest->Blink = 4;
		if (!acknowledged) {
			PlayUnitSound(unit, VoiceAcknowledging);
			acknowledged = 1;
		}
		SendCommandDefend(unit, *dest, flush);
		return;
	}

	if (DoRightButton_Transporter(unit, dest, flush, acknowledged)) {
		return;
	}

	//  Handle resource workers.
	if (action == MouseActionHarvest) {
		DoRightButton_Worker(unit, dest, pos, flush, acknowledged);
		return;
	}

	//  Fighters
	if (action == MouseActionSpellCast || action == MouseActionAttack) {
		DoRightButton_Attack(unit, dest, pos, flush, acknowledged);
		return;
	}

	// FIXME: attack/follow/board ...
	if (dest != NULL && (action == MouseActionMove || action == MouseActionSail)) {
		if (DoRightButton_Follow(unit, *dest, flush, acknowledged)) {
			return;
		}
	}

	// Manage harvester from the destination side.
	if (dest != NULL && dest->Type->BoolFlag[HARVESTER_INDEX].value) {
		if (DoRightButton_Harvest_Reverse(unit, *dest, flush, acknowledged)) {
			return;
		}
	}

	// Manage new order.
	if (!unit.CanMove()) {
		if (DoRightButton_NewOrder(unit, dest, pos, flush, acknowledged)) {
			return;
		}
	}
	if (!acknowledged) {
		PlayUnitSound(unit, VoiceAcknowledging);
		acknowledged = 1;
	}
	SendCommandMove(unit, pos, flush);
}

/**
**  Called when right button is pressed
**
**  @param mapPixelPos  map position in pixels.
*/
void DoRightButton(const PixelPos &mapPixelPos)
{
	// No unit selected
	if (Selected.empty()) {
		return;
	}
	const Vec2i pos = Map.MapPixelPosToTilePos(mapPixelPos);
	CUnit *dest;            // unit under the cursor if any.

	if (UnitUnderCursor != NULL && !UnitUnderCursor->Type->BoolFlag[DECORATION_INDEX].value) {
		dest = UnitUnderCursor;
	} else {
		dest = NULL;
	}

	//  Unit selected isn't owned by the player.
	//  You can't select your own units + foreign unit(s)
	//  except if it is neutral and it is a resource.
	if (!CanSelectMultipleUnits(*Selected[0]->Player)) {
		DoRightButton_ForForeignUnit(dest);
		return;
	}

	if (dest != NULL && dest->Type->CanTransport()) {
		for (size_t i = 0; i != Selected.size(); ++i) {
			if (CanTransport(*dest, *Selected[i])) {
				// We are clicking on a transporter. We have to:
				// 1) Flush the transporters orders.
				// 2) Tell the transporter to follow the units. We have to queue all
				//    these follow orders, so the transport will go and pick them up
				// 3) Tell all selected land units to go board the transporter.

				// Here we flush the order queue
				SendCommandStopUnit(*dest);
				break;
			}
		}
	}

	int acknowledged = 0; // to play sound
	for (size_t i = 0; i != Selected.size(); ++i) {
		Assert(Selected[i]);
		CUnit &unit = *Selected[i];

		DoRightButton_ForSelectedUnit(unit, dest, pos, acknowledged);
	}
	ShowOrdersCount = GameCycle + Preference.ShowOrders * CYCLES_PER_SECOND;
}

/**
**  Check if the mouse is on a button
**
**  @param screenPos  screen coordinate.
**
**  @return True if mouse is on the button, False otherwise.
*/
bool CUIButton::Contains(const PixelPos &screenPos) const
{
	Assert(this->Style);

	return this->X <= screenPos.x && screenPos.x < this->X + this->Style->Width
		   && this->Y <= screenPos.y && screenPos.y < this->Y + this->Style->Height;
}

/**
**  Set flag on which area is the cursor.
**
**  @param screenPos  screen position.
*/
static void HandleMouseOn(const PixelPos screenPos)
{
	MouseScrollState = ScrollNone;
	ButtonAreaUnderCursor = -1;
	ButtonUnderCursor = -1;

	if (IsDemoMode()) {
		// If we are in "demo mode", do nothing.
		return;
	}

	// BigMapMode is the mode which show only the map (without panel, minimap)
	if (BigMapMode) {
		CursorOn = CursorOnMap;
		//  Scrolling Region Handling.
		HandleMouseScrollArea(screenPos);
		return;
	}

	//  Handle buttons
	if (!IsNetworkGame()) {
		if (UI.MenuButton.X != -1) {
			if (UI.MenuButton.Contains(screenPos)) {
				ButtonAreaUnderCursor = ButtonAreaMenu;
				ButtonUnderCursor = ButtonUnderMenu;
				CursorOn = CursorOnButton;
				return;
			}
		}
	} else {
		if (UI.NetworkMenuButton.X != -1) {
			if (UI.NetworkMenuButton.Contains(screenPos)) {
				ButtonAreaUnderCursor = ButtonAreaMenu;
				ButtonUnderCursor = ButtonUnderNetworkMenu;
				CursorOn = CursorOnButton;
				return;
			}
		}
		if (UI.NetworkDiplomacyButton.X != -1) {
			if (UI.NetworkDiplomacyButton.Contains(screenPos)) {
				ButtonAreaUnderCursor = ButtonAreaMenu;
				ButtonUnderCursor = ButtonUnderNetworkDiplomacy;
				CursorOn = CursorOnButton;
				return;
			}
		}
	}
	if (UI.Resources[FreeWorkersCount].TextX != -1) {
		int x, y;
		CGraphic* g = UI.Resources[FreeWorkersCount].G;
		if (g && (x = UI.Resources[FreeWorkersCount].IconX) && (y = UI.Resources[FreeWorkersCount].IconY)) {
			int textX = UI.Resources[FreeWorkersCount].TextX;
			if (textX > 0 || ThisPlayer->GetFreeWorkersCount() > 0) {
				if (screenPos > PixelPos(x, y) && screenPos < PixelPos(x + g->getWidth(), y + g->getHeight())) {
					ButtonAreaUnderCursor = ButtonAreaMenu;
					ButtonUnderCursor = ButtonUnderFreeWorkers;
					CursorOn = CursorOnButton;
					return;
				}
			}
		}
	}
	for (size_t i = 0; i < UI.UserButtons.size(); ++i) {
		const CUIUserButton &button = UI.UserButtons[i];

		if (button.Button.X != -1) {
			if (button.Button.Contains(screenPos)) {
				ButtonAreaUnderCursor = ButtonAreaUser;
				ButtonUnderCursor = i;
				CursorOn = CursorOnButton;
				return;
			}
		}

	}
	const size_t buttonCount = UI.ButtonPanel.Buttons.size();
	for (unsigned int j = 0; j < buttonCount; ++j) {
		if (UI.ButtonPanel.Buttons[j].Contains(screenPos)) {
			ButtonAreaUnderCursor = ButtonAreaButton;
			if (!CurrentButtons.empty() && CurrentButtons[j].Pos != -1) {
				ButtonUnderCursor = j;
				CursorOn = CursorOnButton;
				return;
			}
		}
	}
	if (!Selected.empty()) {
		if (Selected.size() == 1 && Selected[0]->Type->CanTransport() && Selected[0]->BoardCount) {
			const size_t size = UI.TransportingButtons.size();

			for (size_t i = std::min<size_t>(Selected[0]->BoardCount, size); i != 0;) {
				--i;
				if (UI.TransportingButtons[i].Contains(screenPos)) {
					ButtonAreaUnderCursor = ButtonAreaTransporting;
					ButtonUnderCursor = i;
					CursorOn = CursorOnButton;
					return;
				}
			}
		}
		if (Selected.size() == 1) {
			if (Selected[0]->CurrentAction() == UnitActionTrain) {
				if (Selected[0]->Orders.size() == 1) {
					if (UI.SingleTrainingButton->Contains(screenPos)) {
						ButtonAreaUnderCursor = ButtonAreaTraining;
						ButtonUnderCursor = 0;
						CursorOn = CursorOnButton;
						return;
					}
				} else {
					const size_t size = UI.TrainingButtons.size();

					for (size_t i = std::min(Selected[0]->Orders.size(), size); i != 0;) {
						--i;
						if (Selected[0]->Orders[i]->Action == UnitActionTrain
							&& UI.TrainingButtons[i].Contains(screenPos)) {
							ButtonAreaUnderCursor = ButtonAreaTraining;
							ButtonUnderCursor = i;
							CursorOn = CursorOnButton;
							return;
						}
					}
				}
			} else if (Selected[0]->CurrentAction() == UnitActionUpgradeTo) {
				if (UI.UpgradingButton->Contains(screenPos)) {
					ButtonAreaUnderCursor = ButtonAreaUpgrading;
					ButtonUnderCursor = 0;
					CursorOn = CursorOnButton;
					return;
				}
			} else if (Selected[0]->CurrentAction() == UnitActionResearch) {
				if (UI.ResearchingButton->Contains(screenPos)) {
					ButtonAreaUnderCursor = ButtonAreaResearching;
					ButtonUnderCursor = 0;
					CursorOn = CursorOnButton;
					return;
				}
			}
		}
		if (Selected.size() == 1) {
			if (UI.SingleSelectedButton && UI.SingleSelectedButton->Contains(screenPos)) {
				ButtonAreaUnderCursor = ButtonAreaSelected;
				ButtonUnderCursor = 0;
				CursorOn = CursorOnButton;
				return;
			}
		} else {
			const size_t size = UI.SelectedButtons.size();

			for (size_t i = std::min(Selected.size(), size); i != 0;) {
				--i;
				if (UI.SelectedButtons[i].Contains(screenPos)) {
					ButtonAreaUnderCursor = ButtonAreaSelected;
					ButtonUnderCursor = i;
					CursorOn = CursorOnButton;
					return;
				}
			}
		}
	}

	//  Minimap
	if (UI.Minimap.Contains(screenPos)) {
		CursorOn = CursorOnMinimap;
		return;
	}

	//  On UI graphic
	bool on_ui = false;
	const size_t size = UI.Fillers.size();
	for (unsigned int j = 0; j < size; ++j) {
		if (UI.Fillers[j].OnGraphic(screenPos.x, screenPos.y)) {
			on_ui = true;
			break;
		}
	}

	//  Map
	if (!on_ui && UI.MapArea.Contains(screenPos)) {
		CViewport *vp = GetViewport(screenPos);
		Assert(vp);
		// viewport changed
		if (UI.MouseViewport != vp) {
			UI.MouseViewport = vp;
			DebugPrint("current viewport changed to %ld.\n" _C_
					   static_cast<long int>(vp - UI.Viewports));
		}

		// Note cursor on map can be in scroll area
		CursorOn = CursorOnMap;
	} else {
		CursorOn = CursorOnUnknown;
	}

	//  Scrolling Region Handling.
	HandleMouseScrollArea(screenPos);
}

/**
**  Handle cursor exits the game window (only for some videomodes)
**  @todo FIXME: make it so that the game is partially 'paused'.
**         Game should run (for network play), but not react on or show
**         interactive events.
*/
void HandleMouseExit()
{
	// Disabled
	if (!LeaveStops) {
		return;
	}
	// Denote cursor not on anything in window (used?)
	CursorOn = CursorOnUnknown;

	// Prevent scrolling while out of focus (on other applications) */
	KeyScrollState = MouseScrollState = ScrollNone;

	// Show hour-glass (to denote to the user, the game is waiting)
	// FIXME: couldn't define a hour-glass that easily, so used pointer
	CursorScreenPos.x = Video.Width / 2;
	CursorScreenPos.y = Video.Height / 2;
	GameCursor = UI.Point.Cursor;
}

/**
**  Restrict mouse cursor to viewport.
*/
void RestrictCursorToViewport()
{
	UI.SelectedViewport->Restrict(CursorScreenPos.x, CursorScreenPos.y);
	UI.MouseWarpPos = CursorStartScreenPos = CursorScreenPos;
	CursorOn = CursorOnMap;
}

/**
**  Restrict mouse cursor to minimap
*/
void RestrictCursorToMinimap()
{
	clamp(&CursorScreenPos.x, UI.Minimap.X, UI.Minimap.X + UI.Minimap.W - 1);
	clamp(&CursorScreenPos.y, UI.Minimap.Y, UI.Minimap.Y + UI.Minimap.H - 1);

	UI.MouseWarpPos = CursorStartScreenPos = CursorScreenPos;
	CursorOn = CursorOnMinimap;
}

/**
**  Use the mouse to scroll the map
**
**  @param pos  Screen position.
*/
static void MouseScrollMap(const PixelPos &pos)
{
	const int speed = (KeyModifiers & ModifierControl) ? UI.MouseScrollSpeedControl : UI.MouseScrollSpeedDefault;
	const PixelDiff diff(pos - CursorStartScreenPos);

	UI.MouseViewport->Set(UI.MouseViewport->MapPos, UI.MouseViewport->Offset + speed * diff);
	UI.MouseWarpPos = CursorStartScreenPos;
}

/**
**  Handle movement of the cursor.
**
**  @param cursorPos  Screen X position.
*/
void UIHandleMouseMove(const PixelPos &cursorPos)
{
	enum _cursor_on_ OldCursorOn;

	if (IsDemoMode()) {
		// If we are in "demo mode", exit now.
		void ActionDraw();
		ActionDraw();
		return;
	}

	OldCursorOn = CursorOn;
	//  Selecting units.
	if (CursorState == CursorStateRectangle) {
		// Restrict cursor to viewport.
		UI.SelectedViewport->Restrict(CursorScreenPos.x, CursorScreenPos.y);
		UI.MouseWarpPos = CursorScreenPos;
		return;
	}

	//  Move map.
	if (GameCursor == UI.Scroll.Cursor) {
		MouseScrollMap(cursorPos);
		return;
	}

	UnitUnderCursor = NULL;
	GameCursor = UI.Point.Cursor;  // Reset
	HandleMouseOn(cursorPos);

	//  Make the piemenu "follow" the mouse
	if (CursorState == CursorStatePieMenu && CursorOn == CursorOnMap) {
		clamp(&CursorStartScreenPos.x, CursorScreenPos.x - UI.PieMenu.X[2], CursorScreenPos.x + UI.PieMenu.X[2]);
		clamp(&CursorStartScreenPos.y, CursorScreenPos.y - UI.PieMenu.Y[4], CursorScreenPos.y + UI.PieMenu.Y[4]);
		return;
	}

	// Restrict mouse to minimap when dragging
	if (OldCursorOn == CursorOnMinimap && CursorOn != CursorOnMinimap && (MouseButtons & LeftButton)) {
		const Vec2i cursorPos = UI.Minimap.ScreenToTilePos(CursorScreenPos);

		RestrictCursorToMinimap();
		UI.SelectedViewport->Center(Map.TilePosToMapPixelPos_Center(cursorPos));
		return;
	}

	//  User may be draging with button pressed.
	if (GameMenuButtonClicked || GameDiplomacyButtonClicked) {
		return;
	} else {
		for (size_t i = 0; i < UI.UserButtons.size(); ++i) {
			const CUIUserButton &button = UI.UserButtons[i];

			if (button.Clicked) {
				return;
			}
		}
	}

	// This is forbidden for unexplored and not visible space
	// FIXME: This must done new, moving units, scrolling...
	if (CursorOn == CursorOnMap && UI.MouseViewport->IsInsideMapArea(CursorScreenPos)) {
		const CViewport &vp = *UI.MouseViewport;
		const Vec2i tilePos = vp.ScreenToTilePos(cursorPos);

		try {
			if (CursorBuilding && (MouseButtons & LeftButton) && Selected.at(0)
				&& (KeyModifiers & (ModifierAlt | ModifierShift))) {
				const CUnit &unit = *Selected[0];
				const Vec2i tilePos = UI.MouseViewport->ScreenToTilePos(CursorScreenPos);
				bool explored = CanBuildOnArea(*Selected[0], tilePos);

				// We now need to check if there are another build commands on this build spot
				bool buildable = true;
				for (std::vector<COrderPtr>::const_iterator it = unit.Orders.begin();
					 it != unit.Orders.end(); ++it) {
					COrder &order = **it;
					if (order.Action == UnitActionBuild) {
						COrder_Build &build = dynamic_cast<COrder_Build &>(order);
						if (tilePos.x >= build.GetGoalPos().x
							&& tilePos.x < build.GetGoalPos().x + build.GetUnitType().TileWidth
							&& tilePos.y >= build.GetGoalPos().y
							&& tilePos.y < build.GetGoalPos().y + build.GetUnitType().TileHeight) {
							buildable = false;
							break;
						}
					}
				}

				// 0 Test build, don't really build
				if (CanBuildUnitType(Selected[0], *CursorBuilding, tilePos, 0) && buildable && (explored || ReplayRevealMap)) {
					const int flush = !(KeyModifiers & ModifierShift);
					for (size_t i = 0; i != Selected.size(); ++i) {
						SendCommandBuildBuilding(*Selected[i], tilePos, *CursorBuilding, flush);
					}
					if (!(KeyModifiers & (ModifierAlt | ModifierShift))) {
						CancelBuildingMode();
					}
				}
			}
		} catch (const std::out_of_range &oor) {
			DebugPrint("Selected is empty: %s\n" _C_ oor.what());
		}
		if (Preference.ShowNameDelay) {
			ShowNameDelay = GameCycle + Preference.ShowNameDelay;
			ShowNameTime = GameCycle + Preference.ShowNameDelay + Preference.ShowNameTime;
		}

		bool show = ReplayRevealMap ? true : false;
		if (show == false) {
			CMapField &mf = *Map.Field(tilePos);
			for (int i = 0; i < PlayerMax; ++i) {
				if (mf.playerInfo.IsExplored(Players[i])
					&& (i == ThisPlayer->Index || Players[i].HasSharedVisionWith(*ThisPlayer))) {
					show = true;
					break;
				}
			}
		}

		if (show) {
			const PixelPos mapPixelPos = vp.ScreenToMapPixelPos(cursorPos);
			UnitUnderCursor = UnitOnScreen(mapPixelPos.x, mapPixelPos.y);
		}
	} else if (CursorOn == CursorOnMinimap) {
		const Vec2i tilePos = UI.Minimap.ScreenToTilePos(cursorPos);

		if (Map.Field(tilePos)->playerInfo.IsExplored(*ThisPlayer) || ReplayRevealMap) {
			UnitUnderCursor = UnitOnMapTile(tilePos, -1);
		}
	}

	// NOTE: If unit is not selectable as a goal, you can't get a cursor hint
	if (UnitUnderCursor != NULL && !UnitUnderCursor->IsVisibleAsGoal(*ThisPlayer) &&
		!ReplayRevealMap) {
		UnitUnderCursor = NULL;
	}

	//  Selecting target.
	if (CursorState == CursorStateSelect) {
		if (CursorOn == CursorOnMap || CursorOn == CursorOnMinimap) {
			if (CustomCursor.length() && CursorByIdent(CustomCursor)) {
				GameCursor = CursorByIdent(CustomCursor);
			} else {
				GameCursor = UI.YellowHair.Cursor;
			}
			if (UnitUnderCursor != NULL && !UnitUnderCursor->Type->BoolFlag[DECORATION_INDEX].value) {
				if (UnitUnderCursor->Player == ThisPlayer ||
					ThisPlayer->IsAllied(*UnitUnderCursor)) {
					if (CustomCursor.length() && CursorByIdent(CustomCursor)) {
						GameCursor = CursorByIdent(CustomCursor);
					} else {
						GameCursor = UI.YellowHair.Cursor;
					}
				} else if (UnitUnderCursor->Player->Index != PlayerNumNeutral) {
					if (CustomCursor.length() && CursorByIdent(CustomCursor)) {
						GameCursor = CursorByIdent(CustomCursor);
					} else {
						GameCursor = UI.YellowHair.Cursor;
					}
				}
			}
			if (CursorOn == CursorOnMinimap && (MouseButtons & RightButton)) {
				const Vec2i cursorPos = UI.Minimap.ScreenToTilePos(CursorScreenPos);
				//  Minimap move viewpoint
				UI.SelectedViewport->Center(Map.TilePosToMapPixelPos_Center(cursorPos));
			}
		}
		// FIXME: must move minimap if right button is down !
		return;
	}

	//  Cursor pointing.
	if (CursorOn == CursorOnMap) {
		//  Map
		if (UnitUnderCursor != NULL && !UnitUnderCursor->Type->BoolFlag[DECORATION_INDEX].value
			&& (UnitUnderCursor->IsVisible(*ThisPlayer) || ReplayRevealMap)) {
			GameCursor = UI.Glass.Cursor;
		}
		return;
	}

	if (CursorOn == CursorOnMinimap && (MouseButtons & LeftButton)) {
		//  Minimap move viewpoint
		const Vec2i cursorPos = UI.Minimap.ScreenToTilePos(CursorScreenPos);

		UI.SelectedViewport->Center(Map.TilePosToMapPixelPos_Center(cursorPos));
		CursorStartScreenPos = CursorScreenPos;
		return;
	}
}

//.............................................................................

/**
**  Send selected units to repair
**
**  @param tilePos  tile map position.
*/
static int SendRepair(const Vec2i &tilePos)
{
	CUnit *dest = UnitUnderCursor;
	int ret = 0;

	// Check if the dest is repairable!
	if (dest && dest->Variable[HP_INDEX].Value < dest->Variable[HP_INDEX].Max
		&& dest->Type->RepairHP
		&& (dest->Player == ThisPlayer || ThisPlayer->IsAllied(*dest))) {
		for (size_t i = 0; i != Selected.size(); ++i) {
			CUnit *unit = Selected[i];

			if (unit->Type->RepairRange) {
				const int flush = !(KeyModifiers & ModifierShift);

				SendCommandRepair(*unit, tilePos, dest, flush);
				ret = 1;
			} else {
				DebugPrint("Non-worker repairs\n");
			}
		}
	}
	return ret;
}

/**
**  Send selected units to point.
**
**  @param tilePos  tile map position.
**
**  @todo To reduce the CPU load for pathfinder, we should check if
**        the destination is reachable and handle nice group movements.
*/
static int SendMove(const Vec2i &tilePos)
{
	CUnit *goal = UnitUnderCursor;
	int ret = 0;
	const int flush = !(KeyModifiers & ModifierShift);

	// Alt makes unit to defend goal
	if (goal && (KeyModifiers & ModifierAlt)) {
		for (size_t i = 0; i != Selected.size(); ++i) {
			CUnit *unit = Selected[i];

			goal->Blink = 4;
			SendCommandDefend(*unit, *goal, flush);
			ret = 1;
		}
	} else {
		// Move to a transporter.
		if (goal && goal->Type->CanTransport()) {
			size_t i;
			for (i = 0; i != Selected.size(); ++i) {
				if (CanTransport(*goal, *Selected[i])) {
					SendCommandStopUnit(*goal);
					ret = 1;
					break;
				}
			}
			if (i == Selected.size()) {
				goal = NULL;
			}
		} else {
			goal = NULL;
		}

		for (size_t i = 0; i != Selected.size(); ++i) {
			CUnit *unit = Selected[i];

			if (goal && CanTransport(*goal, *unit)) {
				goal->Blink = 4;
				SendCommandFollow(*goal, *unit, 0);
				SendCommandBoard(*unit, *goal, flush);
				ret = 1;
			} else {
				SendCommandMove(*unit, tilePos, flush);
				ret = 1;
			}
		}
	}
	return ret;
}

/**
**  Send the current selected group attacking.
**
**  To empty field:
**    Move to this field attacking all enemy units in reaction range.
**
**  To unit:
**    Move to unit attacking and tracing the unit until dead.
**
**  @param tilePos  tile map position.
**
**  @return 1 if any unit have a new order, 0 else.
**
**  @see Selected
*/
static int SendAttack(const Vec2i &tilePos)
{
	const int flush = !(KeyModifiers & ModifierShift);
	CUnit *dest = UnitUnderCursor;
	int ret = 0;

	if (dest && dest->Type->BoolFlag[DECORATION_INDEX].value) {
		dest = NULL;
	}
	for (size_t i = 0; i != Selected.size(); ++i) {
		CUnit &unit = *Selected[i];

		if (unit.Type->CanAttack) {
			if (!dest || (dest != &unit && CanTarget(*unit.Type, *dest->Type))) {
				if (dest) {
					dest->Blink = 4;
				}
				SendCommandAttack(unit, tilePos, dest, flush);
				ret = 1;
			}
		} else {
			if (unit.CanMove()) {
				SendCommandMove(unit, tilePos, flush);
				ret = 1;
			}
		}
	}
	return ret;
}

/**
**  Send the current selected group ground attacking.
**
**  @param tilePos  tile map position.
*/
static int SendAttackGround(const Vec2i &tilePos)
{
	const int flush = !(KeyModifiers & ModifierShift);
	int ret = 0;

	for (size_t i = 0; i != Selected.size(); ++i) {
		CUnit &unit = *Selected[i];
		if (unit.Type->CanAttack) {
			SendCommandAttackGround(unit, tilePos, flush);
			ret = 1;
		} else {
			SendCommandMove(unit, tilePos, flush);
			ret = 1;
		}
	}
	return ret;
}

/**
**  Let units patrol between current position and the selected.
**
**  @param tilePos  tile map position.
*/
static int SendPatrol(const Vec2i &tilePos)
{
	const int flush = !(KeyModifiers & ModifierShift);

	for (size_t i = 0; i != Selected.size(); ++i) {
		CUnit &unit = *Selected[i];
		SendCommandPatrol(unit, tilePos, flush);
	}
	return Selected.empty() ? 0 : 1;
}

/**
**  Let units harvest wood/mine gold/haul oil
**
**  @param pos  tile map position
**
**  @see Selected
*/
static int SendResource(const Vec2i &pos)
{
	int res;
	CUnit *dest = UnitUnderCursor;
	int ret = 0;
	const int flush = !(KeyModifiers & ModifierShift);
	const CMapField &mf = *Map.Field(pos);

	for (size_t i = 0; i != Selected.size(); ++i) {
		CUnit &unit = *Selected[i];

		if (unit.Type->BoolFlag[HARVESTER_INDEX].value) {
			if (dest
				&& (res = dest->Type->GivesResource) != 0
				&& unit.Type->ResInfo[res]
				&& unit.ResourcesHeld < unit.Type->ResInfo[res]->ResourceCapacity
				&& dest->Type->BoolFlag[CANHARVEST_INDEX].value
				&& (dest->Player == unit.Player || dest->Player->Index == PlayerMax - 1)) {
				dest->Blink = 4;
				SendCommandResource(unit, *dest, flush);
				ret = 1;
				continue;
			} else {
				for (res = 0; res < MaxCosts; ++res) {
					if (unit.Type->ResInfo[res]
						&& unit.Type->ResInfo[res]->TerrainHarvester
						&& mf.playerInfo.IsExplored(*unit.Player)
						/// By disabling this, we allow the harvester to find the nearest tile with a resource by itself, in case mf is empty.
						/*&& mf.IsTerrainResourceOnMap(res)*/ 
						&& unit.ResourcesHeld < unit.Type->ResInfo[res]->ResourceCapacity
						&& (unit.CurrentResource != res || unit.ResourcesHeld < unit.Type->ResInfo[res]->ResourceCapacity)) {
						SendCommandResourceLoc(unit, pos, flush);
						ret = 1;
						break;
					}
				}
				if (res != MaxCosts) {
					continue;
				}
			}
		}
		if (!unit.CanMove()) {
			if (dest && dest->Type->GivesResource && dest->Type->BoolFlag[CANHARVEST_INDEX].value) {
				dest->Blink = 4;
				SendCommandResource(unit, *dest, flush);
				ret = 1;
				continue;
			}
			if (mf.playerInfo.IsExplored(*unit.Player) && mf.IsTerrainResourceOnMap()) {
				SendCommandResourceLoc(unit, pos, flush);
				ret = 1;
				continue;
			}
			SendCommandMove(unit, pos, flush);
			ret = 1;
			continue;
		}
	}
	return ret;
}

/**
**  Send selected units to unload passengers.
**
**  @param tilePos  tile map position.
*/
static int SendUnload(const Vec2i &tilePos)
{
	const int flush = !(KeyModifiers & ModifierShift);

	for (size_t i = 0; i != Selected.size(); ++i) {
		// FIXME: not only transporter selected?
		SendCommandUnload(*Selected[i], tilePos, NoUnitP, flush);
	}
	return Selected.empty() ? 0 : 1;
}

/**
**  Send the current selected group for spell cast.
**
**  To empty field:
**  To unit:
**    Spell cast on unit or on map spot.
**
**  @param tilePos  tile map position.
**
**  @see Selected
*/
static int SendSpellCast(const Vec2i &tilePos)
{
	const int flush = !(KeyModifiers & ModifierShift);
	CUnit *dest = UnitUnderCursor;
	int ret = 0;

	/* NOTE: Vladi:
	   This is a high-level function, it sends target spot and unit
	   (if exists). All checks are performed at spell cast handle
	   function which will cancel function if cannot be executed
	 */
	for (size_t i = 0; i != Selected.size(); ++i) {
		CUnit &unit = *Selected[i];
		if (!unit.Type->CanCastSpell) {
			DebugPrint("but unit %d(%s) can't cast spells?\n" _C_
					   UnitNumber(unit) _C_ unit.Type->Name.c_str());
			// this unit cannot cast spell
			continue;
		}
		// CursorValue here holds the spell type id
		const SpellType *spell = SpellTypeTable[CursorValue];
		if (!spell) {
			fprintf(stderr, "unknown spell-id: %d\n", CursorValue);
			ExitFatal(1);
		}
		if (dest && dest == &unit && (!spell->Condition || spell->Condition->TargetSelf == CONDITION_FALSE)) {
			// Only spells with explicit 'self: true' allows self targetting
			continue;
		}
		SendCommandSpellCast(unit, tilePos, spell->Target == TargetPosition ? NULL : dest , CursorValue, flush);
		ret = 1;
	}
	return ret;
}

/**
**  Send a command to selected units.
**
**  @param tilePos  tile map position.
*/
static void SendCommand(const Vec2i &tilePos)
{
	int ret = 0;

	CurrentButtonLevel = 0;
	UI.ButtonPanel.Update();
	switch (CursorAction) {
		case ButtonMove:
			ret = SendMove(tilePos);
			break;
		case ButtonRepair:
			ret = SendRepair(tilePos);
			break;
		case ButtonAttack:
			ret = SendAttack(tilePos);
			break;
		case ButtonAttackGround:
			ret = SendAttackGround(tilePos);
			break;
		case ButtonPatrol:
			ret = SendPatrol(tilePos);
			break;
		case ButtonHarvest:
			ret = SendResource(tilePos);
			break;
		case ButtonUnload:
			ret = SendUnload(tilePos);
			break;
		case ButtonSpellCast:
			ret = SendSpellCast(tilePos);
			break;
		default:
			DebugPrint("Unsupported send action %d\n" _C_ CursorAction);
			break;
	}
	if (ret) {
		// Acknowledge the command with first selected unit.
		for (size_t i = 0; i != Selected.size(); ++i) {
			if (CursorAction == ButtonAttack || CursorAction == ButtonAttackGround || CursorAction == ButtonSpellCast) {
				if (Selected[i]->Type->MapSound.Attack.Sound) {
					PlayUnitSound(*Selected[i], VoiceAttack);
					break;
				} else if (Selected[i]->Type->MapSound.Acknowledgement.Sound) {
					PlayUnitSound(*Selected[i], VoiceAcknowledging);
					break;
				}
			} else if (CursorAction == ButtonRepair && Selected[i]->Type->MapSound.Repair.Sound) {
				PlayUnitSound(*Selected[i], VoiceRepairing);
				break;
			} else if (CursorAction == ButtonBuild && Selected[i]->Type->MapSound.Build.Sound) {
				PlayUnitSound(*Selected[i], VoiceBuild);
				break;
			} else if (Selected[i]->Type->MapSound.Acknowledgement.Sound) {
				PlayUnitSound(*Selected[i], VoiceAcknowledging);
				break;
			}
		}
		ShowOrdersCount = GameCycle + Preference.ShowOrders * CYCLES_PER_SECOND;
	}
}

//.............................................................................

/**
**  Mouse button press on selection/group area.
**
**  @param num     Button number.
**  @param button  Mouse Button pressed.
*/
static void DoSelectionButtons(int num, unsigned)
{
	if (GameObserve || GamePaused || GameEstablishing) {
		return;
	}

	if (static_cast<size_t>(num) >= Selected.size() || !(MouseButtons & LeftButton)) {
		return;
	}

	CUnit &unit = *Selected[num];

	if ((KeyModifiers & ModifierControl) || (MouseButtons & (LeftButton << MouseDoubleShift))) {
		if (KeyModifiers & ModifierShift) {
			ToggleUnitsByType(unit);
		} else {
			SelectUnitsByType(unit);
		}
	} else if (KeyModifiers & ModifierAlt) {
		if (KeyModifiers & ModifierShift) {
			AddGroupFromUnitToSelection(unit);
		} else {
			SelectGroupFromUnit(unit);
		}
	} else if (KeyModifiers & ModifierShift) {
		ToggleSelectUnit(unit);
	} else {
		SelectSingleUnit(unit);
	}

	UI.StatusLine.Clear();
	UI.StatusLine.ClearCosts();
	CurrentButtonLevel = 0;
	SelectionChanged();
}

//.............................................................................

/**
**  Handle mouse button pressed in select state.
**
**  Select state is used for target of patrol, attack, move, ....
**
**  @param button  Button pressed down.
*/
static void UISelectStateButtonDown(unsigned)
{
	if (GameObserve || GamePaused || GameEstablishing) {
		return;
	}

	//
	//  Clicking on the map.
	//
	if (CursorOn == CursorOnMap && UI.MouseViewport->IsInsideMapArea(CursorScreenPos)) {
		UI.StatusLine.Clear();
		UI.StatusLine.ClearCosts();
		CursorState = CursorStatePoint;
		GameCursor = UI.Point.Cursor;
		CustomCursor.clear();
		CurrentButtonLevel = 0;
		UI.ButtonPanel.Update();

		if (MouseButtons & LeftButton) {
			const CViewport &vp = *UI.MouseViewport;
			const PixelPos mapPixelPos = vp.ScreenToMapPixelPos(CursorScreenPos);

			if (!ClickMissile.empty()) {
				MakeLocalMissile(*MissileTypeByIdent(ClickMissile), mapPixelPos, mapPixelPos);
			}
			SendCommand(Map.MapPixelPosToTilePos(mapPixelPos));
		}
		return;
	}

	//
	//  Clicking on the minimap.
	//
	if (CursorOn == CursorOnMinimap) {
		const Vec2i cursorTilePos = UI.Minimap.ScreenToTilePos(CursorScreenPos);

		if (MouseButtons & LeftButton) {
			const PixelPos mapPixelPos = Map.TilePosToMapPixelPos_Center(cursorTilePos);

			UI.StatusLine.Clear();
			UI.StatusLine.ClearCosts();
			CursorState = CursorStatePoint;
			GameCursor = UI.Point.Cursor;
			CustomCursor.clear();
			CurrentButtonLevel = 0;
			UI.ButtonPanel.Update();
			if (!ClickMissile.empty()) {
				MakeLocalMissile(*MissileTypeByIdent(ClickMissile), mapPixelPos, mapPixelPos);
			}
			SendCommand(cursorTilePos);
		} else {
			UI.SelectedViewport->Center(Map.TilePosToMapPixelPos_Center(cursorTilePos));
		}
		return;
	}

	if (CursorOn == CursorOnButton) {
		// FIXME: other buttons?
		// 74145: Spell-cast on unit portrait
		if (Selected.size() > 1 && ButtonAreaUnderCursor == ButtonAreaSelected
			&& CursorAction == ButtonSpellCast) {
			if (GameObserve || GamePaused || GameEstablishing) {
				return;
			}
			int num = ButtonUnderCursor;

			if (static_cast<size_t>(num) >= Selected.size() || !(MouseButtons & LeftButton)) {
				return;
			}

			CUnit &unit = *Selected[num];

			const Vec2i tilePos = unit.tilePos;
			UnitUnderCursor = &unit;
			SendSpellCast(tilePos);
			UnitUnderCursor = NULL;
		}
		if (ButtonAreaUnderCursor == ButtonAreaButton) {
			OldButtonUnderCursor = ButtonUnderCursor;
			return;
		}
	}

	UI.StatusLine.Clear();
	UI.StatusLine.ClearCosts();
	CursorState = CursorStatePoint;
	if (CustomCursor.length() && CursorByIdent(CustomCursor)) {
		GameCursor = CursorByIdent(CustomCursor);
	} else {
		GameCursor = UI.YellowHair.Cursor;
	}
	CurrentButtonLevel = 0;
	UI.ButtonPanel.Update();
}


static void UIHandleButtonDown_OnMap(unsigned button)
{
	Assert(UI.MouseViewport);
	if ((MouseButtons & LeftButton) && UI.SelectedViewport != UI.MouseViewport) {
		UI.SelectedViewport = UI.MouseViewport;
		DebugPrint("selected viewport changed to %ld.\n" _C_
				   static_cast<long int>(UI.SelectedViewport - UI.Viewports));
	}

	// to redraw the cursor immediately (and avoid up to 1 sec delay
	if (CursorBuilding) {
		// Possible Selected[0] was removed from map
		// need to make sure there is a unit to build
		if (Selected[0] && (MouseButtons & LeftButton)
			&& UI.MouseViewport->IsInsideMapArea(CursorScreenPos)) {// enter select mode
			const Vec2i tilePos = UI.MouseViewport->ScreenToTilePos(CursorScreenPos);
			bool explored = CanBuildOnArea(*Selected[0], tilePos);

			// 0 Test build, don't really build
			if (CanBuildUnitType(Selected[0], *CursorBuilding, tilePos, 0) && (explored || ReplayRevealMap)) {
				const int flush = !(KeyModifiers & ModifierShift);
				PlayGameSound(GameSounds.PlacementSuccess[ThisPlayer->Race].Sound, MaxSampleVolume);
				PlayUnitSound(*Selected[0], VoiceBuild);
				for (size_t i = 0; i != Selected.size(); ++i) {
					SendCommandBuildBuilding(*Selected[i], tilePos, *CursorBuilding, flush);
				}
				if (!(KeyModifiers & (ModifierAlt | ModifierShift))) {
					CancelBuildingMode();
				}
			} else {
				PlayGameSound(GameSounds.PlacementError[ThisPlayer->Race].Sound, MaxSampleVolume);
			}
		} else {
			CancelBuildingMode();
		}
		return;
	}

	if (MouseButtons & UI.PieMenu.MouseButton) { // enter pie menu
		UnitUnderCursor = NULL;
		GameCursor = UI.Point.Cursor;  // Reset
		CursorStartScreenPos = CursorScreenPos;
		if (!Selected.empty() && Selected[0]->Player == ThisPlayer && CursorState == CursorStatePoint) {
			CursorState = CursorStatePieMenu;
		}
	} else if (MouseButtons & RightButton) {
		if (!GameObserve && !GamePaused && !GameEstablishing && UI.MouseViewport->IsInsideMapArea(CursorScreenPos)) {
			CUnit *unit;
			// FIXME: Rethink the complete chaos of coordinates here
			// FIXME: Johns: Perhaps we should use a pixel map coordinates
			const Vec2i tilePos = UI.MouseViewport->ScreenToTilePos(CursorScreenPos);

			if (UnitUnderCursor != NULL && (unit = UnitOnMapTile(tilePos, -1))
				&& !UnitUnderCursor->Type->BoolFlag[DECORATION_INDEX].value) {
				unit->Blink = 4;                // if right click on building -- blink
			} else { // if not not click on building -- green cross
				if (!ClickMissile.empty()) {
					const PixelPos mapPixelPos = UI.MouseViewport->ScreenToMapPixelPos(CursorScreenPos);

					MakeLocalMissile(*MissileTypeByIdent(ClickMissile), mapPixelPos, mapPixelPos);
				}
			}
			const PixelPos mapPixelPos = UI.MouseViewport->ScreenToMapPixelPos(CursorScreenPos);
			DoRightButton(mapPixelPos);
		}
	} else if (MouseButtons & LeftButton) { // enter select mode
		CursorStartScreenPos = CursorScreenPos;
		CursorStartMapPos = UI.MouseViewport->ScreenToMapPixelPos(CursorScreenPos);
		GameCursor = UI.Cross.Cursor;
		CursorState = CursorStateRectangle;
	} else if (MouseButtons & MiddleButton) {// enter move map mode
		CursorStartScreenPos = CursorScreenPos;
		GameCursor = UI.Scroll.Cursor;
	}
}

static void UIHandleButtonDown_OnMinimap(unsigned button)
{
	const Vec2i cursorTilePos = UI.Minimap.ScreenToTilePos(CursorScreenPos);

	if (MouseButtons & LeftButton) { // enter move mini-mode
		UI.SelectedViewport->Center(Map.TilePosToMapPixelPos_Center(cursorTilePos));
	} else if (MouseButtons & RightButton) {
		if (!GameObserve && !GamePaused && !GameEstablishing) {
			const PixelPos mapPixelPos = Map.TilePosToMapPixelPos_Center(cursorTilePos);
			if (!ClickMissile.empty()) {
				MakeLocalMissile(*MissileTypeByIdent(ClickMissile), mapPixelPos, mapPixelPos);
			}
			DoRightButton(mapPixelPos);
		}
	}
}

static void UIHandleButtonDown_OnButton(unsigned button)
{
	// clicked on info panel - selection shown
	if (Selected.size() > 1 && ButtonAreaUnderCursor == ButtonAreaSelected) {
		PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
		DoSelectionButtons(ButtonUnderCursor, button);
	} else if ((MouseButtons & LeftButton)) {
		//  clicked on menu button
		if (ButtonAreaUnderCursor == ButtonAreaMenu) {
			if ((ButtonUnderCursor == ButtonUnderMenu || ButtonUnderCursor == ButtonUnderNetworkMenu)
				&& !GameMenuButtonClicked) {
				PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
				GameMenuButtonClicked = true;
			} else if (ButtonUnderCursor == ButtonUnderNetworkDiplomacy && !GameDiplomacyButtonClicked) {
				PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
				GameDiplomacyButtonClicked = true;
			} else if (ButtonUnderCursor == ButtonUnderFreeWorkers) {
				UiFindIdleWorker();
			}
			//  clicked on user buttons
		} else if (ButtonAreaUnderCursor == ButtonAreaUser) {
			for (size_t i = 0; i < UI.UserButtons.size(); ++i) {
				CUIUserButton &button = UI.UserButtons[i];

				if (i == size_t(ButtonUnderCursor) && !button.Clicked) {
					PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
					button.Clicked = true;
				}
			}
			//  clicked on selected button
		} else if (ButtonAreaUnderCursor == ButtonAreaSelected) {
			//  clicked on single unit shown
			if (ButtonUnderCursor == 0 && Selected.size() == 1) {
				PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
				UI.SelectedViewport->Center(Selected[0]->GetMapPixelPosCenter());
			}
			//  clicked on training button
		} else if (ButtonAreaUnderCursor == ButtonAreaTraining) {
			if (!GameObserve && !GamePaused && !GameEstablishing && ThisPlayer->IsTeamed(*Selected[0])) {
				if (static_cast<size_t>(ButtonUnderCursor) < Selected[0]->Orders.size()
					&& Selected[0]->Orders[ButtonUnderCursor]->Action == UnitActionTrain) {
					const COrder_Train &order = *static_cast<COrder_Train *>(Selected[0]->Orders[ButtonUnderCursor]);

					DebugPrint("Cancel slot %d %s\n" _C_ ButtonUnderCursor _C_ order.GetUnitType().Ident.c_str());
					PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
					SendCommandCancelTraining(*Selected[0], ButtonUnderCursor, &order.GetUnitType());
				}
			}
			//  clicked on upgrading button
		} else if (ButtonAreaUnderCursor == ButtonAreaUpgrading) {
			if (!GameObserve && !GamePaused && !GameEstablishing && ThisPlayer->IsTeamed(*Selected[0])) {
				if (ButtonUnderCursor == 0 && Selected.size() == 1) {
					DebugPrint("Cancel upgrade %s\n" _C_ Selected[0]->Type->Ident.c_str());
					PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
					SendCommandCancelUpgradeTo(*Selected[0]);
				}
			}
			//  clicked on researching button
		} else if (ButtonAreaUnderCursor == ButtonAreaResearching) {
			if (!GameObserve && !GamePaused && !GameEstablishing && ThisPlayer->IsTeamed(*Selected[0])) {
				if (ButtonUnderCursor == 0 && Selected.size() == 1) {
					DebugPrint("Cancel research %s\n" _C_ Selected[0]->Type->Ident.c_str());
					PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
					SendCommandCancelResearch(*Selected[0]);
				}
			}
			//  clicked on button panel
		} else if (ButtonAreaUnderCursor == ButtonAreaTransporting) {
			//  for transporter
			if (!GameObserve && !GamePaused && !GameEstablishing && ThisPlayer->IsTeamed(*Selected[0])) {
				if (Selected[0]->BoardCount >= ButtonUnderCursor) {
					CUnit *uins = Selected[0]->UnitInside;
					size_t j = 0;

					for (int i = 0; i < Selected[0]->InsideCount; ++i, uins = uins->NextContained) {
						if (!uins->Boarded || j >= UI.TransportingButtons.size() || (Selected[0]->Player != ThisPlayer && uins->Player != ThisPlayer)) {
							continue;
						}
						int lastOccupiedButton = j + uins->Type->BoardSize - 1;
						if (ButtonAreaUnderCursor == ButtonAreaTransporting) {
							for (int sub_j = j; sub_j <= lastOccupiedButton; sub_j++) {
								if (static_cast<size_t>(ButtonUnderCursor) == sub_j) {
									Assert(uins->Boarded);
									const int flush = !(KeyModifiers & ModifierShift);
									if (ThisPlayer->IsTeamed(*Selected[0]) || uins->Player == ThisPlayer) {
										PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
										SendCommandUnload(*Selected[0], Selected[0]->tilePos, uins, flush);
									}
								}
							}
						}
						j = lastOccupiedButton + 1;
					}
				}
			}
		} else if (ButtonAreaUnderCursor == ButtonAreaButton) {
			if (!GameObserve && !GamePaused && !GameEstablishing && (ThisPlayer->IsTeamed(*Selected[0]) || Selected[0]->Player->Index == PlayerMax - 1)) {
				PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
				OldButtonUnderCursor = ButtonUnderCursor;
			}
		}
	} else if ((MouseButtons & MiddleButton)) {
		//  clicked on info panel - single unit shown
		if (ButtonAreaUnderCursor == ButtonAreaSelected && ButtonUnderCursor == 0 && Selected.size() == 1) {
			PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
			if (UI.SelectedViewport->Unit == Selected[0]) {
				UI.SelectedViewport->Unit = NULL;
			} else {
				UI.SelectedViewport->Unit = Selected[0];
			}
		}
	} else if ((MouseButtons & RightButton)) {
	}
}

/**
**  Called if mouse button pressed down.
**
**  @param button  Button pressed down.
*/
void UIHandleButtonDown(unsigned button)
{
	// Detect long left selection click
	const bool longLeftButton = (MouseButtons & ((LeftButton << MouseHoldShift))) != 0;

	if (IsDemoMode()) {
		// If we are in "demo mode", exit no matter what we click on.
		void ActionDraw();
		ActionDraw();
		return;
	}

	static bool OldShowSightRange;
	static bool OldShowReactionRange;
	static bool OldShowAttackRange;
	static bool OldValid = false;
	OldButtonUnderCursor = -1;

	// Reset the ShowNameDelay counters
	ShowNameDelay = ShowNameTime = GameCycle;

	if (longLeftButton) {
		if (!OldValid) {
			OldShowSightRange = Preference.ShowSightRange;
			OldShowAttackRange = Preference.ShowAttackRange;
			OldShowReactionRange = Preference.ShowReactionRange;
			OldValid = true;

			Preference.ShowSightRange = true;
			Preference.ShowAttackRange = true;
			Preference.ShowReactionRange = true;
		}
	} else if (OldValid) {
		Preference.ShowSightRange = OldShowSightRange;
		Preference.ShowAttackRange = OldShowAttackRange;
		Preference.ShowReactionRange = OldShowReactionRange;
		OldValid = false;
	}

	// select mode
	if (CursorState == CursorStateRectangle) {
		return;
	}
	// CursorOn should have changed with BigMapMode, so recompute it.
	HandleMouseOn(CursorScreenPos);
	//  Selecting target. (Move,Attack,Patrol,... commands);
	if (CursorState == CursorStateSelect) {
		UISelectStateButtonDown(button);
		return;
	}

	if (CursorState == CursorStatePieMenu) {
		if (CursorOn == CursorOnMap) {
			HandlePieMenuMouseSelection();
			return;
		} else {
			// Pie Menu canceled
			CursorState = CursorStatePoint;
			// Don't return, we might be over another button
		}
	}

	//  Cursor is on the map area
	if (CursorOn == CursorOnMap) {
		UIHandleButtonDown_OnMap(button);
	} else if (CursorOn == CursorOnMinimap) {
		//  Cursor is on the minimap area
		UIHandleButtonDown_OnMinimap(button);
	} else if (CursorOn == CursorOnButton) {
		//  Cursor is on the buttons: group or command
		UIHandleButtonDown_OnButton(button);
	}
}

/**
**  Called if mouse button released.
**
**  @param button  Button released.
*/
void UIHandleButtonUp(unsigned button)
{
	//
	//  Move map.
	//
	if (GameCursor == UI.Scroll.Cursor) {
		GameCursor = UI.Point.Cursor;
		return;
	}

	//
	//  Pie Menu
	//
	if (CursorState == CursorStatePieMenu) {
		// Little threshold
		if (1 < abs(CursorStartScreenPos.x - CursorScreenPos.x)
			|| 1 < abs(CursorStartScreenPos.y - CursorScreenPos.y)) {
			// there was a move, handle the selected button/pie
			HandlePieMenuMouseSelection();
		}
	}

	if ((1 << button) == LeftButton) {
		//
		//  Menu (F10) button
		//
		if (GameMenuButtonClicked) {
			GameMenuButtonClicked = false;
			if (ButtonAreaUnderCursor == ButtonAreaMenu) {
				if (ButtonUnderCursor == ButtonUnderMenu || ButtonUnderCursor == ButtonUnderNetworkMenu) {
					// FIXME: Not if, in input mode.
					if (!IsNetworkGame()) {
						GamePaused = true;
						UI.StatusLine.Set(_("Game Paused"));
					}
					if (ButtonUnderCursor == ButtonUnderMenu) {
						if (UI.MenuButton.Callback) {
							UI.MenuButton.Callback->action("");
						}
					} else {
						if (UI.NetworkMenuButton.Callback) {
							UI.NetworkMenuButton.Callback->action("");
						}
					}
					return;
				}
			}
		}

		//
		//  Diplomacy button
		//
		if (GameDiplomacyButtonClicked) {
			GameDiplomacyButtonClicked = false;
			if (ButtonAreaUnderCursor == ButtonAreaMenu && ButtonUnderCursor == ButtonUnderNetworkDiplomacy) {
				if (UI.NetworkDiplomacyButton.Callback) {
					UI.NetworkDiplomacyButton.Callback->action("");
				}
				return;
			}
		}

		//
		//  User buttons
		//
		for (size_t i = 0; i < UI.UserButtons.size(); ++i) {
			CUIUserButton &button = UI.UserButtons[i];

			if (button.Clicked) {
				button.Clicked = false;
				if (ButtonAreaUnderCursor == ButtonAreaUser) {
					if (button.Button.Callback) {
						button.Button.Callback->action("");
					}
					return;
				}
			}
		}
		if (!GameObserve && !GamePaused && !GameEstablishing && Selected.empty() == false && ThisPlayer->IsTeamed(*Selected[0])) {
			if (OldButtonUnderCursor != -1 && OldButtonUnderCursor == ButtonUnderCursor) {
				UI.ButtonPanel.DoClicked(ButtonUnderCursor);
				OldButtonUnderCursor = -1;
				return;
			}
		}
		if (CursorOn == CursorOnButton) {
			// FIXME: other buttons?
			if (ButtonAreaUnderCursor == ButtonAreaButton && OldButtonUnderCursor != -1 && OldButtonUnderCursor == ButtonUnderCursor) {
				UI.ButtonPanel.DoClicked(ButtonUnderCursor);
				return;
			}
		}
	}

	// FIXME: should be completly rewritten
	// FIXME: must selecting!  (lokh: what does this mean? is this done now?)

	// SHIFT toggles select/unselect a single unit and
	// add the content of the rectangle to the selectection
	// ALT takes group of unit
	// CTRL takes all units of same type (st*rcr*ft)
	if (CursorState == CursorStateRectangle && !(MouseButtons & LeftButton)) { // leave select mode
		int num = 0;
		//
		//  Little threshold
		//
		if (CursorStartScreenPos.x < CursorScreenPos.x - 1 || CursorScreenPos.x + 1 < CursorStartScreenPos.x
			|| CursorStartScreenPos.y < CursorScreenPos.y - 1 || CursorScreenPos.y + 1 < CursorStartScreenPos.y) {
			PixelPos pos0 = CursorStartMapPos;
			const PixelPos cursorMapPos = UI.MouseViewport->ScreenToMapPixelPos(CursorScreenPos);
			PixelPos pos1 = cursorMapPos;

			if (pos0.x > pos1.x) {
				std::swap(pos0.x, pos1.x);
			}
			if (pos0.y > pos1.y) {
				std::swap(pos0.y, pos1.y);
			}
			if (KeyModifiers & ModifierShift) {
				if (KeyModifiers & ModifierAlt) {
					num = AddSelectedGroundUnitsInRectangle(pos0, pos1);
				} else if (KeyModifiers & ModifierControl) {
					num = AddSelectedAirUnitsInRectangle(pos0, pos1);
				} else {
					num = AddSelectedUnitsInRectangle(pos0, pos1);
				}
			} else {
				if (KeyModifiers & ModifierAlt) {
					num = SelectGroundUnitsInRectangle(pos0, pos1);
				} else if (KeyModifiers & ModifierControl) {
					num = SelectAirUnitsInRectangle(pos0, pos1);
				} else {
					num = SelectUnitsInRectangle(pos0, pos1);
				}
			}
		} else {
			//
			// Select single unit
			//
			// cade: cannot select unit on invisible space
			// FIXME: johns: only complete invisibile units
			const Vec2i cursorTilePos = UI.MouseViewport->ScreenToTilePos(CursorScreenPos);
			CUnit *unit = NULL;
			if (ReplayRevealMap || Map.Field(cursorTilePos)->playerInfo.IsTeamVisible(*ThisPlayer)) {
				const PixelPos cursorMapPos = UI.MouseViewport->ScreenToMapPixelPos(CursorScreenPos);

				unit = UnitOnScreen(cursorMapPos.x, cursorMapPos.y);
			}
			if (unit) {
				// FIXME: Not nice coded, button number hardcoded!
				if ((KeyModifiers & ModifierControl)
					|| (button & (1 << MouseDoubleShift))) {
					if (KeyModifiers & ModifierShift) {
						num = ToggleUnitsByType(*unit);
					} else {
						num = SelectUnitsByType(*unit);
					}
				} else if ((KeyModifiers & ModifierAlt) && unit->LastGroup) {
					if (KeyModifiers & ModifierShift) {
						num = AddGroupFromUnitToSelection(*unit);
					} else {
						num = SelectGroupFromUnit(*unit);
					}

					// Don't allow to select own and enemy units.
					// Don't allow mixing buildings
				} else if (KeyModifiers & ModifierShift
						   && (unit->Player == ThisPlayer || ThisPlayer->IsTeamed(*unit))
						   && !unit->Type->Building
						   && (Selected.size() != 1 || !Selected[0]->Type->Building)
						   && (Selected.size() != 1 || Selected[0]->Player == ThisPlayer || ThisPlayer->IsTeamed(*Selected[0]))) {
					num = ToggleSelectUnit(*unit);
					if (!num) {
						SelectionChanged();
					}
				} else {
					SelectSingleUnit(*unit);
					num = 1;
				}
			} else {
				num = 0;
			}
		}

		if (num) {
			UI.StatusLine.Clear();
			UI.StatusLine.ClearCosts();
			CurrentButtonLevel = 0;
			SelectionChanged();

			//
			//  Play selecting sound.
			//    Buildings,
			//    This player, or neutral unit (goldmine,critter)
			//    Other clicks.
			//
			if (Selected.size() == 1) {
				if (Selected[0]->CurrentAction() == UnitActionBuilt && Selected[0]->Player->Index == ThisPlayer->Index) {
					PlayUnitSound(*Selected[0], VoiceBuilding);
				} else if (Selected[0]->Burning) {
					// FIXME: use GameSounds.Burning
					PlayGameSound(SoundForName("burning"), MaxSampleVolume);
				} else if (Selected[0]->Player == ThisPlayer || ThisPlayer->IsTeamed(*Selected[0])
						   || Selected[0]->Player->Type == PlayerTypes::PlayerNeutral) {
					PlayUnitSound(*Selected[0], VoiceSelected);
				} else {
					PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
				}
				if (Selected[0]->Player == ThisPlayer) {
					char buf[64];
					if (Selected[0]->Player->UnitTypesCount[Selected[0]->Type->Slot] > 1) {
						snprintf(buf, sizeof(buf), _("You have ~<%d~> %ss"),
								 Selected[0]->Player->UnitTypesCount[Selected[0]->Type->Slot],
								 Selected[0]->Type->Name.c_str());
					} else {
						snprintf(buf, sizeof(buf), _("You have ~<%d~> %s(s)"),
								 Selected[0]->Player->UnitTypesCount[Selected[0]->Type->Slot],
								 Selected[0]->Type->Name.c_str());
					}
					UI.StatusLine.Set(buf);
				}
			}
		}

		CursorStartScreenPos.x = 0;
		CursorStartScreenPos.y = 0;
		GameCursor = UI.Point.Cursor;
		CursorState = CursorStatePoint;
	}
}

/**
**  Get pie menu under the cursor
**
**  @return  Index of the pie menu under the cursor or -1 for none
*/
static int GetPieUnderCursor()
{
	int x = CursorScreenPos.x - (CursorStartScreenPos.x - ICON_SIZE_X / 2);
	int y = CursorScreenPos.y - (CursorStartScreenPos.y - ICON_SIZE_Y / 2);
	for (int i = 0; i < 9; ++i) {
		if (x > UI.PieMenu.X[i] && x < UI.PieMenu.X[i] + ICON_SIZE_X
			&& y > UI.PieMenu.Y[i] && y < UI.PieMenu.Y[i] + ICON_SIZE_Y) {
			return i;
		}
	}
	return -1; // no pie under cursor
}

/**
**  Draw Pie Menu
*/
void DrawPieMenu()
{
	char buf[2] = "?";

	if (CursorState != CursorStatePieMenu) {
		return;
	}

	if (CurrentButtons.empty()) { // no buttons
		CursorState = CursorStatePoint;
		return;
	}
	std::vector<ButtonAction> &buttons(CurrentButtons);
	CLabel label(GetGameFont());
	CViewport *vp = UI.SelectedViewport;
	PushClipping();
	vp->SetClipping();

	// Draw background
	if (UI.PieMenu.G) {
		UI.PieMenu.G->DrawFrameClip(0,
									CursorStartScreenPos.x - UI.PieMenu.G->Width / 2,
									CursorStartScreenPos.y - UI.PieMenu.G->Height / 2);
	}
	for (int i = 0; i < (int)UI.ButtonPanel.Buttons.size() && i < 9; ++i) {
		if (buttons[i].Pos != -1) {
			int x = CursorStartScreenPos.x - ICON_SIZE_X / 2 + UI.PieMenu.X[i];
			int y = CursorStartScreenPos.y - ICON_SIZE_Y / 2 + UI.PieMenu.Y[i];
			const PixelPos pos(x, y);

			bool gray = false;
			for (size_t j = 0; j != Selected.size(); ++j) {
				if (!IsButtonAllowed(*Selected[j], buttons[i])) {
					gray = true;
					break;
				}
			}
			// Draw icon
			if (gray) {
				buttons[i].Icon.Icon->DrawGrayscaleIcon(pos);
			} else {
				buttons[i].Icon.Icon->DrawIcon(pos, ThisPlayer->Index);
			}

			// Tutorial show command key in icons
			if (UI.ButtonPanel.ShowCommandKey) {
				const char *text;

				if (buttons[i].Key == 27) {
					text = "ESC";
				} else {
					buf[0] = toupper(buttons[i].Key);
					text = (const char *)buf;
				}
				label.DrawClip(x + 4, y + 4, text);
			}
		}
	}

	PopClipping();

	int i = GetPieUnderCursor();
	if (i != -1 && KeyState != KeyStateInput && buttons[i].Pos != -1) {
		if (!Preference.NoStatusLineTooltips) {
			UpdateStatusLineForButton(buttons[i]);
		}
		DrawPopup(buttons[i], UI.ButtonPanel.Buttons[i],
				  CursorStartScreenPos.x + UI.PieMenu.X[i], CursorStartScreenPos.y + UI.PieMenu.Y[i]);
	}
}

/**
**  Handle pie menu mouse selection
*/
static void HandlePieMenuMouseSelection()
{
	if (CurrentButtons.empty()) {  // no buttons
		return;
	}

	int pie = GetPieUnderCursor();
	if (pie != -1) {
		const ButtonCmd action = CurrentButtons[pie].Action;
		UI.ButtonPanel.DoClicked(pie);
		if (action == ButtonButton) {
			// there is a submenu => stay in piemenu mode
			// and recenter the piemenu around the cursor
			CursorStartScreenPos = CursorScreenPos;
		} else {
			if (CursorState == CursorStatePieMenu) {
				CursorState = CursorStatePoint;
			}
			CursorOn = CursorOnUnknown;
			UIHandleMouseMove(CursorScreenPos); // recompute CursorOn and company
		}
	} else {
		CursorState = CursorStatePoint;
		CursorOn = CursorOnUnknown;
		UIHandleMouseMove(CursorScreenPos); // recompute CursorOn and company
	}
}
//@}
