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
/**@name unit.cpp - The units. */
//
//      (c) Copyright 1998-2008 by Lutz Sammer and Jimmy Salmon
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
#include <sstream>
#include <iomanip>

#include "stratagus.h"

#include "unit.h"
#include "unit_manager.h"
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
#include "luacallback.h"
#include "construct.h"
#include "iolib.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

CUnit *Units[MAX_UNIT_SLOTS];             /// Array of used slots
int NumUnits;                             /// Number of slots used

bool EnableTrainingQueue;                 /// Config: training queues enabled
bool EnableBuildingCapture;               /// Config: capture buildings enabled
bool RevealAttacker;                      /// Config: reveal attacker enabled

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

void CUnit::COrder::Release(void) {
	if (Goal) {
		Goal->RefsDecrease();
		Goal = NoUnitP;
	}
	//FIXME: Hardcoded wood
	if (Action == UnitActionResource && CurrentResource != WoodCost &&
		Arg1.Resource.Mine ) {
		Arg1.Resource.Mine->RefsDecrease();
		Arg1.Resource.Mine = NoUnitP;
	}
}

CUnit::COrder::COrder(const CUnit::COrder &ths): Goal(ths.Goal), Range(ths.Range),
	 MinRange(ths.MinRange), Width(ths.Width), Height(ths.Height),
	 Action(ths.Action), CurrentResource(ths.CurrentResource),
	 X(ths.X), Y(ths.Y)
 {
	if (Goal) {
		Goal->RefsIncrease();
	}

	memcpy(&Arg1, &ths.Arg1, sizeof(Arg1));

	//FIXME: Hardcoded wood
	if (Action == UnitActionResource &&
		 CurrentResource != WoodCost && Arg1.Resource.Mine) {
		 Arg1.Resource.Mine->RefsIncrease();
	}
}

CUnit::COrder& CUnit::COrder::operator=(const CUnit::COrder &rhs) {
	if (this != &rhs) {

		//FIXME: Hardcoded wood
		if (Action == UnitActionResource &&
			 CurrentResource != WoodCost && Arg1.Resource.Mine) {
			 Arg1.Resource.Mine->RefsDecrease();
		}

		Action = rhs.Action;
		Range = rhs.Range;
		MinRange = rhs.MinRange;
		Width = rhs.Width;
		Height = rhs.Height;
		CurrentResource = rhs.CurrentResource;
		SetGoal(rhs.Goal);
		X = rhs.X;
		Y = rhs.Y;
		memcpy(&Arg1, &rhs.Arg1, sizeof(Arg1));

		//FIXME: Hardcoded wood
		if(Action == UnitActionResource &&
			 CurrentResource != WoodCost && Arg1.Resource.Mine) {
			 Arg1.Resource.Mine->RefsIncrease();
		}
	}
	return *this;
}

bool CUnit::COrder::CheckRange(void)
{
	return (Range <= Map.Info.MapWidth || Range <= Map.Info.MapHeight);
}


/**
**  Release an unit.
**
**  The unit is only released, if all references are dropped.
*/
void CUnit::Release(bool final)
{
	CUnit *temp;

	Assert(Type); // already free.
	Assert(OrderCount == 1);
	Assert(!CurrentOrder()->HasGoal());
	// Must be removed before here
	Assert(Removed);

	//
	// First release, remove from lists/tables.
	//
	if (!Destroyed) {
		DebugPrint("%d: First release %d\n" _C_ Player->Index _C_ Slot);

		//
		// Are more references remaining?
		//
		Destroyed = 1; // mark as destroyed

		if (Container && !final) {
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

	Type = NULL;

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

	if (UnitTypeVar.GetNumberVariable()) {
		Assert(!Variable);
		Variable = new CVariable[UnitTypeVar.GetNumberVariable()];
		memcpy(Variable, Type->Variable,
			UnitTypeVar.GetNumberVariable() * sizeof(*Variable));
	} else {
		Variable = NULL;
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
	Active = 1;

	Removed = 1;

	Assert(Orders.empty());

	Orders.push_back(new COrder);

	OrderCount = 1; // No orders
	CurrentOrder()->Action = UnitActionStill;
	CurrentOrder()->X = CurrentOrder()->Y = -1;
	Assert(!CurrentOrder()->HasGoal());
	NewOrder.Action = UnitActionStill;
	NewOrder.X = NewOrder.Y = -1;
	Assert(!NewOrder.HasGoal());
	SavedOrder.Action = UnitActionStill;
	SavedOrder.X = SavedOrder.Y = -1;
	Assert(!SavedOrder.HasGoal());
	CriticalOrder.Action = UnitActionStill;
	CriticalOrder.X = CriticalOrder.Y = -1;
	Assert(!CriticalOrder.HasGoal());
}

/**
**  Restore the saved order
**
**  @return      True if the saved order was restored
*/
bool CUnit::RestoreOrder(void)
{
	if (this->SavedOrder.Action != UnitActionStill) {
		// Restart order state.
		this->State = 0;
		this->SubAction = 0;

		Assert(!this->CurrentOrder()->HasGoal());

		//copy
		*(this->CurrentOrder()) = this->SavedOrder;

		this->CurrentResource = this->SavedOrder.CurrentResource;

		NewResetPath(this);

		// This isn't supported
		Assert(!this->SavedOrder.HasGoal());

		this->SavedOrder.Action = UnitActionStill;
		this->SavedOrder.ClearGoal();
		return true;
	}
	return false;
}

/**
**  Store the Current order
**
**  @return      True if the current order was saved
*/
bool CUnit::StoreOrder(void)
{
	if (this->SavedOrder.Action == UnitActionStill) {
		// Save current order to come back or to continue it.
		this->SavedOrder = *(this->CurrentOrder());
		CUnit *temp = this->SavedOrder.GetGoal();
		if (temp) {
			DebugPrint("Have goal to come back %d\n" _C_
					UnitNumber(temp));
			this->SavedOrder.X = temp->X + temp->Type->TileWidth / 2;
			this->SavedOrder.Y = temp->Y + temp->Type->TileHeight / 2;
			this->SavedOrder.MinRange = 0;
			this->SavedOrder.Range = 0;
			this->SavedOrder.ClearGoal();
		}
		return true;
	}
	return false;
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
	if (!type->Vanishes && CurrentAction() != UnitActionDie) {
		PlayerSlot = player->Units + player->TotalNumUnits++;
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
		*PlayerSlot = this;

		player->UnitTypesCount[type->Slot]++;
		player->Demand += type->Demand; // food needed
	}


	// Don't Add the building if it's dieing, used to load a save game
	if (type->Building && CurrentAction() != UnitActionDie) {
		// FIXME: support more races
		if (type != UnitTypeOrcWall && type != UnitTypeHumanWall) {
			player->NumBuildings++;
		}
	}
	Player = player;
	Stats = &type->Stats[Player->Index];
	Colors = &player->UnitColors;
	if (!SaveGameLoading) {
		if (UnitTypeVar.GetNumberVariable()) {
			Assert(Variable);
			Assert(Stats->Variables);
			memcpy(Variable, Stats->Variables,
				UnitTypeVar.GetNumberVariable() * sizeof(*Variable));
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
**  @param unit    Unit to (un)mark.
**  @param x       X coord of first container of unit.
**  @param y       Y coord of first container of unit.
**  @param width   Width of the first container of unit.
**  @param height  Height of the first container of unit.
**  @param f       Function to (un)mark for normal vision.
**  @param f2        Function to (un)mark for cloaking vision.
*/
static void MapMarkUnitSightRec(const CUnit *unit, int x, int y, int width, int height,
	MapMarkerFunc *f, MapMarkerFunc *f2)
{
	CUnit *unit_inside;

	Assert(unit);
	Assert(f);
	MapSight(unit->Player, x, y, width, height,
		unit->Container ? unit->Container->CurrentSightRange : unit->CurrentSightRange, f);

	if (unit->Type && unit->Type->DetectCloak && f2) {
		MapSight(unit->Player, x, y, width, height,
			unit->Container ? unit->Container->CurrentSightRange : unit->CurrentSightRange, f2);
	}

	unit_inside = unit->UnitInside;
	for (int i = unit->InsideCount; i--; unit_inside = unit_inside->NextContained) {
		MapMarkUnitSightRec(unit_inside, x, y, width, height, f, f2);
	}
}

/**
**  Return the unit not transported, by viewing the container recursively.
**
**  @param unit  unit from where look the first conatiner.
**
**  @return      Container of container of ... of unit. It is not null.
*/
static CUnit *GetFirstContainer(const CUnit *unit)
{
	Assert(unit);
	while (unit->Container) {
		unit = unit->Container;
	}
	return (CUnit *)unit;
}

/**
**  Mark on vision table the Sight of the unit
**  (and units inside for transporter)
**
**  @param unit  unit to unmark its vision.
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
		MapMarkTileSight, MapMarkTileDetectCloak);

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
		MapUnmarkTileSight, MapUnmarkTileDetectCloak);

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
**  @param unit  unit to update SightRange
**
**  @internal before using it, MapUnmarkUnitSight(unit)
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
**  @param unit  unit to mark.
*/
void MarkUnitFieldFlags(const CUnit *unit)
{
	Assert(unit);
	CMapField *mf;
	const unsigned int flags = unit->Type->FieldFlags; //
	int w, h = unit->Type->TileHeight;          // Tile height of the unit.
	const int width = unit->Type->TileWidth;          // Tile width of the unit.
	unsigned int index = unit->Offset;
	do {
		mf = Map.Field(index);
		w = width;
		do {
			mf->Flags |= flags;
			++mf;
		} while (--w);
		index += Map.Info.MapWidth;
	} while (--h);
}

struct _UnmarkUnitFieldFlags {
	const CUnit *const main;
	CMapField *mf;
	_UnmarkUnitFieldFlags(const CUnit *const unit)
		 : main(unit) {}
	inline void operator () (CUnit *const unit) {
		if (main != unit && unit->CurrentAction() != UnitActionDie) {
			mf->Flags |= unit->Type->FieldFlags;
		}
	}
};


/**
**  Mark the field with the FieldFlags.
**
**  @param unit  unit to mark.
*/
void UnmarkUnitFieldFlags(const CUnit *unit)
{
	Assert(unit);
	CMapField *mf;
	const unsigned int flags = ~unit->Type->FieldFlags; //
	int w, h = unit->Type->TileHeight;          // Tile height of the unit.
	const int width = unit->Type->TileWidth;          // Tile width of the unit.
	unsigned int index = unit->Offset;

	_UnmarkUnitFieldFlags funct(unit);

	do {
		mf = Map.Field(index);
		w = width;
		do {
			mf->Flags &= flags;//clean flags
			funct.mf = mf;
			mf->UnitCache.for_each(funct);
			++mf;
		} while(--w);
		index += Map.Info.MapWidth;
	} while(--h);
}

/**
**  Add unit to a container. It only updates linked list stuff.
**
**  @param host  Pointer to container.
*/
void CUnit::AddInContainer(CUnit *host)
{
	Assert(host && Container == NoUnitP);
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
**  @param unit  Pointer to unit.
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
**  @param unit  unit to move.
**  @param x     X map tile position.
**  @param y     Y map tile position.
**
**  @internal before use it, Map.Remove(unit), MapUnmarkUnitSight(unit)
**  and after Map.Insert(unit), MapMarkUnitSight(unit)
**  are often necessary. Check Flag also for Pathfinder.
*/
static void UnitInXY(CUnit *unit, int x, int y)
{
	Assert(unit);
	CUnit *unit_inside = unit->UnitInside;

	unit->X = x;
	unit->Y = y;
	unit->Offset = Map.getIndex(x,y);

	if(!unit->Container) {
		//Only Top Units
		const CMapField *const mf = Map.Field(unit->Offset);
		const CPlayer *const p = unit->Player;
		for (int player = 0; player < NumPlayers; ++player) {
			if(player != p->Index && mf->Guard[player] && p->IsEnemy(player)) {
				Players[player].AutoAttackTargets.InsertS(unit);
				unit->RefsIncrease();
			}
		}
	}
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
	Map.Remove(this);
	UnmarkUnitFieldFlags(this);

	Assert(UnitCanBeAt(this, x, y));
	// Move the unit.
	UnitInXY(this, x, y);

	Map.Insert(this);
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
		MapUnmarkUnitGuard(this);
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
	Map.Insert(this);
	//  Calculate the seen count.
	UnitCountSeen(this);
	// Vision
	MapMarkUnitSight(this);
}

/**
**  Create new unit and place on map.
**
**  @param x       X map tile position.
**  @param y       Y map tile position.
**  @param type    Pointer to unit-type.
**  @param player  Pointer to owning player.
**
**  @return        Pointer to created unit.
*/
CUnit *MakeUnitAndPlace(int x, int y, CUnitType *type, CPlayer *player)
{
	CUnit *unit = MakeUnit(type, player);

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

	Map.Remove(this);
	MapUnmarkUnitSight(this);
	UnmarkUnitFieldFlags(this);
	MapUnmarkUnitGuard(this);

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
	if (UnitUnderCursor == this) {
		UnitUnderCursor.Reset();
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
	int i;

	Assert(unit);

	player = unit->Player;
	Assert(player);  // Next code didn't support no player!

	//
	//  Call back to AI, for killed or lost units.
	//
	if (player && player->AiEnabled) {
		AiUnitKilled(unit);
	} else {
		//
		//  Remove unit from its groups
		//
		if (unit->GroupId) {
			RemoveUnitFromGroups(unit);
		}
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

		if (unit->CurrentAction() != UnitActionBuilt) {
			player->UnitTypesCount[type->Slot]--;
		}
	}


	//
	//  Handle unit demand. (Currently only food supported.)
	//
	player->Demand -= type->Demand;

	//
	//  Update information.
	//
	if (unit->CurrentAction() != UnitActionBuilt) {
		player->Supply -= type->Supply;

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
	if (unit->CurrentAction() == UnitActionResearch) {
		unit->Player->UpgradeTimers.Upgrades[unit->Data.Research.Upgrade->ID] = 0;
	}

	DebugPrint("%d: Lost %s(%d)\n"
		_C_ unit->Player->Index
		_C_ unit->Type->Ident.c_str()
		_C_ UnitNumber(unit));

	// Destroy resource-platform, must re-make resource patch.
	if ((b = OnTopDetails(unit, NULL)) != NULL) {
		if (b->ReplaceOnDie && (unit->Type->GivesResource && unit->ResourcesHeld != 0)) {
			temp = MakeUnitAndPlace(unit->X, unit->Y, b->Parent, &Players[PlayerNumNeutral]);
			if (temp == NoUnitP) {
				DebugPrint("Unable to allocate Unit");
			} else {
				temp->ResourcesHeld = unit->ResourcesHeld;
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
		if (i != 0) {
			COrderPtr order = unit->Orders.back();
			delete order;
			unit->Orders.pop_back();
		} else unit->Orders[0]->Release();
	}
	unit->OrderCount = 1;
	unit->NewOrder.Release();
	unit->SavedOrder.Release();
	unit->CurrentOrder()->Action = UnitActionStill;
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
	// Handle unit supply. (Currently only food supported.)
	// Note an upgraded unit can't give more supply.
	//
	if (!upgrade) {
		player->Supply += type->Supply;
	}

	//
	// Update resources
	//
	for (int u = 1; u < MaxCosts; ++u) {
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
void NearestOfUnit(const CUnit *unit, int tx, int ty, int *dx, int *dy)
{
	int x = unit->X;
	int y = unit->Y;

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
**  @param unit  The unit to work on
*/
static void UnitFillSeenValues(CUnit *unit)
{
	// Seen values are undefined for visible units.
	unit->Seen.Y = unit->Y;
	unit->Seen.X = unit->X;
	unit->Seen.IY = unit->IY;
	unit->Seen.IX = unit->IX;
	unit->Seen.Frame = unit->Frame;
	unit->Seen.State = (unit->CurrentAction() == UnitActionBuilt) |
			((unit->CurrentAction() == UnitActionUpgradeTo) << 1);
	if (unit->CurrentAction() == UnitActionDie) {
		unit->Seen.State = 3;
	}
	unit->Seen.Type = unit->Type;
	unit->Seen.Constructed = unit->Constructed;
	if (unit->CurrentAction() == UnitActionBuilt) {
		unit->Seen.CFrame = unit->Data.Built.Frame;
	} else {
		unit->Seen.CFrame = NULL;
	}
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

template<const bool MARK>
struct _TileSeen {
	const CPlayer *player;
	int cloak;

	_TileSeen(const CPlayer *p , int c): player(p), cloak(c) {}

	inline void operator() ( CUnit *const unit) {
		if (cloak == (int)unit->Type->PermanentCloak) {
			const int p = player->Index;
			if(MARK) {
				//
				//  If the unit goes out of fog, this can happen for any player that
				//  this player shares vision with, and can't YET see the unit.
				//  It will be able to see the unit after the Unit->VisCount ++
				//
				if (!unit->VisCount[p]) {
					for (int pi = 0; pi < PlayerMax; ++pi) {
						if ((pi == p /*player->Index*/) ||
								player->IsBothSharedVision(&Players[pi])) {
							if (!unit->IsVisible(Players + pi)) {
								UnitGoesOutOfFog(unit, Players + pi);
							}
						}
					}
				}
				unit->VisCount[p/*player->Index*/]++;
			} else {
				/*
				 * HACK: UGLY !!!
				 * There is bug in Seen code conneded with
				 * UnitActionDie and Cloacked units.
				 */
				if(!unit->VisCount[p] && unit->CurrentAction() == UnitActionDie)
				{
					return;
				}

				Assert(unit->VisCount[p]);
				unit->VisCount[p]--;
				//
				//  If the unit goes under of fog, this can happen for any player that
				//  this player shares vision to. First of all, before unmarking,
				//  every player that this player shares vision to can see the unit.
				//  Now we have to check who can't see the unit anymore.
				//
				if (!unit->VisCount[p]) {
					for (int pi = 0; pi < PlayerMax; ++pi) {
						if (pi == p/*player->Index*/ ||
							player->IsBothSharedVision(&Players[pi])) {
							if (!unit->IsVisible(Players + pi)) {
								UnitGoesUnderFog(unit, Players + pi);
							}
						}
					}
				}
			}
		}
	}
};

/**
**  Mark all units on a tile as now visible.
**
**  @param player  The player this is for.
**  @param x       x location to check
**  @param y       y location to check
**  @param cloak   If we mark cloaked units too.
*/
void UnitsOnTileMarkSeen(const CPlayer *player, const unsigned int index, int cloak)
{
	_TileSeen<true> seen(player, cloak);
	Map.Field(index)->UnitCache.for_each(seen);
}

void UnitsOnTileMarkSeen(const CPlayer *player, int x, int y, int cloak)
{
	UnitsOnTileMarkSeen(player, Map.getIndex(x,y), cloak);
}


/**
**  This function unmarks units on x, y as seen. It uses a reference count.
**
**  @param player    The player to mark for.
**  @param x         x location to check if building is on, and mark as seen
**  @param y         y location to check if building is on, and mark as seen
**  @param cloak     If this is for cloaked units.
*/
void UnitsOnTileUnmarkSeen(const CPlayer *player,
				const unsigned int index, int cloak)
{
	_TileSeen<false> seen(player, cloak);
	Map.Field(index)->UnitCache.for_each(seen);
}

void UnitsOnTileUnmarkSeen(const CPlayer *player, int x, int y, int cloak)
{
	UnitsOnTileUnmarkSeen(player, Map.getIndex(x,y), cloak);
}


/**
**  Recalculates a units visiblity count. This happens really often,
**  Like every time a unit moves. It's really fast though, since we
**  have per-tile counts.
**
**  @param unit  pointer to the unit to check if seen
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

	const int height = unit->Type->TileHeight;          // Tile height of the unit.
	const int width = unit->Type->TileWidth;          // Tile width of the unit.
	unsigned int index;
	CMapField *mf;
	//  Calculate new VisCount values.
	for (p = 0; p < PlayerMax; ++p) {
		if (Players[p].Type != PlayerNobody) {
			newv = 0;
			y = height;
			index = unit->Offset;
			do {
				mf = Map.Field(index);
				x = width;
				do {
					if (unit->Type->PermanentCloak && unit->Player != &Players[p]) {
						if (mf->VisCloak[p]) {
							newv++;
						}
					} else {
						//  Icky ugly code trick. With NoFogOfWar we haveto be > 0;
						if (mf->Visible[p] > 1 - (Map.NoFogOfWar ? 1 : 0)) {
							newv++;
						}
					}
					++mf;
				} while(--x);
				index += Map.Info.MapWidth;
			} while(--y);
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
			if (!oldv[p] && newv) {
				UnitGoesOutOfFog(unit, Players + p);
				// Might have revealed a destroyed unit which caused it to
				// be released
				if (!unit->Type) {
					break;
				}
			}
			if (oldv[p] && !newv) {
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
		if (p != player->Index && player->IsBothSharedVision(&Players[p])) {
			if (VisCount[p]) {
				return true;
			}
		}
	}
	return false;
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
	//
	// Invisible units.
	//
	if (IsInvisibile(ThisPlayer)) {
		return false;
	}
	if (IsVisible(ThisPlayer) || ReplayRevealMap ||
			IsVisibleOnRadar(ThisPlayer))
	{
		return IsAliveOnMap();
	} else {
		return Type->VisibleUnderFog && Seen.State != 3 &&
			(Seen.ByPlayer & (1 << ThisPlayer->Index)) &&
			 !(Seen.Destroyed & (1 << ThisPlayer->Index));
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
	// Check if the graphic is inside the viewport.
	//
	int x = X * TileSizeX + IX - (Type->Width - Type->TileWidth * TileSizeX) / 2 + Type->OffsetX;
	int y = Y * TileSizeY + IY - (Type->Height - Type->TileHeight * TileSizeY) / 2 + Type->OffsetY;

	if (x + Type->Width < vp->MapX * TileSizeX + vp->OffsetX ||
			x > vp->MapX * TileSizeX + vp->OffsetX + (vp->EndX - vp->X) ||
			y + Type->Height < vp->MapY * TileSizeY + vp->OffsetY ||
			y > vp->MapY * TileSizeY + vp->OffsetY + (vp->EndY - vp->Y))
	{
		return false;
	}

	if (!ThisPlayer) {
		//FIXME: ARI: Added here for early game setup state by
		// MakeAndPlaceUnit() from LoadMap(). ThisPlayer not yet set,
		// so don't show anything until first real map-draw.
		DebugPrint("FIXME: ThisPlayer not set yet?!\n");
		return false;
	}

	// Those are never ever visible.
	if (IsInvisibile(ThisPlayer)) {
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
**  @return  True if visible, false otherwise.
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
**  @param sx  Out: Top left X tile map postion.
**  @param sy  Out: Top left Y tile map postion.
**  @param ex  Out: Bottom right X tile map postion.
**  @param ey  Out: Bottom right Y tile map postion.
**
**  @return    sx,sy,ex,ey defining area in Map
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
	if (Type->GivesResource) {
		DebugPrint("Resource transfer not supported\n");
	}
	newplayer->Demand += Type->Demand;
	newplayer->Supply += Type->Supply;
	if (Type->Building) {
		newplayer->NumBuildings++;
	}
	newplayer->UnitTypesCount[Type->Slot]++;

	UpdateForNewUnit(this, 1);
}

void CUnit::AssignWorkerToMine(CUnit *mine)
{
	CUnit *head = mine->Data.Resource.Workers;
/*
	DebugPrint("%d: Worker [%d] is adding into %s [%d] on %d pos\n"
					_C_ this->Player->Index _C_ this->Slot
					_C_ mine->Type->Name.c_str()
					_C_ mine->Slot
					_C_ mine->Data.Resource.Assigned);
*/
	this->RefsIncrease();
	this->NextWorker = head;
	mine->Data.Resource.Workers = this;
	mine->Data.Resource.Assigned++;
}

void CUnit::DeAssignWorkerFromMine(CUnit *mine)
{
	CUnit *prev = NULL,*worker = mine->Data.Resource.Workers;
/*
	DebugPrint("%d: Worker [%d] is removing from %s [%d] left %d units assigned\n"
					_C_ this->Player->Index _C_ this->Slot
					_C_ mine->Type->Name.c_str()
					_C_ mine->Slot
					_C_ mine->Data.Resource.Assigned);
*/
	for(int i = 0; NULL != worker; worker = worker->NextWorker,++i)
	{
		if (worker == this) {
			CUnit *next = worker->NextWorker;
			if (prev) {
				prev->NextWorker = next;
			}
			if (worker == mine->Data.Resource.Workers) {
				mine->Data.Resource.Workers = next;
			}
			worker->RefsDecrease();
			break;
		}
		prev = worker;
		Assert(i<=mine->Data.Resource.Assigned);
	}
	mine->Data.Resource.Assigned--;
	Assert(mine->Data.Resource.Assigned >= 0);
}


/**
**  Change the owner of all units of a player.
**
**  @param oldplayer    Old owning player.
**  @param newplayer    New owning player.
*/
static void ChangePlayerOwner(CPlayer *oldplayer, CPlayer *newplayer)
{
	CUnit *table[UnitMax];
	CUnit *unit;
	int i;
	int n;

	// NOTE: table is changed.
	n = oldplayer->TotalNumUnits;
	memcpy(table, oldplayer->Units, n * sizeof(CUnit *));
	for (i = 0; i < n; ++i) {
		unit = table[i];
		// Don't save the unit again(can happen when inside a town hall)
		if (unit->Player == newplayer) {
			continue;
		}
		unit->ChangeOwner(newplayer);
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
	NoRescueCheck = true;

	//
	//  Look if player could be rescued.
	//
	for (p = Players; p < Players + NumPlayers; ++p) {
		if (p->Type != PlayerRescuePassive && p->Type != PlayerRescueActive) {
			continue;
		}
		if (p->TotalNumUnits) {
			NoRescueCheck = false;
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
					n = Map.Select(
							unit->X - 1, unit->Y - 1,
							unit->X + unit->Type->TileWidth + 1,
							unit->Y + unit->Type->TileHeight + 1, around);
				} else {
					n = Map.Select(
							unit->X - 2, unit->Y - 2,
							unit->X + unit->Type->TileWidth + 2,
							unit->Y + unit->Type->TileHeight + 2, around);
				}
				//
				//  Look if ally near the unit.
				//
				for (i = 0; i < n; ++i) {
					if (around[i]->Type->CanAttack &&
							unit->IsAllied(around[i])) {
						//
						//  City center converts complete race
						//  NOTE: I use a trick here, centers could
						//        store gold. FIXME!!!
						if (unit->Type->CanStore[GoldCost]) {
							ChangePlayerOwner(p, around[i]->Player);
							break;
						}
						unit->RescuedFrom = unit->Player;
						unit->ChangeOwner(around[i]->Player);
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
void UnitUpdateHeading(CUnit *unit)
{
	int dir;
	int nextdir;
	bool neg;

	if (unit->Frame < 0) {
		unit->Frame = -unit->Frame - 1;
		neg = true;
	} else {
		neg = false;
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
**  @param unit  Unit to drop out.
**  @param gx    Goal X map tile position.
**  @param gy    Goal Y map tile position.
**  @param addx  Tile width of unit it's dropping out of.
**  @param addy  Tile height of unit it's dropping out of.
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
		Assert(!unit->CurrentOrder()->HasGoal());
		unit->CurrentOrder()->Action = UnitActionStill;
		unit->SubAction = 0;
	}
	DebugPrint("Drop out %d of %d\n" _C_ i _C_ source->Data.Resource.Active);
}


/*----------------------------------------------------------------------------
  -- Finding units
  ----------------------------------------------------------------------------*/

/**
**  Find the closest piece of wood for an unit.
**
**  @param unit    The unit.
**  @param x       OUT: Map X position of tile.
**  @param y       OUT: Map Y position of tile.
*/
int FindWoodInSight(const CUnit *unit, int *x, int *y)
{
	return FindTerrainType(unit->Type->MovementMask, 0, MapFieldForest, 9999,
		unit->Player, unit->X, unit->Y, x, y);
}

/**
**  Find the closest piece of terrain with the given flags.
**
**  @param movemask    The movement mask to reach that location.
**  @param resmask     Result tile mask.
**  @param rvresult    Return a tile that doesn't match.
**  @param range       Maximum distance for the search.
**  @param player      Only search fields explored by player
**  @param x           Map X start position for the search.
**  @param y           Map Y start position for the search.
**
**  @param px          OUT: Map X position of tile.
**  @param py          OUT: Map Y position of tile.
**
**  @note Movement mask can be 0xFFFFFFFF to have no effect
**  Range is not circular, but square.
**  Player is ignored if nil(search the entire map)
**  Use rvresult if you search for a tile that doesn't
**  match resmask. Like for a tile where an unit can go
**  with it's movement mask.
**
**  @return            True if wood was found.
*/
int FindTerrainType(int movemask, int resmask, int rvresult, int range,
	const CPlayer *player, int x, int y, int *px, int *py)
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
	int wp;
	int rp;
	int ep;
	int i;
	int w;
	unsigned char *m;
	unsigned char *matrix;
	int destx;
	int desty;
	int cdist;

	destx = x;
	desty = y;
	size = std::min(Map.Info.MapWidth * Map.Info.MapHeight / 4, range * range * 5);
	points = new p[size];

	// Make movement matrix. FIXME: can create smaller matrix.
	matrix = CreateMatrix();
	w = Map.Info.MapWidth + 2;
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
				if (!Map.Info.IsPointOnMap(x, y)) {
					continue;
				}
				m = matrix + x + y * w;
				/*
				 *  Check if visited or unexplored for non
				 *	AI players (our exploration code is too week for real
				 *	competition with human players)
				 */
				if (*m || (player && !player->AiEnabled &&!Map.IsFieldExplored(player, x, y))) {
					continue;
				}
				// Look if found what was required.
				bool can_move_to = CanMoveToMask(x, y, resmask);
				if ((rvresult ? can_move_to : !can_move_to)) {
					*px = x;
					*py = y;
					delete[] points;
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
	delete[] points;
	return 0;
}

template <const bool NEARLOCATION>
class BestDepotFinder {
	union {
		const CUnit *worker;
		struct {
			int x;
			int y;
		} loc;
	} u_near;
	const int resource;
	const int range;
	int best_dist;

	inline void operator() (CUnit *const dest) {
		/* Only resource depots */
		if (dest->Type->CanStore[resource] &&
			 dest->IsAliveOnMap() &&
			 dest->CurrentAction() != UnitActionBuilt) {
			// Unit in range?

			if (NEARLOCATION) {
				int d = dest->MapDistanceTo(u_near.loc.x, u_near.loc.y);

				//
				// Take this depot?
				//
				if (d <= range && d < best_dist) {
					best_depot = dest;
					best_dist = d;
				}
			} else {
				int d;
				const CUnit *worker = u_near.worker;
				if (!worker->Container) {
					d = worker->MapDistanceTo(dest);
				} else {
					d = worker->Container->MapDistanceTo(dest);
				}

				// Use Circle, not square :)
				if (d > range) {
					return;
				}

				// calck real travel distance
				if (!worker->Container) {
					d = UnitReachable(worker, dest, 1);
				}
				//
				// Take this depot?
				//
				if (d && d < best_dist) {
					best_depot = dest;
					best_dist = d;
				}

			}
		}
	}

public:
	CUnit *best_depot;

	BestDepotFinder(const CUnit *w, int x, int y, int res, int ran):
		resource(res), range(ran),
		best_dist(INT_MAX), best_depot(0) {
			if (NEARLOCATION) {
				u_near.loc.x = x;
				u_near.loc.y = y;
			} else {
				u_near.worker = w;
			}
		};


	CUnit *Find(CUnit **table, const int table_size) {
#ifdef _MSC_VER
		for (int i = 0; i < table_size; ++i) {
			this->operator() (table[i]);
		}
#else
		if(table_size) {
			int i = 0, n = (table_size+3)/4;
			switch (table_size & 3) {
				case 0: do {
								this->operator() (table[i++]);
				case 3:			this->operator() (table[i++]);
				case 2:			this->operator() (table[i++]);
				case 1:			this->operator() (table[i++]);
					} while ( --n > 0 );
			}
		}
#endif
		return best_depot;
	}


	CUnit *Find(CUnitCache &cache) {
		cache.for_each(*this);
		return best_depot;
	}

};

static CUnit *FindDepositNearLoc(CPlayer *p,
				int x, int y, int range, int resource)
{
	BestDepotFinder<true> finder(NULL,x,y,resource,range);
	CUnit *depot = finder.Find(p->Units, p->TotalNumUnits);
	if (!depot) {
		for (int i = 0; i < PlayerMax; ++i) {
			if (i != p->Index &&
				Players[i].IsAllied(p) &&
				p->IsAllied(&Players[i])) {
				finder.Find(Players[i].Units, Players[i].TotalNumUnits);
			}
		}
		depot = finder.best_depot;
	}
	return depot;
}

/**
**  Find Resource.
**
**  @param unit        The unit that wants to find a resource.
**  @param x           Closest to x
**  @param y           Closest to y
**  @param range       Maximum distance to the resource.
**  @param resource    The resource id.
**
**  @note This will return an usable resource building that doesn't
**  belong to the player or one of his allies.
**
**  @return            NoUnitP or resource unit
*/
CUnit *UnitFindResource(const CUnit *unit, int x, int y, int range, int resource
		,bool check_usage,const CUnit *destu)
{
	static const int xoffset[] = { 0, 0,-1,+1, 0, -1,+1,-1,+1 };
	static const int yoffset[] = { 0,-1, 0, 0,+1, -1,-1,+1,+1 };
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
	int n = 0;
	unsigned char *m;
	unsigned char *matrix;
	CUnit *mine;
	CUnit *bestmine;
	int destx;
	int desty;
	int bestd = 99999, bestw = 99999, besta = 99999;
	int cdist;
	const ResourceInfo *resinfo = unit->Type->ResInfo[resource];

	destx = x;
	desty = y;
	size = std::min(Map.Info.MapWidth * Map.Info.MapHeight / 4, range * range * 5);
	points = new p[size];

	// Find the nearest gold depot
	if (!destu) destu = FindDepositNearLoc(unit->Player,x,y,range,resource);
	if (destu) {
		NearestOfUnit(destu, x, y, &destx, &desty);
	}

	// Make movement matrix. FIXME: can create smaller matrix.
	matrix = CreateMatrix();
	w = Map.Info.MapWidth + 2;
	matrix += w + w + 2;
	//  Unit movement mask
	mask = unit->Type->MovementMask;
	//  Ignore all units along the way. Might seem wierd, but otherwise
	//  peasants would lock at a mine with a lot of workers.
	mask &= ~(MapFieldLandUnit | MapFieldSeaUnit | MapFieldAirUnit);
	points[0].X = x;
	points[0].Y = y;
	rp = 0;
	if(unit->X == x && unit->Y == y)
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
			for (i = 0; i < 9; ++i) { // mark all neighbors
				x = rx + xoffset[i];
				y = ry + yoffset[i];
				m = matrix + x + y * w;
				if (*m) { // already checked
					continue;
				}

				/*
				 *  Check if unexplored for non AI players only.
				 *  Our exploration code is too week for real
				 *	competition with human players.
				 */
				if (!unit->Player->AiEnabled &&
					 !Map.IsFieldExplored(unit->Player, x, y)) { // Unknown.
					continue;
				}

				//
				// Look if there is a mine
				//
				if ((mine = ResourceOnMap(x, y, resource)) &&
						mine->Type->CanHarvest &&
						(resinfo->HarvestFromOutside ||
							mine->Player->Index == PlayerMax - 1 ||
							mine->Player == unit->Player ||
							(unit->IsAllied(mine) && mine->IsAllied(unit)))
							 ) {
					if (destu) {
						bool better = (mine != bestmine);

						if(better) {
							n = std::max(MyAbs(destx - x), MyAbs(desty - y));
							if(check_usage && mine->Type->MaxOnBoard) {
								int assign = mine->Data.Resource.Assigned -
														mine->Type->MaxOnBoard;
								int waiting =
										(assign > 0 ?
											 GetNumWaitingWorkers(mine) : 0);
								if(bestmine != NoUnitP) {
									if(besta >= assign)
									{
										if(assign > 0) {
											waiting = GetNumWaitingWorkers(mine);
											if(besta == assign) {
												if(bestw < waiting) {
													better = false;
												} else {
												  	if(bestw == waiting && bestd < n)
													{
														better = false;
													}
												}
											}
										} else {
											if(besta == assign && bestd < n)
											{
												better = false;
											}
										}
									} else {
										if(assign > 0 || bestd < n) {
											better = false;
										}
									}
								}
								if(better) {
									besta = assign;
									bestw = waiting;
								}
							} else {
								if(bestmine != NoUnitP && bestd < n)
								{
									better = false;
								}
							}
						}
						if(better) {
							bestd = n;
							bestmine = mine;
						}
						*m = 99;
					} else {
						if (check_usage && mine->Type->MaxOnBoard) {
							bool better = (mine != bestmine &&
							//Durring construction Data.Resource is corrupted
								mine->CurrentAction() != UnitActionBuilt);
							if (better) {
								int assign = mine->Data.Resource.Assigned -
														mine->Type->MaxOnBoard;
								int waiting = (assign > 0 ?
									GetNumWaitingWorkers(mine) : 0);
								if (assign < besta ||
									(assign == besta && waiting < bestw)) {
									bestd = n;
									besta = assign;
									bestw = waiting;
									bestmine = mine;
								}
							}
							*m = 99;
						} else { // no goal take the first
							delete[] points;
							return mine;
						}
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
			delete[] points;
			return bestmine;
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
**  Find Mining Area for Resource.
**
**  @param unit        The unit that wants to find a resource.
**  @param x           Closest to x
**  @param y           Closest to y
**  @param range       Maximum distance to the resource.
**  @param resource    The resource id.
**
**  @note This will return an usable resource building that doesn't
**  belong to the player or one of his allies.
**
**  @return            NoUnitP or resource unit
*/
CUnit *UnitFindMiningArea(const CUnit *unit, int x, int y,
		 int range, int resource)
{
	static const int xoffset[] = {0,  0,-1,+1, 0, -1,+1,-1,+1 };
	static const int yoffset[] = {0, -1, 0, 0,+1, -1,-1,+1,+1 };
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
	int n;
	unsigned char *m;
	unsigned char *matrix;
	const CUnit *destu;
	CUnit *mine;
	CUnit *bestmine;
	int destx;
	int desty;
	int bestd;
	int cdist;
	//const ResourceInfo *resinfo = unit->Type->ResInfo[resource];

	destx = x;
	desty = y;
	size = std::min(Map.Info.MapWidth * Map.Info.MapHeight / 4, range * range * 5);
	points = new p[size];

	// Find the nearest resource depot
	if ((destu = FindDepositNearLoc(unit->Player,x,y,range,resource)))
	{
		NearestOfUnit(destu, x, y, &destx, &desty);
	}
	bestd = 99999;
	// Make movement matrix. FIXME: can create smaller matrix.
	matrix = CreateMatrix();
	w = Map.Info.MapWidth + 2;
	matrix += w + w + 2;
	//  Unit movement mask
	mask = unit->Type->MovementMask;
	//  Ignore all units along the way. Might seem wierd, but otherwise
	//  peasants would lock at a mine with a lot of workers.
	mask &= ~(MapFieldLandUnit | MapFieldSeaUnit | MapFieldAirUnit);

	points[0].X = x;
	points[0].Y = y;
	rp = 0;
	if(unit->X == x && unit->Y == y)
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

				/*
				 *  Check if unexplored for non AI players only.
				 *  Our exploration code is too week for real
				 *	competition with human players.
				 */
				if (!unit->Player->AiEnabled &&
					 !Map.IsFieldExplored(unit->Player, x, y)) { // Unknown.
					continue;
				}

				//
				// Look if there is a mine area
				//
				if ((mine = ResourceOnMap(x, y, resource, false))) {
					if (destu) {
						n = std::max(MyAbs(destx - x), MyAbs(desty - y));
						if (n < bestd) {
							bestd = n;
							bestmine = mine;
						}
						*m = 99;
					} else { // no goal take the first
						delete[] points;
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
			delete[] points;
			return bestmine;
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
**  Find deposit. This will find a deposit for a resource
**
**  @param unit        The unit that wants to find a resource.
**  @param x           Closest to x
**  @param y           Closest to y
**  @param range       Maximum distance to the deposit.
**  @param resource    Resource to find deposit from.
**
**  @note This will return a reachable allied depot.
**
**  @return            NoUnitP or deposit unit
*/
CUnit *FindDeposit(const CUnit *unit, int range, int resource)
{
	BestDepotFinder<false> finder(unit,0,0,resource, range);
	CUnit *depot = finder.Find(unit->Player->Units, unit->Player->TotalNumUnits);
	if (!depot) {
		for (int i = 0; i < PlayerMax; ++i) {
			if (i != unit->Player->Index &&
				Players[i].IsAllied(unit->Player) &&
				unit->Player->IsAllied(&Players[i])) {
				finder.Find(Players[i].Units, Players[i].TotalNumUnits);
			}
		}
		depot = finder.best_depot;
	}
	return depot;
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
		if (unit->Type->Harvester && unit->Type->ResInfo && !unit->Removed) {
			if (unit->CurrentAction() == UnitActionStill) {
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
**  @param ounit  Old selected unit.
**  @param x      X pixel position.
**  @param y      Y pixel position.
**
**  @return       An unit on x, y position.
*/
CUnit *UnitOnScreen(CUnit *ounit, int x, int y)
{
	CUnit **table;
	CUnit *unit;
	CUnit *nunit;
	CUnit *funit; // first possible unit
	CUnitType *type;
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
		if (!ReplayRevealMap && !unit->IsVisibleAsGoal(ThisPlayer)) {
			continue;
		}
		type = unit->Type;

		//
		// Check if mouse is over the unit.
		//
		gx = unit->X * TileSizeX + unit->IX;

		{
			const int local_width = type->TileWidth * TileSizeX;
			if (x + (type->BoxWidth - local_width) / 2 < gx) {
				continue;
			}
			if (x > gx + (local_width + type->BoxWidth) / 2) {
				continue;
			}
		}

		gy = unit->Y * TileSizeY + unit->IY;
		{
			const int local_height = type->TileHeight * TileSizeY;
			if (y + (type->BoxHeight - local_height) / 2 < gy) {
				continue;
			}
			if (y > gy + (local_height + type->BoxHeight) / 2) {
				continue;
			}
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
**  Let an unit die.
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
	if (type->DeathExplosion) {
		type->DeathExplosion->pushPreamble();
		type->DeathExplosion->pushInteger(unit->X * TileSizeX +
				type->TileWidth * TileSizeX / 2);
		type->DeathExplosion->pushInteger(unit->Y * TileSizeY +
				type->TileHeight * TileSizeY / 2);
		type->DeathExplosion->run();
	}
	// Handle Teleporter Destination Removal
	if (type->Teleporter && unit->Goal) {
		unit->Goal->Remove(NULL);
		UnitLost(unit->Goal);
		UnitClearOrders(unit->Goal);
		unit->Goal->Release();
		unit->Goal = NULL;
	}

	// During resource build, the worker holds the resource amount,
	// but if canceling building the platform, the worker is already
	// outside.
	if (type->GivesResource &&
			unit->CurrentAction() == UnitActionBuilt &&
			unit->Data.Built.Worker) {
		// Restore value for oil-patch
		unit->ResourcesHeld = unit->Data.Built.Worker->ResourcesHeld;
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
	unit->CurrentOrder()->Action = UnitActionDie;
	if (type->CorpseType) {
#ifdef DYNAMIC_LOAD
		if (!type->Sprite) {
			LoadUnitTypeSprite(type);
		}
#endif
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
		Map.Insert(unit);

		// FIXME: rb: Maybe we need this here because corpse of cloaked units
		//	may crash Sign code

		// Recalculate the seen count.
		//UnitCountSeen(unit);
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
	CUnit *unit = source->UnitInside;

	// No Corpses, we are inside something, and we can't be seen
	for (int i = source->InsideCount; i; --i, unit = unit->NextContained) {
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

	// Units with 0 hp can't be hit
	if (target->Variable[HP_INDEX].Value == 0) {
		return;
	}

	Assert(damage != 0 && target->CurrentAction() != UnitActionDie && !target->Type->Vanishes);

	if (target->Variable[UNHOLYARMOR_INDEX].Value > 0 || target->Type->Indestructible) {
		// vladi: units with active UnholyArmour are invulnerable
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

		if (!target->Type->Building) {
			if (target->Player->AiEnabled) {
				AiHelpMe(attacker, target);
			} else {
				if (target->GroupId) {
					GroupHelpMe(attacker, target);
				}
			}
		}
	}

	if (target->Type->Building && target->Player->AiEnabled) {
		AiHelpMe(attacker, target);
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
			if (UseHPForXp) {
				attacker->Variable[XP_INDEX].Max += target->Variable[HP_INDEX].Value;
			} else {
				attacker->Variable[XP_INDEX].Max += target->Type->Points;
			}
			attacker->Variable[XP_INDEX].Value = attacker->Variable[XP_INDEX].Max;
			attacker->Variable[KILL_INDEX].Value++;
			attacker->Variable[KILL_INDEX].Max++;
			attacker->Variable[KILL_INDEX].Enable = 1;
		}
		LetUnitDie(target);
		return;
	}
	target->Variable[HP_INDEX].Value -= damage;
	if (UseHPForXp && attacker && target->IsEnemy(attacker)) {
		attacker->Variable[XP_INDEX].Value += damage;
		attacker->Variable[XP_INDEX].Max += damage;
	}

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

	/* Target Reaction on Hit */
	switch(target->CurrentAction()) {
		case UnitActionTrain:
		case UnitActionUpgradeTo:
		case UnitActionResearch:
		case UnitActionBuilt:
		case UnitActionBuild:
		case UnitActionTransformInto:
		case UnitActionBoard:
		case UnitActionUnload:
		case UnitActionReturnGoods:
			//
			// Unit is working?
			// Maybe AI should cance action and save resources???
			//
			return;
		case UnitActionResource:
			if (target->SubAction >= 65/* SUB_STOP_GATHERING */)
			{
				//Normal return to depot
				return;
			}
			if (target->SubAction > 55 /* SUB_START_GATHERING */ &&
				target->ResourcesHeld > 0) {
				//escape to Depot with this what you have;
				target->Data.ResWorker.DoneHarvesting = 1;
				return;
			}
		break;
		case UnitActionAttack:
			goal = target->CurrentOrder()->GetGoal();
			if (goal) {
				if (goal == attacker ||
					(goal->CurrentAction() == UnitActionAttack &&
					goal->CurrentOrder()->GetGoal() == target))
				{
					//we already fight with one of attackers;
					return;
				}
			}
		default:
		break;
	}

	//
	// Attack units in range (which or the attacker?)
	//
	if (attacker && target->IsAgressive()) {
			if (RevealAttacker && CanTarget(target->Type, attacker->Type)) {
				// Reveal Unit that is attacking
				goal = attacker;
			} else {
				if (target->CurrentAction() == UnitActionStandGround) {
					goal = AttackUnitsInRange(target);
				} else {
					// Check for any other units in range
					goal = AttackUnitsInReactRange(target);
				}
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

	/*
		What should we do with workers on :
		case UnitActionRepair:

		Drop orders and run away or return after escape?
	*/

	//
	// Can't attack run away.
	//
	if (target->CanMove()) {
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
**  Returns the map distance between two points with unit type.
**
**  @param x1    X map tile position.
**  @param y1    Y map tile position.
**  @param type  Unit type to take into account.
**  @param x2    X map tile position.
**  @param y2    Y map tile position.
**
**  @return      The distance between in tiles.
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
**  Returns the map distance between two points with unit type.
**
**  @param src  src unittype
**  @param x1   X map tile position of src (upperleft).
**  @param y1   Y map tile position of src.
**  @param dst  Unit type to take into account.
**  @param x2   X map tile position of dst.
**  @param y2   Y map tile position of dst.
**
**  @return     The distance between the types.
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
**  @param x  X map tile position.
**  @param y  Y map tile position.
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
**  @param dest  Distance to this unit.
**
**  @todo FIXME: is it the correct place to put this function in?
*/
int ViewPointDistanceToUnit(const CUnit *dest)
{
	const CViewport *vp = UI.SelectedViewport;;
	return dest->MapDistanceTo(vp->MapX + vp->MapWidth / 2,
		vp->MapY + vp->MapHeight / 2);
}

/**
**  Can the source unit attack the destination unit.
**
**  @param source  Unit type pointer of the attacker.
**  @param dest    Unit type pointer of the target.
**
**  @return        0 if attacker can't target the unit, else a positive number.
*/
int CanTarget(const CUnitType *source, const CUnitType *dest)
{
	for (unsigned int i = 0; i < UnitTypeVar.GetNumberBoolFlag(); i++) {
		if (source->BoolFlag[i].CanTargetFlag != CONDITION_TRUE) {
			if ((source->BoolFlag[i].CanTargetFlag == CONDITION_ONLY) ^
				(dest->BoolFlag[i].value)) {
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
**  @return             1 if transporter can transport unit, 0 else.
*/
int CanTransport(const CUnit *transporter, const CUnit *unit)
{
	if (!transporter->Type->CanTransport()) {
		return 0;
	}
	if (transporter->CurrentAction() == UnitActionBuilt) { // Under construction
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
	for (unsigned int i = 0; i < UnitTypeVar.GetNumberBoolFlag(); i++) {
		if (transporter->Type->BoolFlag[i].CanTransport != CONDITION_TRUE) {
			if ((transporter->Type->BoolFlag[i].CanTransport == CONDITION_ONLY) ^
					unit->Type->BoolFlag[i].value) {
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
bool CUnit::IsUnusable(bool ignore_built_state) const
{
	return !IsAliveOnMap() || !ignore_built_state && CurrentAction() == UnitActionBuilt;
}


/*----------------------------------------------------------------------------
--  Initialize/Cleanup
----------------------------------------------------------------------------*/

/**
**  Initialize unit module.
*/
void InitUnits(void)
{
	if (!SaveGameLoading) {
		NumUnits = 0;
		UnitManager.Init();
	}
}

/**
**  Clean up unit module.
*/
void CleanUnits(void)
{
	//
	//  Free memory for all units in unit table.
	//
	while(NumUnits) {
		int count = NumUnits;
		do {
			CUnit *unit = Units[count - 1];
			if (!unit->Destroyed) {
				if (//unit->Type->Harvester && 
					unit->CurrentAction() == UnitActionResource) {
					ResourceInfo *resinfo = unit->Type->ResInfo[unit->CurrentResource];
					if (resinfo && !resinfo->TerrainHarvester) {
						CUnit *mine = unit->CurrentOrder()->Arg1.Resource.Mine;
						if (mine) {
							unit->DeAssignWorkerFromMine(mine);
							mine->RefsDecrease();
							unit->CurrentOrder()->Arg1.Resource.Mine = NULL;
						}
					}
				}
				unit->CurrentOrder()->ClearGoal();
				if(!unit->Removed) {
					unit->Remove(NULL);
				}
				UnitClearOrders(unit);
			}
			unit->Release(true);
		} while(--count);
	}
	NumUnits = 0;

	UnitManager.Init();

	HelpMeLastCycle = 0;
}

//@}
