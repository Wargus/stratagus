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
/**@name unit.c - The units. */
//
//      (c) Copyright 1998-2004 by Lutz Sammer and Jimmy Salmon
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
  -- Includes
  ----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#include "stratagus.h"

#include "video.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
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
#include "construct.h"

/*----------------------------------------------------------------------------
  -- Variables
  ----------------------------------------------------------------------------*/

#ifndef LimitSearch
#define LimitSearch 1                     ///< Limit the search
#endif

Unit* UnitSlots[MAX_UNIT_SLOTS];          ///< All possible units
Unit** UnitSlotFree;                      ///< First free unit slot
Unit* ReleasedHead;                       ///< List of released units.
Unit* ReleasedTail;                       ///< List tail of released units.

Order* ReleasedOrderHead;                 ///< List of released Orders.
Order* ReleasedOrderTail;                 ///< List tail of released orders.

Unit* Units[MAX_UNIT_SLOTS];              ///< Array of used slots
int NumUnits;                             ///< Number of slots used

int XpDamage;                             ///< Hit point regeneration for all units
char EnableTrainingQueue;                 ///< Config: training queues enabled
char EnableBuildingCapture;               ///< Config: capture buildings enabled
char RevealAttacker;                      ///< Config: reveal attacker enabled

static unsigned long HelpMeLastCycle;     ///< Last cycle HelpMe sound played
static int HelpMeLastX;                   ///< Last X coordinate HelpMe sound played
static int HelpMeLastY;                   ///< Last Y coordinate HelpMe sound played

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

static void RemoveUnitFromContainer(Unit* unit);

/**
** Initial memory allocation for units.
*/
void InitUnitsMemory(void)
{
	Unit** slot;

	// Initialize the "list" of free unit slots

	slot = UnitSlots + MAX_UNIT_SLOTS;
	*--slot = NULL; // leave last slot free as no marker
	*--slot = NULL;
	do {
		slot[-1] = (void*)slot;
	} while (--slot > UnitSlots);
	UnitSlotFree = slot;

	ReleasedTail = ReleasedHead = 0; // list of unfreed units.
	NumUnits = 0;
}

#if 0
/**
** Free the memory for an unit slot. Update the global slot table.
** The memory should only be freed, if all references are dropped.
**
** @param unit Pointer to unit.
*/
void FreeUnitMemory(Unit* unit)
{
	Unit** slot;

	//
	// Remove from slot table
	//
	slot = UnitSlots + unit->Slot;
	Assert(*slot == unit);

	*slot = (void*)UnitSlotFree;
	free(unit);
}
#endif

/**
** Increase an unit's reference count.
**
** @param unit The unit
*/
void RefsIncrease(Unit* unit)
{
	RefsAssert(unit->Refs && !unit->Destroyed);
	if (!SaveGameLoading) {
		++unit->Refs;
	}
}

/**
** Decrease an unit's reference count.
**
** @param unit The unit
*/
void RefsDecrease(Unit* unit)
{
	RefsAssert(unit->Refs);
	if (!SaveGameLoading) {
		if (unit->Destroyed) {
			if (!--unit->Refs) {
				ReleaseUnit(unit);
			}
		} else {
			--unit->Refs;
			RefsAssert(unit->Refs);
		}
	}
}

/**
** Release an unit.
**
** The unit is only released, if all references are dropped.
**
** @param unit Pointer to unit.
*/
void ReleaseUnit(Unit* unit)
{
	Unit* temp;

	Assert(unit->Type); // already free.
	Assert(unit->OrderCount == 1);
	Assert(!unit->Orders[0].Goal);

	//
	// First release, remove from lists/tables.
	//
	if (!unit->Destroyed) {
		DebugPrint("First release %d\n" _C_ unit->Slot);

		//
		// Are more references remaining?
		//
		unit->Destroyed = 1; // mark as destroyed

		if (unit->Container) {
			MapUnmarkUnitSight(unit);
			RemoveUnitFromContainer(unit);
		}

		if (--unit->Refs > 0) {
			return;
		}
	}

	RefsAssert(!unit->Refs);

	//
	// No more references remaining, but the network could have an order
	// on the way. We must wait a little time before we could free the
	// memory.
	//
	RemoveUnit(unit, NULL);

	//
	// Remove the unit from the global units table.
	//
	Assert(*unit->UnitSlot == unit);
	temp = Units[--NumUnits];
	temp->UnitSlot = unit->UnitSlot;
	*unit->UnitSlot = temp;
	Units[NumUnits] = NULL;

	if (ReleasedHead) {
		ReleasedTail->Next = unit;
		ReleasedTail = unit;
		unit->Next = 0;
	} else {
		ReleasedHead = ReleasedTail = unit;
		unit->Next = 0;
	}
	unit->Refs = GameCycle + (NetworkMaxLag << 1); // could be reuse after this time
	unit->Type = 0;  // for debugging.
	free(unit->CacheLinks);

	if (ReleasedOrderHead) {
		ReleasedOrderTail->Arg1 = unit->Orders;
		ReleasedOrderTail = unit->Orders;
		unit->Orders->Arg1 = NULL;
	} else {
		ReleasedOrderHead = ReleasedOrderTail = unit->Orders;
		unit->Orders->Arg1 = NULL;
	}
	unit->Orders->X = GameCycle + (NetworkMaxLag << 1); // could be reuse after this time
	unit->Orders->Y = unit->TotalOrders; // store order count for when reused
	unit->Orders = NULL;
}

/**
**  FIXME: Docu
*/
static Unit* AllocUnit(void)
{
	Unit* unit;
	Unit** slot;
	//
	// Game unit limit reached.
	//
	if (NumUnits >= UnitMax) {
		DebugPrint("Over all unit limit (%d) reached.\n" _C_ UnitMax);
		// FIXME: Hoping this is checked everywhere.
		return NoUnitP;
	}

	//
	// Can use released unit?
	//
	if (ReleasedHead && (unsigned)ReleasedHead->Refs < GameCycle) {
		unit = ReleasedHead;
		ReleasedHead = unit->Next;
		if (ReleasedHead == 0) { // last element
			DebugPrint("Released unit queue emptied\n");
			ReleasedTail = ReleasedHead = 0;
		}
		DebugPrint("%lu:Release %p %d\n" _C_ GameCycle _C_ unit _C_ unit->Slot);
		slot = UnitSlots + unit->Slot;
		memset(unit, 0, sizeof(*unit));
		// FIXME: can release here more slots, reducing memory needs.
	} else {
		//
		// Allocate structure
		//
		if (!(slot = UnitSlotFree)) { // should not happen!
			DebugPrint("Maximum of units reached\n");
			return NoUnitP;
		}
		UnitSlotFree = (void*)*slot;
		*slot = unit = calloc(1, sizeof(*unit));
	}
	unit->Slot = slot - UnitSlots; // back index
	return unit;
}

/**
** Initialize the unit slot with default values.
**
** @param unit    Unit pointer (allocated zero filled)
** @param type    Unit-type
*/
void InitUnit(Unit* unit, UnitType* type)
{
	int i;

	Assert(type);

	//  Set refs to 1. This is the "I am alive ref", lost in ReleaseUnit.
	unit->Refs = 1;

	//
	//  Build all unit table
	//
	unit->UnitSlot = &Units[NumUnits]; // back pointer
	Units[NumUnits++] = unit;

	//
	//  Initialise unit structure (must be zero filled!)
	//
	unit->Type = type;
	unit->CacheLinks = calloc(type->TileWidth * type->TileHeight, sizeof(UnitListItem));
	for (i = 0; i < type->TileWidth * type->TileHeight; ++i) {
		unit->CacheLinks[i].Unit = unit;
	}

	unit->Seen.Frame = UnitNotSeen; // Unit isn't yet seen

	// On Load, Some units don't have Still animation, eg Deadbody
	if (unit->Type->Animations->Still) {
		unit->Frame = unit->Type->Animations->Still[0].Frame +
			(type->Building ? 0 : type->NumDirections / 2 + 1 - 1);
	}

	if (UnitTypeVar.NumberVariable) {
		unit->Variable = malloc(UnitTypeVar.NumberVariable * sizeof(*unit->Variable));
		memcpy(unit->Variable, unit->Type->Variable,
			UnitTypeVar.NumberVariable * sizeof(*unit->Variable));
	}

	if (!type->Building && type->Sprite &&
			VideoGraphicFrames(type->Sprite) > 5) {
		unit->Direction = (MyRand() >> 8) & 0xFF; // random heading
		UnitUpdateHeading(unit);
	}

	if (type->CanCastSpell) {
		unit->Mana = (type->_MaxMana * MAGIC_FOR_NEW_UNITS) / 100;
		unit->AutoCastSpell = malloc(SpellTypeCount);
		if (unit->Type->AutoCastActive) {
			memcpy(unit->AutoCastSpell, unit->Type->AutoCastActive, SpellTypeCount);
		} else {
			memset(unit->AutoCastSpell, 0, SpellTypeCount);
		}
	}
	unit->Active = 1;

	unit->Wait = 1;
	unit->Reset = 1;
	unit->Removed = 1;

	unit->Rs = MyRand() % 100; // used for fancy buildings and others

	// Init Orders and Default to Still/None
	if (ReleasedOrderHead && (unsigned)ReleasedOrderHead->X < GameCycle) {
		unit->Orders = ReleasedOrderHead;
		unit->TotalOrders = ReleasedOrderHead->Y;
		ReleasedOrderHead = (Order*)ReleasedOrderHead->Arg1;
	} else {
		// No Available Orders in Memory, create new ones
		unit->TotalOrders = DEFAULT_START_ORDERS;
		unit->Orders = calloc(unit->TotalOrders, sizeof(Order));
	}


	unit->OrderCount = 1; // No orders
	unit->Orders[0].Action = UnitActionStill;
	unit->Orders[0].X = unit->Orders[0].Y = -1;
	Assert(!unit->Orders[0].Goal);
	unit->NewOrder.Action = UnitActionStill;
	unit->NewOrder.X = unit->NewOrder.Y = -1;
	Assert(!unit->NewOrder.Goal);
	unit->SavedOrder.Action = UnitActionStill;
	unit->SavedOrder.X = unit->SavedOrder.Y = -1;
	Assert(!unit->SavedOrder.Goal);
}

/**
**  FIXME: Docu
**
**  @param unit    unit assigned to player.
**  @param player  player which have the unit.
**
**  @todo FIXME DOCU.
*/
void AssignUnitToPlayer(Unit* unit, Player* player)
{
	UnitType* type;  // type of unit.

	Assert(player);
	type = unit->Type;

	//
	// Build player unit table
	//
	if (!type->Vanishes && unit->Orders[0].Action != UnitActionDie) {
		unit->PlayerSlot = player->Units + player->TotalNumUnits++;
		if (!SaveGameLoading) {
			// If unit is dieing, it's already been lost by all players
			// don't count again
			if (type->Building) {
				// FIXME: support more races
				if (type != UnitTypeOrcWall && type != UnitTypeHumanWall) {
					player->TotalBuildings++;
				}
			} else {
				player->TotalUnits++;
			}
		}
		*unit->PlayerSlot = unit;

		player->UnitTypesCount[type->Slot]++;
	}

	if (type->Demand) {
		player->Demand += type->Demand; // food needed
	}
	// Don't Add the building if it's dieing, used to load a save game
	if (type->Building && unit->Orders[0].Action != UnitActionDie) {
		// FIXME: support more races
		if (type != UnitTypeOrcWall && type != UnitTypeHumanWall) {
			player->NumBuildings++;
		}
	}
	unit->Player = player;
	unit->Stats = &type->Stats[unit->Player->Player];
	unit->Colors = &player->UnitColors;
	unit->HP = unit->Stats->HitPoints;
}

/**
** Create a new unit.
**
** @param type      Pointer to unit-type.
** @param player    Pointer to owning player.
**
** @return          Pointer to created unit.
*/
Unit* MakeUnit(UnitType* type, Player* player)
{
	Unit* unit;

	//Assert(player); // Current code didn't support no player

	unit = AllocUnit();
	InitUnit(unit, type);

	// Only Assign if a Player was specified
	if (player) {
		AssignUnitToPlayer(unit, player);
	}

	if (type->Building) {
		//
		//  fancy buildings: mirror buildings (but shadows not correct)
		//
		if (FancyBuildings && unit->Rs > 50) {
			unit->Frame = -unit->Frame - 1;
		}
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
**  @param f2        Function to (un)mark for cloaking vision.
*/
static void MapMarkUnitSightRec(Unit* unit, int x, int y, int width, int height,
	MapMarkerFunc* f, MapMarkerFunc* f2)
{
	Unit* unit_inside; // iterator on units inside unit.
	int i;             // number of units inside to process.

	Assert(unit);
	Assert(f);
	MapSight(unit->Player, x, y, width, height, unit->CurrentSightRange, f);

	if (unit->Type && unit->Type->DetectCloak && f2) {
		MapSight(unit->Player, x, y, width, height, unit->CurrentSightRange, f2);
	}

	unit_inside = unit->UnitInside;
	for (i = unit->InsideCount; i--; unit_inside = unit_inside->NextContained) {
		MapMarkUnitSightRec(unit_inside, x, y, width, height, f, f2);
	}
}

/**
**  Return the unit not transported, by viewing the container recursively.
**
**  @param unit    unit from where look the first conatiner.
**
**  @return        Container of container of ... of unit. It is not null.
*/
static Unit* GetFirstContainer(const Unit* unit)
{
	Assert(unit);
	while (unit->Container) {
		unit = unit->Container;
	}
	return (Unit *) unit;
}

/**
**  Mark on vision table the Sight of the unit
**  (and units inside for transporter)
**
**  @param unit    unit to unmark its vision.
**  @see MapUnmarkUnitSight.
*/
void MapMarkUnitSight(Unit* unit)
{
	Unit* container;  // First container of the unit.

	Assert(unit);

	container = GetFirstContainer(unit);
	Assert(container->Type);
	MapMarkUnitSightRec(unit,
		container->X, container->Y, container->Type->TileWidth, container->Type->TileHeight,
		MapMarkTileSight, MapMarkTileDetectCloak);
}

/**
**  Unmark on vision table the Sight of the unit
**  (and units inside for transporter)
**
**  @param unit    unit to unmark its vision.
**  @see MapMarkUnitSight.
*/
void MapUnmarkUnitSight(Unit* unit)
{
	Unit* container;  // First container of the unit.

	Assert(unit);
	Assert(unit->Type);

	container = GetFirstContainer(unit);
	Assert(container->Type);
	MapMarkUnitSightRec(unit,
		container->X, container->Y, container->Type->TileWidth, container->Type->TileHeight,
		MapUnmarkTileSight, MapUnmarkTileDetectCloak);
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
static void UpdateUnitSightRange(Unit* unit)
{
	Unit* unit_inside; // iterator on units inside unit.
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
		unit->CurrentSightRange = unit->Stats->SightRange;
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
static void MarkUnitFieldFlags(const Unit* unit)
{
	UnitType* type; // Type of the unit.
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
			TheMap.Fields[x + w + (y + h) * TheMap.Width].Flags |= flags;
		}
	}
#ifdef MAP_REGIONS
	// Update map splitting.
	if (!CanMove(unit) && (flags &
		 (MapFieldLandUnit | MapFieldSeaUnit | MapFieldBuilding |
		  MapFieldUnpassable | MapFieldWall | MapFieldRocks | MapFieldForest))) {
		MapSplitterTilesOccuped(x, y, x + type->TileWidth - 1, y + type->TileHeight - 1);
	}
#endif
}

/**
**  Mark the field with the FieldFlags.
**
**  @param unit    unit to mark.
*/
static void UnmarkUnitFieldFlags(const Unit* unit)
{
	UnitType* type; // Type of the unit.
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
			TheMap.Fields[x + w + (y + h) * TheMap.Width].Flags &= ~flags;
		}
	}
#ifdef MAP_REGIONS
	// Update map splitting.
	if (!CanMove(unit) && (flags &
		 (MapFieldLandUnit | MapFieldSeaUnit | MapFieldBuilding |
		  MapFieldUnpassable | MapFieldWall | MapFieldRocks | MapFieldForest))){
		MapSplitterTilesCleared(x, y, x + type->TileWidth - 1, y + type->TileHeight - 1);
	}
#endif
}

/**
** Add unit to a container. It only updates linked list stuff
**
** @param unit    Pointer to unit.
** @param host    Pointer to container.
*/
void AddUnitInContainer(Unit* unit, Unit* host)
{
	Assert(host && unit->Container == 0);
	unit->Container = host;
	if (host->InsideCount == 0) {
		unit->NextContained = unit->PrevContained = unit;
	} else {
		unit->NextContained = host->UnitInside;
		unit->PrevContained = host->UnitInside->PrevContained;
		host->UnitInside->PrevContained->NextContained = unit;
		host->UnitInside->PrevContained = unit;
	}
	host->UnitInside = unit;
	host->InsideCount++;
}

/**
** Remove unit from a container. It only updates linked list stuff
**
** @param unit    Pointer to unit.
*/
static void RemoveUnitFromContainer(Unit* unit)
{
	Unit* host;  // transporter which contain unit.

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
**  Affect Tile coord of an unit (with units inside) to tile (x, y).
**
**  @param unit    unit to move.
**  @param x       X map tile position.
**  @param y       Y map tile position.
**
**  @internal before use it, UnitCacheRemove(unit), MapUnmarkUnitSight(unit)
**  and after UnitCacheInsert(unit), MapMarkUnitSight(unit)
**  are often necessary. Check Flag also for Pathfinder.
*/
static void UnitInXY(Unit* unit, int x, int y)
{
	Unit* unit_inside;      // iterator on units inside unit.
	int i;                  // number of units inside to process.

	Assert(unit);
	unit->X = x;
	unit->Y = y;

	unit_inside = unit->UnitInside;
	for (i = unit->InsideCount; i--; unit_inside = unit_inside->NextContained) {
		UnitInXY(unit_inside, x, y);
	}
}

/**
**  Move an unit (with units inside) to tile (x, y).
**  (Do stuff with vision, cachelist and pathfinding).
**
**  @param unit    unit to move.
**  @param x       X map tile position.
**  @param y       Y map tile position.
**
*/
void MoveUnitToXY(Unit* unit, int x, int y)
{
	MapUnmarkUnitSight(unit);
	UnitCacheRemove(unit);
	UnmarkUnitFieldFlags(unit);

	// Move the unit.
	UnitInXY(unit, x, y);

	UnitCacheInsert(unit);
	MarkUnitFieldFlags(unit);
	MapMarkUnitSight(unit);
}




/**
** Place unit on map.
**
** @param unit    Unit to be placed.
** @param x       X map tile position.
** @param y       Y map tile position.
*/
void PlaceUnit(Unit* unit, int x, int y)
{
	Assert(unit->Removed);

	if (unit->Container) {
		MapUnmarkUnitSight(unit);
		RemoveUnitFromContainer(unit);
	}
	unit->Next = 0;
	if (!SaveGameLoading) {
		UpdateUnitSightRange(unit);
	}
	unit->Removed = 0;
	UnitInXY(unit, x, y);
	// Pathfinding info.
	MarkUnitFieldFlags(unit);
	// Tha cache list.
	UnitCacheInsert(unit);
	// Vision
	MapMarkUnitSight(unit);

	UnitCountSeen(unit);
}

/**
** Create new unit and place on map.
**
** @param x         X map tile position.
** @param y         Y map tile position.
** @param type      Pointer to unit-type.
** @param player    Pointer to owning player.
**
** @return          Pointer to created unit.
*/
Unit* MakeUnitAndPlace(int x, int y, UnitType* type, Player* player)
{
	Unit* unit;

	unit = MakeUnit(type, player);
	PlaceUnit(unit, x, y);

	return unit;
}

/**
** Remove unit from map.
**
** Update selection.
** Update panels.
** Update map.
**
** @param unit    Pointer to unit.
** @param host    Pointer to housing unit.
*/
void RemoveUnit(Unit* unit, Unit* host)
{
	if (unit->Removed) { // could happen!
		// If unit is removed (inside) and building is destroyed.
		DebugPrint("unit '%s(%d)' already removed\n" _C_ unit->Type->Ident _C_ unit->Slot);
		return;
	}
	UnitCacheRemove(unit);
	MapUnmarkUnitSight(unit);
	UnmarkUnitFieldFlags(unit);
	if (host) {
		AddUnitInContainer(unit, host);
		UpdateUnitSightRange(unit);
		UnitInXY(unit, host->X, host->Y);
		MapMarkUnitSight(unit);
		unit->Next = host; // What is it role ?
	}

	unit->Removed = 1;

	//  Remove unit from the current selection
	if (unit->Selected) {
		if (NumSelected == 1) { //  Remove building cursor
			CancelBuildingMode();
		}
		UnSelectUnit(unit);
		SelectionChanged();
	}
	// Remove unit from team selections
	if (!unit->Selected && unit->TeamSelected) {
		UnSelectUnit(unit);
	}

	// Unit is seen as under cursor
	if (unit == UnitUnderCursor) {
		UnitUnderCursor = NULL;
	}
}

/**
**  Update information for lost units.
**
**  @param unit    Pointer to unit.
**
**  @note Also called by ChangeUnitOwner
*/
void UnitLost(Unit* unit)
{
	Unit* temp;
	BuildRestriction* b;
	const UnitType* type;
	Player* player;
	int i;

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
			// FIXME: support more races
			if (type != UnitTypeOrcWall && type != UnitTypeHumanWall) {
				player->NumBuildings--;
			}
		}

		if (unit->Orders[0].Action != UnitActionBuilded) {
			player->UnitTypesCount[type->Slot]--;
		}
	}


	//
	//  Handle unit demand. (Currently only food supported.)
	//
	if (type->Demand) {
		player->Demand -= type->Demand;
	}

	//
	//  Update information.
	//
	if (unit->Orders[0].Action != UnitActionBuilded) {
		if (type->Supply) {
			player->Supply -= type->Supply;
		}

		//
		//  Handle income improvements, look if a player loses a building
		//  which have given him a better income, find the next best
		//  income.
		//
		for (i = 1; i < MaxCosts; ++i) {
			if (player->Incomes[i] && type->ImproveIncomes[i] == player->Incomes[i]) {
				int m;
				int j;

				m = DefaultIncomes[i];
				for (j = 0; j < player->TotalNumUnits; ++j) {
					if (m < player->Units[j]->Type->ImproveIncomes[i]) {
						m = player->Units[j]->Type->ImproveIncomes[i];
					}
				}
				player->Incomes[i] = m;
			}
		}
	}

	//
	//  Handle research cancels.
	//
	if (unit->Orders[0].Action == UnitActionResearch) {
		unit->Player->UpgradeTimers.Upgrades[unit->Data.Research.Upgrade - Upgrades] = 0;
	}

	DebugPrint("Lost %s(%d)\n" _C_ unit->Type->Ident _C_ UnitNumber(unit));

	// Destroy resource-platform, must re-make resource patch.
	if ((b = OnTopDetails(unit, NULL)) != NULL) {
		if (b->Data.OnTop.ReplaceOnDie && (unit->Type->GivesResource && unit->ResourcesHeld != 0)) {
			temp = MakeUnitAndPlace(unit->X, unit->Y, b->Data.OnTop.Parent, &Players[PlayerNumNeutral]);
			temp->ResourcesHeld = unit->ResourcesHeld;
		}
	}
	Assert(player->NumBuildings <= UnitMax);
	Assert(player->TotalNumUnits <= UnitMax);
	Assert(player->UnitTypesCount[type->Slot] <= UnitMax);
}

/**
**  FIXME: Docu
**
**  @param unit  FIXME: docu
*/
void UnitClearOrders(Unit* unit)
{
	int i;

	//
	//  Release all references of the unit.
	//
	for (i = unit->OrderCount; i-- > 0;) {
		if (unit->Orders[i].Goal) {
			RefsDecrease(unit->Orders[i].Goal);
			unit->Orders[i].Goal = NoUnitP;
		}
		unit->OrderCount = 1;
	}
	if (unit->NewOrder.Goal) {
		RefsDecrease(unit->NewOrder.Goal);
		unit->NewOrder.Goal = NoUnitP;
	}
	if (unit->SavedOrder.Goal) {
		RefsDecrease(unit->SavedOrder.Goal);
		unit->SavedOrder.Goal = NoUnitP;
	}
	unit->Orders[0].Action = UnitActionStill;
	unit->SubAction = unit->State = 0;
}

/**
**  Update for new unit. Food and income ...
**
**  @param unit     New unit pointer.
**  @param upgrade  True unit was upgraded.
*/
void UpdateForNewUnit(const Unit* unit, int upgrade)
{
	const UnitType* type;
	Player* player;
	int u;

	player = unit->Player;
	type = unit->Type;

	//
	// Handle unit supply. (Currently only food supported.)
	// Note an upgraded unit can't give more supply.
	//
	if (type->Supply && !upgrade) {
		player->Supply += type->Supply;
	}

	//
	// Update resources
	//
	for (u = 1; u < MaxCosts; ++u) {
		if (player->Incomes[u] < unit->Type->ImproveIncomes[u]) {
			player->Incomes[u] = unit->Type->ImproveIncomes[u];
		}
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
void NearestOfUnit(const Unit* unit, int tx, int ty, int *dx, int *dy)
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
static void UnitFillSeenValues(Unit* unit)
{
	// Seen values are undefined for visible units.
	unit->Seen.IY = unit->IY;
	unit->Seen.IX = unit->IX;
	unit->Seen.Frame = unit->Frame;
	unit->Seen.State = (unit->Orders[0].Action == UnitActionBuilded) |
			((unit->Orders[0].Action == UnitActionUpgradeTo) << 1);
	if (unit->Orders[0].Action == UnitActionDie) {
		unit->Seen.State = 3;
	}
	unit->Seen.Type = unit->Type;
	unit->Seen.Constructed = unit->Constructed;
	unit->Seen.CFrame = unit->Data.Builded.Frame;
}

/**
**  This function should get called when a unit goes under fog of war.
**
**  @param unit    The unit that goes under fog.
**  @param player  The player the unit goes out of fog for.
*/
void UnitGoesUnderFog(Unit* unit, const Player* player)
{
	if (unit->Type->VisibleUnderFog) {
		if (player->Type == PlayerPerson && !unit->Destroyed) {
			RefsIncrease(unit);
		}
		//
		// Icky yucky icky Seen.Destroyed trickery.
		// We track for each player if he's seen the unit as destroyed.
		// Remember, an unit is marked Destroyed when it's gone as in
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
			unit->Seen.Destroyed |= (1 << player->Player);
		}
		if (player == ThisPlayer) {
			UnitFillSeenValues(unit);
		}
	}
}

/**
**  This function should get called when an unit goes out of fog of war.
**
**  @param unit    The unit that goes out of fog.
**  @param player  The player the unit goes out of fog for.
**
** @note For units that are visible under fog (mostly buildings)
** we use reference counts, from the players that know about
** the building. When an building goes under fog it gets a refs
** increase, and when it shows up it gets a decrease. It must
** not get an decrease the first time it's seen, so we have to
** keep track of what player saw what units, with SeenByPlayer.
*/
void UnitGoesOutOfFog(Unit* unit, const Player* player)
{
	if (unit->Type->VisibleUnderFog) {
		if (unit->Seen.ByPlayer & (1 << (player->Player))) {
			if ((player->Type == PlayerPerson) &&
					(!(   unit->Seen.Destroyed & (1 << player->Player)   )) ) {
				RefsDecrease(unit);
			}
		} else {
			unit->Seen.ByPlayer |= (1 << (player->Player));
		}
	}
}

/**
**  Mark all units on a tile as now visible.
**
**  @param player  The player this is for.
**  @param x       x location to check
**  @param y       y location to check
**  @param cloak   If we mark cloaked units too.
*/
void UnitsOnTileMarkSeen(const Player* player, int x, int y, int cloak)
{
	int p;
	int n;
	Unit* units[UnitMax];
	Unit* unit;

	n = UnitCacheOnTile(x, y,units);
	while (n) {
		unit = units[--n];
		if (cloak != (int)unit->Type->PermanentCloak) {
			continue;
		}
		//
		//  If the unit goes out of fog, this can happen for any player that
		//  this player shares vision with, and can't YET see the unit.
		//  It will be able to see the unit after the Unit->VisCount ++
		//
		for (p = 0; p < PlayerMax; ++p) {
			if (PlayersShareVision(player->Player, p) || (p == player->Player)) {
				if (!UnitVisible(unit, Players + p)) {
					UnitGoesOutOfFog(unit, Players + p);
				}
			}
		}
		unit->VisCount[player->Player]++;
	}
}

/**
** This function unmarks units on x, y as seen. It uses a reference count.
**
** @param player    The player to mark for.
** @param x         x location to check if building is on, and mark as seen
** @param y         y location to check if building is on, and mark as seen
** @param cloak     If this is for cloaked units.
*/
void UnitsOnTileUnmarkSeen(const Player* player, int x, int y, int cloak)
{
	int p;
	int n;
	Unit* units[UnitMax];
	Unit* unit;

	n = UnitCacheOnTile(x, y, units);
	while (n) {
		unit = units[--n];
		Assert(unit->X <= x && unit->X + unit->Type->TileWidth - 1 >= x &&
			unit->Y <= y && unit->Y + unit->Type->TileHeight - 1 >= y);
		if (cloak != (int)unit->Type->PermanentCloak) {
			continue;
		}
		p = player->Player;
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
				if (PlayersShareVision(player->Player, p) || p == player->Player) {
					if (!UnitVisible(unit, Players + p)) {
						UnitGoesUnderFog(unit, Players + p);
					}
				}
			}
		}
	}
}

/**
** Recalculates an units visiblity count. This happens really often,
** Like every time an unit moves. It's really fast though, since we
** have per-tile counts.
**
** @param unit    pointer to the unit to check if seen
*/
void UnitCountSeen(Unit* unit)
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
			oldv[p] = UnitVisible(unit, &Players[p]);
		}
	}

	//  Calculate new VisCount values.
	for (p = 0; p < PlayerMax; ++p) {
		if (Players[p].Type != PlayerNobody) {
			newv = 0;
			for (x = 0; x < unit->Type->TileWidth; ++x) {
				for (y = 0; y < unit->Type->TileHeight; ++y) {
					if (unit->Type->PermanentCloak) {
						if (TheMap.Fields[(unit->Y + y) * TheMap.Width + unit->X + x].VisCloak[p]) {
							newv++;
						}
					} else {
						//  Icky ugly code trick. With NoFogOfWar we haveto be > 0;
						if (TheMap.Fields[(unit->Y + y) * TheMap.Width + unit->X + x].Visible[p] > 1 - TheMap.NoFogOfWar) {
							newv++;
						}
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
			newv = UnitVisible(unit, Players + p);
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
**  @todo FIXME: optimize this a lot.
**
**  @note This understands shared vision, and should be used all around.
**
**  @param unit    The unit to check.
**  @param player  The player to check.
*/
int UnitVisible(const Unit* unit, const Player* player)
{
	int p;
	int cp;

	// Current player.
	cp = player->Player;
	if (unit->VisCount[cp]) {
		return 1;
	}
	for (p = 0; p < PlayerMax; ++p) {
		if (PlayersShareVision(p, cp)) {
			if (unit->VisCount[p]) {
				return 1;
			}
		}
	}
	return 0;
}

/**
**  Returns ture, if unit is visible as an action goal for a player
**  on the map.
**
**  @param unit    Unit to be checked.
**  @param player  Player to check for.
**
**  @return        True if visible, false otherwise.
*/
int UnitVisibleAsGoal(const Unit* unit, const Player* player)
{
	//
	// Invisibility
	//
	if (unit->Invisible && (player != unit->Player) &&
			(!PlayersShareVision(player->Player, unit->Player->Player))) {
		return 0;
	}
	if (UnitVisible(unit, player) || player->Type == PlayerComputer) {
		return  (!unit->Removed) &&
				(!unit->Destroyed) &&
				(unit->Orders->Action != UnitActionDie);
	} else {
		return unit->Type->VisibleUnderFog &&
			(unit->Seen.ByPlayer & (1 << player->Player)) &&
			!(unit->Seen.Destroyed & (1 << player->Player));
	}
}

/**
**  Returns true, if unit is visible for this player on the map.
**  The unit has to be out of fog of war and alive
**
**  @param unit    Unit to be checked.
**  @param player  Player to check for.
**
**  @return        True if visible, false otherwise.
*/
int UnitVisibleOnMap(const Unit* unit, const Player* player)
{
	//
	// Invisible units.
	//
	if (unit->Invisible && player != unit->Player &&
			!PlayersShareVision(player->Player, unit->Player->Player)) {
		return 0;
	}

	return !unit->Removed && !unit->Destroyed && unit->HP &&
		unit->Orders->Action != UnitActionDie && UnitVisible(unit, player);
}

/**
**  Returns true, if unit is shown on minimap.
**
**  @warning This is for ::ThisPlayer only.
**  @todo radar support
**
**  @param unit  Unit to be checked.
**
**  @return      True if visible, false otherwise.
*/
int UnitVisibleOnMinimap(const Unit* unit)
{
	//
	// Invisible units.
	//
	if (unit->Invisible && (ThisPlayer != unit->Player) &&
			(!PlayersShareVision(ThisPlayer->Player, unit->Player->Player))) {
		return 0;
	}
	if (UnitVisible(unit, ThisPlayer) || ReplayRevealMap) {
		return (!unit->Removed) &&
				(!unit->Destroyed) &&
				(unit->Orders->Action != UnitActionDie);
	} else {
		if (!unit->Type->VisibleUnderFog) {
			return 0;
		}
		return ((unit->Seen.ByPlayer & (1 << ThisPlayer->Player)) &&
			unit->Seen.State != 3 &&
			!(unit->Seen.Destroyed & (1 << ThisPlayer->Player)));
	}
}

/**
**  Returns true, if unit is visible in viewport.
**
**  @warning This is only true for ::ThisPlayer
**
**  @param vp    Viewport pointer.
**  @param unit  Unit to be checked.
**
**  @return      True if visible, false otherwise.
*/
int UnitVisibleInViewport(const Unit* unit, const Viewport* vp)
{
	//
	// Check if it's at least inside the damn viewport.
	//
	if ((unit->X + unit->Type->TileWidth < vp->MapX) ||
			(unit->X > vp->MapX + vp->MapWidth) ||
			(unit->Y + unit->Type->TileHeight < vp->MapY) ||
			(unit->Y > vp->MapY + vp->MapHeight)) {
		return 0;
	}

	if (!ThisPlayer) {
		//FIXME: ARI: Added here for early game setup state by
		// MakeAndPlaceUnit() from LoadMap(). ThisPlayer not yet set,
		// so don't show anything until first real map-draw.
		DebugPrint("Fix ME ThisPlayer not set yet?!\n");
		return 0;
	}

	// Those are never ever visible.
	if (unit->Invisible && ThisPlayer != unit->Player &&
			!PlayersShareVision(ThisPlayer->Player, unit->Player->Player)) {
		return 0;
	}

	if (UnitVisible(unit, ThisPlayer) || ReplayRevealMap) {
		return !unit->Destroyed;
	} else {
		// Unit has to be 'discovered'
		// Destroyed units ARE visible under fog of war, if we haven't seen them like that.
		if (!unit->Destroyed || !(unit->Seen.Destroyed & (1 << ThisPlayer->Player))) {
			return (unit->Type->VisibleUnderFog && (unit->Seen.ByPlayer & (1 << ThisPlayer->Player)));
		} else {
			return 0;
		}
	}
}

/**
**  Returns true, if unit is visible on current map view (any viewport).
**
**  @param unit  Unit to be checked.
**
**  @return      True if visible, false otherwise.
*/
int UnitVisibleOnScreen(const Unit* unit)
{
	const Viewport* vp;

	for (vp = TheUI.Viewports; vp < TheUI.Viewports + TheUI.NumViewports; ++vp) {
		if (UnitVisibleInViewport(unit, vp)) {
			return 1;
		}
	}
	return 0;
}

/**
**  Get area of map tiles covered by unit, including its displacement.
**
**  @param unit  Unit to be checked and set.
**  @param sx    Out: Top left X tile map postion.
**  @param sy    Out: Top left Y tile map postion.
**  @param ex    Out: Bottom right X tile map postion.
**  @param ey    Out: Bottom right Y tile map postion.
**
**  @return      sx,sy,ex,ey defining area in Map
*/
void GetUnitMapArea(const Unit* unit, int* sx, int* sy, int* ex, int* ey)
{
	*sx = unit->X - (unit->IX < 0);
	*ex = *sx + unit->Type->TileWidth - !unit->IX;
	*sy = unit->Y - (unit->IY < 0);
	*ey = *sy + unit->Type->TileHeight - !unit->IY;
}

/**
**  Change the unit's owner
**
**  @param unit       Unit which should be consigned.
**  @param newplayer  New owning player.
*/
void ChangeUnitOwner(Unit* unit, Player* newplayer)
{
	int i;
	Unit* uins;
	Player* oldplayer;

	oldplayer = unit->Player;

	// This shouldn't happen
	if (oldplayer == newplayer) {
		DebugPrint("Change the unit owner to the same player???\n");
		return;
	}

	// Rescue all units in buildings/transporters.
	uins = unit->UnitInside;
	for (i = unit->InsideCount; i; --i, uins = uins->NextContained) {
		ChangeUnitOwner(uins, newplayer);
	}

	//
	//  Must change food/gold and other.
	//
	UnitLost(unit);

	//
	//  Now the new side!
	//

	// Insert into new player table.

	unit->PlayerSlot = newplayer->Units + newplayer->TotalNumUnits++;
	if (unit->Type->_HitPoints != 0) {
		if (unit->Type->Building) {
			newplayer->TotalBuildings++;
		}
		else {
			newplayer->TotalUnits++;
		}
	}
	*unit->PlayerSlot = unit;

	MapUnmarkUnitSight(unit);
	unit->Player = newplayer;
	MapMarkUnitSight(unit);

	unit->Stats = &unit->Type->Stats[newplayer->Player];
	//
	//  Must change food/gold and other.
	//
	if (unit->Type->GivesResource) {
		DebugPrint("Resource transfer not supported\n");
	}
	newplayer->Demand += unit->Type->Demand;
	newplayer->Supply += unit->Type->Supply;
	if (unit->Type->Building) {
		newplayer->NumBuildings++;
	}
	newplayer->UnitTypesCount[unit->Type->Slot]++;

	UpdateForNewUnit(unit, 0);
}

/**
**  Change the owner of all units of a player.
**
**  @param oldplayer    Old owning player.
**  @param newplayer    New owning player.
*/
static void ChangePlayerOwner(Player* oldplayer, Player* newplayer)
{
	Unit* table[UnitMax];
	Unit* unit;
	int i;
	int n;

	// NOTE: table is changed.
	n = oldplayer->TotalNumUnits;
	memcpy(table, oldplayer->Units, n * sizeof(Unit*));
	for (i = 0; i < n; ++i) {
		unit = table[i];
		// Don't save the unit again(can happen when inside a town hall)
		if (unit->Player == newplayer) {
			continue;
		}
		ChangeUnitOwner(unit, newplayer);
		unit->Blink = 5;
		unit->RescuedFrom = oldplayer;
	}
}

/**
**  Rescue units.
**
**  Look through all rescueable players, if they could be rescued.
*/
void RescueUnits(void)
{
	Player* p;
	Unit* unit;
	Unit* table[UnitMax];
	Unit* around[UnitMax];
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
			memcpy(table, p->Units, l * sizeof(Unit*));
			for (j = 0; j < l; ++j) {
				unit = table[j];
				// Do not rescue removed units. Units inside something are
				// rescued by ChangeUnitOwner
				if (unit->Removed) {
					continue;
				}
				// FIXME: I hope SelectUnits checks bounds?
				// FIXME: Yes, but caller should check.
				// NOTE: +1 right,bottom isn't inclusive :(
				if (unit->Type->UnitType == UnitTypeLand) {
					n = UnitCacheSelect(
							unit->X - 1, unit->Y - 1,
							unit->X + unit->Type->TileWidth + 1,
							unit->Y + unit->Type->TileHeight + 1, around);
				} else {
					n = UnitCacheSelect(
							unit->X - 2, unit->Y - 2,
							unit->X + unit->Type->TileWidth + 2,
							unit->Y + unit->Type->TileHeight + 2, around);
				}
				//
				//  Look if ally near the unit.
				//
				for (i = 0; i < n; ++i) {
					if (around[i]->Type->CanAttack &&
							IsAllied(unit->Player, around[i])) {
						//
						//  City center converts complete race
						//  NOTE: I use a trick here, centers could
						//        store gold. FIXME!!!
						if (unit->Type->CanStore[GoldCost]) {
							ChangePlayerOwner(p, around[i]->Player);
							break;
						}
						unit->RescuedFrom = unit->Player;
						ChangeUnitOwner(unit, around[i]->Player);
						unit->Blink = 5;
						PlayGameSound(GameSounds.Rescue[unit->Player->Race].Sound,
							MaxSampleVolume);
						break;
					}
				}
			}
		}
	}
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
void UnitUpdateHeading(Unit* unit)
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
void UnitHeadingFromDeltaXY(Unit* unit, int dx, int dy)
{
	unit->Direction = DirectionToHeading(dx, dy);
	UnitUpdateHeading(unit);
}

/*----------------------------------------------------------------------------
  -- Drop out units
  ----------------------------------------------------------------------------*/

/**
** Reappear unit on map.
**
** @param unit       Unit to drop out.
** @param heading    Direction in which the unit should appear.
** @param addx       Tile size in x.
** @param addy       Tile size in y.
*/
void DropOutOnSide(Unit* unit, int heading, int addx, int addy)
{
	int x;
	int y;
	int i;
	int mask;

	if (unit->Container) {
		x = unit->Container->X;
		y = unit->Container->Y;
	} else {
		x = unit->X;
		y = unit->Y;
		// n0b0dy: yes, when training an unit.
	}


	mask = UnitMovementMask(unit);

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
			if (CheckedCanMoveToMask(x, y, mask)) {
				goto found;
			}
		}
		++addx;
starts:
		for (i = addx; i--; ++x) {
			if (CheckedCanMoveToMask(x, y, mask)) {
				goto found;
			}
		}
		++addy;
starte:
		for (i = addy; i--; --y) {
			if (CheckedCanMoveToMask(x, y, mask)) {
				goto found;
			}
		}
		++addx;
startn:
		for (i = addx; i--; --x) {
			if (CheckedCanMoveToMask(x, y, mask)) {
				goto found;
			}
		}
		++addy;
	}

found:
	unit->Wait = 1; // should be correct unit has still action

	PlaceUnit(unit, x, y);
}

/**
** Reappear unit on map nearest to x, y.
**
** @param unit    Unit to drop out.
** @param gx      Goal X map tile position.
** @param gy      Goal Y map tile position.
** @param addx    Tile size in x.
** @param addy    Tile size in y.
*/
void DropOutNearest(Unit* unit, int gx, int gy, int addx, int addy)
{
	int x;
	int y;
	int i;
	int bestx;
	int besty;
	int bestd;
	int mask;
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
	mask = UnitMovementMask(unit);

	bestd = 99999;
#ifdef DEBUG
	bestx = besty = 0; // keep the compiler happy
#endif

	// FIXME: if we reach the map borders we can go fast up, left, ...
	--x;
	for (;;) {
		for (i = addy; i--; ++y) { // go down
			if (CheckedCanMoveToMask(x, y, mask)) {
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
			if (CheckedCanMoveToMask(x, y, mask)) {
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
			if (CheckedCanMoveToMask(x, y, mask)) {
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
			if (CheckedCanMoveToMask(x, y, mask)) {
				n = MapDistance(gx, gy, x, y);
				if (n < bestd) {
					bestd = n;
					bestx = x;
					besty = y;
				}
			}
		}
		if (bestd != 99999) {
			unit->Wait = 1; // unit should have action still
			PlaceUnit(unit, bestx, besty);
			return;
		}
		++addy;
	}
}

/**
** Drop out all units inside unit.
**
** @param source    All units inside source are dropped out.
*/
void DropOutAll(const Unit* source)
{
	Unit* unit;
	int i;

	unit = source->UnitInside;
	for (i = source->InsideCount; i; --i, unit = unit->NextContained) {
		DropOutOnSide(unit, LookingW,
			source->Type->TileWidth, source->Type->TileHeight);
		Assert(!unit->Orders[0].Goal);
		unit->Orders[0].Action = UnitActionStill;
		unit->Wait=unit->Reset = 1;
		unit->SubAction = 0;
	}
	DebugPrint("Drop out %d of %d\n" _C_ i _C_ source->Data.Resource.Active);
}

/*----------------------------------------------------------------------------
  --  Building units
  ----------------------------------------------------------------------------*/

/*
**  Find the building restriction that gives me this unit built on top
**  Allows you to define how the restriction is effecting the build
**
**  @param unit    the unit that is "OnTop"
**  @param parent  the parent unit if known. (guess otherwise)
**
**  @return        the BuildingRestrictionDetails
*/
BuildRestriction* OnTopDetails(const Unit* unit, const UnitType* parent)
{
	int w;
	BuildRestriction* b;

	if (unit->Type->BuildingRules) {
		w = 0;
		while (unit->Type->BuildingRules[w] != NULL) {
			b = unit->Type->BuildingRules[w];
			while (b != NULL) {
				if (b->RestrictType == RestrictOnTop) {
					if (parent && parent == b->Data.OnTop.Parent) {
						return b;
					} else if (!parent) {
						// Guess this is right
						return b;
					}
				}
				b = b->Next;
			}
		++w;
		}
	}
	return NULL;
}

/**
**  Can build unit here.
**  Hall to near to goldmine.
**  Refinery or shipyard to near to oil patch.
**
**  @param unit  Unit doing the building
**  @param type  unit-type to be checked.
**  @param x     Map X position.
**  @param y     Map Y position.
**
**  @return      OnTop, parent unit, builder on true or 1 if unit==NULL, NULL false.
*/
Unit* CanBuildHere(const Unit* unit, const UnitType* type, int x, int y)
{
	Unit* table[UnitMax];
	Unit* ontoptarget;
	BuildRestriction* b;
	int n;
	int i;
	int w;
	int h;
	int x1;
	int x2;
	int y1;
	int y2;
	int distance;
	int success;

	//
	//  Can't build outside the map
	//
	if (x + type->TileWidth > TheMap.Width) {
		return NULL;
	}
	if (y + type->TileHeight > TheMap.Height) {
		return NULL;
	}

	// Must be checked before oil!
	if (type->ShoreBuilding) {
		success = 0;

		// Need at least one coast tile
		for (h = type->TileHeight; h--;) {
			for (w = type->TileWidth; w--;) {
				if (TheMap.Fields[x + w + (y + h) * TheMap.Width].Flags &
						MapFieldCoastAllowed) {
					h = w = 0;
					success = 1;
				}
			}
		}
		if (!success) {
			return NULL;
		}
	}


	// Keep gcc happy
	distance = 0;
	x1 = 0;
	x2 = 0;
	y1 = 0;
	y2 = 0;
	ontoptarget = NULL;
	
	if (type->BuildingRules) {
		w = 0;
		while (type->BuildingRules[w] != NULL) {
			b = type->BuildingRules[w];
			success = 1;
			while (b != NULL) {
				// Run Distance Checking
				if (b->RestrictType == RestrictDistance) {
					if (b->Data.Distance.DistanceType == LessThanEqual ||
							b->Data.Distance.DistanceType == GreaterThan ||
							b->Data.Distance.DistanceType == Equal ||
							b->Data.Distance.DistanceType == NotEqual) {
						x1 = x - b->Data.Distance.Distance > 0 ? x - b->Data.Distance.Distance : 0;
						y1 = y - b->Data.Distance.Distance > 0 ? y - b->Data.Distance.Distance : 0;
						x2 = x + type->TileWidth + b->Data.Distance.Distance < TheMap.Width ?
							x + type->TileWidth + b->Data.Distance.Distance : TheMap.Width;
						y2 = y + type->TileHeight + b->Data.Distance.Distance < TheMap.Height ?
							y + type->TileHeight + b->Data.Distance.Distance : TheMap.Height;
						distance = b->Data.Distance.Distance;
					} else if (b->Data.Distance.DistanceType == LessThan ||
							b->Data.Distance.DistanceType == GreaterThanEqual) {
						x1 = x - b->Data.Distance.Distance - 1 > 0 ? x - b->Data.Distance.Distance - 1 : 0;
						y1 = y - b->Data.Distance.Distance - 1 > 0 ? y - b->Data.Distance.Distance - 1 : 0;
						x2 = x + type->TileWidth + b->Data.Distance.Distance + 1 < TheMap.Width ?
							x + type->TileWidth + b->Data.Distance.Distance + 1 : TheMap.Width;
						y2 = y + type->TileHeight + b->Data.Distance.Distance + 1 < TheMap.Height ?
							y + type->TileHeight + b->Data.Distance.Distance + 1 : TheMap.Height;
						distance = b->Data.Distance.Distance - 1;
					}
					n = UnitCacheSelect(x1, y1, x2, y2, table);
				
					// These type find success, not lose it
					if (b->Data.Distance.DistanceType == LessThanEqual ||
						b->Data.Distance.DistanceType == LessThan ||
						b->Data.Distance.DistanceType == Equal) {
						success = 0;
					}
					for (i = 0; i < n; ++i) {
						if (table[i]->Type == b->Data.Distance.RestrictType) {
							if ((b->Data.Distance.DistanceType == GreaterThan ||
								b->Data.Distance.DistanceType == GreaterThanEqual) &&
								MapDistanceBetweenTypes(type, x, y, 
									table[i]->Type, table[i]->X, table[i]->Y) <= distance) {
								// True on failure
								success = 0;
							} else if ((b->Data.Distance.DistanceType == LessThanEqual ||
								b->Data.Distance.DistanceType == LessThan) &&
								MapDistanceBetweenTypes(type, x, y,
									table[i]->Type, table[i]->X, table[i]->Y) <= distance) {
								success = 1;
								break;
							} else if (b->Data.Distance.DistanceType == Equal &&
								MapDistanceBetweenTypes(type, x, y,
									table[i]->Type, table[i]->X, table[i]->Y) == distance) {
								success = 1;
								break;
							} else if (b->Data.Distance.DistanceType == NotEqual &&
								MapDistanceBetweenTypes(type, x, y,
									table[i]->Type, table[i]->X, table[i]->Y) == distance) {
								success = 0;
							}
						}
					}
				}
				// Check AddOn Restriction
				if (b->RestrictType == RestrictAddOn) {
					success = 0;
					x1 = x - b->Data.AddOn.OffsetX < 0 ? -1 : x - b->Data.AddOn.OffsetX;
					x1 = x1 >= TheMap.Width ? -1 : x1;
					y1 = y - b->Data.AddOn.OffsetY < 0 ? -1 : y - b->Data.AddOn.OffsetY;
					y1 = y1 >= TheMap.Height ? -1 : y1;
					if (!(x1 == -1 || y1 == -1)) {
						n = UnitCacheOnTile(x1, y1, table);
						for (i = 0; i < n; ++i) {
							if (table[i]->Type == b->Data.AddOn.Parent &&
								table[i]->X == x1 && table[i]->Y == y1) {
								success = 1;
								break;
							}
						}
					}
				}
				// Check Build On Top
				if (b->RestrictType == RestrictOnTop) {
					success = 0;
					n = UnitCacheOnTile(x, y, table);
					for (i = 0; i < n; ++i) {
						if (table[i]->Type == b->Data.OnTop.Parent &&
							table[i]->X == x && table[i]->Y == y &&
							table[i]->Orders[0].Action != UnitActionBuilded &&
							!table[i]->Destroyed &&
							table[i]->Orders[0].Action != UnitActionDie) {
							success = 1;
							ontoptarget = table[i];
						}
					}
				}
				// All checks processed, did we really have success
				if (success) {
					b = b->Next;
				} else {
					break;
				}
			}
			if (b == NULL) {
				// We passed a full ruleset return
				if (unit == NULL) {
					return ontoptarget ? ontoptarget : (Unit*)1;
				} else {
					return ontoptarget ? ontoptarget : (Unit*)unit;
				}
			}
			++w;
		}
		return NULL;
	}

	if (unit == NULL) {
		return (Unit*)1;
	} else {
		return (Unit*)unit;
	}
}

/**
**  Can build on this point.
*/
int CanBuildOn(int x, int y, int mask)
{
	if (x < 0 || y < 0 || x >= TheMap.Width || y >= TheMap.Height) {
		return 0;
	}
	return (TheMap.Fields[x + y * TheMap.Width].Flags & mask) ? 0 : 1;
}

/**
**  Can build unit-type on this point.
**
**  @param unit  Worker that want to build the building or NULL.
**  @param type  Building unit-type.
**  @param x     X tile map position.
**  @param y     Y tile map position.
**  @param real  Really build, or just placement
**
**  @return      OnTop, parent unit, builder on true, NULL false.
**
*/
Unit* CanBuildUnitType(const Unit* unit, const UnitType* type, int x, int y, int real)
{
	int w;
	int h;
	int j;
	int testmask;
	Player* player;
	Unit* ontop;

	// Terrain Flags don't matter if building on top of a unit.
	ontop = CanBuildHere(unit, type, x, y);
	if (unit != NULL && ontop != unit) {
		return ontop;
	}

	//
	//  Remove unit that is building!
	//
	j = 0;
	if (unit) {
		j = unit->Type->FieldFlags;
		for (h = unit->Type->TileHeight; h > 0; --h) {
			for (w = unit->Type->TileWidth; w > 0; --w) {
				TheMap.Fields[(unit->X + w - 1) + 
						(unit->Y - 1 + h) * TheMap.Width].Flags &= ~j;
			}
		}
	}

	player = NULL;

	if (unit && unit->Player->Type == PlayerPerson) {
		player = unit->Player;
	}

	for (h = type->TileHeight; h--;) {
		for (w = type->TileWidth; w--;) {
			if (player && !real) {
				testmask = MapFogFilterFlags(player, x + w, y + h, type->MovementMask);
			} else {
				testmask = type->MovementMask;
			}
			if (!CanBuildOn(x + w, y + h, testmask)) {
				if (unit) {
					TheMap.Fields[unit->X + unit->Y * TheMap.Width].Flags |= j;
				}
				return NULL;
			}
			if (player && !IsMapFieldExplored(player, x + w, y + h)) {
				return NULL;
			}
		}
	}
	if (unit) {
		j = unit->Type->FieldFlags;
		for (h = unit->Type->TileHeight; h > 0; --h) {
			for (w = unit->Type->TileWidth; w > 0; --w) {
				TheMap.Fields[(unit->X + w - 1) + 
						(unit->Y - 1 + h) * TheMap.Width].Flags |= j;
			}
		}
	}

	//
	// We can build here: check distance to gold mine/oil patch!
	//
	return CanBuildHere(unit, type, x, y);
}

/*----------------------------------------------------------------------------
  -- Finding units
  ----------------------------------------------------------------------------*/

/**
** Find the closest piece of wood for an unit.
**
** @param unit    The unit.
** @param x       OUT: Map X position of tile.
** @param y       OUT: Map Y position of tile.
*/
int FindWoodInSight(const Unit* unit, int* x, int* y)
{
	return FindTerrainType(UnitMovementMask(unit), 0, MapFieldForest, 9999,
		unit->Player, unit->X, unit->Y, x, y);
}

/**
** Find the closest piece of terrain with the given flags.
**
** @param movemask    The movement mask to reach that location.
** @param resmask     Result tile mask.
** @param rvresult    Return a tile that doesn't match.
** @param range       Maximum distance for the search.
** @param player      Only search fields explored by player
** @param x           Map X start position for the search.
** @param y           Map Y start position for the search.
**
** @param px          OUT: Map X position of tile.
** @param py          OUT: Map Y position of tile.
**
** @note Movement mask can be 0xFFFFFFFF to have no effect
** Range is not circular, but square.
** Player is ignored if nil(search the entire map)
** Use rvresult if you search for a til;e that doesn't
** match resmask. Like for a tile where an unit can go
** with it's movement mask.
**
** @return            True if wood was found.
*/
int FindTerrainType(int movemask, int resmask, int rvresult, int range,
	const Player* player, int x, int y, int* px, int* py)
{
	static const int xoffset[] = {  0,-1,+1, 0, -1,+1,-1,+1 };
	static const int yoffset[] = { -1, 0, 0,+1, -1,-1,+1,+1 };
	struct {
		unsigned short X;
		unsigned short Y;
	} *points;
	int size;
	int rx;
	int ry;
	int wp;
	int rp;
	int ep;
	int i;
	int w;
	unsigned char* m;
	unsigned char* matrix;
	int destx;
	int desty;
	int cdist;

	destx = x;
	desty = y;
	size = (TheMap.Width * TheMap.Height / 4 < range * range * 5) ?
		TheMap.Width * TheMap.Height / 4 : range * range * 5;
	points = malloc(size * sizeof(*points));

	// Make movement matrix. FIXME: can create smaller matrix.
	matrix = CreateMatrix();
	w = TheMap.Width + 2;
	matrix += w + w + 2;
	points[0].X = x;
	points[0].Y = y;
	rp = 0;
	matrix[x + y * w] = 1; // mark start point
	ep = wp = 1; // start with one point
	cdist = 0; // current distance is 0

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
				//  Make sure we don't leave the map.
				if (x < 0 || y < 0 || x >= TheMap.Width || y >= TheMap.Height) {
					continue;
				}
				m = matrix + x + y * w;
				//  Check if visited or unexplored
				if (*m || (player && !IsMapFieldExplored(player, x, y))) {
					continue;
				}
				// Look if found what was required.
				if (rvresult ? CanMoveToMask(x, y, resmask) : !CanMoveToMask(x, y, resmask)) {
					*px = x;
					*py = y;
					free(points);
					return 1;
				}
				if (CanMoveToMask(x, y, movemask)) { // reachable
					*m = 1;
					points[wp].X = x; // push the point
					points[wp].Y = y;
					if (++wp >= size) { // round about
						wp = 0;
					}
					if (wp == ep) {
						//  We are out of points, give up!
						DebugPrint("Ran out of points the hard way, beware.\n");
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
		++cdist;
		if (rp == wp || cdist >= range) { // unreachable, no more points available
			break;
		}
		// Continue with next set.
		ep = wp;
	}
	free(points);
	return 0;
}

/**
** Find Resource.
**
** @param unit        The unit that wants to find a resource.
** @param x           Closest to x
** @param y           Closest to y
** @param range       Maximum distance to the resource.
** @param resource    The resource id.
**
** @note This will return an usable resource building that
** belongs to "player" or is neutral.
**
** @return            NoUnitP or resource unit
*/
Unit* FindResource(const Unit* unit, int x, int y, int range, int resource)
{
	static const int xoffset[] = {  0,-1,+1, 0, -1,+1,-1,+1 };
	static const int yoffset[] = { -1, 0, 0,+1, -1,-1,+1,+1 };
	struct {
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
	int n;
	unsigned char* m;
	unsigned char* matrix;
	const Unit* destu;
	Unit* mine;
	Unit* bestmine;
	int destx;
	int desty;
	int bestd;
	int cdist;

	destx = x;
	desty = y;
	size = (TheMap.Width * TheMap.Height / 4 < range * range * 5) ?
		TheMap.Width * TheMap.Height / 4 : range * range * 5;
	points = malloc(size * sizeof(*points));

	// Find the nearest gold depot
	if ((destu = FindDeposit(unit, x, y,range,resource))) {
		NearestOfUnit(destu, x, y, &destx, &desty);
	}
	bestd = 99999;
	// Make movement matrix. FIXME: can create smaller matrix.
	matrix = CreateMatrix();
	w = TheMap.Width + 2;
	matrix += w + w + 2;
	//  Unit movement mask
	mask = UnitMovementMask(unit);
	//  Ignore all units along the way. Might seem wierd, but otherwise
	//  peasants would lock at a mine with a lot of workers.
	mask &= ~(MapFieldLandUnit | MapFieldSeaUnit | MapFieldAirUnit);
	points[0].X = x;
	points[0].Y = y;
	rp = 0;
	matrix[x + y * w] = 1; // mark start point
	ep = wp = 1; // start with one point
	cdist = 0; // current distance is 0
	bestmine = NoUnitP;

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

				if (!IsMapFieldExplored(unit->Player, x, y)) { // Unknown.
					continue;
				}

				//
				// Look if there is a mine
				//
				if ((mine = ResourceOnMap(x, y, resource)) &&
						mine->Type->CanHarvest &&
						(mine->Player->Player == PlayerMax - 1 ||
							mine->Player == unit->Player ||
							IsAllied(unit->Player, mine))) {
					if (destu) {
						n = (abs(destx - x) > abs(desty - y)) ? abs(destx - x) : abs(desty - y);
						if (n < bestd) {
							bestd = n;
							bestmine = mine;
						}
						*m = 99;
					} else { // no goal take the first
						free(points);
						return mine;
					}
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
			free(points);
			return bestmine;
		}
		++cdist;
		if (rp == wp || cdist >= range) { // unreachable, no more points available
			break;
		}
		// Continue with next set.
		ep = wp;
	}
	free(points);
	return NoUnitP;
}

/**
** Find deposit. This will find a deposit for a resource
**
** @param unit        The unit that wants to find a resource.
** @param x           Closest to x
** @param y           Closest to y
** @param range       Maximum distance to the deposit.
** @param resource    Resource to find deposit from.
**
** @note This will return a reachable allied depot.
**
** @return            NoUnitP or deposit unit
*/
Unit* FindDeposit(const Unit* unit, int x, int y, int range, int resource)
{
	static const int xoffset[] = {  0,-1,+1, 0, -1,+1,-1,+1 };
	static const int yoffset[] = { -1, 0, 0,+1, -1,-1,+1,+1 };
	struct {
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
	int nodes_searched;
	unsigned char* m;
	unsigned char* matrix;
	Unit* depot;
	int destx;
	int desty;
	int cdist;

	nodes_searched = 0;

	destx = x;
	desty = y;
	size = (TheMap.Width * TheMap.Height / 4 < range * range * 5) ?
		TheMap.Width * TheMap.Height / 4 : range * range * 5;
	points = malloc(size * sizeof(*points));

	// Make movement matrix. FIXME: can create smaller matrix.
	matrix = CreateMatrix();
	w = TheMap.Width + 2;
	matrix += w + w + 2;
	//  Unit movement mask
	mask = UnitMovementMask(unit);
	//  Ignore all units along the way. Might seem wierd, but otherwise
	//  peasants would lock at a mine with a lot of workers.
	mask &= ~(MapFieldLandUnit | MapFieldSeaUnit | MapFieldAirUnit | MapFieldBuilding);
	points[0].X = x;
	points[0].Y = y;
	rp = 0;
	matrix[x + y * w] = 1; // mark start point
	ep = wp = 1; // start with one point
	cdist = 0; // current distance is 0

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
				++nodes_searched;
				//  Make sure we don't leave the map.
				if (x < 0 || y < 0 || x >= TheMap.Width || y >= TheMap.Height) {
					continue;
				}
				m = matrix + x + y * w;
				//  Check if visited or unexplored
				if (*m || !IsMapFieldExplored(unit->Player, x, y)) {
					continue;
				}
				//
				// Look if there is a deposit
				//
				if ((depot = ResourceDepositOnMap(x, y, resource)) &&
						((IsAllied(unit->Player, depot)) ||
							(unit->Player == depot->Player))) {
					free(points);
					return depot;
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
						DebugPrint("Ran out of points the hard way, beware.\n");
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
		++cdist;
		if (rp == wp || cdist >= range) { // unreachable, no more points available
			break;
		}
		// Continue with next set.
		ep = wp;
	}
	free(points);
	return NoUnitP;
}

/**
** Find the next idle worker
**
** @param player    Player's units to search through
** @param last      Previous idle worker selected
**
** @return NoUnitP or next idle worker
*/
Unit* FindIdleWorker(const Player* player, const Unit* last)
{
	Unit* unit;
	Unit** units;
	Unit* FirstUnitFound;
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
	units = player->Units;

	for (i = 0; i < nunits; ++i) {
		unit = units[i];
		if (unit->Type->Harvester && unit->Type->ResInfo && !unit->Removed) {
			if (unit->Orders[0].Action == UnitActionStill) {
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
**  Unit on map screen.
**
**  Select units on screen. (x, y are in pixels relative to map 0,0).
**  Not GAMEPLAY safe, uses ReplayRevealMap
**
**  More units on same position.
**    Cycle through units. ounit is the old one.
**    First take highest unit.
**
**  @todo FIXME: If no unit, we could select near units?
**
**  @param ounit  Old selected unit.
**  @param x      X pixel position.
**  @param y      Y pixel position.
**
**  @return       An unit on x, y position.
*/
Unit* UnitOnScreen(Unit* ounit, int x, int y)
{
	Unit** table;
	Unit* unit;
	Unit* nunit;
	Unit* funit; // first possible unit
	UnitType* type;
	int flag; // flag take next unit
	int gx;
	int gy;

	funit = NULL;
	nunit = NULL;
	flag = 0;
	if (!ounit) { // no old on this position
		flag = 1;
	}
	for (table = Units; table < Units + NumUnits; ++table) {
		unit = *table;
		if (!UnitVisibleAsGoal(unit, ThisPlayer) && !ReplayRevealMap) {
			continue;
		}
		type = unit->Type;

		//
		// Check if mouse is over the unit.
		//
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

		//
		// This could be taken.
		//
		if (flag) {
			return unit;
		}
		if (unit == ounit) {
			flag = 1;
		} else if (!funit) {
			funit = unit;
		}
		nunit=unit;
	}

	if (flag && funit) {
		return funit;
	}
	return nunit;
}

/**
** Let an unit die.
**
** @param unit    Unit to be destroyed.
*/
void LetUnitDie(Unit* unit)
{
	UnitType* type;

	unit->HP = 0;
	unit->Moving = 0;
	unit->TTL = 0;

	type = unit->Type;

	// removed units,  just remove.
	if (unit->Removed) {
		DebugPrint("Killing a removed unit?\n");
		UnitLost(unit);
		UnitClearOrders(unit);
		ReleaseUnit(unit);
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
		RemoveUnit(unit->Goal, NULL);
		UnitLost(unit->Goal);
		UnitClearOrders(unit->Goal);
		ReleaseUnit(unit->Goal);
		unit->Goal = NULL;
	}

	// During resource build, the worker holds the resource amount,
	// but if canceling building the platform, the worker is already
	// outside.
	if (type->GivesResource &&
			unit->Orders[0].Action == UnitActionBuilded &&
			unit->Data.Builded.Worker) {
		// Restore value for oil-patch
		unit->ResourcesHeld = unit->Data.Builded.Worker->ResourcesHeld;
	}

	// Transporters lose their units and building their workers
	if (unit->UnitInside) {
		// FIXME: destroy or unload : do a flag.
		DestroyAllInside(unit);
	}
	RemoveUnit(unit, NULL);
	UnitLost(unit);
	UnitClearOrders(unit);

	//
	// Unit has death animation.
	//

	// Not good: UnitUpdateHeading(unit);
	unit->SubAction = 0;
	unit->State = 0;
	unit->Reset = 0;
	unit->Wait = 1;
	unit->Orders[0].Action = UnitActionDie;
	if (type->CorpseType) {
#ifdef DYNAMIC_LOAD
		if (!type->Sprite) {
			LoadUnitTypeSprite(type);
		}
#endif
		unit->IX = (type->CorpseType->Width - VideoGraphicWidth(type->CorpseType->Sprite)) / 2;
		unit->IY = (type->CorpseType->Height - VideoGraphicHeight(type->CorpseType->Sprite)) / 2;

		unit->CurrentSightRange = type->CorpseType->Stats[unit->Player->Player].SightRange;
	} else {
		unit->CurrentSightRange = 0;
	}
	MapMarkUnitSight(unit);

	if (type->CorpseType || (type->Animations && type->Animations->Die)) {
		unit->Removed = 0;
		UnitCacheInsert(unit);
	}
}

/**
** Destroy all units inside unit.
*/
void DestroyAllInside(Unit* source)
{
	Unit* unit;
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
		ReleaseUnit(unit);
	}
}


/*----------------------------------------------------------------------------
  -- Unit AI
  ----------------------------------------------------------------------------*/

/**
** Unit is hit by missile or other damage.
**
** @param attacker    Unit that attacks.
** @param target      Unit that is hit.
** @param damage      How many damage to take.
*/
void HitUnit(Unit* attacker, Unit* target, int damage)
{
	UnitType* type;
	Unit* goal;
	unsigned long lastattack;

	if (!damage) { // Can now happen by splash damage
#ifdef DEBUG
		if (!GodMode) {
			DebugPrint("Warning no damage, try to fix by caller?\n");
		}
#endif
		return;
	}

	Assert(damage != 0 && target->HP != 0 && !target->Type->Vanishes);

	if (target->UnholyArmor > 0 || target->Type->Decoration) {
		// vladi: units with active UnholyArmour are invulnerable
		// mr-russ: as are decorations
		return;
	}

	if (target->Removed) {
		DebugPrint("Removed target hit\n");
		return;
	}

	if (GodMode) {
		if (attacker && attacker->Player == ThisPlayer) {
			damage = target->HP;
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
		NotifyPlayer(target->Player, NotifyRed, target->X, target->Y,
			"%s attacked", target->Type->Name);
		if (target->Player->AiEnabled) {
			AiHelpMe(attacker, target);
		}
	}

	if (target->HP <= damage) { // unit is killed or destroyed
		//  increase scores of the attacker, but not if attacking it's own units.
		//  prevents cheating by killing your own units.
		if (attacker && IsEnemy(target->Player, attacker)) {
			attacker->Player->Score += target->Type->Points;
			if (type->Building) {
				attacker->Player->TotalRazings++;
			} else {
				attacker->Player->TotalKills++;
			}
			if (UseHPForXp) {
				attacker->XP += target->HP;
			} else {
				attacker->XP += target->Type->Points;
			}
			attacker->Kills++;
		}
		LetUnitDie(target);
		return;
	}
	target->HP -= damage;
	if (UseHPForXp && attacker && IsEnemy(target->Player, attacker)) {
		attacker->XP += damage;
	}

	// FIXME: this is dumb. I made repairers capture. crap.
	// david: capture enemy buildings
	// Only worker types can capture.
	// Still possible to destroy building if not careful (too many attackers)
	if (EnableBuildingCapture && attacker &&
			type->Building && target->HP <= damage * 3 &&
			IsEnemy(attacker->Player, target) &&
			attacker->Type->RepairRange) {
		ChangeUnitOwner(target, attacker->Player);
		CommandStopUnit(attacker); // Attacker shouldn't continue attack!
	}

	if ((UnitVisibleOnMap(target, ThisPlayer) || ReplayRevealMap) && DamageMissile) {
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
		Missile* missile;
		MissileType* fire;

		f = (100 * target->HP) / target->Stats->HitPoints;
		fire = MissileBurningBuilding(f);
		if (fire) {
			missile = MakeMissile(fire,
				target->X * TileSizeX + (type->TileWidth * TileSizeX) / 2,
				target->Y * TileSizeY + (type->TileHeight * TileSizeY) / 2 - TileSizeY,
				0, 0);
			missile->SourceUnit = target;
			target->Burning = 1;
			RefsIncrease(target);
		}
	}

	//
	// Unit is working?
	//
	if (target->Orders[0].Action != UnitActionStill) {
		return;
	}

	//
	// Attack units in range (which or the attacker?)
	//
	if (attacker && !type->Coward) {
		if (type->CanAttack) {
			if (RevealAttacker && CanTarget(target->Type, attacker->Type)) {
				// Reveal Unit that is attacking
				goal = attacker;
			} else {
				goal = AttackUnitsInReactRange(target);
			}
			if (goal) {
				if (target->SavedOrder.Action == UnitActionStill) {
					// FIXME: should rewrite command handling
					CommandAttack(target, target->X, target->Y, NoUnitP,
						FlushCommands);
					target->SavedOrder = target->Orders[1];
				}
				CommandAttack(target, goal->X, goal->Y, NoUnitP, FlushCommands);
				return;
			}
		}
	}

	//
	// FIXME: Can't attack run away.
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
		} else if (x >= TheMap.Width) {
			x = TheMap.Width - 1;
		}
		y = target->Y + (y * 5) / d + (SyncRand() & 3);
		if (y < 0) {
			y = 0;
		} else if (y >= TheMap.Height) {
			y = TheMap.Height - 1;
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
int MapDistanceToType(int x1, int y1, const UnitType* type, int x2, int y2)
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
int MapDistanceToUnit(int x, int y, const Unit* dest)
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
int MapDistanceBetweenUnits(const Unit* src, const Unit* dst)
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
int MapDistanceBetweenTypes(const UnitType* src, int x1, int y1, const UnitType* dst, int x2, int y2)
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
	const Viewport *vp;

	// first compute the view point coordinate
	vp = TheUI.SelectedViewport;

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
int ViewPointDistanceToUnit(const Unit* dest)
{
	const Viewport* vp;

	// first compute the view point coordinate
	vp = TheUI.SelectedViewport;
	// then use MapDistanceToUnit
	return MapDistanceToUnit(vp->MapX + vp->MapWidth / 2,
		vp->MapY + vp->MapHeight / 2, dest);
}

/**
**  Can the source unit attack the destination unit.
**
**  @param source    Unit type pointer of the attacker.
**  @param dest      Unit type pointer of the target.
*/
int CanTarget(const UnitType* source, const UnitType* dest)
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
int CanTransport(const Unit* transporter, const Unit* unit)
{
	int i;

	if (!transporter->Type->CanTransport) {
		return 0;
	}
	if (transporter->BoardCount >= transporter->Type->MaxOnBoard) { // full
		return 0;
	}
	// FIXME: remove UnitTypeLand requirement
	if (PlayersTeamed(transporter->Player->Player, unit->Player->Player) &&
			unit->Type->UnitType != UnitTypeLand) {
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

/*----------------------------------------------------------------------------
--  SAVE/LOAD
----------------------------------------------------------------------------*/

/**
**  Generate a unit reference, a printable unique string for unit.
*/
char* UnitReference(const Unit* unit)
{
	char* ref;

	ref = malloc(10);
	sprintf(ref, "U%04X", UnitNumber(unit));
	return ref;
}

/**
**  Save an order.
**
**  @param order    Order who should be saved.
**  @param file     Output file.
*/
void SaveOrder(const Order* order, CLFile* file)
{
	char* ref;

	CLprintf(file, "{");
	switch (order->Action) {
		case UnitActionNone:
			CLprintf(file, "\"action-none\",");
			break;

		case UnitActionStill:
			CLprintf(file, "\"action-still\",");
			break;
		case UnitActionStandGround:
			CLprintf(file, "\"action-stand-ground\",");
			break;
		case UnitActionFollow:
			CLprintf(file, "\"action-follow\",");
			break;
		case UnitActionMove:
			CLprintf(file, "\"action-move\",");
			break;
		case UnitActionAttack:
			CLprintf(file, "\"action-attack\",");
			break;
		case UnitActionAttackGround:
			CLprintf(file, "\"action-attack-ground\",");
			break;
		case UnitActionDie:
			CLprintf(file, "\"action-die\",");
			break;

		case UnitActionSpellCast:
			CLprintf(file, "\"action-spell-cast\",");
			break;

		case UnitActionTrain:
			CLprintf(file, "\"action-train\",");
			break;
		case UnitActionUpgradeTo:
			CLprintf(file, "\"action-upgrade-to\",");
			break;
		case UnitActionResearch:
			CLprintf(file, "\"action-research\",");
			break;
		case UnitActionBuilded:
			CLprintf(file, "\"action-builded\",");
			break;

		case UnitActionBoard:
			CLprintf(file, "\"action-board\",");
			break;
		case UnitActionUnload:
			CLprintf(file, "\"action-unload\",");
			break;
		case UnitActionPatrol:
			CLprintf(file, "\"action-patrol\",");
			break;
		case UnitActionBuild:
			CLprintf(file, "\"action-build\",");
			break;

		case UnitActionRepair:
			CLprintf(file, "\"action-repair\",");
			break;
		case UnitActionResource:
			CLprintf(file, "\"action-resource\",");
			break;
		case UnitActionReturnGoods:
			CLprintf(file, "\"action-return-goods\",");
			break;

		default:
			DebugPrint("Unknown action in order\n");
	}
	CLprintf(file, " \"flags\", %d,", order->Flags);
	CLprintf(file, " \"range\", %d,", order->Range);
	CLprintf(file, " \"width\", %d,", order->Width);
	CLprintf(file, " \"height\", %d,", order->Height);
	CLprintf(file, " \"min-range\", %d,", order->MinRange);
	if (order->Goal) {
		if (order->Goal->Destroyed) {
			/* this unit is destroyed so it's not in the global unit
			 * array - this means it won't be saved!!! */
			printf ("FIXME: storing destroyed Goal - loading will fail.\n");
		}
		CLprintf(file, " \"goal\", \"%s\",", ref = UnitReference(order->Goal));
		free(ref);
	}
	CLprintf(file, " \"tile\", {%d, %d},", order->X, order->Y);
	if (order->Type) {
		CLprintf(file, " \"type\", \"%s\",", order->Type->Ident);
	}
	if (order->Arg1) {
		// patrol=pos, research=upgrade, spell=spell
		switch (order->Action) {
			case UnitActionPatrol:
				CLprintf(file, " \"patrol\", {%d, %d},",
					(int)order->Arg1 >> 16, (int)order->Arg1 & 0xFFFF);
				break;
			case UnitActionSpellCast:
				CLprintf(file, " \"spell\", \"%s\",", ((SpellType*)order->Arg1)->Ident);
				break;
			case UnitActionResearch:
				CLprintf(file, " \"upgrade\", \"%s\",", ((Upgrade*)order->Arg1)->Ident);
				break;
			default:
				CLprintf(file, " \"arg1\", %d,", (int)order->Arg1);
				break;
		}
	}
	CLprintf(file, "}");
}

/**
**  Save the state of an unit to file.
**
**  @param unit    Unit pointer to be saved.
**  @param file    Output file.
*/
void SaveUnit(const Unit* unit, CLFile* file)
{
	char* ref;
	Unit* uins;
	int i;

	CLprintf(file, "\nUnit(%d, ", UnitNumber(unit));

	// 'type and 'player must be first, needed to create the unit slot
	CLprintf(file, "\"type\", \"%s\", ", unit->Type->Ident);
	if (unit->Seen.Type) {
		CLprintf(file, "\"seen-type\", \"%s\", ", unit->Seen.Type->Ident);
	}

	CLprintf(file, "\"player\", %d,\n  ", unit->Player->Player);

	if (unit->Next) {
		CLprintf(file, "\"next\", %d, ", UnitNumber(unit->Next));
	}

	CLprintf(file, "\"tile\", {%d, %d}, ", unit->X, unit->Y);
	CLprintf(file, "\"refs\", %d, ", unit->Refs);
#if 0
	// latimerius: why is this so complex?
	// JOHNS: An unit can be owned by a new player and have still the old stats
	for (i = 0; i < PlayerMax; ++i) {
		if (&unit->Type->Stats[i] == unit->Stats) {
			CLprintf(file, "\"stats\", %d,\n  ", i);
			break;
		}
	}
	// latimerius: what's the point of storing a pointer value anyway?
	if (i == PlayerMax) {
		CLprintf(file, "\"stats\", \"S%08X\",\n  ", (int)unit->Stats);
	}
#else
	CLprintf(file, "\"stats\", %d,\n  ", unit->Player->Player);
#endif
	CLprintf(file, "\"pixel\", {%d, %d}, ", unit->IX, unit->IY);
	CLprintf(file, "\"seen-pixel\", {%d, %d}, ", unit->Seen.IX, unit->Seen.IY);
	CLprintf(file, "\"frame\", %d, ", unit->Frame);
	if (unit->Seen.Frame != UnitNotSeen) {
		CLprintf(file, "\"seen\", %d, ", unit->Seen.Frame);
	} else {
		CLprintf(file, "\"not-seen\", ");
	}
	CLprintf(file, "\"direction\", %d,\n  ", unit->Direction);
	CLprintf(file, "\"attacked\", %lu,\n ", unit->Attacked);
	CLprintf(file, " \"current-sight-range\", %d,", unit->CurrentSightRange);
	if (unit->Burning) {
		CLprintf(file, " \"burning\",");
	}
	if (unit->Destroyed) {
		CLprintf(file, " \"destroyed\",");
	}
	if (unit->Removed) {
		CLprintf(file, " \"removed\",");
	}
	if (unit->Selected) {
		CLprintf(file, " \"selected\",");
	}
	if (unit->RescuedFrom) {
		CLprintf(file, " \"rescued-from\", %d,", unit->RescuedFrom->Player);
	}
	// n0b0dy: How is this usefull?
	// mr-russ: You can't always load units in order, it saved the information
	// so you can load a unit who's Container hasn't been loaded yet.
	// SEE unit loading code.
	if (unit->Container && unit->Removed) {
		CLprintf(file, " \"host-info\", {%d, %d, %d, %d}, ",
			unit->Container->X, unit->Container->Y,
			unit->Container->Type->TileWidth,
			unit->Container->Type->TileHeight);
	}
	CLprintf(file, " \"seen-by-player\", \"");
	for (i = 0; i < PlayerMax; ++i) {
		CLprintf(file, "%c", (unit->Seen.ByPlayer & (1 << i)) ? 'X' : '_');
	}
	CLprintf(file, "\",\n ");
	CLprintf(file, " \"seen-destroyed\", \"");
	for (i = 0; i < PlayerMax; ++i) {
		CLprintf(file, "%c", (unit->Seen.Destroyed & (1 << i)) ? 'X' : '_');
	}
	CLprintf(file, "\",\n ");
	if (unit->Constructed) {
		CLprintf(file, " \"constructed\",");
	}
	if (unit->Seen.Constructed) {
		CLprintf(file, " \"seen-constructed\",");
	}
	CLprintf(file, " \"seen-state\", %d, ", unit->Seen.State);
	if (unit->Active) {
		CLprintf(file, " \"active\",");
	}
	CLprintf(file, " \"mana\", %d,", unit->Mana);
	CLprintf(file, " \"hp\", %d,", unit->HP);
	CLprintf(file, " \"xp\", %d,", unit->XP);
	CLprintf(file, " \"kills\", %d,\n  ", unit->Kills);

	CLprintf(file, "\"ttl\", %lu, ", unit->TTL);
	CLprintf(file, "\"bloodlust\", %d, ", unit->Bloodlust);
	CLprintf(file, "\"haste\", %d, ", unit->Haste);
	CLprintf(file, "\"slow\", %d,\n  ", unit->Slow);
	CLprintf(file, "\"invisible\", %d, ", unit->Invisible);
	CLprintf(file, "\"flame-shield\", %d, ", unit->FlameShield);
	CLprintf(file, "\"unholy-armor\", %d,\n  ", unit->UnholyArmor);

	for (i = 0; i < UnitTypeVar.NumberVariable; i++) {
			CLprintf(file, "\"%s\", {Value = %d, Max = %d, Increase = %d, Enable = %s},\n  ",
				UnitTypeVar.VariableName[i], unit->Variable[i].Value, unit->Variable[i].Max,
				unit->Variable[i].Increase, unit->Variable[i].Enable ? "true" : "false");
	}

	CLprintf(file, "\"group-id\", %d,\n  ", unit->GroupId);
	CLprintf(file, "\"last-group\", %d,\n  ", unit->LastGroup);

	CLprintf(file, "\"resources-held\", %d,\n  ", unit->ResourcesHeld);
	if (unit->CurrentResource) {
		CLprintf(file, "\"current-resource\", \"%s\",\n  ",
			DefaultResourceNames[unit->CurrentResource]);
	}

	CLprintf(file, "\"sub-action\", %d, ", unit->SubAction);
	CLprintf(file, "\"wait\", %d, ", unit->Wait);
	CLprintf(file, "\"state\", %d,", unit->State);
	if (unit->Reset) {
		CLprintf(file, " \"reset\",");
	}
	CLprintf(file, "\n  \"blink\", %d,", unit->Blink);
	if (unit->Moving) {
		CLprintf(file, " \"moving\",");
	}
	if (unit->ReCast) {
		CLprintf(file, " \"re-cast\",");
	}
	if (unit->Boarded) {
		CLprintf(file, " \"boarded\",");
	}
	CLprintf(file, " \"rs\", %d,", unit->Rs);
	CLprintf(file, " \"units-boarded-count\", %d,", unit->BoardCount);

	if (unit->UnitInside) {
		CLprintf(file, "\n  \"units-contained\", {");
		uins = unit->UnitInside->PrevContained;
		for (i = unit->InsideCount; i; --i, uins = uins->PrevContained) {
			CLprintf(file, "\"%s\"", ref = UnitReference(uins));
			free(ref);
			if (i > 1) {
				CLprintf(file, ", ");
			}
		}
		CLprintf(file, "},\n  ");
	}
	CLprintf(file, "\"order-count\", %d,\n  ", unit->OrderCount);
	CLprintf(file, "\"order-flush\", %d,\n  ", unit->OrderFlush);
	CLprintf(file, "\"order-total\", %d,\n	", unit->TotalOrders);
	CLprintf(file, "\"orders\", {");
	for (i = 0; i < unit->TotalOrders; ++i) {
		CLprintf(file, "\n ");
		SaveOrder(&unit->Orders[i], file);
		CLprintf(file, ",");
	}
	CLprintf(file, "},\n  \"saved-order\", ");
	SaveOrder(&unit->SavedOrder, file);
	CLprintf(file, ",\n  \"new-order\", ");
	SaveOrder(&unit->NewOrder, file);

	//
	//  Order data part
	//
	switch (unit->Orders[0].Action) {
		case UnitActionStill:
			// FIXME: support other resource types
			if (unit->Type->GivesResource) {
				CLprintf(file, ", \"resource-active\", %d", unit->Data.Resource.Active);
			}
			break;
		case UnitActionResource:
			CLprintf(file, ", \"data-res-worker\", {\"time-to-harvest\", %d,", unit->Data.ResWorker.TimeToHarvest);
			if (unit->Data.ResWorker.DoneHarvesting) {
				CLprintf(file, " \"done-harvesting\",");
			}
			CLprintf(file, "}");
			break;
		case UnitActionBuilded:
			{
				ConstructionFrame* cframe;
				int frame;

				cframe = unit->Type->Construction->Frames;
				frame = 0;
				while (cframe != unit->Data.Builded.Frame) {
					cframe = cframe->Next;
					++frame;
				}
				CLprintf(file, ",\n  \"data-builded\", {");

				if (unit->Data.Builded.Worker) {
					CLprintf(file, "\"worker\", \"%s\", ",
					ref = UnitReference(unit->Data.Builded.Worker));
					free(ref);
				}
				CLprintf(file, "\"progress\", %d, \"frame\", %d,",
					unit->Data.Builded.Progress, frame);
				if (unit->Data.Builded.Cancel) {
					CLprintf(file, " \"cancel\",");
				}
				CLprintf(file, "}");
				break;
			}
		case UnitActionResearch:
			CLprintf(file, ",\n  \"data-research\", {");
			CLprintf(file, "\"ident\", \"%s\",", unit->Data.Research.Upgrade->Ident);
			CLprintf(file, "}");
			break;
		case UnitActionUpgradeTo:
			CLprintf(file, ",\n  \"data-upgrade-to\", {");
			CLprintf(file, "\"ticks\", %d,", unit->Data.UpgradeTo.Ticks);
			CLprintf(file, "}");
			break;
		case UnitActionTrain:
			CLprintf(file, ",\n  \"data-train\", {");
			CLprintf(file, "\"ticks\", %d, ", unit->Data.Train.Ticks);
			CLprintf(file, "}");
			break;
		default:
			CLprintf(file, ",\n  \"data-move\", {");
			if (unit->Data.Move.Fast) {
				CLprintf(file, "\"fast\", ");
			}
			if (unit->Data.Move.Length > 0) {
				CLprintf(file, "\"path\", {");
				for (i = 0; i < unit->Data.Move.Length; ++i) {
					CLprintf(file, "%d, ", unit->Data.Move.Path[i]);
				}
				CLprintf(file, "},");
			}
			CLprintf(file, "}");
			break;
	}

	if (unit->Goal) {
		CLprintf(file, ",\n  \"goal\", %d", UnitNumber(unit->Goal));
	}
	if (unit->AutoCastSpell) {
		for (i = 0; i < SpellTypeCount; ++i) {
			if (unit->AutoCastSpell[i]) {
				CLprintf(file, ",\n  \"auto-cast\", \"%s\"", SpellTypeTable[i]->Ident);
			}
		}
	}

	CLprintf(file, ")\n");
}

/**
**  Save state of units to file.
**
**  @param file    Output file.
*/
void SaveUnits(CLFile* file)
{
	Unit** table;
	Unit* unit;
	int i;
	unsigned char SlotUsage[MAX_UNIT_SLOTS / 8 + 1];
	int InRun;
	int RunStart;
	int j;

	CLprintf(file, "\n--- -----------------------------------------\n");
	CLprintf(file, "--- MODULE: units $Id$\n\n");

#if 0
	//
	//  Local variables
	//
	CLprintf(file, "(set-xp-damage! #%s)\n",
		XpDamage ? "t" : "f");
	CLprintf(file, "(set-fancy-buildings! #%s)\n",
		FancyBuildings ? "t" : "f");
	CLprintf(file, "(set-training-queue! #%s)\n",
		EnableTrainingQueue ? "t" : "f");
#endif

	CLprintf(file, "-- Unit slot usage bitmap\n");
	CLprintf(file, "SlotUsage(");

	memset(SlotUsage, 0, MAX_UNIT_SLOTS / 8 + 1);
	for (i = 0; i < NumUnits; ++i) {
		int slot;
		slot = Units[i]->Slot;
		SlotUsage[slot / 8] |= 1 << (slot % 8);
	}
#if 0
	/* the old way */
	for (i = 0; i < MAX_UNIT_SLOTS / 8 + 1; ++i) {
		CLprintf(file, " %d", SlotUsage[i]);
		if ((i + 1) % 16 == 0) // 16 numbers per line
			CLprintf(file, "\n");
	}

#else
#define SlotUsed(slot) (SlotUsage[(slot) / 8] & (1 << ((slot) % 8)))
	RunStart = InRun = 0;
	j = 0;
	for (i = 0; i < MAX_UNIT_SLOTS; ++i) {
		if (!InRun && SlotUsed(i)) {
			InRun = 1;
			RunStart = i;
		}
		if (!SlotUsed(i) && InRun) {
			InRun = 0;
			if (!j) {
				j = 1;
			} else {
				CLprintf(file, ", ");
			}
			if (i - 1 == RunStart) {
				CLprintf(file, "%d", i - 1);
			} else {
				CLprintf(file, "%d, \"-\", %d", RunStart, i - 1);
			}
		}
	}
#endif

	CLprintf(file, ")\n");

	for (table = Units; table < &Units[NumUnits]; ++table) {
		SaveUnit(*table, file);
	}

	//  Save the Unit allocator state, sadly. I don't want to do this!
	CLprintf(file, "\nUnitAllocQueue(\n");
	for (unit = ReleasedHead; unit; unit = unit->Next) {
		CLprintf(file, "\t{Slot = %d, FreeCycle = %d}%c \n", unit->Slot, unit->Refs,
				(unit->Next ? ',' : ' '));
		DebugPrint("{Slot = %d, FreeCycle = %d}\n" _C_ unit->Slot _C_ unit->Refs);
	}
	CLprintf(file, ")\n");
}

/*----------------------------------------------------------------------------
--  Initialize/Cleanup
----------------------------------------------------------------------------*/

/**
**  Initialize unit module.
*/
void InitUnits(void)
{
}

/**
**  Clean up unit module.
*/
void CleanUnits(void)
{
	Unit** table;
	Unit* unit;
	Order* order;

	//
	//  Free memory for all units in unit table.
	//
	for (table = Units; table < &Units[NumUnits]; ++table) {
		free((*table)->AutoCastSpell);
		free((*table)->Variable);
		free((*table)->Orders);
		free((*table)->CacheLinks);
		free(*table);
		*table = NULL;
	}

	//
	//  Release memory of units in release queue.
	//
	while ((unit = ReleasedHead)) {
		ReleasedHead = unit->Next;
		free(unit);
	}

	//
	//  Release memory of Orders in the release queue.
	while ((order = ReleasedOrderHead)) {
		ReleasedOrderHead = order->Arg1;
		free(order);
	}

	InitUnitsMemory();

	XpDamage = 0;
	FancyBuildings = 0;
	HelpMeLastCycle = 0;
}

//@}
