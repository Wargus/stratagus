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
/**@name network.h	-	The network header file. */
/*
**	(c) Copyright 1998-2000 by Lutz Sammer
**
**	$Id$
*/

#ifndef __NETWORK_H__
#define __NETWORK_H__

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "unittype.h"
#include "unit.h"

/*----------------------------------------------------------------------------
--	Defines
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern int NetworkFildes;		/// Network file descriptor
extern int NetworkInSync;		/// Network is in sync
extern int CommandLogEnabled;		/// True if command log is on

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

extern void InitNetwork(void);		/// initialise network module
extern void ExitNetwork(void);		/// cleanup network module
extern void NetworkEvent(void);		/// handle network events
extern void NetworkSync(void);		/// hold in sync
extern void NetworkQuit(void);		/// quit game
extern void NetworkChatMessage(const char*msg);	/// send chat message

    /// Send stop command
extern void SendCommandStopUnit(Unit* unit);
    /// Send stand ground command
extern void SendCommandStandGround(Unit* unit);
    /// Send move command
extern void SendCommandMoveUnit(Unit* unit,int x,int y);
    /// Send repair command
extern void SendCommandRepair(Unit* unit,int x,int y);
    /// Send attack command
extern void SendCommandAttack(Unit* unit,int x,int y,Unit* dest);
    /// Send attack ground command
extern void SendCommandAttackGround(Unit* unit,int x,int y);
    /// Send patrol command
extern void SendCommandPatrolUnit(Unit* unit,int x,int y);
    /// Send board command
extern void SendCommandBoard(Unit* unit,Unit* dest);
    /// Send unload command
extern void SendCommandUnload(Unit* unit,int x,int y,Unit* what);
    /// Send build building command
extern void SendCommandBuildBuilding(Unit* unit,int x,int y,UnitType* what);
    /// Send cancel building command
extern void SendCommandCancelBuilding(Unit* unit,Unit* peon);
    /// Send harvest command
extern void SendCommandHarvest(Unit* unit,int x,int y);
    /// Send mine gold command
extern void SendCommandMineGold(Unit* unit,Unit* dest);
    /// Send haul oil command
extern void SendCommandHaulOil(Unit* unit,Unit* dest);
    /// Send return goods command
extern void SendCommandReturnGoods(Unit* unit);
    /// Send train command
extern void SendCommandTrainUnit(Unit* unit,UnitType* what);
    /// Send cancel training command
extern void SendCommandCancelTraining(Unit* unit);
    /// Send upgrade to command
extern void SendCommandUpgradeTo(Unit* unit,UnitType* what);
    /// Send cancel upgrade to command
extern void SendCommandCancelUpgradeTo(Unit* unit);
    /// Send research command
extern void SendCommandResearch(Unit* unit,int what);
    /// Send cancel research command
extern void SendCommandCancelResearch(Unit* unit);
    /// Send demolish command
extern void SendCommandDemolish(Unit* unit,int x,int y,Unit* dest);

//@}

#endif	// !__NETWORK_H__
