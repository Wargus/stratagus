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
/**@name ai.h		-	The ai headerfile. */
/*
**	(c) Copyright 1998-2000 by Lutz Sammer
**
**	$Id$
*/

#ifndef __AI_H__
#define __AI_H__

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "player.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern int AiSleep;			/// Ai sleeps # frames
extern int AiTimeFactor;		/// Adjust the AI build times
extern int AiCostFactor;		/// Adjust the AI costs

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

extern void AiEachFrame(Player* player);/// Called each frame
extern void AiEachSecond(Player* player);/// Called each second
extern void AiInit(Player* player);	/// Init AI for this player

extern void AiCclRegister(void);	/// register ccl features

/*--------------------------------------------------------
--     Call Backs/Triggers
--------------------------------------------------------*/

extern void AiHelpMe(Unit* unit);
extern void AiWorkComplete(Unit* unit,Unit* what);
extern void AiCanNotBuild(Unit* unit,const UnitType* what);
extern void AiCanNotReach(Unit* unit,const UnitType* what);
extern void AiTrainingComplete(Unit* unit,Unit* what);
extern void AiUpgradeToComplete(Unit* unit,Unit* what);
extern void AiResearchComplete(Unit* unit,int what);

//@}

#endif	// !__AI_H__
