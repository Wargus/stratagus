//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name unit.cpp - The units. */
//
//      (c) Copyright 1998-2007 by Lutz Sammer and Jimmy Salmon
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
#include <string.h>
#include <limits.h>
#include <math.h>

#include "stratagus.h"

#include "unit.h"
#include "unit_manager.h"
#include "unit_cache.h"
#include "video.h"
#include "unitsound.h"
#include "unittype.h"
#include "animation.h"
#include "player.h"
#include "tileset.h"
#include "map.h"
#include "actions.h"
#include "sound_server.h"
#include "missile.h"
#include "interface.h"
#include "sound.h"
#include "ai.h"
#include "pathfinder.h"
#include "network.h"
#include "ui.h"
#include "script.h"
#include "editor.h"
#include "spells.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

CUnit *Units[MAX_UNIT_SLOTS];             /// Array of used slots
int NumUnits;                             /// Number of slots used

bool EnableBuildingCapture;               /// Config: capture buildings enabled

static unsigned long HelpMeLastCycle;     /// Last cycle HelpMe sound played
static int HelpMeLastX;                   /// Last X coordinate HelpMe sound played
static int HelpMeLastY;                   /// Last Y coordinate HelpMe sound played


/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

static void RemoveUnitFromContainer(CUnit *unit);

/**
**  Increase a unit's reference count.
*/
void CUnit::RefsIncrease()
{
	RefsAssert(Refs && !Destroyed);
	if (!SaveGameLoading) {
		++Refs;
	}
}

/**
**  Decrease a unit's reference count.
*/
void CUnit::RefsDecrease()
{
	RefsAssert(Refs);
	if (!SaveGameLoading) {
		if (Destroyed) {
			if (!--Refs) {
				Release();
			}
		} else {
			--Refs;
			RefsAssert(Refs);
		}
	}
}

/**
**  Release a unit.
**
**  The unit is only released, if all references are dropped.
*/
void CUnit::Release()
{
	CUnit *temp;

	Assert(Type); // already free.
	Assert(OrderCount == 1);
	Assert(!Orders[0]->Goal);
	// Must be removed before here
	Assert(Removed);

	//
	// First release, remove from lists/tables.
	//
	if (!Destroyed) {
		DebugPrint("First release %d\n" _C_ Slot);

		//
		// Are more references remaining?
		//
		Destroyed = 1; // mark as destroyed

		if (Container) {
			MapUnmarkUnitSight(this);
			RemoveUnitFromContainer(this);
		}

		if (--Refs > 0) {
			return;
		}
	}

	RefsAssert(!Refs);

	//
	// No more references remaining, but the network could have an order
	// on the way. We must wait a little time before we could free the
	// memory.
	//

	//
	// Remove the unit from the global units table.
	//
	Assert(*UnitSlot == this);
	temp = Units[--NumUnits];
	temp->UnitSlot = UnitSlot;
	*UnitSlot = temp;
	Units[NumUnits] = NULL;

	Type = NULL;  // for debugging.

	delete[] AutoCastSpell;
	delete[] Variable;
	for (std::vector<COrder *>::iterator order = Orders.begin(); order != Orders.end(); ++order) {
		delete *order;
	}
	Orders.clear();

	UnitManager.ReleaseUnit(this);
}

/**
**  Initialize the unit slot with default values.
**
**  @param type    Unit-type
*/
void CUnit::Init(CUnitType *type)
{
	Assert(type);

	//  Set refs to 1. This is the "I am alive ref", lost in ReleaseUnit.
	Refs = 1;

	//
	//  Build all unit table
	//
	UnitSlot = &Units[NumUnits]; // back pointer
	Units[NumUnits++] = this;

	//
	//  Initialise unit structure (must be zero filled!)
	//
	Type = type;

	Seen.Frame = UnitNotSeen; // Unit isn't yet seen

	Frame = type->StillFrame;

	if (UnitTypeVar.NumberVariable) {
		Assert(!Variable);
		Variable = new CVariable[UnitTypeVar.NumberVariable];
		memcpy(Variable, Type->Variable,
			UnitTypeVar.NumberVariable * sizeof(*Variable));
	}

	// Set a heading for the unit if it Handles Directions
	// Don't set a building heading, as only 1 construction direction
	//   is allowed.
	if (type->NumDirections > 1 && type->Sprite && !type->Building) {
		Direction = (MyRand() >> 8) & 0xFF; // random heading
		UnitUpdateHeading(this);
	}

	if (type->CanCastSpell) {
		AutoCastSpell = new char[SpellTypeTable.size()];
		if (Type->AutoCastActive) {
			memcpy(AutoCastSpell, Type->AutoCastActive, SpellTypeTable.size());
		} else {
			memset(AutoCastSpell, 0, SpellTypeTable.size());
		}
	}

	Removed = 1;

	for (int i = 0; i < MaxCosts; ++i) {
		ResourcesHeld[i] = type->ProductionCosts[i];
	}

	Assert(Orders.empty());

	Orders.push_back(new COrder);

	OrderCount = 1; // No orders
	Orders[0]->Action = UnitActionStill;
	Orders[0]->X = Orders[0]->Y = -1;
	Assert(!Orders[0]->Goal);
	NewOrder.Action = UnitActionStill;
	NewOrder.X = NewOrder.Y = -1;
	Assert(!NewOrder.Goal);
	SavedOrder.Action = UnitActionStill;
	SavedOrder.X = SavedOrder.Y = -1;
	Assert(!SavedOrder.Goal);
}

/**
**  Assigns a unit to a player, adjusting buildings, food and totals
**
**  @param player  player which have the unit.
*/
void CUnit::AssignToPlayer(CPlayer *player)
{
	CUnitType *type;  // type of unit.

	Assert(player);
	type = Type;

	//
	// Build player unit table
	//
	if (!type->Vanishes && Orders[0]->Action != UnitActionDie) {
		PlayerSlot = player->Units + player->TotalNumUnits++;
		if (!SaveGameLoading) {
			// If unit is dying, it's already been lost by all players
			// don't count again
			if (type->Building) {
				player->TotalBuildings++;
			} else {
				player->TotalUnits++;
			}
		}
		*PlayerSlot = this;

		player->UnitTypesCount[type->Slot]++;
	}


	// Don't Add the building if it's dieing, used to load a save game
	if (type->Building && Orders[0]->Action != UnitActionDie) {
		player->NumBuildings++;
	}
	Player = player;
	Stats = &type->Stats[Player->Index];
	Colors = &player->UnitColors;
	if (!SaveGameLoading) {
		if (UnitTypeVar.NumberVariable) {
			Assert(Variable);
			Assert(Stats->Variables);
			memcpy(Variable, Stats->Variables,
				UnitTypeVar.NumberVariable * sizeof(*Variable));
		}
	}
}

/**
**  Create a new unit.
**
**  @param type      Pointer to unit-type.
**  @param player    Pointer to owning player.
**
**  @return          Pointer to created unit.
*/
CUnit *MakeUnit(CUnitType *type, CPlayer *player)
{
	CUnit *unit;

	//
	// Game unit limit reached.
	//
	if (NumUnits >= UnitMax) {
		DebugPrint("Over all unit limit (%d) reached.\n" _C_ UnitMax);
		return NoUnitP;
	}

	unit = UnitManager.AllocUnit();
	if (unit == NoUnitP) {
		return NoUnitP;
	}

	unit->Init(type);

	// Only Assign if a Player was specified
	if (player) {
		unit->AssignToPlayer(player);
	}

	return unit;
}

/**
**  (Un)Mark on vision table the Sight of the unit
**  (and units inside for transporter (recursively))
**
**  @param unit      Unit to (un)mark.
**  @param x         X coord of first container of unit.
**  @param y         Y coord of first container of unit.
**  @param width     Width of the first container of unit.
**  @param height    Height of the first container of unit.
**  @param f         Function to (un)mark for normal vision.
*/
static void MapMarkUnitSightRec(const CUnit *unit, int x, int y, int width, int height,
	MapMarkerFunc *f)
{
	CUnit *unit_inside;

	Assert(unit);
	Assert(f);
	MapSight(unit->Player, x, y, width, height,
		unit->Container ? unit->Container->CurrentSightRange : unit->CurrentSightRange, f);

	unit_inside = unit->UnitInside;
	for (int i = unit->InsideCount; i--; unit_inside = unit_inside->NextContained) {
		MapMarkUnitSightRec(unit_inside, x, y, width, height, f);
	}
}

/**
**  Return the unit not transported, by viewing the container recursively.
**
**  @param unit    unit from where look the first conatiner.
**
**  @return        Container of container of ... of unit. It is not null.
*/
static CUnit *GetFirstContainer(const CUnit *unit)
{
	Assert(unit);
	while (unit->Container) {
		unit = unit->Container;
	}
	return const_cast<CUnit *>(unit);
}

/**
**  Mark on vision table the Sight of the unit
**  (and units inside for transporter)
**
**  @param unit    unit to unmark its vision.
**  @see MapUnmarkUnitSight.
*/
void MapMarkUnitSight(CUnit *unit)
{
	CUnit *container;  // First container of the unit.

	Assert(unit);

	container = GetFirstContainer(unit);
	Assert(container->Type);

	MapMarkUnitSightRec(unit,
		container->X, container->Y, container->Type->TileWidth, container->Type->TileHeight,
		MapMarkTileSight);

	// Never mark radar, except if the top unit, and unit is usable
	if (unit == container && !unit->IsUnusable()) {
		if (unit->Stats->Variables[RADAR_INDEX].Value) {
			MapMarkRadar(unit->Player, unit->X, unit->Y, unit->Type->TileWidth,
				unit->Type->TileHeight, unit->Stats->Variables[RADAR_INDEX].Value);
		}
		if (unit->Stats->Variables[RADARJAMMER_INDEX].Value) {
			MapMarkRadarJammer(unit->Player, unit->X, unit->Y, unit->Type->TileWidth,
				unit->Type->TileHeight, unit->Stats->Variables[RADARJAMMER_INDEX].Value);
		}
	}
}

/**
**  Unmark on vision table the Sight of the unit
**  (and units inside for transporter)
**
**  @param unit    unit to unmark its vision.
**  @see MapMarkUnitSight.
*/
void MapUnmarkUnitSight(CUnit *unit)
{
	CUnit *container;  // First container of the unit.

	Assert(unit);
	Assert(unit->Type);

	container = GetFirstContainer(unit);
	Assert(container->Type);
	MapMarkUnitSightRec(unit,
		container->X, container->Y, container->Type->TileWidth, container->Type->TileHeight,
		MapUnmarkTileSight);

	// Never mark radar, except if the top unit?
	if (unit == container && !unit->IsUnusable()) {
		if (unit->Stats->Variables[RADAR_INDEX].Value) {
			MapUnmarkRadar(unit->Player, unit->X, unit->Y, unit->Type->TileWidth,
				unit->Type->TileHeight, unit->Stats->Variables[RADAR_INDEX].Value);
		}
		if (unit->Stats->Variables[RADARJAMMER_INDEX].Value) {
			MapUnmarkRadarJammer(unit->Player, unit->X, unit->Y, unit->Type->TileWidth,
				unit->Type->TileHeight, unit->Stats->Variables[RADARJAMMER_INDEX].Value);
		}
	}
}

/**
**  Update the Unit Current sight range to good value and transported units inside.
**
**  @param unit    unit to update SightRange
**
**  @internal before use it, MapUnmarkUnitSight(unit)
**  and after MapMarkUnitSight(unit)
**  are often necessary.
**
**  FIXME @todo manage differently unit inside with option.
**  (no vision, min, host value, own value, bonus value, ...)
*/
void UpdateUnitSightRange(CUnit *unit)
{
	CUnit *unit_inside; // iterator on units inside unit.
	int i;             // number of units inside to process.

#if 0 // which is the better ? caller check ?
	if (SaveGameLoading) {
		return ;
	}
#else
	Assert(!SaveGameLoading);
#endif
	// FIXME : these values must be configurable.
	if (unit->Constructed) { // Units under construction have no sight range.
		unit->CurrentSightRange = 0;
	} else if (!unit->Container) { // proper value.
		unit->CurrentSightRange = unit->Stats->Variables[SIGHTRANGE_INDEX].Max;
	} else { // value of it container.
		unit->CurrentSightRange = unit->Container->CurrentSightRange;
	}

	unit_inside = unit->UnitInside;
	for (i = unit->InsideCount; i--; unit_inside = unit_inside->NextContained) {
		UpdateUnitSightRange(unit_inside);
	}
}

/**
**  Mark the field with the FieldFlags.
**
**  @param unit    unit to mark.
*/
void MarkUnitFieldFlags(const CUnit *unit)
{
	CUnitType *type; // Type of the unit.
	unsigned flags; //
	int h;          // Tile height of the unit.
	int w;          // Tile width of the unit.
	int x;          // X tile of the unit.
	int y;          // Y tile of the unit.

	Assert(unit);
	type = unit->Type;
	x = unit->X;
	y = unit->Y;
	flags = type->FieldFlags;
	for (h = type->TileHeight; h--;) {
		for (w = type->TileWidth; w--;) {
			Map.Field(x + w, y + h)->Flags |= flags;
		}
	}
}

/**
**  Mark the field with the FieldFlags.
**
**  @param unit    unit to mark.
*/
void UnmarkUnitFieldFlags(const CUnit *unit)
{
	CUnitType *type; // Type of the unit.
	unsigned flags; //
	int h;          // Tile height of the unit.
	int w;          // Tile width of the unit.
	int x;          // X tile of the unit.
	int y;          // Y tile of the unit.
	CUnit *table[UnitMax];

	Assert(unit);
	type = unit->Type;
	x = unit->X;
	y = unit->Y;
	flags = type->FieldFlags;
	for (h = type->TileHeight; h--;) {
		for (w = type->TileWidth; w--;) {
			Map.Field(x + w, y + h)->Flags &= ~flags;

			int n = UnitCache.Select(x + w, y + h, table, UnitMax);
			while (n--) {
				if (table[n] != unit && table[n]->Orders[0]->Action != UnitActionDie) {
					Map.Field(x + w, y + h)->Flags |= table[n]->Type->FieldFlags;
				}
			}
		}
	}
}

/**
**  Add unit to a container. It only updates linked list stuff.
**
**  @param host  Pointer to container.
*/
void CUnit::AddInContainer(CUnit *host)
{
	Assert(host && Container == 0);
	Container = host;
	if (host->InsideCount == 0) {
		NextContained = PrevContained = this;
	} else {
		NextContained = host->UnitInside;
		PrevContained = host->UnitInside->PrevContained;
		host->UnitInside->PrevContained->NextContained = this;
		host->UnitInside->PrevContained = this;
	}
	host->UnitInside = this;
	host->InsideCount++;
}

/**
**  Remove unit from a container. It only updates linked list stuff.
**
**  @param unit    Pointer to unit.
*/
static void RemoveUnitFromContainer(CUnit *unit)
{
	CUnit *host;  // transporter which contain unit.

	host = unit->Container;
	Assert(unit->Container);
	Assert(unit->Container->InsideCount > 0);
	host->InsideCount--;
	unit->NextContained->PrevContained = unit->PrevContained;
	unit->PrevContained->NextContained = unit->NextContained;
	if (host->InsideCount == 0) {
		host->UnitInside = NoUnitP;
	} else {
		if (host->UnitInside == unit) {
			host->UnitInside = unit->NextContained;
		}
	}
	unit->Container = NoUnitP;
}


/**
**  Affect Tile coord of a unit (with units inside) to tile (x, y).
**
**  @param unit    unit to move.
**  @param x       X map tile position.
**  @param y       Y map tile position.
**
**  @internal before use it, UnitCacheRemove(unit), MapUnmarkUnitSight(unit)
**  and after UnitCacheInsert(unit), MapMarkUnitSight(unit)
**  are often necessary. Check Flag also for Pathfinder.
*/
static void UnitInXY(CUnit *unit, int x, int y)
{
	CUnit *unit_inside;      // iterator on units inside unit.

	Assert(unit);
	unit->X = x;
	unit->Y = y;

	unit_inside = unit->UnitInside;
	for (int i = unit->InsideCount; i--; unit_inside = unit_inside->NextContained) {
		UnitInXY(unit_inside, x, y);
	}
}

/**
**  Move a unit (with units inside) to tile (x, y).
**  (Do stuff with vision, cachelist and pathfinding).
**
**  @param x  X map tile position.
**  @param y  Y map tile position.
**
*/
void CUnit::MoveToXY(int x, int y)
{
	MapUnmarkUnitSight(this);
	UnitCache.Remove(this);
	UnmarkUnitFieldFlags(this);

	Assert(UnitCanBeAt(this, x, y));
	// Move the unit.
	UnitInXY(this, x, y);

	UnitCache.Insert(this);
	MarkUnitFieldFlags(this);
	//  Recalculate the seen count.
	UnitCountSeen(this);
	MapMarkUnitSight(this);
}

/**
**  Place unit on map.
**
**  @param x  X map tile position.
**  @param y  Y map tile position.
*/
void CUnit::Place(int x, int y)
{
	Assert(Removed);

	if (Container) {
		MapUnmarkUnitSight(this);
		RemoveUnitFromContainer(this);
	}
	if (!SaveGameLoading) {
		UpdateUnitSightRange(this);
	}
	Removed = 0;
	UnitInXY(this, x, y);
	// Pathfinding info.
	MarkUnitFieldFlags(this);
	// Tha cache list.
	UnitCache.Insert(this);
	//  Calculate the seen count.
	UnitCountSeen(this);
	// Vision
	MapMarkUnitSight(this);
}

/**
**  Create new unit and place on map.
**
**  @param x         X map tile position.
**  @param y         Y map tile position.
**  @param type      Pointer to unit-type.
**  @param player    Pointer to owning player.
**
**  @return          Pointer to created unit.
*/
CUnit *MakeUnitAndPlace(int x, int y, CUnitType *type, CPlayer *player)
{
	CUnit *unit;

	unit = MakeUnit(type, player);

	if (unit != NoUnitP) {
		unit->Place(x, y);
	}

	return unit;
}

/**
**  Remove unit from map.
**
**  Update selection.
**  Update panels.
**  Update map.
**
**  @param host  Pointer to housing unit.
*/
void CUnit::Remove(CUnit *host)
{
	if (Removed) { // could happen!
		// If unit is removed (inside) and building is destroyed.
		DebugPrint("unit '%s(%d)' already removed\n" _C_ Type->Ident.c_str() _C_ Slot);
		return;
	}
	UnitCache.Remove(this);
	MapUnmarkUnitSight(this);
	UnmarkUnitFieldFlags(this);
	if (host) {
		AddInContainer(host);
		UpdateUnitSightRange(this);
		UnitInXY(this, host->X, host->Y);
		MapMarkUnitSight(this);
	}

	Removed = 1;

	//  Remove unit from the current selection
	if (Selected) {
		if (NumSelected == 1) { //  Remove building cursor
			CancelBuildingMode();
		}
		UnSelectUnit(this);
		SelectionChanged();
	}
	// Remove unit from team selections
	if (!Selected && TeamSelected) {
		UnSelectUnit(this);
	}

	// Unit is seen as under cursor
	if (this == UnitUnderCursor) {
		UnitUnderCursor = NULL;
	}
}

/**
**  Update information for lost units.
**
**  @param unit  Pointer to unit.
**
**  @note Also called by ChangeUnitOwner
*/
void UnitLost(CUnit *unit)
{
	CUnit *temp;
	CBuildRestrictionOnTop *b;
	const CUnitType *type;
	CPlayer *player;

	Assert(unit);

	player = unit->Player;
	Assert(player);  // Next code didn't support no player!

	//
	//  Call back to AI, for killed or lost units.
	//
	if (player && player->AiEnabled) {
		AiUnitKilled(unit);
	}

	//
	//  Remove unit from its groups
	//
	if (unit->GroupId) {
		RemoveUnitFromGroups(unit);
	}

	//
	//  Remove the unit from the player's units table.
	//
	type = unit->Type;
	if (player && !type->Vanishes) {
		Assert(*unit->PlayerSlot == unit);
		temp = player->Units[--player->TotalNumUnits];
		temp->PlayerSlot = unit->PlayerSlot;
		*unit->PlayerSlot = temp;
		player->Units[player->TotalNumUnits] = NULL;

		if (unit->Type->Building) {
			player->NumBuildings--;
		}

		if (unit->Orders[0]->Action != UnitActionBuilt) {
			player->UnitTypesCount[type->Slot]--;
		}
	}

	DebugPrint("Lost %s(%d)\n" _C_ unit->Type->Ident.c_str() _C_ UnitNumber(unit));

	// Destroy resource-platform, must re-make resource patch.
	if ((b = OnTopDetails(unit, NULL)) != NULL) {
		if (b->ReplaceOnDie && (unit->Type->CanHarvestFrom && UnitHoldsResources(unit))) {
			temp = MakeUnitAndPlace(unit->X, unit->Y, b->Parent, &Players[PlayerNumNeutral]);
			if (temp == NoUnitP) {
				DebugPrint("Unable to allocate Unit");
			} else {
				memcpy(temp->ResourcesHeld, unit->ResourcesHeld, sizeof(temp->ResourcesHeld));
			}
		}
	}
	Assert(player->NumBuildings <= UnitMax);
	Assert(player->TotalNumUnits <= UnitMax);
	Assert(player->UnitTypesCount[type->Slot] <= UnitMax);
}

/**
**  Removes all orders from a unit.
**
**  @param unit  The unit that will have all its orders cleared
*/
void UnitClearOrders(CUnit *unit)
{
	int i;

	//
	//  Release all references of the unit.
	//
	for (i = unit->OrderCount; i-- > 0;) {
		if (unit->Orders[i]->Goal) {
			unit->Orders[i]->Goal->RefsDecrease();
			unit->Orders[i]->Goal = NoUnitP;
		}
		if (i != 0) {
			unit->Orders.pop_back();
		}
	}
	unit->OrderCount = 1;
	if (unit->NewOrder.Goal) {
		unit->NewOrder.Goal->RefsDecrease();
		unit->NewOrder.Goal = NoUnitP;
	}
	if (unit->SavedOrder.Goal) {
		unit->SavedOrder.Goal->RefsDecrease();
		unit->SavedOrder.Goal = NoUnitP;
	}
	unit->Orders[0]->Action = UnitActionStill;
	unit->SubAction = unit->State = 0;
}

/**
**  Update for new unit. Food and income ...
**
**  @param unit     New unit pointer.
**  @param upgrade  True unit was upgraded.
*/
void UpdateForNewUnit(const CUnit *unit, int upgrade)
{
	const CUnitType *type = unit->Type;
	CPlayer *player = unit->Player;

	//
	// Update resources
	//
	for (int u = 0; u < MaxCosts; ++u) {
		player->ProductionRate[u] += type->ProductionRate[u] * unit->ProductionEfficiency / 100;
		player->StorageCapacity[u] += type->StorageCapacity[u];
	}
}

/**
**  Find nearest point of unit.
**
**  @param unit  Pointer to unit.
**  @param tx    X tile map postion.
**  @param ty    Y tile map postion.
**  @param dx    Out: nearest point X tile map postion to (tx,ty).
**  @param dy    Out: nearest point Y tile map postion to (tx,ty).
*/
void NearestOfUnit(const CUnit *unit, int tx, int ty, int *dx, int *dy)
{
	int x;
	int y;

	x = unit->X;
	y = unit->Y;

	if (tx >= x + unit->Type->TileWidth) {
		*dx = x + unit->Type->TileWidth - 1;
	} else if (tx < x) {
		*dx = x;
	} else {
		*dx = tx;
	}
	if (ty >= y + unit->Type->TileHeight) {
		*dy = y + unit->Type->TileHeight - 1;
	} else if (ty < y) {
		*dy = y;
	} else {
		*dy = ty;
	}
}

/**
**  Copy the unit look in Seen variables. This should be called when
**  buildings go under fog of war for ThisPlayer.
**
**  @param unit    The unit to work on
*/
static void UnitFillSeenValues(CUnit *unit)
{
	// Seen values are undefined for visible units.
	unit->Seen.Y = unit->Y;
	unit->Seen.X = unit->X;
	unit->Seen.IY = unit->IY;
	unit->Seen.IX = unit->IX;
	unit->Seen.Frame = unit->Frame;
	unit->Seen.State = (unit->Orders[0]->Action == UnitActionBuilt);
	if (unit->Orders[0]->Action == UnitActionDie) {
		unit->Seen.State = 3;
	}
	unit->Seen.Type = unit->Type;
	unit->Seen.Constructed = unit->Constructed;
	unit->Seen.CFrame = unit->Data.Built.Frame;
}

/**
**  This function should get called when a unit goes under fog of war.
**
**  @param unit    The unit that goes under fog.
**  @param player  The player the unit goes out of fog for.
*/
void UnitGoesUnderFog(CUnit *unit, const CPlayer *player)
{
	if (unit->Type->VisibleUnderFog) {
		if (player->Type == PlayerPerson && !unit->Destroyed) {
			unit->RefsIncrease();
		}
		//
		// Icky yucky icky Seen.Destroyed trickery.
		// We track for each player if he's seen the unit as destroyed.
		// Remember, a unit is marked Destroyed when it's gone as in
		// completely gone, the corpses vanishes. In that case the unit
		// only survives since some players did NOT see the unit destroyed.
		// Keeping trackof that is hard, mostly due to complex shared vision
		// configurations.
		// A unit does NOT get a reference when it goes under fog if it's
		// Destroyed. Furthermore, it shouldn't lose a reference if it was
		// Seen destroyed. That only happend with complex shared vision, and
		// it's sort of the whole point of this tracking.
		//
		if (unit->Destroyed) {
			unit->Seen.Destroyed |= (1 << player->Index);
		}
		if (player == ThisPlayer) {
			UnitFillSeenValues(unit);
		}
	}
}

/**
**  This function should get called when a unit goes out of fog of war.
**
**  @param unit    The unit that goes out of fog.
**  @param player  The player the unit goes out of fog for.
**
**  @note For units that are visible under fog (mostly buildings)
**  we use reference counts, from the players that know about
**  the building. When an building goes under fog it gets a refs
**  increase, and when it shows up it gets a decrease. It must
**  not get an decrease the first time it's seen, so we have to
**  keep track of what player saw what units, with SeenByPlayer.
*/
void UnitGoesOutOfFog(CUnit *unit, const CPlayer *player)
{
	if (unit->Type->VisibleUnderFog) {
		if (unit->Seen.ByPlayer & (1 << (player->Index))) {
			if ((player->Type == PlayerPerson) &&
					(!(   unit->Seen.Destroyed & (1 << player->Index)   )) ) {
				unit->RefsDecrease();
			}
		} else {
			unit->Seen.ByPlayer |= (1 << (player->Index));
		}
	}
}

/**
**  Mark all units on a tile as now visible.
**
**  @param player  The player this is for.
**  @param x       x location to check
**  @param y       y location to check
*/
void UnitsOnTileMarkSeen(const CPlayer *player, int x, int y)
{
	int p;
	int n;
	CUnit *units[UnitMax];
	CUnit *unit;

	n = UnitCache.Select(x, y, units, UnitMax);
	while (n) {
		unit = units[--n];
		//
		//  If the unit goes out of fog, this can happen for any player that
		//  this player shares vision with, and can't YET see the unit.
		//  It will be able to see the unit after the Unit->VisCount ++
		//
		for (p = 0; p < PlayerMax; ++p) {
			if (player->IsBothSharedVision(&Players[p]) || (p == player->Index)) {
				if (!unit->IsVisible(Players + p)) {
					UnitGoesOutOfFog(unit, Players + p);
				}
			}
		}
		unit->VisCount[player->Index]++;
	}
}

/**
**  This function unmarks units on x, y as seen. It uses a reference count.
**
**  @param player    The player to mark for.
**  @param x         x location to check if building is on, and mark as seen
**  @param y         y location to check if building is on, and mark as seen
*/
void UnitsOnTileUnmarkSeen(const CPlayer *player, int x, int y)
{
	int p;
	int n;
	CUnit *units[UnitMax];
	CUnit *unit;

	n = UnitCache.Select(x, y, units, UnitMax);
	while (n) {
		unit = units[--n];
		Assert(unit->X <= x && unit->X + unit->Type->TileWidth - 1 >= x &&
			unit->Y <= y && unit->Y + unit->Type->TileHeight - 1 >= y);

		p = player->Index;
		Assert(unit->VisCount[p]);
		unit->VisCount[p]--;
		//
		//  If the unit goes under of fog, this can happen for any player that
		//  this player shares vision to. First of all, before unmarking,
		//  every player that this player shares vision to can see the unit.
		//  Now we have to check who can't see the unit anymore.
		//
		if (!unit->VisCount[p]) {
			for (p = 0; p < PlayerMax; ++p) {
				if (player->IsBothSharedVision(&Players[p]) || p == player->Index) {
					if (!unit->IsVisible(Players + p)) {
						UnitGoesUnderFog(unit, Players + p);
					}
				}
			}
		}
	}
}

/**
**  Recalculates a units visiblity count. This happens really often,
**  Like every time a unit moves. It's really fast though, since we
**  have per-tile counts.
**
**  @param unit    pointer to the unit to check if seen
*/
void UnitCountSeen(CUnit *unit)
{
	int x;
	int y;
	int p;
	int oldv[PlayerMax];
	int newv;

	Assert(unit->Type);

	// FIXME: optimize, only work on certain players?
	// This is for instance good for updating shared vision...

	//
	//  Store old values in oldv[p]. This store if the player could see the
	//  unit before this calc.
	//
	for (p = 0; p < PlayerMax; ++p) {
		if (Players[p].Type != PlayerNobody) {
			oldv[p] = unit->IsVisible(&Players[p]);
		}
	}

	//  Calculate new VisCount values.
	for (p = 0; p < PlayerMax; ++p) {
		if (Players[p].Type != PlayerNobody) {
			newv = 0;
			for (x = 0; x < unit->Type->TileWidth; ++x) {
				for (y = 0; y < unit->Type->TileHeight; ++y) {
					//  Icky ugly code trick. With NoFogOfWar we have to be > 0;
					if (Map.Field(unit->X + x, unit->Y + y)->Visible[p] >
							1 - (Map.NoFogOfWar ? 1 : 0)) {
						newv++;
					}
				}
			}
			unit->VisCount[p] = newv;
		}
	}

	//
	// Now here comes the tricky part. We have to go in and out of fog
	// for players. Hopefully this works with shared vision just great.
	//
	for (p = 0; p < PlayerMax; ++p) {
		if (Players[p].Type != PlayerNobody) {
			newv = unit->IsVisible(Players + p);
			if ((!oldv[p]) &&  (newv)) {
				UnitGoesOutOfFog(unit, Players + p);
			}
			if ( (oldv[p]) && (!newv)) {
				UnitGoesUnderFog(unit, Players + p);
			}
		}
	}
}

/**
**  Returns true, if the unit is visible. It check the Viscount of
**  the player and everyone who shares vision with him.
**
**  @note This understands shared vision, and should be used all around.
**
**  @param player  The player to check.
*/
bool CUnit::IsVisible(const CPlayer *player) const
{
	if (VisCount[player->Index]) {
		return true;
	}
	for (int p = 0; p < PlayerMax; ++p) {
		if (player->IsBothSharedVision(&Players[p])) {
			if (VisCount[p]) {
				return true;
			}
		}
	}
	return false;
}

/**
**  Returns true, if unit is visible as an action goal for a player
**  on the map.
**
**  @param player  Player to check for.
**
**  @return        True if visible, false otherwise.
*/
bool CUnit::IsVisibleAsGoal(const CPlayer *player) const
{
	if (IsVisible(player) || player->Type == PlayerComputer ||
			UnitVisibleOnRadar(player, this)) {
		return !Removed && !Destroyed && Orders[0]->Action != UnitActionDie;
	} else {
		return Type->VisibleUnderFog &&
			(Seen.ByPlayer & (1 << player->Index)) &&
			!(Seen.Destroyed & (1 << player->Index));
	}
}

/**
**  Returns true, if unit is visible for this player on the map.
**  The unit has to be out of fog of war and alive
**
**  @param player  Player to check for.
**
**  @return        True if visible, false otherwise.
*/
bool CUnit::IsVisibleOnMap(const CPlayer *player) const
{
	return !Removed && !Destroyed &&
		Orders[0]->Action != UnitActionDie && IsVisible(player);
}

/**
**  Returns true, if unit is shown on minimap.
**
**  @warning This is for ::ThisPlayer only.
**  @todo radar support
**
**  @return      True if visible, false otherwise.
*/
bool CUnit::IsVisibleOnMinimap() const
{
	if (IsVisible(ThisPlayer) || ReplayRevealMap ||
			UnitVisibleOnRadar(ThisPlayer, this)) {
		return !Removed && !Destroyed && (Orders[0]->Action != UnitActionDie);
	} else {
		if (!Type->VisibleUnderFog) {
			return false;
		}
		return (Seen.ByPlayer & (1 << ThisPlayer->Index)) &&
			Seen.State != 3 && !(Seen.Destroyed & (1 << ThisPlayer->Index));
	}
}

/**
**  Returns true, if unit is visible in viewport.
**
**  @warning This is only true for ::ThisPlayer
**
**  @param vp  Viewport pointer.
**
**  @return    True if visible, false otherwise.
*/
bool CUnit::IsVisibleInViewport(const CViewport *vp) const
{
	//
	// Check if it's at least inside the damn viewport.
	//
	if ((X + Type->TileWidth < vp->MapX) ||
			(X > vp->MapX + vp->MapWidth) ||
			(Y + Type->TileHeight < vp->MapY) ||
			(Y > vp->MapY + vp->MapHeight)) {
		return false;
	}

	if (!ThisPlayer) {
		//FIXME: ARI: Added here for early game setup state by
		// MakeAndPlaceUnit() from LoadMap(). ThisPlayer not yet set,
		// so don't show anything until first real map-draw.
		DebugPrint("FIXME: ThisPlayer not set yet?!\n");
		return false;
	}

	if (IsVisible(ThisPlayer) || ReplayRevealMap) {
		return !Destroyed;
	} else {
		// Unit has to be 'discovered'
		// Destroyed units ARE visible under fog of war, if we haven't seen them like that.
		if (!Destroyed || !(Seen.Destroyed & (1 << ThisPlayer->Index))) {
			return (Type->VisibleUnderFog && (Seen.ByPlayer & (1 << ThisPlayer->Index)));
		} else {
			return false;
		}
	}
}

/**
**  Returns true, if unit is visible on current map view (any viewport).
**
**  @return      True if visible, false otherwise.
*/
bool CUnit::IsVisibleOnScreen() const
{
	for (CViewport *vp = UI.Viewports; vp < UI.Viewports + UI.NumViewports; ++vp) {
		if (IsVisibleInViewport(vp)) {
			return true;
		}
	}
	return false;
}

/**
**  Get area of map tiles covered by unit, including its displacement.
**
**  @param sx    Out: Top left X tile map postion.
**  @param sy    Out: Top left Y tile map postion.
**  @param ex    Out: Bottom right X tile map postion.
**  @param ey    Out: Bottom right Y tile map postion.
**
**  @return      sx,sy,ex,ey defining area in Map
*/
void CUnit::GetMapArea(int *sx, int *sy, int *ex, int *ey) const
{
	*sx = X - (IX < 0);
	*ex = *sx + Type->TileWidth - !IX;
	*sy = Y - (IY < 0);
	*ey = *sy + Type->TileHeight - !IY;
}

/**
**  Change the unit's owner
**
**  @param newplayer  New owning player.
*/
void CUnit::ChangeOwner(CPlayer *newplayer)
{
	int i;
	CUnit *uins;
	CPlayer *oldplayer;

	oldplayer = Player;

	// This shouldn't happen
	if (oldplayer == newplayer) {
		DebugPrint("Change the unit owner to the same player???\n");
		return;
	}

	// Rescue all units in buildings/transporters.
	uins = UnitInside;
	for (i = InsideCount; i; --i, uins = uins->NextContained) {
		uins->ChangeOwner(newplayer);
	}

	//
	//  Must change food/gold and other.
	//
	UnitLost(this);

	//
	//  Now the new side!
	//

	// Insert into new player table.

	PlayerSlot = newplayer->Units + newplayer->TotalNumUnits++;
	if (Type->Building) {
		newplayer->TotalBuildings++;
	}
	else {
		newplayer->TotalUnits++;
	}
	*PlayerSlot = this;

	MapUnmarkUnitSight(this);
	Player = newplayer;
	Stats = &Type->Stats[newplayer->Index];
	UpdateUnitSightRange(this);
	MapMarkUnitSight(this);

	//
	//  Must change food/gold and other.
	//
	if (Type->CanHarvestFrom) {
		DebugPrint("Resource transfer not supported\n");
	}
	if (Type->Building) {
		newplayer->NumBuildings++;
	}
	newplayer->UnitTypesCount[Type->Slot]++;

	UpdateForNewUnit(this, 1);
}


/**
**  Rescue units.
**
**  Look through all rescueable players, if they could be rescued.
*/
void RescueUnits(void)
{
	CPlayer *p;
	CUnit *unit;
	CUnit *table[UnitMax];
	CUnit *around[UnitMax];
	int n;
	int i;
	int j;
	int l;

	if (NoRescueCheck) {  // all possible units are rescued
		return;
	}
	NoRescueCheck = 1;

	//
	//  Look if player could be rescued.
	//
	for (p = Players; p < Players + NumPlayers; ++p) {
		if (p->Type != PlayerRescuePassive && p->Type != PlayerRescueActive) {
			continue;
		}
		if (p->TotalNumUnits) {
			NoRescueCheck = 0;
			// NOTE: table is changed.
			l = p->TotalNumUnits;
			memcpy(table, p->Units, l * sizeof(CUnit *));
			for (j = 0; j < l; ++j) {
				unit = table[j];
				// Do not rescue removed units. Units inside something are
				// rescued by ChangeUnitOwner
				if (unit->Removed) {
					continue;
				}

				if (unit->Type->UnitType == UnitTypeLand) {
					n = UnitCache.Select(
							unit->X - 1, unit->Y - 1,
							unit->X + unit->Type->TileWidth + 1,
							unit->Y + unit->Type->TileHeight + 1, around, UnitMax);
				} else {
					n = UnitCache.Select(
							unit->X - 2, unit->Y - 2,
							unit->X + unit->Type->TileWidth + 2,
							unit->Y + unit->Type->TileHeight + 2, around, UnitMax);
				}
				//
				//  Look if ally near the unit.
				//
				for (i = 0; i < n; ++i) {
					if (unit->IsAllied(around[i])) {
						unit->RescuedFrom = unit->Player;
						unit->ChangeOwner(around[i]->Player);
						unit->Blink = 5;
						PlayGameSound(GameSounds.Rescue.Sound, MaxSampleVolume);
						break;
					}
				}
			}
		}
	}
}

/**
**  Check if a unit holds any resources
**
**  @param unit  Unit to check
**
**  @return      True if the unit holds resources, false if it doesn't
*/
bool UnitHoldsResources(const CUnit *unit)
{
	for (int i = 0; i < MaxCosts; ++i) {
		if (unit->ResourcesHeld[i] != 0) {
			return true;
		}
	}
	return false;
}

/*----------------------------------------------------------------------------
--  Unit headings
----------------------------------------------------------------------------*/

/**
**  Fast arc tangent function.
**
**  @param val  atan argument
**
**  @return     atan(val)
*/
static int myatan(int val)
{
	static int init;
	static unsigned char atan_table[2608];

	if (val >= 2608) {
		return 63;
	}
	if (!init) {
		for (; init < 2608; ++init) {
			atan_table[init] =
				(unsigned char)(atan((double)init / 64) * (64 * 4 / 6.2831853));
		}
	}

	return atan_table[val];
}

/**
**  Convert direction to heading.
**
**  @param delta_x  Delta X.
**  @param delta_y  Delta Y.
**
**  @return         Angle (0..255)
*/
int DirectionToHeading(int delta_x, int delta_y)
{
	//
	//  Check which quadrant.
	//
	if (delta_x > 0) {
		if (delta_y < 0) { // Quadrant 1?
			return myatan((delta_x * 64) / -delta_y);
		}
		// Quadrant 2?
		return myatan((delta_y * 64) / delta_x) + 64;
	}
	if (delta_y>0) { // Quadrant 3?
		return myatan((delta_x * -64) / delta_y) + 64 * 2;
	}
	if (delta_x) { // Quadrant 4.
		return myatan((delta_y * -64) / -delta_x) + 64 * 3;
	}
	return 0;
}

/**
**  Update sprite frame for new heading.
*/
void UnitUpdateHeading(CUnit *unit)
{
	int dir;
	int nextdir;
	int neg;

	if (unit->Frame < 0) {
		unit->Frame = -unit->Frame - 1;
		neg = 1;
	} else {
		neg = 0;
	}
	unit->Frame /= unit->Type->NumDirections / 2 + 1;
	unit->Frame *= unit->Type->NumDirections / 2 + 1;
	// Remove heading, keep animation frame

	nextdir = 256 / unit->Type->NumDirections;
	dir = ((unit->Direction + nextdir / 2) & 0xFF) / nextdir;
	if (dir <= LookingS / nextdir) { // north->east->south
		unit->Frame += dir;
	} else {
		unit->Frame += 256 / nextdir - dir;
		unit->Frame = -unit->Frame - 1;
	}
	if (neg && !unit->Frame && unit->Type->Building) {
		unit->Frame = -1;
	}
}

/**
**  Change unit heading/frame from delta direction x, y.
**
**  @param unit  Unit for new direction looking.
**  @param dx    X map tile delta direction.
**  @param dy    Y map tile delta direction.
*/
void UnitHeadingFromDeltaXY(CUnit *unit, int dx, int dy)
{
	unit->Direction = DirectionToHeading(dx, dy);
	UnitUpdateHeading(unit);
}

/*----------------------------------------------------------------------------
  -- Drop out units
  ----------------------------------------------------------------------------*/

/**
**  Place a unit on the map to the side of a unit.
**
**  @param unit       Unit to drop out.
**  @param heading    Direction in which the unit should appear.
**  @param addx       Tile width of unit it's dropping out of.
**  @param addy       Tile height of unit it's dropping out of.
*/
void DropOutOnSide(CUnit *unit, int heading, int addx, int addy)
{
	int x;
	int y;
	int i;

	if (unit->Container) {
		x = unit->Container->X;
		y = unit->Container->Y;
	} else {
		x = unit->X;
		y = unit->Y;
	}

	if (heading < LookingNE || heading > LookingNW) {
		x += addx - 1;
		--y;
		goto startn;
	}
	if (heading < LookingSE) {
		x += addx;
		y += addy - 1;
		goto starte;
	}
	if (heading < LookingSW) {
		y += addy;
		goto starts;
	}
	--x;
	goto startw;

	// FIXME: don't search outside of the map
	for (;;) {
startw:
		for (i = addy; i--; ++y) {
			if (UnitCanBeAt(unit, x, y)) {
				goto found;
			}
		}
		++addx;
starts:
		for (i = addx; i--; ++x) {
			if (UnitCanBeAt(unit, x, y)) {
				goto found;
			}
		}
		++addy;
starte:
		for (i = addy; i--; --y) {
			if (UnitCanBeAt(unit, x, y)) {
				goto found;
			}
		}
		++addx;
startn:
		for (i = addx; i--; --x) {
			if (UnitCanBeAt(unit, x, y)) {
				goto found;
			}
		}
		++addy;
	}

found:
	unit->Place(x, y);
}

/**
**  Place a unit on the map nearest to x, y.
**
**  @param unit    Unit to drop out.
**  @param gx      Goal X map tile position.
**  @param gy      Goal Y map tile position.
**  @param addx    Tile width of unit it's dropping out of.
**  @param addy    Tile height of unit it's dropping out of.
*/
void DropOutNearest(CUnit *unit, int gx, int gy, int addx, int addy)
{
	int x;
	int y;
	int i;
	int bestx;
	int besty;
	int bestd;
	int n;

	Assert(unit->Removed);

	x = y = -1;
	if (unit->Container) {
		x = unit->Container->X;
		y = unit->Container->Y;
	} else {
		x = unit->X;
		y = unit->Y;
	}

	Assert(x != -1 && y != -1);

	bestd = 99999;
	bestx = besty = 0;

	// FIXME: if we reach the map borders we can go fast up, left, ...
	--x;
	for (;;) {
		for (i = addy; i--; ++y) { // go down
			if (UnitCanBeAt(unit, x, y)) {
				n = MapDistance(gx, gy, x, y);
				if (n < bestd) {
					bestd = n;
					bestx = x;
					besty = y;
				}
			}
		}
		++addx;
		for (i = addx; i--; ++x) { // go right
			if (UnitCanBeAt(unit, x, y)) {
				n = MapDistance(gx, gy, x, y);
				if (n < bestd) {
					bestd = n;
					bestx = x;
					besty = y;
				}
			}
		}
		++addy;
		for (i = addy; i--; --y) { // go up
			if (UnitCanBeAt(unit, x, y)) {
				n = MapDistance(gx, gy, x, y);
				if (n < bestd) {
					bestd = n;
					bestx = x;
					besty = y;
				}
			}
		}
		++addx;
		for (i = addx; i--; --x) { // go left
			if (UnitCanBeAt(unit, x, y)) {
				n = MapDistance(gx, gy, x, y);
				if (n < bestd) {
					bestd = n;
					bestx = x;
					besty = y;
				}
			}
		}
		if (bestd != 99999) {
			unit->Place(bestx, besty);
			return;
		}
		++addy;
	}
}

/**
**  Drop out all units inside unit.
**
**  @param source  All units inside source are dropped out.
*/
void DropOutAll(const CUnit *source)
{
	CUnit *unit;
	int i;

	unit = source->UnitInside;
	for (i = source->InsideCount; i; --i, unit = unit->NextContained) {
		DropOutOnSide(unit, LookingW,
			source->Type->TileWidth, source->Type->TileHeight);
		Assert(!unit->Orders[0]->Goal);
		unit->Orders[0]->Action = UnitActionStill;
		unit->SubAction = 0;
	}
	DebugPrint("Drop out %d of %d\n" _C_ i _C_ source->Data.Resource.Active);
}

/*----------------------------------------------------------------------------
  --  Building units
  ----------------------------------------------------------------------------*/

/**
**  Find the building restriction that gives me this unit built on top
**  Allows you to define how the restriction is effecting the build
**
**  @param unit    the unit that is "OnTop"
**  @param parent  the parent unit if known. (guess otherwise)
**
**  @return        the BuildingRestrictionDetails
*/
CBuildRestrictionOnTop *OnTopDetails(const CUnit *unit, const CUnitType *parent)
{
	CBuildRestrictionAnd *andb;
	CBuildRestrictionOnTop *ontopb;

	for (std::vector<CBuildRestriction *>::iterator i = unit->Type->BuildingRules.begin();
			i != unit->Type->BuildingRules.end(); ++i) {
		if ((ontopb = dynamic_cast<CBuildRestrictionOnTop *>(*i))) {
			if (!parent) {
				// Guess this is right
				return ontopb;
			}
			if (parent == ontopb->Parent) {
				return ontopb;
			}
		} else if ((andb = dynamic_cast<CBuildRestrictionAnd *>(*i))) {
			for (std::vector<CBuildRestriction *>::iterator i = andb->_or_list.begin();
					i != andb->_or_list.end(); ++i) {
				if ((ontopb = dynamic_cast<CBuildRestrictionOnTop *>(*i))) {
					if (!parent) {
						// Guess this is right
						return ontopb;
					}
					if (parent == ontopb->Parent) {
						return ontopb;
					}
				}
			}
		}
	}
	return NULL;
}

/**
**  Check And Restriction
*/
bool CBuildRestrictionAnd::Check(const CUnitType *type, int x, int y, CUnit *&ontoptarget) const
{
	for (std::vector<CBuildRestriction*>::const_iterator i = _or_list.begin();
		i != _or_list.end(); ++i) {
		if (!(*i)->Check(type, x, y, ontoptarget)) {
			return false;
		}
	}
	return true;
}

/**
**  Check Distance Restriction
*/
bool CBuildRestrictionDistance::Check(const CUnitType *type, int x, int y, CUnit *&ontoptarget) const
{
	CUnit *table[UnitMax];
	int n;
	int i;
	int x1 = 0;
	int x2 = 0;
	int y1 = 0;
	int y2 = 0;
	int distance = 0;

	if (this->DistanceType == LessThanEqual ||
			this->DistanceType == GreaterThan ||
			this->DistanceType == Equal ||
			this->DistanceType == NotEqual) {
		x1 = std::max(x - this->Distance, 0);
		y1 = std::max(y - this->Distance, 0);
		x2 = std::min(x + type->TileWidth + this->Distance, Map.Info.MapWidth);
		y2 = std::min(y + type->TileHeight + this->Distance, Map.Info.MapHeight);
		distance = this->Distance;
	} else if (this->DistanceType == LessThan ||
			this->DistanceType == GreaterThanEqual) {
		x1 = std::max(x - this->Distance - 1, 0);
		y1 = std::max(y - this->Distance - 1, 0);
		x2 = std::min(x + type->TileWidth + this->Distance + 1, Map.Info.MapWidth);
		y2 = std::min(y + type->TileHeight + this->Distance + 1, Map.Info.MapHeight);
		distance = this->Distance - 1;
	}
	n = UnitCache.Select(x1, y1, x2, y2, table, UnitMax);

	switch (this->DistanceType) {
		case GreaterThan :
		case GreaterThanEqual :
			for (i = 0; i < n; ++i) {
				if (this->RestrictType == table[i]->Type &&
					MapDistanceBetweenTypes(type, x, y, table[i]->Type, table[i]->X, table[i]->Y) <= distance) {
					return false;
				}
			}
			return true;
		case LessThan :
		case LessThanEqual :
			for (i = 0; i < n; ++i) {
				if (this->RestrictType == table[i]->Type &&
					MapDistanceBetweenTypes(type, x, y, table[i]->Type, table[i]->X, table[i]->Y) <= distance) {
					return true;
				}
			}
			return false;
		case Equal :
			for (i = 0; i < n; ++i) {
				if (this->RestrictType == table[i]->Type &&
					MapDistanceBetweenTypes(type, x, y, table[i]->Type, table[i]->X, table[i]->Y) == distance) {
					return true;
				}
			}
			return false;
		case NotEqual :
			for (i = 0; i < n; ++i) {
				if (this->RestrictType == table[i]->Type &&
					MapDistanceBetweenTypes(type, x, y, table[i]->Type, table[i]->X, table[i]->Y) == distance) {
					return false;
				}
			}
			return true;
	}
	return false;
}

/**
**  Check AddOn Restriction
*/
bool CBuildRestrictionAddOn::Check(const CUnitType *type, int x, int y, CUnit *&ontoptarget) const
{
	CUnit *table[UnitMax];
	int n;
	int i;
	int x1;
	int y1;

	x1 = x - this->OffsetX < 0 ? -1 : x - this->OffsetX;
	x1 = x1 >= Map.Info.MapWidth ? -1 : x1;
	y1 = y - this->OffsetY < 0 ? -1 : y - this->OffsetY;
	y1 = y1 >= Map.Info.MapHeight ? -1 : y1;
	if (!(x1 == -1 || y1 == -1)) {
		n = UnitCache.Select(x1, y1, table, UnitMax);
		for (i = 0; i < n; ++i) {
			if (table[i]->Type == this->Parent &&
					table[i]->X == x1 && table[i]->Y == y1) {
				return true;
			}
		}
	}
	return false;
}

/**
**  Check OnTop Restriction
*/
bool CBuildRestrictionOnTop::Check(const CUnitType *type, int x, int y, CUnit *&ontoptarget) const
{
	CUnit *table[UnitMax];
	int n;
	int i;

	ontoptarget = NULL;
	n = UnitCache.Select(x, y, table, UnitMax);
	for (i = 0; i < n; ++i) {
		if (table[i]->X == x && table[i]->Y == y && !table[i]->Destroyed &&
				table[i]->Orders[0]->Action != UnitActionDie) {
			if (table[i]->Type == this->Parent &&
					table[i]->Orders[0]->Action != UnitActionBuilt) {
				// Available to build on
				ontoptarget = table[i];
			} else {
				// Something else is built on this already
				ontoptarget = NULL;
				return false;
			}
		}
	}
	return ontoptarget != NULL;
}


/**
**  Can build unit here.
**
**  @param unit  Unit doing the building
**  @param type  unit-type to be checked.
**  @param x     Map X position.
**  @param y     Map Y position.
**
**  @return      OnTop, parent unit, builder on true or 1 if unit==NULL, NULL false.
*/
CUnit *CanBuildHere(const CUnit *unit, const CUnitType *type, int x, int y)
{
	CUnit *ontoptarget;

	//
	//  Can't build outside the map
	//
	if (x + type->TileWidth > Map.Info.MapWidth) {
		return NULL;
	}
	if (y + type->TileHeight > Map.Info.MapHeight) {
		return NULL;
	}

	// Must be checked before oil!
	if (type->ShoreBuilding) {
		bool success = false;

		// Need at least one coast tile
		for (int h = type->TileHeight; h--;) {
			for (int w = type->TileWidth; w--;) {
				if (Map.Field(x + w, y + h)->Flags & MapFieldCoastAllowed) {
					h = w = 0;
					success = true;
					break;
				}
			}
		}
		if (!success) {
			return NULL;
		}
	}

	if (type->BuildingRules.empty()) {
		return (unit == NULL) ? (CUnit *)1 : const_cast<CUnit *>(unit);
	}

	ontoptarget = NULL;
	for (std::vector<CBuildRestriction *>::const_iterator ib = type->BuildingRules.begin();
			ib != type->BuildingRules.end(); ++ib) {
		// All checks processed, did we really have success
		if ((*ib)->Check(type, x, y, ontoptarget)) {
			// We passed a full ruleset return
			if (unit == NULL) {
				return ontoptarget ? ontoptarget : (CUnit *)1;
			} else {
				return ontoptarget ? ontoptarget : (CUnit *)unit;
			}
		}
	}
	return NULL;
}

/**
**  Can build on this point.
**
**  @param x     X tile map position.
**  @param y     Y tile map position.
**  @param  mask terrain mask
**
**  @return 1 if we can build on this point.
*/
bool CanBuildOn(int x, int y, int mask)
{
	if (x < 0 || y < 0 || x >= Map.Info.MapWidth || y >= Map.Info.MapHeight) {
		return false;
	}
	return (Map.Field(x, y)->Flags & mask) ? false : true;
}

/**
**  Can build unit-type at this point.
**
**  @param unit  Worker that want to build the building or NULL.
**  @param type  Building unit-type.
**  @param x     X tile map position.
**  @param y     Y tile map position.
**  @param real  Really build, or just placement
**
**  @return      OnTop, parent unit, builder on true, NULL false.
*/
CUnit *CanBuildUnitType(const CUnit *unit, const CUnitType *type, int x, int y, int real)
{
	int j;
	int testmask;
	CPlayer *player;
	CUnit *ontop;

	// Terrain Flags don't matter if building on top of a unit.
	ontop = CanBuildHere(unit, type, x, y);
	if (ontop == NULL) {
		return NULL;
	}
	if (ontop != (CUnit *)1 && ontop != unit) {
		return ontop;
	}

	//
	//  Remove unit that is building!
	//
	j = 0;
	if (unit) {
		UnmarkUnitFieldFlags(unit);
	}

	player = NULL;

	if (unit && unit->Player->Type == PlayerPerson) {
		player = unit->Player;
	}

	for (int h = type->TileHeight; h--;) {
		for (int w = type->TileWidth; w--;) {
			if (player && !real) {
				testmask = MapFogFilterFlags(player, x + w, y + h, type->MovementMask);
			} else {
				testmask = type->MovementMask;
			}
			if (!CanBuildOn(x + w, y + h, testmask)) {
				if (unit) {
					MarkUnitFieldFlags(unit);
				}
				return NULL;
			}
			if (player && !Map.IsFieldExplored(player, x + w, y + h)) {
				if (unit) {
					MarkUnitFieldFlags(unit);
				}
				return NULL;
			}
		}
	}
	if (unit) {
		MarkUnitFieldFlags(unit);
	}

	//
	// We can build here: check distance to gold mine/oil patch!
	//
	return ontop;
}

/*----------------------------------------------------------------------------
  -- Finding units
  ----------------------------------------------------------------------------*/

/**
**  Find Resource.
**
**  @param unit        The unit that wants to find a resource.
**  @param x           Closest to x
**  @param y           Closest to y
**  @param range       Maximum distance to the resource.
**  @param resource    The resource id.
**
**  @note This will return an usable resource building that
**  belongs to "player" or is neutral.
**
**  @return            NoUnitP or resource unit
*/
CUnit *UnitFindResource(const CUnit *unit, int x, int y, int range, int resource)
{
	static const int xoffset[] = {  0,-1,+1, 0, -1,+1,-1,+1 };
	static const int yoffset[] = { -1, 0, 0,+1, -1,-1,+1,+1 };
	struct p {
		unsigned short X;
		unsigned short Y;
	} *points;
	int size;
	int rx;
	int ry;
	int mask;
	int wp;
	int rp;
	int ep;
	int i;
	int w;
	unsigned char *m;
	unsigned char *matrix;
	CUnit *res;
	CUnit *bestres;
	int destx;
	int desty;
	int bestd;
	int cdist;

	destx = x;
	desty = y;
	size = std::min(Map.Info.MapWidth * Map.Info.MapHeight / 4, range * range * 5);
	points = new p[size];

	bestd = 99999;
	// Make movement matrix. FIXME: can create smaller matrix.
	matrix = CreateMatrix();
	w = Map.Info.MapWidth + 2;
	matrix += w + w + 2;
	// Unit movement mask
	mask = unit->Type->MovementMask;
	// Ignore all units along the way. Might seem weird, but otherwise
	// peasants would lock at a resource with a lot of workers.
	mask &= ~(MapFieldLandUnit | MapFieldSeaUnit | MapFieldAirUnit);
	points[0].X = x;
	points[0].Y = y;
	rp = 0;
	matrix[x + y * w] = 1; // mark start point
	ep = wp = 1; // start with one point
	cdist = 0; // current distance is 0
	bestres = NoUnitP;

	//
	// Pop a point from stack, push all neighbors which could be entered.
	//
	for (;;) {
		while (rp != ep) {
			rx = points[rp].X;
			ry = points[rp].Y;
			for (i = 0; i < 8; ++i) { // mark all neighbors
				x = rx + xoffset[i];
				y = ry + yoffset[i];
				m = matrix + x + y * w;
				if (*m) { // already checked
					continue;
				}

				if (!Map.IsFieldExplored(unit->Player, x, y)) { // Unknown.
					continue;
				}

				//
				// Look if there is a resource
				//
				if ((res = ResourceOnMap(x, y, resource)) &&
						res->Type->CanHarvestFrom &&
						(res->Player->Index == PlayerMax - 1 ||
							res->Player == unit->Player ||
							(unit->IsAllied(res) && res->IsAllied(unit)))) {
					delete[] points;
					return res;
				}

				if (CanMoveToMask(x, y, mask)) { // reachable
					*m = 1;
					points[wp].X = x; // push the point
					points[wp].Y = y;
					if (++wp >= size) { // round about
						wp = 0;
					}
					if (wp == ep) {
						//  We are out of points, give up!
						break;
					}
				} else { // unreachable
					*m = 99;
				}
			}
			if (++rp >= size) { // round about
				rp = 0;
			}
		}
		// Take best of this frame, if any.
		if (bestd != 99999) {
			delete[] points;
			return bestres;
		}
		++cdist;
		if (rp == wp || cdist >= range) { // unreachable, no more points available
			break;
		}
		// Continue with next set.
		ep = wp;
	}
	delete[] points;
	return NoUnitP;
}

/**
**  Find the next idle worker
**
**  @param player    Player's units to search through
**  @param last      Previous idle worker selected
**
**  @return NoUnitP or next idle worker
*/
CUnit *FindIdleWorker(const CPlayer *player, const CUnit *last)
{
	CUnit *unit;
	CUnit *FirstUnitFound;
	int nunits;
	int i;
	int SelectNextUnit;

	FirstUnitFound = NoUnitP;
	if (last == NoUnitP) {
		SelectNextUnit = 1;
	} else {
		SelectNextUnit = 0;
	}

	nunits = player->TotalNumUnits;

	for (i = 0; i < nunits; ++i) {
		unit = player->Units[i];
		if (unit->Type->Harvester && !unit->Removed) {
			if (unit->Orders[0]->Action == UnitActionStill) {
				if (SelectNextUnit && !IsOnlySelected(unit)) {
					return unit;
				}
				if (FirstUnitFound == NULL) {
					FirstUnitFound = unit;
				}
			}
		}
		if (unit == last) {
			SelectNextUnit = 1;
		}
	}

	if (FirstUnitFound != NoUnitP && !IsOnlySelected(FirstUnitFound)) {
		return FirstUnitFound;
	}

	return NoUnitP;
}

/*----------------------------------------------------------------------------
  -- Select units
  ----------------------------------------------------------------------------*/

/**
**  Select unit on screen. (x, y are in pixels relative to map 0,0).
**
**  @param x  X pixel position.
**  @param y  Y pixel position.
**
**  @return   Unit on x, y position.
*/
CUnit *UnitOnScreen(int x, int y)
{
	CUnit *bestUnit = NULL;

	for (int i = 0; i < NumUnits; ++i) {
		CUnit *unit = Units[i];
		CUnitType *type = unit->Type;
		int gx, gy;

		// Must be visible
		if (!unit->IsVisibleAsGoal(ThisPlayer) && !ReplayRevealMap) {
			continue;
		}

		// Mouse is in the unit's box
		gx = unit->X * TileSizeX + unit->IX;
		if (x + (type->BoxWidth - type->TileWidth * TileSizeX) / 2 < gx) {
			continue;
		}
		if (x > gx + (type->TileWidth * TileSizeX + type->BoxWidth) / 2) {
			continue;
		}

		gy = unit->Y * TileSizeY + unit->IY;
		if (y + (type->BoxHeight - type->TileHeight * TileSizeY) / 2 < gy) {
			continue;
		}
		if (y > gy + (type->TileHeight * TileSizeY + type->BoxHeight) / 2) {
			continue;
		}

		// This could be taken.
		if (!bestUnit || unit->Type->DrawLevel > bestUnit->Type->DrawLevel) {
			bestUnit = unit;
		}
	}

	return bestUnit;
}

/**
**  Check if a unit should be removed from UnitsConsumingResources
*/
void UnitRemoveConsumingResources(CUnit *unit)
{
	if ((unit->Orders[0]->Action == UnitActionBuild && !unit->Type->BuilderOutside && unit->SubAction == 40) ||
			(unit->Orders[0]->Action == UnitActionRepair && unit->SubAction == 20)) {
		unit->Player->RemoveFromUnitsConsumingResources(unit);
	} else if (unit->Orders[0]->Action == UnitActionResource && unit->SubAction >= 55) {
		for (int u = 0; u < MaxCosts; ++u) {
			unit->Player->ProductionRate[u] -= unit->Data.Harvest.CurrentProduction[u];
		}
	} else if (unit->Orders[0]->Action == UnitActionTrain && unit->SubAction != 0) {
		unit->Player->RemoveFromUnitsConsumingResources(unit);
	}
}

/**
**  Let a unit die.
**
**  @param unit    Unit to be destroyed.
*/
void LetUnitDie(CUnit *unit)
{
	CUnitType *type;

	unit->Moving = 0;
	unit->TTL = 0;
	unit->Anim.Unbreakable = 0;

	type = unit->Type;

	// removed units,  just remove.
	if (unit->Removed) {
		DebugPrint("Killing a removed unit?\n");
		UnitLost(unit);
		UnitClearOrders(unit);
		unit->Release();
		return;
	}

	PlayUnitSound(unit, VoiceDying);

	//
	// Catapults,... explodes.
	//
	if (type->ExplodeWhenKilled) {
		MakeMissile(type->Explosion.Missile,
			unit->X * TileSizeX + type->TileWidth * TileSizeX / 2,
			unit->Y * TileSizeY + type->TileHeight * TileSizeY / 2,
			0, 0);
	}

	// Handle Teleporter Destination Removal
	if (type->Teleporter && unit->Goal) {
		unit->Goal->Remove(NULL);
		UnitLost(unit->Goal);
		UnitClearOrders(unit->Goal);
		unit->Goal->Release();
		unit->Goal = NULL;
	}

	UnitRemoveConsumingResources(unit);
	if (unit->Orders[0]->Action != UnitActionBuilt) {
		for (int u = 0; u < MaxCosts; ++u) {
			unit->Player->ProductionRate[u] -= unit->Type->ProductionRate[u] * unit->ProductionEfficiency / 100;
			unit->Player->StorageCapacity[u] -= unit->Type->StorageCapacity[u];
			if (unit->Player->StoredResources[u] > unit->Player->StorageCapacity[u]) {
				unit->Player->StoredResources[u] = unit->Player->StorageCapacity[u];
			}
		}
	}

	// During resource build, the worker holds the resource amount,
	// but if canceling building the platform, the worker is already
	// outside.
	if (type->CanHarvestFrom &&
			unit->Orders[0]->Action == UnitActionBuilt &&
			unit->Data.Built.Worker) {
		// Restore value for oil-patch
		memcpy(unit->ResourcesHeld, unit->Data.Built.Worker->ResourcesHeld, sizeof(unit->ResourcesHeld));
	}

	// Transporters lose their units and building their workers
	if (unit->UnitInside) {
		// FIXME: destroy or unload : do a flag.
		DestroyAllInside(unit);
	}
	unit->Remove(NULL);
	UnitLost(unit);
	UnitClearOrders(unit);

	//
	// Unit has death animation.
	//

	// Not good: UnitUpdateHeading(unit);
	unit->SubAction = 0;
	unit->State = 0;
	unit->Orders[0]->Action = UnitActionDie;
	if (type->CorpseType) {
		unit->IX = (type->CorpseType->Width - type->CorpseType->Sprite->Width) / 2;
		unit->IY = (type->CorpseType->Height - type->CorpseType->Sprite->Height) / 2;

		unit->CurrentSightRange = type->CorpseType->Stats[unit->Player->Index].Variables[SIGHTRANGE_INDEX].Max;
	} else {
		unit->CurrentSightRange = 0;
	}

	// If we have a corpse, or a death animation, we are put back on the map
	// This enables us to be tracked.  Possibly for spells (eg raise dead)
	if (type->CorpseType || (type->Animations && type->Animations->Death)) {
		unit->Removed = 0;
		UnitCache.Insert(unit);
	}
	MapMarkUnitSight(unit);
}

/**
**  Destroy all units inside unit.
**
**  @param source  container.
*/
void DestroyAllInside(CUnit *source)
{
	CUnit *unit;
	int i;

	// No Corpses, we are inside something, and we can't be seen
	unit = source->UnitInside;
	for (i = source->InsideCount; i; --i, unit = unit->NextContained) {
		// Transporter inside a transporter?
		if (unit->UnitInside) {
			DestroyAllInside(unit);
		}
		UnitLost(unit);
		UnitClearOrders(unit);
		unit->Release();
	}
}


/*----------------------------------------------------------------------------
  -- Unit AI
  ----------------------------------------------------------------------------*/

/**
**  Unit is hit by missile or other damage.
**
**  @param attacker    Unit that attacks.
**  @param target      Unit that is hit.
**  @param damage      How many damage to take.
*/
void HitUnit(CUnit *attacker, CUnit *target, int damage)
{
	CUnitType *type;
	CUnit *goal;
	unsigned long lastattack;

	// Can now happen by splash damage
	// Multiple places send x/y as damage, which may be zero
	if (!damage) {
		return;
	}

	Assert(damage != 0 && target->Orders[0]->Action != UnitActionDie && !target->Type->Vanishes);

	if (target->Type->Indestructible) {
		return;
	}

	if (target->Removed) {
		DebugPrint("Removed target hit\n");
		return;
	}

	if (GodMode) {
		if (attacker && attacker->Player == ThisPlayer) {
			damage = target->Variable[HP_INDEX].Value;
		}
		if (target->Player == ThisPlayer) {
			damage = 0;
		}
	}

	type = target->Type;
	lastattack = target->Attacked;
	target->Attacked = GameCycle ? GameCycle : 1;


	if (!lastattack || lastattack + 2 * CYCLES_PER_SECOND < GameCycle) {
		// NOTE: perhaps this should also be moved into the notify?
		if (target->Player == ThisPlayer) {
			// FIXME: Problem with load+save.

			//
			// One help cry each 2 second is enough
			// If on same area ignore it for 2 minutes.
			//
			if (HelpMeLastCycle < GameCycle) {
				if (!HelpMeLastCycle ||
						HelpMeLastCycle + CYCLES_PER_SECOND * 120 < GameCycle ||
						target->X < HelpMeLastX - 14 ||
						target->X > HelpMeLastX + 14 ||
						target->Y < HelpMeLastY - 14 ||
						target->Y > HelpMeLastY + 14) {
					HelpMeLastCycle = GameCycle + CYCLES_PER_SECOND * 2;
					HelpMeLastX = target->X;
					HelpMeLastY = target->Y;
					PlayUnitSound(target, VoiceHelpMe);
				}
			}
		}
		target->Player->Notify(NotifyRed, target->X, target->Y,
			_("%s attacked"), target->Type->Name.c_str());
		if (target->Player->AiEnabled) {
			AiHelpMe(attacker, target);
		}
	}

	if (target->Variable[HP_INDEX].Value <= damage) { // unit is killed or destroyed
		//  increase scores of the attacker, but not if attacking it's own units.
		//  prevents cheating by killing your own units.
		if (attacker && target->IsEnemy(attacker)) {
			attacker->Player->Score += target->Type->Points;
			if (type->Building) {
				attacker->Player->TotalRazings++;
			} else {
				attacker->Player->TotalKills++;
			}
			attacker->Variable[KILL_INDEX].Value++;
			attacker->Variable[KILL_INDEX].Max++;
			attacker->Variable[KILL_INDEX].Enable = 1;
		}
		LetUnitDie(target);
		return;
	}
	target->Variable[HP_INDEX].Value -= damage;

	// FIXME: this is dumb. I made repairers capture. crap.
	// david: capture enemy buildings
	// Only worker types can capture.
	// Still possible to destroy building if not careful (too many attackers)
	if (EnableBuildingCapture && attacker &&
			type->Building && target->Variable[HP_INDEX].Value <= damage * 3 &&
			attacker->IsEnemy(target) &&
			attacker->Type->RepairRange) {
		target->ChangeOwner(attacker->Player);
		CommandStopUnit(attacker); // Attacker shouldn't continue attack!
	}

	if ((target->IsVisibleOnMap(ThisPlayer) || ReplayRevealMap) && !DamageMissile.empty()) {
		MakeLocalMissile(MissileTypeByIdent(DamageMissile),
				target->X * TileSizeX + target->Type->TileWidth * TileSizeX / 2,
				target->Y * TileSizeY + target->Type->TileHeight * TileSizeY / 2,
				target->X * TileSizeX + target->Type->TileWidth * TileSizeX / 2 + 3,
				target->Y * TileSizeY + target->Type->TileHeight * TileSizeY / 2 -
					MissileTypeByIdent(DamageMissile)->Range)->Damage = -damage;
	}

#if 0
	// FIXME: want to show hits.
	if (type->Organic) {
		MakeMissile(MissileBlood,
			target->X * TileSizeX + TileSizeX / 2,
			target->Y * TileSizeY + TileSizeY / 2, 0, 0);
	}
	if (type->Building) {
		MakeMissile(MissileSmallFire,
			target->X * TileSizeX + (type->TileWidth * TileSizeX) / 2,
			target->Y * TileSizeY + (type->TileHeight * TileSizeY) / 2, 0, 0);
	}
#endif

	if (type->Building && !target->Burning) {
		int f;
		Missile *missile;
		MissileType *fire;

		f = (100 * target->Variable[HP_INDEX].Value) / target->Variable[HP_INDEX].Max;
		fire = MissileBurningBuilding(f);
		if (fire) {
			missile = MakeMissile(fire,
				target->X * TileSizeX + (type->TileWidth * TileSizeX) / 2,
				target->Y * TileSizeY + (type->TileHeight * TileSizeY) / 2 - TileSizeY,
				0, 0);
			missile->SourceUnit = target;
			target->Burning = 1;
			target->RefsIncrease();
		}
	}

	//
	// Unit is working?
	//
	if (target->Orders[0]->Action != UnitActionStill) {
		return;
	}

	//
	// Attack units in range (which or the attacker?)
	//
	if (attacker && !type->Coward) {
		if (type->CanAttack) {
			if (CanTarget(target->Type, attacker->Type)) {
				// Attack unit that is attacking
				goal = attacker;
			} else {
				// Check for any other units in range
				goal = AttackUnitsInReactRange(target);
			}
			if (goal) {
				if (target->SavedOrder.Action == UnitActionStill) {
					// FIXME: should rewrite command handling
					CommandAttack(target, target->X, target->Y, NoUnitP,
						FlushCommands);
					target->SavedOrder = *target->Orders[1];
				}
				CommandAttack(target, goal->X, goal->Y, NoUnitP, FlushCommands);
				return;
			}
		}
	}

	//
	// Can't attack run away.
	//
	if (CanMove(target)) {
		int x;
		int y;
		int d;

		x = target->X - attacker->X;
		y = target->Y - attacker->Y;
		d = isqrt(x * x + y * y);
		if (!d) {
			d = 1;
		}
		x = target->X + (x * 5) / d + (SyncRand() & 3);
		if (x < 0) {
			x = 0;
		} else if (x >= Map.Info.MapWidth) {
			x = Map.Info.MapWidth - 1;
		}
		y = target->Y + (y * 5) / d + (SyncRand() & 3);
		if (y < 0) {
			y = 0;
		} else if (y >= Map.Info.MapHeight) {
			y = Map.Info.MapHeight - 1;
		}
		CommandStopUnit(target);
		CommandMove(target, x, y, 0);
	}
}

/*----------------------------------------------------------------------------
--  Conflicts
----------------------------------------------------------------------------*/

/**
**  Returns the map distance between two points.
**
**  @param x1    X map tile position.
**  @param y1    Y map tile position.
**  @param x2    X map tile position.
**  @param y2    Y map tile position.
**
**  @return      The distance between in tiles.
*/
int MapDistance(int x1, int y1, int x2, int y2)
{
	return isqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

/**
**  Returns the map distance between two points with unit type.
**
**  @param x1      X map tile position.
**  @param y1      Y map tile position.
**  @param type    Unit type to take into account.
**  @param x2      X map tile position.
**  @param y2      Y map tile position.
**
**  @return        The distance between in tiles.
*/
int MapDistanceToType(int x1, int y1, const CUnitType *type, int x2, int y2)
{
	int dx;
	int dy;

	if (x1 <= x2) {
		dx = x2 - x1;
	} else {
		dx = x1 - x2 - type->TileWidth + 1;
		if (dx < 0) {
			dx = 0;
		}
	}

	if (y1 <= y2) {
		dy = y2 - y1;
	} else {
		dy = y1 - y2 - type->TileHeight + 1;
		if (dy < 0) {
			dy = 0;
		}
	}

	return isqrt(dy * dy + dx * dx);
}

/**
**  Returns the map distance to unit.
**
**  @param x       X map tile position.
**  @param y       Y map tile position.
**  @param dest    Distance to this unit.
**
**  @return        The distance between in tiles.
*/
int MapDistanceToUnit(int x, int y, const CUnit *dest)
{
	return MapDistanceToType(x, y, dest->Type, dest->X, dest->Y);
}

/**
**  Returns the map distance between two units.
**
**  @param src    Distance from this unit.
**  @param dst    Distance  to  this unit.
**
**  @return       The distance between in tiles.
*/
int MapDistanceBetweenUnits(const CUnit *src, const CUnit *dst)
{
	return MapDistanceBetweenTypes(src->Type, src->X, src->Y,
		dst->Type, dst->X, dst->Y);
}

/**
**  Returns the map distance between two points with unit type.
**
**  @param src     src unittype
**  @param x1      X map tile position of src (upperleft).
**  @param y1      Y map tile position of src.
**  @param dst     Unit type to take into account.
**  @param x2      X map tile position of dst.
**  @param y2      Y map tile position of dst.
**
**  @return        The distance between the types.
*/
int MapDistanceBetweenTypes(const CUnitType *src, int x1, int y1, const CUnitType *dst, int x2, int y2)
{
	int dx;
	int dy;

	if (x1 + src->TileWidth <= x2) {
		dx = x2 - x1 - src->TileWidth + 1;
		if (dx < 0) {
			dx = 0;
		}
	} else {
		dx = x1 - x2 - dst->TileWidth + 1;
		if (dx < 0) {
			dx = 0;
		}
	}

	if (y1 + src->TileHeight <= y2) {
		dy = y2 - y1 - src->TileHeight + 1;
	} else {
		dy = y1 - y2 - dst->TileHeight + 1;
		if (dy < 0) {
			dy = 0;
		}
	}

	return isqrt(dy * dy + dx * dx);
}

/**
**  Compute the distance from the view point to a given point.
**
**  @param x    X map tile position.
**  @param y    Y map tile position.
**
**  @todo FIXME: is it the correct place to put this function in?
*/
int ViewPointDistance(int x, int y)
{
	const CViewport *vp;

	// first compute the view point coordinate
	vp = UI.SelectedViewport;

	// then use MapDistance
	return MapDistance(vp->MapX + vp->MapWidth / 2,
		vp->MapY + vp->MapHeight / 2, x, y);
}

/**
**  Compute the distance from the view point to a given unit.
**
**  @param dest    Distance to this unit.
**
**  @todo FIXME: is it the correct place to put this function in?
*/
int ViewPointDistanceToUnit(const CUnit *dest)
{
	const CViewport *vp;

	// first compute the view point coordinate
	vp = UI.SelectedViewport;
	// then use MapDistanceToUnit
	return MapDistanceToUnit(vp->MapX + vp->MapWidth / 2,
		vp->MapY + vp->MapHeight / 2, dest);
}

/**
**  Can the source unit attack the destination unit.
**
**  @param source    Unit type pointer of the attacker.
**  @param dest      Unit type pointer of the target.
**
**  @return 0 if attacker can't target the unit, else a positive number.
*/
int CanTarget(const CUnitType *source, const CUnitType *dest)
{
	int i;

	for (i = 0; i < UnitTypeVar.NumberBoolFlag; i++) {
		if (source->CanTargetFlag[i] != CONDITION_TRUE) {
			if ((source->CanTargetFlag[i] == CONDITION_ONLY) ^ (dest->BoolFlag[i])) {
				return 0;
			}
		}
	}
	if (dest->UnitType == UnitTypeLand) {
		if (dest->ShoreBuilding) {
			return source->CanTarget & (CanTargetLand | CanTargetSea);
		}
		return source->CanTarget & CanTargetLand;
	}
	if (dest->UnitType == UnitTypeFly) {
		return source->CanTarget & CanTargetAir;
	}
	if (dest->UnitType == UnitTypeNaval) {
		return source->CanTarget & CanTargetSea;
	}
	return 0;
}

/**
**  Can the transporter transport the other unit.
**
**  @param transporter  Unit which is the transporter.
**  @param unit         Unit which wants to go in the transporter.
**
**  @return 1 if transporter can transport unit, 0 else.
*/
int CanTransport(const CUnit *transporter, const CUnit *unit)
{
	int i;

	if (!transporter->Type->CanTransport) {
		return 0;
	}
	if (transporter->Orders[0]->Action == UnitActionBuilt) { // Under construction
		return 0;
	}
	if (transporter == unit) { // Cannot transporter itself.
		return 0;
	}
	if (transporter->BoardCount >= transporter->Type->MaxOnBoard) { // full
		return 0;
	}
	// FIXME: remove UnitTypeLand requirement
	if (unit->Type->UnitType != UnitTypeLand) {
		return 0;
	}
	// Can transport only allied unit.
	// FIXME : should be parametrable.
	if (!transporter->IsTeamed(unit)) {
		return 0;
	}
	for (i = 0; i < UnitTypeVar.NumberBoolFlag; i++) {
		if (transporter->Type->CanTransport[i] != CONDITION_TRUE) {
			if ((transporter->Type->CanTransport[i] == CONDITION_ONLY) ^
					unit->Type->BoolFlag[i]) {
				return 0;
			}
		}
	}
	return 1;
}

/**
**  Check if the player is an enemy
**
**  @param x  Player to check
*/
bool CUnit::IsEnemy(const CPlayer *x) const
{
	return (this->Player->Enemy & (1 << x->Index)) != 0;
}

/**
**  Check if the unit is an enemy
**
**  @param x  Unit to check
*/
bool CUnit::IsEnemy(const CUnit *x) const
{
	return IsEnemy(x->Player);
}

/**
**  Check if the player is an ally
**
**  @param x  Player to check
*/
bool CUnit::IsAllied(const CPlayer *x) const
{
	return (this->Player->Allied & (1 << x->Index)) != 0;
}

/**
**  Check if the unit is an ally
**
**  @param x  Unit to check
*/
bool CUnit::IsAllied(const CUnit *x) const
{
	return IsAllied(x->Player);
}

/**
**  Check if unit shares vision with the player
**
**  @param x  Player to check
*/
bool CUnit::IsSharedVision(const CPlayer *x) const
{
	return (this->Player->SharedVision & (1 << x->Index)) != 0;
}

/**
**  Check if the unit shares vision with the unit
**
**  @param x  Unit to check
*/
bool CUnit::IsSharedVision(const CUnit *x) const
{
	return IsSharedVision(x->Player);
}

/**
**  Check if both players share vision
**
**  @param x  Player to check
*/
bool CUnit::IsBothSharedVision(const CPlayer *x) const
{
	return (this->Player->SharedVision & (1 << x->Index)) != 0 &&
		(x->SharedVision & (1 << this->Player->Index)) != 0;
}

/**
**  Check if both units share vision
**
**  @param x  Unit to check
*/
bool CUnit::IsBothSharedVision(const CUnit *x) const
{
	return IsBothSharedVision(x->Player);
}

/**
**  Check if the player is on the same team
**
**  @param x  Player to check
*/
bool CUnit::IsTeamed(const CPlayer *x) const
{
	return (this->Player->Team == x->Team);
}

/**
**  Check if the unit is on the same team
**
**  @param x  Unit to check
*/
bool CUnit::IsTeamed(const CUnit *x) const
{
	return this->IsTeamed(x->Player);
}

/**
**  Check if the unit is unusable (for attacking...)
**  @todo look if correct used (UnitActionBuilt is no problem if attacked)?
*/
bool CUnit::IsUnusable() const
{
	return this->Removed || this->Orders[0]->Action == UnitActionDie ||
		this->Orders[0]->Action == UnitActionBuilt || this->Destroyed;
}


/*----------------------------------------------------------------------------
--  Initialize/Cleanup
----------------------------------------------------------------------------*/

/**
**  Initialize unit module.
*/
void InitUnits(void)
{
	NumUnits = 0;
	UnitManager.Init();
}

/**
**  Clean up unit module.
*/
void CleanUnits(void)
{
	CUnit **table;

	//
	//  Free memory for all units in unit table.
	//
	for (table = Units; table < &Units[NumUnits]; ++table) {
		delete[] (*table)->AutoCastSpell;
		delete[] (*table)->Variable;
		for (std::vector<COrder *>::iterator order = (*table)->Orders.begin(); order != (*table)->Orders.end(); ++order) {
			delete *order;
		}
		(*table)->Orders.clear();
		delete *table;
		*table = NULL;
	}
	NumUnits = 0;

	UnitManager.Init();

	HelpMeLastCycle = 0;
}

//@}
