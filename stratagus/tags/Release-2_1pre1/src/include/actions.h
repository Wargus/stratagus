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
//      (c) Copyright 1998-2004 by Lutz Sammer
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
--  Includes
----------------------------------------------------------------------------*/

#include "unittype.h"
#include "unit.h"
#include "upgrade.h"
#include "spells.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

enum _diplomacy_ {
	DiplomacyAllied,   ///< Ally with opponent
	DiplomacyNeutral,  ///< Don't attack be neutral
	DiplomacyEnemy,    ///< Attack opponent
	DiplomacyCrazy,    ///< Ally and attack opponent
}; ///< Diplomacy states for CommandDiplomacy

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern unsigned SyncHash;  ///< Hash calculated to find sync failures

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Commands: in command.c
----------------------------------------------------------------------------*/

/*
**  This functions gives an unit a new command. If the command is given
**  by the user the function with Send prefix should be used.
*/

	/// Prepare command quit
extern void CommandQuit(int player);
	/// Prepare command stop
extern void CommandStopUnit(Unit* unit);
	/// Prepare command stand ground
extern void CommandStandGround(Unit* unit,int flush);
	/// Prepare command follow
extern void CommandFollow(Unit* unit,Unit* dest,int flush);
	/// Prepare command move
extern void CommandMove(Unit* unit,int x,int y,int flush);
	/// Prepare command repair
extern void CommandRepair(Unit* unit,int x,int y,Unit* dest,int flush);
	/// Prepare command attack
extern void CommandAttack(Unit* unit,int x,int y,Unit* dest,int flush);
	/// Prepare command attack ground
extern void CommandAttackGround(Unit* unit,int x,int y,int flush);
	/// Prepare command patrol
extern void CommandPatrolUnit(Unit* unit,int x,int y,int flush);
	/// Prepare command board
extern void CommandBoard(Unit* unit,Unit* dest,int flush);
	/// Prepare command unload
extern void CommandUnload(Unit* unit,int x,int y,Unit* what,int flush);
	/// Prepare command build
extern void CommandBuildBuilding(Unit*,int,int,UnitType*,int);
	/// Prepare command dismiss
extern void CommandDismiss(Unit* unit);
	/// Prepare command resource location
extern void CommandResourceLoc(Unit* unit,int x,int y,int flush);
	/// Prepare command resource
extern void CommandResource(Unit* unit,Unit* dest,int flush);
	/// Prepare command return
extern void CommandReturnGoods(Unit* unit,Unit* goal,int flush);
	/// Prepare command train
extern void CommandTrainUnit(Unit* unit,UnitType* what,int flush);
	/// Prepare command cancel training
extern void CommandCancelTraining(Unit* unit,int slot,const UnitType* type);
	/// Prepare command upgrade to
extern void CommandUpgradeTo(Unit* unit,UnitType* what,int flush);
	/// Prepare command cancel upgrade to
extern void CommandCancelUpgradeTo(Unit* unit);
	/// Prepare command research
extern void CommandResearch(Unit* unit,Upgrade* what,int flush);
	/// Prepare command cancel research
extern void CommandCancelResearch(Unit* unit);
#if 0
	/// Prepare command upgrade
extern void CommandUpgradeUnit(Unit* unit,int what,int flush);
#endif
	/// Prepare command spellcast
extern void CommandSpellCast(Unit* unit,int x,int y,Unit* dest
		,SpellType* spell,int flush);
	/// Prepare command auto spellcast
extern void CommandAutoSpellCast(Unit* unit, int spellid, int on);
	/// Prepare diplomacy command
extern void CommandDiplomacy(int player,int state,int opponent);
	/// Prepare shared vision command
extern void CommandSharedVision(int player,int state,int opponent);
	/// Send any command
extern void CommandAnyOrder(Unit* unit,Order * order,int flush);
	/// Move an order in command queue
extern void CommandMoveOrder(Unit* unit,int src,int dst);
/*----------------------------------------------------------------------------
--  Actions: in action_<name>.c
----------------------------------------------------------------------------*/

	/// Generic still action
extern void ActionStillGeneric(Unit* unit,int ground);
	/// Handle command still
extern void HandleActionStill(Unit* unit);
	/// Handle command stand ground
extern void HandleActionStandGround(Unit* unit);
	/// Handle command follow
extern void HandleActionFollow(Unit* unit);
	/// Generic move action
extern int DoActionMove(Unit* unit);
	/// Handle command move
extern void HandleActionMove(Unit* unit);
	/// Handle command repair
extern void HandleActionRepair(Unit* unit);
	/// Handle command patrol
extern void HandleActionPatrol(Unit* unit);
	/// Show attack animation
extern void AnimateActionAttack(Unit* unit);
	/// Handle command attack
extern void HandleActionAttack(Unit* unit);
	/// Handle command board
extern void HandleActionBoard(Unit* unit);
	/// Handle command unload
extern void HandleActionUnload(Unit* unit);
	/// Handle command resource
extern void HandleActionResource(Unit* unit);
	/// Handle command return
extern void HandleActionReturnGoods(Unit* unit);
	/// Handle command die
extern void HandleActionDie(Unit* unit);
	/// Handle command build
extern void HandleActionBuild(Unit* unit);
	/// Handle command builded
extern void HandleActionBuilded(Unit* unit);
	/// Handle command train
extern void HandleActionTrain(Unit* unit);
	/// Handle command upgrade to
extern void HandleActionUpgradeTo(Unit* unit);
	/// Handle command upgrade
extern void HandleActionUpgrade(Unit* unit);
	/// Handle command research
extern void HandleActionResearch(Unit* unit);
	/// Handle command spellcast
extern void HandleActionSpellCast(Unit* unit);

/*----------------------------------------------------------------------------
--  Actions: actions.c
----------------------------------------------------------------------------*/

	/// Handle the animation of a unit
extern int UnitShowAnimation(Unit* unit,const Animation* animation);
	/// Handle the actions of all units each game cycle
extern void UnitActions(void);
	/// Unload an unit.
extern int UnloadUnit(Unit* unit);
//@}

#endif // !__ACTIONS_H__
