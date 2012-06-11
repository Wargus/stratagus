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
//      (c) Copyright 2000-2005 by Lutz Sammer and Antonis Chaniotis.
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

#ifndef __AI_LOCAL_H__
#define __AI_LOCAL_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <vector>

#include "upgrade_structs.h"
#include "unit.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUnit;
class CUnitType;
class CUpgrade;
class CPlayer;

/**
**  Ai Type structure.
*/
class CAiType
{
public:
	CAiType() {}

	std::string Name;     /// Name of this ai
	std::string Race;     /// for this race
	std::string Class;    /// class of this ai
	std::string Script;   /// Main script
};

/**
**  AI unit-type table with counter in front.
*/
class AiRequestType
{
public:
	AiRequestType() : Count(0), Type(NULL) {}

	unsigned int Count;  /// elements in table
	CUnitType *Type;     /// the type
};

/**
**  Ai unit-type in a force.
*/
class AiUnitType
{
public:
	AiUnitType() : Want(0), Type(NULL) {}

	unsigned int Want; /// number of this unit-type wanted
	CUnitType *Type;   /// unit-type self
};

/**
**  Roles for forces
*/
enum AiForceRole {
	AiForceRoleDefault = 0, /// So default is attacking
	AiForceRoleAttack = 0, /// Force should attack
	AiForceRoleDefend      /// Force should defend
};

enum AiForceAttackingState {
	AiForceAttackingState_Free = -1,
	AiForceAttackingState_Waiting = 0,
	AiForceAttackingState_Boarding,
	AiForceAttackingState_AttackingWithTransporter,
	AiForceAttackingState_Attacking,
};

/**
**  Define an AI force.
**
**  A force is a group of units belonging together.
*/
class AiForce
{
	friend class AiForceManager;
public:
	AiForce() :
		Completed(false), Defending(false), Attacking(false),
		Role(AiForceRoleDefault), State(AiForceAttackingState_Free) {
		HomePos.x = HomePos.y = GoalPos.x = GoalPos.y = -1;
	}

	void Remove(CUnit &unit) {
		if (Units.Remove(&unit)) {
			InternalRemoveUnit(&unit);
		}
	}

	/**
	**  Reset the force. But don't change its role and its demand.
	*/
	void Reset(bool types = false) {
		Completed = false;
		Defending = false;
		Attacking = false;
		if (types) {
			UnitTypes.clear();
			State = AiForceAttackingState_Free;
		} else {
			State = AiForceAttackingState_Waiting;
		}
		Units.for_each(InternalRemoveUnit);
		Units.clear();
		HomePos.x = HomePos.y = GoalPos.x = GoalPos.y = -1;
	}
	inline size_t Size() const { return Units.size(); }

	inline bool IsAttacking() const { return (!Defending && Attacking); }

	void Attack(const Vec2i &pos);
	void RemoveDeadUnit();
	int PlanAttack();

private:
	void CountTypes(unsigned int *counter, const size_t len);
	bool IsBelongsTo(const CUnitType *type);
	void Insert(CUnit &unit) {
		Units.Insert(&unit);
		unit.RefsIncrease();
	}

	void Update();

	static void InternalRemoveUnit(CUnit *unit) {
		unit->GroupId = 0;
		unit->RefsDecrease();
	}

public:
	bool Completed;    /// Flag saying force is complete build
	bool Defending;    /// Flag saying force is defending
	bool Attacking;    /// Flag saying force is attacking
	AiForceRole Role;  /// Role of the force

	std::vector<AiUnitType> UnitTypes; /// Count and types of unit-type
	CUnitCache Units;  /// Units in the force

	// If attacking
	AiForceAttackingState State; /// Attack state
	Vec2i GoalPos; /// Attack point tile map position
	Vec2i HomePos; /// Return after attack tile map position
};

// forces
#define AI_MAX_FORCES 50                    /// How many forces are supported

/**
**  AI force manager.
**
**  A Forces container for the force manager to handle
*/
class AiForceManager
{
public:
	AiForceManager();

	inline size_t Size() const { return forces.size(); }

	const AiForce &operator[](unsigned int index) const { return forces[index]; }
	AiForce &operator[](unsigned int index) { return forces[index]; }

	int getIndex(AiForce *force) const {
		for (unsigned int i = 0; i < forces.size(); ++i) {
			if (force == &forces[i]) {
				return i;
			}
		}
		return -1;
	}

	unsigned int getScriptForce(unsigned int index) {
		if (script[index] == -1) {
			script[index] = FindFreeForce();
		}
		return script[index];
	}

	void RemoveDeadUnit();
	bool Assign(CUnit &unit);
	void Update();
	unsigned int FindFreeForce(AiForceRole role = AiForceRoleDefault);
	void CheckUnits(int *counter);
private:
	std::vector<AiForce> forces;
	char script[AI_MAX_FORCES];
};

/**
**  AI build queue.
**
**  List of orders for the resource manager to handle
*/
class AiBuildQueue
{
public:
	AiBuildQueue() : Want(0), Made(0), Type(NULL), Wait(0) {
		Pos.x = Pos.y = -1;
	}

public:
	unsigned int Want;  /// requested number
	unsigned int Made;  /// built number
	CUnitType *Type;    /// unit-type
	unsigned long Wait; /// wait until this cycle
	Vec2i Pos;          /// build near pos on map
};

/**
**  AI exploration request
*/
class AiExplorationRequest
{
public:
	AiExplorationRequest(const Vec2i &pos, int mask) : pos(pos), Mask(mask) {}

public:
	Vec2i pos;          /// pos on map
	int Mask;           /// mask ( ex: MapFieldLandUnit )
};

/**
**  AI variables.
*/
class PlayerAi
{
public:
	PlayerAi() : Player(NULL), AiType(NULL),
		SleepCycles(0), NeededMask(0), NeedSupply(false),
		ScriptDebug(false), LastExplorationGameCycle(0),
		LastCanNotMoveGameCycle(0), LastRepairBuilding(0) {
		memset(Reserve, 0, sizeof(Reserve));
		memset(Used, 0, sizeof(Used));
		memset(Needed, 0, sizeof(Needed));
		memset(Collect, 0, sizeof(Collect));
		memset(TriedRepairWorkers, 0, sizeof(TriedRepairWorkers));
	}

public:
	CPlayer *Player;            /// Engine player structure
	CAiType *AiType;            /// AI type of this player AI
	// controller
	std::string Script;         /// Script executed
	unsigned long SleepCycles;  /// Cycles to sleep

	AiForceManager Force;  /// Forces controlled by AI

	// resource manager
	int Reserve[MaxCosts]; /// Resources to keep in reserve
	int Used[MaxCosts];    /// Used resources
	int Needed[MaxCosts];  /// Needed resources
	int Collect[MaxCosts]; /// Collect % of resources
	int NeededMask;        /// Mask for needed resources
	bool NeedSupply;       /// Flag need food
	bool ScriptDebug;      /// Flag script debuging on/off

	std::vector<AiExplorationRequest> FirstExplorationRequest;/// Requests for exploration
	unsigned long LastExplorationGameCycle;       /// When did the last explore occur?
	unsigned long LastCanNotMoveGameCycle;        /// Last can not move cycle
	std::vector<AiRequestType> UnitTypeRequests;  /// unit-types to build/train request,priority list
	std::vector<CUnitType *> UpgradeToRequests;   /// Upgrade to unit-type requested and priority list
	std::vector<CUpgrade *> ResearchRequests;     /// Upgrades requested and priority list
	std::vector<AiBuildQueue> UnitTypeBuilt;      /// What the resource manager should build
	int LastRepairBuilding;                       /// Last building checked for repair in this turn
	unsigned int TriedRepairWorkers[UnitMax];     /// No. workers that failed trying to repair a building
};

/**
**  AI Helper.
**
**  Contains informations needed for the AI. If the AI needs an unit or
**  building or upgrade or spell, it could lookup in this tables to find
**  where it could be trained, built or researched.
*/
class AiHelper
{
public:
	/**
	** The index is the unit that should be trained, giving a table of all
	** units/buildings which could train this unit.
	*/
	std::vector<std::vector<CUnitType *> > Train;
	/**
	** The index is the unit that should be build, giving a table of all
	** units/buildings which could build this unit.
	*/
	std::vector<std::vector<CUnitType *> > Build;
	/**
	** The index is the upgrade that should be made, giving a table of all
	** units/buildings which could do the upgrade.
	*/
	std::vector<std::vector<CUnitType *> > Upgrade;
	/**
	** The index is the research that should be made, giving a table of all
	** units/buildings which could research this upgrade.
	*/
	std::vector<std::vector<CUnitType *> > Research;
	/**
	** The index is the unit that should be repaired, giving a table of all
	** units/buildings which could repair this unit.
	*/
	std::vector<std::vector<CUnitType *> > Repair;
	/**
	** The index is the unit-limit that should be solved, giving a table of all
	** units/buildings which could reduce this unit-limit.
	*/
	std::vector<std::vector<CUnitType *> > UnitLimit;
	/**
	** The index is the unit that should be made, giving a table of all
	** units/buildings which are equivalent.
	*/
	std::vector<std::vector<CUnitType *> > Equiv;

	/**
	** The index is the resource id - 1 (we can't mine TIME), giving a table of all
	** units/buildings/mines which can harvest this resource.
	*/
	std::vector<std::vector<CUnitType *> > Refinery;

	/**
	** The index is the resource id - 1 (we can't store TIME), giving a table of all
	** units/buildings/mines which can store this resource.
	*/
	std::vector<std::vector<CUnitType *> > Depots;
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern std::vector<CAiType *> AiTypes;   /// List of all AI types
extern AiHelper AiHelpers; /// AI helper variables

extern int UnitTypeEquivs[UnitTypeMax + 1]; /// equivalence between unittypes
extern PlayerAi *AiPlayer; /// Current AI player

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

//
// Resource manager
//
/// Add unit-type request to resource manager
extern void AiAddUnitTypeRequest(CUnitType &type, int count);
/// Add upgrade-to request to resource manager
extern void AiAddUpgradeToRequest(CUnitType &type);
/// Add research request to resource manager
extern void AiAddResearchRequest(CUpgrade *upgrade);
/// Periodic called resource manager handler
extern void AiResourceManager();
/// Ask the ai to explore around pos
extern void AiExplore(const Vec2i &pos, int exploreMask);
/// Make two unittypes be considered equals
extern void AiNewUnitTypeEquiv(CUnitType *a, CUnitType *b);
/// Remove any equivalence between unittypes
extern void AiResetUnitTypeEquiv();
/// Finds all equivalents units to a given one
extern int AiFindUnitTypeEquiv(const CUnitType &type, int *result);
/// Finds all available equivalents units to a given one, in the prefered order
extern int AiFindAvailableUnitTypeEquiv(const CUnitType &type, int *result);
extern int AiGetBuildRequestsCount(const PlayerAi &pai, int (&counter)[UnitTypeMax]);

extern void AiNewDepotRequest(CUnit &worker);

//
// Buildings
//
/// Find nice building place
extern int AiFindBuildingPlace(const CUnit &worker, const CUnitType &type, const Vec2i &nearPos, Vec2i *dpos);

//
// Forces
//
/// Cleanup units in force
extern void AiRemoveDeadUnitInForces();
/// Assign a new unit to a force
extern bool AiAssignToForce(CUnit &unit);
/// Assign a free units to a force
extern void AiAssignFreeUnitsToForce();
/// Attack with force at position
extern void AiAttackWithForceAt(unsigned int force, int x, int y);
/// Attack with force
extern void AiAttackWithForce(unsigned int force);
/// Attack with forces in array
extern void AiAttackWithForces(int *forces);

/// Periodic called force manager handler
extern void AiForceManager();

//
// Plans
//
/// Find a wall to attack
extern int AiFindWall(AiForce *force);
/// Plan the an attack
/// Send explorers around the map
extern void AiSendExplorers();
/// Enemy units in distance
extern int AiEnemyUnitsInDistance(const CPlayer &player, const CUnitType *type,
								  const Vec2i &pos, unsigned range);

//
// Magic
//
/// Check for magic
extern void AiCheckMagic();

//@}

#endif // !__AI_LOCAL_H__
