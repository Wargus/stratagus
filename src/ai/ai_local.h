//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __|
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name ai_local.h	-	The local AI header file. */
//
//      (c) Copyright 2000-2003 by Lutz Sammer and Antonis Chaniotis.
//
//	Stratagus is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
//
//	Stratagus is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//      $Id$

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

    SCM			Script;			/// Main script (gc-protected!)
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
**	Roles for forces
*/
enum _ai_force_role_ {
    AiForceRoleAttack,			/// Force should attack
    AiForceRoleDefend,			/// Force should defend
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
    char		Completed;	/// Flag saying force is complete build
    char		Defending;	/// Flag saying force is defending
    char		Attacking;	/// Flag saying force is attacking

    char		Role;		/// Role of the force

    AiUnitType*		UnitTypes;	/// Count and types of unit-type
    AiUnit*		Units;		/// Units in the force
	//
	// If attacking
	//
    int			State;		/// Attack state
    int			GoalX;		/// Attack point X tile map position
    int			GoalY;		/// Attack point Y tile map position
    int			MustTransport;	/// Flag must use transporter
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
    unsigned long	SleepCycles;	/// Cycles to sleep

    // forces
#define AI_MAX_FORCES	10		/// How many forces are supported
#define AI_MAX_ATTACKING_FORCES	30	/// Attacking forces
    AiForce	Force[AI_MAX_ATTACKING_FORCES];	/// Forces controlled by AI

    // resource manager

    int		Reserve[MaxCosts];	/// Resources to keep in reserve
    int		Used[MaxCosts];		/// Used resources
    int		Needed[MaxCosts];	/// Needed resources
    int		Collect[MaxCosts];	/// Collect % of resources
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
    int			LastRepairBuilding;
	/// Number of workers that unsuccessfully tried to repair a building
    unsigned		TriedRepairWorkers[UnitMax];

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
extern char** AiTypeWcNames;		/// pud num to internal string mapping

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
    /// Cleanup units in force
extern void AiCleanForces(void);
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

//
//	Plans
//
    /// Find a wall to attack
extern int AiFindWall(AiForce* force);
    /// Plan the an attack
extern int AiPlanAttack(AiForce* force);

//
//	Magic
//
    /// Check for magic
extern void AiCheckMagic(void);

//@}

#endif	// NEW_AI

#endif	// !__AI_LOCAL_H__
