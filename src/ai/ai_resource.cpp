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
/**@name ai_resource.c	-	AI resource manager. */
//
//      (c) Copyright 2000,2001 by Lutz Sammer
//
//      $Id$

#ifdef NEW_AI	// {

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"

#include "unit.h"
#include "ai_local.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Add unit-type request to resource manager.
**
**	@param type	Unit type requested.
**	@param count	How many units.
*/
global void AiAddUnitTypeRequest(UnitType* type,int count)
{
    int n;

    DebugLevel0Fn("%s %d\n",type->Ident,count);
    if( AiPlayer->UnitTypeBuilded ) {
	n=AiPlayer->BuildedCount;
	AiPlayer->UnitTypeBuilded=realloc(AiPlayer->UnitTypeBuilded,
		(n+1)*sizeof(*AiPlayer->UnitTypeBuilded));
    } else {
	AiPlayer->UnitTypeBuilded=malloc(sizeof(*AiPlayer->UnitTypeBuilded));
	n=0;
    }
    AiPlayer->UnitTypeBuilded[n].Type=type;
    AiPlayer->UnitTypeBuilded[n].Want=count;
    AiPlayer->UnitTypeBuilded[n].Made=0;
    AiPlayer->BuildedCount=n+1;
}

/**
**	Entry point of resource manager, perodic called.
*/
global void AiResourceManager(void)
{
}

//@}

#endif // } NEW_AI
