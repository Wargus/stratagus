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
    AiCmdForce,				/// Set force.
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
    AiGoal*	Down;			/// dependend goals.
    AiGoal*	Prev;			/// double linked list of all goals
    int		Priority;		/// Priority of this goal.
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
    AiType*		Next;			/// Next ai type.

    char*		Name;			/// Name of this ai.
    char*		Race;			/// for this race.
    char*		Class;			/// class of this ai.

    // nice flags
    //unsigned char	AllExplored : 1;	/// Ai sees unexplored area.
    //unsigned char	AllVisibile : 1;	/// Ai sees invisibile area.

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
**	AI force typedef.
*/
typedef struct _ai_force_ AiForce;

/**
**	Define an AI force.
**
**	A force is a group of units belonging together.
*/
struct _ai_force_ {
    AiUnitTypeTable	UnitTypeTable;	/// Count and types of unit-type
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

    // resource manager

	/// number of elements in UnitTypeRequests
    int			RequestsCount;
	/// unit-types to build/train requested and priority list
    AiUnitTypeTable*	UnitTypeRequests;

	/// number of elements in UnitTypeBuilded
    int			BuildedCount;
    struct {
	int		Want;		/// requested number
	int		Made;		/// builded number
	UnitType*	Type;		/// unit-type
    }*		UnitTypeBuilded;	/// What the resource manager does

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
    /// Periodic called resource manager handler
extern void AiResourceManager(void);


//@}

#endif	// NEW_AI

#endif	// !__AI_LOCAL_H__
