//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name unit.h - The unit headerfile. */
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

#ifndef __UNIT_H__
#define __UNIT_H__

//@{

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
**  Maximal 65535 (=#MAX_UNIT_SLOTS) simultaneous units are
**  supported.
**
**  CUnit::UnitSlot
**
**  This is the pointer into #Units[], where the unit pointer is
**  stored.  #Units[] is a table of all units currently active in
**  game. This pointer is only needed to speed up, the remove of
**  the unit pointer from #Units[], it didn't must be searched in
**  the table.
**
**  CUnit::PlayerSlot
**
**  A pointer into Player::Units[], where the unit pointer is
**  stored. Player::Units[] is a table of all units currently
**  belonging to a player. This pointer is only needed to speed
**  up, the remove of the unit pointer from Player::Units[].
**
**  CUnit::Next
**
**  A generic link pointer. This member is currently used, if an
**  unit is on the map, to link all units on the same map field
**  together. This also links corpses and stuff. Also, this is
**  used in memory management to link unused units.
**
**  CUnit::Container
**
**  Pointer to the unit containing it, or NoUnitP if the unit is
**  free. This points to the transporter for units on board, or to
**  the building for peasants inside(when they are mining).
**
**  CUnit::UnitInside
**
**  Pointer to the last unit added inside. Order doesn't really
**  matter. All units inside are kept in a circular linked list.
**  This is NoUnitP if there are no units inside. Multiple levels
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
**  CUnit::X CUnit::Y
**
**  The tile map coordinates of the unit. 0,0 is the upper left on
**  the map. To convert the map coordinates into pixels, they
**  must be multiplicated with the #TileSizeX and #TileSizeY.
**  To get the pixel coordinates of a unit, calculate
**  CUnit::X*#TileSizeX+CUnit::IX , CUnit::Y*#TileSizeY+CUnit::IY.
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
**  If the unit is moving from one tile to an adjacent one.
**  CUnit::IX is between -::TileSizeX and +::TileSizeX,
**  and counts towards 0 during horizontal motion.
**  CUnit::IY is similar.
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
**  0, 45, 90, 135, 180, 225, 270, 315, 360 or north,
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
**  Unit is destroyed. pending reference.
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
**
**  CUnit::SubAction
**
**  This is an action private variable, it is zero on the first
**  entry of an action. Must be set to zero, if an action finishes.
**  It should only be used inside of actions.
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
**  CUnit::OrderCount
**
**  The number of the orders unit to process. An unit has atleast
**  one order. CUnit::OrderCount should be a number at least 1.
**  The orders are in CUnit::Orders[].
**
**  CUnit::OrderFlush
**
**  A flag, which tells the unit to stop with the current order
**  and immediately start with the next order.
**
**  CUnit::TotalOrders
**
**  The number of Orders allocated for this unit to use.
**  Default is 4, but is dynamically updated if more orders are
**  given.
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
**  CUnit::Data
**
**  @todo continue documentation
**
**  CUnit::Goal
**
**  Generic goal pointer. Used by teleporters to point to circle of power.
**
*/

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <vector>
#include "SDL.h"
#include "upgrade_structs.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUnit;
class CUnitType;
class CUnitStats;
class CPlayer;
class SpellType;
class CUnitColors;
class CConstructionFrame;
class CVariable;
class CBuildRestrictionOnTop;
class CFile;
struct lua_State;
class CViewport;
class CAnimation;

	/// Called whenever the selected unit was updated
extern void SelectedUnitChanged(void);

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
typedef enum _unit_action_ {
	UnitActionNone,         /// No valid action

	UnitActionStill,        /// unit stand still, does nothing
	UnitActionStandGround,  /// unit stands ground
	UnitActionFollow,       /// unit follows units
	UnitActionMove,         /// unit moves to position/unit
	UnitActionAttack,       /// unit attacks position/unit
	UnitActionAttackGround, /// unit attacks ground
	UnitActionDie,          /// unit dies

	UnitActionSpellCast,    /// unit casts spell

	UnitActionTrain,        /// building is training
	UnitActionBuilt,      /// building is under construction

// Compound actions
	UnitActionBoard,        /// unit entering transporter
	UnitActionUnload,       /// unit leaving transporter
	UnitActionPatrol,       /// unit paroling area
	UnitActionBuild,        /// unit builds building

	UnitActionRepair,       /// unit repairing
	UnitActionResource,     /// unit harvesting resources
} UnitAction;

/**
**  Unit order structure.
*/
class COrder {
public:
	COrder() : Action(UnitActionNone), Range(0), MinRange(0), Width(0),
		Height(0), Goal(NULL), X(-1), Y(-1), Type(NULL)
	{
		memset(&Arg1, 0, sizeof(Arg1));
	};

	void Init() {
		Action = UnitActionNone;
		Range = 0;
		MinRange = 0;
		Width = 0;
		Height = 0;
		Assert(!Goal);
		X = -1; Y = -1;
		Type = NULL;
		memset(&Arg1, 0, sizeof(Arg1));
	};

	unsigned char Action;   /// global action
	int Range;              /// How far away
	unsigned int  MinRange; /// How far away minimum
	unsigned char Width;    /// Goal Width (used when Goal is not)
	unsigned char Height;   /// Goal Height (used when Goal is not)

	CUnit *Goal;            /// goal of the order (if any)
	int X;                  /// or X tile coordinate of destination
	int Y;                  /// or Y tile coordinate of destination
	CUnitType *Type;        /// Unit-type argument

	union {
		struct {
			int X;                    /// X position for patroling.
			int Y;                    /// Y position for patroling.
		} Patrol;                     /// position.
		SpellType *Spell;             /// spell when casting.
	} Arg1;             /// Extra command argument.
};

/**
**  Voice groups for a unit
*/
enum UnitVoiceGroup {
	VoiceSelected,          /// If selected
	VoiceAcknowledging,     /// Acknowledge command
	VoiceReady,             /// Command completed
	VoiceHelpMe,            /// If attacked
	VoiceDying,             /// If killed
	VoiceBuilding,          /// only for building under construction
	VoiceDocking,           /// only for transport reaching coast
};

/**
**  Unit/Missile headings.
**          N
**  NW              NE
**  W                E
**  SW              SE
**          S
*/
enum _directions_ {
	LookingN  = 0 * 32,      /// Unit looking north
	LookingNE = 1 * 32,      /// Unit looking north east
	LookingE  = 2 * 32,      /// Unit looking east
	LookingSE = 3 * 32,      /// Unit looking south east
	LookingS  = 4 * 32,      /// Unit looking south
	LookingSW = 5 * 32,      /// Unit looking south west
	LookingW  = 6 * 32,      /// Unit looking west
	LookingNW = 7 * 32,      /// Unit looking north west
};

#define NextDirection 32        /// Next direction N->NE->E...
#define UnitNotSeen 0x7fffffff  /// Unit not seen, used by CUnit::SeenFrame

	/// The big unit structure
class CUnit {
public:
	CUnit() { Init(); }

	void Init() {
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
		X = 0;
		Y = 0;
		Type = NULL;
		Player = NULL;
		Stats = NULL;
		CurrentSightRange = 0;
		Colors = NULL;
		IX = 0;
		IY = 0;
		Frame = 0;
		Direction = 0;
		Attacked = 0;
		Burning = 0;
		Destroyed = 0;
		Removed = 0;
		Selected = 0;
		TeamSelected = 0;
		Constructed = 0;
		Boarded = 0;
		RescuedFrom = NULL;
		memset(VisCount, 0, sizeof(VisCount));
		memset(&Seen, 0, sizeof(Seen));
		Variable = NULL;
		TTL = 0;
		GroupId = 0;
		LastGroup = 0;
		memset(ResourcesHeld, 0, sizeof(ResourcesHeld));
		ProductionEfficiency = 100;
		SubAction = 0;
		Wait = 0;
		State = 0;
		Blink = 0;
		Moving = 0;
		ReCast = 0;
		memset(&Anim, 0, sizeof(Anim));
		OrderCount = 0;
		OrderFlush = 0;
		Orders.clear();
		SavedOrder.Init();
		NewOrder.Init();
		AutoCastSpell = NULL;
		AutoRepair = 0;
		memset(&Data, 0, sizeof(Data));
		Goal = NULL;
	}

	// @note int is faster than shorts
	unsigned long Refs;   /// Reference counter
	int Slot;             /// Assigned slot number
	CUnit **UnitSlot;     /// Slot pointer of Units
	CUnit **PlayerSlot;   /// Slot pointer of Player->Units

	CUnit   *Next;          /// Generic link pointer (on map)

	int    InsideCount;   /// Number of units inside.
	int    BoardCount;    /// Number of units transported inside.
	CUnit *UnitInside;    /// Pointer to one of the units inside.
	CUnit *Container;     /// Pointer to the unit containing it (or 0)
	CUnit *NextContained; /// Next unit in the container.
	CUnit *PrevContained; /// Previous unit in the container.

	int X;                /// Map position X
	int Y;                /// Map position Y

	CUnitType  *Type;              /// Pointer to unit-type (peon,...)
	CPlayer    *Player;            /// Owner of this unit
	CUnitStats *Stats;             /// Current unit stats
	int         CurrentSightRange; /// Unit's Current Sight Range

// DISPLAY:
	CUnitColors *Colors;    /// Player colors
	signed char IX;         /// X image displacement to map position
	signed char IY;         /// Y image displacement to map position
	int         Frame;      /// Image frame: <0 is mirrored

	unsigned Direction : 8; /// angle (0-255) unit looking

	unsigned long Attacked; /// gamecycle unit was last attacked

	unsigned Burning : 1;   /// unit is burning
	unsigned Destroyed : 1; /// unit is destroyed pending reference
	unsigned Removed : 1;   /// unit is removed (not on map)
	unsigned Selected : 1;  /// unit is selected

	unsigned Constructed : 1;    /// Unit is in construction
	unsigned Boarded : 1;        /// Unit is on board a transporter.
	unsigned TeamSelected;       /// unit is selected by a team member.
	CPlayer *RescuedFrom;        /// The original owner of a rescued unit.
							     /// NULL if the unit was not rescued.
	/* Seen stuff. */
	int VisCount[PlayerMax];     /// Unit visibility counts
	struct _unit_seen_ {
		unsigned            ByPlayer : PlayerMax;    /// Track unit seen by player
		int                 Frame;                   /// last seen frame/stage of buildings
		CUnitType          *Type;                    /// Pointer to last seen unit-type
		int                 X;                       /// Last unit->X Seen
		int                 Y;                       /// Last unit->Y Seen
		signed char         IX;                      /// Seen X image displacement to map position
		signed char         IY;                      /// seen Y image displacement to map position
		unsigned            Constructed : 1;         /// Unit seen construction
		unsigned            State : 3;               /// Unit seen build/upgrade state
		unsigned            Destroyed : PlayerMax;   /// Unit seen destroyed or not
		CConstructionFrame *CFrame;                  /// Seen construction frame
	} Seen;

	CVariable *Variable; /// array of User Defined variables.

	unsigned long TTL;   /// time to live

	int GroupId;         /// unit belongs to this group id
	int LastGroup;       /// unit belongs to this last group

	int ResourcesHeld[MaxCosts];       /// Resources held by a unit
	unsigned ProductionEfficiency : 8; /// Production efficiency

	unsigned SubAction : 8; /// sub-action of unit
	unsigned Wait;          /// action counter
	unsigned State : 8;     /// action state
	unsigned Blink : 3;     /// Let selection rectangle blink
	unsigned Moving : 1;    /// The unit is moving
	unsigned ReCast : 1;    /// Recast again next cycle

	struct _unit_anim_ {
		const CAnimation *Anim;     /// Anim
		const CAnimation *CurrAnim; /// CurrAnim
		int Wait;                   /// Wait
		int Unbreakable;            /// Unbreakable
	} Anim;

	char OrderCount;              /// how many orders in queue
	char OrderFlush;              /// cancel current order, take next
	std::vector<COrder *> Orders; /// orders to process
	COrder SavedOrder;            /// order to continue after current
	COrder NewOrder;              /// order for new trained units
	char *AutoCastSpell;          /// spells to auto cast
	unsigned AutoRepair : 1;      /// True if unit tries to repair on still action.

	union _order_data_ {
	struct _order_move_ {
		char Fast;                  /// Flag fast move (one step)
		char Length;                /// stored path length
#define MAX_PATH_LENGTH 28          /// max length of precalculated path
		char Path[MAX_PATH_LENGTH]; /// directions of stored path
	} Move; /// ActionMove,...
	struct _order_built_ {
		int Progress;               /// Progress counter, in 1/100 cycles.
		int Cancel;                 /// Cancel construction
		CConstructionFrame *Frame;  /// Construction frame
	} Built; /// ActionBuilt,...
	struct _order_harvest_ {
		int CurrentProduction[MaxCosts];
	} Harvest; /// Harvest
	struct _order_train_ {
		int Ticks;                  /// Ticks to complete
	} Train; /// Train units action
	struct _order_repair_ {
		int Progress;               /// Progress counter
	} Repair; /// Repair
	} Data; /// Storage room for different commands

	CUnit *Goal; /// Generic goal pointer


	inline bool IsIdle() const {
		return Orders[0]->Action == UnitActionStill && OrderCount == 1;
	}

	inline void ClearAction() {
		Orders[0]->Action = UnitActionStill;
		SubAction = 0;
		if (Selected) {
			SelectedUnitChanged();
		}
	}

	/// Increase a unit's reference count
	void RefsIncrease();
	/// Decrease a unit's reference count
	void RefsDecrease();

	/// Initialize unit structure with default values
	void Init(CUnitType *type);
	/// Assign unit to player
	void AssignToPlayer(CPlayer *player);

	/// Draw a single unit
	void Draw() const;
	/// Place a unit on map
	void Place(int x, int y);

	/// Move unit to tile(x, y). (Do special stuff : vision, cachelist, pathfinding)
	void MoveToXY(int x, int y);
	/// Add a unit inside a container. Only deal with list stuff.
	void AddInContainer(CUnit *host);
	/// Change owner of unit
	void ChangeOwner(CPlayer *newplayer);

	/// Remove unit from map/groups/...
	void Remove(CUnit *host);
	/// Release a unit
	void Release();

	/// Returns true, if unit is directly seen by an allied unit.
	bool IsVisible(const CPlayer *player) const;
	/// Returns true, if unit is visible as a goal.
	bool IsVisibleAsGoal(const CPlayer *player) const;
	/// Returns true, if the unit is alive and on the map.
	bool IsAliveOnMap() const;
	/// Returns true, if unit is Visible for game logic on the map.
	bool IsVisibleOnMap(const CPlayer *player) const;
	/// Returns true if unit is visible on minimap. Only for ThisPlayer.
	bool IsVisibleOnMinimap() const;
	/// Returns true if unit is visible in an viewport. Only for ThisPlayer.
	bool IsVisibleInViewport(const CViewport *vp) const;
	/// Returns true, if unit is visible on current map view (any viewport).
	bool IsVisibleOnScreen() const;

	/// @todo more docu
	void GetMapArea(int *sx, int *sy, int *ex, int *ey) const;

	bool IsEnemy(const CPlayer *x) const;
	bool IsEnemy(const CUnit *x) const;
	bool IsAllied(const CPlayer *x) const;
	bool IsAllied(const CUnit *x) const;
	bool IsSharedVision(const CPlayer *x) const;
	bool IsSharedVision(const CUnit *x) const;
	bool IsBothSharedVision(const CPlayer *x) const;
	bool IsBothSharedVision(const CUnit *x) const;
	bool IsTeamed(const CPlayer *x) const;
	bool IsTeamed(const CUnit *x) const;

	bool IsUnusable() const;
};

#define NoUnitP (CUnit *)0        /// return value: for no unit found
#define InfiniteDistance INT_MAX  /// the distance is unreachable

#define FlushCommands 1           /// Flush commands in queue

#define MAX_UNIT_SLOTS 65535      /// Maximal number of used slots

/**
**  Returns unit number (unique to this unit)
*/
#define UnitNumber(unit) ((unit)->Slot)

/**
**  How many groups supported
*/
#define NUM_GROUPS 10

/**
**  User preference.
*/
class CPreference {
public:
	CPreference() : ShowSightRange(false), ShowReactionRange(false),
		ShowAttackRange(false), ShowOrders(0) {};

	bool ShowSightRange;     /// Show right range.
	bool ShowReactionRange;  /// Show reaction range.
	bool ShowAttackRange;    /// Show attack range.
	int  ShowOrders;         /// How many second show orders of unit on map.
};

extern CPreference Preference;

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

extern CUnit *Units[MAX_UNIT_SLOTS]; /// Units used
extern int NumUnits;                 /// Number of units used

// in unit_draw.cpp
/// @todo could be moved into the user interface ?
extern unsigned long ShowOrdersCount;   /// Show orders for some time
extern bool EnableBuildingCapture;      /// Config: building capture enabled
extern const CViewport *CurrentViewport;/// CurrentViewport
extern void DrawUnitSelection(const CUnit *);
extern void DrawSelection(Uint32 color, int x1, int y1, int x2, int y2);
extern int MaxSelectable;                   /// How many units could be selected

extern CUnit **Selected;                    /// currently selected units
extern CUnit **TeamSelected[PlayerMax];     /// teams currently selected units
extern int     NumSelected;                 /// how many units selected
extern int     TeamNumSelected[PlayerMax];  /// Number of Units a team member has selected

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

	/// Mark the field with the FieldFlags.
void MarkUnitFieldFlags(const CUnit *unit);
	/// Unmark the field with the FieldFlags.
void UnmarkUnitFieldFlags(const CUnit *unit);
	/// Update unit->CurrentSightRange.
void UpdateUnitSightRange(CUnit *unit);
	/// Create a new unit
extern CUnit *MakeUnit(CUnitType *type, CPlayer *player);
	/// Create a new unit and place on map
extern CUnit *MakeUnitAndPlace(int x, int y, CUnitType *type, CPlayer *player);
	/// Handle the loss of a unit (food,...)
extern void UnitLost(CUnit *unit);
	/// Remove the Orders of a Unit
extern void UnitClearOrders(CUnit *unit);
	/// @todo more docu
extern void UpdateForNewUnit(const CUnit *unit, int upgrade);
	/// @todo more docu
extern void NearestOfUnit(const CUnit *unit, int tx, int ty, int *dx, int *dy);

	/// Call when an Unit goes under fog.
extern void UnitGoesUnderFog(CUnit *unit, const CPlayer *player);
	/// Call when an Unit goes out of fog.
extern void UnitGoesOutOfFog(CUnit *unit, const CPlayer *player);
	/// Marks a unit as seen
extern void UnitsOnTileMarkSeen(const CPlayer *player, int x, int y);
	/// Unmarks a unit as seen
extern void UnitsOnTileUnmarkSeen(const CPlayer *player, int x, int y);
	/// Does a recount for VisCount
extern void UnitCountSeen(CUnit *unit);

	/// Check for rescue each second
extern void RescueUnits(void);

	/// Convert direction (dx,dy) to heading (0-255)
extern int DirectionToHeading(int, int);
	/// Update frame from heading
extern void UnitUpdateHeading(CUnit *unit);
	/// Heading and frame from delta direction x,y
extern void UnitHeadingFromDeltaXY(CUnit *unit, int x, int y);

	/// @todo more docu
extern void DropOutOnSide(CUnit *unit, int heading, int addx, int addy);
	/// @todo more docu
extern void DropOutNearest(CUnit *unit, int x, int y, int addx, int addy);
	/// Drop out all units in the unit
extern void DropOutAll(const CUnit *unit);

	/// Return the rule used to build this building.
extern CBuildRestrictionOnTop *OnTopDetails(const CUnit *unit, const CUnitType *parent);
	/// @todo more docu
extern CUnit *CanBuildHere(const CUnit *unit, const CUnitType *type, int x, int y);
	/// @todo more docu
extern bool CanBuildOn(int x, int y, int mask);
	/// FIXME: more docu
extern CUnit *CanBuildUnitType(const CUnit *unit, const CUnitType *type, int x, int y, int real);

	/// Holds resources
extern bool UnitHoldsResources(const CUnit *unit);
	/// Find resource
extern CUnit *UnitFindResource(const CUnit *unit, int x, int y, int range, int resource = -1);
	/// Find the next idle worker
extern CUnit *FindIdleWorker(const CPlayer *player);

	/// @todo more docu
extern CUnit *UnitOnScreen(int x, int y);
	/// Check if a unit should be removed from UnitsConsumingResources
extern void UnitRemoveConsumingResources(CUnit *unit);

	/// Let a unit die
extern void LetUnitDie(CUnit *unit);
	/// Destory all units inside another unit
extern void DestroyAllInside(CUnit *source);
	/// Hit unit with damage, if destroyed give attacker the points
extern void HitUnit(CUnit *attacker, CUnit *target, int damage);

	/// Returns the map distance between two points
extern int MapDistance(int x1, int y1, int x2, int y2);
	/// Returns the map distance between two points with unit-type
extern int MapDistanceToType(int x1, int y1, const CUnitType *type, int x2, int y2);
	/// Returns the map distance to unit
extern int MapDistanceToUnit(int x, int y, const CUnit *dest);
	/// Returns the map diestance between to unittype as locations
extern int MapDistanceBetweenTypes(const CUnitType *src, int x1, int y1, const CUnitType *dst, int x2, int y2);
	/// Returns the map distance between two units
extern int MapDistanceBetweenUnits(const CUnit *src, const CUnit *dst);

	/// Calculate the distance from current view point to coordinate
extern int ViewPointDistance(int x, int y);
	/// Calculate the distance from current view point to unit
extern int ViewPointDistanceToUnit(const CUnit *dest);

	/// Can this unit-type attack the other (destination)
extern int CanTarget(const CUnitType *type, const CUnitType *dest);
	/// Can transporter transport the other unit
extern int CanTransport(const CUnit *transporter, const CUnit *unit);

	/// Check if unit can move.
extern bool CanMove(const CUnit *unit);

	/// Generate a unit reference, a printable unique string for unit
extern std::string UnitReference(const CUnit *unit);
	/// Save an order
extern void SaveOrder(const COrder *order, CFile *file);
	/// save unit-structure
extern void SaveUnit(const CUnit *unit, CFile *file);
	/// save all units
extern void SaveUnits(CFile *file);

	/// Initialize unit module
extern void InitUnits(void);
	/// Clean unit module
extern void CleanUnits(void);

// in unit_draw.cpp
	/// Register CCL decorations features
extern void DecorationCclRegister(void);
	/// Load the decorations (health,mana) of units
extern void LoadDecorations(void);
	/// Clean the decorations (health,mana) of units
extern void CleanDecorations(void);

	/// Draw unit's shadow
extern void DrawShadow(const CUnit *unit, const CUnitType *type,
	int frame, int x, int y);
	/// Draw all units visible on map in viewport
extern int FindAndSortUnits(const CViewport *vp, CUnit **table, int tablesize);
	/// Show a unit's orders.
extern void ShowOrder(const CUnit *unit);

// in unit_find.cpp
	/// Find all units of this type
extern int FindUnitsByType(const CUnitType *type, CUnit **table, int tablesize);
	/// Find all units of this type of the player
extern int FindPlayerUnitsByType(const CPlayer *, const CUnitType *, CUnit **, int);
	/// Return any unit on that map tile
extern CUnit *UnitOnMapTile(int tx, int ty, unsigned type = (unsigned)-1);
	/// Return possible attack target on that map area
extern CUnit *TargetOnMap(const CUnit *unit, int x1, int y1, int x2, int y2);

	/// Return resource, if on map tile
extern CUnit *ResourceOnMap(int tx, int ty, int resource = -1);

	/// Find best enemy in numeric range to attack
extern CUnit *AttackUnitsInDistance(const CUnit *unit, int range);
	/// Find best enemy in attack range to attack
extern CUnit *AttackUnitsInRange(const CUnit *unit);
	/// Find best enemy in reaction range to attack
extern CUnit *AttackUnitsInReactRange(const CUnit *unit);

// in groups.cpp
	/// Initialize data structures for groups
extern void InitGroups(void);
	/// Save groups
extern void SaveGroups(CFile *file);
	/// Cleanup groups
extern void CleanGroups(void);

	// 2 functions to conseal the groups internal data structures...
	/// Get the number of units in a particular group
extern int GetNumberUnitsOfGroup(int num);
	/// Get the array of units of a particular group
extern CUnit **GetUnitsOfGroup(int num);

	/// Remove all units from a group
extern void ClearGroup(int num);
	/// Add the array of units to the group
extern void AddToGroup(CUnit **units, int nunits, int num);
	/// Set the contents of a particular group with an array of units
extern void SetGroup(CUnit **units, int nunits, int num);
	/// Remove a unit from a group
extern void RemoveUnitFromGroups(CUnit *unit);
	/// Register CCL group features
extern void GroupCclRegister(void);

// in selection.cpp
	/// Check if unit is the currently only selected
#define IsOnlySelected(unit) (NumSelected == 1 && Selected[0] == (unit))

	///  Save selection to restore after.
extern void SaveSelection(void);
	///  Restore selection.
extern void RestoreSelection(void);
	/// Clear current selection
extern void UnSelectAll(void);
	/// Select group as selection
extern void ChangeSelectedUnits(CUnit **units, int num_units);
	/// Changed TeamUnit Selection
extern void ChangeTeamSelectedUnits(CPlayer *player, CUnit **units, int adjust, int count);
	/// Add a unit to selection
extern int SelectUnit(CUnit *unit);
	/// Select one unit as selection
extern void SelectSingleUnit(CUnit *unit);
	/// Remove a unit from selection
extern void UnSelectUnit(CUnit *unit);
	/// Add a unit to selected if not already selected, remove it otherwise
extern int ToggleSelectUnit(CUnit *unit);
	/// Select units from the same type (if selectable by rectangle)
extern int SelectUnitsByType(CUnit *base);
	/// Toggle units from the same type (if selectable by rectangle)
extern int ToggleUnitsByType(CUnit *base);
	/// Select the units belonging to a particular group
extern int SelectGroup(int group_number);
	/// Add the units from the same group as the one in parameter
extern int AddGroupFromUnitToSelection(CUnit *unit);
	/// Select the units from the same group as the one in parameter
extern int SelectGroupFromUnit(CUnit *unit);
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
extern void SaveSelections(CFile *file);
	/// Clean up selections
extern void CleanSelections(void);
	/// Register CCL selection features
extern void SelectionCclRegister(void);

// in script_unit.cpp
	/// Parse order
extern void CclParseOrder(lua_State *l, COrder *order);
	/// register CCL units features
extern void UnitCclRegister(void);

//@}

#endif // !__UNIT_H__
