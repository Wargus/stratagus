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
/**@name selection.cpp - The units' selection. */
//
//      (c) Copyright 1999-2005 by Patrice Fortier, Lutz Sammer, and
//                                 Jimmy Salmon
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

#include "commands.h"
#include "interface.h"
#include "iolib.h"
#include "map.h"
#include "network.h"
#include "player.h"
#include "ui.h"
#include "unittype.h"
#include "unit.h"
#include "unit_find.h"
#include "unit_manager.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

int NumSelected;                 /// Number of selected units
int TeamNumSelected[PlayerMax];  /// How many units selected
int MaxSelectable;               /// Maximum number of selected units
CUnit **Selected;                 /// All selected units
CUnit **TeamSelected[PlayerMax];  /// teams currently selected units

static int _NumSelected;                 /// save of NumSelected
static int _TeamNumSelected[PlayerMax];  /// save of TeamNumSelected
static CUnit **_Selected;                 /// save of Selected
static CUnit **_TeamSelected[PlayerMax];  /// save of TeamSelected

static unsigned GroupId;         /// Unique group # for automatic groups

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Save selection to restore after.
*/
void SaveSelection()
{
	for (int i = 0; i < PlayerMax; ++i) {
		_TeamNumSelected[i] = TeamNumSelected[i];
		for (int j = 0; j < TeamNumSelected[i]; ++j) {
			_TeamSelected[i][j] = TeamSelected[i][j];
		}
	}
	_NumSelected = NumSelected;
	for (int j = 0; j < NumSelected; ++j) {
		_Selected[j] = Selected[j];
	}
}

/**
**  Restore selection.
*/
void RestoreSelection()
{
	UnSelectAll();
	for (int i = 0; i < PlayerMax; ++i) {
		TeamNumSelected[i] = _TeamNumSelected[i];
		for (int j = 0; j < _TeamNumSelected[i]; ++j) {
			TeamSelected[i][j] = _TeamSelected[i][j];
			TeamSelected[i][j]->TeamSelected |= (1 << i);
		}
	}
	NumSelected = _NumSelected;
	for (int j = 0; j < _NumSelected; ++j) {
		Selected[j] = _Selected[j];
		Selected[j]->Selected = 1;
	}
}

/**
**  Unselect all the units in the current selection
*/
void UnSelectAll()
{
	while (!++GroupId) { // Advance group id, but keep non zero
	}

	while (NumSelected) {
		CUnit *unit = Selected[--NumSelected];
		Selected[NumSelected] = NULL; // FIXME: only needed for old code
		unit->Selected = 0;
	}
	UI.SelectedViewport->Unit = NULL;

}

/**
**  Handle a suicide unit click
**
**  @param unit  suicide unit.
*/
static void HandleSuicideClick(CUnit &unit)
{
	static int NumClicks = 0;

	Assert(unit.Type->ClicksToExplode);
	if (GameObserve) {
		return;
	}
	if (NumSelected == 1 && Selected[0] == &unit) {
		NumClicks++;
	} else {
		NumClicks = 1;
	}
	if (NumClicks == unit.Type->ClicksToExplode) {
		SendCommandDismiss(unit);
		NumClicks = 0;
	}
}

/**
**  Replace a group of selected units by an other group of units.
**
**  @param units  Array of units to be selected.
**  @param count  Number of units in array to be selected.
*/
static void ChangeSelectedUnits(CUnit **units, int count)
{
	Assert(count <= MaxSelectable);

	if (count == 1 && units[0]->Type->ClicksToExplode &&
		!units[0]->Type->IsNotSelectable) {
		HandleSuicideClick(*units[0]);
		if (!units[0]->IsAlive()) {
			NetworkSendSelection(units, count);
			return ;
		}
	}
	UnSelectAll();
	NetworkSendSelection(units, count);
	int n = 0;
	for (int i = 0; i < count; ++i) {
		CUnit &unit = *units[i];
		if (!unit.Removed && !unit.TeamSelected && !unit.Type->IsNotSelectable) {
			Selected[n++] = &unit;
			unit.Selected = 1;
			if (count > 1) {
				unit.LastGroup = GroupId;
			}
		}
	}
	NumSelected = n;
}

/**
**  Change A Unit Selection from my Team
**
**  @param player  The Player who is selecting the units
**  @param units   The Units to add/remove
**  @param adjust  0 = reset, 1 = remove units, 2 = add units
*/
void ChangeTeamSelectedUnits(CPlayer &player, const std::vector<CUnit *> &units, int adjust)
{
	switch (adjust) {
		case 0:
			// UnSelectAllTeam(player);
			while (TeamNumSelected[player.Index]) {
				CUnit *unit = TeamSelected[player.Index][--TeamNumSelected[player.Index]];
				unit->TeamSelected &= ~(1 << player.Index);
				TeamSelected[player.Index][TeamNumSelected[player.Index]] = NULL; // FIXME: only needed for old code
			}
			// FALL THROUGH
		case 2:
			for (size_t i = 0; i != units.size(); ++i) {
				CUnit &unit = *units[i];
				Assert(!unit.Removed);
				if (!unit.Type->IsNotSelectable) {
					TeamSelected[player.Index][TeamNumSelected[player.Index]++] = &unit;
					unit.TeamSelected |= 1 << player.Index;
				}
			}
			Assert(TeamNumSelected[player.Index] <= MaxSelectable);
			break;
		case 1:
			for (int n = 0; n < TeamNumSelected[player.Index]; ++n) {
				for (size_t i = 0; i != units.size(); ++i) {
					if (units[i] == TeamSelected[player.Index][n]) {
						TeamSelected[player.Index][n] =
							TeamSelected[player.Index][TeamNumSelected[player.Index]--];
					}
				}
			}
			Assert(TeamNumSelected[player.Index] >= 0);
			break;
		default:
			Assert(0);
	}
}

/**
**  Add a unit to the other selected units.
**
**  @param unit  Pointer to unit to add.
**
**  @return      true if added to selection, false otherwise
**               (if NumSelected == MaxSelectable or
**                unit is already selected or unselectable)
*/
int SelectUnit(CUnit &unit)
{
	if (unit.Type->Revealer) { // Revealers cannot be selected
		DebugPrint("Selecting revealer?\n");
		return 0;
	}

	if (unit.Removed) { // Removed cannot be selected
		DebugPrint("Selecting removed?\n");
		return 0;
	}

	if (NumSelected == MaxSelectable) {
		return 0;
	}

	if (unit.Selected) {
		return 0;
	}

	if (unit.Type->IsNotSelectable && GameRunning) {
		return 0;
	}

	Selected[NumSelected++] = &unit;
	unit.Selected = 1;
	if (NumSelected > 1) {
		Selected[0]->LastGroup = unit.LastGroup = GroupId;
	}
	return 1;
}

/**
**  Select a single unit, unselecting the previous ones
**
**  @param unit  Pointer to unit to be selected.
*/
void SelectSingleUnit(CUnit &unit)
{
	CUnit *unitPtr = &unit;

	ChangeSelectedUnits(&unitPtr, 1);
}

/**
**  Unselect unit
**
**  @param unit  Pointer to unit to be unselected.
*/
void UnSelectUnit(CUnit &unit)
{
	if (unit.TeamSelected) {
		for (int i = 0; i < PlayerMax; ++i) {
			if (unit.TeamSelected & (1 << i)) {
				int j;
				for (j = 0; TeamSelected[i][j] != &unit; ++i) {
					;
				}
				Assert(j < TeamNumSelected[i]);

				if (j < --TeamNumSelected[i]) {

					TeamSelected[i][j] = TeamSelected[i][TeamNumSelected[i]];
				}
				unit.TeamSelected &= ~(1 << i);
			}
		}
	}
	if (!unit.Selected) {
		return;
	}
	int j;

	for (j = 0; Selected[j] != &unit; ++j) {
		;
	}
	Assert(j < NumSelected);

	if (j < --NumSelected) {
		Selected[j] = Selected[NumSelected];
	}

	if (NumSelected > 1) { // Assign new group to remaining units
		while (!++GroupId) { // Advance group id, but keep non zero
		}
		for (int i = 0; i < NumSelected; ++i) {
			Selected[i]->LastGroup = GroupId;
		}
	}
	Selected[NumSelected] = NULL; // FIXME: only needed for old code
	unit.Selected = 0;

	//Turn track unit mode off
	UI.SelectedViewport->Unit = NULL;
}

/**
**  Toggle the selection of a unit in a group of selected units
**
**  @param unit  Pointer to unit to be toggled.
**  @return      0 if unselected, 1 otherwise
*/
int ToggleSelectUnit(CUnit &unit)
{
	if (unit.Selected) {
		UnSelectUnit(unit);
		return 0;
	}
	SelectUnit(unit);
	return 1;
}

/**
**  Select units from a particular type and belonging to the local player.
**
**  The base is included in the selection and defines
**  the type of the other units to be selected.
**
**  @param base  Select all units of same type.
**
**  @return      Number of units found, 0 means selection unchanged
**
**  FIXME: 0 can't happen. Maybe when scripting will use it?
**
**  FIXME: should always select the nearest 9 units to the base!
*/
int SelectUnitsByType(CUnit &base)
{
	const CUnitType &type = *base.Type;
	const CViewport *vp = UI.MouseViewport;

	Assert(UI.MouseViewport);

	if (type.ClicksToExplode) {
		HandleSuicideClick(base);
	}

	// if unit is a cadaver or hidden (not on map)
	// no unit can be selected.
	if (base.Removed || base.IsAlive() == false) {
		return 0;
	}

	if (type.IsNotSelectable && GameRunning) {
		return 0;
	}
	if (base.TeamSelected) { // Somebody else onteam has this unit
		return 0;
	}

	UnSelectAll();
	Selected[0] = &base;
	base.Selected = 1;
	NumSelected = 1;

	// if unit isn't belonging to the player or allied player, or is a static unit
	// (like a building), only 1 unit can be selected at the same time.
	if (!CanSelectMultipleUnits(*base.Player) || !type.SelectableByRectangle) {
		return NumSelected;
	}

	//
	// Search for other visible units of the same type
	//
	std::vector<CUnit *> table;
	// select all visible units.
	// StephanR: should be (MapX,MapY,MapX+MapWidth-1,MapY+MapHeight-1) ???
	/* FIXME: this should probably be cleaner implemented if SelectUnitsByType()
	 * took parameters of the selection rectangle as arguments */
	const Vec2i offset(1, 1);
	const Vec2i minPos = vp->MapPos - offset;
	const Vec2i vpSize(vp->MapWidth, vp->MapHeight);
	const Vec2i maxPos = vp->MapPos + vpSize + offset;
	Select(minPos, maxPos, table, HasSameTypeAs(type));

	// FIXME: peon/peasant with gold/wood & co are considered from
	//   different type... idem for tankers
	for (size_t i = 0; i != table.size(); ++i) {
		CUnit &unit = *table[i];
		if (!CanSelectMultipleUnits(*unit.Player)) {
			continue;
		}
		if (unit.IsUnusable()) {  // guess SelectUnits doesn't check this
			continue;
		}
		if (&unit == &base) {  // no need to have the same unit twice :)
			continue;
		}
		if (unit.TeamSelected) { // Somebody else onteam has this unit
			continue;
		}
		Selected[NumSelected++] = &unit;
		unit.Selected = 1;
		if (NumSelected == MaxSelectable) {
			break;
		}
	}
	if (NumSelected > 1) {
		for (int i = 0; i < NumSelected; ++i) {
			Selected[i]->LastGroup = GroupId;
		}
	}
	NetworkSendSelection(Selected, NumSelected);
	return NumSelected;
}

/**
**  Toggle units from a particular type and belonging to the local player.
**
**  The base is included in the selection and defines
**  the type of the other units to be selected.
**
**  @param base  Toggle all units of same type.
**
**  @return      Number of units found, 0 means selection unchanged
**
**  FIXME: toggle not written
**  FIXME: should always select the nearest 9 units to the base!
*/
int ToggleUnitsByType(CUnit &base)
{
	const CUnitType &type = *base.Type;

	// if unit is a cadaver or hidden (not on map)
	// no unit can be selected.
	if (base.Removed || base.IsAlive() == false) {
		return 0;
	}
	// if unit isn't belonging to the player, or is a static unit
	// (like a building), only 1 unit can be selected at the same time.
	if (!CanSelectMultipleUnits(*base.Player) || !type.SelectableByRectangle) {
		return 0;
	}

	if (!SelectUnit(base)) { // Add base to selection
		return 0;
	}
	//
	//  Search for other visible units of the same type
	//

	// select all visible units.
	// StephanR: should be (MapX,MapY,MapX+MapWidth-1,MapY+MapHeight-1) ???
	// FIXME: this should probably be cleaner implemented if SelectUnitsByType()
	// took parameters of the selection rectangle as arguments */
	const CViewport *vp = UI.MouseViewport;
	const Vec2i offset(1, 1);
	const Vec2i minPos = vp->MapPos - offset;
	const Vec2i vpSize(vp->MapWidth, vp->MapHeight);
	const Vec2i maxPos = vp->MapPos + vpSize + offset;
	std::vector<CUnit *> table;

	Select(minPos, maxPos, table, HasSameTypeAs(type));

	// FIXME: peon/peasant with gold/wood & co are considered from
	// different type... idem for tankers
	for (size_t i = 0; i < table.size(); ++i) {
		CUnit &unit = *table[i];

		if (!CanSelectMultipleUnits(*unit.Player)) {
			continue;
		}
		if (unit.IsUnusable()) { // guess SelectUnits doesn't check this
			continue;
		}
		if (&unit == &base) { // no need to have the same unit twice
			continue;
		}
		if (unit.TeamSelected) { // Somebody else onteam has this unit
			continue;
		}
		if (!SelectUnit(unit)) { // add unit to selection
			return NumSelected;
		}
	}

	NetworkSendSelection(Selected, NumSelected);
	return NumSelected;
}

/**
**  Change selected units to units from group group_number
**  Doesn't change the selection if the group has no unit.
**
**  @param group_number  number of the group to be selected.
**
**  @return              number of units in the group.
*/
int SelectGroup(int group_number, GroupSelectionMode mode)
{
	int nunits = GetNumberUnitsOfGroup(group_number, SELECT_ALL);
	if (nunits) {
		if (mode == SELECT_ALL || !IsGroupTainted(group_number)) {
			ChangeSelectedUnits(GetUnitsOfGroup(group_number), nunits);
			return NumSelected;
		} else {
			std::vector<CUnit *> table;
			CUnit **group = GetUnitsOfGroup(group_number);

			for (int i = 0; i < nunits; ++i) {
				const CUnitType *type = group[i]->Type;
				if (type && type->CanSelect(mode)) {
					table.push_back(group[i]);
				}
			}
			if (table.empty() == false) {
				ChangeSelectedUnits(&table[0], static_cast<int>(table.size()));
				return NumSelected;
			}
		}
	}
	return 0;
}

/**
**  Add units from group of a particular unit to selection.
**
**  @param unit  unit belonging to the group to be selected.
**
**  @return      0 if the unit doesn't belong to a group,
**               or the number of units in the group.
*/
int AddGroupFromUnitToSelection(CUnit &unit)
{
	int group = unit.LastGroup;

	if (!group) { // belongs to no group
		return 0;
	}

	for (CUnitManager::Iterator it = UnitManager.begin(); it != UnitManager.end(); ++it) {
		CUnit &unit = **it;
		if (unit.LastGroup == group && !unit.Removed) {
			SelectUnit(unit);
			if (NumSelected == MaxSelectable) {
				return NumSelected;
			}
		}
	}
	return NumSelected;
}

/**
**  Select units from group of a particular unit.
**  Doesn't change the selection if the group has no unit,
**  or the unit doesn't belong to any group.
**
**  @param unit  unit belonging to the group to be selected.
**
**  @return      0 if the unit doesn't belong to a group,
**               or the number of units in the group.
*/
int SelectGroupFromUnit(CUnit &unit)
{
	if (!unit.LastGroup) { // belongs to no group
		return 0;
	}

	UnSelectAll();
	return AddGroupFromUnitToSelection(unit);
}

/**
**  Select the units selecteable by rectangle in a local table.
**  Act like a filter: The source table is modified.
**  Return the original table if no unit is found.
**
**  @param table      Input/Output table of units.
**
**  return true if at least a unit is found;
*/
static bool SelectOrganicUnitsInTable(std::vector<CUnit *> &table)
{
	int n = 0;

	for (size_t i = 0; i != table.size(); ++i) {
		CUnit &unit = *table[i];

		if (!CanSelectMultipleUnits(*unit.Player) || !unit.Type->SelectableByRectangle) {
			continue;
		}
		if (unit.IsUnusable()) {  // guess SelectUnits doesn't check this
			continue;
		}
		if (unit.TeamSelected) { // Somebody else onteam has this unit
			continue;
		}
		table[n++] = &unit;
		if (n == MaxSelectable) {
			break;
		}
	}
	if (n != 0) {
		table.resize(n);
		return true;
	}
	return false;
}

/**
**  Selects units from the table whose sprite is at least partially
**  covered by the rectangle. The rectangle is determined by coordinates
**  of its upper left and lower right corner expressed in screen map
**  coordinate system.
**
**  @param corner_topleft      coord of upper left corner of the rectangle
**  @param corner_bottomright  coord of lower right corner of the rectangle
**  @param table               table of units
**
**  @return           number of units found
*/
static void SelectSpritesInsideRectangle(const PixelPos &corner_topleft, const PixelPos &corner_bottomright,
										 std::vector<CUnit *> &table)
{
	int n = 0;

	for (size_t i = 0; i != table.size(); ++i) {
		CUnit &unit = *table[i];
		const CUnitType &type = *unit.Type;
		PixelPos spritePos = unit.GetMapPixelPosCenter();

		spritePos.x += type.OffsetX - (type.BoxWidth + type.BoxOffsetX) / 2;
		spritePos.y += type.OffsetY - (type.BoxHeight + type.BoxOffsetY) / 2;
		if (spritePos.x + type.BoxWidth + type.BoxOffsetX < corner_topleft.x
			|| spritePos.x > corner_bottomright.x
			|| spritePos.y + type.BoxHeight + type.BoxOffsetY < corner_topleft.y
			|| spritePos.y > corner_bottomright.y) {
			continue;
		}
		table[n++] = &unit;
	}
	table.resize(n);
}

/**
**  Select units in a rectangle.
**  Proceed in order in none found:
**    @li select local player mobile units
**    @li select one local player static unit (random)
**    @li select one neutral unit (critter, mine...)
**    @li select one enemy unit (random)
**
**  @param corner_topleft,  start of selection rectangle
**  @param corner_bottomright end of selection rectangle
**
**  @return     the number of units found.
*/
int SelectUnitsInRectangle(const PixelPos &corner_topleft, const PixelPos &corner_bottomright)
{
	const Vec2i t0 = Map.MapPixelPosToTilePos(corner_topleft);
	const Vec2i t1 = Map.MapPixelPosToTilePos(corner_bottomright);
	const Vec2i range(2, 2);
	std::vector<CUnit *> table;

	Select(t0 - range, t1 + range, table);
	SelectSpritesInsideRectangle(corner_topleft, corner_bottomright, table);

	// 1) search for the player units selectable with rectangle
	if (SelectOrganicUnitsInTable(table)) {
		const int size = static_cast<int>(table.size());
		ChangeSelectedUnits(&table[0], size);
		return size;
	}

	// 2) If no unit found, try a player's unit not selectable by rectangle
	for (size_t i = 0; i != table.size(); ++i) {
		CUnit &unit = *table[i];

		if (!CanSelectMultipleUnits(*unit.Player)) {
			continue;
		}
		// FIXME: Can we get this?
		if (!unit.Removed && unit.IsAlive()) {
			SelectSingleUnit(unit);
			return 1;
		}
	}

	// 3) If no unit found, try a resource or a neutral critter
	for (size_t i = 0; i != table.size(); ++i) {
		CUnit &unit = *table[i];
		// Unit visible FIXME: write function UnitSelectable
		if (!unit.IsVisibleInViewport(*UI.SelectedViewport)) {
			continue;
		}
		const CUnitType &type = *unit.Type;
		// Buildings are visible but not selectable
		if (type.Building && !unit.IsVisibleOnMap(*ThisPlayer)) {
			continue;
		}
		if ((type.GivesResource && !unit.Removed)) { // no built resources.
			SelectSingleUnit(unit);
			return 1;
		}
	}

	// 4) If no unit found, select an enemy unit (first found)
	for (size_t i = 0; i != table.size(); ++i) {
		CUnit &unit = *table[i];
		// Unit visible FIXME: write function UnitSelectable
		if (!unit.IsVisibleInViewport(*UI.SelectedViewport)) {
			continue;
		}
		// Buildings are visible but not selectable
		if (unit.Type->Building && !unit.IsVisibleOnMap(*ThisPlayer)) {
			continue;
		}
		if (unit.IsAliveOnMap()) {
			SelectSingleUnit(unit);
			return 1;
		}
	}
	return 0;
}

/**
**  Add the units in the rectangle to the current selection
**
**  @param corner_topleft,  start of selection rectangle
**  @param corner_bottomright end of selection rectangle
**
**  @return    the _total_ number of units selected.
*/
int AddSelectedUnitsInRectangle(const PixelPos &corner_topleft, const PixelPos &corner_bottomright)
{
	// Check if the original selected unit (if it's alone) is ours,
	// and can be selectable by rectangle.
	// In this case, do nothing.
	if (NumSelected == 1
		&& (!CanSelectMultipleUnits(*Selected[0]->Player)
			|| !Selected[0]->Type->SelectableByRectangle)) {
		return NumSelected;
	}
	// If there is no selected unit yet, do a simple selection.
	if (!NumSelected) {
		return SelectUnitsInRectangle(corner_topleft, corner_bottomright);
	}
	const Vec2i tilePos0 = Map.MapPixelPosToTilePos(corner_topleft);
	const Vec2i tilePos1 = Map.MapPixelPosToTilePos(corner_bottomright);
	const Vec2i range(2, 2);
	std::vector<CUnit *> table;

	Select(tilePos0 - range, tilePos1 + range, table);
	SelectSpritesInsideRectangle(corner_topleft, corner_bottomright, table);
	// If no unit in rectangle area... do nothing
	if (table.empty()) {
		return NumSelected;
	}

	// Now we should only have mobile (organic) units belonging to us,
	// so if there's no such units in the rectangle, do nothing.
	if (SelectOrganicUnitsInTable(table) == false) {
		return NumSelected;
	}

	for (size_t i = 0; i < table.size() && NumSelected < MaxSelectable; ++i) {
		SelectUnit(*table[i]);
	}
	return NumSelected;
}

/**
**  Select own ground units in a rectangle.
**
**  @param corner_topleft,  start of selection rectangle
**  @param corner_bottomright end of selection rectangle
**
**  @return     the number of units found.
*/
int SelectGroundUnitsInRectangle(const PixelPos &corner_topleft, const PixelPos &corner_bottomright)
{
	const Vec2i t0 = Map.MapPixelPosToTilePos(corner_topleft);
	const Vec2i t1 = Map.MapPixelPosToTilePos(corner_bottomright);
	const Vec2i range(2, 2);
	std::vector<CUnit *> table;

	Select(t0 - range, t1 + range, table);
	SelectSpritesInsideRectangle(corner_topleft, corner_bottomright, table);

	int n = 0;
	for (size_t i = 0; i != table.size(); ++i) {
		CUnit &unit = *table[i];

		if (!CanSelectMultipleUnits(*unit.Player) || !unit.Type->SelectableByRectangle) {
			continue;
		}
		if (unit.IsUnusable()) {  // guess SelectUnits doesn't check this
			continue;
		}
		if (unit.Type->UnitType == UnitTypeFly) {
			continue;
		}
		if (unit.TeamSelected) { // Somebody else onteam has this unit
			continue;
		}
		table[n++] = &unit;
		if (n == MaxSelectable) {
			break;
		}
	}
	if (n) {
		ChangeSelectedUnits(&table[0], n);
	}
	return n;
}

/**
**  Select own air units in a rectangle.
**
**  @param corner_topleft,  start of selection rectangle
**  @param corner_bottomright end of selection rectangle
**
**  @return     the number of units found.
*/
int SelectAirUnitsInRectangle(const PixelPos &corner_topleft, const PixelPos &corner_bottomright)
{
	const Vec2i t0 = Map.MapPixelPosToTilePos(corner_topleft);
	const Vec2i t1 = Map.MapPixelPosToTilePos(corner_bottomright);
	const Vec2i range(2, 2);
	std::vector<CUnit *> table;

	Select(t0 - range, t1 + range, table);
	SelectSpritesInsideRectangle(corner_topleft, corner_bottomright, table);
	int n = 0;
	for (size_t i = 0; i != table.size(); ++i) {
		CUnit &unit = *table[i];
		if (!CanSelectMultipleUnits(*unit.Player) || !unit.Type->SelectableByRectangle) {
			continue;
		}
		if (unit.IsUnusable()) { // guess SelectUnits doesn't check this
			continue;
		}
		if (unit.Type->UnitType != UnitTypeFly) {
			continue;
		}
		if (unit.TeamSelected) { // Somebody else onteam has this unit
			continue;
		}
		table[n++] = &unit;
		if (n == MaxSelectable) {
			break;
		}
	}
	if (n) {
		ChangeSelectedUnits(&table[0], n);
	}
	return n;
}


/**
**  Add the ground units in the rectangle to the current selection
**
**  @param corner_topleft,     start of selection rectangle
**  @param corner_bottomright  end of selection rectangle
**
**  @return     the number of units found.
*/
int AddSelectedGroundUnitsInRectangle(const PixelPos &corner_topleft, const PixelPos &corner_bottomright)
{
	// Check if the original selected unit (if it's alone) is ours,
	// and can be selectable by rectangle.
	// In this case, do nothing.
	if (NumSelected == 1
		&& (!CanSelectMultipleUnits(*Selected[0]->Player)
			|| !Selected[0]->Type->SelectableByRectangle)) {
		return NumSelected;
	}

	// If there is no selected unit yet, do a simple selection.
	if (!NumSelected) {
		return SelectGroundUnitsInRectangle(corner_topleft, corner_bottomright);
	}

	const Vec2i t0 = Map.MapPixelPosToTilePos(corner_topleft);
	const Vec2i t1 = Map.MapPixelPosToTilePos(corner_bottomright);
	const Vec2i range(2, 2);
	std::vector<CUnit *> table;

	Select(t0 - range, t1 + range, table);
	SelectSpritesInsideRectangle(corner_topleft, corner_bottomright, table);

	int n = 0;
	for (size_t i = 0; i < table.size(); ++i) {
		CUnit &unit = *table[i];

		if (!CanSelectMultipleUnits(*unit.Player) ||
			!unit.Type->SelectableByRectangle) {
			continue;
		}
		if (unit.IsUnusable()) {  // guess SelectUnits doesn't check this
			continue;
		}
		if (unit.Type->UnitType == UnitTypeFly) {
			continue;
		}
		if (unit.TeamSelected) { // Somebody else onteam has this unit
			continue;
		}
		table[n++] = &unit;
		if (n == MaxSelectable) {
			break;
		}
	}

	// Add the units to selected.
	for (int i = 0; i < n && NumSelected < MaxSelectable; ++i) {
		SelectUnit(*table[i]);
	}
	return NumSelected;
}

/**
**  Add the air units in the rectangle to the current selection
**
**  @param corner_topleft,     start of selection rectangle
**  @param corner_bottomright  end of selection rectangle
**
**  @return     the number of units found.
*/
int AddSelectedAirUnitsInRectangle(const PixelPos &corner_topleft, const PixelPos &corner_bottomright)
{
	// Check if the original selected unit (if it's alone) is ours,
	// and can be selectable by rectangle.
	// In this case, do nothing.
	if (NumSelected == 1
		&& (!CanSelectMultipleUnits(*Selected[0]->Player)
			|| !Selected[0]->Type->SelectableByRectangle)) {
		return NumSelected;
	}

	// If there is no selected unit yet, do a simple selection.
	if (!NumSelected) {
		return SelectAirUnitsInRectangle(corner_topleft, corner_bottomright);
	}

	const Vec2i t0 = Map.MapPixelPosToTilePos(corner_topleft);
	const Vec2i t1 = Map.MapPixelPosToTilePos(corner_bottomright);
	const Vec2i range(2, 2);
	std::vector<CUnit *> table;

	Select(t0 - range, t1 + range, table);
	SelectSpritesInsideRectangle(corner_topleft, corner_bottomright, table);
	int n = 0;
	for (size_t i = 0; i < table.size(); ++i) {
		CUnit &unit = *table[i];
		if (!CanSelectMultipleUnits(*unit.Player) ||
			!unit.Type->SelectableByRectangle) {
			continue;
		}
		if (unit.IsUnusable()) {  // guess SelectUnits doesn't check this
			continue;
		}
		if (unit.Type->UnitType != UnitTypeFly) {
			continue;
		}
		if (unit.TeamSelected) { // Somebody else onteam has this unit
			continue;
		}
		table[n++] = &unit;
		if (n == MaxSelectable) {
			break;
		}
	}

	// Add the units to selected.
	for (int i = 0; i < n && NumSelected < MaxSelectable; ++i) {
		SelectUnit(*table[i]);
	}
	return NumSelected;
}

/**
**  Initialize the selection module.
*/
void InitSelections()
{
	// This could have been initialized already when loading a game
	if (!Selected) {
		Selected = new CUnit *[MaxSelectable];
		_Selected = new CUnit *[MaxSelectable];
	}
	for (int i = 0; i < PlayerMax; ++i) {
		if (!TeamSelected[i]) {
			TeamSelected[i] = new CUnit *[MaxSelectable];
			_TeamSelected[i] = new CUnit *[MaxSelectable];
		}
	}
}

/**
**  Save current selection state.
**
**  @param file  Output file.
*/
void SaveSelections(CFile &file)
{
	file.printf("\n--- -----------------------------------------\n");
	file.printf("--- MODULE: selection\n\n");

	file.printf("SetGroupId(%d)\n", GroupId);
	file.printf("Selection(%d, {", NumSelected);
	for (int i = 0; i < NumSelected; ++i) {
		file.printf("\"%s\", ", UnitReference(*Selected[i]).c_str());
	}
	file.printf("})\n");
}

/**
**  Clean up the selection module.
*/
void CleanSelections()
{
	GroupId = 0;
	NumSelected = 0;
	delete[] Selected;
	Selected = NULL;
	delete[] _Selected;
	_Selected = NULL;

	for (int i = 0; i < PlayerMax; ++i) {
		delete[] TeamSelected[i];
		TeamSelected[i] = NULL;
		delete[] _TeamSelected[i];
		_TeamSelected[i] = NULL;
		TeamNumSelected[i] = 0;
		_TeamNumSelected[i] = 0;
	}
}

// ----------------------------------------------------------------------------

/**
**  Set the current group id. (Needed for load/save)
**
**  @param l  Lua state.
*/
static int CclSetGroupId(lua_State *l)
{
	int old;

	LuaCheckArgs(l, 1);
	old = GroupId;
	GroupId = LuaToNumber(l, 1);

	lua_pushnumber(l, old);
	return 1;
}

/**
**  Define the current selection.
**
**  @param l  Lua state.
*/
static int CclSelection(lua_State *l)
{
	LuaCheckArgs(l, 2);
	if (!lua_istable(l, 2)) {
		LuaError(l, "incorrect argument");
	}
	InitSelections();
	NumSelected = LuaToNumber(l, 1);
	const int args = lua_rawlen(l, 2);
	for (int j = 0; j < args; ++j) {
		lua_rawgeti(l, 2, j + 1);
		const char *str = LuaToString(l, -1);
		lua_pop(l, 1);
		Selected[j] = &UnitManager.GetSlotUnit(strtol(str + 1, NULL, 16));
	}
	return 0;
}

/**
**  Register CCL features for selections.
*/
void SelectionCclRegister()
{
	lua_register(Lua, "SetGroupId", CclSetGroupId);
	lua_register(Lua, "Selection", CclSelection);
}

//@}
