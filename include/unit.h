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
/**@name unit.h - The unit headerfile. */
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

#ifndef __UNIT_H__
#define __UNIT_H__

//@{

/*----------------------------------------------------------------------------
-- Documentation
----------------------------------------------------------------------------*/

/**
**  @struct _unit_ unit.h
**
**  \#include "unit.h"
**
**  typedef struct _unit_ Unit;
**
**  Everything belonging to an unit. FIXME: rearrange for less memory.
**
**  This structure contains all informations about an unit in game.
**  An unit could be everything, a man, a vehicle, a ship or a building.
**  Currently only a tile, an unit or a missile could be placed on the map.
**
**  The unit structure members:
**
**  Unit::Refs
**
**  The reference counter of the unit. If the pointer to the unit
**  is stored the counter must be incremented and if this reference
**  is destroyed the counter must be decremented. Alternative it
**  would be possible to implement a garbage collector for this.
**
**  Unit::Slot
**
**  This is the unique slot number. It is not possible that two
**  units have the same slot number at the same time. The slot
**  numbers are reused.
**  This field could be accessed by the macro UnitNumber(Unit*).
**  Maximal 65535 (=#MAX_UNIT_SLOTS) simultaneous units are
**  supported.
**
**  Unit::UnitSlot
**
**  This is the pointer into #Units[], where the unit pointer is
**  stored.  #Units[] is a table of all units currently active in
**  game. This pointer is only needed to speed up, the remove of
**  the unit pointer from #Units[], it didn't must be searched in
**  the table.
**
**  Unit::PlayerSlot
**
**  A pointer into Player::Units[], where the unit pointer is
**  stored. Player::Units[] is a table of all units currently
**  belonging to a player. This pointer is only needed to speed
**  up, the remove of the unit pointer from Player::Units[].
**
**  Unit::Next
**
**  A generic link pointer. This member is currently used, if an
**  unit is on the map, to link all units on the same map field
**  together. This also links corpses and stuff. Also, this is
**  used in memory management to link unused units.
**
**  Unit::Container
**
**  Pointer to the unit containing it, or NoUnitP if the unit is
**  free. This points to the transporter for units on board, or to
**  the building for peasants inside(when they are mining).
**
**  Unit::UnitInside
**
**  Pointer to the last unit added inside. Order doesn't really
**  matter. All units inside are kept in a circular linked list.
**  This is NoUnitP if there are no units inside. Multiple levels
**  of inclusion are allowed, though not very usefull right now
**
**  Unit::NextContained, Unit::PrevContained
**
**  The next and previous element in the curent container. Bogus
**  values allowed for units not contained.
**
**  Unit::InsideCount
**
**  The number of units inside the container.
**
**  Unit::BoardCount
**
**  The number of units transported inside the container. This
**  does not include for instance stuff like harvesters returning
**  cargo.
**
**  Unit::Name
**
**  Name of the unit.
**
**  Unit::X Unit::Y
**
**  The tile map coordinates of the unit. 0,0 is the upper left on
**  the map. To convert the map coordinates into pixels, they
**  must be multiplicated with the #TileSizeX and #TileSizeY.
**  To get the pixel coordinates of an unit, calculate
**  Unit::X*#TileSize+Unit::IX , Unit::Y*#TileSizeY+Unit::IY.
**
**  Unit::Type
**
**  Pointer to the unit-type (::UnitType). The unit-type contains
**  all informations that all units of the same type shares.
**  (Animations, Name, Stats, ...)
**
**  Unit::SeenType
**  Pointer to the unit-type that this unit was, when last seen.
**  Currently only used by buildings.
**
**  Unit::Player
**
**  Pointer to the owner of this unit (::Player). An unit could
**  only be owned by one player.
**
**  Unit::Stats
**
**  Pointer to the current status (::UnitStats) of an unit. The
**  units of the same player and the same type could share the same
**  stats. The status contains all values which could be different
**  for each player. This f.e. the upgradeable abilities of an
**  unit.  (Unit::Stats::SightRange, Unit::Stats::Armor,
**  Unit::Stats::HitPoints, ...)
**
**  Unit::CurrentSightRange
**
**  Current sight range of a unit, this changes when a unit enters
**  a transporter or building or exits one of these.
**
**  Unit::Colors
**
**  Player colors of the unit. Contains the hardware dependent
**  pixel values for the player colors (palette index #208-#211).
**  Setup from the global palette. This is a pointer.
**  @note Index #208-#211 are various SHADES of the team color
**  (#208 is brightest shade, #211 is darkest shade) .... these
**  numbers are NOT red=#208, blue=#209, etc
**
**  Unit::IX Unit::IY
**
**  Coordinate displacement in pixels or coordinates inside a tile.
**  Currently only !=0, if the unit is moving from one tile to
**  another (0-32 and for ships/flyers 0-64).
**
**  Unit::Frame
**
**  Current graphic image of the animation sequence. The high bit
**  (128) is used to flip this image horizontal (x direction).
**  This also limits the number of different frames/image to 126.
**
**  Unit::SeenFrame
**
**  Graphic image (see Unit::Frame) what the player on this
**  computer has last seen. If UnitNotSeen the player haven't seen
**  this unit yet.
**
**  Unit::Direction
**
**  Contains the binary angle (0-255) in which the direction the
**  unit looks. 0, 32, 64, 128, 160, 192, 224, 256 corresponds to
**  0°, 45°, 90°, 135°, 180°, 225°, 270°, 315°, 360° or north,
**  north-east, east, south-east, south, south-west, west,
**  north-west, north. Currently only 8 directions are used, this
**  is more for the future.
**
**  Unit::Attacked
**
**  Last cycle the unit was attacked. 0 means never.
**
**  Unit::Burning
**
**  If Burning is non-zero, the unit is burning.
**
**  Unit::VisCount[PlayerMax]
**
**              Used to keep track of visible units on the map, it counts the
**              Number of seen tiles for each player. This is only modified
**              in UnitsMarkSeen and UnitsUnmarkSeen, from fow.
**              We keep track of visilibty for each player, and combine with
**              Shared vision ONLY when querying and such.
**
**      Unit::SeenByPlayer
**
**              This is a bitmask of 1 and 0 values. SeenByPlayer & (1<<p) is 0
**              If p never saw the unit and 1 if it did. This is important for
**              keeping track of dead units under fog. We only keep track of units
**              that are visible under fog with this.
**
**  Unit::Destroyed
**
** @todo docu.
**  If you need more informations, please send me an email or write it self.
**
**  Unit::Removed
**
**  This flag means the unit is not active on map. This flag
**  have workers set if they are inside a building, units that are
**  on board of a transporter.
**
**  Unit::Selected
**
**
**  Unit::Constructed
**  Set when a building is under construction, and still using the
**  generic building animation.
**
**  Unit::SeenConstructed
**  Last seen state of construction.  Used to draw correct building
**  frame. See Unit::Constructed for more information.
**
**  Unit::SeenState
**  The Seen State of the building.
**  01 The building in being built when last seen.
**  10 The building was been upgraded when last seen.
**
**  Unit::Boarded
**
**  This is 1 if the unit is on board a transporter.
**
**  Unit::Mana
**
**
**  Unit::HP
**
**
**  Unit::XP
**
**
**  Unit::Kills
**
**
**  Unit::Bloodlust
**
**
**  Unit::Haste
**
**
**  Unit::Slow
**
**
**  Unit::Invisible
**
**
**  Unit::FlameShield
**
**
**  Unit::UnholyArmor
**
**
**  Unit::GroupId
**
**  Number of the group to that the unit belongs. This is the main
**  group showed on map, an unit can belong to many groups.
**
**  Unit::LastGroup
**
**  Automatic group number, to reselect the same units. When the
**  user selects more than one unit all units is given the next
**  same number. (Used for ALT-CLICK)
**
**  Unit::Value
**
**  This values hold the amount of resources in a resource or in
**  in a harvester.
**  @todo continue documentation
**
**  Unit::SubAction
**
**  This is an action private variable, it is zero on the first
**  entry of an action. Must be set to zero, if an action finishes.
**  It should only be used inside of actions.
**
**  Unit::Wait
**
**  The unit is forced too wait for that many cycles. Be carefull,
**  setting this to 0 will lock the unit.
**
**  Unit::State
**
**  Animation state, currently position in the animation script.
**  0 if an animation has just started, it should only be changed
**  inside of actions.
**
**  Unit::Reset
**
**  @todo continue documentation
**
**  Unit::Blink
**
**
**  Unit::Moving
**
**
**  Unit::RescuedFrom
**
**  Pointer to the original owner of an unit. It will be NULL if
**  the unit was not rescued.
**
**  Unit::OrderCount
**
**  The number of the orders unit to process. An unit has atleast
**  one order. Unit::OrderCount should be a number at least 1.
**  The orders are in Unit::Orders[].
**
**  Unit::OrderFlush
**
**  A flag, which tells the unit to stop with the current order
**  and immediately start with the next order.
**
**  Unit::TotalOrders
**
**  The number of Orders allocated for this unit to use.
**  Default is 4, but is dynamically updated if more orders are
**  given.
**
**  Unit::Orders
**
**  Contains all orders of the unit. Slot 0 is always used.
**
**  Unit::SavedOrder
**
**  This order is executed, if the current order is finished.
**  This is used for attacking units, to return to the old
**  place or for patrolling units to return to patrol after
**  killing some enemies. Any new order given to the unit,
**  clears this saved order.
**
**  Unit::NewOrder
**
**  This field is only used by buildings and this order is
**  assigned to any by this building new trained unit.
**  This is can be used to set the exit or gathering point of a
**  building.
**
**  Unit::Data
**
**  @todo continue documentation
**
**  Unit::Goal
**
**  Generic goal pointer. Used by teleporters to point to circle of power.
**
**  Unit::Retreating
**
** @todo continue documentation
**
*/

/*----------------------------------------------------------------------------
-- Includes
----------------------------------------------------------------------------*/

#include "SDL.h"
#include "player.h"

/*----------------------------------------------------------------------------
-- Declarations
----------------------------------------------------------------------------*/

typedef struct _unit_ Unit;            ///< unit itself
typedef enum _unit_action_ UnitAction; ///< all possible unit actions
#ifdef USE_OPENGL
struct _graphic_;
#endif
struct _unit_type_;
struct _unit_stats_;
struct _spell_type_;
struct _unit_colors_;
struct _construction_frame_;
struct _variable_type_;
struct _upgrade_;
struct _building_restrictions_;
struct _CL_File_;
struct lua_State;

/**
**  Unit references over network, or for memory saving.
*/
typedef unsigned short UnitRef;

/**
**  All possible unit actions.
**
**  @note  Always change the table ::HandleActionTable
**
**  @see HandleActionTable
*/
enum _unit_action_ {
	UnitActionNone,         ///< No valid action

	UnitActionStill,        ///< unit stand still, does nothing
	UnitActionStandGround,  ///< unit stands ground
	UnitActionFollow,       ///< unit follows units
	UnitActionMove,         ///< unit moves to position/unit
	UnitActionAttack,       ///< unit attacks position/unit
	UnitActionAttackGround, ///< unit attacks ground
	UnitActionDie,          ///< unit dies

	UnitActionSpellCast,    ///< unit casts spell

	UnitActionTrain,        ///< building is training
	UnitActionUpgradeTo,    ///< building is upgrading itself
	UnitActionResearch,     ///< building is researching spell
	UnitActionBuilded,      ///< building is under construction

// Compound actions
	UnitActionBoard,        ///< unit entering transporter
	UnitActionUnload,       ///< unit leaving transporter
	UnitActionPatrol,       ///< unit paroling area
	UnitActionBuild,        ///< unit builds building

	UnitActionRepair,       ///< unit repairing
	UnitActionResource,     ///< unit harvesting resources
	UnitActionReturnGoods   ///< unit returning any resource
};

/**
**  Unit order structure.
*/
typedef struct _order_ {
	unsigned char Action;   ///< global action
	unsigned char Flags;    ///< Order flags (unused)
	int           Range;    ///< How far away
	unsigned int  MinRange; ///< How far away minimum
	unsigned char Width;    ///< Goal Width (used when Goal is not)
	unsigned char Height;   ///< Goal Height (used when Goal is not)

	Unit*     Goal;         ///< goal of the order (if any)
	int       X;            ///< or X tile coordinate of destination
	int       Y;            ///< or Y tile coordinate of destination
	struct _unit_type_* Type;///< Unit-type argument

	union {
		struct {
			int X;                    ///< X position for patroling.
			int Y;                    ///< Y position for patroling.
		} Patrol;                     ///< position.
		int ResourcePos;              ///< ResourcePos == (X<<16 | Y).
		struct _spell_type_* Spell;   ///< spell when casting.
		struct _upgrade_* Upgrade;    ///< upgrade.
		struct _order_* Order;        ///< FIXME : seems to be a hack for free memory.
	} Arg1;             ///< Extra command argument.
} Order;

/**
**  Voice groups for an unit
*/
typedef enum _unit_voice_group_ {
	VoiceSelected,          ///< If selected
	VoiceAcknowledging,     ///< Acknowledge command
	VoiceAttacking,         ///< FIXME: Should be removed?
	VoiceReady,             ///< Command completed
	VoiceHelpMe,            ///< If attacked
	VoiceDying,             ///< If killed
	VoiceWorkCompleted,     ///< only worker, work completed
	VoiceBuilding,          ///< only for building under construction
	VoiceDocking,           ///< only for transport reaching coast
	VoiceRepairing,         ///< repairing
	VoiceHarvesting,        ///< harvesting
} UnitVoiceGroup;

/**
**  Unit/Missile headings.
**          N
**  NW              NE
**  W                E
**  SW              SE
**          S
*/
enum _directions_ {
	LookingN  = 0 * 32,      ///< Unit looking north
	LookingNE = 1 * 32,      ///< Unit looking north east
	LookingE  = 2 * 32,      ///< Unit looking east
	LookingSE = 3 * 32,      ///< Unit looking south east
	LookingS  = 4 * 32,      ///< Unit looking south
	LookingSW = 5 * 32,      ///< Unit looking south west
	LookingW  = 6 * 32,      ///< Unit looking west
	LookingNW = 7 * 32,      ///< Unit looking north west
};


	/// A linked list element.
typedef struct _unit_list_item_ UnitListItem;
struct _unit_list_item_ {
	Unit*         Unit; ///< Points to the unit it links
	UnitListItem* Prev; ///< Previous item.
	UnitListItem* Next; ///< Next item.
};

#define NextDirection 32        ///< Next direction N->NE->E...
#define UnitNotSeen 0x7fffffff  ///< Unit not seen, used by Unit::SeenFrame

	/// The big unit structure
struct _unit_ {
	// @note int is faster than shorts
	int    Refs;         ///< Reference counter
	int    Slot;         ///< Assigned slot number
	Unit** UnitSlot;     ///< Slot pointer of Units
	Unit** PlayerSlot;   ///< Slot pointer of Player->Units

	Unit*         Next;          ///< Generic link pointer (on map)
	UnitListItem* CacheLinks;    ///< Link nodes for unit cache
	unsigned      CacheLock : 1; ///< Used to lock unit out of the cache.

	int   InsideCount;   ///< Number of units inside.
	int   BoardCount;    ///< Number of units transported inside.
	Unit* UnitInside;    ///< Pointer to one of the units inside.
	Unit* Container;     ///< Pointer to the unit containing it (or 0)
	Unit* NextContained; ///< Next unit in the container.
	Unit* PrevContained; ///< Previous unit in the container.

	int X; ///< Map position X
	int Y; ///< Map position Y

	struct _unit_type_*  Type;    ///< Pointer to unit-type (peon,...)
	Player*    Player;            ///< Owner of this unit
	struct _unit_stats_* Stats;   ///< Current unit stats
	int        CurrentSightRange; ///< Unit's Current Sight Range

// DISPLAY:
	struct _unit_colors_* Colors; ///< Player colors
	signed char IX;         ///< X image displacement to map position
	signed char IY;         ///< Y image displacement to map position
	int         Frame;      ///< Image frame: <0 is mirrored

	unsigned Direction : 8; ///< angle (0-255) unit looking

	unsigned long Attacked; ///< gamecycle unit was last attacked

	unsigned Burning : 1;   ///< unit is burning
	unsigned Destroyed : 1; ///< unit is destroyed pending reference
	unsigned Removed : 1;   ///< unit is removed (not on map)
	unsigned Selected : 1;  ///< unit is selected
	unsigned TeamSelected;  ///< unit is selected by a team member.

	unsigned Constructed : 1; ///< Unit is in construction
	unsigned Active : 1;      ///< Unit is active for AI
	unsigned Boarded : 1;     ///< Unit is on board a transporter.
	Player*  RescuedFrom;     ///< The original owner of a rescued unit.
							  ///< NULL if the unit was not rescued.
	/* Seen stuff. */
	char VisCount[PlayerMax]; ///< Unit visibility counts
	struct _seen_stuff_ {
		unsigned            ByPlayer : PlayerMax;    ///< Track unit seen by player
		int                 Frame;                   ///< last seen frame/stage of buildings
		struct _unit_type_* Type;                    ///< Pointer to last seen unit-type
		int                 X;                       ///< Last unit->X Seen
		int                 Y;                       ///< Last unit->Y Seen
		signed char         IX;                      ///< Seen X image displacement to map position
		signed char         IY;                      ///< seen Y image displacement to map position
		unsigned            Constructed : 1;         ///< Unit seen construction
		unsigned            State : 3;               ///< Unit seen build/upgrade state
		unsigned            Destroyed : PlayerMax;   ///< Unit seen destroyed or not
		struct _construction_frame_* CFrame;         ///< Seen construction frame
	} Seen;

	int Mana;               ///< mana points
	int HP;                 ///< hit points
	int XP;                 ///< experience points
	int Kills;              ///< how many unit has this unit killed
	struct _variable_type_* Variable; ///< array of User Defined variables.

	unsigned long TTL;  ///< time to live
	int Bloodlust;      ///< ticks bloodlust
	int Haste;          ///< ticks haste (disables slow)
	int Slow;           ///< ticks slow (disables haste)
	int Invisible;      ///< ticks invisible
	int FlameShield;    ///< ticks flame shield
	int UnholyArmor;    ///< ticks unholy armor

	int GroupId;        ///< unit belongs to this group id
	int LastGroup;      ///< unit belongs to this last group

	int ResourcesHeld;      ///< Resources Held by a unit

	unsigned SubAction : 8; ///< sub-action of unit
	unsigned Wait;          ///< action counter
	unsigned State : 8;     ///< action state
#define MAX_UNIT_STATE 255  ///< biggest state for action
	unsigned Reset : 1;     ///< can process new command
	unsigned Blink : 3;     ///< Let selection rectangle blink
	unsigned Moving : 1;    ///< The unit is moving
	unsigned ReCast : 1;    ///< Recast again next cycle

	/** set to random 1..100 when MakeUnit()
	** used for fancy buildings
	*/
	unsigned Rs : 8;
	unsigned char CurrentResource;

	char OrderCount;            ///< how many orders in queue
	char OrderFlush;            ///< cancel current order, take next
	int  TotalOrders;           ///< Total Number of orders available
	Order* Orders;              ///< orders to process
	Order SavedOrder;           ///< order to continue after current
	Order NewOrder;             ///< order for new trained units
	char* AutoCastSpell;        ///< spells to auto cast

	union _order_data_ {
	struct _order_move_ {
		char Fast;                  ///< Flag fast move (one step)
		char Length;                ///< stored path length
#define MAX_PATH_LENGTH 28          ///< max length of precalculated path
		char Path[MAX_PATH_LENGTH]; ///< directions of stored path
	} Move; ///< ActionMove,...
	struct _order_builded_ {
		Unit* Worker;               ///< Worker building this unit
		int Progress;               ///< Progress counter, in 1/100 cycles.
		int Cancel;                 ///< Cancel construction
		struct _construction_frame_* Frame; ///< Construction frame
	} Builded; ///< ActionBuilded,...
	struct _order_resource_ {
		int Active; ///< how many units are harvesting from the resource.
	} Resource; ///< Resource still
	struct _order_resource_worker_ {
		int TimeToHarvest;          ///< how much time until we harvest some more.
		unsigned DoneHarvesting:1;  ///< Harvesting done, wait for action to break.
	} ResWorker; ///< Worker harvesting
	struct _order_research_ {
		struct _upgrade_* Upgrade; ///< Upgrade researched
	} Research; ///< Research action
	struct _order_upgradeto_ {
		int Ticks; ///< Ticks to complete
	} UpgradeTo; ///< Upgrade to action
	struct _order_train_ {
		int Ticks;                      ///< Ticks to complete
	} Train; ///< Train units action
	} Data; ///< Storage room for different commands

	Unit* Goal; ///< Generic goal pointer
};

#define NoUnitP (Unit*)0         ///< return value: for no unit found
#define InfiniteDistance INT_MAX ///< the distance is unreachable

#define FlushCommands 1          ///< Flush commands in queue

#define MAX_UNIT_SLOTS 65535     ///< Maximal number of used slots

/**
**  Returns true, if unit is unusable. (for attacking,...)
**  @todo look if correct used (UnitActionBuilded is no problem if attacked)?
*/
#define UnitUnusable(unit) \
	((unit)->Removed || (unit)->Orders[0].Action == UnitActionDie || \
	  (unit)->Orders[0].Action == UnitActionBuilded || (unit)->Destroyed)

/**
**  Returns unit number (unique to this unit)
*/
#define UnitNumber(unit) ((unit)->Slot)

/**
**  Check if a unit is idle.
*/
#define UnitIdle(unit) ((unit->Orders[0].Action == UnitActionStill) && (unit->OrderCount == 1))

/**
**  Return the unit type movement mask.
**
**  @param type Unit type pointer.
**
**  @return Movement mask of unit type.
*/
#define TypeMovementMask(type) ((type)->MovementMask)

/**
**  Return units movement mask.
**
**  @param unit Unit pointer.
**
**  @return Movement mask of unit.
*/
#define UnitMovementMask(unit) ((unit)->Type->MovementMask)

/**
** How many groups supported
*/
#define NUM_GROUPS 10

/**
** Always show unit orders
*/
#define SHOW_ORDERS_ALWAYS -1

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

extern Unit* UnitSlots[MAX_UNIT_SLOTS]; ///< All possible units
extern Unit** UnitSlotFree;             ///< First free unit slot

extern Unit* Units[MAX_UNIT_SLOTS]; ///< Units used
extern int NumUnits;                ///< Number of units used

// in unit_draw.c
/// @todo could be moved into the user interface ?
extern int ShowSightRange;              ///< Flag: show right range
extern int ShowReactionRange;           ///< Flag: show reaction range
extern int ShowAttackRange;             ///< Flag: show attack range
extern int ShowOrders;                  ///< Flag: show orders of unit on map
extern unsigned long ShowOrdersCount;   ///< Show orders for some time
extern int XpDamage;                    ///< unit XP adds more damage to attacks
extern char EnableTrainingQueue;        ///< Config: training queues enabled
extern char EnableBuildingCapture;      ///< Config: building capture enabled
extern char RevealAttacker;             ///< Config: reveal attacker enabled
extern const Viewport* CurrentViewport; ///< CurrentViewport
extern void DrawUnitSelection(const Unit*);
extern void (*DrawSelection)(Uint32, int, int, int, int);
extern int MaxSelectable;                  ///< How many units could be selected

extern Unit** Selected;                    ///< currently selected units
extern Unit** TeamSelected[PlayerMax];     ///< teams currently selected units
extern int    NumSelected;                 ///< how many units selected
extern int    NumTeamSelected[PlayerMax];  ///< Number of Units a team member has selected

extern Unit* ReleasedHead;                 ///< Head of the released unit list.
extern Unit* ReleasedTail;                 ///< Tail of the released unit list.

extern Order* ReleasedOrderHead;           ///< Head of the released orders list.
extern Order* ReleasedOrderTail;           ///< Tail of the released unit list.

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

	/// Prepare unit memory allocator
extern void InitUnitsMemory(void);
#if 0
	/// Free memory used by unit
extern void FreeUnitMemory(Unit* unit);
#endif
	/// Increase an unit's reference count
extern void RefsIncrease(Unit* unit);
	/// Decrease an unit's reference count
extern void RefsDecrease(Unit* unit);
	/// Release an unit
extern void ReleaseUnit(Unit* unit);
	/// Initialize unit structure with default values
extern void InitUnit(Unit* unit, struct _unit_type_* type);
	/// Assign unit to player
extern void AssignUnitToPlayer(Unit* unit, Player* player);
	/// Create a new unit
extern Unit* MakeUnit(struct _unit_type_* type,Player* player);
	/// Place an unit on map
extern void PlaceUnit(Unit* unit, int x, int y);
	/// Create a new unit and place on map
extern Unit* MakeUnitAndPlace(int x, int y, struct _unit_type_* type,Player* player);
	/// Move unit to tile(x, y). (Do special stuff : vision, cachelist, pathfinding)
extern void MoveUnitToXY(Unit* unit, int x, int y);
	/// Add an unit inside a container. Only deal with list stuff.
extern void AddUnitInContainer(Unit* unit, Unit* host);
	/// Remove unit from map/groups/...
extern void RemoveUnit(Unit* unit, Unit* host);
	/// Handle the loose of an unit (food,...)
extern void UnitLost(Unit* unit);
	/// Remove the Orders of a Unit
extern void UnitClearOrders(Unit* unit);
	/// @todo more docu
extern void UpdateForNewUnit(const Unit* unit, int upgrade);
	/// @todo more docu
extern void NearestOfUnit(const Unit* unit, int tx, int ty, int *dx, int *dy);

	/// Call when an Unit goes under fog.
extern void UnitGoesUnderFog(Unit* unit, const Player* player);
	/// Call when an Unit goes out of fog.
extern void UnitGoesOutOfFog(Unit* unit, const Player* player);
	/// Marks an unit as seen
extern void UnitsOnTileMarkSeen(const Player* player, int x, int y, int p);
	/// Unmarks an unit as seen
extern void UnitsOnTileUnmarkSeen(const Player* player, int x, int y, int p);
	/// Does a recount for VisCount
extern void UnitCountSeen(Unit* unit);

	/// Returns true, if unit is directly seen by an allied unit.
extern int UnitVisible(const Unit* unit, const Player* player);
	/// Returns true, if unit is visible as a goal.
extern int UnitVisibleAsGoal(const Unit* unit, const Player* player);
	/// Returns true, if unit is Visible for game logic on the map.
extern int UnitVisibleOnMap(const Unit* unit, const Player* player);
	/// Returns true if unit is visible on minimap. Only for ThisPlayer.
extern int UnitVisibleOnMinimap(const Unit* unit);
	/// Returns true if unit is visible in an viewport. Only for ThisPlayer.
extern int UnitVisibleInViewport(const Unit* unit, const Viewport* vp);

	/// @todo more docu
extern void GetUnitMapArea(const Unit* unit, int *sx, int *sy,
	int *ex, int *ey);
	/// Check for rescue each second
extern void RescueUnits(void);
	/// Change owner of unit
extern void ChangeUnitOwner(Unit* unit, Player* newplayer);

	/// Convert direction (dx,dy) to heading (0-255)
extern int DirectionToHeading(int, int);
	/// Update frame from heading
extern void UnitUpdateHeading(Unit* unit);
	/// Heading and frame from delta direction x,y
extern void UnitHeadingFromDeltaXY(Unit* unit, int x, int y);

	/// @todo more docu
extern void DropOutOnSide(Unit* unit, int heading, int addx, int addy);
	/// @todo more docu
extern void DropOutNearest(Unit* unit, int x, int y, int addx, int addy);
	/// Drop out all units in the unit
extern void DropOutAll(const Unit* unit);

	/// Return the rule used to build this building.
extern struct _building_restrictions_* OnTopDetails(const Unit* unit, const struct _unit_type_* parent);
	/// @todo more docu
extern Unit* CanBuildHere(const Unit* unit, const struct _unit_type_* type, int x, int y);
	/// @todo more docu
extern int CanBuildOn(int x, int y, int mask);
	/// FIXME: more docu
extern Unit* CanBuildUnitType(const Unit* unit,const struct _unit_type_* type, int x, int y, int real);

	/// Find resource
extern Unit* FindResource(const Unit* unit, int x, int y, int range, int resource);
	/// Find nearest deposit
extern Unit* FindDeposit(const Unit* unit, int x, int y, int range, int resource);
	/// Find the next idle worker
extern Unit* FindIdleWorker(const Player* player,const Unit* last);

	/// Find the neareast piece of terrain with specific flags.
extern int FindTerrainType(int movemask, int resmask, int rvresult, int range,
		const Player *player, int x, int y, int* px, int* py);
	/// Find the nearest piece of wood in sight range
extern int FindWoodInSight(const Unit* unit, int* x, int* y);

	/// @todo more docu
extern Unit* UnitOnScreen(Unit* unit, int x, int y);

	/// Let an unit die
extern void LetUnitDie(Unit* unit);
	/// Destory all units inside another unit
extern void DestroyAllInside(Unit* source);
	/// Hit unit with damage, if destroyed give attacker the points
extern void HitUnit(Unit* attacker, Unit* target, int damage);

	/// Returns the map distance between two points
extern int MapDistance(int x1, int y1, int x2, int y2);
	/// Returns the map distance between two points with unit-type
extern int MapDistanceToType(int x1, int y1,const struct _unit_type_* type, int x2, int y2);
	/// Returns the map distance to unit
extern int MapDistanceToUnit(int x, int y,const Unit* dest);
	/// Returns the map diestance between to unittype as locations
extern int MapDistanceBetweenTypes(const struct _unit_type_* src, int x1, int y1, const struct _unit_type_* dst, int x2, int y2);
	/// Returns the map distance between two units
extern int MapDistanceBetweenUnits(const Unit* src,const Unit* dst);

	/// Calculate the distance from current view point to coordinate
extern int ViewPointDistance(int x, int y);
	/// Calculate the distance from current view point to unit
extern int ViewPointDistanceToUnit(const Unit* dest);

	/// Return true, if unit is an enemy of the player
#define IsEnemy(player, dest) \
	((player)->Enemy & (1 << (dest)->Player->Player))
	/// Return true, if unit is allied of the player
#define IsAllied(player, dest) \
	((player)->Allied & (1 << (dest)->Player->Player))
	/// Return true, if unit is shared vision with the player
#define IsSharedVision(player, dest) \
	(((player)->SharedVision & (1 << (dest)->Player->Player)) && \
		((dest)->Player->SharedVision & (1 << (player)->Player)))
	/// Can this unit-type attack the other (destination)
extern int CanTarget(const struct _unit_type_* type, const struct _unit_type_* dest);
	/// Can transporter transport the other unit
extern int CanTransport(const Unit* transporter, const Unit* unit);

	/// Check if unit can move.
extern int CanMove(const Unit*);

	/// Generate a unit reference, a printable unique string for unit
extern char* UnitReference(const Unit*);
	/// Save an order
extern void SaveOrder(const Order* order, struct _CL_File_* file);
	/// save unit-structure
extern void SaveUnit(const Unit* unit, struct _CL_File_* file);
	/// save all units
extern void SaveUnits(struct _CL_File_* file);

	/// Initialize unit module
extern void InitUnits(void);
	/// Clean unit module
extern void CleanUnits(void);

// in unitcache.c
	/// Insert new unit into cache
extern void UnitCacheInsert(Unit* unit);
	/// Remove unit from cache
extern void UnitCacheRemove(Unit* unit);
	/// Select units in range
extern int UnitCacheSelect(int x1, int y1, int x2, int y2, Unit** table);
	/// Select units on tile
extern int UnitCacheOnTile(int x, int y, Unit** table);
	/// Select unit on X,Y of type naval,fly,land
extern Unit* UnitCacheOnXY(int x, int y, unsigned type);
	/// Initialize unit-cache
extern void InitUnitCache(void);

// in unit_draw.c
//--------------------
	/// Draw nothing around unit
extern void DrawSelectionNone(Uint32, int, int, int, int);
	/// Draw circle around unit
extern void DrawSelectionCircle(Uint32, int, int, int, int);
	/// Draw circle filled with alpha around unit
extern void DrawSelectionCircleWithTrans(Uint32, int, int, int, int);
	/// Draw rectangle around unit
extern void DrawSelectionRectangle(Uint32, int, int, int, int);
	/// Draw rectangle filled with alpha around unit
extern void DrawSelectionRectangleWithTrans(Uint32, int, int, int, int);
	/// Draw corners around unit
extern void DrawSelectionCorners(Uint32, int, int, int, int);

	/// Register CCL decorations features
extern void DecorationCclRegister(void);
	/// Load the decorations (health,mana) of units
extern void LoadDecorations(void);
	/// Clean the decorations (health,mana) of units
extern void CleanDecorations(void);

	/// Draw unit's shadow
extern void DrawShadow(const Unit* unit, const struct _unit_type_* type,
	int frame, int x, int y);
	/// Draw A single Unit
extern void DrawUnit(const Unit* unit);
	/// Draw all units visible on map in viewport
extern int FindAndSortUnits(const Viewport* vp, Unit** table);
	/// Show an unit's orders.
extern void ShowOrder(const Unit* unit);

// in unit_find.c
	/// Find all units of this type
extern int FindUnitsByType(const struct _unit_type_* type, Unit** table);
	/// Find all units of this type of the player
extern int FindPlayerUnitsByType(const Player*, const struct _unit_type_*, Unit**);
	/// Return any unit on that map tile
extern Unit* UnitOnMapTile(int tx, int ty);
	/// Return repairable unit on that map tile
extern Unit* RepairableOnMapTile(int tx, int ty);
	/// Return possible attack target on a tile
extern Unit* TargetOnMapTile(const Unit* soruce, int tx, int ty);
	/// Return possible attack target on that map area
extern Unit* TargetOnMap(const Unit* unit, int x1, int y1, int x2, int y2);
	/// Return transporter unit on that map tile
extern Unit* TransporterOnMapTile(int tx, int ty);

	/// Return unit of a fixed type on a map tile.
extern Unit* UnitTypeOnMap(int tx, int ty, struct _unit_type_* type);
	/// Return resource, if on map tile
extern Unit* ResourceOnMap(int tx, int ty, int resource);
	/// Return resource deposit, if on map tile
extern Unit* ResourceDepositOnMap(int tx, int ty, int resource);

	/// Find best enemy in numeric range to attack
extern Unit* AttackUnitsInDistance(const Unit* unit, int range);
	/// Find best enemy in attack range to attack
extern Unit* AttackUnitsInRange(const Unit* unit);
	/// Find best enemy in reaction range to attack
extern Unit* AttackUnitsInReactRange(const Unit* unit);

// in groups.c

	/// Initialize data structures for groups
extern void InitGroups(void);
	/// Save groups
extern void SaveGroups(struct _CL_File_* file);
	/// Cleanup groups
extern void CleanGroups(void);

	// 2 functions to conseal the groups internal data structures...
	/// Get the number of units in a particular group
extern int GetNumberUnitsOfGroup(int num);
	/// Get the array of units of a particular group
extern Unit** GetUnitsOfGroup(int num);

	/// Remove all units from a group
extern void ClearGroup(int num);
	/// Add the array of units to the group
extern void AddToGroup(Unit** units, int nunits, int num);
	/// Set the contents of a particular group with an array of units
extern void SetGroup(Unit** units, int nunits, int num);
	/// Remove a unit from a group
extern void RemoveUnitFromGroups(Unit* unit);
	/// Register CCL group features
extern void GroupCclRegister(void);

// in selection.c

	/// Check if unit is the currently only selected
#define IsOnlySelected(unit) (NumSelected == 1 && Selected[0] == (unit))

	/// Clear current selection
extern void UnSelectAll(void);
	/// Select group as selection
extern void ChangeSelectedUnits(Unit** units, int num_units);
	/// Changed TeamUnit Selection
extern void ChangeTeamSelectedUnits(Player* player, Unit** units, int adjust, int count);
	/// Add a unit to selection
extern int SelectUnit(Unit* unit);
	/// Select one unit as selection
extern void SelectSingleUnit(Unit* unit);
	/// Remove a unit from selection
extern void UnSelectUnit(Unit* unit);
	/// Add a unit to selected if not already selected, remove it otherwise
extern int ToggleSelectUnit(Unit* unit);
	/// Select units from the same type (if selectable by rectangle)
extern int SelectUnitsByType(Unit* base);
	/// Toggle units from the same type (if selectable by rectangle)
extern int ToggleUnitsByType(Unit* base);
	/// Select the units belonging to a particular group
extern int SelectGroup(int group_number);
	/// Add the units from the same group as the one in parameter
extern int AddGroupFromUnitToSelection(Unit* unit);
	/// Select the units from the same group as the one in parameter
extern int SelectGroupFromUnit(Unit* unit);
	/// Select the units in the selection rectangle
extern int SelectUnitsInRectangle(int tx, int ty, int w, int h);
	/// Select ground units in the selection rectangle
extern int SelectGroundUnitsInRectangle(int tx, int ty, int w, int h);
	/// Select flying units in the selection rectangle
extern int SelectAirUnitsInRectangle(int tx, int ty, int w, int h);
	/// Add the units in the selection rectangle to the current selection
extern int AddSelectedUnitsInRectangle(int tx, int ty, int w, int h);
	/// Add ground units in the selection rectangle to the current selection
extern int AddSelectedGroundUnitsInRectangle(int tx, int ty, int w, int h);
	/// Add flying units in the selection rectangle to the current selection
extern int AddSelectedAirUnitsInRectangle(int tx, int ty, int w, int h);

	/// Init selections
extern void InitSelections(void);
	/// Save current selection state
extern void SaveSelections(struct _CL_File_* file);
	/// Clean up selections
extern void CleanSelections(void);
	/// Register CCL selection features
extern void SelectionCclRegister(void);

// in ccl_unit.c

	/// Parse order
extern void CclParseOrder(struct lua_State* l, Order* order);
	/// register CCL units features
extern void UnitCclRegister(void);

//@}

#endif // !__UNIT_H__
