/*
**	A clone of a famous game.
*/
/**@name ai.h		-	The ai headerfile. */
/*
**	(c) Copyright 1998,1999 by Lutz Sammer
**
**	$Id$
*/

#ifndef __AI_H__
#define __AI_H__

//@{

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern int AiSleep;			/// Ai sleeps # frames
extern int AiTimeFactor;		/// Adjust the AI build times
extern int AiCostFactor;		/// Adjust the AI costs

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

extern void AiEachFrame(int player);	/// Called each frame
extern void AiEachSecond(int player);	/// Called each second
extern void AiInit(int player);		/// Init AI for this player

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
