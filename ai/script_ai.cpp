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
/**@name ccl_ai.c	-	The AI ccl functions. */
/*
**	(c) Copyright 2000 by Lutz Sammer
**
**	$Id$
*/

//@{

#include <stdio.h>
#include <stdlib.h>

#include "clone.h"

#if defined(USE_CCL) || defined(USE_CCL2) // {

/**
**	Register CCL features for unit-type.
*/
global void AiCclRegister(void)
{
    // FIXME: Need to save memory here.
}

#endif	// } USE_CCL && USE_CCL2

//@}
