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

#include "stratagus.h"

#include "unit.h"

#include "actions.h"
#include "ai.h"
#include "animation.h"
#include "commands.h"
#include "construct.h"
#include "game.h"
#include "editor.h"
#include "interface.h"
#include "luacallback.h"
#include "map.h"
#include "missile.h"
#include "network.h"
#include "pathfinder.h"
#include "player.h"
#include "script.h"
#include "sound.h"
#include "sound_server.h"
#include "spells.h"
#include "tileset.h"
#include "translate.h"
#include "ui.h"
#include "unit_find.h"
#include "unit_manager.h"
#include "unitsound.h"
#include "unittype.h"
#include "video.h"

#include <math.h>


/*----------------------------------------------------------------------------
-- Documentation
----------------------------------------------------------------------------*/

/**
**  @class CUnit unit.h
**
**  \#include "unit.h"
**
**  Everything belonging to a unit. FIXME: rearrange for less memory.
**
**  This class contains all information about a unit in game.
**  A unit could be anything: a man, a vehicle, a ship, or a building.
**  Currently only a tile, a unit, or a missile could be placed on the map.
**
**  The unit structure members:
**
**  CUnit::Refs
**
**  The reference counter of the unit. If the pointer to the unit
**  is stored the counter must be incremented and if this reference
**  is destroyed the counter must be decremented. Alternative it
**  would be possible to implement a garbage collector for this.
**
**  CUnit::Slot
**
**  This is the unique slot number. It is not possible that two
**  units have the same slot number at the same time. The slot
**  numbers are reused.
**  This field could be accessed by the macro UnitNumber(Unit *).
**
**  CUnit::UnitSlot
**
**  This is the index into #Units[], where the unit pointer is
**  stored.  #Units[] is a table of all units currently active in
**  game. This pointer is only needed to speed up, the remove of
**  the unit pointer from #Units[], it didn't must be searched in
**  the table.
**
**  CUnit::PlayerSlot
**
**  The index into Player::Units[], where the unit pointer is
**  stored. Player::Units[] is a table of all units currently
**  belonging to a player. This pointer is only needed to speed
**  up, the remove of the unit pointer from Player::Units[].
**
**  CUnit::Container
**
**  Pointer to the unit containing it, or NULL if the unit is
**  free. This points to the transporter for units on board, or to
**  the building for peasants inside(when they are mining).
**
**  CUnit::UnitInside
**
**  Pointer to the last unit added inside. Order doesn't really
**  matter. All units inside are kept in a circular linked list.
**  This is NULL if there are no units inside. Multiple levels
**  of inclusion are allowed, though not very usefull right now
**
**  CUnit::NextContained, CUnit::PrevContained
**
**  The next and previous element in the curent container. Bogus
**  values allowed for units not contained.
**
**  CUnit::InsideCount
**
**  The number of units inside the container.
**
**  CUnit::BoardCount
**
**  The number of units transported inside the container. This
**  does not include for instance stuff like harvesters returning
**  cargo.
**
**  CUnit::tilePos
**
**  The tile map coordinates of the unit.
**  0,0 is the upper left on the map.
**
**  CUnit::Type
**
**  Pointer to the unit-type (::UnitType). The unit-type contains
**  all informations that all units of the same type shares.
**  (Animations, Name, Stats, ...)
**
**  CUnit::SeenType
**  Pointer to the unit-type that this unit was, when last seen.
**  Currently only used by buildings.
**
**  CUnit::Player
**
**  Pointer to the owner of this unit (::Player). An unit could
**  only be owned by one player.
**
**  CUnit::Stats
**
**  Pointer to the current status (::UnitStats) of a unit. The
**  units of the same player and the same type could share the same
**  stats. The status contains all values which could be different
**  for each player. This f.e. the upgradeable abilities of an
**  unit.  (CUnit::Stats::SightRange, CUnit::Stats::Armor,
**  CUnit::Stats::HitPoints, ...)
**
**  CUnit::CurrentSightRange
**
**  Current sight range of a unit, this changes when a unit enters
**  a transporter or building or exits one of these.
**
**  CUnit::Colors
**
**  Player colors of the unit. Contains the hardware dependent
**  pixel values for the player colors (palette index #208-#211).
**  Setup from the global palette. This is a pointer.
**  @note Index #208-#211 are various SHADES of the team color
**  (#208 is brightest shade, #211 is darkest shade) .... these
**  numbers are NOT red=#208, blue=#209, etc
**
**  CUnit::IX CUnit::IY
**
**  Coordinate displacement in pixels or coordinates inside a tile.
**  Currently only !=0, if the unit is moving from one tile to
**  another (0-32 and for ships/flyers 0-64).
**
**  CUnit::Frame
**
**  Current graphic image of the animation sequence. The high bit
**  (128) is used to flip this image horizontal (x direction).
**  This also limits the number of different frames/image to 126.
**
**  CUnit::SeenFrame
**
**  Graphic image (see CUnit::Frame) what the player on this
**  computer has last seen. If UnitNotSeen the player haven't seen
**  this unit yet.
**
**  CUnit::Direction
**
**  Contains the binary angle (0-255) in which the direction the
**  unit looks. 0, 32, 64, 128, 160, 192, 224, 256 corresponds to
**  0�, 45�, 90�, 135�, 180�, 225�, 270�, 315�, 360� or north,
**  north-east, east, south-east, south, south-west, west,
**  north-west, north. Currently only 8 directions are used, this
**  is more for the future.
**
**  CUnit::Attacked
**
**  Last cycle the unit was attacked. 0 means never.
**
**  CUnit::Burning
**
**  If Burning is non-zero, the unit is burning.
**
**  CUnit::VisCount[PlayerMax]
**
**              Used to keep track of visible units on the map, it counts the
**              Number of seen tiles for each player. This is only modified
**              in UnitsMarkSeen and UnitsUnmarkSeen, from fow.
**              We keep track of visilibty for each player, and combine with
**              Shared vision ONLY when querying and such.
**
**  CUnit::SeenByPlayer
**
**              This is a bitmask of 1 and 0 values. SeenByPlayer & (1<<p) is 0
**              If p never saw the unit and 1 if it did. This is important for
**              keeping track of dead units under fog. We only keep track of units
**              that are visible under fog with this.
**
**  CUnit::Destroyed
**
** @todo docu.
**  If you need more informations, please send me an email or write it self.
**
**  CUnit::Removed
**
**  This flag means the unit is not active on map. This flag
**  have workers set if they are inside a building, units that are
**  on board of a transporter.
**
**  CUnit::Selected
**
**  Unit is selected. (So you can give it orders)
**
**  CUnit::Constructed
**  Set when a building is under construction, and still using the
**  generic building animation.
**
**  CUnit::SeenConstructed
**  Last seen state of construction.  Used to draw correct building
**  frame. See CUnit::Constructed for more information.
**
**  CUnit::SeenState
**  The Seen State of the building.
**  01 The building in being built when last seen.
**  10 The building was been upgraded when last seen.
**
**  CUnit::Boarded
**
**  This is 1 if the unit is on board a transporter.
**
**
**  CUnit::XP
**
**  Number of XP of the unit.
**
**  CUnit::Kills
**
**  How many units have been killed by the unit.
**
**  CUnit::GroupId
**
**  Number of the group to that the unit belongs. This is the main
**  group showed on map, a unit can belong to many groups.
**
**  CUnit::LastGroup
**
**  Automatic group number, to reselect the same units. When the
**  user selects more than one unit all units is given the next
**  same number. (Used for ALT-CLICK)
**
**  CUnit::Value
**
**  This values hold the amount of resources in a resource or in
**  in a harvester.
**  @todo continue documentation
**
**  CUnit::Wait
**
**  The unit is forced too wait for that many cycles. Be carefull,
**  setting this to 0 will lock the unit.
**
**  CUnit::State
**
**  Animation state, currently position in the animation script.
**  0 if an animation has just started, it should only be changed
**  inside of actions.
**
**  CUnit::Reset
**
**  @todo continue documentation
**
**  CUnit::Blink
**
**
**  CUnit::Moving
**
**
**  CUnit::RescuedFrom
**
**  Pointer to the original owner of a unit. It will be NULL if
**  the unit was not rescued.
**
**  CUnit::Orders
**
**  Contains all orders of the unit. Slot 0 is always used.
**
**  CUnit::SavedOrder
**
**  This order is executed, if the current order is finished.
**  This is used for attacking units, to return to the old
**  place or for patrolling units to return to patrol after
**  killing some enemies. Any new order given to the unit,
**  clears this saved order.
**
**  CUnit::NewOrder
**
**  This field is only used by buildings and this order is
**  assigned to any by this building new trained unit.
**  This is can be used to set the exit or gathering point of a
**  building.
**
**  CUnit::Goal
**
**  Generic goal pointer. Used by teleporters to point to circle of power.
**
**
** @todo continue documentation
**
*/

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

bool EnableTrainingQueue;                 /// Config: training queues enabled
bool EnableBuildingCapture;               /// Config: capture buildings enabled
bool RevealAttacker;                      /// Config: reveal attacker enabled
int ResourcesMultiBuildersMultiplier = 0; /// Config: spend resources for building with multiple workers

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
	ReleaseCycle = 0;
	PlayerSlot = static_cast<size_t>(-1);
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

	pathFinderData = new PathFinderData;
	pathFinderData->input.SetUnit(*this);

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
	Threshold = 0;
	GroupId = 0;
	LastGroup = 0;
	ResourcesHeld = 0;
	Wait = 0;
	Blink = 0;
	Moving = 0;
	ReCast = 0;
	CacheLock = 0;
	memset(&Anim, 0, sizeof(Anim));
	CurrentResource = 0;
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
	if (Type == NULL) {
		DebugPrint("unit already free\n");
		return;
	}
	Assert(Orders.size() == 1);
	// Must be removed before here
	Assert(Removed);

	// First release, remove from lists/tables.
	if (!Destroyed) {
		DebugPrint("%d: First release %d\n" _C_ Player->Index _C_ UnitNumber(*this));

		// Are more references remaining?
		Destroyed = 1; // mark as destroyed

		if (Container && !final) {
			if (Boarded) {
				Container->BoardCount--;
			}
			MapUnmarkUnitSight(*this);
			RemoveUnitFromContainer(*this);
		}

		while (Resource.Workers) {
			Resource.Workers->DeAssignWorkerFromMine(*this);
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

	Type = NULL;

	delete pathFinderData;
	delete[] AutoCastSpell;
	delete[] Variable;
	for (std::vector<COrder *>::iterator order = Orders.begin(); order != Orders.end(); ++order) {
		delete *order;
	}
	Orders.clear();

	// Remove the unit from the global units table.
	UnitManager.ReleaseUnit(this);
}

unsigned int CUnit::CurrentAction() const
{
	return (CurrentOrder()->Action);
}

void CUnit::ClearAction()
{
	Orders[0]->Finished = true;

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
void CUnit::Init(const CUnitType &type)
{
	//  Set refs to 1. This is the "I am alive ref", lost in ReleaseUnit.
	Refs = 1;

	//  Build all unit table
	UnitManager.Add(this);

	//  Initialise unit structure (must be zero filled!)
	Type = &type;

	Seen.Frame = UnitNotSeen; // Unit isn't yet seen

	Frame = type.StillFrame;

	if (UnitTypeVar.GetNumberVariable()) {
		Assert(!Variable);
		const unsigned int size = UnitTypeVar.GetNumberVariable();
		Variable = new CVariable[size];
		std::copy(type.DefaultStat.Variables, type.DefaultStat.Variables + size, Variable);
	} else {
		Variable = NULL;
	}

	// Set a heading for the unit if it Handles Directions
	// Don't set a building heading, as only 1 construction direction
	//   is allowed.
	if (type.NumDirections > 1 && type.NoRandomPlacing == false && type.Sprite && !type.Building) {
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
	COrder *savedOrder = this->SavedOrder;

	if (savedOrder == NULL) {
		return false;
	}

	if (savedOrder->IsValid() == false) {
		delete savedOrder;
		this->SavedOrder = NULL;
		return false;
	}

	// Cannot delete this->Orders[0] since it is generally that order
	// which call this method.
	this->Orders[0]->Finished = true;

	//copy
	this->Orders.insert(this->Orders.begin() + 1, savedOrder);

	this->SavedOrder = NULL;
	return true;
}

/**
**  Check if we can store this order
**
**  @return      True if the order could be saved
*/
bool CUnit::CanStoreOrder(COrder *order)
{
	Assert(order);

	if (order && order->Finished == true && order->IsValid() == false) {
		return false;
	}
	if (this->SavedOrder != NULL) {
		return false;
	}
	return true;
}

/**
**  Assigns a unit to a player, adjusting buildings, food and totals
**
**  @param player  player which have the unit.
*/
void CUnit::AssignToPlayer(CPlayer &player)
{
	const CUnitType &type = *Type;

	// Build player unit table
	if (!type.Vanishes && CurrentAction() != UnitActionDie) {
		player.AddUnit(*this);
		if (!SaveGameLoading) {
			// If unit is dieing, it's already been lost by all players
			// don't count again
			if (type.Building) {
				// FIXME: support more races
				if (!type.Wall && &type != UnitTypeOrcWall && &type != UnitTypeHumanWall) {
					player.TotalBuildings++;
				}
			} else {
				player.TotalUnits++;
			}
		}
		player.UnitTypesCount[type.Slot]++;
		player.Demand += type.Demand; // food needed
	}

	// Don't Add the building if it's dieing, used to load a save game
	if (type.Building && CurrentAction() != UnitActionDie) {
		// FIXME: support more races
		if (!type.Wall && &type != UnitTypeOrcWall && &type != UnitTypeHumanWall) {
			player.NumBuildings++;
		}
	}
	Player = &player;
	Stats = &type.Stats[Player->Index];
	Colors = &player.UnitColors;
	if (!SaveGameLoading) {
		if (UnitTypeVar.GetNumberVariable()) {
			Assert(Variable);
			Assert(Stats->Variables);
			memcpy(Variable, Stats->Variables, UnitTypeVar.GetNumberVariable() * sizeof(*Variable));
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
CUnit *MakeUnit(const CUnitType &type, CPlayer *player)
{
	CUnit *unit = UnitManager.AllocUnit();
	if (unit == NULL) {
		return NULL;
	}
	unit->Init(type);
	// Only Assign if a Player was specified
	if (player) {
		unit->AssignToPlayer(*player);
	}

	if (type.Building) {
		//
		//  fancy buildings: mirror buildings (but shadows not correct)
		//
		if (FancyBuildings && unit->Type->NoRandomPlacing == false && unit->Rs > 50) {
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
	CUnit *container = GetFirstContainer(unit);// First container of the unit.
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
	Assert(unit.Type);

	CUnit *container = GetFirstContainer(unit);
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
#if 0 // which is the better ? caller check ?
	if (SaveGameLoading) {
		return ;
	}
#else
	Assert(!SaveGameLoading);
#endif
	// FIXME : these values must be configurable.
	if (unit.Constructed) { // Units under construction have no sight range.
		unit.CurrentSightRange = 1;
	} else if (!unit.Container) { // proper value.
		unit.CurrentSightRange = unit.Stats->Variables[SIGHTRANGE_INDEX].Max;
	} else { // value of it container.
		unit.CurrentSightRange = unit.Container->CurrentSightRange;
	}

	CUnit *unit_inside = unit.UnitInside;
	for (int i = unit.InsideCount; i--; unit_inside = unit_inside->NextContained) {
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
	const unsigned int flags = unit.Type->FieldFlags;
	int h = unit.Type->TileHeight;          // Tile height of the unit.
	const int width = unit.Type->TileWidth; // Tile width of the unit.
	unsigned int index = unit.Offset;

	if (unit.Type->Vanishes) {
		return ;
	}
	do {
		CMapField *mf = Map.Field(index);
		int w = width;
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

	void operator()(CUnit *const unit) const {
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
		} while (--w);
		index += Map.Info.MapWidth;
	} while (--h);
}

/**
**  Add unit to a container. It only updates linked list stuff.
**
**  @param host  Pointer to container.
*/
void CUnit::AddInContainer(CUnit &host)
{
	Assert(Container == NULL);
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
	CUnit *host = unit.Container; // transporter which contain unit.
	Assert(unit.Container);
	Assert(unit.Container->InsideCount > 0);

	host->InsideCount--;
	unit.NextContained->PrevContained = unit.PrevContained;
	unit.PrevContained->NextContained = unit.NextContained;
	if (host->InsideCount == 0) {
		host->UnitInside = NULL;
	} else {
		if (host->UnitInside == &unit) {
			host->UnitInside = unit.NextContained;
		}
	}
	unit.Container = NULL;
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
CUnit *MakeUnitAndPlace(const Vec2i &pos, const CUnitType &type, CPlayer *player)
{
	CUnit *unit = MakeUnit(type, player);

	if (unit != NULL) {
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
		DebugPrint("unit '%s(%d)' already removed\n" _C_ Type->Ident.c_str() _C_ UnitNumber(*this));
		return;
	}
	Map.Remove(*this);
	MapUnmarkUnitSight(*this);
	UnmarkUnitFieldFlags(*this);
	if (host) {
		AddInContainer(*host);
		UpdateUnitSightRange(*this);
		UnitInXY(*this, host->tilePos);
		MapMarkUnitSight(*this);
	}

	Removed = 1;

	// Correct surrounding walls directions
	if (this->Type->Wall) {
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
void UnitLost(CUnit &unit)
{
	CPlayer &player = *unit.Player;

	Assert(&player);  // Next code didn't support no player!

	//  Call back to AI, for killed or lost units.
	if (Editor.Running == EditorNotRunning) {
		if (player.AiEnabled) {
			AiUnitKilled(unit);
		} else {
			//  Remove unit from its groups
			if (unit.GroupId) {
				RemoveUnitFromGroups(unit);
			}
		}
	}

	//  Remove the unit from the player's units table.

	const CUnitType &type = *unit.Type;
	if (!type.Vanishes) {
		player.RemoveUnit(unit);

		if (type.Building) {
			// FIXME: support more races
			if (!type.Wall && &type != UnitTypeOrcWall && &type != UnitTypeHumanWall) {
				player.NumBuildings--;
			}
		}
		if (unit.CurrentAction() != UnitActionBuilt) {
			player.UnitTypesCount[type.Slot]--;
		}
	}

	//  Handle unit demand. (Currently only food supported.)
	player.Demand -= type.Demand;

	//  Update information.
	if (unit.CurrentAction() != UnitActionBuilt) {
		player.Supply -= type.Supply;
		// Decrease resource limit
		for (int i = 0; i < MaxCosts; ++i) {
			if (player.MaxResources[i] != -1 && type.Stats[player.Index].Storing[i]) {
				const int newMaxValue = player.MaxResources[i] - type.Stats[player.Index].Storing[i];

				player.MaxResources[i] = std::max(0, newMaxValue);
				player.SetResource(i, player.StoredResources[i], STORE_BUILDING);
			}
		}
		//  Handle income improvements, look if a player loses a building
		//  which have given him a better income, find the next best
		//  income.
		for (int i = 1; i < MaxCosts; ++i) {
			if (player.Incomes[i] && type.ImproveIncomes[i] == player.Incomes[i]) {
				int m = DefaultIncomes[i];

				for (int j = 0; j < player.GetUnitCount(); ++j) {
					m = std::max(m, player.GetUnit(j).Type->ImproveIncomes[i]);
				}
				player.Incomes[i] = m;
			}
		}
	}

	//  Handle order cancels.
	unit.CurrentOrder()->Cancel(unit);

	DebugPrint("%d: Lost %s(%d)\n" _C_ player.Index _C_ type.Ident.c_str() _C_ UnitNumber(unit));

	// Destroy resource-platform, must re-make resource patch.
	CBuildRestrictionOnTop *b = OnTopDetails(unit, NULL);
	if (b != NULL) {
		if (b->ReplaceOnDie && (type.GivesResource && unit.ResourcesHeld != 0)) {
			CUnit *temp = MakeUnitAndPlace(unit.tilePos, *b->Parent, &Players[PlayerNumNeutral]);
			if (temp == NULL) {
				DebugPrint("Unable to allocate Unit");
			} else {
				temp->ResourcesHeld = unit.ResourcesHeld;
			}
		}
	}
}

/**
**  Removes all orders from a unit.
**
**  @param unit  The unit that will have all its orders cleared
*/
void UnitClearOrders(CUnit &unit)
{
	for (size_t i = 0; i != unit.Orders.size(); ++i) {
		delete unit.Orders[i];
	}
	unit.Orders.clear();
	unit.Orders.push_back(COrder::NewActionStill());
}

/**
**  Update for new unit. Food and income ...
**
**  @param unit     New unit pointer.
**  @param upgrade  True unit was upgraded.
*/
void UpdateForNewUnit(const CUnit &unit, int upgrade)
{
	const CUnitType &type = *unit.Type;
	CPlayer &player = *unit.Player;

	// Handle unit supply and max resources.
	// Note an upgraded unit can't give more supply.
	if (!upgrade) {
		player.Supply += type.Supply;
		for (int i = 0; i < MaxCosts; ++i) {
			if (player.MaxResources[i] != -1 && type.Stats[player.Index].Storing[i]) {
				player.MaxResources[i] += type.Stats[player.Index].Storing[i];
			}
		}
	}

	// Update resources
	for (int u = 1; u < MaxCosts; ++u) {
		player.Incomes[u] = std::max(player.Incomes[u], type.ImproveIncomes[u]);
	}
}

/**
**  Find nearest point of unit.
**
**  @param unit  Pointer to unit.
**  @param pos   tile map postion.
**  @param dpos  Out: nearest point tile map postion to (tx,ty).
*/
void NearestOfUnit(const CUnit &unit, const Vec2i &pos, Vec2i *dpos)
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
	unit.Seen.tilePos = unit.tilePos;
	unit.Seen.IY = unit.IY;
	unit.Seen.IX = unit.IX;
	unit.Seen.Frame = unit.Frame;
	unit.Seen.Type = unit.Type;
	unit.Seen.Constructed = unit.Constructed;

	unit.CurrentOrder()->FillSeenValues(unit);
}

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
	} configs[] = {{Vec2i(0, -1), W_NORTH}, {Vec2i(1, 0), W_EAST},
		{Vec2i(0, 1), W_SOUTH}, {Vec2i(-1, 0), W_WEST}
	};
	int flags = 0;

	for (int i = 0; i != sizeof(configs) / sizeof(*configs); ++i) {
		const Vec2i pos = unit.tilePos + configs[i].offset;
		const int dirFlag = configs[i].dirFlag;

		if (Map.Info.IsPointOnMap(pos) == false) {
			flags |= dirFlag;
		} else {
			const CUnitCache &unitCache = Map.Field(pos)->UnitCache;
			const CUnit *neighboor = unitCache.find(HasSamePlayerAndTypeAs(unit));

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

	const Vec2i offset[] = {Vec2i(1, 0), Vec2i(-1, 0), Vec2i(0, 1), Vec2i(0, -1)};

	for (unsigned int i = 0; i < sizeof(offset) / sizeof(*offset); ++i) {
		const Vec2i pos = unit.tilePos + offset[i];

		if (Map.Info.IsPointOnMap(pos) == false) {
			continue;
		}
		CUnitCache &unitCache = Map.Field(pos)->UnitCache;
		CUnit *neighboor = unitCache.find(HasSamePlayerAndTypeAs(unit));

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
		if ((player.Type == PlayerPerson) && (!(unit.Seen.Destroyed & (1 << player.Index)))) {
			unit.RefsDecrease();
		}
	} else {
		unit.Seen.ByPlayer |= (1 << (player.Index));
	}
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
						if (mf->playerInfo.VisCloak[p]) {
							newv++;
						}
					} else {
						if (mf->playerInfo.IsVisible(Players[p])) {
							newv++;
						}
					}
					++mf;
				} while (--x);
				index += Map.Info.MapWidth;
			} while (--y);
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
	// Invisible units.
	if (IsInvisibile(*ThisPlayer)) {
		return false;
	}
	if (IsVisible(*ThisPlayer) || ReplayRevealMap || IsVisibleOnRadar(*ThisPlayer)) {
		return IsAliveOnMap();
	} else {
		return Type->VisibleUnderFog && Seen.State != 3
			   && (Seen.ByPlayer & (1 << ThisPlayer->Index))
			   && !(Seen.Destroyed & (1 << ThisPlayer->Index));
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
bool CUnit::IsVisibleInViewport(const CViewport &vp) const
{
	// Check if the graphic is inside the viewport.
	int x = tilePos.x * PixelTileSize.x + IX - (Type->Width - Type->TileWidth * PixelTileSize.x) / 2 + Type->OffsetX;
	int y = tilePos.y * PixelTileSize.y + IY - (Type->Height - Type->TileHeight * PixelTileSize.y) / 2 + Type->OffsetY;
	const PixelSize vpSize = vp.GetPixelSize();
	const PixelPos vpTopLeftMapPos = Map.TilePosToMapPixelPos_TopLeft(vp.MapPos) + vp.Offset;
	const PixelPos vpBottomRightMapPos = vpTopLeftMapPos + vpSize;

	if (x + Type->Width < vpTopLeftMapPos.x || x > vpBottomRightMapPos.x
		|| y + Type->Height < vpTopLeftMapPos.y || y > vpBottomRightMapPos.y) {
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

	if (Type->Building) {
		if (!Type->Wall) {
			newplayer.TotalBuildings++;
		}
	} else {
		newplayer.TotalUnits++;
	}

	MapUnmarkUnitSight(*this);
	newplayer.AddUnit(*this);
	Stats = &Type->Stats[newplayer.Index];
	UpdateUnitSightRange(*this);
	MapMarkUnitSight(*this);

	//  Must change food/gold and other.
	if (Type->GivesResource) {
		DebugPrint("Resource transfer not supported\n");
	}
	newplayer.Demand += Type->Demand;
	newplayer.Supply += Type->Supply;
	// Increase resource limit
	for (int i = 0; i < MaxCosts; ++i) {
		if (newplayer.MaxResources[i] != -1 && Type->Stats[newplayer.Index].Storing[i]) {
			newplayer.MaxResources[i] += Type->Stats[newplayer.Index].Storing[i];
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
	for (CUnit *it = mine.Resource.Workers; it; it = it->NextWorker) {
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
#if 0
	DebugPrint("%d: Worker [%d] is adding into %s [%d] on %d pos\n"
			   _C_ this->Player->Index _C_ this->Slot
			   _C_ mine.Type->Name.c_str()
			   _C_ mine.Slot
			   _C_ mine.Data.Resource.Assigned);
#endif
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
#if 0
	DebugPrint("%d: Worker [%d] is removing from %s [%d] left %d units assigned\n"
			   _C_ this->Player->Index _C_ this->Slot
			   _C_ mine.Type->Name.c_str()
			   _C_ mine.Slot
			   _C_ mine.CurrentOrder()->Data.Resource.Assigned);
#endif
	for (int i = 0; NULL != worker; worker = worker->NextWorker, ++i) {
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
	if (&oldplayer == &newplayer) {
		return ;
	}

	for (int i = 0; i != oldplayer.GetUnitCount(); ++i) {
		CUnit &unit = oldplayer.GetUnit(i);

		unit.Blink = 5;
		unit.RescuedFrom = &oldplayer;
	}
	// ChangeOwner remove unit from the player: so change the array.
	while (oldplayer.GetUnitCount() != 0) {
		CUnit &unit = oldplayer.GetUnit(0);

		unit.ChangeOwner(newplayer);
	}
}

/**
**  Rescue units.
**
**  Look through all rescueable players, if they could be rescued.
*/
void RescueUnits()
{
	if (NoRescueCheck) {  // all possible units are rescued
		return;
	}
	NoRescueCheck = true;

	//  Look if player could be rescued.
	for (CPlayer *p = Players; p < Players + NumPlayers; ++p) {
		if (p->Type != PlayerRescuePassive && p->Type != PlayerRescueActive) {
			continue;
		}
		if (p->GetUnitCount() != 0) {
			NoRescueCheck = false;
			// NOTE: table is changed.
			std::vector<CUnit *> table;
			table.insert(table.begin(), p->UnitBegin(), p->UnitEnd());

			const size_t l = table.size();
			for (size_t j = 0; j != l; ++j) {
				CUnit &unit = *table[j];
				// Do not rescue removed units. Units inside something are
				// rescued by ChangeUnitOwner
				if (unit.Removed) {
					continue;
				}
				std::vector<CUnit *> around;

				SelectAroundUnit(unit, 1, around);
				//  Look if ally near the unit.
				for (size_t i = 0; i != around.size(); ++i) {
					if (around[i]->Type->CanAttack && unit.IsAllied(*around[i])) {
						//  City center converts complete race
						//  NOTE: I use a trick here, centers could
						//        store gold. FIXME!!!
						if (unit.Type->CanStore[GoldCost]) {
							ChangePlayerOwner(*p, *around[i]->Player);
							break;
						}
						unit.RescuedFrom = unit.Player;
						unit.ChangeOwner(*around[i]->Player);
						unit.Blink = 5;
						PlayGameSound(GameSounds.Rescue[unit.Player->Race].Sound, MaxSampleVolume);
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
int DirectionToHeading(const Vec2i &delta)
{
	//  Check which quadrant.
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
int DirectionToHeading(const PixelDiff &delta)
{
	// code is identic for Vec2i and PixelDiff
	Vec2i delta2(delta.x, delta.y);
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
	Vec2i bestPos;
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
				const int n = SquareDistance(goalPos, pos);

				if (n < bestd) {
					bestd = n;
					bestPos = pos;
				}
			}
		}
		++addx;
		for (int i = addx; i--; ++pos.x) { // go right
			if (UnitCanBeAt(unit, pos)) {
				const int n = SquareDistance(goalPos, pos);

				if (n < bestd) {
					bestd = n;
					bestPos = pos;
				}
			}
		}
		++addy;
		for (int i = addy; i--; --pos.y) { // go up
			if (UnitCanBeAt(unit, pos)) {
				const int n = SquareDistance(goalPos, pos);

				if (n < bestd) {
					bestd = n;
					bestPos = pos;
				}
			}
		}
		++addx;
		for (int i = addx; i--; --pos.x) { // go left
			if (UnitCanBeAt(unit, pos)) {
				const int n = SquareDistance(goalPos, pos);

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
	}
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
**    Cycle through units.
**    First take highest unit.
**
**  @param x      X pixel position.
**  @param y      Y pixel position.
**
**  @return       An unit on x, y position.
*/
CUnit *UnitOnScreen(int x, int y)
{
	CUnit *candidate = NULL;
	for (CUnitManager::Iterator it = UnitManager.begin(); it != UnitManager.end(); ++it) {
		CUnit &unit = **it;
		if (!ReplayRevealMap && !unit.IsVisibleAsGoal(*ThisPlayer)) {
			continue;
		}
		const CUnitType &type = *unit.Type;

		//
		// Check if mouse is over the unit.
		//
		PixelPos unitSpritePos = unit.GetMapPixelPosCenter();
		unitSpritePos.x = unitSpritePos.x - type.BoxWidth / 2 -
						  (type.Width - type.Sprite->Width) / 2 + type.BoxOffsetX;
		unitSpritePos.y = unitSpritePos.y - type.BoxHeight / 2 -
						  (type.Height - type.Sprite->Height) / 2 + type.BoxOffsetY;
		if (x >= unitSpritePos.x && x < unitSpritePos.x + type.BoxWidth
			&& y >= unitSpritePos.y  && y < unitSpritePos.y + type.BoxHeight) {
			// Check if there are other units on this place
			candidate = &unit;
			if ((candidate == Selected[0] && NumSelected == 1)
				|| candidate->Type->IsNotSelectable) {
				continue;
			} else {
				break;
			}
		} else {
			continue;
		}
	}
	return candidate;
}

PixelPos CUnit::GetMapPixelPosTopLeft() const
{
	const PixelPos pos(tilePos.x * PixelTileSize.x + IX, tilePos.y * PixelTileSize.y + IY);
	return pos;
}

PixelPos CUnit::GetMapPixelPosCenter() const
{
	return GetMapPixelPosTopLeft() + Type->GetPixelSize() / 2;
}

/**
**  Let an unit die.
**
**  @param unit    Unit to be destroyed.
*/
void LetUnitDie(CUnit &unit)
{
	unit.Variable[HP_INDEX].Value = std::min<int>(0, unit.Variable[HP_INDEX].Value);
	unit.Moving = 0;
	unit.TTL = 0;
	unit.Anim.Unbreakable = 0;

	const CUnitType *type = unit.Type;

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
	if (unit.UnitInside && unit.Type->SaveCargo) {
		DropOutAll(unit);
	} else if (unit.UnitInside) {
		DestroyAllInside(unit);
	}

	unit.Remove(NULL);
	UnitLost(unit);
	UnitClearOrders(unit);


	// Unit has death animation.

	// Not good: UnitUpdateHeading(unit);
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

int ThreatCalculate(const CUnit &unit, const CUnit &dest)
{
	const CUnitType &type = *unit.Type;
	const CUnitType &dtype = *dest.Type;
	int cost = 0;

	// Buildings and non-aggressive units have the lowest priority
	if (dest.IsAgressive() == false) {
		if (dest.Type->CanMove() == false) {
			return INT_MAX;
		} else {
			return INT_MAX / 2;
		}
	}

	// Priority 0-255
	cost -= dtype.Priority * PRIORITY_FACTOR;
	// Remaining HP (Health) 0-65535
	cost += dest.Variable[HP_INDEX].Value * 100 / dest.Variable[HP_INDEX].Max * HEALTH_FACTOR;

	const int d = unit.MapDistanceTo(dest);

	if (d <= unit.Stats->Variables[ATTACKRANGE_INDEX].Max && d >= type.MinAttackRange) {
		cost += d * INRANGE_FACTOR;
		cost -= INRANGE_BONUS;
	} else {
		cost += d * DISTANCE_FACTOR;
	}

	for (unsigned int i = 0; i < UnitTypeVar.GetNumberBoolFlag(); i++) {
		if (type.BoolFlag[i].AiPriorityTarget != CONDITION_TRUE) {
			if ((type.BoolFlag[i].AiPriorityTarget == CONDITION_ONLY) & (dtype.BoolFlag[i].value)) {
				cost -= AIPRIORITY_BONUS;
			}
			if ((type.BoolFlag[i].AiPriorityTarget == CONDITION_FALSE) & (dtype.BoolFlag[i].value)) {
				cost += AIPRIORITY_BONUS;
			}
		}
	}

	// Unit can attack back.
	if (CanTarget(dtype, type)) {
		cost -= CANATTACK_BONUS;
	}
	return cost;
}

static void HitUnit_lastAttack(const CUnit *attacker, CUnit &target)
{
	const unsigned long lastattack = target.Attacked;

	target.Attacked = GameCycle ? GameCycle : 1;
	if (target.Type->Wall || (lastattack && GameCycle <= lastattack + 2 * CYCLES_PER_SECOND)) {
		return;
	}
	// NOTE: perhaps this should also be moved into the notify?
	if (target.Player == ThisPlayer) {
		// FIXME: Problem with load+save.

		//
		// One help cry each 2 second is enough
		// If on same area ignore it for 2 minutes.
		//
		if (HelpMeLastCycle < GameCycle) {
			if (!HelpMeLastCycle
				|| HelpMeLastCycle + CYCLES_PER_SECOND * 120 < GameCycle
				|| target.tilePos.x < HelpMeLastX - 14
				|| target.tilePos.x > HelpMeLastX + 14
				|| target.tilePos.y < HelpMeLastY - 14
				|| target.tilePos.y > HelpMeLastY + 14) {
				HelpMeLastCycle = GameCycle + CYCLES_PER_SECOND * 2;
				HelpMeLastX = target.tilePos.x;
				HelpMeLastY = target.tilePos.y;
				PlayUnitSound(target, VoiceHelpMe);
			}
		}
	}
	target.Player->Notify(NotifyRed, target.tilePos, _("%s attacked"), target.Type->Name.c_str());

	if (attacker && !target.Type->Building) {
		if (target.Player->AiEnabled) {
			AiHelpMe(attacker, target);
		}
	}
}

static bool HitUnit_IsUnitWillDie(const CUnit *attacker, const CUnit &target, int damage)
{
	return (target.Variable[HP_INDEX].Value <= damage && attacker && attacker->Type->ShieldPiercing)
		   || (target.Variable[HP_INDEX].Value <= damage - target.Variable[SHIELD_INDEX].Value)
		   || (target.Variable[HP_INDEX].Value == 0);
}

static void HitUnit_IncreaseScoreForKill(CUnit &attacker, CUnit &target)
{
	attacker.Player->Score += target.Variable[POINTS_INDEX].Value;
	if (target.Type->Building) {
		attacker.Player->TotalRazings++;
	} else {
		attacker.Player->TotalKills++;
	}
	if (UseHPForXp) {
		attacker.Variable[XP_INDEX].Max += target.Variable[HP_INDEX].Value;
	} else {
		attacker.Variable[XP_INDEX].Max += target.Variable[POINTS_INDEX].Value;
	}
	attacker.Variable[XP_INDEX].Value = attacker.Variable[XP_INDEX].Max;
	attacker.Variable[KILL_INDEX].Value++;
	attacker.Variable[KILL_INDEX].Max++;
	attacker.Variable[KILL_INDEX].Enable = 1;
}

static void HitUnit_applyDamage(CUnit *attacker, CUnit &target, int damage)
{
	if (attacker && attacker->Type->ShieldPiercing) {
		target.Variable[HP_INDEX].Value -= damage;
	} else if (target.Variable[SHIELD_INDEX].Value >= damage) {
		target.Variable[SHIELD_INDEX].Value -= damage;
	} else {
		target.Variable[HP_INDEX].Value -= damage - target.Variable[SHIELD_INDEX].Value;
		target.Variable[SHIELD_INDEX].Value = 0;
	}
	if (UseHPForXp && attacker && target.IsEnemy(*attacker)) {
		attacker->Variable[XP_INDEX].Value += damage;
		attacker->Variable[XP_INDEX].Max += damage;
	}
}

static void HitUnit_BuildingCapture(CUnit *attacker, CUnit &target, int damage)
{
	// FIXME: this is dumb. I made repairers capture. crap.
	// david: capture enemy buildings
	// Only worker types can capture.
	// Still possible to destroy building if not careful (too many attackers)
	if (EnableBuildingCapture && attacker
		&& target.Type->Building && target.Variable[HP_INDEX].Value <= damage * 3
		&& attacker->IsEnemy(target)
		&& attacker->Type->RepairRange) {
		target.ChangeOwner(*attacker->Player);
		CommandStopUnit(*attacker); // Attacker shouldn't continue attack!
	}
}

static void HitUnit_ShowDamageMissile(const CUnit &target, int damage)
{
	const PixelPos targetPixelCenter = target.GetMapPixelPosCenter();

	if ((target.IsVisibleOnMap(*ThisPlayer) || ReplayRevealMap) && !DamageMissile.empty()) {
		const MissileType *mtype = MissileTypeByIdent(DamageMissile);
		const PixelDiff offset(3, -mtype->Range);

		MakeLocalMissile(*mtype, targetPixelCenter, targetPixelCenter + offset)->Damage = -damage;
	}
}

static void HitUnit_ShowImpactMissile(const CUnit &target)
{
	const PixelPos targetPixelCenter = target.GetMapPixelPosCenter();
	const CUnitType &type = *target.Type;

	if (target.Variable[SHIELD_INDEX].Value > 0
		&& !type.Impact[ANIMATIONS_DEATHTYPES + 1].Name.empty()) { // shield impact
		MakeMissile(*type.Impact[ANIMATIONS_DEATHTYPES + 1].Missile, targetPixelCenter, targetPixelCenter);
	} else if (target.DamagedType && !type.Impact[target.DamagedType].Name.empty()) { // specific to damage type impact
		MakeMissile(*type.Impact[target.DamagedType].Missile, targetPixelCenter, targetPixelCenter);
	} else if (!type.Impact[ANIMATIONS_DEATHTYPES].Name.empty()) { // generic impact
		MakeMissile(*type.Impact[ANIMATIONS_DEATHTYPES].Missile, targetPixelCenter, targetPixelCenter);
	}
}

static void HitUnit_ChangeVariable(CUnit &target, const Missile &missile)
{
	const int var = missile.Type->ChangeVariable;

	target.Variable[var].Enable = 1;
	target.Variable[var].Value += missile.Type->ChangeAmount;
	if (target.Variable[var].Value > target.Variable[var].Max) {
		if (missile.Type->ChangeMax) {
			target.Variable[var].Max = target.Variable[var].Value;
		} else {
			target.Variable[var].Value = target.Variable[var].Max;
		}
	}
}


static void HitUnit_Burning(CUnit &target)
{
	const int f = (100 * target.Variable[HP_INDEX].Value) / target.Variable[HP_INDEX].Max;
	MissileType *fire = MissileBurningBuilding(f);

	if (fire) {
		const PixelPos targetPixelCenter = target.GetMapPixelPosCenter();
		const PixelDiff offset(0, -PixelTileSize.y);
		Missile *missile = MakeMissile(*fire, targetPixelCenter + offset, targetPixelCenter + offset);

		missile->SourceUnit = &target;
		target.Burning = 1;
	}
}

static void HitUnit_RunAway(CUnit &target, const CUnit &attacker)
{
	Vec2i pos = target.tilePos - attacker.tilePos;
	int d = isqrt(pos.x * pos.x + pos.y * pos.y);

	if (!d) {
		d = 1;
	}
	pos.x = target.tilePos.x + (pos.x * 5) / d + (SyncRand() & 3);
	pos.y = target.tilePos.y + (pos.y * 5) / d + (SyncRand() & 3);
	Map.Clamp(pos);
	CommandStopUnit(target);
	CommandMove(target, pos, 0);
}

static void HitUnit_AttackBack(CUnit &attacker, CUnit &target)
{
	const int threshold = 30;
	COrder *savedOrder = NULL;
	if (target.CanStoreOrder(target.CurrentOrder())) {
		savedOrder = target.CurrentOrder()->Clone();
	}
	CUnit *oldgoal = target.CurrentOrder()->GetGoal();
	CUnit *goal, *best = oldgoal;

	if (RevealAttacker && CanTarget(*target.Type, *attacker.Type)) {
		// Reveal Unit that is attacking
		goal = &attacker;
	} else {
		if (target.CurrentAction() == UnitActionStandGround) {
			goal = AttackUnitsInRange(target);
		} else {
			// Check for any other units in range
			goal = AttackUnitsInReactRange(target);
		}
	}

	// Calculate the best target we could attack
	if (!best || (goal && (ThreatCalculate(target, *goal) < ThreatCalculate(target, *best)))) {
		best = goal;
	}
	if (CanTarget(*target.Type, *attacker.Type)
		&& (!best || (goal != &attacker
					  && (ThreatCalculate(target, attacker) < ThreatCalculate(target, *best))))) {
		best = &attacker;
	}
	if (best && best != oldgoal && best->Player != target.Player && best->IsAllied(target) == false) {
		CommandAttack(target, best->tilePos, best, FlushCommands);
		// Set threshold value only for agressive units
		if (best->IsAgressive()) {
			target.Threshold = threshold;
		}
		if (savedOrder != NULL) {
			target.SavedOrder = savedOrder;
		}
	}
}

/**
**  Unit is hit by missile or other damage.
**
**  @param attacker    Unit that attacks.
**  @param target      Unit that is hit.
**  @param damage      How many damage to take.
**  @param missile     Which missile took the damage.
*/
void HitUnit(CUnit *attacker, CUnit &target, int damage, const Missile *missile)
{
	if (!damage) {
		// Can now happen by splash damage
		// Multiple places send x/y as damage, which may be zero
		return;
	}

	if (target.Variable[UNHOLYARMOR_INDEX].Value > 0 || target.Type->Indestructible) {
		// vladi: units with active UnholyArmour are invulnerable
		return;
	}
	if (target.Removed) {
		DebugPrint("Removed target hit\n");
		return;
	}

	Assert(damage != 0 && target.CurrentAction() != UnitActionDie && !target.Type->Vanishes);

	if (GodMode) {
		if (attacker && attacker->Player == ThisPlayer) {
			damage = target.Variable[HP_INDEX].Value;
		}
		if (target.Player == ThisPlayer) {
			damage = 0;
		}
	}
	HitUnit_lastAttack(attacker, target);
	const CUnitType *type = target.Type;
	if (attacker) {
		target.DamagedType = ExtraDeathIndex(attacker->Type->DamageType.c_str());
	}

	if (attacker && !target.Type->Wall && target.Player->AiEnabled) {
		AiHelpMe(attacker, target);
	}

	if (HitUnit_IsUnitWillDie(attacker, target, damage)) { // unit is killed or destroyed
		if (attacker) {
			//  Setting ai threshold counter to 0 so it can target other units
			attacker->Threshold = 0;
			if (target.IsEnemy(*attacker)) {
				HitUnit_IncreaseScoreForKill(*attacker, target);
			}
		}
		LetUnitDie(target);
		return;
	}

	HitUnit_applyDamage(attacker, target, damage);
	HitUnit_BuildingCapture(attacker, target, damage);
	HitUnit_ShowDamageMissile(target, damage);

	// OnHit callback
	if (type->OnHit) {
		const int slot = UnitNumber(target);

		type->OnHit->pushPreamble();
		type->OnHit->pushInteger(slot);
		type->OnHit->run();
	}

	// Increase variables
	if (missile && missile->Type->ChangeVariable != -1) {
		HitUnit_ChangeVariable(target, *missile);
	}

	HitUnit_ShowImpactMissile(target);

	if (type->Building && !target.Burning) {
		HitUnit_Burning(target);
	}

	/* Target Reaction on Hit */
	if (target.Player->AiEnabled) {
		if (target.CurrentOrder()->OnAiHitUnit(target, attacker, damage)) {
			return;
		}
	}

	if (!attacker) {
		return;
	}

	// Can't attack run away.
	if (!target.IsAgressive() && target.CanMove() && target.CurrentAction() == UnitActionStill) {
		HitUnit_RunAway(target, *attacker);
	}

	// The rest instructions is only for AI units.
	if (!target.Player->AiEnabled) {
		return;
	}

	const int threshold = 30;

	if (target.Threshold && target.CurrentOrder()->HasGoal() && target.CurrentOrder()->GetGoal() == attacker) {
		target.Threshold = threshold;
		return;
	}
	if (target.Threshold == 0 && target.IsAgressive() && target.CanMove() && !target.ReCast) {
		// Attack units in range (which or the attacker?)
		// Don't bother unit if it casting repeatable spell
		HitUnit_AttackBack(*attacker, target);
	}

	// What should we do with workers on :
	// case UnitActionRepair:
	// Drop orders and run away or return after escape?
}

/*----------------------------------------------------------------------------
--  Conflicts
----------------------------------------------------------------------------*/

/**
 **  Returns the map distance to unit.
 **
 **  @param pos   map tile position.
 **
 **  @return      The distance between in tiles.
 */
int CUnit::MapDistanceTo(const Vec2i &pos) const
{
	int dx;
	int dy;

	if (pos.x <= tilePos.x) {
		dx = tilePos.x - pos.x;
	} else {
		dx = std::max<int>(0, pos.x - tilePos.x - Type->TileWidth + 1);
	}
	if (pos.y <= tilePos.y) {
		dy = tilePos.y - pos.y;
	} else {
		dy = std::max<int>(0, pos.y - tilePos.y - Type->TileHeight + 1);
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
	const Vec2i vpSize(vp.MapWidth, vp.MapHeight);
	const Vec2i middle = vp.MapPos + vpSize / 2;

	return Distance(middle, pos);
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
	const Vec2i vpSize(vp.MapWidth, vp.MapHeight);
	const Vec2i midPos = vp.MapPos + vpSize / 2;

	return dest.MapDistanceTo(midPos);
}

/**
**  Can the source unit attack the destination unit.
**
**  @param source  Unit type pointer of the attacker.
**  @param dest    Unit type pointer of the target.
**
**  @return        0 if attacker can't target the unit, else a positive number.
*/
int CanTarget(const CUnitType &source, const CUnitType &dest)
{
	for (unsigned int i = 0; i < UnitTypeVar.GetNumberBoolFlag(); i++) {
		if (source.BoolFlag[i].CanTargetFlag != CONDITION_TRUE) {
			if ((source.BoolFlag[i].CanTargetFlag == CONDITION_ONLY) ^
				(dest.BoolFlag[i].value)) {
				return 0;
			}
		}
	}
	if (dest.UnitType == UnitTypeLand) {
		if (dest.ShoreBuilding) {
			return source.CanTarget & (CanTargetLand | CanTargetSea);
		}
		return source.CanTarget & CanTargetLand;
	}
	if (dest.UnitType == UnitTypeFly) {
		return source.CanTarget & CanTargetAir;
	}
	if (dest.UnitType == UnitTypeNaval) {
		return source.CanTarget & CanTargetSea;
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

	// Can transport only allied unit.
	// FIXME : should be parametrable.
	if (!transporter.IsTeamed(unit)) {
		return 0;
	}
	for (unsigned int i = 0; i < UnitTypeVar.GetNumberBoolFlag(); i++) {
		if (transporter.Type->BoolFlag[i].CanTransport != CONDITION_TRUE) {
			if ((transporter.Type->BoolFlag[i].CanTransport == CONDITION_ONLY) ^ unit.Type->BoolFlag[i].value) {
				return 0;
			}
		}
	}
	return 1;
}

/**
**  Check if the player is an enemy
**
**  @param player  Player to check
*/
bool CUnit::IsEnemy(const CPlayer &player) const
{
	return this->Player->IsEnemy(player);
}

/**
**  Check if the unit is an enemy
**
**  @param unit  Unit to check
*/
bool CUnit::IsEnemy(const CUnit &unit) const
{
	return IsEnemy(*unit.Player);
}

/**
**  Check if the player is an ally
**
**  @param player  Player to check
*/
bool CUnit::IsAllied(const CPlayer &player) const
{
	return this->Player->IsAllied(player);
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
	return this->Player->IsSharedVision(player);
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
	return this->Player->IsBothSharedVision(player);
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
		UnitManager.Init();
	}
}

/**
**  Clean up unit module.
*/
void CleanUnits()
{
	//  Free memory for all units in unit table.
	std::vector<CUnit *> units(UnitManager.begin(), UnitManager.end());

	for (std::vector<CUnit *>::iterator it = units.begin(); it != units.end(); ++it) {
		CUnit &unit = **it;

		if (&unit == NULL) {
			continue;
		}
		if (!unit.Destroyed) {
			if (!unit.Removed) {
				unit.Remove(NULL);
			}
			UnitClearOrders(unit);
		}
		unit.Release(true);
	}

	UnitManager.Init();

	FancyBuildings = false;
	HelpMeLastCycle = 0;
}

//@}
