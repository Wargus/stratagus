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
//      (c) Copyright 1998-2005 by Lutz Sammer
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

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

enum _diplomacy_ {
	DiplomacyAllied,   /// Ally with opponent
	DiplomacyNeutral,  /// Don't attack be neutral
	DiplomacyEnemy,    /// Attack opponent
	DiplomacyCrazy,    /// Ally and attack opponent
}; /// Diplomacy states for CommandDiplomacy

struct _unit_;
struct _unit_type_;
struct _upgrade_;
struct _spell_type_;
struct _order_;
struct _animation_;

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
**  This functions gives an unit a new command. If the command is given
**  by the user the function with Send prefix should be used.
*/

	/// Prepare command quit
extern void CommandQuit(int player);
	/// Prepare command stop
extern void CommandStopUnit(struct _unit_* unit);
	/// Prepare command stand ground
extern void CommandStandGround(struct _unit_* unit, int flush);
	/// Prepare command follow
extern void CommandFollow(struct _unit_* unit, struct _unit_* dest,
	int flush);
	/// Prepare command move
extern void CommandMove(struct _unit_* unit, int x, int y, int flush);
	/// Prepare command repair
extern void CommandRepair(struct _unit_* unit, int x, int y,
	struct _unit_* dest, int flush);
	/// Send auto repair command
extern void CommandAutoRepair(struct _unit_* unit, int on);
	/// Prepare command attack
extern void CommandAttack(struct _unit_* unit, int x, int y,
	struct _unit_* dest, int flush);
	/// Prepare command attack ground
extern void CommandAttackGround(struct _unit_* unit, int x, int y, int flush);
	/// Prepare command patrol
extern void CommandPatrolUnit(struct _unit_* unit, int x, int y, int flush);
	/// Prepare command board
extern void CommandBoard(struct _unit_* unit, struct _unit_* dest, int flush);
	/// Prepare command unload
extern void CommandUnload(struct _unit_* unit, int x, int y,
	struct _unit_* what, int flush);
	/// Prepare command build
extern void CommandBuildBuilding(struct _unit_*, int x, int y,
	struct _unit_type_*, int);
	/// Prepare command dismiss
extern void CommandDismiss(struct _unit_* unit);
	/// Prepare command resource location
extern void CommandResourceLoc(struct _unit_* unit, int x, int y, int flush);
	/// Prepare command resource
extern void CommandResource(struct _unit_* unit, struct _unit_* dest,
	int flush);
	/// Prepare command return
extern void CommandReturnGoods(struct _unit_* unit, struct _unit_* goal,
	int flush);
	/// Prepare command train
extern void CommandTrainUnit(struct _unit_* unit, struct _unit_type_* what,
	int flush);
	/// Prepare command cancel training
extern void CommandCancelTraining(struct _unit_* unit, int slot,
	const struct _unit_type_* type);
	/// Prepare command upgrade to
extern void CommandUpgradeTo(struct _unit_* unit, struct _unit_type_* what,
	int flush);
	/// Prepare command cancel upgrade to
extern void CommandCancelUpgradeTo(struct _unit_* unit);
	/// Prepare command research
extern void CommandResearch(struct _unit_* unit, struct _upgrade_* what,
	int flush);
	/// Prepare command cancel research
extern void CommandCancelResearch(struct _unit_* unit);
	/// Prepare command spellcast
extern void CommandSpellCast(struct _unit_* unit, int x, int y,
	struct _unit_* dest, struct _spell_type_* spell, int flush);
	/// Prepare command auto spellcast
extern void CommandAutoSpellCast(struct _unit_* unit, int spellid, int on);
	/// Prepare diplomacy command
extern void CommandDiplomacy(int player, int state, int opponent);
	/// Prepare shared vision command
extern void CommandSharedVision(int player, int state, int opponent);
	/// Send any command
extern void CommandAnyOrder(struct _unit_* unit, struct _order_ * order, int flush);
	/// Move an order in command queue
extern void CommandMoveOrder(struct _unit_* unit, int src, int dst);

/*----------------------------------------------------------------------------
--  Actions: in action_<name>.c
----------------------------------------------------------------------------*/

	/// Generic still action
extern void ActionStillGeneric(struct _unit_* unit, int ground);
	/// Handle command still
extern void HandleActionStill(struct _unit_* unit);
	/// Handle command stand ground
extern void HandleActionStandGround(struct _unit_* unit);
	/// Handle command follow
extern void HandleActionFollow(struct _unit_* unit);
	/// Generic move action
extern int DoActionMove(struct _unit_* unit);
	/// Handle command move
extern void HandleActionMove(struct _unit_* unit);
	/// Handle command repair
extern void HandleActionRepair(struct _unit_* unit);
	/// Handle command patrol
extern void HandleActionPatrol(struct _unit_* unit);
	/// Show attack animation
extern void AnimateActionAttack(struct _unit_* unit);
	/// Handle command attack
extern void HandleActionAttack(struct _unit_* unit);
	/// Handle command board
extern void HandleActionBoard(struct _unit_* unit);
	/// Handle command unload
extern void HandleActionUnload(struct _unit_* unit);
	/// Handle command resource
extern void HandleActionResource(struct _unit_* unit);
	/// Handle command return
extern void HandleActionReturnGoods(struct _unit_* unit);
	/// Handle command die
extern void HandleActionDie(struct _unit_* unit);
	/// Handle command build
extern void HandleActionBuild(struct _unit_* unit);
	/// Handle command built
extern void HandleActionBuilt(struct _unit_* unit);
	/// Handle command train
extern void HandleActionTrain(struct _unit_* unit);
	/// Handle command upgrade to
extern void HandleActionUpgradeTo(struct _unit_* unit);
	/// Handle command upgrade
extern void HandleActionUpgrade(struct _unit_* unit);
	/// Handle command research
extern void HandleActionResearch(struct _unit_* unit);
	/// Handle command spellcast
extern void HandleActionSpellCast(struct _unit_* unit);

/*----------------------------------------------------------------------------
--  Actions: actions.c
----------------------------------------------------------------------------*/

	/// Handle the animation of a unit
extern int UnitShowAnimationScaled(struct _unit_* unit, const struct _animation_* anim, int scale);
	/// Handle the animation of a unit
extern int UnitShowAnimation(struct _unit_* unit, const struct _animation_* anim);
	/// Handle the animation of a unit
extern int UnitShowAnimation(struct _unit_* unit, const struct _animation_* animation);
	/// Handle the actions of all units each game cycle
extern void UnitActions(void);
	/// Unload an unit.
extern int UnloadUnit(struct _unit_* unit);
//@}

#endif // !__ACTIONS_H__
