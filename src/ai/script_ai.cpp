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

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>

#include "freecraft.h"

#if defined(USE_CCL) || defined(USE_CCL2) // {

#include <stdlib.h>

#include "unittype.h"
#include "ccl.h"
#include "ai_local.h"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

#if defined(NEW_AI)

/**
**	Setup AI helper table.
*/
local void AiHelperSetupTable(int* count,AiUnitTable** table,UnitType* type)
{
    if( type->Type>=*count ) {
	if( *table ) {
	    *table=realloc(*table,(1+type->Type)*sizeof(UnitType*));
	    memset(*table+*count,0,((1+type->Type)-*count)*sizeof(UnitType*));
	} else {
	    *table=malloc((1+type->Type)*sizeof(UnitType*));
	    memset(*table,0,(1+type->Type)*sizeof(UnitType*));
	}
	*count=type->Type+1;
    }
}

/**
**	Insert new element.
*/
local void AiHelperInsert(AiUnitTable** table,UnitType* base)
{
    int i;
    int n;

    if( !*table ) {
	*table=malloc(sizeof(AiUnitTable*)+sizeof(UnitType*));
	(*table)->Count=1;
	(*table)->Table[0]=base;
	return;
    }
    n=(*table)->Count;
    for( i=0; i<n; ++i ) {
	if( (*table)->Table[i]==base ) {
	    return;
	}
    }
    n++;
    *table=realloc(*table,sizeof(AiUnitTable*)+sizeof(UnitType*)*n);
    (*table)->Count=n;
    (*table)->Table[n-1]=base;
}

/**
**	Define helper for Ai.
*/
local SCM CclDefineAiHelper(SCM list)
{
    SCM sub_list;
    SCM value;
    int what;
    char* str;
    UnitType* base;
    UnitType* type;

    while( !gh_null_p(list) ) {
	sub_list=gh_car(list);
	list=gh_cdr(list);

	//
	//	Type build,train,research/upgrade.
	//
	value=gh_car(sub_list);
	sub_list=gh_cdr(sub_list);
	if( gh_eq_p(value,gh_symbol2scm("build")) ) {
	    what=0;
	} else if( gh_eq_p(value,gh_symbol2scm("train")) ) {
	    what=1;
	} else if( gh_eq_p(value,gh_symbol2scm("upgrade")) ) {
	    what=2;
	} else if( gh_eq_p(value,gh_symbol2scm("research")) ) {
	    what=3;
	} else {
	    fprintf(stderr,"unknown tag");
	    continue;
	}
	//
	//	Get the base unit type, which could handle the action.
	//
	value=gh_car(sub_list);
	sub_list=gh_cdr(sub_list);
	str=gh_scm2newstr(value,NULL);
	base=UnitTypeByIdent(str);
	if( !base ) {
	    fprintf(stderr,"unknown unittype %s",str);
	    free(str);
	    continue;
	}
	DebugLevel0(__FUNCTION__": %s\n",base->Name);
	free(str);

	//
	//	Get the unit types, which could be produced
	//
	while( !gh_null_p(sub_list) ) {
	    value=gh_car(sub_list);
	    sub_list=gh_cdr(sub_list);
	    str=gh_scm2newstr(value,NULL);
	    type=UnitTypeByIdent(str);
	    if( !type ) {
		fprintf(stderr,"unknown unittype %s",str);
		free(str);
		continue;
	    }
	    DebugLevel0(__FUNCTION__": > %s\n",type->Name);
	    free(str);

	    switch( what ) {
		case 0:			// build
		    AiHelperSetupTable(
			    &AiHelpers.BuildCount,&AiHelpers.Build,type);
		    AiHelperInsert(
			    &AiHelpers.Build+type->Type,base);
		    break;
		case 1:			// train
		    AiHelperSetupTable(
			    &AiHelpers.TrainCount,&AiHelpers.Train,type);
		    AiHelperInsert(
			    &AiHelpers.Train+type->Type,base);
		    break;
		case 2:			// upgrade
		    AiHelperSetupTable(
			    &AiHelpers.UpgradeCount,&AiHelpers.Upgrade,type);
		    AiHelperInsert(
			    &AiHelpers.Upgrade+type->Type,base);
		    break;
		case 3:			// research
		    break;
	    }
	}
    }
    return list;
}

/**
**	Define an Ai engine.
*/
local SCM CclDefineAi(SCM list)
{
    SCM sub_list;
    SCM value;
    char* str;
    AiType* aitype;

    aitype=malloc(sizeof(AiType));
    aitype->Next=AiTypes;
    AiTypes=aitype;

    //
    //	AI Name
    //
    value=gh_car(list);
    list=gh_cdr(list);
    str=gh_scm2newstr(value,NULL);
    DebugLevel0(__FUNCTION__": %s\n",str);
    aitype->Name=str;

    //
    //	AI Race
    //
    value=gh_car(list);
    list=gh_cdr(list);
    str=gh_scm2newstr(value,NULL);
    DebugLevel0(__FUNCTION__": %s\n",str);
    aitype->Race=str;

    //
    //	AI Class
    //
    value=gh_car(list);
    list=gh_cdr(list);
    str=gh_scm2newstr(value,NULL);
    DebugLevel0(__FUNCTION__": %s\n",str);
    aitype->Class=str;

    //
    //	AI Script
    //
    list=gh_car(list);
    while( !gh_null_p(list) ) {
	sub_list=gh_car(list);
	list=gh_cdr(list);

	value=gh_car(sub_list);
	sub_list=gh_cdr(sub_list);
	if( gh_eq_p(value,gh_symbol2scm("set-cheat-unexplored!")) ) {
	} else if( gh_eq_p(value,gh_symbol2scm("set-cheat-visible!")) ) {
	} else if( gh_eq_p(value,gh_symbol2scm("need")) ) {
	} else if( gh_eq_p(value,gh_symbol2scm("train")) ) {
	} else if( gh_eq_p(value,gh_symbol2scm("build")) ) {
	} else if( gh_eq_p(value,gh_symbol2scm("wait")) ) {
	} else if( gh_eq_p(value,gh_symbol2scm("force")) ) {
	} else {
	    str=gh_scm2newstr(value,NULL);
	    fprintf(stderr,"unknown tag %s",str);
	    free(str);
	    continue;
	}
    }
    return list;
}

#else

/**
**	Define helper for Ai.
*/
local SCM CclDefineAiHelper(SCM list)
{
    return list;
}

/**
**	Define an Ai engine.
*/
local SCM CclDefineAi(SCM list)
{
    return list;
}

#endif

/**
**	Register CCL features for unit-type.
*/
global void AiCclRegister(void)
{
    // FIXME: Need to save memory here.
    // Loading all into memory isn't necessary.

    gh_new_procedureN("define-ai-helper",CclDefineAiHelper);
    gh_new_procedureN("define-ai",CclDefineAi);
}

#endif	// } USE_CCL && USE_CCL2

//@}
