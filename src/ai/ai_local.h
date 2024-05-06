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

#include <array>
#include <memory>
#include <optional>
#include <vector>

#include "upgrade_structs.h" // MaxCost
#include "vec2i.h"

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
	AiRequestType() = default;

	unsigned int Count = 0;    /// elements in table
	CUnitType *Type = nullptr; /// the type
};

/**
**  Ai unit-type in a force.
*/
class AiUnitType
{
public:
	AiUnitType() = default;

	unsigned int Want = 0;     /// number of this unit-type wanted
	CUnitType *Type = nullptr; /// unit-type self
};

/**
**  Roles for forces
*/
enum class AiForceRole {
	Default = 0, /// So default is attacking
	Attack = 0, /// Force should attack
	Defend      /// Force should defend
};

std::optional<AiForceRole> AiForceRoleFromString(std::string_view);
std::string_view ToString(AiForceRole);

enum class AiForceAttackingState {
	Free = -1,
	Waiting = 0,
	Boarding,
	GoingToRallyPoint,
	AttackingWithTransporter,
	Attacking,
};

#define AI_WAIT_ON_RALLY_POINT 60          /// Max seconds AI units will wait on rally point

/**
**  Define an AI force.
**
**  A force is a group of units belonging together.
*/
class AiForce
{
	friend class AiForceManager;
public:
	AiForce() = default;

	void Remove(CUnit &unit)
	{
		auto it = ranges::find(Units, &unit);
		if (it != Units.end()) {
			Units.erase(it);
			InternalRemoveUnit(&unit);
		}
	}

	/**
	**  Reset the force. But don't change its role and its demand.
	*/
	void Reset(bool types = false)
	{
		FormerForce = -1;
		Completed = false;
		Defending = false;
		Attacking = false;
		WaitOnRallyPoint = AI_WAIT_ON_RALLY_POINT;
		if (types) {
			UnitTypes.clear();
			State = AiForceAttackingState::Free;
		} else {
			State = AiForceAttackingState::Waiting;
		}
		for (CUnit *unit : Units) {
			InternalRemoveUnit(unit);
		}
		Units.clear();
		HomePos.x = HomePos.y = GoalPos.x = GoalPos.y = -1;
	}
	size_t Size() const { return Units.size(); }

	bool IsAttacking() const { return (!Defending && Attacking); }

	void Attack(const Vec2i &pos);
	void RemoveDeadUnit();
	bool PlanAttack();

	void ReturnToHome();
	std::optional<Vec2i> NewRallyPoint(const Vec2i &startPos);
	void Insert(CUnit &unit);

private:
	std::vector<std::size_t> CountTypes() const;
	bool IsBelongsTo(const CUnitType &type);

	void Update();

	static void InternalRemoveUnit(CUnit *unit);

public:
	bool Completed = false;    /// Flag saying force is complete build
	bool Defending = false;    /// Flag saying force is defending
	bool Attacking = false;    /// Flag saying force is attacking
	AiForceRole Role = AiForceRole::Default;  /// Role of the force

	std::vector<AiUnitType> UnitTypes; /// Count and types of unit-type
	std::vector<CUnit *> Units;  /// Units in the force

	// If attacking
	int FormerForce = -1;             /// Original force number
	AiForceAttackingState State = AiForceAttackingState::Free; /// Attack state
	Vec2i GoalPos{-1, -1}; /// Attack point tile map position
	Vec2i HomePos{-1, -1}; /// Return after attack tile map position
	int WaitOnRallyPoint = AI_WAIT_ON_RALLY_POINT; /// Counter for waiting on rally point
};

// forces
#define AI_MAX_FORCES 50                           /// How many forces are supported
#define AI_MAX_FORCE_INTERNAL (AI_MAX_FORCES / 2)  /// The forces after AI_MAX_FORCE_INTERNAL are for internal use

/**
**  AI force manager.
**
**  A Forces container for the force manager to handle
*/
class AiForceManager
{
public:
	AiForceManager();

	size_t Size() const { return forces.size(); }

	const AiForce &operator[](unsigned int index) const { return forces[index]; }
	AiForce &operator[](unsigned int index) { return forces[index]; }

	int getIndex(const AiForce &force) const
	{
		for (unsigned int i = 0; i < forces.size(); ++i) {
			if (&force == &forces[i]) {
				return i;
			}
		}
		throw std::runtime_error("Invalid force");
	}

	unsigned int getScriptForce(unsigned int index)
	{
		if (script[index] == -1) {
			script[index] = FindFreeForce();
		}
		return script[index];
	}

	std::optional<int> GetForce(const CUnit &unit);
	void RemoveDeadUnit();
	bool Assign(CUnit &unit, int force = -1);
	void Update();
	unsigned int FindFreeForce(AiForceRole role = AiForceRole::Default, int begin = 0);
	void CheckUnits(std::array<int, UnitTypeMax> &counter);

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
	AiBuildQueue() = default;

public:
	unsigned int Want = 0;  /// requested number
	unsigned int Made = 0;  /// built number
	CUnitType *Type = nullptr; /// unit-type
	unsigned long Wait = 0; /// wait until this cycle
	Vec2i Pos{-1, -1}; /// build near pos on map
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
	PlayerAi() = default;

public:
	CPlayer *Player = nullptr;  /// Engine player structure
	CAiType *AiType = nullptr;  /// AI type of this player AI
	// controller
	std::string Script;         /// Script executed
	unsigned long SleepCycles = 0; /// Cycles to sleep

	AiForceManager Force;  /// Forces controlled by AI

	// resource manager
	int Reserve[MaxCosts]{}; /// Resources to keep in reserve
	int Used[MaxCosts]{};    /// Used resources
	int Needed[MaxCosts]{};  /// Needed resources
	int Collect[MaxCosts]{}; /// Collect % of resources
	int NeededMask = 0;      /// Mask for needed resources
	bool NeedSupply = false; /// Flag need food
	bool ScriptDebug = false;/// Flag script debuging on/off
	bool BuildDepots = true; /// Build new depots if nessesary

	std::vector<AiExplorationRequest> FirstExplorationRequest;/// Requests for exploration
	unsigned long LastExplorationGameCycle = 0;   /// When did the last explore occur?
	unsigned long LastCanNotMoveGameCycle = 0;    /// Last can not move cycle
	std::vector<AiRequestType> UnitTypeRequests;  /// unit-types to build/train request,priority list
	std::vector<CUnitType *> UpgradeToRequests;   /// Upgrade to unit-type requested and priority list
	std::vector<CUpgrade *> ResearchRequests;     /// Upgrades requested and priority list
	std::vector<AiBuildQueue> UnitTypeBuilt;      /// What the resource manager should build
	int LastRepairBuilding = 0;                   /// Last building checked for repair in this turn
};

/**
**  AI Helper.
**
**  Contains information needed for the AI. If the AI needs an unit or
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
	std::vector<std::vector<CUnitType *>> &Train();
	/**
	** The index is the unit that should be build, giving a table of all
	** units/buildings which could build this unit.
	*/
	std::vector<std::vector<CUnitType *>> &Build();
	/**
	** The index is the upgrade that should be made, giving a table of all
	** units/buildings which could do the upgrade.
	*/
	std::vector<std::vector<CUnitType *>> &Upgrade();
	/**
	** The index is the research that should be made, giving a table of all
	** units/buildings which could research this upgrade. This table only
	** includes those unit types which have the research defined as a button
	** without the "check-single-research" restriction.
	*/
	std::vector<std::vector<CUnitType *>> &Research();
	/**
	** The index is the research that should be made, giving a table of all
	** units/buildings which could research this upgrade. This table only
	** includes those unit types which have the research defined as a button
	** with the "check-single-research" restriction.
	*/
	std::vector<std::vector<CUnitType *>> &SingleResearch();
	/**
	** The index is the unit that should be repaired, giving a table of all
	** units/buildings which could repair this unit.
	*/
	std::vector<std::vector<CUnitType *>> &Repair();
	/**
	** The index is the unit-limit that should be solved, giving a table of all
	** units/buildings which could reduce this unit-limit.
	*/
	std::vector<std::vector<CUnitType *>> &UnitLimit();
	/**
	** The index is the unit that should be made, giving a table of all
	** units/buildings which are equivalent.
	*/
	std::vector<std::vector<CUnitType *>> &Equiv();

	/**
	** The index is the resource id - 1 (we can't mine TIME), giving a table of all
	** units/buildings/mines which can harvest this resource.
	*/
	std::vector<std::vector<CUnitType *>> &Refinery();

	/**
	** The index is the resource id - 1 (we can't store TIME), giving a table of all
	** units/buildings/mines which can store this resource.
	*/
	std::vector<std::vector<CUnitType *>> &Depots();
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern std::vector<std::unique_ptr<CAiType>> AiTypes; /// List of all AI types
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
extern void AiNewUnitTypeEquiv(const CUnitType &a, const CUnitType &b);
/// Remove any equivalence between unittypes
extern void AiResetUnitTypeEquiv();
/// Finds all equivalents unittypes to a given one
extern std::vector<int> AiFindUnitTypeEquiv(const CUnitType &type);
/// Finds all available equivalents units to a given one, in the preferred order
extern std::vector<int> AiFindAvailableUnitTypeEquiv(const CUnitType &type);
extern std::array<int, UnitTypeMax> AiGetBuildRequestsCount(const PlayerAi &pai);

extern void AiNewDepotRequest(CUnit &worker);
extern std::pair<CUnit *, CUnit *> AiGetSuitableDepot(const CUnit &worker, const CUnit &oldDepot);

//
// Buildings
//
/// Find nice building place
extern std::optional<Vec2i> AiFindBuildingPlace(const CUnit &worker, const CUnitType &type, const Vec2i &nearPos);

//
// Forces
//
/// Cleanup units in force
extern void AiRemoveDeadUnitInForces();
/// Assign a new unit to a force
extern bool AiAssignToForce(CUnit &unit);
/// Assign a free units to a force
extern void AiAssignFreeUnitsToForce(int force = -1);
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
extern bool AiFindWall(AiForce &force);
/// Plan the an attack
/// Send explorers around the map
extern void AiSendExplorers();
/// Check if there are enemy units in a given range (optionally of type)
extern bool AiEnemyUnitsInDistance(const CPlayer &player, const CUnitType *type,
								  const Vec2i &pos, unsigned range);

//
// Magic
//
/// Check for magic
extern void AiCheckMagic();

//@}

#endif // !__AI_LOCAL_H__
