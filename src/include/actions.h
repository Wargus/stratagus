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
//      $Id$

#ifndef __ACTIONS_H__
#define __ACTIONS_H__

//@{
#ifndef __UNIT_CACHE_H__
#include "unit_cache.h"
#endif

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

enum _diplomacy_ {
	DiplomacyAllied,   /// Ally with opponent
	DiplomacyNeutral,  /// Don't attack be neutral
	DiplomacyEnemy,    /// Attack opponent
	DiplomacyCrazy,    /// Ally and attack opponent
}; /// Diplomacy states for CommandDiplomacy

class CUnit;
class CUnitType;
class CUpgrade;
class SpellType;
class CAnimation;

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
extern void CommandStopUnit(CUnit *unit);
	/// Prepare command stand ground
extern void CommandStandGround(CUnit *unit, int flush);
	/// Prepare command follow
extern void CommandFollow(CUnit *unit, CUnit *dest,
	int flush);
	/// Prepare command move
extern void CommandMove(CUnit *unit, int x, int y, int flush);
	/// Prepare command repair
extern void CommandRepair(CUnit *unit, int x, int y,
	CUnit *dest, int flush);
	/// Send auto repair command
extern void CommandAutoRepair(CUnit *unit, int on);
	/// Prepare command attack
extern void CommandAttack(CUnit *unit, int x, int y,
	CUnit *dest, int flush);
	/// Prepare command attack ground
extern void CommandAttackGround(CUnit *unit, int x, int y, int flush);
	/// Prepare command patrol
extern void CommandPatrolUnit(CUnit *unit, int x, int y, int flush);
	/// Prepare command board
extern void CommandBoard(CUnit *unit, CUnit *dest, int flush);
	/// Prepare command unload
extern void CommandUnload(CUnit *unit, int x, int y,
	CUnit *what, int flush);
	/// Prepare command build
extern void CommandBuildBuilding(CUnit *, int x, int y,
	CUnitType *, int);
	/// Prepare command dismiss
extern void CommandDismiss(CUnit *unit);
	/// Prepare command resource location
extern void CommandResourceLoc(CUnit *unit, int x, int y, int flush);
	/// Prepare command resource
extern void CommandResource(CUnit *unit, CUnit *dest,
	int flush);
	/// Prepare command return
extern void CommandReturnGoods(CUnit *unit, CUnit *goal,
	int flush);
	/// Prepare command train
extern void CommandTrainUnit(CUnit *unit, CUnitType *what,
	int flush);
	/// Prepare command cancel training
extern void CommandCancelTraining(CUnit *unit, int slot,
	const CUnitType *type);
	/// Prepare command upgrade to
extern void CommandUpgradeTo(CUnit *unit, CUnitType *what,
	int flush);
	/// immediate transforming into type.
extern void CommandTransformIntoType(CUnit *unit, CUnitType *type);
	/// Prepare command cancel upgrade to
extern void CommandCancelUpgradeTo(CUnit *unit);
	/// Prepare command research
extern void CommandResearch(CUnit *unit, CUpgrade *what, int flush);
	/// Prepare command cancel research
extern void CommandCancelResearch(CUnit *unit);
	/// Prepare command spellcast
extern void CommandSpellCast(CUnit *unit, int x, int y,
	CUnit *dest, SpellType *spell, int flush);
	/// Prepare command auto spellcast
extern void CommandAutoSpellCast(CUnit *unit, int spellid, int on);
	/// Prepare diplomacy command
extern void CommandDiplomacy(int player, int state, int opponent);
	/// Prepare shared vision command
extern void CommandSharedVision(int player, bool state, int opponent);
	/// Send any command
//extern void CommandAnyOrder(CUnit *unit, COrder *order, int flush);
	/// Move an order in command queue
extern void CommandMoveOrder(CUnit *unit, int src, int dst);

/*----------------------------------------------------------------------------
--  Actions: in action_<name>.c
----------------------------------------------------------------------------*/

extern void DropResource(CUnit *unit);
extern void ResourceGiveUp(CUnit *unit);
extern int GetNumWaitingWorkers(const CUnit *mine);
extern void AutoAttack(CUnit *unit, CUnitCache &targets, bool stand_ground);
extern void MapUnmarkUnitGuard(CUnit *unit);
extern void UnHideUnit(CUnit *unit);

	/// Generic still action
extern void ActionStillGeneric(CUnit *unit, bool stand_ground);
	/// Handle command still
extern void HandleActionStill(CUnit *unit);
	/// Handle command stand ground
extern void HandleActionStandGround(CUnit *unit);
	/// Handle command follow
extern void HandleActionFollow(CUnit *unit);
	/// Generic move action
extern int DoActionMove(CUnit *unit);
	/// Handle command move
extern void HandleActionMove(CUnit *unit);
	/// Handle command repair
extern void HandleActionRepair(CUnit *unit);
	/// Handle command patrol
extern void HandleActionPatrol(CUnit *unit);
	/// Show attack animation
extern void AnimateActionAttack(CUnit *unit);
	/// Handle command attack
extern void HandleActionAttack(CUnit *unit);
	/// Handle command board
extern void HandleActionBoard(CUnit *unit);
	/// Handle command unload
extern void HandleActionUnload(CUnit *unit);
	/// Handle command resource
extern void HandleActionResource(CUnit *unit);
	/// Handle command return
extern void HandleActionReturnGoods(CUnit *unit);
	/// Handle command die
extern void HandleActionDie(CUnit *unit);
	/// Handle command build
extern void HandleActionBuild(CUnit *unit);
	/// Handle command built
extern void HandleActionBuilt(CUnit *unit);
	/// Handle command train
extern void HandleActionTrain(CUnit *unit);
	/// Handle command upgrade to
extern void HandleActionUpgradeTo(CUnit *unit);
	/// Handle command transform into
extern void HandleActionTransformInto(CUnit *unit);
	/// Handle command upgrade
extern void HandleActionUpgrade(CUnit *unit);
	/// Handle command research
extern void HandleActionResearch(CUnit *unit);
	/// Handle command spellcast
extern void HandleActionSpellCast(CUnit *unit);

/*----------------------------------------------------------------------------
--  Actions: actions.c
----------------------------------------------------------------------------*/

	/// Handle the animation of a unit
extern int UnitShowAnimationScaled(CUnit *unit, const CAnimation *anim, int scale);
	/// Handle the animation of a unit
extern int UnitShowAnimation(CUnit *unit, const CAnimation *anim);
	/// Handle the actions of all units each game cycle
extern void UnitActions(void);
	/// Unload a unit.
extern int UnloadUnit(CUnit *unit);
//@}

#endif // !__ACTIONS_H__
