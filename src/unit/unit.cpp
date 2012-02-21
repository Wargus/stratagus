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

static void RemoveUnitFromContainer(CUnit &unit);

extern int ExtraDeathIndex(const char *death);

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

void CUnit::Init()
{
	Refs = 0;
	Slot = 0;
	UnitSlot = NULL;
	PlayerSlot = NULL;
	Next = NULL;
	InsideCount = 0;
	BoardCount = 0;
	UnitInside = NULL;
	Container = NULL;
	NextContained = NULL;
	PrevContained = NULL;
	NextWorker = NULL;

	Resource.Workers = NULL;
	Resource.Assigned = 0;
	Resource.Active = 0;

	tilePos.x = 0;
	tilePos.y = 0;
	Offset = 0;
	Type = NULL;
	Player = NULL;
	Stats = NULL;
	CurrentSightRange = 0;
	Colors = NULL;
	IX = 0;
	IY = 0;
	Frame = 0;
	Direction = 0;
	DamagedType = ANIMATIONS_DEATHTYPES;
	Attacked = 0;
	Burning = 0;
	Destroyed = 0;
	Removed = 0;
	Selected = 0;
	TeamSelected = 0;
	Constructed = 0;
	Active = 0;
	Boarded = 0;
	RescuedFrom = NULL;
	memset(VisCount, 0, sizeof(VisCount));
	memset(&Seen, 0, sizeof(Seen));
	Variable = NULL;
	TTL = 0;
	GroupId = 0;
	LastGroup = 0;
	ResourcesHeld = 0;
	SubAction = 0;
	Wait = 0;
	State = 0;
	Blink = 0;
	Moving = 0;
	ReCast = 0;
	CacheLock = 0;
	GuardLock = 0;
	memset(&Anim, 0, sizeof(Anim));
	CurrentResource = 0;
	OrderFlush = 0;
	Orders.clear();
	delete SavedOrder;
	SavedOrder = NULL;
	delete NewOrder;
	NewOrder = NULL;
	delete CriticalOrder;
	CriticalOrder = NULL;
	AutoCastSpell = NULL;
	AutoRepair = 0;
	Goal = NULL;
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
	Assert(Orders.size() == 1);
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
			MapUnmarkUnitSight(*this);
			RemoveUnitFromContainer(*this);
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

unsigned int CUnit::CurrentAction() const
{
	return (CurrentOrder()->Action);
}

void CUnit::ClearAction()
{
	delete CurrentOrder();
	Orders[0] = COrder::NewActionStill();

	SubAction = 0;
	if (Selected) {
		SelectedUnitChanged();
	}
}


bool CUnit::IsIdle() const
{
	return Orders.size() == 1 && CurrentAction() == UnitActionStill;
}

bool CUnit::IsAlive() const
{
	return !Destroyed && CurrentAction() != UnitActionDie;
}

int CUnit::GetDrawLevel() const
{
	return ((Type->CorpseType && CurrentAction() == UnitActionDie) ?
				Type->CorpseType->DrawLevel : Type->DrawLevel);
}



/**
**  Initialize the unit slot with default values.
**
**  @param type    Unit-type
*/
void CUnit::Init(CUnitType &type)
{
	//  Set refs to 1. This is the "I am alive ref", lost in ReleaseUnit.
	Refs = 1;

	//  Build all unit table
	UnitSlot = &Units[NumUnits]; // back pointer
	Units[NumUnits++] = this;

	//  Initialise unit structure (must be zero filled!)
	Type = &type;

	Seen.Frame = UnitNotSeen; // Unit isn't yet seen

	Frame = type.StillFrame;

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
	if (type.NumDirections > 1 && type.Sprite && !type.Building) {
		Direction = (MyRand() >> 8) & 0xFF; // random heading
		UnitUpdateHeading(*this);
	}

	if (type.CanCastSpell) {
		AutoCastSpell = new char[SpellTypeTable.size()];
		if (Type->AutoCastActive) {
			memcpy(AutoCastSpell, Type->AutoCastActive, SpellTypeTable.size());
		} else {
			memset(AutoCastSpell, 0, SpellTypeTable.size());
		}
	}
	Active = 1;
	Removed = 1;

	// Has StartingResources, Use those
	this->ResourcesHeld = type.StartingResources;

	Rs = MyRand() % 100; // used for fancy buildings and others

	Assert(Orders.empty());

	Orders.push_back(COrder::NewActionStill());

	Assert(NewOrder == NULL);
	NewOrder = NULL;
	Assert(SavedOrder == NULL);
	SavedOrder = NULL;
	Assert(CriticalOrder == NULL);
	CriticalOrder = NULL;
}

/**
**  Restore the saved order
**
**  @return      True if the saved order was restored
*/
bool CUnit::RestoreOrder()
{
	if (this->SavedOrder == NULL) {
		return false;
	}

	// Restart order state.
	this->State = 0;
	this->SubAction = 0;

	// Cannot delete this->Orders[0] since it is generally that order
	// which call this method.

	//copy
	this->Orders[0] = this->SavedOrder;
	this->CurrentResource = this->SavedOrder->CurrentResource;

	this->CurrentOrder()->NewResetPath();

	this->SavedOrder = NULL;
	return true;
}

/**
**  Store the Current order
**
**  @return      True if the current order was saved
*/
bool CUnit::StoreOrder(COrder* order)
{
	Assert(order);
	Assert(order->HasGoal() || Map.Info.IsPointOnMap(order->goalPos));

	if (this->SavedOrder != NULL) {
		return false;
	}
	// Save current order to come back or to continue it.
	this->SavedOrder = order;
	return true;
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
				if (!type->Wall && type != UnitTypeOrcWall && type != UnitTypeHumanWall) {
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
		if (!type->Wall && type != UnitTypeOrcWall && type != UnitTypeHumanWall) {
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
CUnit *MakeUnit(CUnitType &type, CPlayer *player)
{
	// Game unit limit reached.
	if (NumUnits >= UnitMax) {
		DebugPrint("Over all unit limit (%d) reached.\n" _C_ UnitMax);
		return NoUnitP;
	}

	CUnit *unit = UnitManager.AllocUnit();
	if (unit == NoUnitP) {
		return NoUnitP;
	}
	unit->Init(type);
	// Only Assign if a Player was specified
	if (player) {
		unit->AssignToPlayer(player);
	}

	// Increase the max resources limit
	for (int i = 0; i < MaxCosts; ++i) {
		if (type._Storing[i] && unit->Player->MaxResources[i] != -1) {
			unit->Player->MaxResources[i] += type._Storing[i];
		}
	}

	if (type.Building) {
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
**  @param unit    Unit to (un)mark.
**  @param pos     coord of first container of unit.
**  @param width   Width of the first container of unit.
**  @param height  Height of the first container of unit.
**  @param f       Function to (un)mark for normal vision.
**  @param f2        Function to (un)mark for cloaking vision.
*/
static void MapMarkUnitSightRec(const CUnit &unit, const Vec2i &pos, int width, int height,
	MapMarkerFunc *f, MapMarkerFunc *f2)
{
	Assert(f);
	MapSight(*unit.Player, pos, width, height,
		unit.Container ? unit.Container->CurrentSightRange : unit.CurrentSightRange, f);

	if (unit.Type && unit.Type->DetectCloak && f2) {
		MapSight(*unit.Player, pos, width, height,
			unit.Container ? unit.Container->CurrentSightRange : unit.CurrentSightRange, f2);
	}

	CUnit *unit_inside = unit.UnitInside;
	for (int i = unit.InsideCount; i--; unit_inside = unit_inside->NextContained) {
		MapMarkUnitSightRec(*unit_inside, pos, width, height, f, f2);
	}
}

/**
**  Return the unit not transported, by viewing the container recursively.
**
**  @param unit  unit from where look the first conatiner.
**
**  @return      Container of container of ... of unit. It is not null.
*/
CUnit *GetFirstContainer(const CUnit &unit)
{
	const CUnit *container = &unit;

	while (container->Container) {
		container = container->Container;
	}
	return const_cast<CUnit *>(container);
}

/**
**  Mark on vision table the Sight of the unit
**  (and units inside for transporter)
**
**  @param unit  unit to unmark its vision.
**  @see MapUnmarkUnitSight.
*/
void MapMarkUnitSight(CUnit &unit)
{
	CUnit *container;  // First container of the unit.

	container = GetFirstContainer(unit);
	Assert(container->Type);

	MapMarkUnitSightRec(unit, container->tilePos, container->Type->TileWidth, container->Type->TileHeight,
		MapMarkTileSight, MapMarkTileDetectCloak);

	// Never mark radar, except if the top unit, and unit is usable
	if (&unit == container && !unit.IsUnusable()) {
		if (unit.Stats->Variables[RADAR_INDEX].Value) {
			MapMarkRadar(*unit.Player, unit.tilePos, unit.Type->TileWidth,
				unit.Type->TileHeight, unit.Stats->Variables[RADAR_INDEX].Value);
		}
		if (unit.Stats->Variables[RADARJAMMER_INDEX].Value) {
			MapMarkRadarJammer(*unit.Player, unit.tilePos, unit.Type->TileWidth,
				unit.Type->TileHeight, unit.Stats->Variables[RADARJAMMER_INDEX].Value);
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
void MapUnmarkUnitSight(CUnit &unit)
{
	CUnit *container;  // First container of the unit.

	Assert(unit.Type);

	container = GetFirstContainer(unit);
	Assert(container->Type);
	MapMarkUnitSightRec(unit,
		container->tilePos, container->Type->TileWidth, container->Type->TileHeight,
		MapUnmarkTileSight, MapUnmarkTileDetectCloak);

	// Never mark radar, except if the top unit?
	if (&unit == container && !unit.IsUnusable()) {
		if (unit.Stats->Variables[RADAR_INDEX].Value) {
			MapUnmarkRadar(*unit.Player, unit.tilePos, unit.Type->TileWidth,
				unit.Type->TileHeight, unit.Stats->Variables[RADAR_INDEX].Value);
		}
		if (unit.Stats->Variables[RADARJAMMER_INDEX].Value) {
			MapUnmarkRadarJammer(*unit.Player, unit.tilePos, unit.Type->TileWidth,
				unit.Type->TileHeight, unit.Stats->Variables[RADARJAMMER_INDEX].Value);
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
void UpdateUnitSightRange(CUnit &unit)
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
	if (unit.Constructed) { // Units under construction have no sight range.
		unit.CurrentSightRange = 0;
	} else if (!unit.Container) { // proper value.
		unit.CurrentSightRange = unit.Stats->Variables[SIGHTRANGE_INDEX].Max;
	} else { // value of it container.
		unit.CurrentSightRange = unit.Container->CurrentSightRange;
	}

	unit_inside = unit.UnitInside;
	for (i = unit.InsideCount; i--; unit_inside = unit_inside->NextContained) {
		UpdateUnitSightRange(*unit_inside);
	}
}

/**
**  Mark the field with the FieldFlags.
**
**  @param unit  unit to mark.
*/
void MarkUnitFieldFlags(const CUnit &unit)
{
	CMapField *mf;
	const unsigned int flags = unit.Type->FieldFlags; //
	int w, h = unit.Type->TileHeight;          // Tile height of the unit.
	const int width = unit.Type->TileWidth;          // Tile width of the unit.
	unsigned int index = unit.Offset;

	if (unit.Type->Vanishes) {
		return ;
	}
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

class _UnmarkUnitFieldFlags
{
public:
	_UnmarkUnitFieldFlags(const CUnit &unit, CMapField *mf) : main(&unit), mf(mf)
	{}

	void operator () (CUnit *const unit) const
	{
		if (main != unit && unit->CurrentAction() != UnitActionDie) {
			mf->Flags |= unit->Type->FieldFlags;
		}
	}
private:
	const CUnit *const main;
	CMapField *mf;
};


/**
**  Mark the field with the FieldFlags.
**
**  @param unit  unit to mark.
*/
void UnmarkUnitFieldFlags(const CUnit &unit)
{
	const unsigned int flags = ~unit.Type->FieldFlags;
	const int width = unit.Type->TileWidth;
	int h = unit.Type->TileHeight;
	unsigned int index = unit.Offset;

	if (unit.Type->Vanishes) {
		return ;
	}
	do {
		CMapField *mf = Map.Field(index);

		int w = width;
		do {
			mf->Flags &= flags;//clean flags
			_UnmarkUnitFieldFlags funct(unit, mf);

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
void CUnit::AddInContainer(CUnit &host)
{
	Assert(Container == NoUnitP);
	Container = &host;
	if (host.InsideCount == 0) {
		NextContained = PrevContained = this;
	} else {
		NextContained = host.UnitInside;
		PrevContained = host.UnitInside->PrevContained;
		host.UnitInside->PrevContained->NextContained = this;
		host.UnitInside->PrevContained = this;
	}
	host.UnitInside = this;
	host.InsideCount++;
}

/**
**  Remove unit from a container. It only updates linked list stuff.
**
**  @param unit  Pointer to unit.
*/
static void RemoveUnitFromContainer(CUnit &unit)
{
	CUnit *host;  // transporter which contain unit.

	host = unit.Container;
	Assert(unit.Container);
	Assert(unit.Container->InsideCount > 0);
	host->InsideCount--;
	unit.NextContained->PrevContained = unit.PrevContained;
	unit.PrevContained->NextContained = unit.NextContained;
	if (host->InsideCount == 0) {
		host->UnitInside = NoUnitP;
	} else {
		if (host->UnitInside == &unit) {
			host->UnitInside = unit.NextContained;
		}
	}
	unit.Container = NoUnitP;
}


/**
**  Affect Tile coord of a unit (with units inside) to tile (x, y).
**
**  @param unit  unit to move.
**  @param pos   map tile position.
**
**  @internal before use it, Map.Remove(unit), MapUnmarkUnitSight(unit)
**  and after Map.Insert(unit), MapMarkUnitSight(unit)
**  are often necessary. Check Flag also for Pathfinder.
*/
static void UnitInXY(CUnit &unit, const Vec2i &pos)
{
	CUnit *unit_inside = unit.UnitInside;

	unit.tilePos = pos;
	unit.Offset = Map.getIndex(pos);

	if (!unit.Container) {
		//Only Top Units
		const CMapField *const mf = Map.Field(unit.Offset);
		const CPlayer *const p = unit.Player;
		for (int player = 0; player < NumPlayers; ++player) {
			if(player != p->Index && mf->Guard[player] && p->IsEnemy(player)) {
				Players[player].AutoAttackTargets.InsertS(&unit);
				unit.RefsIncrease();
			}
		}
	}
	for (int i = unit.InsideCount; i--; unit_inside = unit_inside->NextContained) {
		UnitInXY(*unit_inside, pos);
	}
}

/**
**  Move a unit (with units inside) to tile (pos).
**  (Do stuff with vision, cachelist and pathfinding).
**
**  @param pos  map tile position.
**
*/
void CUnit::MoveToXY(const Vec2i &pos)
{
	MapUnmarkUnitSight(*this);
	Map.Remove(*this);
	UnmarkUnitFieldFlags(*this);

	Assert(UnitCanBeAt(*this, pos));
	// Move the unit.
	UnitInXY(*this, pos);

	Map.Insert(*this);
	MarkUnitFieldFlags(*this);
	//  Recalculate the seen count.
	UnitCountSeen(*this);
	MapMarkUnitSight(*this);
}

/**
**  Place unit on map.
**
**  @param pos  map tile position.
*/
void CUnit::Place(const Vec2i &pos)
{
	Assert(Removed);

	if (Container) {
		MapUnmarkUnitGuard(*this);
		MapUnmarkUnitSight(*this);
		RemoveUnitFromContainer(*this);
	}
	if (!SaveGameLoading) {
		UpdateUnitSightRange(*this);
	}
	Removed = 0;
	UnitInXY(*this, pos);
	// Pathfinding info.
	MarkUnitFieldFlags(*this);
	// Tha cache list.
	Map.Insert(*this);
	//  Calculate the seen count.
	UnitCountSeen(*this);
	// Vision
	MapMarkUnitSight(*this);

	// Correct directions for wall units
	if (this->Type->Wall && this->CurrentAction() != UnitActionBuilt) {
		CorrectWallDirections(*this);
		UnitUpdateHeading(*this);
		CorrectWallNeighBours(*this);
	}
}

/**
**  Create new unit and place on map.
**
**  @param pos     map tile position.
**  @param type    Pointer to unit-type.
**  @param player  Pointer to owning player.
**
**  @return        Pointer to created unit.
*/
CUnit *MakeUnitAndPlace(const Vec2i &pos, CUnitType &type, CPlayer *player)
{
	CUnit *unit = MakeUnit(type, player);

	if (unit != NoUnitP) {
		unit->Place(pos);
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

	Map.Remove(*this);
	MapUnmarkUnitSight(*this);
	UnmarkUnitFieldFlags(*this);
	MapUnmarkUnitGuard(*this);

	if (host) {
		AddInContainer(*host);
		UpdateUnitSightRange(*this);
		UnitInXY(*this, host->tilePos);
		MapMarkUnitSight(*this);
	}

	Removed = 1;

	// Correct surrounding walls directions
	if (this->Type->Wall){
		CorrectWallNeighBours(*this);
	}

	//  Remove unit from the current selection
	if (Selected) {
		if (NumSelected == 1) { //  Remove building cursor
			CancelBuildingMode();
		}
		UnSelectUnit(*this);
		SelectionChanged();
	}
	// Remove unit from team selections
	if (!Selected && TeamSelected) {
		UnSelectUnit(*this);
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
void UnitLost(CUnit &unit)
{
	CUnit *temp;
	CBuildRestrictionOnTop *b;
	const CUnitType *type;
	CPlayer *player = unit.Player;

	Assert(player);  // Next code didn't support no player!

	//
	//  Call back to AI, for killed or lost units.
	//
	if (Editor.Running == EditorNotRunning){
		if (player && player->AiEnabled) {
			AiUnitKilled(unit);
		} else {
			//
			//  Remove unit from its groups
			//
			if (unit.GroupId) {
				RemoveUnitFromGroups(unit);
			}
		}
	}

	//
	//  Remove the unit from the player's units table.
	//
	type = unit.Type;
	if (player && !type->Vanishes) {
		Assert(*unit.PlayerSlot == &unit);
		temp = player->Units[--player->TotalNumUnits];
		temp->PlayerSlot = unit.PlayerSlot;
		*unit.PlayerSlot = temp;
		player->Units[player->TotalNumUnits] = NULL;

		if (unit.Type->Building) {
			// FIXME: support more races
			if (!type->Wall && type != UnitTypeOrcWall && type != UnitTypeHumanWall) {
				player->NumBuildings--;
			}
		}

		if (unit.CurrentAction() != UnitActionBuilt) {
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
	if (unit.CurrentAction() != UnitActionBuilt) {
		player->Supply -= type->Supply;
		// Decrease resource limit
		for (int i = 0; i < MaxCosts; ++i) {
			if (unit.Player->MaxResources[i] != -1 && type->_Storing[i]) {
				const int newMaxValue = unit.Player->MaxResources[i] - type->_Storing[i];

				unit.Player->MaxResources[i] = std::max(0, newMaxValue);
				unit.Player->SetResource(i, unit.Player->Resources[i]);
			}
		}
		//
		//  Handle income improvements, look if a player loses a building
		//  which have given him a better income, find the next best
		//  income.
		//
		for (int i = 1; i < MaxCosts; ++i) {
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
	//  Handle order cancels.
	//
	unit.CurrentOrder()->Cancel(unit);

	DebugPrint("%d: Lost %s(%d)\n"
		_C_ unit.Player->Index
		_C_ unit.Type->Ident.c_str()
		_C_ UnitNumber(unit));

	// Destroy resource-platform, must re-make resource patch.
	if ((b = OnTopDetails(unit, NULL)) != NULL) {
		if (b->ReplaceOnDie && (unit.Type->GivesResource && unit.ResourcesHeld != 0)) {
			temp = MakeUnitAndPlace(unit.tilePos, *b->Parent, &Players[PlayerNumNeutral]);
			if (temp == NoUnitP) {
				DebugPrint("Unable to allocate Unit");
			} else {
				temp->ResourcesHeld = unit.ResourcesHeld;
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
void UnitClearOrders(CUnit &unit)
{
	for (size_t i = 0; i != unit.Orders.size(); ++i)
	{
		delete unit.Orders[i];
	}
	unit.Orders.clear();
	unit.Orders.push_back(COrder::NewActionStill());
	unit.SubAction = unit.State = 0;
}

/**
**  Update for new unit. Food and income ...
**
**  @param unit     New unit pointer.
**  @param upgrade  True unit was upgraded.
*/
void UpdateForNewUnit(const CUnit &unit, int upgrade)
{
	const CUnitType *type = unit.Type;
	CPlayer *player = unit.Player;

	//
	// Handle unit supply and max resources.
	// Note an upgraded unit can't give more supply.
	//
	if (!upgrade) {
		player->Supply += type->Supply;
		for (int i = 0; i < MaxCosts; ++i) {
			if (unit.Player->MaxResources[i] != -1 && type->_Storing[i]) {
				unit.Player->MaxResources[i] += type->_Storing[i];
			}
		}
	}

	//
	// Update resources
	//
	for (int u = 1; u < MaxCosts; ++u) {
		if (player->Incomes[u] < unit.Type->ImproveIncomes[u]) {
			player->Incomes[u] = unit.Type->ImproveIncomes[u];
		}
	}
}

/**
**  Find nearest point of unit.
**
**  @param unit  Pointer to unit.
**  @param pos   tile map postion.
**  @param dpos  Out: nearest point tile map postion to (tx,ty).
*/
void NearestOfUnit(const CUnit &unit, const Vec2i& pos, Vec2i *dpos)
{
	int x = unit.tilePos.x;
	int y = unit.tilePos.y;

	if (pos.x >= x + unit.Type->TileWidth) {
		dpos->x = x + unit.Type->TileWidth - 1;
	} else if (pos.x < x) {
		dpos->x = x;
	} else {
		dpos->x = pos.x;
	}
	if (pos.y >= y + unit.Type->TileHeight) {
		dpos->y = y + unit.Type->TileHeight - 1;
	} else if (pos.y < y) {
		dpos->y = y;
	} else {
		dpos->y = pos.y;
	}
}

/**
**  Copy the unit look in Seen variables. This should be called when
**  buildings go under fog of war for ThisPlayer.
**
**  @param unit  The unit to work on
*/
static void UnitFillSeenValues(CUnit &unit)
{
	// Seen values are undefined for visible units.
	unit.Seen.Y = unit.tilePos.y;
	unit.Seen.X = unit.tilePos.x;
	unit.Seen.IY = unit.IY;
	unit.Seen.IX = unit.IX;
	unit.Seen.Frame = unit.Frame;
	unit.Seen.Type = unit.Type;
	unit.Seen.Constructed = unit.Constructed;

	unit.CurrentOrder()->FillSeenValues(unit);
}

class SamePlayerAndTypeAs
{
public:
	explicit SamePlayerAndTypeAs(const CUnit &unit) :
		player(unit.Player), type(unit.Type)
	{}

	bool operator() (const CUnit *unit) const
	{
		return (unit->Player == player && unit->Type == type);
	}

private:
	const CPlayer *player;
	const CUnitType *type;
};

// Wall unit positions
enum {
	W_NORTH = 0x10,
	W_WEST = 0x20,
	W_SOUTH = 0x40,
	W_EAST = 0x80
};

/**
**  Correct direction for placed wall.
**
**  @param unit    The wall unit.
*/
void CorrectWallDirections(CUnit &unit)
{
	Assert(unit.Type->Wall);
	Assert(unit.Type->NumDirections == 16);
	Assert(!unit.Type->Flip);

	if (!Map.Info.IsPointOnMap(unit.tilePos)) {
		return ;
	}
	const struct {
		Vec2i offset;
		const int dirFlag;
	} configs[] = {{{0, -1}, W_NORTH}, {{1, 0}, W_EAST},
		{{0, 1}, W_SOUTH}, {{-1, 0}, W_WEST}};
	int flags = 0;

	for (int i = 0; i != sizeof (configs) / sizeof (*configs); ++i) {
		const Vec2i pos = unit.tilePos + configs[i].offset;
		const int dirFlag = configs[i].dirFlag;

		if (Map.Info.IsPointOnMap(pos) == false) {
			flags |= dirFlag;
		} else {
			const CUnitCache &unitCache = Map.Field(pos)->UnitCache;
			const CUnit *neighboor = unitCache.find(SamePlayerAndTypeAs(unit));

			if (neighboor != NULL) {
				flags |= dirFlag;
			}
		}
	}
	unit.Direction = flags;
}

/**
** Correct the surrounding walls.
**
** @param unit The wall unit.
*/
void CorrectWallNeighBours(CUnit &unit)
{
	Assert(unit.Type->Wall);

	const Vec2i offset[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

	for (unsigned int i = 0; i < sizeof (offset) / sizeof (*offset); ++i) {
		const Vec2i pos = unit.tilePos + offset[i];

		if (Map.Info.IsPointOnMap(pos) == false) {
			continue;
		}
		CUnitCache &unitCache = Map.Field(pos)->UnitCache;
		CUnit *neighboor = unitCache.find(SamePlayerAndTypeAs(unit));

		if (neighboor != NULL) {
			CorrectWallDirections(*neighboor);
			UnitUpdateHeading(*neighboor);
		}
	}
}

/**
**  This function should get called when a unit goes under fog of war.
**
**  @param unit    The unit that goes under fog.
**  @param player  The player the unit goes out of fog for.
*/
void UnitGoesUnderFog(CUnit &unit, const CPlayer &player)
{
	if (unit.Type->VisibleUnderFog) {
		if (player.Type == PlayerPerson && !unit.Destroyed) {
			unit.RefsIncrease();
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
		if (unit.Destroyed) {
			unit.Seen.Destroyed |= (1 << player.Index);
		}
		if (&player == ThisPlayer) {
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
void UnitGoesOutOfFog(CUnit &unit, const CPlayer &player)
{
	if (!unit.Type->VisibleUnderFog) {
		return;
	}
	if (unit.Seen.ByPlayer & (1 << (player.Index))) {
		if ((player.Type == PlayerPerson) &&
				(!(unit.Seen.Destroyed & (1 << player.Index))) ) {
			unit.RefsDecrease();
		}
	} else {
		unit.Seen.ByPlayer |= (1 << (player.Index));
	}
}

template<const bool MARK>
class _TileSeen
{
public:
	_TileSeen(const CPlayer &p , int c) : player(&p), cloak(c)
	{}

	void operator() (CUnit *const unit) const {
		if (cloak != (int)unit->Type->PermanentCloak) {
			return ;
		}
		const int p = player->Index;
		if (MARK) {
			//  If the unit goes out of fog, this can happen for any player that
			//  this player shares vision with, and can't YET see the unit.
			//  It will be able to see the unit after the Unit->VisCount ++
			if (!unit->VisCount[p]) {
				for (int pi = 0; pi < PlayerMax; ++pi) {
					if ((pi == p /*player->Index*/)
						|| player->IsBothSharedVision(Players[pi])) {
						if (!unit->IsVisible(Players[pi])) {
							UnitGoesOutOfFog(*unit, Players[pi]);
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
			if (!unit->VisCount[p] && unit->CurrentAction() == UnitActionDie) {
				return;
			}

			Assert(unit->VisCount[p]);
			unit->VisCount[p]--;
			//  If the unit goes under of fog, this can happen for any player that
			//  this player shares vision to. First of all, before unmarking,
			//  every player that this player shares vision to can see the unit.
			//  Now we have to check who can't see the unit anymore.
			if (!unit->VisCount[p]) {
				for (int pi = 0; pi < PlayerMax; ++pi) {
					if (pi == p/*player->Index*/ ||
						player->IsBothSharedVision(Players[pi])) {
						if (!unit->IsVisible(Players[pi])) {
							UnitGoesUnderFog(*unit, Players[pi]);
						}
					}
				}
			}
		}
	}
private:
	const CPlayer *player;
	int cloak;
};

/**
**  Mark all units on a tile as now visible.
**
**  @param player  The player this is for.
**  @param x       x location to check
**  @param y       y location to check
**  @param cloak   If we mark cloaked units too.
*/
void UnitsOnTileMarkSeen(const CPlayer &player, const unsigned int index, int cloak)
{
	_TileSeen<true> seen(player, cloak);
	Map.Field(index)->UnitCache.for_each(seen);
}

void UnitsOnTileMarkSeen(const CPlayer &player, int x, int y, int cloak)
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
void UnitsOnTileUnmarkSeen(const CPlayer &player,
				const unsigned int index, int cloak)
{
	_TileSeen<false> seen(player, cloak);
	Map.Field(index)->UnitCache.for_each(seen);
}

void UnitsOnTileUnmarkSeen(const CPlayer &player, int x, int y, int cloak)
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
void UnitCountSeen(CUnit &unit)
{
	Assert(unit.Type);

	// FIXME: optimize, only work on certain players?
	// This is for instance good for updating shared vision...

	//  Store old values in oldv[p]. This store if the player could see the
	//  unit before this calc.
	int oldv[PlayerMax];
	for (int p = 0; p < PlayerMax; ++p) {
		if (Players[p].Type != PlayerNobody) {
			oldv[p] = unit.IsVisible(Players[p]);
		}
	}

	//  Calculate new VisCount values.
	const int height = unit.Type->TileHeight;
	const int width = unit.Type->TileWidth;

	for (int p = 0; p < PlayerMax; ++p) {
		if (Players[p].Type != PlayerNobody) {
			int newv = 0;
			int y = height;
			unsigned int index = unit.Offset;
			do {
				CMapField *mf = Map.Field(index);
				int x = width;
				do {
					if (unit.Type->PermanentCloak && unit.Player != &Players[p]) {
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
			unit.VisCount[p] = newv;
		}
	}

	//
	// Now here comes the tricky part. We have to go in and out of fog
	// for players. Hopefully this works with shared vision just great.
	//
	for (int p = 0; p < PlayerMax; ++p) {
		if (Players[p].Type != PlayerNobody) {
			int newv = unit.IsVisible(Players[p]);
			if (!oldv[p] && newv) {
				UnitGoesOutOfFog(unit, Players[p]);
				// Might have revealed a destroyed unit which caused it to
				// be released
				if (!unit.Type) {
					break;
				}
			}
			if (oldv[p] && !newv) {
				UnitGoesUnderFog(unit, Players[p]);
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
bool CUnit::IsVisible(const CPlayer &player) const
{
	if (VisCount[player.Index]) {
		return true;
	}
	for (int p = 0; p < PlayerMax; ++p) {
		if (p != player.Index && player.IsBothSharedVision(Players[p])) {
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
	if (IsInvisibile(*ThisPlayer)) {
		return false;
	}
	if (IsVisible(*ThisPlayer) || ReplayRevealMap || IsVisibleOnRadar(*ThisPlayer))
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
	int x = tilePos.x * PixelTileSize.x + IX - (Type->Width - Type->TileWidth * PixelTileSize.x) / 2 + Type->OffsetX;
	int y = tilePos.y * PixelTileSize.y + IY - (Type->Height - Type->TileHeight * PixelTileSize.y) / 2 + Type->OffsetY;

	if (x + Type->Width < vp->MapX * PixelTileSize.x + vp->OffsetX ||
			x > vp->MapX * PixelTileSize.x + vp->OffsetX + (vp->EndX - vp->X) ||
			y + Type->Height < vp->MapY * PixelTileSize.y + vp->OffsetY ||
			y > vp->MapY * PixelTileSize.y + vp->OffsetY + (vp->EndY - vp->Y))
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
	if (IsInvisibile(*ThisPlayer)) {
		return false;
	}

	if (IsVisible(*ThisPlayer) || ReplayRevealMap) {
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
	*sx = tilePos.x - (IX < 0);
	*ex = *sx + Type->TileWidth - !IX;
	*sy = tilePos.y - (IY < 0);
	*ey = *sy + Type->TileHeight - !IY;
}

/**
**  Change the unit's owner
**
**  @param newplayer  New owning player.
*/
void CUnit::ChangeOwner(CPlayer &newplayer)
{
	CPlayer *oldplayer = Player;

	// This shouldn't happen
	if (oldplayer == &newplayer) {
		DebugPrint("Change the unit owner to the same player???\n");
		return;
	}

	// Rescue all units in buildings/transporters.
	CUnit *uins = UnitInside;
	for (int i = InsideCount; i; --i, uins = uins->NextContained) {
		uins->ChangeOwner(newplayer);
	}

	//  Must change food/gold and other.
	UnitLost(*this);

	//  Now the new side!

	// Insert into new player table.

	PlayerSlot = newplayer.Units + newplayer.TotalNumUnits++;
	if (Type->Building) {
		if (!Type->Wall) {
			newplayer.TotalBuildings++;
		}
	} else {
		newplayer.TotalUnits++;
	}
	*PlayerSlot = this;

	MapUnmarkUnitSight(*this);
	MapUnmarkUnitGuard(*this);
	Player = &newplayer;
	Stats = &Type->Stats[newplayer.Index];
	UpdateUnitSightRange(*this);
	MapMarkUnitGuard(*this);
	MapMarkUnitSight(*this);

	//  Must change food/gold and other.
	if (Type->GivesResource) {
		DebugPrint("Resource transfer not supported\n");
	}
	newplayer.Demand += Type->Demand;
	newplayer.Supply += Type->Supply;
	// Increase resource limit
	for (int i = 0; i < MaxCosts; ++i) {
		if (newplayer.MaxResources[i] != -1 && Type->_Storing[i]) {
			newplayer.MaxResources[i] += Type->_Storing[i];
		}
	}
	if (Type->Building && !Type->Wall) {
		newplayer.NumBuildings++;
	}
	newplayer.UnitTypesCount[Type->Slot]++;

	UpdateForNewUnit(*this, 1);
}

static bool IsMineAssignedBy(const CUnit &mine, const CUnit &worker)
{
	for (CUnit* it = mine.Resource.Workers; it; it = it->NextWorker) {
		if (it == &worker) {
			return true;
		}
	}
	return false;
}


void CUnit::AssignWorkerToMine(CUnit &mine)
{
	if (IsMineAssignedBy(mine, *this) == true) {
		return;
	}
	Assert(this->NextWorker == NULL);

	CUnit *head = mine.Resource.Workers;
/*
	DebugPrint("%d: Worker [%d] is adding into %s [%d] on %d pos\n"
					_C_ this->Player->Index _C_ this->Slot
					_C_ mine.Type->Name.c_str()
					_C_ mine.Slot
					_C_ mine.Data.Resource.Assigned);
*/
	this->RefsIncrease();
	this->NextWorker = head;
	mine.Resource.Workers = this;
	mine.Resource.Assigned++;
}

void CUnit::DeAssignWorkerFromMine(CUnit &mine)
{
	if (IsMineAssignedBy(mine, *this) == false) {
		return ;
	}
	CUnit *prev = NULL, *worker = mine.Resource.Workers;
/*
	DebugPrint("%d: Worker [%d] is removing from %s [%d] left %d units assigned\n"
					_C_ this->Player->Index _C_ this->Slot
					_C_ mine.Type->Name.c_str()
					_C_ mine.Slot
					_C_ mine.CurrentOrder()->Data.Resource.Assigned);
*/
	for (int i = 0; NULL != worker; worker = worker->NextWorker, ++i)
	{
		if (worker == this) {
			CUnit *next = worker->NextWorker;
			worker->NextWorker = NULL;
			if (prev) {
				prev->NextWorker = next;
			}
			if (worker == mine.Resource.Workers) {
				mine.Resource.Workers = next;
			}
			worker->RefsDecrease();
			break;
		}
		prev = worker;
		Assert(i <= mine.Resource.Assigned);
	}
	mine.Resource.Assigned--;
	Assert(mine.Resource.Assigned >= 0);
}


/**
**  Change the owner of all units of a player.
**
**  @param oldplayer    Old owning player.
**  @param newplayer    New owning player.
*/
static void ChangePlayerOwner(CPlayer &oldplayer, CPlayer &newplayer)
{
	CUnit *table[UnitMax];

	// NOTE: table is changed.
	int n = oldplayer.TotalNumUnits;
	memcpy(table, oldplayer.Units, n * sizeof(CUnit *));
	for (int i = 0; i < n; ++i) {
		CUnit &unit = *table[i];
		// Don't save the unit again(can happen when inside a town hall)
		if (unit.Player == &newplayer) {
			continue;
		}
		unit.ChangeOwner(newplayer);
		unit.Blink = 5;
		unit.RescuedFrom = &oldplayer;
	}
}

/**
**  Rescue units.
**
**  Look through all rescueable players, if they could be rescued.
*/
void RescueUnits()
{
	CUnit *table[UnitMax];
	CUnit *around[UnitMax];
	int n;

	if (NoRescueCheck) {  // all possible units are rescued
		return;
	}
	NoRescueCheck = true;

	//  Look if player could be rescued.
	for (CPlayer *p = Players; p < Players + NumPlayers; ++p) {
		if (p->Type != PlayerRescuePassive && p->Type != PlayerRescueActive) {
			continue;
		}
		if (p->TotalNumUnits) {
			NoRescueCheck = false;
			// NOTE: table is changed.
			const int l = p->TotalNumUnits;
			memcpy(table, p->Units, l * sizeof(CUnit *));
			for (int j = 0; j < l; ++j) {
				CUnit *unit = table[j];
				// Do not rescue removed units. Units inside something are
				// rescued by ChangeUnitOwner
				if (unit->Removed) {
					continue;
				}

				if (unit->Type->UnitType == UnitTypeLand) {
					n = Map.Select(
							unit->tilePos.x - 1, unit->tilePos.y - 1,
							unit->tilePos.x + unit->Type->TileWidth + 1,
							unit->tilePos.y + unit->Type->TileHeight + 1, around);
				} else {
					n = Map.Select(
							unit->tilePos.x - 2, unit->tilePos.y - 2,
							unit->tilePos.x + unit->Type->TileWidth + 2,
							unit->tilePos.y + unit->Type->TileHeight + 2, around);
				}
				//
				//  Look if ally near the unit.
				//
				for (int i = 0; i < n; ++i) {
					if (around[i]->Type->CanAttack && unit->IsAllied(*around[i])) {
						//
						//  City center converts complete race
						//  NOTE: I use a trick here, centers could
						//        store gold. FIXME!!!
						if (unit->Type->CanStore[GoldCost]) {
							ChangePlayerOwner(*p, *around[i]->Player);
							break;
						}
						unit->RescuedFrom = unit->Player;
						unit->ChangeOwner(*around[i]->Player);
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
**  @param delta  Delta.
**
**  @return         Angle (0..255)
*/
int DirectionToHeading(const Vec2i& delta)
{
	//
	//  Check which quadrant.
	//
	if (delta.x > 0) {
		if (delta.y < 0) { // Quadrant 1?
			return myatan((delta.x * 64) / -delta.y);
		}
		// Quadrant 2?
		return myatan((delta.y * 64) / delta.x) + 64;
	}
	if (delta.y > 0) { // Quadrant 3?
		return myatan((delta.x * -64) / delta.y) + 64 * 2;
	}
	if (delta.x) { // Quadrant 4.
		return myatan((delta.y * -64) / -delta.x) + 64 * 3;
	}
	return 0;
}

/**
**  Convert direction to heading.
**
**  @param delta  Delta.
**
**  @return         Angle (0..255)
*/
int DirectionToHeading(const PixelDiff& delta)
{
	// code is identic for Vec2i and PixelDiff
	Vec2i delta2 = {delta.x, delta.y};
	return DirectionToHeading(delta2);
}

/**
**  Update sprite frame for new heading.
*/
void UnitUpdateHeading(CUnit &unit)
{
	int dir;
	int nextdir;
	bool neg;

	if (unit.Frame < 0) {
		unit.Frame = -unit.Frame - 1;
		neg = true;
	} else {
		neg = false;
	}
	unit.Frame /= unit.Type->NumDirections / 2 + 1;
	unit.Frame *= unit.Type->NumDirections / 2 + 1;
	// Remove heading, keep animation frame

	nextdir = 256 / unit.Type->NumDirections;
	dir = ((unit.Direction + nextdir / 2) & 0xFF) / nextdir;
	if (dir <= LookingS / nextdir) { // north->east->south
		unit.Frame += dir;
	} else {
		unit.Frame += 256 / nextdir - dir;
		unit.Frame = -unit.Frame - 1;
	}
	if (neg && !unit.Frame && unit.Type->Building) {
		unit.Frame = -1;
	}
}

/**
**  Change unit heading/frame from delta direction x, y.
**
**  @param unit  Unit for new direction looking.
**  @param delta  map tile delta direction.
*/
void UnitHeadingFromDeltaXY(CUnit &unit, const Vec2i &delta)
{
	unit.Direction = DirectionToHeading(delta);
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
**  @param container  Unit "containing" unit to drop (may be different of unit.Container).
*/
void DropOutOnSide(CUnit &unit, int heading, const CUnit *container)
{
	Vec2i pos;
	int addx = 0;
	int addy = 0;

	if (container) {
		pos = container->tilePos;
		pos.x -= unit.Type->TileWidth - 1;
		pos.y -= unit.Type->TileHeight - 1;
		addx = container->Type->TileWidth + unit.Type->TileWidth - 1;
		addy = container->Type->TileHeight + unit.Type->TileHeight - 1;

		if (heading < LookingNE || heading > LookingNW) {
			pos.x += addx - 1;
			--pos.y;
			goto startn;
		} else if (heading < LookingSE) {
			pos.x += addx;
			pos.y += addy - 1;
			goto starte;
		} else if (heading < LookingSW) {
			pos.y += addy;
			goto starts;
		} else {
			--pos.x;
			goto startw;
		}
	} else {
		pos = unit.tilePos;

		if (heading < LookingNE || heading > LookingNW) {
			goto starts;
		} else if (heading < LookingSE) {
			goto startw;
		} else if (heading < LookingSW) {
			goto startn;
		} else {
			goto starte;
		}
	}
	// FIXME: don't search outside of the map
	for (;;) {
startw:
		for (int i = addy; i--; ++pos.y) {
			if (UnitCanBeAt(unit, pos)) {
				goto found;
			}
		}
		++addx;
starts:
		for (int i = addx; i--; ++pos.x) {
			if (UnitCanBeAt(unit, pos)) {
				goto found;
			}
		}
		++addy;
starte:
		for (int i = addy; i--; --pos.y) {
			if (UnitCanBeAt(unit, pos)) {
				goto found;
			}
		}
		++addx;
startn:
		for (int i = addx; i--; --pos.x) {
			if (UnitCanBeAt(unit, pos)) {
				goto found;
			}
		}
		++addy;
	}

found:
	unit.Place(pos);
}

/**
**  Place a unit on the map nearest to goalPos.
**
**  @param unit  Unit to drop out.
**  @param goalPos Goal map tile position.
**  @param addx  Tile width of unit it's dropping out of.
**  @param addy  Tile height of unit it's dropping out of.
*/
void DropOutNearest(CUnit &unit, const Vec2i &goalPos, const CUnit *container)
{
	Vec2i pos;
	Vec2i bestPos = {0, 0};
	int bestd = 99999;
	int addx = 0;
	int addy = 0;
	Assert(unit.Removed);

	if (container) {
		pos = container->tilePos;
		pos.x -= unit.Type->TileWidth - 1;
		pos.y -= unit.Type->TileHeight - 1;
		addx = container->Type->TileWidth + unit.Type->TileWidth - 1;
		addy = container->Type->TileHeight + unit.Type->TileHeight - 1;
		--pos.x;
	} else {
		pos = unit.tilePos;
	}
	// FIXME: if we reach the map borders we can go fast up, left, ...

	for (;;) {
		for (int i = addy; i--; ++pos.y) { // go down
			if (UnitCanBeAt(unit, pos)) {
				const int n = MapDistance(goalPos, pos);

				if (n < bestd) {
					bestd = n;
					bestPos = pos;
				}
			}
		}
		++addx;
		for (int i = addx; i--; ++pos.x) { // go right
			if (UnitCanBeAt(unit, pos)) {
				const int n = MapDistance(goalPos, pos);

				if (n < bestd) {
					bestd = n;
					bestPos = pos;
				}
			}
		}
		++addy;
		for (int i = addy; i--; --pos.y) { // go up
			if (UnitCanBeAt(unit, pos)) {
				const int n = MapDistance(goalPos, pos);

				if (n < bestd) {
					bestd = n;
					bestPos = pos;
				}
			}
		}
		++addx;
		for (int i = addx; i--; --pos.x) { // go left
			if (UnitCanBeAt(unit, pos)) {
				const int n = MapDistance(goalPos, pos);

				if (n < bestd) {
					bestd = n;
					bestPos = pos;
				}
			}
		}
		if (bestd != 99999) {
			unit.Place(bestPos);
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
void DropOutAll(const CUnit &source)
{
	CUnit *unit = source.UnitInside;

	for (int i = source.InsideCount; i; --i, unit = unit->NextContained) {
		DropOutOnSide(*unit, LookingW, &source);
		Assert(!unit->CurrentOrder()->HasGoal());
		unit->CurrentOrder()->Action = UnitActionStill;
		unit->SubAction = 0;
	}
}

/*----------------------------------------------------------------------------
  -- Finding units
  ----------------------------------------------------------------------------*/

/**
**  Find the closest piece of wood for an unit.
**
**  @param unit    The unit.
**  @param pos     OUT: Map position of tile.
*/
int FindWoodInSight(const CUnit &unit, Vec2i *pos)
{
	return FindTerrainType(unit.Type->MovementMask, 0, MapFieldForest, 9999,
		unit.Player, unit.tilePos, pos);
}

/**
**  Find the closest piece of terrain with the given flags.
**
**  @param movemask    The movement mask to reach that location.
**  @param resmask     Result tile mask.
**  @param rvresult    Return a tile that doesn't match.
**  @param range       Maximum distance for the search.
**  @param player      Only search fields explored by player
**  @param startPos    Map start position for the search.
**
**  @param pos         OUT: Map position of tile.
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
	const CPlayer *player, const Vec2i &startPos, Vec2i *terrainPos)
{
	const Vec2i offset[] = {{0, -1}, {-1, 0}, {1, 0}, {0, 1}, {-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
	Vec2i *points;
	int size;
	Vec2i rpos;
	int wp;
	int rp;
	int ep;
	int i;
	int w;
	unsigned char *m;
	unsigned char *matrix;
	int cdist;
	Vec2i pos(startPos);

	size = std::min<int>(Map.Info.MapWidth * Map.Info.MapHeight / 4, range * range * 5);
	points = new Vec2i[size];

	// Make movement matrix. FIXME: can create smaller matrix.
	matrix = CreateMatrix();
	w = Map.Info.MapWidth + 2;
	matrix += w + w + 2;
	points[0] = pos;
	rp = 0;
	matrix[pos.x + pos.y * w] = 1; // mark start point
	ep = wp = 1; // start with one point
	cdist = 0; // current distance is 0

	//
	// Pop a point from stack, push all neighbors which could be entered.
	//
	for (;;) {
		while (rp != ep) {
			rpos = points[rp];
			for (i = 0; i < 8; ++i) { // mark all neighbors
				pos = rpos + offset[i];
				//  Make sure we don't leave the map.
				if (!Map.Info.IsPointOnMap(pos)) {
					continue;
				}
				m = matrix + pos.x + pos.y * w;
				/*
				 *  Check if visited or unexplored for non
				 *	AI players (our exploration code is too week for real
				 *	competition with human players)
				 */
				if (*m || (player && !player->AiEnabled &&!Map.IsFieldExplored(*player, pos))) {
					continue;
				}
				// Look if found what was required.
				bool can_move_to = CanMoveToMask(pos, resmask);
				if ((rvresult ? can_move_to : !can_move_to)) {
					*terrainPos = pos;
					delete[] points;
					return 1;
				}
				if (CanMoveToMask(pos, movemask)) { // reachable
					*m = 1;
					points[wp] = pos; // push the point
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
					d = worker->MapDistanceTo(*dest);
				} else {
					d = worker->Container->MapDistanceTo(*dest);
				}

				// Use Circle, not square :)
				if (d > range) {
					return;
				}

				// calck real travel distance
				if (!worker->Container) {
					d = UnitReachable(*worker, *dest, 1);
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
	BestDepotFinder(const CUnit &w, int res, int ran) :
		resource(res), range(ran),
		best_dist(INT_MAX), best_depot(0)
	{
		u_near.worker = &w;
	}

	BestDepotFinder(const Vec2i &pos, int res, int ran) :
		resource(res), range(ran),
		best_dist(INT_MAX), best_depot(0)
	{
		u_near.loc = pos;
	}

	CUnit *Find(CUnit **table, const int table_size) {
#ifdef _MSC_VER
		for (int i = 0; i < table_size; ++i) {
			this->operator() (table[i]);
		}
#else
		if (table_size) {
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
private:
	union {
		const CUnit *worker;
		Vec2i loc;
	} u_near;
	const int resource;
	const int range;
	int best_dist;
public:
	CUnit *best_depot;
};

CUnit *FindDepositNearLoc(CPlayer &p, const Vec2i &pos, int range, int resource)
{
	BestDepotFinder<true> finder(pos, resource, range);
	CUnit *depot = finder.Find(p.Units, p.TotalNumUnits);

	if (!depot) {
		for (int i = 0; i < PlayerMax; ++i) {
			if (i != p.Index &&
				Players[i].IsAllied(p) &&
				p.IsAllied(Players[i])) {
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
**  @param startPos    Find closest unit from this location
**  @param range       Maximum distance to the resource.
**  @param resource    The resource id.
**
**  @note This will return an usable resource building that doesn't
**  belong to the player or one of his allies.
**
**  @return            NoUnitP or resource unit
*/
CUnit *UnitFindResource(const CUnit &unit, const Vec2i &startPos, int range, int resource,
		bool check_usage, const CUnit *destu)
{
	const Vec2i offset[] = {{0, 0}, {0, -1}, {-1, 0}, {1, 0}, {0, 1}, {-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
	Vec2i *points;
	int size;
	Vec2i rpos;
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
	Vec2i pos = startPos;
	Vec2i dest = pos;
	int bestd = 99999, bestw = 99999, besta = 99999;
	int cdist;
	const ResourceInfo &resinfo = *unit.Type->ResInfo[resource];

	size = std::min<int>(Map.Info.MapWidth * Map.Info.MapHeight / 4, range * range * 5);
	points = new Vec2i[size];

	// Find the nearest gold depot
	if (!destu)
		destu = FindDepositNearLoc(*unit.Player, pos, range, resource);
	if (destu) {
		NearestOfUnit(*destu, pos, &dest);
	}

	// Make movement matrix. FIXME: can create smaller matrix.
	matrix = CreateMatrix();
	w = Map.Info.MapWidth + 2;
	matrix += w + w + 2;
	//  Unit movement mask
	mask = unit.Type->MovementMask;
	//  Ignore all units along the way. Might seem wierd, but otherwise
	//  peasants would lock at a mine with a lot of workers.
	mask &= ~(MapFieldLandUnit | MapFieldSeaUnit | MapFieldAirUnit);
	points[0] = pos;
	rp = 0;
	if (unit.tilePos == pos)
		matrix[pos.x + pos.y * w] = 1; // mark start point
	ep = wp = 1; // start with one point
	cdist = 0; // current distance is 0
	bestmine = NoUnitP;

	CResourceFinder res_finder(resource, 1);

	//
	// Pop a point from stack, push all neighbors which could be entered.
	//
	for (;;) {
		while (rp != ep) {
			rpos = points[rp];
			for (i = 0; i < 9; ++i) { // mark all neighbors
				pos = rpos + offset[i];
				m = matrix + pos.x + pos.y * w;
				if (*m) { // already checked
					continue;
				}

				/*
				 *  Check if unexplored for non AI players only.
				 *  Our exploration code is too week for real
				 *	competition with human players.
				 */
				if (!unit.Player->AiEnabled &&
					 !Map.IsFieldExplored(*unit.Player, pos)) { // Unknown.
					continue;
				}

				//
				// Look if there is a mine
				//
				if ((mine = res_finder.Find(Map.Field(pos))) &&
					mine->Type->CanHarvest && mine->ResourcesHeld &&
						(resinfo.HarvestFromOutside ||
							mine->Player->Index == PlayerMax - 1 ||
							mine->Player == unit.Player ||
							(unit.IsAllied(*mine) && mine->IsAllied(unit)))
							 ) {
					if (destu) {
						bool better = (mine != bestmine);

						if(better) {
							n = std::max<int>(MyAbs(dest.x - pos.x), MyAbs(dest.y - pos.y));
							if(check_usage && mine->Type->MaxOnBoard) {
								int assign = mine->Resource.Assigned - mine->Type->MaxOnBoard;
								int waiting = (assign > 0 ? GetNumWaitingWorkers(*mine) : 0);
								if (bestmine != NoUnitP) {
									if (besta >= assign)
									{
										if (assign > 0) {
											waiting = GetNumWaitingWorkers(*mine);
											if (besta == assign) {
												if (bestw < waiting) {
													better = false;
												} else {
													if (bestw == waiting && bestd < n) {
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
								if (better) {
									besta = assign;
									bestw = waiting;
								}
							} else {
								if (bestmine != NoUnitP && bestd < n) {
									better = false;
								}
							}
						}
						if (better) {
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
								int assign = mine->Resource.Assigned - mine->Type->MaxOnBoard;
								int waiting = (assign > 0 ? GetNumWaitingWorkers(*mine) : 0);
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

				if (CanMoveToMask(pos, mask)) { // reachable
					*m = 1;
					points[wp] = pos; // push the point
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
CUnit *FindDeposit(const CUnit &unit, int range, int resource)
{
	BestDepotFinder<false> finder(unit, resource, range);
	CUnit *depot = finder.Find(unit.Player->Units, unit.Player->TotalNumUnits);
	if (!depot) {
		for (int i = 0; i < PlayerMax; ++i) {
			if (i != unit.Player->Index &&
				Players[i].IsAllied(*unit.Player) &&
				unit.Player->IsAllied(Players[i])) {
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
CUnit *FindIdleWorker(const CPlayer &player, const CUnit *last)
{
	CUnit *FirstUnitFound = NoUnitP;
	int SelectNextUnit = (last == NoUnitP) ? 1 : 0;
	const int nunits = player.TotalNumUnits;

	for (int i = 0; i < nunits; ++i) {
		CUnit &unit = *player.Units[i];
		if (unit.Type->Harvester && unit.Type->ResInfo && !unit.Removed) {
			if (unit.CurrentAction() == UnitActionStill) {
				if (SelectNextUnit && !IsOnlySelected(unit)) {
					return &unit;
				}
				if (FirstUnitFound == NULL) {
					FirstUnitFound = &unit;
				}
			}
		}
		if (&unit == last) {
			SelectNextUnit = 1;
		}
	}
	if (FirstUnitFound != NoUnitP && !IsOnlySelected(*FirstUnitFound)) {
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
		if (!ReplayRevealMap && !unit->IsVisibleAsGoal(*ThisPlayer)) {
			continue;
		}
		type = unit->Type;

		//
		// Check if mouse is over the unit.
		//
		gx = unit->tilePos.x * PixelTileSize.x + unit->IX;

		{
			const int local_width = type->TileWidth * PixelTileSize.x;
			if (x + (type->BoxWidth - local_width) / 2 < gx) {
				continue;
			}
			if (x > gx + (local_width + type->BoxWidth) / 2) {
				continue;
			}
		}

		gy = unit->tilePos.y * PixelTileSize.y + unit->IY;
		{
			const int local_height = type->TileHeight * PixelTileSize.y;
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

PixelPos CUnit::GetMapPixelPosCenter() const
{
	const PixelPos center = { tilePos.x * PixelTileSize.x + IX + Type->TileWidth * PixelTileSize.x / 2,
							tilePos.y * PixelTileSize.y + IY + Type->TileHeight * PixelTileSize.y / 2};
	return center;
}

/**
**  Let an unit die.
**
**  @param unit    Unit to be destroyed.
*/
void LetUnitDie(CUnit &unit)
{
	unit.Moving = 0;
	unit.TTL = 0;
	unit.Anim.Unbreakable = 0;

	CUnitType *type = unit.Type;

	while (unit.Resource.Workers) {
		unit.Resource.Workers->DeAssignWorkerFromMine(unit);
	}

	// removed units,  just remove.
	if (unit.Removed) {
		DebugPrint("Killing a removed unit?\n");
		UnitLost(unit);
		UnitClearOrders(unit);
		unit.Release();
		return;
	}

	PlayUnitSound(unit, VoiceDying);

	//
	// Catapults,... explodes.
	//
	if (type->ExplodeWhenKilled) {
		const PixelPos pixelPos = unit.GetMapPixelPosCenter();

		MakeMissile(*type->Explosion.Missile, pixelPos, pixelPos);
	}
	if (type->DeathExplosion) {
		const PixelPos pixelPos = unit.GetMapPixelPosCenter();

		type->DeathExplosion->pushPreamble();
		type->DeathExplosion->pushInteger(pixelPos.x);
		type->DeathExplosion->pushInteger(pixelPos.y);
		type->DeathExplosion->run();
	}
	// Handle Teleporter Destination Removal
	if (type->Teleporter && unit.Goal) {
		unit.Goal->Remove(NULL);
		UnitLost(*unit.Goal);
		UnitClearOrders(*unit.Goal);
		unit.Goal->Release();
		unit.Goal = NULL;
	}

	// Transporters lose or save their units and building their workers
	if (unit.UnitInside && unit.Type->SaveCargo)
		DropOutAll(unit);
	else if (unit.UnitInside)
		DestroyAllInside(unit);

	unit.Remove(NULL);
	UnitLost(unit);
	UnitClearOrders(unit);


	// Unit has death animation.

	// Not good: UnitUpdateHeading(unit);
	unit.SubAction = 0;
	unit.State = 0;
	delete unit.Orders[0];
	unit.Orders[0] = COrder::NewActionDie();
	if (type->CorpseType) {
#ifdef DYNAMIC_LOAD
		if (!type->Sprite) {
			LoadUnitTypeSprite(type);
		}
#endif
		unit.IX = (type->CorpseType->Width - type->CorpseType->Sprite->Width) / 2;
		unit.IY = (type->CorpseType->Height - type->CorpseType->Sprite->Height) / 2;

		unit.CurrentSightRange = type->CorpseType->Stats[unit.Player->Index].Variables[SIGHTRANGE_INDEX].Max;
	} else {
		unit.CurrentSightRange = 0;
	}

	// If we have a corpse, or a death animation, we are put back on the map
	// This enables us to be tracked.  Possibly for spells (eg raise dead)
	if (type->CorpseType || (type->Animations && type->Animations->Death)) {
		unit.Removed = 0;
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
void DestroyAllInside(CUnit &source)
{
	CUnit *unit = source.UnitInside;

	// No Corpses, we are inside something, and we can't be seen
	for (int i = source.InsideCount; i; --i, unit = unit->NextContained) {
		// Transporter inside a transporter?
		if (unit->UnitInside) {
			DestroyAllInside(*unit);
		}
		UnitLost(*unit);
		UnitClearOrders(*unit);
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
void HitUnit(CUnit *attacker, CUnit &target, int damage)
{
	// Can now happen by splash damage
	// Multiple places send x/y as damage, which may be zero
	if (!damage) {
		return;
	}
	// Units with 0 hp can't be hit
	if (target.Variable[HP_INDEX].Value == 0) {
		return;
	}

	Assert(damage != 0 && target.CurrentAction() != UnitActionDie && !target.Type->Vanishes);

	if (target.Variable[UNHOLYARMOR_INDEX].Value > 0 || target.Type->Indestructible) {
		// vladi: units with active UnholyArmour are invulnerable
		return;
	}
	if (target.Removed) {
		DebugPrint("Removed target hit\n");
		return;
	}
	if (GodMode) {
		if (attacker && attacker->Player == ThisPlayer) {
			damage = target.Variable[HP_INDEX].Value;
		}
		if (target.Player == ThisPlayer) {
			damage = 0;
		}
	}
	const CUnitType *type = target.Type;
	const unsigned long lastattack = target.Attacked;
	target.Attacked = GameCycle ? GameCycle : 1;
	if (attacker) {
		target.DamagedType = ExtraDeathIndex(attacker->Type->DamageType.c_str());
	}
	if (!target.Type->Wall && (!lastattack || lastattack + 2 * CYCLES_PER_SECOND < GameCycle)) {
		// NOTE: perhaps this should also be moved into the notify?
		if (target.Player == ThisPlayer) {
			// FIXME: Problem with load+save.

			//
			// One help cry each 2 second is enough
			// If on same area ignore it for 2 minutes.
			//
			if (HelpMeLastCycle < GameCycle) {
				if (!HelpMeLastCycle ||
						HelpMeLastCycle + CYCLES_PER_SECOND * 120 < GameCycle ||
						target.tilePos.x < HelpMeLastX - 14 ||
						target.tilePos.x > HelpMeLastX + 14 ||
						target.tilePos.y < HelpMeLastY - 14 ||
						target.tilePos.y > HelpMeLastY + 14) {
					HelpMeLastCycle = GameCycle + CYCLES_PER_SECOND * 2;
					HelpMeLastX = target.tilePos.x;
					HelpMeLastY = target.tilePos.y;
					PlayUnitSound(target, VoiceHelpMe);
				}
			}
		}
		target.Player->Notify(NotifyRed, target.tilePos.x, target.tilePos.y,
			_("%s attacked"), target.Type->Name.c_str());

		if (attacker && !target.Type->Building) {
			if (target.Player->AiEnabled) {
				AiHelpMe(attacker, target);
			}
		}
	}

	if (attacker && !target.Type->Wall && target.Type->Building && target.Player->AiEnabled) {
		AiHelpMe(attacker, target);
	}

	if ((target.Variable[HP_INDEX].Value <= damage && attacker && attacker->Type->ShieldPiercing) ||
		(target.Variable[HP_INDEX].Value <= damage - target.Variable[SHIELD_INDEX].Value)) { // unit is killed or destroyed
		//  increase scores of the attacker, but not if attacking it's own units.
		//  prevents cheating by killing your own units.
		if (attacker && target.IsEnemy(*attacker)) {
			attacker->Player->Score += target.Variable[POINTS_INDEX].Value;
			if (type->Building) {
				attacker->Player->TotalRazings++;
			} else {
				attacker->Player->TotalKills++;
			}
			if (UseHPForXp) {
				attacker->Variable[XP_INDEX].Max += target.Variable[HP_INDEX].Value;
			} else {
				attacker->Variable[XP_INDEX].Max += target.Variable[POINTS_INDEX].Value;
			}
			attacker->Variable[XP_INDEX].Value = attacker->Variable[XP_INDEX].Max;
			attacker->Variable[KILL_INDEX].Value++;
			attacker->Variable[KILL_INDEX].Max++;
			attacker->Variable[KILL_INDEX].Enable = 1;
		}
		LetUnitDie(target);
		return;
	}
	if (attacker && attacker->Type->ShieldPiercing)
		target.Variable[HP_INDEX].Value -= damage;
	else if (target.Variable[SHIELD_INDEX].Value >= damage)
		target.Variable[SHIELD_INDEX].Value -= damage;
	else
	{
		target.Variable[HP_INDEX].Value -= damage - target.Variable[SHIELD_INDEX].Value;
		target.Variable[SHIELD_INDEX].Value = 0;
	}
	if (UseHPForXp && attacker && target.IsEnemy(*attacker)) {
		attacker->Variable[XP_INDEX].Value += damage;
		attacker->Variable[XP_INDEX].Max += damage;
	}

	// FIXME: this is dumb. I made repairers capture. crap.
	// david: capture enemy buildings
	// Only worker types can capture.
	// Still possible to destroy building if not careful (too many attackers)
	if (EnableBuildingCapture && attacker &&
			type->Building && target.Variable[HP_INDEX].Value <= damage * 3 &&
			attacker->IsEnemy(target) &&
			attacker->Type->RepairRange) {
		target.ChangeOwner(*attacker->Player);
		CommandStopUnit(*attacker); // Attacker shouldn't continue attack!
	}

	const PixelPos targetPixelCenter = target.GetMapPixelPosCenter();

	if ((target.IsVisibleOnMap(*ThisPlayer) || ReplayRevealMap) && !DamageMissile.empty()) {
		MissileType *mtype = MissileTypeByIdent(DamageMissile);
		const PixelDiff offset = {3, -mtype->Range};

		MakeLocalMissile(*mtype, targetPixelCenter, targetPixelCenter + offset)->Damage = -damage;
	}

	// Show impact missiles
	if (target.Variable[SHIELD_INDEX].Value > 0
		&& !target.Type->Impact[ANIMATIONS_DEATHTYPES + 1].Name.empty()) { // shield impact
		MakeMissile(*target.Type->Impact[ANIMATIONS_DEATHTYPES + 1].Missile, targetPixelCenter, targetPixelCenter);
	} else if (target.DamagedType && !target.Type->Impact[target.DamagedType].Name.empty()) { // specific to damage type impact
		MakeMissile(*target.Type->Impact[target.DamagedType].Missile, targetPixelCenter, targetPixelCenter);
	} else if (!target.Type->Impact[ANIMATIONS_DEATHTYPES].Name.empty()) { // generic impact
		MakeMissile(*target.Type->Impact[ANIMATIONS_DEATHTYPES].Missile, targetPixelCenter, targetPixelCenter);
	}

	if (type->Building && !target.Burning) {
		const int f = (100 * target.Variable[HP_INDEX].Value) / target.Variable[HP_INDEX].Max;
		MissileType *fire = MissileBurningBuilding(f);

		if (fire) {
			const PixelDiff offset = {0, -PixelTileSize.y};
			Missile *missile = MakeMissile(*fire, targetPixelCenter - offset, targetPixelCenter - offset);

			target.RefsIncrease();
			missile->SourceUnit = &target;
			target.Burning = 1;
		}
	}

	/* Target Reaction on Hit */
	if (target.Player->AiEnabled) {
		if (target.CurrentOrder()->OnAiHitUnit(target, attacker, damage)) {
			return;
		}
	}

	// Attack units in range (which or the attacker?)
	if (attacker && target.IsAgressive() && target.CanMove()) {
		if (target.CurrentAction() != UnitActionStill && !target.Player->AiEnabled) {
			return;
		}
		CUnit *goal;

		if (RevealAttacker && CanTarget(target.Type, attacker->Type)) {
			// Reveal Unit that is attacking
			goal = attacker;
		} else {
			if (target.CurrentAction() == UnitActionStandGround) {
				goal = AttackUnitsInRange(target);
			} else {
				// Check for any other units in range
				goal = AttackUnitsInReactRange(target);
			}
		}
		if (goal) {
			COrder *savedOrder = target.CurrentOrder()->Clone();

			CommandAttack(target, goal->tilePos, NoUnitP, FlushCommands);
			if (target.StoreOrder(savedOrder) == false) {
				delete savedOrder;
				savedOrder = NULL;
			}
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
	if (attacker && target.CanMove() && target.CurrentAction() == UnitActionStill) {
		Vec2i pos = target.tilePos - attacker->tilePos;
		int d = isqrt(pos.x * pos.x + pos.y * pos.y);

		if (!d) {
			d = 1;
		}
		pos.x = target.tilePos.x + (pos.x * 5) / d + (SyncRand() & 3);
		if (pos.x < 0) {
			pos.x = 0;
		} else if (pos.x >= Map.Info.MapWidth) {
			pos.x = Map.Info.MapWidth - 1;
		}
		pos.y = target.tilePos.y + (pos.y * 5) / d + (SyncRand() & 3);
		if (pos.y < 0) {
			pos.y = 0;
		} else if (pos.y >= Map.Info.MapHeight) {
			pos.y = Map.Info.MapHeight - 1;
		}
		CommandStopUnit(target);
		CommandMove(target, pos, 0);
	}
}

/*----------------------------------------------------------------------------
--  Conflicts
----------------------------------------------------------------------------*/

/**
**  Returns the map distance between two points with unit type.
**
**  @param pos1  map tile position.
**  @param type  Unit type to take into account.
**  @param pos2  map tile position.
**
**  @return      The distance between in tiles.
*/
int MapDistanceToType(const Vec2i &pos1, const CUnitType &type, const Vec2i &pos2)
{
	int dx;
	int dy;

	if (pos1.x <= pos2.x) {
		dx = pos2.x - pos1.x;
	} else {
		dx = std::max<int>(0, pos1.x - pos2.x - type.TileWidth + 1);
	}
	if (pos1.y <= pos2.y) {
		dy = pos2.y - pos1.y;
	} else {
		dy = std::max<int>(0, pos1.y - pos2.y - type.TileHeight + 1);
	}
	return isqrt(dy * dy + dx * dx);
}

/**
**  Returns the map distance between two points with unit type.
**
**  @param src  src unittype
**  @param pos1 map tile position of src (upperleft).
**  @param dst  Unit type to take into account.
**  @param pos2 map tile position of dst.
**
**  @return     The distance between the types.
*/
int MapDistanceBetweenTypes(const CUnitType &src, const Vec2i &pos1, const CUnitType &dst, const Vec2i &pos2)
{
	int dx;
	int dy;

	if (pos1.x + src.TileWidth <= pos2.x) {
		dx = std::max<int>(0, pos2.x - pos1.x - src.TileWidth + 1);
	} else {
		dx = std::max<int>(0, pos1.x - pos2.x - dst.TileWidth + 1);
	}
	if (pos1.y + src.TileHeight <= pos2.y) {
		dy = pos2.y - pos1.y - src.TileHeight + 1;
	} else {
		dy = std::max<int>(0, pos1.y - pos2.y - dst.TileHeight + 1);
	}
	return isqrt(dy * dy + dx * dx);
}

/**
**  Compute the distance from the view point to a given point.
**
**  @param pos  map tile position.
**
**  @todo FIXME: is it the correct place to put this function in?
*/
int ViewPointDistance(const Vec2i &pos)
{
	const CViewport &vp = *UI.SelectedViewport;
	const Vec2i middle = {vp.MapX + vp.MapWidth / 2, vp.MapY + vp.MapHeight / 2};

	return MapDistance(middle, pos);
}

/**
**  Compute the distance from the view point to a given unit.
**
**  @param dest  Distance to this unit.
**
**  @todo FIXME: is it the correct place to put this function in?
*/
int ViewPointDistanceToUnit(const CUnit &dest)
{
	const CViewport &vp = *UI.SelectedViewport;
	return dest.MapDistanceTo(vp.MapX + vp.MapWidth / 2, vp.MapY + vp.MapHeight / 2);
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
int CanTransport(const CUnit &transporter, const CUnit &unit)
{
	if (!transporter.Type->CanTransport()) {
		return 0;
	}
	if (transporter.CurrentAction() == UnitActionBuilt) { // Under construction
		return 0;
	}
	if (&transporter == &unit) { // Cannot transporter itself.
		return 0;
	}
	if (transporter.BoardCount >= transporter.Type->MaxOnBoard) { // full
		return 0;
	}
	// FIXME: remove UnitTypeLand requirement
	if (unit.Type->UnitType != UnitTypeLand) {
		return 0;
	}
	// Can transport only allied unit.
	// FIXME : should be parametrable.
	if (!transporter.IsTeamed(unit)) {
		return 0;
	}
	for (unsigned int i = 0; i < UnitTypeVar.GetNumberBoolFlag(); i++) {
		if (transporter.Type->BoolFlag[i].CanTransport != CONDITION_TRUE) {
			if ((transporter.Type->BoolFlag[i].CanTransport == CONDITION_ONLY) ^
					unit.Type->BoolFlag[i].value) {
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
bool CUnit::IsEnemy(const CPlayer &player) const
{
	return (this->Player->Enemy & (1 << player.Index)) != 0;
}

/**
**  Check if the unit is an enemy
**
**  @param x  Unit to check
*/
bool CUnit::IsEnemy(const CUnit &unit) const
{
	return IsEnemy(*unit.Player);
}

/**
**  Check if the player is an ally
**
**  @param x  Player to check
*/
bool CUnit::IsAllied(const CPlayer &player) const
{
	return (this->Player->Allied & (1 << player.Index)) != 0;
}

/**
**  Check if the unit is an ally
**
**  @param x  Unit to check
*/
bool CUnit::IsAllied(const CUnit &unit) const
{
	return IsAllied(*unit.Player);
}

/**
**  Check if unit shares vision with the player
**
**  @param x  Player to check
*/
bool CUnit::IsSharedVision(const CPlayer &player) const
{
	return (this->Player->SharedVision & (1 << player.Index)) != 0;
}

/**
**  Check if the unit shares vision with the unit
**
**  @param x  Unit to check
*/
bool CUnit::IsSharedVision(const CUnit &unit) const
{
	return IsSharedVision(*unit.Player);
}

/**
**  Check if both players share vision
**
**  @param x  Player to check
*/
bool CUnit::IsBothSharedVision(const CPlayer &player) const
{
	return (this->Player->SharedVision & (1 << player.Index)) != 0 &&
		(player.SharedVision & (1 << this->Player->Index)) != 0;
}

/**
**  Check if both units share vision
**
**  @param x  Unit to check
*/
bool CUnit::IsBothSharedVision(const CUnit &unit) const
{
	return IsBothSharedVision(*unit.Player);
}

/**
**  Check if the player is on the same team
**
**  @param x  Player to check
*/
bool CUnit::IsTeamed(const CPlayer &player) const
{
	return (this->Player->Team == player.Team);
}

/**
**  Check if the unit is on the same team
**
**  @param x  Unit to check
*/
bool CUnit::IsTeamed(const CUnit &unit) const
{
	return this->IsTeamed(*unit.Player);
}

/**
**  Check if the unit is unusable (for attacking...)
**  @todo look if correct used (UnitActionBuilt is no problem if attacked)?
*/
bool CUnit::IsUnusable(bool ignore_built_state) const
{
	return (!IsAliveOnMap() || (!ignore_built_state && CurrentAction() == UnitActionBuilt));
}


/*----------------------------------------------------------------------------
--  Initialize/Cleanup
----------------------------------------------------------------------------*/

/**
**  Initialize unit module.
*/
void InitUnits()
{
	if (!SaveGameLoading) {
		NumUnits = 0;
		UnitManager.Init();
	}
}

/**
**  Clean up unit module.
*/
void CleanUnits()
{
	//
	//  Free memory for all units in unit table.
	//
	while (NumUnits) {
		int count = NumUnits;
		do {
			CUnit *unit = Units[count - 1];

			if (unit == NULL) {
				continue;
			}
			if (!unit->Destroyed) {
				if (unit->CurrentAction() == UnitActionResource) {
					ResourceInfo *resinfo = unit->Type->ResInfo[unit->CurrentResource];
					if (resinfo && !resinfo->TerrainHarvester) {
						CUnit *mine = unit->CurrentOrder()->Arg1.Resource.Mine;
						if (mine && !mine->Destroyed) {
							unit->DeAssignWorkerFromMine(*mine);
							mine->RefsDecrease();
							unit->CurrentOrder()->Arg1.Resource.Mine = NULL;
						}
					}
				}
				unit->CurrentOrder()->ClearGoal();
				if (!unit->Removed) {
					unit->Remove(NULL);
				}
				UnitClearOrders(*unit);
			}
			unit->Release(true);
		} while(--count);
	}
	NumUnits = 0;

	UnitManager.Init();

	FancyBuildings = false;
	HelpMeLastCycle = 0;
}

//@}
