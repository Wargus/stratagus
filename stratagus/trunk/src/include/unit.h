//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name unit.h		-	The unit headerfile. */
//
//	(c) Copyright 1998-2000 by Lutz Sammer
//
//	$Id$

#ifndef __UNIT_H__
#define __UNIT_H__

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#ifndef __STRUCT_PLAYER__
#define __STRUCT_PLAYER__
typedef struct _player_ Player;		// recursive includes :(
#endif

#include "video.h"
#include "unittype.h"
#include "upgrade_structs.h"
#include "upgrade.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

typedef struct _unit_ Unit;		/// unit itself
typedef enum _unit_action_ UnitAction;	/// all possible unit actions
#ifndef NEW_ORDERS
typedef struct _command_ Command;	/// unit command
#endif

/**
**	Unit references over network, or for memory saving.
*/
typedef unsigned short UnitRef;

/**
**	All possible unit actions.
*/
enum _unit_action_ {
    UnitActionNone,			/// No valid action

    UnitActionStill,			/// unit stand still, does nothing
    UnitActionStandGround,		/// unit stands ground
    UnitActionFollow,			/// unit follows units
    UnitActionMove,			/// unit moves to position/unit
    UnitActionAttack,			/// unit attacks position/unit
    UnitActionAttackGround,		/// unit attacks ground
    UnitActionDie,			/// unit dies

    UnitActionSpellCast,		/// unit casts spell

    UnitActionTrain,			/// building is training
    UnitActionUpgradeTo,		/// building is upgrading itself
    // UnitActionUpgrade,		/// building is researching upgrade
    UnitActionResearch,			/// building is researching spell
    UnitActionBuilded,			/// building is under construction

// Compound actions
    UnitActionBoard,			/// unit entering transporter
    UnitActionUnload,			/// unit leaving transporter
    UnitActionPatrol,			/// unit paroling area
    UnitActionBuild,			/// unit builds building

    UnitActionRepair,			/// unit repairing
    UnitActionHarvest,			/// unit harvest lumber
    UnitActionMineGold,			/// unit mines gold
    UnitActionMineOre,			/// unit mines ore FIXME: not written
    UnitActionMineCoal,			/// unit mines coal FIXME: not written
    UnitActionQuarryStone,		/// unit quarrying stone FIXME: not written
    UnitActionHaulOil,			/// unit hauling oil
    UnitActionReturnGoods,		/// unit returning any resource

    UnitActionDemolish,			/// unit demolish at position/unit
};

#ifdef NEW_ORDERS
/**
**	Unit order structure.
*/
typedef struct _order_ {
    UnitAction		Action : 8;	/// global action
    unsigned char	Flags;		/// Order flags (unused)
    unsigned char	RangeX;		/// How near in X direction
    unsigned char	RangeY;		/// How near in Y direction

    Unit*		Goal;		/// goal of the order (if any)
    int			X;		/// or X tile coordinate of destination
    int			Y;		/// or Y tile coordinate of destination
    UnitType*		Type;		/// Unit-type argument

    void*		Arg1;		/// Extra command argument
} Order;

#else
/**
**	Unit command data structure.
*/
struct _command_ {
    UnitAction	Action : 8;		/// global action
    union {
	// FIXME: will be changed with new pathfinder
	/*
	** FIXME: rewrite this complete
	struct {
	    unsigned	RangeX;		/// Range to goal in X
	    unsigned	RangeY;		/// Range to goal in Y
	    Unit*	Goal;		/// Goal unit
	    unsigned	DX;
	    unsigned	DY;		/// Destination
	} Move;				/// move:
	*/
	struct {
	    unsigned	Fast : 1;	/// Can fast move
	    unsigned	Range : 31;	/// Range to goal
	    Unit*	Goal;		/// Goal unit
	    unsigned	SX;
	    unsigned	SY;		/// Source
	    unsigned	DX;
	    unsigned	DY;		/// Destination
	    int         SpellId;	/// spell type id
	} Move;				/// move:
	struct {
	    unsigned	Fast : 1;	/// Can fast move
	    unsigned	Range : 31;	/// Range to goal
	    Unit*	Goal;		/// Goal unit
	    unsigned	SX;
	    unsigned	SY;		/// Source
	    unsigned	DX;
	    unsigned	DY;		/// Destination
	    UnitType*	BuildThis;	/// Unit to build
	} Build;			/// build:
	struct {
	    int		Sum;		/// HP for building
	    int		Add;
	    int		Val;		/// Counter
	    int		Sub;
	    int		Cancel;		/// Cancel construction
	    Unit*	Worker;		/// Worker building the unit
	} Builded;			// builded:
	struct {
	    unsigned	Ticks;		/// Ticks to complete
	    unsigned	Count;		/// Units in training queue
	    // FIXME: vladi: later we should train more units or automatic
#define MAX_UNIT_TRAIN	6
	    UnitType*	What[MAX_UNIT_TRAIN];	/// Unit trained
	} Train;			/// train:
	struct {
	    unsigned	Ticks;		/// Ticks to complete
	    UnitType*	What;		/// Unit upgraded to
	} UpgradeTo;			/// upgradeto:
	struct {
	    unsigned	Ticks;		/// Ticks to complete
	    Upgrade*	What;		/// Unit researching this
	} Research;			/// research:
	struct {
	    unsigned	Active;		/// how much units are in the goldmine
	} GoldMine;			/// gold-mine:
	struct {
	    unsigned	Active;		/// how much units are in the goldmine
	} OilWell;			/// oil-well
    } Data;				/// data for action
};
#endif

/**
**	Voice groups for an unit
*/
typedef enum _unit_voice_group_ {
    VoiceSelected,			/// If selected
    VoiceAcknowledging,			/// Acknowledge command
    VoiceAttacking,			/// FIXME: Should be removed?
    VoiceReady,				/// Command completed
    VoiceHelpMe,			/// If attacked
    VoiceDying,				/// If killed
    VoiceWorkCompleted,			/// only worker, work completed
    VoiceBuilding,			/// only for building under construction
    VoiceDocking,			/// only for transport reaching coast
    VoiceTreeChopping			/// only worker
} UnitVoiceGroup;

/**
**	Unit/Missile headings.
**		N
**	NW		NE
**	W		 E
**	SW		SE
**		S
*/
enum _directions_ {
    LookingN	=0*32,			/// Unit looking north
    LookingNE	=1*32,			/// Unit looking north east
    LookingE	=2*32,			/// Unit looking east
    LookingSE	=3*32,			/// Unit looking south east
    LookingS	=4*32,			/// Unit looking south
    LookingSW	=5*32,			/// Unit looking south west
    LookingW	=6*32,			/// Unit looking west
    LookingNW	=7*32,			/// Unit looking north west
};

#define NextDirection	32		/// Next direction N->NE->E...

/**
**	The big unit structure.
**
**	Everything belonging to an unit. FIXME: rearrange for less memory.
*/
struct _unit_ {
    // int is faster than shorts.
    unsigned	Refs;			/// Reference counter
    unsigned	Slot;			/// Assignd slot number
    Unit**	UnitSlot;		/// slot pointer of Units
    Unit**	PlayerSlot;		/// slot pointer of Player->Units
    Unit*	Next;			/// generic link pointer (on map)

    int		X;			/// Map position X
    int		Y;			/// Map position Y

    UnitType*	Type;			/// pointer to unit type (peon,...)
    Player*     Player;			/// owner of this unit
    UnitStats*	Stats;			/// current unit stats
 
//	DISPLAY:
    char	IX;			/// X image displacement to map position
    char	IY;			/// Y image displacement to map position
    unsigned	Frame : 8;		/// Image frame: high bit used for flip
    unsigned   	SeenFrame : 8;		/// last seen frame/stage of buildings

    unsigned	Direction : 8;		/// angle (0-255) unit looking

    unsigned	Burning : 1;		/// unit is burning
    unsigned	Attacked : 1;		/// unit is attacked
    // FIXME: next not used!
    //unsigned	Visible : 1;		/// unit is visible (submarine)
    unsigned	Destroyed : 1;		/// unit is destroyed pending reference
    unsigned	Removed : 1;		/// unit is removed (not on map)
    unsigned	Selected : 1;		/// unit is selected
    unsigned	Constructed : 1;	/// unit is in construction

    unsigned	Mana : 8;		/// mana points
    unsigned	HP;			/// hit points

    unsigned	Bloodlust;		/// ticks bloodlust
    unsigned	Haste;			/// ticks haste (disables slow)
    unsigned	Slow;			/// ticks slow (disables haste)
    unsigned	Invisible;		/// ticks invisible
    unsigned	FlameShield;		/// ticks flame shield
    unsigned	UnholyArmor;		/// ticks unholy armor

    unsigned	GroupId;		/// unit belongs to this group id

    unsigned	Value;			/// value used for much

    unsigned	SubAction : 8;		/// sub-action of unit
    unsigned	Wait : 8;		/// action counter
#define MAX_UNIT_WAIT	255		/// biggest number in action counter
    unsigned	State : 8; 		/// action state
#define MAX_UNIT_STATE	255		/// biggest state for action
    unsigned	Reset : 1;		/// can process new command
    unsigned	Blink : 3;		/// Let selection rectangle blink
    unsigned	Moving : 1;		/// The unit is moving
					/** set to random 1..100 when MakeUnit()
					** ,used for fancy buildings
					*/
    unsigned 	Rs : 8;
    unsigned	Revealer;               // hack -- `revealer' is unit that
                                        // has to keep FOW revealed for some
					// time, this unit cannot be used in
					// usual manner

#define MAX_UNITS_ONBOARD 6		/// max number of units in transporter
    // FIXME: use the new next pointer
    Unit*	OnBoard[MAX_UNITS_ONBOARD];	/// Units in transporter

#ifdef NEW_ORDERS	//---------------------------------------------

#define MAX_ORDERS 16			/// How many outstanding orders?
    char	OrderCount;		/// how many orders in queue
    char	OrderFlush;		/// cancel current order, take next
    Order	Orders[MAX_ORDERS];	/// orders to process
    Order	SavedOrder;		/// order to continue after current
    Order	NewOrder;		/// order for new trained units.

    union _order_data_ {
    struct _order_move_ {
	char	Fast;			/// Flag fast move (one step)
	char	Length;			/// stored path length
#define MAX_PATH_LENGTH	14		/// max length of precalculated path
	char	Path[MAX_PATH_LENGTH];	/// directions of stored path
    }		Move;			/// ActionMove,...
    struct _order_builded_ {
	Unit*	Worker;			/// Worker building this unit.
	int	Sum;			/// HP for building
	int	Add;
	int	Val;			/// Counter
	int	Sub;
	int	Cancel;			/// Cancel construction
    }		Builded;		/// ActionBuilded,...
    }		Data;			/// Storage room for different commands

#else			//---------------------------------------------

    unsigned	WoodToHarvest;		/// Ticks for harvest

#define MAX_COMMANDS 16			/// max number of outstanding commands
//	NEW-ACTIONS:
    Command	Command;		/// current command processed
    Command	SavedCommand;		/// saved command
    Command	NextCommand[MAX_COMMANDS];/// next command to process
    char	NextCount;		/// how many commands are in the queue
    char	NextFlush;	/// true: cancel command and proceed to next one
    Command	PendCommand;		/// pending commands
#endif			//---------------------------------------------
};

#define NoUnitP		(Unit*)0	/// return value: for no unit found
#define InfiniteDistance INT_MAX	/// the distance is unreachable

#define FlushCommands	1		/// Flush commands in queue

// FIXME: will be removed, we get player limits
#define MAX_UNITS	1800		/// maximal number of units supported

/**
**	Maximal number of used slots.
*/
#define MAX_UNIT_SLOTS	65535

#ifdef NEW_ORDERS
/**
**	Returns true, if unit is unusable. (for attacking,...)
*/
#define UnitUnusable(unit) \
    ( (unit)->Removed || (unit)->Orders[0].Action==UnitActionDie || \
      (unit)->Orders[0].Action==UnitActionBuilded)
#else
/**
**	Returns true, if unit is unusable. (for attacking,...)
*/
#define UnitUnusable(unit) \
    ( (unit)->Removed || (unit)->Command.Action==UnitActionDie || \
      (unit)->Command.Action==UnitActionBuilded)
#endif

/**
**	Returns unit number (unique to this unit)
*/
#define UnitNumber(unit)	((unit)->Slot)

/**
**	How many units could be selected
*/
#define MaxSelectable	9

// FIXME: hardcoded...
/**
**	How many units could be in a group
*/
#define NUM_UNITS_PER_GROUP 9
/**
**	How many groups supported
*/
#define NUM_GROUPS 10

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern Unit* UnitSlots[MAX_UNIT_SLOTS];	/// All possible units
extern Unit** UnitSlotFree; 		/// First free unit slot

extern Unit* Units[MAX_UNIT_SLOTS];	/// Units used
extern int NumUnits;			/// Number of units used

//	in unit_draw.c (FIXME: could be moved into the user interface?)
extern int ShowHealthBar;		/// Flag: show health bar
extern int ShowHealthDot;		/// Flag: show health dot
extern int ShowManaBar;			/// Flag: show mana bar
extern int ShowManaDot;			/// Flag: show mana dot
extern int ShowHealthHorizontal;	/// Flag: show health bar horizontal
extern int ShowManaHorizontal;		/// Flag: show mana bar horizontal
extern int ShowNoFull;			/// Flag: show no full health or mana
extern int ShowEnergySelectedOnly;	/// Flag: show energy only for selected
extern int DecorationOnTop;		/// Flag: show health and mana on top
extern int ShowSightRange;		/// Flag: show right range
extern int ShowReactRange;		/// Flag: show react range
extern int ShowAttackRange;		/// Flag: show attack range
extern int ShowOrders;			/// Flag: show orders of unit on map

//	in selection.c
extern int NumSelected;			/// how many units selected
extern Unit* Selected[MaxSelectable];	/// currently selected units

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// Prepare unit memory allocator
extern void InitUnitsMemory(void);
    /// Free memory used by unit
extern void FreeUnitMemory(Unit* unit);
    /// Release an unit.
extern void ReleaseUnit(Unit* unit);
    ///	Create a new unit
extern Unit* MakeUnit(UnitType* type,Player* player);
    ///	Create a new unit and place on map
extern Unit* MakeUnitAndPlace(int x,int y,UnitType* type,Player* player);

    /// FIXME: more docu
extern void UnitLost(const Unit* unit);
    /// FIXME: more docu
extern void UpdateForNewUnit(const Unit* unit,int upgrade);
    /// FIXME: more docu
extern void NearestOfUnit(const Unit* unit,int tx,int ty,int *dx,int *dy);
    /// FIXME: more docu
extern int UnitVisible(const Unit* unit);
    /// FIXME: more docu
extern void RemoveUnit(Unit* unit);
    /// Increment mana of all magic units each second.
extern void UnitIncrementMana(void);
    /// Increment health of all regenerating units each second.
extern void UnitIncrementHealth(void);
    /// Check for rescue each second.
extern void RescueUnits(void);
    /// Change owner of unit.
extern void ChangeUnitOwner(Unit* unit,Player* old,Player* new);

    /// Convert direction (dx,dy) to heading (0-255)
extern int DirectionToHeading(int,int);
    /// Update frame from heading
extern void UnitUpdateHeading(Unit* unit);
    /// Heading and frame from delta direction x,y
extern void UnitHeadingFromDeltaXY(Unit* unit,int x,int y);

    /// FIXME: more docu
extern void DropOutOnSide(Unit* unit,int heading,int addx,int addy);
    /// FIXME: more docu
extern void DropOutNearest(Unit* unit,int x,int y,int addx,int addy);
    /// FIXME: more docu
extern void DropOutAll(const Unit* unit);

    /// FIXME: more docu
extern int CanBuildHere(UnitType* type,unsigned x,unsigned y);
    /// FIXME: more docu
extern int CanBuildOn(int x,int y,int mask);
    /// FIXME: more docu
extern int CanBuildUnitType(Unit* unit,UnitType* type,int x,int y);

    /// Find nearest gold mine
extern Unit* FindGoldMine(const Unit*,int,int);
    /// Find nearest gold deposit
extern Unit* FindGoldDeposit(const Unit*,int,int);
    /// Find nearest wood deposit
extern Unit* FindWoodDeposit(const Player* player,int x,int y);
    /// Find nearest oil deposit
extern Unit* FindOilDeposit(const Player* player,int x,int y);

    /// FIXME: more docu
extern int FindWoodInSight(Unit* unit,int* x,int* y);
    /// FIXME: more docu
extern Unit* FindOilPlatform(const Player* player,int x,int y);

    /// FIXME: more docu
extern Unit* UnitOnScreen(Unit* unit,unsigned x,unsigned y);

    /// FIXME: more docu
extern void DestroyUnit(Unit* unit);
    /// FIXME: more docu
extern void DestroyAllInside(Unit* source);
    /// FIXME: more docu
extern void HitUnit(Unit* unit,int damage);

    /// FIXME: more docu
extern int MapDistance(int x1,int y1,int x2,int y2);
    /// FIXME: more docu
extern int MapDistanceToType(int x1,int y1,const UnitType* type,int x2,int y2);
    /// FIXME: more docu
extern int MapDistanceToUnit(int x,int y,const Unit* dest);

    /// FIXME: more docu
extern int ViewPointDistance(int x,int y);
    /// FIXME: more docu
extern int ViewPointDistanceToUnit(Unit* dest);

    /// Return true, if unit is an enemy of the player
extern int IsEnemy(const Player* player,const Unit* dest);
    /// Return true, if unit is allied with the player
extern int IsAllied(const Player* player,const Unit* dest);
    /// FIXME: more docu
extern int CanTarget(const UnitType* type,const UnitType* dest);

extern void SaveUnit(const Unit* unit,FILE* file);	/// save unit-structure
extern void SaveUnits(FILE* file);			/// save all units

//	in unitcache.c
    /// Insert new unit into cache.
extern void UnitCacheInsert(Unit* unit);
    /// Remove unit from cache.
extern void UnitCacheRemove(Unit* unit);
    /// Change unit position in cache.
extern void UnitCacheChange(Unit* unit);
    /// Select units in range.
extern int UnitCacheSelect(int x1,int y1,int x2,int y2,Unit** table);
    /// Select units on tile.
extern int UnitCacheOnTile(int x,int y,Unit** table);
    /// Select unit on X,Y of type naval,fly,land.
extern Unit* UnitCacheOnXY(int x,int y,int type);
    /// Print unit-cache statistic.
extern void UnitCacheStatistic(void);
    /// Initialize unit-cache.
extern void InitUnitCache(void);

// 	in map.c 	belongs to map or unit??
    /// FIXME: more docu
extern int UnitMovement(const Unit* unit);
    /// FIXME: more docu
extern unsigned UnitFieldFlags(const Unit* unit);
    /// FIXME: more docu
extern int TypeMovementMask(const UnitType* type);
    /// FIXME: more docu
extern int UnitMovementMask(const Unit* unit);

//	in bottom_panel.c
    /// FIXME: more docu
extern void UpgradeButtons(int upgrade);

//	in unit_draw.c
    /// FIXME: more docu
extern void LoadDecorations(void);
    /// FIXME: more docu
extern void DrawUnits(void);

//	in unit_find.c
    /// Select units in rectangle range.
extern int SelectUnits(int x1,int y1,int x2,int y2,Unit** table);
    /// Select units on map tile.
extern int SelectUnitsOnTile(int x,int y,Unit** table);

    /// Find all units of this type
extern int FindUnitsByType(const UnitType* type,Unit** table);
    /// Find all units of this type of the player
extern int FindPlayerUnitsByType(const Player*,const UnitType*,Unit**);
    /// Return any unit on that map tile
extern Unit* UnitOnMapTile(unsigned tx,unsigned ty);
    /// Return repairable unit on that map tile
extern Unit* RepairableOnMapTile(unsigned tx,unsigned ty);
    /// Return possible attack target on that map tile
extern Unit* TargetOnMapTile(Unit* unit,unsigned tx,unsigned ty);

    /// Return gold mine, if on map tile
extern Unit* GoldMineOnMap(int tx,int ty);
    /// Return gold deposit, if on map tile
extern Unit* GoldDepositOnMap(int tx,int ty);
    /// Return oil patch, if on map tile
extern Unit* OilPatchOnMap(int tx,int ty);
    /// Return oil platform, if on map tile
extern Unit* PlatformOnMap(int tx,int ty);
    /// Return oil deposit, if on map tile
extern Unit* OilDepositOnMap(int tx,int ty);
    /// Return wood deposit, if on map tile
extern Unit* WoodDepositOnMap(int tx,int ty);

    /// Find any enemy in numeric range
extern Unit* EnemyInRage(const Unit* unit,unsigned range);
    /// Find best enemy in numeric range to attack
extern Unit* AttackUnitsInDistance(const Unit* unit,unsigned range);
    /// Find best enemy in attack range to attack
extern Unit* AttackUnitsInRange(const Unit* unit);
    /// Find best enemy in reaction range to attack
extern Unit* AttackUnitsInReactRange(const Unit* unit);

//      in groups.c

    /// Initialize data structures for groups
extern void InitGroups(void);

    // 2 functions to conseal the groups internal data structures...
    /// Get the number of units in a particular group.
extern int GetNumberUnitsOfGroup(int num);
    /// Get the array of units of a particular group.
extern Unit** GetUnitsOfGroup(int num);

    /// Remove all units from a group.
extern void ClearGroup(int num);
    /// Set the contents of a particular group with an array of units.
extern void SetGroup(Unit **units,int nunits,int num);
    /// Remove a unit from a group.
extern void RemoveUnitFromGroup(Unit *unit);

//	in selection.c

    /// Check if unit is the currently only selected
// FIXME: lokh: Is it necessary to check if NumSelected==1?
//              Maybe we can add a #define IsOnlySelected(unit)?
#define IsSelected(unit)	(NumSelected==1 && Selected[0]==(unit))

    /// Clear current selection
extern void UnSelectAll(void);
    /// Select group as selection
extern void ChangeSelectedUnits(Unit** units,int num_units);
    /// Add a unit to selection
extern int SelectUnit(Unit* unit);
    /// Select one unit as selection
extern void SelectSingleUnit(Unit* unit);
    /// Remove a unit from selection
extern void UnSelectUnit(Unit* unit);
    /// Add a unit to selected if not already selected, remove it otherwise.
extern int ToggleSelectUnit(Unit* unit);
    /// Select units from the same type (if selectable by rectangle)
extern int SelectUnitsByType(Unit* base);
    /// Select the units belonging to a particular group
extern int SelectGroup(int group_number);
    /// Select the unit from the same group as the one in parameter
extern int SelectGroupFromUnit(Unit *unit);
    /// Add the units in the selection rectangle to the current selection
extern int AddSelectedUnitsInRectangle(int tx,int ty,int w,int h);
    /// Select the units in the selection rectangle
extern int SelectUnitsInRectangle(int tx,int ty,int w,int h);

//@}

#endif	// !__UNIT_H__
