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
//      the Free Software Foundation; version 2 dated June, 1991.
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

#include "ccl.h"
#include "player.h"
#include "unittype.h"
#include "upgrade.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

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

#if defined(USE_GUILE) || defined(USE_SIOD)
	SCM Script;   /// Main script (gc-protected!)
#elif defined(USE_LUA)
	char* Script; /// Main script
#endif
};

/**
**  AI unit-type table with counter in front.
*/
typedef struct _ai_unittype_table_ {
	int       Count;    /// elements in table
	UnitType* Table[1]; /// the table
} AiUnitTypeTable;

/**
**  Ai unit-type typedef
*/
typedef struct _ai_unit_type_ AiUnitType;

/**
**  Ai unit-type in a force.
*/
struct _ai_unit_type_ {
	AiUnitType* Next; /// next unit-type
	int         Want; /// number of this unit-type wanted
	UnitType*   Type; /// unit-type self
};

/**
**  AIUnit typedef
*/
typedef struct _ai_unit_ AiUnit;

/**
**  Ai unit in a force.
*/
struct _ai_unit_ {
	AiUnit* Next; /// next unit
	Unit*   Unit; /// unit self
};

/**
**  Roles for forces
*/
enum _ai_force_role_ {
	AiForceRoleAttack, /// Force should attack
	AiForceRoleDefend, /// Force should defend
};


/**
**  Ways to populate a force
*/
enum _ai_force_populate_mode_ {
	AiForceDontPopulate,        /// Force won't receive any unit
	AiForcePopulateFromScratch, /// Force unit's will be builded
	AiForcePopulateFromAttack,  /// Force will receive units from idle attack force only - nothing builded
	AiForcePopulateAny          /// Force will receive units from any idle force - nothing builded
};

/**
**  How to react when an unit is attacked in a force
*/
enum _ai_force_help_mode_ {
	AiForceDontHelp,  /// Don't react to attack on this force
	AiForceHelpForce, /// Send idle units to defend
	AiForceHelpFull   /// Create a defend force, send it, ...
};

/**
**  Define an AI force.
**
**  A force is a group of units belonging together.
*/
typedef struct _ai_force_ {
	char Completed;     /// Flag saying force is complete build
	char Attacking;     /// Is this force attacking ( aka not idle )
	char Role;          /// Role of the force
	char PopulateMode;  /// Which forces can be used to fill this force ?
	char UnitsReusable; /// Indicate moving units of this force into others is allowed.
	char HelpMode;      /// How to react to treat in this force ?

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
	AiBuildQueue* Next; /// next request
	int           Want; /// requested number
	int           Made; /// builded number
	UnitType*     Type; /// unit-type
};

/**
**  AI running script ( with state, ... )
*/
typedef struct _ai_running_script_ {
#if defined(USE_GUILE) || defined(USE_SIOD)
	SCM           Script;      /// Script executed
#elif defined(USE_LUA)
	char*         Script;      /// Script executed
#endif
	unsigned long SleepCycles; /// Cycles to sleep
	char          Ident[10];   /// Debugging !
	int           HotSpotX;    /// Hot spot ( for defense, attack, ... )
	int           HotSpotY;
	int           HotSpotRay;
	int           OwnForce;    /// A force ID ( the n° of the script... )
	int*          Gauges;      /// Gauges values ( initially 0 )

	// Total number of resource gauges
#define RESOURCE_COUNT  3
	// Total number of forces gauges
#define FORCE_COUNT		11

#define GAUGE_NB (3 + (RESOURCE_COUNT * 2) + (FORCE_COUNT * 6))
} AiRunningScript;

/**
**  Ai script action
**
**  Describe each different attack/defend scheme.
**
**  Linked list.
*/
typedef struct _ai_script_action_ {
#if defined(USE_GUILE) || defined(USE_SIOD)
	SCM Action;    /// Scheme description, in the form :
	               /// '((name evaluate-lambda run-script) ... )
#elif defined(USE_LUA)
	char* Action;  /// Name of lua table
#endif

	int Defensive; /// Is this action usable for defense
	int Offensive; /// Is this action usable for attack

	/// TODO : hotspot_kind : set if the hotspot should contain path from base
} AiScriptAction;

/**
**  AiActionEvaluation typedef
*/
typedef struct _ai_action_evaluation_ AiActionEvaluation;

/**
**  Ai action evaluation
**
**  Each AiPlayer periodically evaluation an attack action.
**
**  If it is ready, the attack is fired. Else, it is keept for a while.
**  From time to time, the best unfired try is fired.
**
*/
struct _ai_action_evaluation_ {
	AiScriptAction*     AiScriptAction; /// Action evaluated
	unsigned long       GameCycle;      /// Gamecycle when this evaluation occured
	int                 HotSpotX;       /// X position of the hotspot, or -1
	int                 HotSpotY;       /// Y position of the hotspot, or -1
	int                 HotSpotValue;   /// Value of the hotspot ( total points to get... )
	int                 Value;          /// Result of the evaluation ( resources needed... )
	AiActionEvaluation* Next;           /// Next in linked list
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
	Unit*               Unit;
	Order               Order;
	AiTransportRequest* Next;
};

/**
**  AI variables.
*/
typedef struct _player_ai_ {
	Player* Player; /// Engine player structure
	AiType* AiType; /// AI type of this player AI
	// controller
#define AI_MAX_RUNNING_SCRIPTS 5 /// ( generic, attack, defend, ... )
#define AI_MAIN_SCRIPT 0
	AiRunningScript Scripts[AI_MAX_RUNNING_SCRIPTS]; /// All running scripts

	// Ai "memory"
#define AI_MEMORY_SIZE 30 /// Max number of keept evaluation ( => 30 sec )
	AiActionEvaluation* FirstEvaluation; /// begining of linked list of evaluation
	AiActionEvaluation* LastEvaluation;  /// end of linked list of evaluation
	int                 EvaluationCount; /// size of linked list of evaluation
	int                 ScriptDebug;     /// Flag script debuging on/off
	int                 AutoAttack;      /// Are attack started automatically ?

	// forces
#define AI_MAX_FORCES 10 /// How many forces are supported
#define AI_GENERIC_FORCES (AI_MAX_FORCES-AI_MAX_RUNNING_SCRIPTS) /// How many forces are useable in the main script
	AiForce Force[AI_MAX_FORCES]; /// Forces controlled by AI

	// resource manager
	int Reserve[MaxCosts]; /// Resources to keep in reserve
	int Used[MaxCosts];    /// Used resources
	int Needed[MaxCosts];  /// Needed resources
	int Collect[MaxCosts]; /// Collect % of resources
	int NeededMask;        /// Mask for needed resources
	int NeedSupply;        /// Flag need food

	AiExplorationRequest* FirstExplorationRequest;  /// Requests for exploration
	unsigned int          LastExplorationGameCycle; /// When did the last explore occur ?
	AiTransportRequest*   TransportRequests;
	unsigned int          LastCanNotMoveGameCycle;  /// number of elements in UnitTypeRequests
	int                   UnitTypeRequestsCount;    /// unit-types to build/train request,priority list
	AiUnitTypeTable*      UnitTypeRequests;         /// number of elements in UpgradeRequests
	int                   UpgradeToRequestsCount;   /// Upgrade to unit-type requested and priority list
	UnitType**            UpgradeToRequests;        /// number of elements in ResearchRequests
	int                   ResearchRequestsCount;    /// Upgrades requested and priority list
	Upgrade**             ResearchRequests;         /// What the resource manager should build
	AiBuildQueue*         UnitTypeBuilded;          /// Last building checked for repair in this turn
	int                   LastRepairBuilding;       /// No. workers that failed trying to repair a building
	unsigned              TriedRepairWorkers[UnitMax];
} PlayerAi;

/**
**  AI Helper.
**
**  Contains informations needed for the AI. If the AI needs an unit or
**  building or upgrade or spell, it could lookup in this tables to find
**  where it could be trained, builded or researched.
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

#define MaxAiScriptActions 64 /// How many AiScriptActions are supported
extern int AiScriptActionNum; /// Current number of AiScriptAction
extern AiScriptAction AiScriptActions[MaxAiScriptActions]; /// All availables AI script actions
extern int UnitTypeEquivs[UnitTypeMax + 1]; /// equivalence between unittypes
extern PlayerAi* AiPlayer; /// Current AI player
extern AiRunningScript* AiScript; /// Currently running script
extern char** AiTypeWcNames; /// pud num to internal string mapping

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

//
// Resource manager
//
	/// Add unit-type request to resource manager
extern void AiAddUnitTypeRequest(UnitType* type, int count);
	/// Add upgrade-to request to resource manager
extern void AiAddUpgradeToRequest(UnitType* type);
	/// Add research request to resource manager
extern void AiAddResearchRequest(Upgrade*  upgrade);
	/// Periodic called resource manager handler
extern void AiResourceManager(void);
	/// Ask the ai to explore around x,y
extern void AiExplore(int x, int y, int exploreMask);
	/// Count the number of builder unit available for the given unittype
extern int AiCountUnitBuilders(UnitType* type);
	/// Make two unittypes be considered equals
extern void AiNewUnitTypeEquiv(UnitType* a, UnitType* b);
	/// Remove any equivalence between unittypes
extern void AiResetUnitTypeEquiv(void);
	/// Finds all equivalents units to a given one
extern int AiFindUnitTypeEquiv(const UnitType* i, int* result);
	/// Finds all available equivalents units to a given one, in the prefered order
extern int AiFindAvailableUnitTypeEquiv(const UnitType* i, int* result);

//
// Buildings
//
	/// Find nice building place
extern int AiFindBuildingPlace(const Unit*, const UnitType*, int*, int*);

//
// Forces
//
	/// Cleanup units in force
extern void AiCleanForces(void);
	/// Cleanup units in the given force
extern void AiCleanForce(int force);
	/// Remove everything in the given force
extern void AiEraseForce(int force);
	/// Assign a new unit to a force
extern void AiAssignToForce(Unit* unit);
	/// Assign a free units to a force
extern void AiAssignFreeUnitsToForce(void);
	/// Complete a force with units form another
extern void AiForceTransfert(int src, int dst);
	/// Group a force on the nearest unit to target
extern void AiGroupForceNear(int force, int targetx, int targety);
	/// Attack with force at position
extern void AiAttackWithForceAt(int force, int x, int y);
	/// Attack with force
extern void AiAttackWithForce(int force);
	/// Send force home
extern void AiSendForceHome(int force);
	/// Evaluate the cost to build a force (time to build + resources)
extern int AiEvaluateForceCost(int force, int total);
	/// Complete a force from existing units.
extern void AiForceComplete(int force);
	/// Enrole one or more units of a type in a force
extern int AiEnroleSpecificUnitType(int force, UnitType* ut, int count);
	/// Create a force from existing units, ready to respond to the powers
extern int AiCreateSpecificForce(int *power, int *unittypes, int unittypescount);
	/// Force's unit is attacked.
extern void AiForceHelpMe(int force, const Unit* attacker, Unit* defender);
	/// Periodic called force manager handler
extern void AiForceManager(void);
	/// Calculate the number of unit produced for each wanted unittype
extern void AiForceCountUnits(int force, int* unittypeCount);
	/// Substract the number of unit wanted for each unittype
extern int AiForceSubstractWant(int force, int* unittypeCount);

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
// Scripts
//
	/// Run a script ( for the current AiPlayer )
#if defined(USE_GUILE) || defined(USE_SIOD)
extern void AiRunScript(int script, SCM list, int hotSpotX, int hotSpotY, int hotSpotRay);
#elif defined(USE_LUA)
#endif
	/// Find a script for defense.
extern void AiFindDefendScript(int attackX, int attackY);
	/// Check if attack is possible
extern void AiPeriodicAttack(void);

//
// Gauges
//
	/// Compute gauges for the current RunningScript
extern void AiComputeCurrentScriptGauges(void);
	/// Output gauges values
extern void AiDebugGauges(void);
	/// Give the value of a specific gauge, for the current RunningScript
extern int AiGetGaugeValue(int gauge);
	/// Find a gauge given its identifier.
#if defined(USE_GUILE) || defined(USE_SIOD)
extern int AiFindGaugeId(SCM id);
#elif defined(USE_LUA)
extern int AiFindGaugeId(lua_State* l);
#endif
	/// return the force of the unittype.
extern int AiUnitTypeForce(UnitType* unitType);

//
// Magic
//
	/// Check for magic
extern void AiCheckMagic(void);

//
// Ccl helpers
//

	/// Save/Load a PlayerAi structure ( see ccl_helpers.h for details )
#if defined(USE_GUILE) || defined(USE_SIOD)
extern void IOPlayerAiFullPtr(SCM form, void* binaryform, void* para);
#elif defined(USE_LUA)
#endif

//@}

#endif // !__AI_LOCAL_H__
