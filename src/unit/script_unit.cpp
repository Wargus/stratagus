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
/**@name ccl_unit.c	-	The unit ccl functions. */
//
//	(c) Copyright 2001 by Lutz Sammer
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>

#include "freecraft.h"

#if defined(USE_CCL) // {

#include <stdlib.h>

#include "unit.h"
#include "ccl.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Set hit-point regeneration
**
**	@param flag	Flag enabling or disabling it.
**
**	@return		The old state of the hit-point regeneration.
**
**	@todo move to unit ccl part.
*/
local SCM CclSetHitPointRegeneration(SCM flag)
{
    int old;

    old=HitPointRegeneration;
    HitPointRegeneration=gh_scm2bool(flag);

    return gh_bool2scm(old);
}

/**
**	Parse unit
**
**	@param list	List describing unit
*/
local SCM CclUnit(SCM list)
{
    DebugLevel0Fn("FIXME: not written\n");

    return SCM_UNSPECIFIED;
}

/**
**	Register CCL features for unit.
*/
global void UnitCclRegister(void)
{
    gh_new_procedure1_0("set-hitpoint-regeneration!",
	    CclSetHitPointRegeneration);
    gh_new_procedureN("unit",CclUnit);
}

#endif	// } USE_CCL

//@}
