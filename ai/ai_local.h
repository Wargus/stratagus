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
/**@name ai_local.h	-	The local AI header file. */
/*
**      (c) Copyright 2000 by Lutz Sammer
**
**      $Id$
*/

#ifndef __AI_LOCAL_H__
#define __AI_LOCAL_H__

#ifdef NEW_AI

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "ccl.h"
#include "player.h"
#include "unittype.h"
#include "upgrade.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

#if 0

/**
**	Ai Script commands.
*/
enum _ai_script_command_ {
    AiCmdNeed,				/// need building/unit
    AiCmdBuild,				/// build building
    AiCmdTrain,				/// train unit
    AiCmdResearch,			/// research upgrade
    AiCmdForce,				/// Set force
};

/**
**	Ai Script.
*/
typedef struct _ai_script_ {
    unsigned char	Cmd;		/// command
    unsigned char	Arg;		/// argument
    unsigned char	Cnt;		/// counter
} AiScript;

/**
**	AI goal typedef.
*/
typedef struct _ai_goal_ AiGoal;

/**
**	AI Priority, will later be finer tuned.
*/
enum _ai_priority_ {
    AiPriorityVeryLow,			/// very low
    AiPriorityLow,			/// low
    AiPriorityMid,			/// middle
    AiPriorityHigh,			/// high
    AiPriorityVeryHigh,			/// very high
};

/**
**	Define the AI goals.
*/
struct _ai_goal_ {
    AiGoal*	Next;			/// double linked list of all goals
    AiGoal*	Down;			/// dependend goals
    AiGoal*	Prev;			/// double linked list of all goals
    int		Priority;		/// Priority of this goal
};

    // goals stuff
    AiGoal*     GoalHead;               /// Goals start of double linked list
    AiGoal*     GoalNil1;               /// Goals dummy end of dl-list
    AiGoal*     GoalTail;               /// Goals end of double linked list

    AiGoal*     WaitHead;               /// Wait start of double linked list
    AiGoal*     WaitNil1;               /// Wait dummy end of dl-list
    AiGoal*     WaitTail;               /// Wait end of double linked list

    // scripting stuff
    AiScript*	Ip;			/// AI script instruction pointer
    AiScript**	Sp;			/// Ai script stack pointer
    AiScript**	Stack;			/// Ai script stack

#endif	// --------------------------------------------------------------------

/**
**	Ai Type typedef.
*/
typedef struct _ai_type_ AiType;

/**
**	Ai Type structure.
*/
struct _ai_type_ {
    AiType*		Next;			/// Next ai type

    char*		Name;			/// Name of this ai
    char*		Race;			/// for this race
    char*		Class;			/// class of this ai

    // nice flags
    //unsigned char	AllExplored : 1;	/// Ai sees unexplored area
    //unsigned char	AllVisibile : 1;	/// Ai sees invisibile area

    SCM			Script;			/// Main script
};

/**
**	AI unit-type table with counter in front.
*/
typedef struct _ai_unittype_table_ {
    int		Count;			/// elements in table
    UnitType*	Table[1];		/// the table
} AiUnitTypeTable;

/**
**	Ai unit-type typedef.
*/
typedef struct _ai_unit_type_ AiUnitType;

/**
**	Ai unit-type in a force.
*/
struct _ai_unit_type_ {
    AiUnitType*	Next;			/// next unit-type
    int		Want;			/// number of this unit-type wanted
    UnitType*	Type;			/// unit-type self
};

/**
**	Ai unit typedef.
*/
typedef struct _ai_unit_ AiUnit;

/**
**	Ai unit in a force.
*/
struct _ai_unit_ {
    AiUnit*	Next;			/// next unit
    Unit*	Unit;			/// unit self
};

/**
**	AI force typedef.
*/
typedef struct _ai_force_ AiForce;

/**
**	Define an AI force.
**
**	A force is a group of units belonging together.
*/
struct _ai_force_ {
    int			Completed;	/// Flag saying force is complete build
    int			Defending;	/// Flag saying force is defending
    int			Attacking;	/// Flag saying force is attacking
    AiUnitType*		UnitTypes;	/// Count and types of unit-type
    AiUnit*		Units;		/// Units in the force
};

/**
**	AI build queue typedef.
*/
typedef struct _ai_build_queue_ AiBuildQueue;

/**
**	AI build queue.
**
**	List of orders for the resource manager to handle
*/
struct _ai_build_queue_ {
	AiBuildQueue*	Next;		/// next request
	int		Want;		/// requested number
	int		Made;		/// builded number
	UnitType*	Type;		/// unit-type
};

/**
**	AI variables.
*/
typedef struct _player_ai_ {
    Player*     Player;                 /// Engine player structure

    AiType*	AiType;			/// AI type of this player AI

    // controller
    SCM		Script;			/// Script executed
    int		ScriptDebug;		/// Flag script debuging on/off

    // forces
#define AI_MAX_FORCES	10		/// How many forces are supported
    AiForce	Force[AI_MAX_FORCES];	/// Forces controlled by AI

    // resource manager

    int		Reserve[MaxCosts];	/// Resources to keep in reserve
    int		Used[MaxCosts];		/// Used resources
    int		Needed[MaxCosts];	/// Needed resources
    int		NeededMask;		/// Mask for needed resources

    int		NeedFood;		/// Flag need food

	/// number of elements in UnitTypeRequests
    int			UnitTypeRequestsCount;
	/// unit-types to build/train requested and priority list
    AiUnitTypeTable*	UnitTypeRequests;
	/// number of elements in UpgradeRequests
    int			UpgradeToRequestsCount;
	/// Upgrade to unit-type requested and priority list
    UnitType**		UpgradeToRequests;
	/// number of elements in ResearchRequests
    int			ResearchRequestsCount;
	/// Upgrades requested and priority list
    Upgrade**		ResearchRequests;

	/// What the resource manager should build
    AiBuildQueue*	UnitTypeBuilded;

	/// Last building checked for repair in this turn
    int		LastRepairBuilding;

} PlayerAi;

/**
**	AI Helper.
**
**	Contains informations needed for the AI. If the AI needs an unit or
**	building or upgrade or spell, it could lookup in this tables to find
**	where it could be trained, builded or researched.
*/
typedef struct _ai_helper_ {
    /**
    **	The index is the unit that should be trained, giving a table of all
    **	units/buildings which could train this unit.
    */
    int			TrainCount;
    AiUnitTypeTable**	Train;
    /**
    **	The index is the unit that should be build, giving a table of all
    **	units/buildings which could build this unit.
    */
    int			BuildCount;
    AiUnitTypeTable**	Build;
    /**
    **	The index is the upgrade that should be made, giving a table of all
    **	units/buildings which could do the upgrade.
    */
    int			UpgradeCount;
    AiUnitTypeTable**	Upgrade;
    /**
    **	The index is the research that should be made, giving a table of all
    **	units/buildings which could research this upgrade.
    */
    int			ResearchCount;
    AiUnitTypeTable**	Research;
    /**
    **	The index is the unit that should be repaired, giving a table of all
    **	units/buildings which could repair this unit.
    */
    int			RepairCount;
    AiUnitTypeTable**	Repair;
    /**
    **	The index is the costs that should be collected, giving a table of all
    **	units/buildings which could collect this resource.
    */
    int			CollectCount;
    AiUnitTypeTable**	Collect;
    /**
    **	The index is the costs that should be collected, giving a table of all
    **	units/buildings which could carray this resource.
    */
    int			WithGoodsCount;
    AiUnitTypeTable**	WithGoods;
    /**
    **	The index is the unit-limit that should be solved, giving a table of all
    **	units/buildings which could reduce this unit-limit.
    */
    int			UnitLimitCount;
    AiUnitTypeTable**	UnitLimit;
    /**
    **	The index is the unit that should be made, giving a table of all
    **	units/buildings which are equivalent.
    */
    int			EquivCount;
    AiUnitTypeTable**	Equiv;
} AiHelper;

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern AiType* AiTypes;			/// List of all AI types
extern AiHelper AiHelpers;		/// AI helper variables

extern PlayerAi* AiPlayer;		/// Current AI player

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

//
//	Resource manager
//
    /// Add unit-type request to resource manager
extern void AiAddUnitTypeRequest(UnitType* type,int count);
    /// Add upgrade-to request to resource manager
extern void AiAddUpgradeToRequest(UnitType* type);
    /// Add research request to resource manager
extern void AiAddResearchRequest(Upgrade* upgrade);
    /// Periodic called resource manager handler
extern void AiResourceManager(void);

//
//	Buildings
//
    /// Find nice building place
extern int AiFindBuildingPlace(const Unit*, const UnitType * , int *, int *);

//
//	Forces
//
    /// Assign a new unit to a force
extern void AiAssignToForce(Unit* unit);
    /// Assign a free units to a force
extern void AiAssignFreeUnitsToForce(void);
    /// Attack with force at position
extern void AiAttackWithForceAt(int force,int x,int y);
    /// Attack with force
extern void AiAttackWithForce(int force);
    /// Periodic called force manager handler
extern void AiForceManager(void);

//@}

#endif	// NEW_AI

#endif	// !__AI_LOCAL_H__
