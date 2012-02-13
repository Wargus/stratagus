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
//      (c) Copyright 1998-2006 by Lutz Sammer and Jimmy Salmon
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
#ifndef __UNIT_CACHE_H__
#include "unit_cache.h"
#endif

#ifndef __VEC2I_H__
#include "vec2i.h"
#endif

//#include "vec2i.h"

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
	UnitActionReturnGoods,  /// unit returning any resource
	UnitActionTransformInto /// unit transform into type.
};

class CConstructionFrame;
class CUnit;
class CUnitType;
class CUpgrade;
class SpellType;
class CAnimation;

/**
**  Unit order structure.
*/
class COrder
{
public:
	COrder() : Goal(NULL), Range(0), MinRange(0), Width(0),
		Height(0), Action(UnitActionNone), CurrentResource(0)
	{
		goalPos.x = -1;
		goalPos.y = -1;
		memset(&Arg1, 0, sizeof (Arg1));
		memset(&Data, 0, sizeof (Data));
	}
	COrder(const COrder &ths);
	~COrder();

	void ReleaseRefs(CUnit &owner);
	COrder& operator=(const COrder &rhs);
	bool CheckRange() const;

	void Init() {
		Assert(Action != UnitActionResource
				|| (Action == UnitActionResource && Arg1.Resource.Mine == NULL));
		Action = UnitActionNone;
		Range = 0;
		MinRange = 0;
		Width = 0;
		Height = 0;
		CurrentResource = 0;
		Assert(!Goal);
		goalPos.x = -1;
		goalPos.y = -1;
		memset(&Arg1, 0, sizeof(Arg1));
		memset(&Data, 0, sizeof(Data));
	};

	bool HasGoal() const { return Goal != NULL; }
	CUnit * GetGoal() const { return Goal; };
	void SetGoal(CUnit *const new_goal);
	void ClearGoal();

	/**
	**  To remove pathfinder internals. Called if path destination changed.
	*/
	void NewResetPath() { Data.Move.Fast = 1; Data.Move.Length = 0; }


private:
	friend void CclParseOrder(lua_State *l, const CUnit &unit, COrder* order);

	CUnit *Goal;
public:
	int Range;              /// How far away
	unsigned int  MinRange; /// How far away minimum
	unsigned char Width;    /// Goal Width (used when Goal is not)
	unsigned char Height;   /// Goal Height (used when Goal is not)
	unsigned char Action;   /// global action
	unsigned char CurrentResource;	 //used in 	UnitActionResource and
										//UnitActionReturnGoods

	Vec2i goalPos;          /// or tile coordinate of destination

	union {
		Vec2i Patrol; /// position for patroling.
		struct {
			Vec2i Pos; /// position for terrain resource.
			CUnit *Mine;
		} Resource;
		SpellType *Spell;             /// spell when casting.
		CUpgrade *Upgrade;            /// upgrade.
		CUnitType *Type;        /// Unit-type argument used mostly for traning/building, etc.
	} Arg1;             /// Extra command argument.

	union _order_data_ {
	struct _order_move_ {
		unsigned short int Cycles;          /// how much Cycles we move.
		char Fast;                  /// Flag fast move (one step)
		char Length;                /// stored path length
#define MAX_PATH_LENGTH 28          /// max length of precalculated path
		char Path[MAX_PATH_LENGTH]; /// directions of stored path
	} Move; /// ActionMove,...
	struct _order_built_ {
		CUnit *Worker;              /// Worker building this unit
		int Progress;               /// Progress counter, in 1/100 cycles.
		int Cancel;                 /// Cancel construction
		CConstructionFrame *Frame;   /// Construction frame
	} Built; /// ActionBuilt,...
	struct _order_build_ {
		int Cycles;                 /// Cycles unit has been building for
	} Build; /// ActionBuild
	struct _order_resource_ {
		CUnit *Workers; //pointer to first assigned worker to this resource.
		int Assigned; /// how many units are assigned to harvesting from the resource.
		int Active; /// how many units are harvesting from the resource.
	} Resource; /// Resource still
	struct _order_resource_worker_ {
		int TimeToHarvest;          /// how much time until we harvest some more.
		unsigned DoneHarvesting:1;  /// Harvesting done, wait for action to break.
	} ResWorker; /// Worker harvesting
	struct _order_repair_ {
		int Cycles;                 /// Cycles unit has been repairing for
	} Repair; /// Repairing unit
	struct _order_research_ {
		CUpgrade *Upgrade;          /// Upgrade researched
	} Research; /// Research action
	struct _order_upgradeto_ {
		int Ticks; /// Ticks to complete
	} UpgradeTo; /// Upgrade to action
	struct _order_train_ {
		int Ticks;                  /// Ticks to complete
	} Train; /// Train units action
	} Data; /// Storage room for different commands
};




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
	/// Prepare set resources command
extern void CommandSetResource(int player, int resource, int value);
	/// Prepare shared vision command
extern void CommandSharedVision(int player, bool state, int opponent);
	/// Send any command
//extern void CommandAnyOrder(CUnit &unit, COrder *order, int flush);
	/// Move an order in command queue
extern void CommandMoveOrder(CUnit &unit, int src, int dst);

/*----------------------------------------------------------------------------
--  Actions: in action_<name>.c
----------------------------------------------------------------------------*/

extern void DropResource(CUnit &unit);
extern void ResourceGiveUp(CUnit &unit);
extern int GetNumWaitingWorkers(const CUnit &mine);
extern void AutoAttack(CUnit &unit, CUnitCache &targets, bool stand_ground);
extern void UnHideUnit(CUnit &unit);

typedef void HandleActionFunc(COrder& order, CUnit &unit);

	/// Generic still action
extern void ActionStillGeneric(CUnit &unit, bool stand_ground);
	/// Handle command still
extern HandleActionFunc HandleActionStill;
	/// Handle command stand ground
extern HandleActionFunc HandleActionStandGround;
	/// Handle command follow
extern HandleActionFunc HandleActionFollow;
	/// Generic move action
extern int DoActionMove(CUnit &unit);
	/// Handle command move
extern HandleActionFunc HandleActionMove;
	/// Handle command repair
extern HandleActionFunc HandleActionRepair;
	/// Handle command patrol
extern HandleActionFunc HandleActionPatrol;
	/// Show attack animation
extern void AnimateActionAttack(CUnit &unit);
	/// Handle command attack
extern HandleActionFunc  HandleActionAttack;
	/// Handle command board
extern HandleActionFunc HandleActionBoard;
	/// Handle command unload
extern HandleActionFunc HandleActionUnload;
	/// Handle command resource
extern HandleActionFunc HandleActionResource;
	/// Handle command return
extern HandleActionFunc HandleActionReturnGoods;
	/// Handle command die
extern HandleActionFunc HandleActionDie;
	/// Handle command build
extern HandleActionFunc HandleActionBuild;
	/// Handle command built
extern HandleActionFunc HandleActionBuilt;
	/// Handle command train
extern HandleActionFunc HandleActionTrain;
	/// Handle command upgrade to
extern HandleActionFunc HandleActionUpgradeTo;
	/// Handle command transform into
extern HandleActionFunc HandleActionTransformInto;
	/// Handle command upgrade
extern HandleActionFunc HandleActionUpgrade;
	/// Handle command research
extern HandleActionFunc HandleActionResearch;
	/// Handle command spellcast
extern HandleActionFunc HandleActionSpellCast;

/*----------------------------------------------------------------------------
--  Actions: actions.c
----------------------------------------------------------------------------*/

	/// Handle the animation of a unit
extern int UnitShowAnimationScaled(CUnit &unit, const CAnimation *anim, int scale);
	/// Handle the animation of a unit
extern int UnitShowAnimation(CUnit &unit, const CAnimation *anim);
	/// Handle the actions of all units each game cycle
extern void UnitActions();

//@}

#endif // !__ACTIONS_H__
