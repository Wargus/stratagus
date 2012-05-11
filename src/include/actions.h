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
/**@name actions.h - The actions headerfile. */
//
//      (c) Copyright 1998-2012 by Lutz Sammer and Jimmy Salmon
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

#ifndef __ACTIONS_H__
#define __ACTIONS_H__

//@{

#include "unitptr.h"
#include "vec2i.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

enum _diplomacy_ {
	DiplomacyAllied,   /// Ally with opponent
	DiplomacyNeutral,  /// Don't attack be neutral
	DiplomacyEnemy,    /// Attack opponent
	DiplomacyCrazy     /// Ally and attack opponent
}; /// Diplomacy states for CommandDiplomacy

/**
**  All possible unit actions.
**
**  @note  Always change the table ::HandleActionTable
**
**  @see HandleActionTable
*/
enum UnitAction {
	UnitActionNone,         /// No valid action

	UnitActionStill,        /// unit stand still, does nothing
	UnitActionStandGround,  /// unit stands ground
	UnitActionFollow,       /// unit follows units
	UnitActionMove,         /// unit moves to position/unit
	UnitActionAttack,       /// unit attacks position/unit
	UnitActionAttackGround, /// unit attacks ground
	UnitActionDie,          /// unit dies

	UnitActionSpellCast,    /// unit casts spell

	UnitActionTrain,        /// building is training
	UnitActionUpgradeTo,    /// building is upgrading itself
	UnitActionResearch,     /// building is researching spell
	UnitActionBuilt,      /// building is under construction

	// Compound actions
	UnitActionBoard,        /// unit entering transporter
	UnitActionUnload,       /// unit leaving transporter
	UnitActionPatrol,       /// unit paroling area
	UnitActionBuild,        /// unit builds building

	UnitActionRepair,       /// unit repairing
	UnitActionResource,     /// unit harvesting resources
	UnitActionTransformInto /// unit transform into type.
};

class CAnimation;
class CConstructionFrame;
class CFile;
class CUnit;
class CUnitType;
class CUpgrade;
class PathFinderInput;
class SpellType;
class CViewport;
struct lua_State;

/**
**  Unit order structure.
*/
class COrder
{
public:
	explicit COrder(int action) : Goal(), Action(action), Finished(false) {
	}
	virtual ~COrder();

	virtual COrder *Clone() const = 0;
	virtual void Execute(CUnit &unit) = 0;
	virtual void Cancel(CUnit &unit) {}
	virtual bool IsValid() const = 0;

	virtual PixelPos Show(const CViewport &vp, const PixelPos &lastScreenPos) const = 0;

	virtual void OnAnimationAttack(CUnit &unit);

	virtual void Save(CFile &file, const CUnit &unit) const = 0;
	bool ParseGenericData(lua_State *l, int &j, const char *value);
	virtual bool ParseSpecificData(lua_State *l, int &j, const char *value, const CUnit &unit) = 0;

	virtual void UpdateUnitVariables(CUnit &unit) const {}
	virtual void FillSeenValues(CUnit &unit) const;
	virtual void AiUnitKilled(CUnit &unit);

	virtual void UpdatePathFinderData(PathFinderInput &input) = 0;

	bool HasGoal() const { return Goal != NULL; }
	CUnit *GetGoal() const { return Goal; };
	void SetGoal(CUnit *const new_goal);
	void ClearGoal();

	virtual bool OnAiHitUnit(CUnit &unit, CUnit *attacker, int /*damage*/);

	static COrder *NewActionAttack(const CUnit &attacker, CUnit &target);
	static COrder *NewActionAttack(const CUnit &attacker, const Vec2i &dest);
	static COrder *NewActionAttackGround(const CUnit &attacker, const Vec2i &dest);
	static COrder *NewActionBoard(CUnit &unit);
	static COrder *NewActionBuild(const CUnit &builder, const Vec2i &pos, CUnitType &building);
	static COrder *NewActionBuilt(CUnit &builder, CUnit &unit);
	static COrder *NewActionDie();
	static COrder *NewActionFollow(CUnit &dest);
	static COrder *NewActionMove(const Vec2i &pos);
	static COrder *NewActionPatrol(const Vec2i &currentPos, const Vec2i &dest);
	static COrder *NewActionRepair(CUnit &unit, CUnit &target);
	static COrder *NewActionRepair(const Vec2i &pos);
	static COrder *NewActionResearch(CUnit &unit, CUpgrade &upgrade);
	static COrder *NewActionResource(CUnit &harvester, const Vec2i &pos);
	static COrder *NewActionResource(CUnit &harvester, CUnit &mine);
	static COrder *NewActionReturnGoods(CUnit &harvester, CUnit *depot);
	static COrder *NewActionSpellCast(SpellType &spell, const Vec2i &pos, CUnit *target);
	static COrder *NewActionStandGround();
	static COrder *NewActionStill();
	static COrder *NewActionTrain(CUnit &trainer, CUnitType &type);
	static COrder *NewActionTransformInto(CUnitType &type);
	static COrder *NewActionUnload(const Vec2i &pos, CUnit *what);
	static COrder *NewActionUpgradeTo(CUnit &unit, CUnitType &type);

protected:
	void UpdatePathFinderData_NotCalled(PathFinderInput &input);

private:
	CUnitPtr Goal;
public:
	const unsigned char Action;   /// global action
	bool Finished; /// true when order is finish
};

typedef COrder *COrderPtr;


/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern unsigned SyncHash;  /// Hash calculated to find sync failures

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Commands: in command.c
----------------------------------------------------------------------------*/

/**
**  This function gives a unit a new command. If the command is given
**  by the user the function with Send prefix should be used.
*/

/// Prepare command quit
extern void CommandQuit(int player);
/// Prepare command stop
extern void CommandStopUnit(CUnit &unit);
/// Prepare command stand ground
extern void CommandStandGround(CUnit &unit, int flush);
/// Prepare command follow
extern void CommandFollow(CUnit &unit, CUnit &dest, int flush);
/// Prepare command move
extern void CommandMove(CUnit &unit, const Vec2i &pos, int flush);
/// Prepare command repair
extern void CommandRepair(CUnit &unit, const Vec2i &pos, CUnit *dest, int flush);
/// Send auto repair command
extern void CommandAutoRepair(CUnit &unit, int on);
/// Prepare command attack
extern void CommandAttack(CUnit &unit, const Vec2i &pos, CUnit *dest, int flush);
/// Prepare command attack ground
extern void CommandAttackGround(CUnit &unit, const Vec2i &pos, int flush);
/// Prepare command patrol
extern void CommandPatrolUnit(CUnit &unit, const Vec2i &pos, int flush);
/// Prepare command board
extern void CommandBoard(CUnit &unit, CUnit &dest, int flush);
/// Prepare command unload
extern void CommandUnload(CUnit &unit, const Vec2i &pos, CUnit *what, int flush);
/// Prepare command build
extern void CommandBuildBuilding(CUnit &unit, const Vec2i &pos, CUnitType &, int flush);
/// Prepare command dismiss
extern void CommandDismiss(CUnit &unit);
/// Prepare command resource location
extern void CommandResourceLoc(CUnit &unit, const Vec2i &pos, int flush);
/// Prepare command resource
extern void CommandResource(CUnit &unit, CUnit &dest, int flush);
/// Prepare command return
extern void CommandReturnGoods(CUnit &unit, CUnit *goal, int flush);
/// Prepare command train
extern void CommandTrainUnit(CUnit &unit, CUnitType &what, int flush);
/// Prepare command cancel training
extern void CommandCancelTraining(CUnit &unit, int slot, const CUnitType *type);
/// Prepare command upgrade to
extern void CommandUpgradeTo(CUnit &unit, CUnitType &what, int flush);
/// immediate transforming into type.
extern void CommandTransformIntoType(CUnit &unit, CUnitType &type);
/// Prepare command cancel upgrade to
extern void CommandCancelUpgradeTo(CUnit &unit);
/// Prepare command research
extern void CommandResearch(CUnit &unit, CUpgrade *what, int flush);
/// Prepare command cancel research
extern void CommandCancelResearch(CUnit &unit);
/// Prepare command spellcast
extern void CommandSpellCast(CUnit &unit, const Vec2i &pos, CUnit *dest, SpellType *spell, int flush);
/// Prepare command auto spellcast
extern void CommandAutoSpellCast(CUnit &unit, int spellid, int on);
/// Prepare diplomacy command
extern void CommandDiplomacy(int player, int state, int opponent);
/// Prepare shared vision command
extern void CommandSharedVision(int player, bool state, int opponent);

/*----------------------------------------------------------------------------
--  Actions: in action_<name>.c
----------------------------------------------------------------------------*/

extern int GetNumWaitingWorkers(const CUnit &mine);
extern bool AutoAttack(CUnit &unit);
extern bool AutoRepair(CUnit &unit);
extern bool AutoCast(CUnit &unit);
extern void UnHideUnit(CUnit &unit);

/// Generic move action
extern int DoActionMove(CUnit &unit);
/// Show attack animation
extern void AnimateActionAttack(CUnit &unit, COrder &order);

/*----------------------------------------------------------------------------
--  Actions: actions.c
----------------------------------------------------------------------------*/

/// Parse order
extern void CclParseOrder(lua_State *l, CUnit &unit, COrderPtr *order);

/// Handle the actions of all units each game cycle
extern void UnitActions();

//@}

#endif // !__ACTIONS_H__
