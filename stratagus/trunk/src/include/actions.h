//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name actions.h	-	The actions headerfile. */
/*
**	(c) Copyright 1998-2000 by Lutz Sammer
**
**	$Id$
*/

#ifndef __ACTIONS_H__
#define __ACTIONS_H__

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "unittype.h"
#include "unit.h"
#include "upgrade.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Commands:	in command.c
----------------------------------------------------------------------------*/

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
    /// Prepare command cancel build
extern void CommandCancelBuilding(Unit* unit,Unit* worker);
    /// Prepare command harvest
extern void CommandHarvest(Unit* unit,int x,int y,int flush);
    /// Prepare command mine
extern void CommandMineGold(Unit* unit,Unit* dest,int flush);
    /// Prepare command haul
extern void CommandHaulOil(Unit* unit,Unit* dest,int flush);
    /// Prepare command return
extern void CommandReturnGoods(Unit* unit,int flush);
    /// Prepare command train
extern void CommandTrainUnit(Unit* unit,UnitType* what,int flush);
    /// Prepare command cancel training
extern void CommandCancelTraining(Unit* unit,int slot);
    /// Prepare command upgrade to
extern void CommandUpgradeTo(Unit* unit,UnitType* what,int flush);
    /// Prepare command cancel upgrade to
extern void CommandCancelUpgradeTo(Unit* unit);
    /// Prepare command research
extern void CommandResearch(Unit* unit,Upgrade* what,int flush);
    /// Prepare command cancel research
extern void CommandCancelResearch(Unit* unit);
    /// Prepare command upgrade
//extern void CommandUpgradeUnit(Unit* unit,int what,int flush);
    /// Prepare command demolish
extern void CommandDemolish(Unit* unit,int x,int y,Unit* dest,int flush);
    /// Prepare command spellcast
extern void CommandSpellCast(Unit* unit,int x,int y,Unit* dest
	,int spellid, int flush);

/*----------------------------------------------------------------------------
--	Actions:	in action_<name>.c
----------------------------------------------------------------------------*/

    /// Generic still action
extern void ActionStillGeneric(Unit* unit,int ground);
    /// Handle command still
extern void HandleActionStill(Unit* unit);
    /// Handle command stand ground
extern void HandleActionStandGround(Unit* unit);
    /// Handle command move
extern int HandleActionMove(Unit* unit);
    /// Handle command repair
extern int HandleActionRepair(Unit* unit);
    /// Handle command patrol
extern void HandleActionPatrol(Unit* unit);
    /// Handle command attack
extern void HandleActionAttack(Unit* unit);
    /// Handle command board
extern void HandleActionBoard(Unit* unit);
    /// Handle command unload
extern void HandleActionUnload(Unit* unit);
    /// Handle command harvest
extern void HandleActionHarvest(Unit* unit);
    /// Handle command mine
extern void HandleActionMineGold(Unit* unit);
    /// Handle command haul
extern void HandleActionHaulOil(Unit* unit);
    /// Handle command return
extern void HandleActionReturnGoods(Unit* unit);
    /// Handle command die
extern int HandleActionDie(Unit* unit);
    /// Handle command build
extern void HandleActionBuild(Unit* unit);
    /// Handle command builded
extern void HandleActionBuilded(Unit* unit);
    /// Handle command attack
extern int AnimateActionAttack(Unit* unit);
    /// Handle command train
extern void HandleActionTrain(Unit* unit);
    /// Handle command upgrade to
extern void HandleActionUpgradeTo(Unit* unit);
    /// Handle command upgrade
extern void HandleActionUpgrade(Unit* unit);
    /// Handle command research
extern void HandleActionResearch(Unit* unit);
    /// Handle command demolish
extern void HandleActionDemolish(Unit* unit);
    /// Handle command spellcast
extern void HandleActionSpellCast(Unit* unit);

/*----------------------------------------------------------------------------
--	Actions:	actions.c
----------------------------------------------------------------------------*/

    /// handle the animation of a unit
extern int UnitShowAnimation(Unit* unit,const Animation* animation);
    /// Handle the actions of all units each frame
extern void UnitActions(void);

//@}

#endif // !__ACTIONS_H__
