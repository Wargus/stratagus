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
//      the Free Software Foundation; version 2 dated June, 1991.
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
  --		Includes
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

/*----------------------------------------------------------------------------
  --		Variables
  ----------------------------------------------------------------------------*/

#ifndef LimitSearch
#define LimitSearch 1						/// Limit the search
#endif

global Unit* UnitSlots[MAX_UNIT_SLOTS];		/// All possible units
global Unit** UnitSlotFree;				/// First free unit slot
local Unit* ReleasedHead;				/// List of released units.
local Unit** ReleasedTail;				/// List tail of released units.

global Unit* Units[MAX_UNIT_SLOTS];		/// Array of used slots
global int NumUnits;						/// Number of slots used

global int XpDamage;						/// Hit point regeneration for all units
global char EnableTrainingQueue;		/// Config: training queues enabled
global char EnableBuildingCapture;		/// Config: capture buildings enabled
global char RevealAttacker;				/// Config: reveal attacker enabled

local unsigned long HelpMeLastCycle;		/// Last cycle HelpMe sound played
local int HelpMeLastX;						/// Last X coordinate HelpMe sound played
local int HelpMeLastY;						/// Last Y coordinate HelpMe sound played

/*----------------------------------------------------------------------------
  --		Functions
  ----------------------------------------------------------------------------*/

/**
**		Initial memory allocation for units.
*/
global void InitUnitsMemory(void)
{
	Unit** slot;

	// Initialize the "list" of free unit slots

	slot = UnitSlots + MAX_UNIT_SLOTS;
	*--slot = NULL;						// leave last slot free as no marker
	*--slot = NULL;
	do {
		slot[-1] = (void*)slot;
	} while (--slot > UnitSlots);
	UnitSlotFree = slot;

	ReleasedTail = &ReleasedHead;				// list of unfreed units.
	NumUnits = 0;
}

#if 0
/**
**		Free the memory for an unit slot. Update the global slot table.
**		The memory should only be freed, if all references are dropped.
**
**		@param unit		Pointer to unit.
*/
global void FreeUnitMemory(Unit* unit)
{
	Unit** slot;

	//
	//		Remove from slot table
	//
	slot = UnitSlots + unit->Slot;
	DebugCheck(*slot != unit);

	*slot = (void*)UnitSlotFree;
	free(unit);
}
#endif

/**
**		Increase an unit's reference count.
**
**		@param unit		The unit
*/
global void RefsIncrease(Unit* unit)
{
	RefsDebugCheck(!unit->Refs || unit->Destroyed);
	if (!SaveGameLoading) {
		++unit->Refs;
	}
}

/**
**		Decrease an unit's reference count.
**
**		@param unit		The unit
*/
global void RefsDecrease(Unit* unit)
{
	RefsDebugCheck(!unit->Refs);
	if (!SaveGameLoading) {
		if (unit->Destroyed) {
			if (!--unit->Refs) {
				ReleaseUnit(unit);
			}
		} else {
			--unit->Refs;
			RefsDebugCheck(!unit->Refs);
		}
	}
}

/**
**		Release an unit.
**
**		The unit is only released, if all references are dropped.
**
**		@param unit		Pointer to unit.
*/
global void ReleaseUnit(Unit* unit)
{
	Unit* temp;
	DebugLevel2Fn("%lu:Unit %p %d `%s'\n" _C_ GameCycle _C_
		unit _C_ UnitNumber(unit) _C_ unit->Type->Ident);

	DebugCheck(!unit->Type);				// already free.
	DebugCheck(unit->OrderCount != 1);
	DebugCheck(unit->Orders[0].Goal);

	//
	//		First release, remove from lists/tables.
	//
	if (!unit->Destroyed) {
		DebugLevel0Fn("First release %d\n" _C_ unit->Slot);

		//
		//		Are more references remaining?
		//
		unit->Destroyed = 1;				// mark as destroyed

		if (--unit->Refs > 0) {
			DebugLevel2Fn("%lu:More references of %d #%d\n" _C_ GameCycle _C_
				UnitNumber(unit) _C_ unit->Refs);
			return;
		}
	}

	RefsDebugCheck(unit->Refs);

	//
	//		No more references remaining, but the network could have an order
	//		on the way. We must wait a little time before we could free the
	//		memory.
	//
	*ReleasedTail = unit;
	UnitCacheRemove(unit);
	//
	//		Remove the unit from the global units table.
	//
	DebugCheck(*unit->UnitSlot != unit);
	temp = Units[--NumUnits];
	temp->UnitSlot = unit->UnitSlot;
	*unit->UnitSlot = temp;
	Units[NumUnits] = NULL;

	unit->Next = 0;
	ReleasedTail = &unit->Next;
	unit->Refs = GameCycle + (NetworkMaxLag << 1);	// could be reuse after this time
	DebugLevel2Fn("%lu:No more references, only wait for network lag, unit %d\n" _C_
		GameCycle _C_ UnitNumber(unit));
	unit->Type = 0; 						// for debugging.
#ifdef NEW_UNIT_CACHE
	free(unit->CacheLinks);
#endif
}

/**
**		FIXME: Docu
*/
local Unit *AllocUnit(void)
{
	Unit* unit;
	Unit** slot;
	//
	//		Game unit limit reached.
	//
	if (NumUnits >= UnitMax) {
		DebugLevel0Fn("Over all unit limit (%d) reached.\n" _C_ UnitMax);
		// FIXME: Hoping this is checked everywhere.
		return NoUnitP;
	}

	//
	//		Can use released unit?
	//
	if (ReleasedHead && (unsigned)ReleasedHead->Refs < GameCycle) {
		unit = ReleasedHead;
		ReleasedHead = unit->Next;
		if (ReleasedTail == &unit->Next) {		// last element
			ReleasedTail = &ReleasedHead;
		}
		DebugLevel2Fn("%lu:Release %p %d\n" _C_ GameCycle _C_ unit _C_
			UnitNumber(unit));
		slot = UnitSlots + unit->Slot;
		memset(unit, 0, sizeof(*unit));
		// FIXME: can release here more slots, reducing memory needs.
	} else {
		//
		//		Allocate structure
		//
		if (!(slot = UnitSlotFree)) {		// should not happen!
			DebugLevel0Fn("Maximum of units reached\n");
			return NoUnitP;
		}
		UnitSlotFree = (void*)*slot;
		*slot = unit = calloc(1, sizeof(*unit));
	}
	unit->Slot = slot - UnitSlots;				// back index
	return unit;
}

/**
**		Initialize the unit slot with default values.
**
**		@param unit		Unit pointer (allocated zero filled)
**		@param type		Unit-type
*/
global void InitUnit(Unit* unit, UnitType* type)
{
#ifdef NEW_UNIT_CACHE
	int i;
#endif

	DebugCheck(!type);

	//  Set refs to 1. This is the "I am alive ref", lost in ReleaseUnit.
	unit->Refs = 1;

	//
	//  Build all unit table
	//
	unit->UnitSlot = &Units[NumUnits];		// back pointer
	Units[NumUnits++] = unit;

	DebugLevel3Fn("%p %d\n" _C_ unit _C_ UnitNumber(unit));

	//
	//  Initialise unit structure (must be zero filled!)
	//
	unit->Type = type;
#ifdef NEW_UNIT_CACHE
	unit->CacheLinks = (UnitListItem *)malloc(type->TileWidth * type->TileHeight * sizeof(UnitListItem));
	for (i = 0; i < type->TileWidth * type->TileHeight; ++i) {
		unit->CacheLinks[i].Prev = unit->CacheLinks[i].Next = 0;
		unit->CacheLinks[i].Unit = unit;
	}
#endif

	unit->Seen.Frame = UnitNotSeen;				// Unit isn't yet seen

	// On Load, Some units don't have Still animation, eg Deadbody
	if (unit->Type->Animations->Still) {
		unit->Frame = unit->Type->Animations->Still[0].Frame +
			(type->Building ? 0 : type->NumDirections / 2 + 1 - 1);
	}

	if (!type->Building && type->Sprite &&
			VideoGraphicFrames(type->Sprite) > 5) {
		unit->Direction = (MyRand() >> 8) & 0xFF;		// random heading
		UnitUpdateHeading(unit);
	}

	if (type->CanCastSpell) {
		unit->Mana = (type->_MaxMana * MAGIC_FOR_NEW_UNITS) / 100;
		unit->AutoCastSpell = malloc(SpellTypeCount);
		memset(unit->AutoCastSpell, 0, SpellTypeCount);
	}
	unit->Active = 1;

	unit->Wait = 1;
	unit->Reset = 1;
	unit->Removed = 1;

	unit->Rs = MyRand() % 100;				// used for fancy buildings and others

	unit->OrderCount = 1;				// No orders
	unit->Orders[0].Action = UnitActionStill;
	unit->Orders[0].X = unit->Orders[0].Y = -1;
	DebugCheck(unit->Orders[0].Goal);
	unit->NewOrder.Action = UnitActionStill;
	unit->NewOrder.X = unit->NewOrder.Y = -1;
	DebugCheck(unit->NewOrder.Goal);
	unit->SavedOrder.Action = UnitActionStill;
	unit->SavedOrder.X = unit->SavedOrder.Y = -1;
	DebugCheck(unit->SavedOrder.Goal);
}

/**
**		FIXME: Docu
*/
global void AssignUnitToPlayer(Unit* unit, Player* player)
{
	UnitType* type;

	type = unit->Type;

	//
	//		Build player unit table
	//
	if (player && !type->Vanishes && unit->Orders[0].Action != UnitActionDie) {
		unit->PlayerSlot = player->Units + player->TotalNumUnits++;
		if (type->_HitPoints != 0) {
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
		player->Demand += type->Demand;		// food needed
		if (player == ThisPlayer) {
			MustRedraw |= RedrawResources;		// update food
		}
	}
	if (type->Building) {
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
**		Create a new unit.
**
**		@param type		Pointer to unit-type.
**		@param player		Pointer to owning player.
**
**		@return				Pointer to created unit.
*/
global Unit* MakeUnit(UnitType* type, Player* player)
{
	Unit* unit;

	//DebugCheck(!player);				// Current code didn't support no player

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
**		Place unit on map.
**
**		@param unit		Unit to be placed.
**		@param x		X map tile position.
**		@param y		Y map tile position.
*/
global void PlaceUnit(Unit* unit, int x, int y)
{
	const UnitType* type;
	int h;
	int w;
	unsigned flags;

	DebugCheck(!unit->Removed);

	type = unit->Type;


	unit->X = x;
	unit->Y = y;

	//
	//		Place unit on the map, mark the field with the FieldFlags.
	//
	flags = type->FieldFlags;
	for (h = type->TileHeight; h--;) {
		for (w = type->TileWidth; w--;) {
			TheMap.Fields[x + w + (y + h) * TheMap.Width].Flags |= flags;
		}
	}

#ifdef MAP_REGIONS
	if (type->Building &&
			(type->FieldFlags &
		 (MapFieldLandUnit | MapFieldSeaUnit | MapFieldBuilding |
		  MapFieldUnpassable | MapFieldWall | MapFieldRocks | MapFieldForest))){
		MapSplitterTilesOccuped(x, y, x + type->TileWidth - 1, y + type->TileHeight - 1);
	}
#endif

	x += unit->Type->TileWidth / 2;
	y += unit->Type->TileHeight / 2;

	//
	//		Units under construction have no sight range.
	//
	if (!unit->Constructed) {
		//
		//		Update fog of war, if unit belongs to player on this computer
		//
		if (unit->Container && unit->Removed) {
			MapUnmarkUnitOnBoardSight(unit, unit->Container);
		}
		if (unit->Container) {
			RemoveUnitFromContainer(unit);
		}
		unit->CurrentSightRange = unit->Stats->SightRange;
		MapMarkUnitSight(unit);
	}

	unit->Removed = 0;
	unit->Next = 0;
	UnitCacheInsert(unit);

	MustRedraw |= RedrawMinimap;
	CheckUnitToBeDrawn(unit);
	UnitCountSeen(unit);
}

/**
**		Create new unit and place on map.
**
**		@param x		X map tile position.
**		@param y		Y map tile position.
**		@param type		Pointer to unit-type.
**		@param player		Pointer to owning player.
**
**		@return				Pointer to created unit.
*/
global Unit* MakeUnitAndPlace(int x, int y, UnitType* type, Player* player)
{
	Unit* unit;

	unit = MakeUnit(type, player);
	PlaceUnit(unit, x, y);

	return unit;
}

/**
**		Add unit to a container. It only updates linked list stuff
**
**		@param unit		Pointer to unit.
**		@param host		Pointer to container.
*/
global void AddUnitInContainer(Unit* unit, Unit* host)
{
	if (unit->Container) {
		DebugLevel0Fn("Unit is already contained.\n");
		exit(0);
	}
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
**		Remove unit from a container. It only updates linked list stuff
**
**		@param unit		Pointer to unit.
*/
global void RemoveUnitFromContainer(Unit* unit)
{
	Unit* host;
	host = unit->Container;
	if (!unit->Container) {
		DebugLevel0Fn("Unit not contained.\n");
		exit(0);
	}
	if (host->InsideCount == 0) {
		DebugLevel0Fn("host's inside count reached -1.");
		exit(0);
	}
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
**		Remove unit from map.
**
**		Update selection.
**		Update panels.
**		Update map.
**
**		@param unit		Pointer to unit.
**		@param host		Pointer to housing unit.
*/
global void RemoveUnit(Unit* unit, Unit* host)
{
	int h;
	int w;
	const UnitType* type;
	unsigned flags;

	if (unit->Removed && unit->Container) {
		MapUnmarkUnitOnBoardSight(unit, unit->Container);
	} else {
		MapUnmarkUnitSight(unit);
	}
	if (host) {
		unit->CurrentSightRange = host->CurrentSightRange;
		MapMarkUnitOnBoardSight(unit, host);
		AddUnitInContainer(unit, host);
	}

	if (unit->Removed) {				// could happen!
		// If unit is removed (inside) and building is destroyed.
		return;
	}
	unit->Removed = 1;
	//  Remove unit from the current selection
	if (unit->Selected) {
		if (NumSelected == 1) {				//  Remove building cursor
			CancelBuildingMode();
		}
		MustRedraw |= RedrawPanels;
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

	type = unit->Type;

	//
	//		Update map
	//
	flags = ~type->FieldFlags;
	for (h = type->TileHeight; h--;) {
		for (w = type->TileWidth; w--;) {
			TheMap.Fields[unit->X + w + (unit->Y + h) * TheMap.Width].Flags &= flags;
		}
	}

#ifdef MAP_REGIONS
	//
	//		Update map splitting.
	//
	if (type->Building &&
			(type->FieldFlags &
		 (MapFieldLandUnit | MapFieldSeaUnit | MapFieldBuilding |
		  MapFieldUnpassable | MapFieldWall | MapFieldRocks | MapFieldForest))){
		MapSplitterTilesCleared(unit->X, unit->Y,
			unit->X + type->TileWidth - 1, unit->Y + type->TileHeight - 1);
	}
#endif

	DebugLevel3Fn("%d %p %p\n" _C_ UnitNumber(unit) _C_ unit _C_ unit->Next);

	if (host) {
		UnitCacheRemove(unit);
		unit->Next = host;
	}

	MustRedraw |= RedrawMinimap;
	CheckUnitToBeDrawn(unit);
}

/**
**  Update information for lost units.
**
**  @param unit  Pointer to unit.
**
**  @note Also called by ChangeUnitOwner
*/
global void UnitLost(Unit* unit)
{
	Unit* temp;
	const UnitType* type;
	Player* player;
	int i;

	DebugCheck(!unit);

	player = unit->Player;
	DebugCheck(!player);  // Next code didn't support no player!

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
		DebugCheck(*unit->PlayerSlot != unit);
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
		if (player == ThisPlayer) {
			MustRedraw |= RedrawResources;  // update food
			// FIXME: MustRedraw |= RedrawFood;
		}
	}

	//
	//  Update information.
	//
	if (unit->Orders[0].Action != UnitActionBuilded) {
		if (type->Supply) {
			player->Supply -= type->Supply;
			if (player == ThisPlayer) {
				MustRedraw |= RedrawResources;
				// FIXME: MustRedraw |= RedrawFood;
			}
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
				if (player == ThisPlayer) {
					MustRedraw |= RedrawInfoPanel;
				}
			}
		}
	}

	//
	//  Handle research cancels.
	//
	if (unit->Orders[0].Action == UnitActionResearch) {
		unit->Player->UpgradeTimers.Upgrades[unit->Data.Research.Upgrade - Upgrades] = 0;
	}

	DebugLevel0Fn("Lost %s(%d)\n" _C_ unit->Type->Ident _C_ UnitNumber(unit));

	// Destroy resource-platform, must re-make resource patch.
	if (type->MustBuildOnTop && unit->Value > 0) {
		temp = MakeUnitAndPlace(unit->X, unit->Y, type->MustBuildOnTop, &Players[PlayerNumNeutral]);
		temp->Value = unit->Value;
	}
	DebugCheck(player->NumBuildings > UnitMax);
	DebugCheck(player->TotalNumUnits > UnitMax);
	DebugCheck(player->UnitTypesCount[type->Slot] > UnitMax);
}

/**
**  FIXME: Docu
**
**  @param unit  FIXME: docu
*/
global void UnitClearOrders(Unit *unit)
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
**		Update for new unit. Food and income ...
**
**		@param unit		New unit pointer.
**		@param upgrade		True unit was upgraded.
*/
global void UpdateForNewUnit(const Unit* unit, int upgrade)
{
	const UnitType* type;
	Player* player;
	int u;

	player = unit->Player;
	type = unit->Type;

	//
	//		Handle unit supply. (Currently only food supported.)
	//				Note an upgraded unit can't give more supply.
	//
	if (type->Supply && !upgrade) {
		player->Supply += type->Supply;
		if (player == ThisPlayer) {
			MustRedraw |= RedrawResources;		// update food
		}
	}

	//
	//		Update resources
	//
	for (u = 1; u < MaxCosts; ++u) {
		if (player->Incomes[u] < unit->Type->ImproveIncomes[u]) {
			player->Incomes[u] = unit->Type->ImproveIncomes[u];
			if (player == ThisPlayer) {
				MustRedraw |= RedrawInfoPanel;
			}
		}
	}
}

/**
**		Find nearest point of unit.
**
**		@param unit		Pointer to unit.
**		@param tx		X tile map postion.
**		@param ty		Y tile map postion.
**		@param dx		Out: nearest point X tile map postion to (tx,ty).
**		@param dy		Out: nearest point Y tile map postion to (tx,ty).
*/
global void NearestOfUnit(const Unit* unit, int tx, int ty, int *dx, int *dy)
{
	int x;
	int y;

	x = unit->X;
	y = unit->Y;

	DebugLevel3("Nearest of (%d,%d) - (%d,%d)\n" _C_ tx _C_ ty _C_ x _C_ y);
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

	DebugLevel3Fn("Goal is (%d,%d)\n" _C_ *dx _C_ *dy);
}

/**
**		Copy the unit look in Seen variables. This should be called when
**		buildings go under fog of war for ThisPlayer.
**
**      @param unit		The unit to work on
*/
local void UnitFillSeenValues(Unit* unit)
{
	//	Seen values are undefined for visible units.
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
**	This function should get called when an unit goes under fog of war.
**
**	@param unit	The unit that goes under fog.
**	@param p	The player the unit goes out of fog for.
*/
global void UnitGoesUnderFog(Unit* unit, const Player* player)
{
	DebugLevel3Fn("Unit %d(%s) at %d %d goes under fog for %d\n" _C_
			unit->Slot _C_ unit->Type->Name _C_ unit->X _C_ unit->Y _C_ player->Player);
	if (unit->Type->VisibleUnderFog) {
		if (player->Type == PlayerPerson && !unit->Destroyed) {
			DebugLevel3Fn("unit %d(%s): got a ref from player %d\n" _C_
					unit->Slot _C_ unit->Type->Name _C_ p);
			RefsIncrease(unit);
		}
		//
		//	Icky yucky icky Seen.Destroyed trickery.
		//	We track for each player if he's seen the unit as destroyed.
		//	Remember, an unit is marked Destroyed when it's gone as in
		//	completely gone, the corpses vanishes. In that case the unit
		//	only survives since some players did NOT see the unit destroyed.
		//	Keeping trackof that is hard, mostly due to complex shared vision
		//	configurations.
		//	A unit does NOT get a reference when it goes under fog if it's
		//	Destroyed. Furthermore, it shouldn't lose a reference if it was
		//	Seen destroyed. That only happend with complex shared vision, and
		//	it's sort of the whole point of this tracking.
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
**	This function should get called when an unit goes out of fog of war.
**
**	@param unit	The unit that goes out of fog.
**	@param p	The player the unit goes out of fog for.
**
**	@note For units that are visible under fog (mostly buildings)
**	we use reference counts, from the players that know about
**	the building. When an building goes under fog it gets a refs
**	increase, and when it shows up it gets a decrease. It must
**	not get an decrease the first time it's seen, so we have to
**	keep track of what player saw what units, with SeenByPlayer.
*/
global void UnitGoesOutOfFog(Unit* unit, const Player* player)
{
	DebugLevel3Fn("Unit %d(%s) at %d %d goes out of fog for %d.\n" _C_
			unit->Slot _C_ unit->Type->Name _C_ unit->X _C_ unit->Y _C_ player->Player);
	if (unit->Type->VisibleUnderFog) {
		if (unit->Seen.ByPlayer & (1 << (player->Player))) {
			if ((player->Type == PlayerPerson) &&
					(!(   unit->Seen.Destroyed & (1 << player->Player)   )) ) {
				DebugLevel3Fn("unit %d(%s): cleaned a ref from player %d\n" _C_
						unit->Slot _C_ unit->Type->Name _C_ player->Player);
				RefsDecrease(unit);
			}
		} else {
			DebugLevel3Fn("Player %d discovers unit %d(%s) at %d %d.\n" _C_ player->Player _C_
					unit->Slot _C_ unit->Type->Name _C_ unit->X _C_ unit->Y);
			unit->Seen.ByPlayer |= (1 << (player->Player));
		}
	}
}

/**
**		Mark all units on a tile as now visible.
**
**		@param p		The player this is for.
**		@param x		x location to check
**		@param y		y location to check
**		@param clock	If we mark cloaked units too.
*/
global void UnitsOnTileMarkSeen(const Player* player, int x, int y, int cloak)
{
	int p;
	int n;
	Unit* units[UnitMax];
	Unit* unit;

	n = UnitCacheOnTile(x, y,units);
	DebugLevel3Fn("I can see %d units from here.\n" _C_ n);
	while (n) {
		unit = units[--n];
		if (cloak != (int)unit->Type->PermanentCloak) {
			continue;
		}
		DebugLevel3("Unit %d(%d, %d) player %d seen %d -> %d (%d, %d)\n" _C_ unit->Slot _C_ unit->X _C_ unit->Y _C_
				player->Player _C_ unit->VisCount[player->Player] _C_ unit->VisCount[player->Player] + 1 _C_ x _C_ y);
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
**	This function unmarks units on x, y as seen. It uses a reference count.
**
**	@param player	The player to mark for.
**	@param x        x location to check if building is on, and mark as seen
**	@param y        y location to check if building is on, and mark as seen
**	@param cloak	If this is for cloaked units.
*/
global void UnitsOnTileUnmarkSeen(const Player* player, int x, int y, int cloak)
{
	int p;
	int n;
	Unit* units[UnitMax];
	Unit* unit;

	n = UnitCacheOnTile(x, y, units);
	DebugLevel3Fn("I can see %d units from here.\n" _C_ n);
	while (n) {
		unit = units[--n];
		if ((unit->X > x || unit->X + unit->Type->TileWidth - 1 < x ||
				unit->Y > y || unit->Y + unit->Type->TileHeight - 1 < y)) {
			DebugLevel0Fn("Wrong cache %d %d -> %d %d\n" _C_ x _C_ y _C_ unit->X _C_ unit->Y);
		}
		DebugCheck((unit->X > x || unit->X + unit->Type->TileWidth - 1 < x ||
				unit->Y > y || unit->Y + unit->Type->TileHeight - 1 < y));
		if (cloak != (int)unit->Type->PermanentCloak) {
			continue;
		}
		p = player->Player;
		DebugLevel3("Unit %d(%d, %d) player %d seen %d -> %d (%d, %d)\n" _C_ unit->Slot _C_ unit->X _C_ unit->Y _C_
				p _C_ unit->VisCount[p] _C_ unit->VisCount[p] - 1 _C_ x _C_ y);
		DebugCheck(!unit->VisCount[p]);
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
**	Recalculates an units visiblity count. This happens really often,
**	Like every time an unit moves. It's really fast though, since we
**	have per-tile counts.
**
**	@param unit	pointer to the unit to check if seen
*/
global void UnitCountSeen(Unit* unit)
{
	int x;
	int y;
	int p;
	int oldv[PlayerMax];
	int newv;

	DebugCheck(!unit->Type);

	//	FIXME: optimize, only work on certain players?
	//	This is for instance good for updating shared vision...

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
	//	Now here comes the tricky part. We have to go in and out of fog
	//	for players. Hopefully this works with shared vision just great.
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
**      Returns true, if the unit is visible. It check the Viscount of
**		the player and everyone who shares vision with him.
**		FIXME: optimize this a lot.
**
**		@note	This understands shared vision, and should be used all around.
**
**		@param unit				The unit to check.
**		@param player			The player to check.
*/
global int UnitVisible(const Unit* unit, const Player* player)
{
	int p;
	int cp;
	//	Current player.
	cp = player->Player;
	if (unit->VisCount[cp]) {
		return 1;
	}
	for (p = 0; p < PlayerMax; p++) {
		if (PlayersShareVision(p, cp)) {
			if (unit->VisCount[p]) {
				return 1;
			}
		}
	}
	return 0;
}

/**
**      Returns ture, if unit is visible as an action goal for a player
**      on the map.
**
**		@param unit		Unit to be checked.
**		@param player	Player to check for.
**		@return			True if visible, false otherwise.
*/
global int UnitVisibleAsGoal(const Unit* unit, const Player* player)
{
	//
	//	Invisibility
	//
	if (unit->Invisible && (player != unit->Player) &&
			(!PlayersShareVision(player->Player, unit->Player->Player))) {
		return 0;
	}
	if (UnitVisible(unit, player)) {
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
**		Returns true, if unit is visible for this player on the map.
**		The unit has to be out of fog of war and alive
**
**		@param unit		Unit to be checked.
**		@param player	Player to check for.
**		@return			True if visible, false otherwise.
*/
global int UnitVisibleOnMap(const Unit* unit, const Player* player)
{
	//
	//	Invisible units.
	//
	if (unit->Invisible && (player != unit->Player) &&
			(!PlayersShareVision(player->Player, unit->Player->Player))) {
		return 0;
	}

	return  (!unit->Removed) &&
			(!unit->Destroyed) &&
			(unit->HP) &&
			(unit->Orders->Action != UnitActionDie) &&
			(UnitVisible(unit, player));
}

/**
**		Returns true, if unit is shown on minimap.
**
**		@warning This is for ::ThisPlayer only.
**		@todo	radar support
**
**		@param unit		Unit to be checked.
**		@return			True if wisible, false otherwise.
*/
global int UnitVisibleOnMinimap(const Unit* unit)
{
	//
	//	Invisible units.
	//
	if (unit->Invisible && (ThisPlayer != unit->Player) &&
			(!PlayersShareVision(ThisPlayer->Player, unit->Player->Player))) {
		return 0;
	}
	if (UnitVisible(unit, ThisPlayer) || ReplayRevealMap) {
		return  (!unit->Removed) &&
				(!unit->Destroyed) &&
				(unit->Orders->Action != UnitActionDie);
	} else {
		if (!unit->Type->VisibleUnderFog) {
			return 0;
		}
		return ((unit->Seen.ByPlayer & (1 << ThisPlayer->Player)) && 
				(!(unit->Seen.Destroyed & (1 << ThisPlayer->Player))) );
	}
}

/**
**		Returns true, if unit is visible in viewport.
**
**		@warning This is only true for ::ThisPlayer
**		@param vp			Viewport pointer.
**		@param unit			Unit to be checked.
**		@return				True if visible, false otherwise.
*/
global int UnitVisibleInViewport(const Unit* unit, const Viewport* vp)
{
	//
	//	Check if it's at least inside the damn viewport.
	//
	if ((unit->X + unit->Type->TileWidth < vp->MapX) ||
			(unit->X > vp->MapX + vp->MapWidth) ||
			(unit->Y + unit->Type->TileHeight < vp->MapY) ||
			(unit->Y > vp->MapY + vp->MapHeight)) {
		return 0;
	}

	if (!ThisPlayer) {
		//FIXME: ARI: Added here for early game setup state by
		//		MakeAndPlaceUnit() from LoadMap(). ThisPlayer not yet set,
		//		so don't show anything until first real map-draw.
		DebugLevel0Fn("Fix ME ThisPlayer not set yet?!\n");
		return 0;
	}

	//	Those are never ever visible.
	if (unit->Invisible && (ThisPlayer != unit->Player) &&
			(!PlayersShareVision(ThisPlayer->Player, unit->Player->Player))) {
		return 0;
	}

	if (UnitVisible(unit, ThisPlayer) || ReplayRevealMap) {
		return !unit->Destroyed;
	} else {
		//	Unit has to be 'discovered'
		//	Destroyed units ARE visible under fog of war, if we haven't seen them like that.
		if (!unit->Destroyed || !(unit->Seen.Destroyed & (1 << ThisPlayer->Player))) {
			return (unit->Type->VisibleUnderFog && (unit->Seen.ByPlayer & (1 << ThisPlayer->Player)));
		} else {
			return 0;
		}
	}
}

/**
**		Returns true, if unit is visible on current map view (any viewport).
**
**		@param unit		Unit to be checked.
**		@return				True if visible, false otherwise.
*/
global int UnitVisibleOnScreen(const Unit* unit)
{
	const Viewport* vp;

	for (vp = TheUI.Viewports; vp < TheUI.Viewports + TheUI.NumViewports; ++vp) {
		if (UnitVisibleInViewport(unit, vp)) {
			DebugLevel3Fn("unit %d(%s) is visibile\n" _C_ unit->Slot _C_ unit->Type->Ident);
			return 1;
		}
	}
	return 0;
}

/**
**	  Get area of map tiles covered by unit, including its displacement.
**
**	  @param unit	 Unit to be checked and set.
**		@param sx		Out: Top left X tile map postion.
**		@param sy		Out: Top left Y tile map postion.
**		@param ex		Out: Bottom right X tile map postion.
**		@param ey		Out: Bottom right Y tile map postion.
**
**	  @return				sx,sy,ex,ey defining area in Map
*/
global void GetUnitMapArea(const Unit* unit, int* sx, int* sy, int* ex, int* ey)
{
	*sx = unit->X - (unit->IX < 0);
	*ex = *sx + unit->Type->TileWidth - !unit->IX;
	*sy = unit->Y - (unit->IY < 0);
	*ey = *sy + unit->Type->TileHeight - !unit->IY;
}

#ifdef NEW_DECODRAW
/**
**		Decoration redraw function that will redraw an unit (no building) for
**		set clip rectangle by decoration mechanism.
**
**		@param data		Unit pointer to be drawn
*/
local void DecoUnitDraw(void* data)
{
	Unit* unit;

	unit = (Unit*)data;
	DebugCheck(unit->Removed);
	//DebugCheck(!UnitVisibleOnScreen(unit));

	DrawUnit(unit);
}

/**
**		Create decoration for any unit-type
**
**		@param u		an unit which is visible on screen
**	  @param x		x pixel position on screen of left-top
**	  @param y		y pixel position on screen of left-top
**	  @param w		width in pixels of area to be drawn from (x, y)
**	  @param h		height in pixels of area to be drawn from (x, y)
*/
local void AddUnitDeco(Unit* u, int x, int y, int w, int h)
{
	u->Decoration = DecorationAdd(u, DecoUnitDraw, 1, x, y, w, h);
}
#endif

/**
**	  Check and sets if unit must be drawn on screen-map
**
**	  @param unit	 Unit to be checked.
**	  @return		 True if map marked to be drawn, false otherwise.
*/
global int CheckUnitToBeDrawn(Unit* unit)
{
#ifdef NEW_MAPDRAW
	int sx;
	int sy;
	int ex;
	int ey;

	// in debug-mode check unsupported displacement exceeding an entire Tile
	// FIXME: displacement could always be made positive and smaller than Tile
#if NEW_MAPDRAW > 1
	if (unit->IX <= -TileSizeX || unit->IX >= TileSizeX ||
			unit->IY <= -TileSizeY || unit->IY >= TileSizeY) {
		printf("internal error in CheckUnitToBeDrawn\n");
	}
#endif

	GetUnitMapArea(unit, &sx, &sy, &ex, &ey);

	// FIXME: extra tiles added here for attached statusbar/mana/shadow/..
	--sx;
	--sy;
	++ex;
	++ey;

	if (MarkDrawAreaMap(sx, sy, ex, ey)) {
		//  MustRedraw|=RedrawMinimap;
		return 1;
	}
#else
#ifdef NEW_DECODRAW
	if (!unit->Removed && UnitVisibleOnScreen(unit)) {
		int x;
		int y;
		int w;
		int h;

		// FIXME: Inaccurate dimension to take unit's extras into account..
		//		Should be solved by adding each unit extra as separate decoration
		x = Map2ViewportX(TheUI.SelectedViewport, unit->X) + unit->IX
				+ unit->Type->TileWidth * TileSizeX / 2 - unit->Type->Width / 2 - 10;
		y = Map2ViewportY(TheUI.SelectedViewport, unit->Y) + unit->IY
				+ unit->Type->TileHeight * TileSizeY / 2 - unit->Type->Height / 2 - 10;
		w = unit->Type->Width + 20;
		h = unit->Type->Height + 20;

		if (unit->Decoration) {
			unit->Decoration = DecorationMove(unit->Decoration, x, y, w, h);
		} else {
			DebugLevel3Fn("Adding Decoration for %d(%s)\n" _C_ unit->Slot _C_ unit->Type->Name);
			AddUnitDeco((Unit*)unit, x, y, w, h);
		}

		return 1;
	} else if (unit->Decoration) {
		// not longer visible: so remove from auto Decorationration redraw
		DebugLevel3Fn("Removing Decoration for %d(%s)\n" _C_ unit->Slot _C_ unit->Type->Name);
		DecorationRemove(unit->Decoration);
		unit->Decoration = NULL;
	}
#else
	if (UnitVisibleOnScreen(unit)) {
		MustRedraw |= RedrawMap;
		return 1;
	}
#endif
#endif
	return 0;
}

/**
**		Change the unit's owner
**
**		@param unit				Unit which should be consigned.
**		@param newplayer		New owning player.
**
**				inside?
*/
global void ChangeUnitOwner(Unit* unit, Player* newplayer)
{
	int i;
	Unit* uins;
	Player* oldplayer;

	oldplayer = unit->Player;

	// This shouldn't happen
	if (oldplayer == newplayer) {
		DebugLevel0Fn("Change the unit owner to the same player???\n");
		return;
	}

	// Rescue all units in buildings/transporters.
	uins = unit->UnitInside;
	for (i = unit->InsideCount; i; --i, uins = uins->NextContained) {
		ChangeUnitOwner(uins, newplayer);
	}

	//
	//		Must change food/gold and other.
	//
	UnitLost(unit);

	//
	//		Now the new side!
	//

	//		Insert into new player table.

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

	if (unit->Removed && unit->Container) {
		MapUnmarkUnitOnBoardSight(unit, unit->Next);
		unit->Player = newplayer;
		MapMarkUnitOnBoardSight(unit, unit->Next);
	} else {
		MapUnmarkUnitSight(unit);
		unit->Player = newplayer;
		MapMarkUnitSight(unit);
	}

	unit->Stats = &unit->Type->Stats[newplayer->Player];
	//
	//		Must change food/gold and other.
	//
	if (unit->Type->GivesResource) {
		DebugLevel0Fn("Resource transfer not supported\n");
	}
	newplayer->Demand += unit->Type->Demand;
	newplayer->Supply += unit->Type->Supply;
	if (newplayer == ThisPlayer) {
		MustRedraw |= RedrawResources;// update food
	}
	if (unit->Type->Building) {
		newplayer->NumBuildings++;
	}
	newplayer->UnitTypesCount[unit->Type->Slot]++;

	UpdateForNewUnit(unit, 0);
}

/**
**		Change the owner of all units of a player.
**
**		@param oldplayer		Old owning player.
**		@param newplayer		New owning player.
*/
local void ChangePlayerOwner(Player* oldplayer, Player* newplayer)
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
**		Rescue units.
**
**		Look through all rescueable players, if they could be rescued.
*/
global void RescueUnits(void)
{
	Player* p;
	Unit* unit;
	Unit* table[UnitMax];
	Unit* around[UnitMax];
	int n;
	int i;
	int j;
	int l;

	if (NoRescueCheck) {				// all possible units are rescued
		return;
	}
	NoRescueCheck = 1;

	//
	//		Look if player could be rescued.
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
				DebugLevel3("Checking %d(%s)" _C_ UnitNumber(unit) _C_
					unit->Type->Ident);
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
				DebugLevel3(" = %d\n" _C_ n);
				//
				//		Look if ally near the unit.
				//
				for (i = 0; i < n; ++i) {
					if (around[i]->Type->CanAttack &&
							IsAllied(unit->Player, around[i])) {
						//
						//		City center converts complete race
						//		NOTE: I use a trick here, centers could
						//				store gold. FIXME!!!
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
  --		Unit headings
  ----------------------------------------------------------------------------*/

/**
**		Fast arc tangent function.
**
**		@param val		atan argument
**
**		@return				atan(val)
*/
local int myatan(int val)
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
**		Convert direction to heading.
**
**		@param delta_x		Delta X.
**		@param delta_y		Delta Y.
**
**		@return				Angle (0..255)
*/
global int DirectionToHeading(int delta_x, int delta_y)
{
	//
	//		Check which quadrant.
	//
	if (delta_x > 0) {
		if (delta_y < 0) {		// Quadrant 1?
			return myatan((delta_x * 64) / -delta_y);
		}
		// Quadrant 2?
		return myatan((delta_y * 64) / delta_x) + 64;
	}
	if (delta_y>0) {				// Quadrant 3?
		return myatan((delta_x * -64) / delta_y) + 64 * 2;
	}
	if (delta_x) {				// Quadrant 4.
		return myatan((delta_y * -64) / -delta_x) + 64 * 3;
	}
	return 0;
}

/**
**		Update sprite frame for new heading.
*/
global void UnitUpdateHeading(Unit* unit)
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
	if (dir <= LookingS / nextdir) {		// north->east->south
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
**		Change unit heading/frame from delta direction x, y.
**
**		@param unit		Unit for new direction looking.
**		@param dx		X map tile delta direction.
**		@param dy		Y map tile delta direction.
*/
global void UnitHeadingFromDeltaXY(Unit* unit, int dx, int dy)
{
	unit->Direction = DirectionToHeading(dx, dy);
	UnitUpdateHeading(unit);
}

/*----------------------------------------------------------------------------
  --		Drop out units
  ----------------------------------------------------------------------------*/

/**
**		Reappear unit on map.
**
**		@param unit		Unit to drop out.
**		@param heading		Direction in which the unit should appear.
**		@param addx		Tile size in x.
**		@param addy		Tile size in y.
*/
global void DropOutOnSide(Unit* unit, int heading, int addx, int addy)
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
	unit->Wait = 1;				// should be correct unit has still action

	PlaceUnit(unit, x, y);
}

/**
**		Reappear unit on map nearest to x, y.
**
**		@param unit		Unit to drop out.
**		@param gx		Goal X map tile position.
**		@param gy		Goal Y map tile position.
**		@param addx		Tile size in x.
**		@param addy		Tile size in y.
*/
global void DropOutNearest(Unit* unit, int gx, int gy, int addx, int addy)
{
	int x;
	int y;
	int i;
	int bestx;
	int besty;
	int bestd;
	int mask;
	int n;

	DebugLevel3Fn("%d\n" _C_ UnitNumber(unit));
	DebugCheck(!unit->Removed);

	x = y = -1;
	if (unit->Container) {
		x = unit->Container->X;
		y = unit->Container->Y;
	} else {
		x = unit->X;
		y = unit->Y;
	}

	DebugCheck(x == -1 || y == -1);
	mask = UnitMovementMask(unit);

	bestd = 99999;
#ifdef DEBUG
	bestx = besty = 0;				// keep the compiler happy
#endif

	// FIXME: if we reach the map borders we can go fast up, left, ...
	--x;
	for (;;) {
		for (i = addy; i--; ++y) {		// go down
			if (CheckedCanMoveToMask(x, y, mask)) {
				n = MapDistance(gx, gy, x, y);
				DebugLevel3("Distance %d,%d %d\n" _C_ x _C_ y _C_ n);
				if (n < bestd) {
					bestd = n;
					bestx = x;
					besty = y;
				}
			}
		}
		++addx;
		for (i = addx; i--; ++x) {		// go right
			if (CheckedCanMoveToMask(x, y, mask)) {
				n = MapDistance(gx, gy, x, y);
				DebugLevel3("Distance %d,%d %d\n" _C_ x _C_ y _C_ n);
				if (n < bestd) {
					bestd = n;
					bestx = x;
					besty = y;
				}
			}
		}
		++addy;
		for (i = addy; i--; --y) {		// go up
			if (CheckedCanMoveToMask(x, y, mask)) {
				n = MapDistance(gx, gy, x, y);
				DebugLevel3("Distance %d,%d %d\n" _C_ x _C_ y _C_ n);
				if (n < bestd) {
					bestd = n;
					bestx = x;
					besty = y;
				}
			}
		}
		++addx;
		for (i = addx; i--; --x) {		// go left
			if (CheckedCanMoveToMask(x, y, mask)) {
				n = MapDistance(gx, gy, x, y);
				DebugLevel3("Distance %d,%d %d\n" _C_ x _C_ y _C_ n);
				if (n < bestd) {
					bestd = n;
					bestx = x;
					besty = y;
				}
			}
		}
		if (bestd != 99999) {
			unit->Wait = 1;				// unit should have action still
			PlaceUnit(unit, bestx, besty);
			return;
		}
		++addy;
	}
}

/**
**		Drop out all units inside unit.
**
**		@param source		All units inside source are dropped out.
*/
global void DropOutAll(const Unit* source)
{
	Unit* unit;
	int i;

	unit = source->UnitInside;
	for (i = source->InsideCount; i; --i, unit = unit->NextContained) {
		DropOutOnSide(unit, LookingW,
			source->Type->TileWidth, source->Type->TileHeight);
		DebugCheck(unit->Orders[0].Goal);
		unit->Orders[0].Action = UnitActionStill;
		unit->Wait=unit->Reset = 1;
		unit->SubAction = 0;
	}
	DebugLevel0Fn("Drop out %d of %d\n" _C_ i _C_ source->Data.Resource.Active);
}

/*----------------------------------------------------------------------------
  --		Building units
  ----------------------------------------------------------------------------*/

/**
**		Can build unit here.
**		Hall to near to goldmine.
**		Refinery or shipyard to near to oil patch.
**
**		@param type		unit-type to be checked.
**		@param x		Map X position.
**		@param y		Map Y position.
**		@return				True if could build here, otherwise false.
*/
global int CanBuildHere(const UnitType* type, int x, int y)
{
	Unit* table[UnitMax];
	int n;
	int i;
	int w;
	int h;
	int found;

	//
	//		Can't build outside the map
	//
	if (x + type->TileWidth > TheMap.Width) {
		return 0;
	}
	if (y + type->TileHeight > TheMap.Height) {
		return 0;
	}

	if (EditorRunning) {
		if (type->GivesResource == OilCost) {
			// FIXME: Better ideas? type->OnlyPlaceable on odd tiles? Yuck.
			// Oil patches and platforms can only be placed on even tiles
			if (!(x & 1 && y & 1)) {
				return 0;
			}
			n = UnitCacheSelect(x, y, x + type->TileWidth, y + type->TileHeight, table);
			for (i = 0; i < n; ++i) {
				if (table[i]->Type->GivesResource == OilCost) {
					return 0;
				}
			}
		} else if (type->UnitType == UnitTypeFly || type->UnitType == UnitTypeNaval) {
			// Flyers and naval units can only be placed on odd tiles
			if (x & 1 || y & 1) {
				return 0;
			}
		}
	}

	// Must be checked before oil!
	if (type->ShoreBuilding) {
		found = 0;

		DebugLevel3("Shore building\n");
		// Need atleast one coast tile
		for (h = type->TileHeight; h--;) {
			for (w = type->TileWidth; w--;) {
				if (TheMap.Fields[x + w + (y + h) * TheMap.Width].Flags &
						MapFieldCoastAllowed) {
					h = w = 0;
					found = 1;
				}
			}
		}
		if (!found) {
			return 0;
		}
	}

	//		resource deposit can't be build too near to resource
	// FIXME: (mr-russ) check bound for Select
	n = UnitCacheSelect(x - RESOURCE_DISTANCE, y - RESOURCE_DISTANCE,
		x + type->TileWidth + RESOURCE_DISTANCE, y + type->TileHeight + RESOURCE_DISTANCE, table);
	for (i = 0; i < n; ++i) {
		if (type->CanStore[table[i]->Type->GivesResource]) {
			return 0;
		}
	}

	if (type->MustBuildOnTop && !EditorRunning) {
		// Resource platform could only be build on resource patch.
		n = UnitCacheSelect(x, y, x + 1, y + 1, table);
		for (i = 0; i < n; ++i) {
			if (table[i]->Type != type->MustBuildOnTop) {
				continue;
			}
			if (table[i]->Orders[0].Action == UnitActionBuilded) {
				continue;
			}
			if (table[i]->X == x && table[i]->Y == y) {
				return 1;
			}
		}

		return 0;
	}

	return 1;
}

/**
**		Can build on this point.
*/
global int CanBuildOn(int x, int y, int mask)
{
	if (x < 0 || y < 0 || x >= TheMap.Width || y >= TheMap.Height) {
		return 0;
	}
	return (TheMap.Fields[x + y * TheMap.Width].Flags & mask) ? 0 : 1;
}

/**
**		Can build unit-type on this point.
**
**		@param unit		Worker that want to build the building or NULL.
**		@param type		Building unit-type.
**		@param x		X tile map position.
**		@param y		Y tile map position.
**		@return				True if the building could be build..
**
**		@todo can't handle building units !1x1, needs a rewrite.
*/
global int CanBuildUnitType(const Unit* unit, const UnitType* type, int x, int y)
{
	int w;
	int h;
	int j;
	int mask;
	Player* player;

	// Terrain Flags don't matter.
	if (type->MustBuildOnTop) {
		return CanBuildHere(type, x, y);
	}

	//
	//		Remove unit that is building!
	//
#ifdef DEBUG
	j = 0;
#endif
	if (unit) {
		// FIXME: This only works with 1x1 big units
		DebugCheck(unit->Type->TileWidth != 1 || unit->Type->TileHeight != 1);
		j = unit->Type->FieldFlags;
		TheMap.Fields[unit->X + unit->Y * TheMap.Width].Flags &= ~j;
	}

#if 0
	// FIXME: Should be moved into unittype structure, and allow more types.
	if (type->ShoreBuilding) {
		mask = MapFieldLandUnit |
			MapFieldSeaUnit |
			MapFieldBuilding |		// already occuppied
			MapFieldWall |
			MapFieldRocks |
			MapFieldForest |		// wall,rock,forest not 100% clear?
			MapFieldLandAllowed |		// can't build on this
			//MapFieldUnpassable |		// FIXME: I think shouldn't be used
			MapFieldNoBuilding;
	} else if (type->Building) {
		switch (type->UnitType) {
			case UnitTypeLand:
				mask = MapFieldLandUnit |
					MapFieldBuilding |		// already occuppied
					MapFieldWall |
					MapFieldRocks |
					MapFieldForest |		// wall,rock,forest not 100% clear?
					MapFieldCoastAllowed |
					MapFieldWaterAllowed |		// can't build on this
					MapFieldUnpassable |		// FIXME: I think shouldn't be used
					MapFieldNoBuilding;
				break;
			case UnitTypeNaval:
				mask = MapFieldSeaUnit |
					MapFieldBuilding |		// already occuppied
					MapFieldCoastAllowed |
					MapFieldLandAllowed |		// can't build on this
					MapFieldUnpassable |		// FIXME: I think shouldn't be used
					MapFieldNoBuilding;
				break;
			case UnitTypeFly:
				mask = MapFieldAirUnit;		// already occuppied
				break;
			default:
				DebugLevel1Fn("Were moves this unit?\n");
				if (unit) {
					TheMap.Fields[unit->X + unit->Y * TheMap.Width].Flags |= j;
				}
				return 0;
		}
	} else switch (type->UnitType) {
		case UnitTypeLand:
			mask = MapFieldLandUnit |
				MapFieldBuilding |		// already occuppied
				MapFieldWall |
				MapFieldRocks |
				MapFieldForest |		// wall,rock,forest not 100% clear?
				MapFieldCoastAllowed |
				MapFieldWaterAllowed |		// can't build on this
				MapFieldUnpassable;		// FIXME: I think shouldn't be used
			break;
		case UnitTypeNaval:
			mask = MapFieldSeaUnit |
				MapFieldBuilding |		// already occuppied
				MapFieldCoastAllowed |
				MapFieldLandAllowed |		// can't build on this
				MapFieldUnpassable;		// FIXME: I think shouldn't be used
			break;
		case UnitTypeFly:
			mask = MapFieldAirUnit;		// already occuppied
			break;
		default:
			DebugLevel1Fn("Were moves this unit?\n");
			if (unit) {
				TheMap.Fields[unit->X + unit->Y * TheMap.Width].Flags |= j;
			}
			return 0;
	}
#else

	mask = type->MovementMask;

#endif

	player = NULL;

	if (unit && unit->Player->Type == PlayerPerson) {
		player = unit->Player;
	}

	for (h = type->TileHeight; h--;) {
		for (w = type->TileWidth; w--;) {
			if (!CanBuildOn(x + w, y + h, mask)) {
				if (unit) {
					TheMap.Fields[unit->X + unit->Y * TheMap.Width].Flags |= j;
				}
				return 0;
			}
			if (player && !IsMapFieldExplored(player, x + w, y + h)) {
				return 0;
			}
		}
	}
	if (unit) {
		TheMap.Fields[unit->X + unit->Y * TheMap.Width].Flags |= j;
	}

	//
	//		We can build here: check distance to gold mine/oil patch!
	//
	return CanBuildHere(type, x, y);
}

/*----------------------------------------------------------------------------
  --		Finding units
  ----------------------------------------------------------------------------*/

/**
**		Find the closest piece of wood for an unit.
**
**		@param unit		The unit.
**		@param x		OUT: Map X position of tile.
**		@param y		OUT: Map Y position of tile.
*/
global int FindWoodInSight(const Unit* unit, int* x, int* y)
{
	return FindTerrainType(UnitMovementMask(unit), 0, MapFieldForest, 9999,
		unit->Player, unit->X, unit->Y, x, y);
}

/**
**		Find the closest piece of terrain with the given flags.
**
**		@param movemask		The movement mask to reach that location.
**		@param resmask		Result tile mask.
**		@param rvresult Return a tile that doesn't match.
**		@param range		Maximum distance for the search.
**		@param player		Only search fields explored by player
**		@param x		Map X start position for the search.
**		@param y		Map Y start position for the search.
**
**		@param px		OUT: Map X position of tile.
**		@param py		OUT: Map Y position of tile.
**
**		@note				Movement mask can be 0xFFFFFFFF to have no effect
**						Range is not circular, but square.
**						Player is ignored if nil(search the entire map)
**						Use rvresult if you search for a til;e that doesn't
**						match resmask. Like for a tile where an unit can go
**						with it's movement mask.
**
**		@return				True if wood was found.
*/
global int FindTerrainType(int movemask, int resmask, int rvresult, int range,
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
	size = min(TheMap.Width * TheMap.Height / 4, range * range * 5);
	points = malloc(size * sizeof(*points));

	//		Make movement matrix. FIXME: can create smaller matrix.
	matrix = CreateMatrix();
	w = TheMap.Width + 2;
	matrix += w + w + 2;
	points[0].X = x;
	points[0].Y = y;
	rp = 0;
	matrix[x + y * w] = 1;				// mark start point
	ep = wp = 1;						// start with one point
	cdist = 0;								// current distance is 0

	//
	//		Pop a point from stack, push all neighbors which could be entered.
	//
	for (;;) {
		while (rp != ep) {
			rx = points[rp].X;
			ry = points[rp].Y;
			DebugLevel3("%d,%d\n" _C_ rx _C_ ry);
			for (i = 0; i < 8; ++i) {				// mark all neighbors
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
				//		Look if found what was required.
				if (rvresult ? CanMoveToMask(x, y, resmask) : !CanMoveToMask(x, y, resmask)) {
					*px = x;
					*py = y;
					DebugLevel3("Found it! %X %X\n" _C_ TheMap.Fields[x+y*TheMap.Width].Flags _C_ resmask);
					free(points);
					return 1;
				}
				if (CanMoveToMask(x, y, movemask)) {		// reachable
					*m = 1;
					points[wp].X = x;				// push the point
					points[wp].Y = y;
					if (++wp >= size) {				// round about
						wp = 0;
					}
					if (wp == ep) {
						//  We are out of points, give up!
						DebugLevel0Fn("Ran out of points the hard way, beware.\n");
						break;
					}
				} else {						// unreachable
					*m = 99;
				}
			}
			if (++rp >= size) {						// round about
				rp = 0;
			}
		}
		++cdist;
		if (rp == wp || cdist >= range) {		// unreachable, no more points available
			break;
		}
		//		Continue with next set.
		ep = wp;
	}
	free(points);
	return 0;
}

/**
**		Find Resource.
**
**		@param unit		The unit that wants to find a resource.
**		@param x		Closest to x
**		@param y		Closest to y
**		@param range	Maximum distance to the resource.
**		@param resource The resource id.
**
**		@note				This will return an usable resource building that
**						belongs to "player" or is neutral.
**
**		@return				NoUnitP or resource unit
*/
global Unit* FindResource(const Unit* unit, int x, int y, int range, int resource)
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
	size = min(TheMap.Width * TheMap.Height / 4, range * range * 5);
	points = malloc(size * sizeof(*points));

	//		Find the nearest gold depot
	if ((destu = FindDeposit(unit, x, y,range,resource))) {
		NearestOfUnit(destu, x, y, &destx, &desty);
	}
	bestd = 99999;
	//		Make movement matrix. FIXME: can create smaller matrix.
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
	matrix[x + y * w] = 1;				// mark start point
	ep = wp = 1;						// start with one point
	cdist = 0;								// current distance is 0
	bestmine = NoUnitP;

	//
	//		Pop a point from stack, push all neighbors which could be entered.
	//
	for (;;) {
		while (rp != ep) {
			rx = points[rp].X;
			ry = points[rp].Y;
			for (i = 0; i < 8; ++i) {				// mark all neighbors
				x = rx + xoffset[i];
				y = ry + yoffset[i];
				m = matrix + x + y * w;
				if (*m) {						// already checked
					continue;
				}

				if (!IsMapFieldExplored(unit->Player, x, y)) { // Unknown.
					continue;
				}

				//
				//		Look if there is a mine
				//
				if ((mine = ResourceOnMap(x, y, resource)) &&
						mine->Type->CanHarvest &&
						(mine->Player->Player == PlayerMax - 1 ||
							mine->Player == unit->Player ||
							IsAllied(unit->Player, mine))) {
					if (destu) {
						n = max(abs(destx - x), abs(desty - y));
						if (n < bestd) {
							bestd = n;
							bestmine = mine;
						}
						*m = 99;
					} else {						// no goal take the first
						free(points);
						return mine;
					}
				}

				if (CanMoveToMask(x, y, mask)) {		// reachable
					*m = 1;
					points[wp].X = x;				// push the point
					points[wp].Y = y;
					if (++wp >= size) {				// round about
						wp = 0;
					}
					if (wp == ep) {
						//  We are out of points, give up!
						break;
					}
				} else {						// unreachable
					*m = 99;
				}
			}
			if (++rp >= size) {						// round about
				rp = 0;
			}
		}
		//		Take best of this frame, if any.
		if (bestd != 99999) {
			free(points);
			return bestmine;
		}
		++cdist;
		if (rp == wp || cdist >= range) {		// unreachable, no more points available
			break;
		}
		//		Continue with next set.
		ep = wp;
	}
	DebugLevel3Fn("no resource found\n");
	free(points);
	return NoUnitP;
}

/**
**		Find deposit. This will find a deposit for a resource
**
**		@param unit		The unit that wants to find a resource.
**		@param x		Closest to x
**		@param y		Closest to y
**		@param range	Maximum distance to the deposit.
**		@param resource		Resource to find deposit from.
**
**		@note				This will return a reachable allied depot.
**
**		@return				NoUnitP or deposit unit
*/
global Unit* FindDeposit(const Unit* unit, int x, int y, int range, int resource)
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
	size = min(TheMap.Width * TheMap.Height / 4, range * range * 5);
	points = malloc(size * sizeof(*points));

	//		Make movement matrix. FIXME: can create smaller matrix.
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
	matrix[x + y * w] = 1;				// mark start point
	ep = wp = 1;						// start with one point
	cdist = 0;								// current distance is 0

	DebugLevel3Fn("Searching for a deposit(%d,%d,%d,%d,%s)" _C_
		UnitNumber(unit) _C_ x _C_ y _C_ range _C_ DefaultResourceNames[resource]);
	//
	//		Pop a point from stack, push all neighbors which could be entered.
	//
	for (;;) {
		while (rp != ep) {
			rx = points[rp].X;
			ry = points[rp].Y;
			for (i = 0; i < 8; ++i) {				// mark all neighbors
				x = rx + xoffset[i];
				y = ry + yoffset[i];
				++nodes_searched;
				DebugLevel3("(%d,%d) " _C_ x _C_ y);
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
				//		Look if there is a deposit
				//
				if ((depot = ResourceDepositOnMap(x, y, resource)) &&
						((IsAllied(unit->Player, depot)) ||
							(unit->Player == depot->Player))) {
					free(points);
					DebugLevel3("Found a resource deposit at %d,%d\n" _C_ x _C_ y);
					return depot;
				}
				if (CanMoveToMask(x, y, mask)) {		// reachable
					*m = 1;
					points[wp].X = x;				// push the point
					points[wp].Y = y;
					if (++wp >= size) {				// round about
						wp = 0;
					}
					if (wp == ep) {
						//  We are out of points, give up!
						DebugLevel0Fn("Ran out of points the hard way, beware.\n");
						break;
					}
				} else {						// unreachable
					*m = 99;
				}
			}
			if (++rp >= size) {						// round about
				rp = 0;
			}
		}
		++cdist;
		if (rp == wp || cdist >= range) {		// unreachable, no more points available
			break;
		}
		//		Continue with next set.
		ep = wp;
	}
	DebugLevel3("No resource deposit found, after we searched %d nodes.\n" _C_ nodes_searched);
	free(points);
	return NoUnitP;
}

/**
**		Find the next idle worker
**
**		@param player		Player's units to search through
**		@param last		Previous idle worker selected
**
**		@return				NoUnitP or next idle worker
*/
global Unit* FindIdleWorker(const Player* player, const Unit* last)
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
  --		Select units
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
**  FIXME: If no unit, we could select near units?
**
**  @param ounit  Old selected unit.
**  @param x      X pixel position.
**  @param y      Y pixel position.
**
**  @return       An unit on x, y position.
*/
global Unit* UnitOnScreen(Unit* ounit, int x, int y)
{
	Unit** table;
	Unit* unit;
	Unit* nunit;
	Unit* funit;						// first possible unit
	UnitType* type;
	int flag;								// flag take next unit
	int gx;
	int gy;

	funit = NULL;
	nunit = NULL;
	flag = 0;
	if (!ounit) {						// no old on this position
		flag = 1;
	}
	for (table = Units; table < Units + NumUnits; ++table) {
		unit = *table;
		if (!UnitVisibleAsGoal(unit, ThisPlayer) && !ReplayRevealMap) {
			continue;
		}
		type = unit->Type;

		//
		//		Check if mouse is over the unit.
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
		//		This could be taken.
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
**		Let an unit die.
**
**		@param unit		Unit to be destroyed.
*/
global void LetUnitDie(Unit* unit)
{
	UnitType* type;

	unit->HP = 0;
	unit->Moving = 0;
	unit->TTL = 0;

	type = unit->Type;

	//		removed units,  just remove.
	if (unit->Removed) {
		DebugLevel0Fn("Killing a removed unit?\n");
		RemoveUnit(unit, NULL);
		UnitLost(unit);
		UnitClearOrders(unit);
		ReleaseUnit(unit);
		return;
	}

	PlayUnitSound(unit, VoiceDying);

	//
	//		Catapults,... explodes.
	//
	if (type->ExplodeWhenKilled) {
		MakeMissile(type->Explosion.Missile,
			unit->X * TileSizeX + type->TileWidth * TileSizeX / 2,
			unit->Y * TileSizeY + type->TileHeight * TileSizeY / 2,
			0, 0);
	}

	//
	// Handle Teleporter Destination Removal
	if (type->Teleporter && unit->Goal) {
		RemoveUnit(unit->Goal, NULL);
		UnitLost(unit->Goal);
		UnitClearOrders(unit->Goal);
		ReleaseUnit(unit->Goal);
		unit->Goal = NULL;
	}

	//
	//		Building,...
	//
	if (type->Building) {
		//
		//		Building with units inside?
		//
		//
		//		During resource build, the worker holds the resource amount,
		//		but if canceling building the platform, the worker is already
		//		outside.
		if (type->GivesResource &&
				unit->Orders[0].Action == UnitActionBuilded &&
				unit->Data.Builded.Worker) {
			// Restore value for oil-patch
			unit->Value = unit->Data.Builded.Worker->Value;
		}
		DestroyAllInside(unit);

		RemoveUnit(unit, NULL);
		UnitLost(unit);
		UnitClearOrders(unit);

		// FIXME: buildings should get a die sequence

		if (type->CorpseType) {
			unit->State = unit->Type->CorpseScript;
			DebugCheck(type->TileWidth != type->CorpseType->TileWidth ||
					type->TileHeight != type->CorpseType->TileHeight);
			type = unit->Type = type->CorpseType;

			if (!type->Sprite) {
				LoadUnitTypeSprite(type);
			}
			unit->IX = (type->Width - VideoGraphicWidth(type->Sprite)) / 2;
			unit->IY = (type->Height - VideoGraphicHeight(type->Sprite)) / 2;

			unit->SubAction = 0;
			//unit->Removed = 0;
			unit->Frame = 0;
			unit->Orders[0].Action = UnitActionDie;

			DebugCheck(!unit->Type->Animations ||
				!unit->Type->Animations->Die);
			UnitShowAnimation(unit, unit->Type->Animations->Die);
			DebugLevel0Fn("Frame %d\n" _C_ unit->Frame);
			unit->CurrentSightRange = type->Stats[unit->Player->Player].SightRange;
			MapMarkUnitSight(unit);
		} else {
			// no corpse available
			MapMarkUnitSight(unit);
			MapUnmarkUnitSight(unit);
			unit->CurrentSightRange = 0;
		}
		return;
	}

	// Transporters lose their units
	if (unit->Type->Transporter) {
		DestroyAllInside(unit);
	}

	RemoveUnit(unit, NULL);
	UnitLost(unit);
	UnitClearOrders(unit);

	//
	//		Unit has death animation.
	//

	// Not good: UnitUpdateHeading(unit);
	unit->SubAction = 0;
	//unit->Removed = 0;
	unit->State = 0;
	unit->Reset = 0;
	unit->Wait = 1;
	unit->Orders[0].Action = UnitActionDie;

	if (unit->Type->CorpseType) {
		unit->CurrentSightRange = unit->Type->CorpseType->Stats[unit->Player->Player].SightRange;
	} else {
		unit->CurrentSightRange = 0;
	}
	MapMarkUnitSight(unit);
}

/**
**		Destroy all units inside unit.
*/
global void DestroyAllInside(Unit* source)
{
	Unit* unit;
	int i;

	// No Corpses, we are inside something, and we can't be seen
	unit = source->UnitInside;
	for (i = source->InsideCount; i; --i, unit = unit->NextContained) {
		RemoveUnit(unit, NULL);
		UnitLost(unit);
		UnitClearOrders(unit);
		ReleaseUnit(unit);
	}
}


/*----------------------------------------------------------------------------
  --		Unit AI
  ----------------------------------------------------------------------------*/

/**
**		Unit is hit by missile or other damage.
**
**		@param attacker		Unit that attacks.
**		@param target		Unit that is hit.
**		@param damage		How many damage to take.
*/
global void HitUnit(Unit* attacker, Unit* target, int damage)
{
	UnitType* type;
	Unit* goal;
	unsigned long lastattack;

	if (!damage) {						// Can now happen by splash damage
#ifdef DEBUG
		if (!GodMode) {
			DebugLevel0Fn("Warning no damage, try to fix by caller?\n");
		}
#endif
		return;
	}

	DebugCheck(damage == 0 || target->HP == 0 || target->Type->Vanishes);

	if (target->UnholyArmor > 0) {
		// vladi: units with active UnholyArmour are invulnerable
		return;
	}

	if (target->Removed) {
		DebugLevel0Fn("Removed target hit\n");
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
			//		One help cry each 2 second is enough
			//		If on same area ignore it for 2 minutes.
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

	if (target->HP <= damage) {		// unit is killed or destroyed
		//  increase scores of the attacker, but not if attacking it's own units.
		//  prevents cheating by killing your own units.
		if (attacker && (target->Player->Enemy & (1 << attacker->Player->Player))) {
			attacker->Player->Score += target->Type->Points;
			if (type->Building) {
				attacker->Player->TotalRazings++;
			} else {
				attacker->Player->TotalKills++;
			}
#ifdef USE_HP_FOR_XP
			attacker->XP += target->HP;
#else
			attacker->XP += target->Type->Points;
#endif
			attacker->Kills++;
		}
		LetUnitDie(target);
		return;
	}
	target->HP -= damage;
#ifdef USE_HP_FOR_XP
	if (attacker) {
		attacker->XP += damage;
	}
#endif

	// FIXME: this is dumb. I made repairers capture. crap.
	// david: capture enemy buildings
	// Only worker types can capture.
	// Still possible to destroy building if not careful (too many attackers)
	if (EnableBuildingCapture && attacker &&
			type->Building && target->HP <= damage * 3 &&
			IsEnemy(attacker->Player, target) &&
			attacker->Type->RepairRange) {
		ChangeUnitOwner(target, attacker->Player);
		CommandStopUnit(attacker);		// Attacker shouldn't continue attack!
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
	//		Unit is working?
	//
	if (target->Orders[0].Action != UnitActionStill) {
		return;
	}

	//
	//		Attack units in range (which or the attacker?)
	//
	if (attacker && !type->Coward) {
		if (type->CanAttack && target->Stats->Speed) {
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
	//		FIXME: Can't attack run away.
	//
	if (!type->Building) {
		int x;
		int y;
		int d;

		DebugLevel3Fn("Unit at %d, %d attacked from %d, %d, running away.\n" _C_
				target->X _C_ target->Y _C_ attacker->X _C_ attacker->Y);
		x = target->X - attacker->X;
		y = target->Y - attacker->Y;
		d = isqrt(x * x + y * y);
		x = target->X + (x * 5) / d;
		y = target->Y + (y * 5) / d;
		CommandStopUnit(target);
		CommandMove(target, x + (SyncRand() & 3), y + (SyncRand() & 3), 0);
	}
}

/*----------------------------------------------------------------------------
  --		Conflicts
  ----------------------------------------------------------------------------*/

/**
**		Returns the map distance between two points.
**
**		@param x1		X map tile position.
**		@param y1		Y map tile position.
**		@param x2		X map tile position.
**		@param y2		Y map tile position.
**
**		@return				The distance between in tiles.
*/
global int MapDistance(int x1, int y1, int x2, int y2)
{
	return isqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

/**
**		Returns the map distance between two points with unit type.
**
**		@param x1		X map tile position.
**		@param y1		Y map tile position.
**		@param type		Unit type to take into account.
**		@param x2		X map tile position.
**		@param y2		Y map tile position.
**
**		@return				The distance between in tiles.
*/
global int MapDistanceToType(int x1, int y1, const UnitType* type, int x2, int y2)
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

	DebugLevel3("\tDistance %d,%d -> %d,%d = %d\n" _C_
		x1 _C_ y1 _C_ x2 _C_ y2 _C_ (dy<dx) ? dx : dy);

	return isqrt(dy * dy + dx * dx);
}

/**
**		Returns the map distance to unit.
**
**		@param x		X map tile position.
**		@param y		Y map tile position.
**		@param dest		Distance to this unit.
**
**		@return				The distance between in tiles.
*/
global int MapDistanceToUnit(int x, int y, const Unit* dest)
{
	return MapDistanceToType(x, y, dest->Type, dest->X, dest->Y);
}

/**
**		Returns the map distance between two units.
**
**		@param src		Distance from this unit.
**		@param dst		Distance  to  this unit.
**
**		@return				The distance between in tiles.
*/
global int MapDistanceBetweenUnits(const Unit* src, const Unit* dst)
{
	int dx;
	int dy;
	int x1;
	int x2;
	int y1;
	int y2;

	x1 = src->X;
	y1 = src->Y;
	x2 = dst->X;
	y2 = dst->Y;

	if (x1 + src->Type->TileWidth <= x2) {
		dx = x2 - x1 - src->Type->TileWidth + 1;
		if (dx < 0) {
			dx = 0;
		}
	} else {
		dx = x1 - x2 - dst->Type->TileWidth + 1;
		if (dx < 0) {
			dx = 0;
		}
	}

	if (y1 + src->Type->TileHeight <= y2) {
		dy = y2 - y1 - src->Type->TileHeight + 1;
	} else {
		dy = y1 - y2 - dst->Type->TileHeight + 1;
		if (dy < 0) {
			dy = 0;
		}
	}

	return isqrt(dy * dy + dx * dx);
}

/**
**		Compute the distance from the view point to a given point.
**
**		@param x		X map tile position.
**		@param y		Y map tile position.
**
**		@todo
**				FIXME: is it the correct place to put this function in?
*/
global int ViewPointDistance(int x, int y)
{
	const Viewport *vp;

	// first compute the view point coordinate
	vp = TheUI.SelectedViewport;

	// then use MapDistance
	return MapDistance(vp->MapX + vp->MapWidth / 2,
		vp->MapY + vp->MapHeight / 2, x, y);
}

/**
**		Compute the distance from the view point to a given unit.
**
**		@param dest		Distance to this unit.
**
**		@todo
**				FIXME: is it the correct place to put this function in?
*/
global int ViewPointDistanceToUnit(const Unit* dest)
{
	const Viewport* vp;

	// first compute the view point coordinate
	vp = TheUI.SelectedViewport;
	// then use MapDistanceToUnit
	return MapDistanceToUnit(vp->MapX + vp->MapWidth / 2,
		vp->MapY + vp->MapHeight / 2, dest);
}

/**
**		Check if unit is an enemy.
**
**		@param player		The source player.
**		@param dest		The destination unit.
**
**		@return				Returns true, if the destination unit is an enemy.
*/
global int IsEnemy(const Player* player, const Unit* dest)
{
	return player->Enemy & (1 << dest->Player->Player);
}

/**
**		Check if unit is allied.
**
**		@param player		The source player.
**		@param dest		The destination unit.
**
**		@return				Returns true, if the destination unit is allied.
*/
global int IsAllied(const Player* player, const Unit* dest)
{
	return player->Allied & (1 << dest->Player->Player);
}

/**
**		Check if unit is shared vision.
**
**		@param player		The source player.
**		@param dest		The destination unit.
**
**		@return				Returns true, if the destination unit is shared
**						vision.
*/
global int IsSharedVision(const Player* player, const Unit* dest)
{
	return (player->SharedVision & (1 << dest->Player->Player)) &&
		(dest->Player->SharedVision & (1 << player->Player));
}

/**
**		Can the source unit attack the destination unit.
**
**		@param source		Unit type pointer of the attacker.
**		@param dest		Unit type pointer of the target.
*/
global int CanTarget(const UnitType* source, const UnitType* dest)
{
	int i;

	for (i = 0; i < NumberBoolFlag; i++) {
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

/*----------------------------------------------------------------------------
  --		SAVE/LOAD
  ----------------------------------------------------------------------------*/

/**
**		Generate a unit reference, a printable unique string for unit.
*/
global char* UnitReference(const Unit* unit)
{
	char* ref;

	ref = malloc(10);
	sprintf(ref, "U%04X", UnitNumber(unit));
	return ref;
}

/**
**  Save an order.
**
**  @param order  Order who should be saved.
**  @param file   Output file.
*/
global void SaveOrder(const Order* order, CLFile* file)
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
			DebugLevel0Fn("Unknown action in order\n");
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
**		Save the state of an unit to file.
**
**		@param unit		Unit pointer to be saved.
**		@param file		Output file.
*/
global void SaveUnit(const Unit* unit, CLFile* file)
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
	CLprintf(file, "\"%sframe\", %d, ",
		unit->Frame < 0 ? "flipped-" : "",
		unit->Frame < 0 ? -unit->Frame - 1 : unit->Frame);
	if (unit->Seen.Frame != UnitNotSeen) {
		CLprintf(file, "\"%sseen\", %d, ",
			unit->Seen.Frame < 0 ? "flipped-" : "",
			unit->Seen.Frame < 0 ? -unit->Seen.Frame - 1 : unit->Seen.Frame);
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

	CLprintf(file, "\"group-id\", %d,\n  ", unit->GroupId);
	CLprintf(file, "\"last-group\", %d,\n  ", unit->LastGroup);

	CLprintf(file, "\"value\", %d,\n  ", unit->Value);
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
	CLprintf(file, " \"rs\", %d,", unit->Rs);
	CLprintf(file, " \"units-contained-count\", %d,", unit->InsideCount);
	CLprintf(file, "\n  \"units-contained\", {");
	uins = unit->UnitInside;
	for (i = unit->InsideCount; i; --i, uins = uins->NextContained) {
		CLprintf(file, "\"%s\",", ref = UnitReference(uins));
		free(ref);
		if (i > 1) {
			CLprintf(file, " ");
		}
	}
	CLprintf(file, "},\n  ");
	CLprintf(file, "\"order-count\", %d,\n  ", unit->OrderCount);
	CLprintf(file, "\"order-flush\", %d,\n  ", unit->OrderFlush);
	CLprintf(file, "\"orders\", {");
	for (i = 0; i < MAX_ORDERS; ++i) {
		CLprintf(file, "\n	");
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
			CLprintf(file, "\"count\", %d, ", unit->Data.Train.Count);
			CLprintf(file, "\"queue\", {");
			for (i = 0; i < MAX_UNIT_TRAIN; ++i) {
				if (i < unit->Data.Train.Count) {
					CLprintf(file, "\"%s\", ", unit->Data.Train.What[i]->Ident);
				} else {
					/* this slot is currently unused */
					CLprintf(file, "\"unit-none\", ");
				}
			}
			CLprintf(file, "}}");
			break;
		default:
			CLprintf(file, ",\n  \"data-move\", {");
			if (unit->Data.Move.Fast) {
				CLprintf(file, "\"fast\", ");
			}
			if (unit->Data.Move.Length) {
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
		for (i = 0; i < SpellTypeCount; i++) {
			if (unit->AutoCastSpell[i]) {
				CLprintf(file, ",\n  \"auto-cast\", \"%s\"", SpellTypeTable[i]->Ident);
			}
		}
	}

	CLprintf(file, ")\n");
}

/**
**		Save state of units to file.
**
**		@param file		Output file.
*/
global void SaveUnits(CLFile* file)
{
	Unit** table;
	int i;
	unsigned char SlotUsage[MAX_UNIT_SLOTS / 8 + 1];
	int InRun;
	int RunStart;
	int j;

	CLprintf(file, "\n--- -----------------------------------------\n");
	CLprintf(file, "--- MODULE: units $Id$\n\n");

#if 0
	//
	//		Local variables
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
		if ((i + 1) % 16 == 0)				// 16 numbers per line
			CLprintf(file, "\n");
	}

#else
#define SlotUsed(slot)		(SlotUsage[(slot) / 8] & (1 << ((slot) % 8)))
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

	CLprintf (file, ")\n");

	for (table = Units; table < &Units[NumUnits]; ++table) {
		SaveUnit(*table, file);
	}
}

/*----------------------------------------------------------------------------
  --		Initialize/Cleanup
  ----------------------------------------------------------------------------*/

/**
**		Initialize unit module.
*/
global void InitUnits(void)
{
}

/**
**		Cleanup unit module.
*/
global void CleanUnits(void)
{
	Unit** table;
	Unit* unit;

	//
	//		Free memory for all units in unit table.
	//
	for (table = Units; table < &Units[NumUnits]; ++table) {
		free((*table)->AutoCastSpell);
		free(*table);
		*table = NULL;
	}

	//
	//		Release memory of units in release queue.
	//
	while ((unit = ReleasedHead)) {
		ReleasedHead = unit->Next;
		free(unit);
	}

	InitUnitsMemory();

	XpDamage = 0;
	FancyBuildings = 0;
	HelpMeLastCycle = 0;
}

//@}
