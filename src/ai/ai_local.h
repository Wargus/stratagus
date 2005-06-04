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
/**@name ai_local.h - The local AI header file. */
//
//      (c) Copyright 2000-2004 by Lutz Sammer and Antonis Chaniotis.
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

#ifndef __AI_LOCAL_H__
#define __AI_LOCAL_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "upgrade_structs.h"
#include "unit.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

struct _unit_;
struct _unit_type_;
struct _upgrade_;
struct _player_;

/**
**  Ai Type typedef
*/
typedef struct _ai_type_ AiType;

/**
**  Ai Type structure.
*/
struct _ai_type_{
	AiType* Next;  /// Next ai type

	char*   Name;  /// Name of this ai
	char*   Race;  /// for this race
	char*   Class; /// class of this ai

	// nice flags
#if 0
	unsigned char AllExplored : 1; /// Ai sees unexplored area
	unsigned char AllVisbile : 1;  /// Ai sees invisibile area
#endif

	char* Script; /// Main script
	char* FunctionName; /// Name of the function
};

/**
**  AI unit-type table with counter in front.
*/
typedef struct _ai_unittype_table_ {
	int                 Count;    /// elements in table
	struct _unit_type_* Table[1]; /// the table
} AiUnitTypeTable;

/**
**  Ai unit-type typedef
*/
typedef struct _ai_unit_type_ AiUnitType;

/**
**  Ai unit-type in a force.
*/
struct _ai_unit_type_ {
	AiUnitType*         Next; /// next unit-type
	int                 Want; /// number of this unit-type wanted
	struct _unit_type_* Type; /// unit-type self
};

/**
**  AIUnit typedef
*/
typedef struct _ai_unit_ AiUnit;

/**
**  Ai unit in a force.
*/
struct _ai_unit_ {
	AiUnit*        Next; /// next unit
	struct _unit_* Unit; /// unit self
};

/**
**  Roles for forces
*/
enum _ai_force_role_ {
	AiForceRoleAttack, /// Force should attack
	AiForceRoleDefend, /// Force should defend
};


/**
**  Define an AI force.
**
**  A force is a group of units belonging together.
*/
typedef struct _ai_force_ {
	char Completed;     /// Flag saying force is complete build
	char Defending;     /// Flag saying force is defending
	char Attacking;     /// Flag saying force is attacking
	char Role;          /// Role of the force

	AiUnitType* UnitTypes; /// Count and types of unit-type
	AiUnit*     Units;     /// Units in the force

	//
	// If attacking
	//
	int State;         /// Attack state
	int GoalX;         /// Attack point X tile map position
	int GoalY;         /// Attack point Y tile map position
	int MustTransport; /// Flag must use transporter
} AiForce;

/**
**  AI build queue typedef
*/
typedef struct _ai_build_queue_ AiBuildQueue;

/**
**  AI build queue.
**
**  List of orders for the resource manager to handle
*/
struct _ai_build_queue_ {
	AiBuildQueue*       Next; /// next request
	int                 Want; /// requested number
	int                 Made; /// built number
	struct _unit_type_* Type; /// unit-type
};

typedef struct _ai_exploration_request_ AiExplorationRequest;

struct _ai_exploration_request_ {
	int                   X;    /// x pos on map
	int                   Y;    /// y pos on map
	int                   Mask; /// mask ( ex: MapFieldLandUnit )
	AiExplorationRequest* Next; /// Next in linked list
};

typedef struct _ai_transport_request_ AiTransportRequest;

struct _ai_transport_request_ {
	struct _unit_*      Unit;
	Order               Order;
	AiTransportRequest* Next;
};

/**
**  AI variables.
*/
typedef struct _player_ai_ {
	struct _player_* Player; /// Engine player structure
	AiType* AiType;          /// AI type of this player AI
	// controller
	char*               Script;          /// Script executed
	int                 ScriptDebug;     /// Flag script debuging on/off
	unsigned long       SleepCycles;     /// Cycles to sleep

	// forces
#define AI_MAX_FORCES 10                    /// How many forces are supported
#define AI_MAX_ATTACKING_FORCES 30          /// Attacking forces
	AiForce Force[AI_MAX_ATTACKING_FORCES]; /// Forces controlled by AI

	// resource manager
	int Reserve[MaxCosts]; /// Resources to keep in reserve
	int Used[MaxCosts];    /// Used resources
	int Needed[MaxCosts];  /// Needed resources
	int Collect[MaxCosts]; /// Collect % of resources
	int NeededMask;        /// Mask for needed resources
	int NeedSupply;        /// Flag need food

	AiExplorationRequest* FirstExplorationRequest;  /// Requests for exploration
	unsigned long         LastExplorationGameCycle; /// When did the last explore occur?
	AiTransportRequest*   TransportRequests;        /// Requests for transport
	unsigned long         LastCanNotMoveGameCycle;  /// Last can not move cycle
	int                   UnitTypeRequestsCount;    /// unit-types to build/train request,priority list
	AiUnitTypeTable*      UnitTypeRequests;         /// number of elements in UpgradeRequests
	int                   UpgradeToRequestsCount;   /// Upgrade to unit-type requested and priority list
	struct _unit_type_**  UpgradeToRequests;        /// number of elements in ResearchRequests
	int                   ResearchRequestsCount;    /// Upgrades requested and priority list
	struct _upgrade_**    ResearchRequests;         /// What the resource manager should build
	AiBuildQueue*         UnitTypeBuilt;          /// Last building checked for repair in this turn
	int                   LastRepairBuilding;       /// No. workers that failed trying to repair a building
	unsigned              TriedRepairWorkers[UnitMax];
} PlayerAi;

/**
**  AI Helper.
**
**  Contains informations needed for the AI. If the AI needs an unit or
**  building or upgrade or spell, it could lookup in this tables to find
**  where it could be trained, built or researched.
*/
typedef struct _ai_helper_ {
	/**
	** The index is the unit that should be trained, giving a table of all
	** units/buildings which could train this unit.
	*/
	int               TrainCount;
	AiUnitTypeTable** Train;
	/**
	** The index is the unit that should be build, giving a table of all
	** units/buildings which could build this unit.
	*/
	int               BuildCount;
	AiUnitTypeTable** Build;
	/**
	** The index is the upgrade that should be made, giving a table of all
	** units/buildings which could do the upgrade.
	*/
	int               UpgradeCount;
	AiUnitTypeTable** Upgrade;
	/**
	** The index is the research that should be made, giving a table of all
	** units/buildings which could research this upgrade.
	*/
	int               ResearchCount;
	AiUnitTypeTable** Research;
	/**
	** The index is the unit that should be repaired, giving a table of all
	** units/buildings which could repair this unit.
	*/
	int               RepairCount;
	AiUnitTypeTable** Repair;
	/**
	** The index is the unit-limit that should be solved, giving a table of all
	** units/buildings which could reduce this unit-limit.
	*/
	int               UnitLimitCount;
	AiUnitTypeTable** UnitLimit;
	/**
	** The index is the unit that should be made, giving a table of all
	** units/buildings which are equivalent.
	*/
	int               EquivCount;
	AiUnitTypeTable** Equiv;
} AiHelper;

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern AiType* AiTypes; /// List of all AI types
extern AiHelper AiHelpers; /// AI helper variables

extern int UnitTypeEquivs[UnitTypeMax + 1]; /// equivalence between unittypes
extern PlayerAi* AiPlayer; /// Current AI player

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

//
// Resource manager
//
	/// Add unit-type request to resource manager
extern void AiAddUnitTypeRequest(struct _unit_type_* type, int count);
	/// Add upgrade-to request to resource manager
extern void AiAddUpgradeToRequest(struct _unit_type_* type);
	/// Add research request to resource manager
extern void AiAddResearchRequest(struct _upgrade_* upgrade);
	/// Periodic called resource manager handler
extern void AiResourceManager(void);
	/// Ask the ai to explore around x,y
extern void AiExplore(int x, int y, int exploreMask);
	/// Make two unittypes be considered equals
extern void AiNewUnitTypeEquiv(struct _unit_type_* a, struct _unit_type_* b);
	/// Remove any equivalence between unittypes
extern void AiResetUnitTypeEquiv(void);
	/// Finds all equivalents units to a given one
extern int AiFindUnitTypeEquiv(const struct _unit_type_* i, int* result);
	/// Finds all available equivalents units to a given one, in the prefered order
extern int AiFindAvailableUnitTypeEquiv(const struct _unit_type_* i,
	int* result);

//
// Buildings
//
	/// Find nice building place
extern int AiFindBuildingPlace(const struct _unit_* worker,
	const struct _unit_type_* type, int* dx, int* dy);

//
// Forces
//
	/// Cleanup units in force
extern void AiCleanForces(void);
	/// Assign a new unit to a force
extern void AiAssignToForce(Unit* unit);
	/// Assign a free units to a force
extern void AiAssignFreeUnitsToForce(void);
	/// Attack with force at position
extern void AiAttackWithForceAt(int force, int x, int y);
	/// Attack with force
extern void AiAttackWithForce(int force);
	/// Periodic called force manager handler
extern void AiForceManager(void);

//
// Plans
//
	/// Find a wall to attack
extern int AiFindWall(AiForce* force);
	/// Plan the an attack
extern int AiPlanAttack(AiForce* force);
	/// Send explorers around the map
extern void AiSendExplorers(void);

//
// Magic
//
	/// Check for magic
extern void AiCheckMagic(void);

//@}

#endif // !__AI_LOCAL_H__
