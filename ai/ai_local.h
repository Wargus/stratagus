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

#include "player.h"
#include "unittype.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

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
    unsigned char	AllExplored : 1;	/// Ai sees unexplored area.
    unsigned char	AllVisibile : 1;	/// Ai sees invisibile area.

    AiScript*		Script;			/// Main script
};

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

/**
**	AI variables.
*/
typedef struct _player_ai_ {
    Player*     Player;                 /// engine player structure.

    AiType*	AiType;			/// AI type of this player AI.

    // goals stuff.
    AiGoal*     GoalHead;               /// goals start of double linked list
    AiGoal*     GoalNil1;               /// goals dummy end of dl-list
    AiGoal*     GoalTail;               /// goals end of double linked list

    AiGoal*     WaitHead;               /// wait start of double linked list
    AiGoal*     WaitNil1;               /// wait dummy end of dl-list
    AiGoal*     WaitTail;               /// wait end of double linked list

    // scripting stuff.
    AiScript*		Ip;		/// AI script instruction pointer.
    AiScript**		Sp;		/// Ai script stack pointer.
    AiScript**		Stack;		/// Ai script stack.

    UnitType*	Priority[256];		/// Building order
} PlayerAi;

/**
**	AI Unittable with counter in front.
*/
typedef struct _ai_unittable_ {
    unsigned	Count;			/// elements in table
    UnitType*	Table[0];		/// the table (GNU feature used) 
} AiUnitTable;

/**
**	AI Helper.
*/
typedef struct _ai_helper_ {
    /**
    **	The index is the unit that should be trained, giving a table of all
    **	units/buildings which could train this unit.
    */
    int			TrainCount;
    AiUnitTable*	Train;
    /**
    **	The index is the unit that should be build, giving a table of all
    **	units/buildings which could build this unit.
    */
    int			BuildCount;
    AiUnitTable*	Build;
    /**
    **	The index is the upgrade that should be made, giving a table of all
    **	units/buildings which could do the upgrade.
    */
    int			UpgradeCount;
    AiUnitTable*	Upgrade;
    /**
    **	The index is the research that should be made, giving a table of all
    **	units/buildings which could research this upgrade.
    */
    int			ResearchCount;
    AiUnitTable*	Research;
} AiHelper;

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern AiType* AiTypes;			/// List of all AI types.
extern AiHelper AiHelpers;		/// AI helper variables

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

//@}

#endif	// NEW_AI

#endif	// !__AI_LOCAL_H__
