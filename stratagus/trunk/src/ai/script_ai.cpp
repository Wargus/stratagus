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
//
//	(c) Copyright 2000,2001 by Lutz Sammer
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

#include "unittype.h"
#include "ccl.h"
#include "ai_local.h"

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

#if defined(NEW_AI)

/**
**	Setup AI helper table.
**
**	Expand the table if needed.
**
**	@param count	Pointer to number of elements in table.
**	@param table	Pointer to table with elements.
**	@param n	Index to insert new into table
*/
local void AiHelperSetupTable(int* count,AiUnitTypeTable*** table,int n)
{
    int i;
    
    ++n;
    if( n>(i=*count) ) {
	if( *table ) {
	    *table=realloc(*table,n*sizeof(AiUnitTypeTable*));
	    memset((*table)+i,0,(n-i)*sizeof(AiUnitTypeTable*));
	} else {
	    *table=malloc(n*sizeof(AiUnitTypeTable*));
	    memset(*table,0,n*sizeof(AiUnitTypeTable*));
	}
	*count=n;
    }
}

/**
**	Insert new unit-type element.
**
**	@param table	Pointer to table with elements.
**	@param base	Base type to insert into table.
*/
local void AiHelperInsert(AiUnitTypeTable** tablep,UnitType* base)
{
    int i;
    int n;
    AiUnitTypeTable* table;

    //
    //	New unit-type
    //
    if( !(table=*tablep) ) {
	table=*tablep=malloc(sizeof(AiUnitTypeTable));
	table->Count=1;
	table->Table[0]=base;
	return;
    }

    //
    //	Look if already known.
    //
    n=table->Count;
    for( i=0; i<n; ++i ) {
	if( table->Table[i]==base ) {
	    return;
	}
    }

    //
    //	Append new base unit-type to units.
    //
    table=*tablep=realloc(table,sizeof(AiUnitTypeTable)+sizeof(UnitType*)*n);
    table->Count=n+1;
    table->Table[n]=base;
}

#ifdef DEBUG
/**
**	Print AI helper table.
*/
local void PrintAiHelperTable(void)
{
}
#endif

/**
**	Define helper for Ai.
**
**	@param list	List of all helpers.
*/
local SCM CclDefineAiHelper(SCM list)
{
    SCM sub_list;
    SCM value;
    int what;
    char* str;
    UnitType* base;
    UnitType* type;
    Upgrade* upgrade;

    IfDebug( type=NULL; upgrade=NULL; );	// keep the compiler happy
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
	    fprintf(stderr,"unknown tag\n");
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
	    fprintf(stderr,"unknown unittype %s\n",str);
	    free(str);
	    continue;
	}
	DebugLevel0Fn("%s\n",base->Name);
	free(str);

	//
	//	Get the unit types, which could be produced
	//
	while( !gh_null_p(sub_list) ) {
	    value=gh_car(sub_list);
	    sub_list=gh_cdr(sub_list);
	    str=gh_scm2newstr(value,NULL);
	    if( what==3 ) {
		upgrade=UpgradeByIdent(str);
		if( !upgrade ) {
		    fprintf(stderr,"unknown upgrade %s\n",str);
		    free(str);
		    continue;
		}
		DebugLevel0Fn("> %s\n",upgrade->Ident);
	    } else {
		type=UnitTypeByIdent(str);
		if( !type ) {
		    fprintf(stderr,"unknown unittype %s\n",str);
		    free(str);
		    continue;
		}
		DebugLevel0Fn("> %s\n",type->Name);
	    }
	    free(str);

	    switch( what ) {
		case 0:			// build
		    AiHelperSetupTable(
			    &AiHelpers.BuildCount,&AiHelpers.Build,type->Type);
		    AiHelperInsert(
			    AiHelpers.Build+type->Type,base);
		    break;
		case 1:			// train
		    AiHelperSetupTable(
			    &AiHelpers.TrainCount,&AiHelpers.Train,type->Type);
		    AiHelperInsert(
			    AiHelpers.Train+type->Type,base);
		    break;
		case 2:			// upgrade
		    AiHelperSetupTable(
			    &AiHelpers.UpgradeCount,&AiHelpers.Upgrade,
			    type->Type);
		    AiHelperInsert(
			    AiHelpers.Upgrade+type->Type,base);
		    break;
		case 3:			// research
		    AiHelperSetupTable(
			    &AiHelpers.ResearchCount,&AiHelpers.Research,
			    upgrade-Upgrades);
		    AiHelperInsert(
			    AiHelpers.Research+(upgrade-Upgrades),base);
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
    DebugLevel0Fn("%s\n",str);
    aitype->Name=str;

    //
    //	AI Race
    //
    value=gh_car(list);
    list=gh_cdr(list);
    str=gh_scm2newstr(value,NULL);
    DebugLevel0Fn("%s\n",str);
    aitype->Race=str;

    //
    //	AI Class
    //
    value=gh_car(list);
    list=gh_cdr(list);
    str=gh_scm2newstr(value,NULL);
    DebugLevel0Fn("%s\n",str);
    aitype->Class=str;

    //
    //	AI Script
    //
    value=gh_car(list);
    list=gh_cdr(list);
    aitype->Script=value;
    //gc_protect(&aitype->Script);

#if 0
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
	    fprintf(stderr,"unknown tag %s\n",str);
	    free(str);
	    continue;
	}
    }
#endif
    return list;
}

/*----------------------------------------------------------------------------
--	Ai script functions
----------------------------------------------------------------------------*/

/**
**	Get unit-type.
*/
extern UnitType* CclGetUnitType(SCM ptr);

/**
**	Append unit-type to request table.
**
**	@param type	Unit-type to be appended.
**	@param count	How many unit-types to build.
*/
local void InsertRequests(UnitType* type,int count)
{
    int n;

    if( AiPlayer->UnitTypeRequests ) {
	n=AiPlayer->RequestsCount;
	AiPlayer->UnitTypeRequests=realloc(AiPlayer->UnitTypeRequests,
		(n+1)*sizeof(*AiPlayer->UnitTypeRequests));
    } else {
	AiPlayer->UnitTypeRequests=malloc(sizeof(*AiPlayer->UnitTypeRequests));
	n=0;
    }
    AiPlayer->UnitTypeRequests[n].Table[0]=type;
    AiPlayer->UnitTypeRequests[n].Count=count;
    AiPlayer->RequestsCount=n+1;
}

/**
**	Find unit-type in request table.
**
**	@param type	Unit-type to be found.
*/
local AiUnitTypeTable* FindInRequests(const UnitType* type)
{
    int i;
    int n;


    n=AiPlayer->RequestsCount;
    for( i=0; i<n; ++i ) {
	if( AiPlayer->UnitTypeRequests[i].Table[0]==type ) {
	    return &AiPlayer->UnitTypeRequests[i];
	}
    }
    return NULL;
}
/**
**	Set debuging flag of AI script.
*/
local SCM CclAiDebug(SCM flag)
{
    if( gh_eq_p(flag,SCM_BOOL_T) ) {
	AiPlayer->ScriptDebug=1;
    } else {
	AiPlayer->ScriptDebug=0;
    }
    return SCM_BOOL_F;
}

/**
**	Need an unit.
**
**	@param value	Unit-type as string/symbol/object.
*/
local SCM CclAiNeed(SCM value)
{
    InsertRequests(CclGetUnitType(value),1);

    return SCM_BOOL_F;
}

/**
**	Set the number of units.
**
**	@param value	Unit-type as string/symbol/object.
**	@param count	Number of unit-types requested.
*/
local SCM CclAiSet(SCM value,SCM count)
{
    AiUnitTypeTable* autt;
    UnitType* type;

    printf("Set: ");
    gh_display(value);
    gh_newline();

    type=CclGetUnitType(value);
    if( (autt=FindInRequests(type)) ) {
	autt->Count=gh_scm2int(count);
    } else {
	InsertRequests(type,gh_scm2int(count));
    }

    return SCM_BOOL_F;
}

/**
**	Wait for an unit.
**
**	@param value	Unit-type as string/symbol/object.
*/
local SCM CclAiWait(SCM value)
{
    AiUnitTypeTable* autt;
    UnitType* type;

    printf("Wait: ");
    gh_display(value);
    gh_newline();

    type=CclGetUnitType(value);
    if( !(autt=FindInRequests(type)) ) {
	DebugLevel0Fn("Broken, waiting on unit-type which wasn't requested.\n");
	return SCM_BOOL_F;
    }
    // units available?
    DebugLevel0Fn("%d,%d\n",AiPlayer->Player->UnitTypesCount[type->Type],autt->Count);
    if( AiPlayer->Player->UnitTypesCount[type->Type]>=autt->Count ) {
	return SCM_BOOL_F;
    }

    return SCM_BOOL_T;
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

#if defined(NEW_AI)
    gh_new_procedure1_0("ai:debug",CclAiDebug);
    gh_new_procedure1_0("ai:need",CclAiNeed);
    gh_new_procedure2_0("ai:set",CclAiSet);
    gh_new_procedure1_0("ai:wait",CclAiWait);
#endif

}

#endif	// } USE_CCL

//@}
