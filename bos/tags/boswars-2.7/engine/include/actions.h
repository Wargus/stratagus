//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name actions.h - The actions headerfile. */
//
//      (c) Copyright 1998-2007 by Lutz Sammer and Jimmy Salmon
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

#ifndef __ACTIONS_H__
#define __ACTIONS_H__

//@{

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
class SpellType;
class COrder;
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
	/// Prepare command resource
extern void CommandResource(CUnit *unit, CUnit *dest,
	int flush);
	/// Prepare command train
extern void CommandTrainUnit(CUnit *unit, CUnitType *what,
	int flush);
	/// Prepare command cancel training
extern void CommandCancelTraining(CUnit *unit, int slot,
	const CUnitType *type);
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
extern void CommandAnyOrder(CUnit *unit, COrder *order, int flush);
	/// Move an order in command queue
extern void CommandMoveOrder(CUnit *unit, int src, int dst);

extern bool TerrainAllowsTraining(const CUnit *trainer, const CUnitType *traineeType);

/*----------------------------------------------------------------------------
--  Actions: in action_<name>.c
----------------------------------------------------------------------------*/

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
	/// Handle command die
extern void HandleActionDie(CUnit *unit);
	/// Handle command build
extern void HandleActionBuild(CUnit *unit);
	/// Handle command built
extern void HandleActionBuilt(CUnit *unit);
	/// Handle command train
extern void HandleActionTrain(CUnit *unit);
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
