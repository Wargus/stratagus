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
/**@name ccl_missile.c	-	The missile-type ccl functions. */
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

#include "video.h"
#include "tileset.h"
#include "unittype.h"
#include "missile.h"
#include "ccl_sound.h"
#include "ccl.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

IfDebug(
extern int NoWarningMissileType;		/// quiet ident lookup.
);

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Parse missile-type.
**
**	@param list	List describing missile-type.
*/
local SCM CclDefineMissileType(SCM list)
{
    SCM value;
    char* str;
    MissileType* type;
    unsigned i;

    //	Slot identifier

    str=gh_scm2newstr(gh_car(list),NULL);
    list=gh_cdr(list);
    IfDebug( i=NoWarningMissileType; NoWarningMissileType=1; );
    type=MissileTypeByIdent(str);
    IfDebug( NoWarningMissileType=i; );
    if( type ) {
	DebugLevel0Fn("Redefining missile-type `%s'\n",str);
	CclFree(str);
    } else {
	type=NewMissileTypeSlot(str);
    }

    //
    //	Parse the arguments, already the new tagged format.
    //
    while( !gh_null_p(list) ) {
	value=gh_car(list);
	list=gh_cdr(list);
	if( gh_eq_p(value,gh_symbol2scm("file")) ) {
	    CclFree(type->File);
	    type->File=gh_scm2newstr(gh_car(list),NULL);
	} else if( gh_eq_p(value,gh_symbol2scm("size")) ) {
	    value=gh_car(list);
	    type->Width=gh_scm2int(gh_car(value));
	    value=gh_cdr(value);
	    type->Height=gh_scm2int(gh_car(value));
	} else if( gh_eq_p(value,gh_symbol2scm("frames")) ) {
	    //gh_scm2int(gh_car(list));
	    // FIXME: not used now
	} else if( gh_eq_p(value,gh_symbol2scm("fired-sound")) ) {
	    CclFree(type->FiredSound.Name);
	    type->FiredSound.Name=gh_scm2newstr(gh_car(list),NULL);
	} else if( gh_eq_p(value,gh_symbol2scm("impact-sound")) ) {
	    CclFree(type->ImpactSound.Name);
	    type->ImpactSound.Name=gh_scm2newstr(gh_car(list),NULL);
	} else if( gh_eq_p(value,gh_symbol2scm("class")) ) {
	    value=gh_car(list);
	    for( i=0; MissileClassNames[i]; ++i ) {
		if( gh_eq_p(value,
			    gh_symbol2scm((char*)MissileClassNames[i])) ) {
		    type->Class=i;
		    break;
		}
	    }
	    if( !MissileClassNames[i] ) {
		// FIXME: this leaves a half initialized missile-type
		errl("Unsupported class",value);
	    }
	} else if( gh_eq_p(value,gh_symbol2scm("delay")) ) {
	    type->Delay=gh_scm2int(gh_car(list));
	} else if( gh_eq_p(value,gh_symbol2scm("sleep")) ) {
	    type->Sleep=gh_scm2int(gh_car(list));
	} else if( gh_eq_p(value,gh_symbol2scm("speed")) ) {
	    type->Speed=gh_scm2int(gh_car(list));
	} else if( gh_eq_p(value,gh_symbol2scm("range")) ) {
	    type->Range=gh_scm2int(gh_car(list));
	} else if( gh_eq_p(value,gh_symbol2scm("impact-missile")) ) {
	    CclFree(type->ImpactName);
	    type->ImpactName=gh_scm2newstr(gh_car(list),NULL);
	} else {
	    // FIXME: this leaves a half initialized missile-type
	    errl("Unsupported tag",value);
	}
	list=gh_cdr(list);
    }

    return SCM_UNSPECIFIED;
}

/**
**	Define missile type mapping from original number to internal symbol
**
**	@param list	List of all names.
*/
local SCM CclDefineMissileTypeWcNames(SCM list)
{
    int i;
    char** cp;

    if( (cp=MissileTypeWcNames) ) {		// Free all old names
	while( *cp ) {
	    free(*cp++);
	}
	free(MissileTypeWcNames);
    }

    //
    //	Get new table.
    //
    i=gh_length(list);
    MissileTypeWcNames=cp=malloc((i+1)*sizeof(char*));
    while( i-- ) {
	*cp++=gh_scm2newstr(gh_car(list),NULL);
	list=gh_cdr(list);
    }
    *cp=NULL;

    return SCM_UNSPECIFIED;
}

/**
**	Create a missile.
**
**	@param list	List of all names.
*/
local SCM CclMissile(SCM list)
{
    DebugLevel0Fn("FIXME: not written\n");

    return SCM_UNSPECIFIED;
}

/**
**	Register CCL features for missile-type.
*/
global void MissileCclRegister(void)
{
    gh_new_procedureN("define-missiletype-wc-names",
	    CclDefineMissileTypeWcNames);
    gh_new_procedureN("define-missile-type",CclDefineMissileType);
    gh_new_procedureN("missile",CclMissile);
}

#endif	// } USE_CCL

//@}
