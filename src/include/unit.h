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
/*
**	(c) Copyright 1998-2000 by Lutz Sammer
**
**	$Id$
*/

#ifndef __UNIT_H__
#define __UNIT_H__

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#ifndef __STRUCT_PLAYER__
#define __STRUCT_PLAYER__
typedef struct _player_ Player;

// #include "player.h"			// recursive!
#endif

#include "video.h"
#include "unittype.h"
#include "upgrade_structs.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

typedef struct _unit_ Unit;		/// unit itself
typedef enum _unit_action_ UnitAction;	/// all possible unit actions
typedef struct _command_ Command;	/// unit command

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
    UnitActionMove,			/// unit moves to position/unit
    UnitActionAttack,			/// unit attacks position/unit
    UnitActionDie,			/// unit dies

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

/**
**	Unit command data structure.
*/
struct _command_ {
    UnitAction	Action : 8;		/// global action
    union {
	// FIXME: will be changed with new pathfinder
	struct {
	    unsigned	Fast : 1;	/// Can fast move
	    unsigned	Range : 31;	/// Range to goal
	    Unit*	Goal;		/// Goal unit
	    unsigned	SX;
	    unsigned	SY;		/// Source
	    unsigned	DX;
	    unsigned	DY;		/// Destination
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
	    Unit*	Peon;		/// Peon/Peasant building the unit
	} Builded;			// builded:
	struct {
	    unsigned	Ticks;		/// Ticks to complete
	    unsigned	Count;		/// Units in training queue
	    // FIXME: cade: later we should train more units or automatic
#define MAX_UNIT_TRAIN	6
	    UnitType*	What[MAX_UNIT_TRAIN];	/// Unit trained
	} Train;			/// train:
	struct {
	    unsigned	Ticks;		/// Ticks to complete
	    UnitType*	What;		/// Unit upgraded to
	} UpgradeTo;			/// upgradeto:
	struct {
	    unsigned	Ticks;		/// Ticks to complete
	    int		What;		/// Unit researching this
	} Research;			/// research:
	struct {
	    unsigned	Active;		/// how much units are in the goldmine
	} GoldMine;			/// gold-mine:
	struct {
	    unsigned	Active;		/// how much units are in the goldmine
	} OilWell;			/// oil-well
    } Data;				/// data for action
};

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
**	The big unit structure.
*/
struct _unit_ {
#ifdef NEW_UNIT
    short	Refs;			/// Reference counter
    UnitRef	Slot;			/// Assignd slot number
    UnitRef	UnitSlot;		/// slot number in Units
    UnitRef	PlayerSlot;		/// slot number in Player->Units
    Unit*	Next;			/// generic link pointer
#else
    // FIXME: this will be removed
    unsigned	Id;			/// unique unit id
#endif

    int		X;			/// Map position X
    int		Y;			/// Map position Y

    UnitType*	Type;			/// pointer to unit type (peon,...)
    Player*     Player;			/// owner of this unit
    UnitStats*	Stats;			/// current unit stats
    
//	DISPLAY:
    char	IX;
    char	IY;			/// image displacement to map position
    unsigned	Frame : 8;		/// Image frame: high bit used for flip
    unsigned   	SeenFrame : 8;		/// last seen frame/stage of buildings

    unsigned	Heading : 8;		/// direction of unit looking

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
    unsigned	Haste;			/// ticks haste
    unsigned	Slow;			/// ticks slow
    unsigned	Invisible;		/// ticks invisible
    unsigned	Shield;			/// ticks shield

    unsigned	GroupId;		/// unit belongs to this group id

    unsigned	SubAction : 8;		/// sub-action of unit
    unsigned	Wait : 8;		/// action counter
#define UNIT_MAX_WAIT	255		/// biggest number in action counter
    unsigned	State : 8; 		/// action state
#define UNIT_MAX_STATE	255		/// biggest state for action
    unsigned	Reset : 1;		/// can process new command
    unsigned	Blink : 3;		/// Let selection rectangle blink
    unsigned	Moving : 1;		/// The unit is moving
					/** set to random 1..100 when MakeUnit()
					** ,used for fancy buildings
					*/
    unsigned 	Rs : 8;

    unsigned	Value;			/// value used for much
    unsigned	WoodToHarvest;
#define MAX_UNITS_ONBOARD 6		/// max number of units in transporter
    // FIXME: use the new next pointer
    Unit*	OnBoard[MAX_UNITS_ONBOARD];	/// Units in transporter

    union _command_data_ {
	struct _command_move_ {
#define MAX_PATH_LENGTH	15		/// max length of precalculated path
	    unsigned char Length;	/// stored path length
					/// stored path directions
	    unsigned char Path[MAX_PATH_LENGTH];
	}	Move;			/// for command move
    }		Data;			/// Storage room for different commands

#define MAX_COMMANDS 16			/// max number of outstanding commands
//	NEW-ACTIONS:
    Command	Command;		/// current command processed
    Command	SavedCommand;		/// saved command
    Command	NextCommand[MAX_COMMANDS];/// next command to process
    int		NextCount;		/// how many commands are in the queue
    int		NextFlush;	/// true: cancel command and proceed to next one
    Command	PendCommand;		/// pending commands
};

//		N
//	NW		NE
//	W		 E
//	SW		SE
//		S
// FIXME: this heading should be changed see tasks.txt
#define HeadingN		0	/// Unit heading north
#define HeadingNE		1	/// Unit heading north east
#define HeadingE		2	/// Unit heading east
#define HeadingSE		3	/// Unit heading south east
#define HeadingS		4	/// Unit heading south
#define HeadingSW		5	/// Unit heading south west
#define HeadingW		6	/// Unit heading west
#define HeadingNW		7	/// Unit heading north west

#define NoUnitP		(Unit*)0	/// return value: for no unit found
#define InfiniteDistance INT_MAX	/// the distance is unreachable

// FIXME: will be removed, we get player limits
#define MAX_UNITS	1800		/// maximal number of units supported

/**
**	Maximal number of used slots.
*/
#define MAX_UNIT_SLOTS	65535

/**
**	Returns true, if unit is unusable. (for attacking,...)
*/
#define UnitUnusable(unit) \
    ( (unit)->Removed || (unit)->Command.Action==UnitActionDie || \
      (unit)->Command.Action==UnitActionBuilded)

/**
**	Returns unit number (unique to this unit)
*/
#ifdef NEW_UNIT
#define UnitNumber(unit)	((unit)->Slot)
#else
#define UnitNumber(unit)	((unit)-UnitsPool)
#endif

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

#ifdef NEW_UNIT
extern Unit* UnitSlots[MAX_UNIT_SLOTS];	/// All possible units
extern Unit** UnitSlotFree; 		/// First free unit slot

extern Unit* Units[MAX_UNIT_SLOTS];	/// Units used
extern int NumUnits;			/// Number of units used
#else
extern int NumUnits;			/// Number of units used
extern Unit** Units;			/// Units used
extern Unit* UnitsPool;			/// Units memory pool
#endif

//	in unit_draw.c (FIXME: could be moved into the user interface?)
extern int ShowHealthBar;		/// Flag: show health bar
extern int ShowHealthDot;		/// Flag: show health dot
extern int ShowManaBar;			/// Flag: show mana bar
extern int ShowManaDot;			/// Flag: show mana dot
extern int ShowNoFull;			/// Flag: show no full health or mana
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
    ///	Create a new unit
extern Unit* MakeUnit(UnitType* type,Player* player);
    ///	Create a new unit and place on map
extern Unit* MakeUnitAndPlace(int x,int y,UnitType* type,Player* player);

    /// FIXME: more docu
extern void UnitLost(const Unit* unit);
extern void UpdateForNewUnit(const Unit* unit,int upgrade);
extern void NearestOfUnit(const Unit* unit,int tx,int ty,int *dx,int *dy);
extern int UnitVisible(const Unit* unit);
extern void RemoveUnit(Unit* unit);
extern void UnitIncrementMana(void);
extern void ChangeUnitOwner(Unit* unit,Player* old,Player* new);

extern void UnitNewHeading(Unit* unit);
extern void UnitNewHeadingFromXY(Unit* unit,int x,int y);
extern int HeadingFromXY2XY(int x,int y,int dx,int dy);

extern void DropOutOnSide(Unit* unit,int heading,int addx,int addy);
extern void DropOutNearest(Unit* unit,int x,int y,int addx,int addy);
extern void DropOutAll(const Unit* unit);

extern int CanBuildHere(UnitType* type,unsigned x,unsigned y);
extern int CanBuildOn(int x,int y,int mask);
extern int CanBuildUnitType(Unit* unit,UnitType* type,int x,int y);

extern Unit* FindGoldMine(int x,int y);
extern Unit* GoldDepositOnMap(int tx,int ty);
extern Unit* FindGoldDeposit(const Player* player,int x,int y);

extern Unit* WoodDepositOnMap(int tx,int ty);
extern Unit* FindWoodDeposit(const Player* player,int x,int y);

extern int FindWoodInSight(Unit* unit,int* x,int* y);
extern Unit* FindOilDeposit(const Player* player,int x,int y);
extern Unit* FindOilPlatform(const Player* player,int x,int y);

extern Unit* UnitOnScreen(Unit* unit,unsigned x,unsigned y);

extern void DestroyUnit(Unit* unit);
extern void DestroyAllInside(Unit* source);
extern void HitUnit(Unit* unit,int damage);

extern int MapDistance(int x1,int y1,int x2,int y2);
extern int MapDistanceToType(int x1,int y1,const UnitType* type,int x2,int y2);
extern int MapDistanceToUnit(int x,int y,const Unit* dest);

extern int ViewPointDistance(int x,int y);
extern int ViewPointDistanceToUnit(Unit* dest);

extern int IsEnemy(const Player* player,const Unit* dest);
extern int CanTarget(const UnitType* type,const UnitType* dest);

extern void UnitConflicts(void);

extern void SaveUnit(const Unit* unit,FILE* file);	/// save unit-structure
extern void SaveUnits(FILE* file);			/// save all units

//	in unitcache.c
extern void UnitCacheInsert(Unit* unit);
extern void UnitCacheRemove(Unit* unit);
extern void UnitCacheChange(Unit* unit);
extern int UnitCacheSelect(int x1,int y1,int x2,int y2,Unit** table);
extern Unit* UnitCacheOnXY(int x,int y,int type);
extern void UnitCacheStatistic(void);
extern void InitUnitCache(void);

// 	in map.c 	belongs to map or unit??
extern int UnitMovement(const Unit* unit);
extern unsigned UnitFieldFlags(const Unit* unit);
extern int TypeMovementMask(const UnitType* type);
extern int UnitMovementMask(const Unit* unit);

//	in bottom_panel.c
extern void UpgradeButtons(int upgrade);

//	in unit_draw.c
extern void LoadDecorations(void);
extern void DrawUnits(void);

//	in unit_find.c
extern int SelectUnits(int x1,int y1,int x2,int y2,Unit** table);
extern int FindUnitsByType(int type,Unit** table);
extern int FindPlayerUnitsByType(const Player* player,int type,Unit** table);
extern Unit* UnitOnMapTile(unsigned tx,unsigned ty);
extern Unit* TargetOnMapTile(Unit* unit,unsigned tx,unsigned ty);

extern Unit* GoldMineOnMap(int tx,int ty);
extern Unit* OilPatchOnMap(int tx,int ty);
extern Unit* PlatformOnMap(int tx,int ty);
extern Unit* OilDepositOnMap(int tx,int ty);

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

    /// 2 functions to conseal the groups internal data structures...
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

#endif // !__UNIT_H__
