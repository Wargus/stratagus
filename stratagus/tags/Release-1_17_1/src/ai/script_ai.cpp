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
//	(c) Copyright 2000-2002 by Lutz Sammer
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"
#include "unittype.h"
#include "ccl.h"
#include "ai.h"
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
**	@param tablep	Pointer to table with elements.
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
**	Define helper for AI.
**
**	@param list	List of all helpers.
**
**	@todo	FIXME: the first unit could be a list see ../doc/ccl/ai.html
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
    int cost;

    IfDebug( type=NULL; upgrade=NULL; cost=0; );// keep the compiler happy
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
	} else if( gh_eq_p(value,gh_symbol2scm("collect")) ) {
	    what=4;
	} else if( gh_eq_p(value,gh_symbol2scm("with-goods")) ) {
	    what=5;
	} else if( gh_eq_p(value,gh_symbol2scm("unit-limit")) ) {
	    what=6;
	} else if( gh_eq_p(value,gh_symbol2scm("unit-equiv")) ) {
	    what=7;
	} else if( gh_eq_p(value,gh_symbol2scm("repair")) ) {
	    what=8;
	} else {
	    fprintf(stderr,"unknown tag\n");
	    continue;
	}

	//
	//	Get the base unit type, which could handle the action.
	//
	value=gh_car(sub_list);
	sub_list=gh_cdr(sub_list);

	// FIXME: support value as list!
	str=gh_scm2newstr(value,NULL);
	base=UnitTypeByIdent(str);
	if( !base ) {
	    fprintf(stderr,"unknown unittype %s\n",str);
	    free(str);
	    continue;
	}
	DebugLevel3Fn("%s\n" _C_ base->Name);
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
		DebugLevel3Fn("> %s\n" _C_ upgrade->Ident);
	    } else if( what==4 || what==5 ) {
		if( !strcmp(DEFAULT_NAMES[1],str) ) {
		    cost=1;
		} else if( !strcmp(DEFAULT_NAMES[2],str) ) {
		    cost=2;
		} else if( !strcmp(DEFAULT_NAMES[3],str) ) {
		    cost=3;
		} else if( !strcmp(DEFAULT_NAMES[4],str) ) {
		    cost=4;
		} else if( !strcmp(DEFAULT_NAMES[5],str) ) {
		    cost=5;
		} else if( !strcmp(DEFAULT_NAMES[6],str) ) {
		    cost=6;
		} else {
		    fprintf(stderr,"unknown cost %s\n",str);
		    free(str);
		    continue;
		}
		DebugLevel3Fn("> %s\n" _C_ str);
	    } else if( what==6 ) {
		if( !strcmp("food",str) ) {
		    cost=0;
		} else {
		    fprintf(stderr,"unknown limit %s\n",str);
		    free(str);
		    continue;
		}
		DebugLevel3Fn("> %s\n" _C_ str);
	    } else {
		type=UnitTypeByIdent(str);
		if( !type ) {
		    fprintf(stderr,"unknown unittype %s\n",str);
		    free(str);
		    continue;
		}
		DebugLevel3Fn("> %s\n" _C_ type->Name);
	    }
	    free(str);

	    switch( what ) {
		case 0:			// build
		    AiHelperSetupTable(
			    &AiHelpers.BuildCount,&AiHelpers.Build,type->Type);
		    AiHelperInsert(AiHelpers.Build+type->Type,base);
		    break;
		case 1:			// train
		    AiHelperSetupTable(
			    &AiHelpers.TrainCount,&AiHelpers.Train,type->Type);
		    AiHelperInsert(AiHelpers.Train+type->Type,base);
		    break;
		case 2:			// upgrade
		    AiHelperSetupTable(
			    &AiHelpers.UpgradeCount,&AiHelpers.Upgrade,
			    type->Type);
		    AiHelperInsert(AiHelpers.Upgrade+type->Type,base);
		    break;
		case 3:			// research
		    AiHelperSetupTable(
			    &AiHelpers.ResearchCount,&AiHelpers.Research,
			    upgrade-Upgrades);
		    AiHelperInsert(AiHelpers.Research+(upgrade-Upgrades),base);
		    break;
		case 4:			// collect
		    AiHelperSetupTable(
			    &AiHelpers.CollectCount,&AiHelpers.Collect,cost);
		    AiHelperInsert(AiHelpers.Collect+cost,base);
		    break;
		case 5:			// with-goods
		    AiHelperSetupTable(
			&AiHelpers.WithGoodsCount,&AiHelpers.WithGoods,cost);
		    AiHelperInsert(AiHelpers.WithGoods+cost,base);
		    break;
		case 6:			// unit-limit
		    AiHelperSetupTable(
			&AiHelpers.UnitLimitCount,&AiHelpers.UnitLimit,cost);
		    AiHelperInsert(AiHelpers.UnitLimit+cost,base);
		    break;
		case 7:			// equivalence
		    AiHelperSetupTable(
			    &AiHelpers.EquivCount,&AiHelpers.Equiv,base->Type);
		    AiHelperInsert(AiHelpers.Equiv+base->Type,type);
		    break;
		case 8:			// repair
		    AiHelperSetupTable(
			&AiHelpers.RepairCount,&AiHelpers.Repair,type->Type);
		    AiHelperInsert(AiHelpers.Repair+type->Type,base);
		    break;
	    }
	}
    }

    return list;
}

/**
**	Define an AI engine.
*/
local SCM CclDefineAi(SCM list)
{
    SCM value;
    char* str;
    AiType* aitype;
#ifdef DEBUG
    const AiType* ait;
#endif

    aitype=malloc(sizeof(AiType));
    aitype->Next=AiTypes;
    AiTypes=aitype;

    //
    //	AI Name
    //
    value=gh_car(list);
    list=gh_cdr(list);
    str=gh_scm2newstr(value,NULL);
    DebugLevel3Fn("%s\n" _C_ str);
    aitype->Name=str;

#ifdef DEBUG
    for( ait=AiTypes->Next; ait; ait=ait->Next ) {
	if( !strcmp(aitype->Name,ait->Name) ) {
	    DebugLevel0Fn("Warning two or more AI's with the same name '%s'\n" _C_
		    ait->Name);
	}
    }
#endif

    //
    //	AI Race
    //
    value=gh_car(list);
    list=gh_cdr(list);
    str=gh_scm2newstr(value,NULL);
    DebugLevel3Fn("%s\n" _C_ str);
    if( *str!='*' ) {
	aitype->Race=str;
    } else {
	aitype->Race=NULL;
	free(str);
    }

    //
    //	AI Class
    //
    value=gh_car(list);
    list=gh_cdr(list);
    str=gh_scm2newstr(value,NULL);
    DebugLevel3Fn("%s\n" _C_ str);
    aitype->Class=str;

    //
    //	AI Script
    //
    value=gh_car(list);
    list=gh_cdr(list);
    aitype->Script=value;

    // Protect the scheme script against GC garbage-collect.
    CclGcProtect(value);

    return list;
}

/*----------------------------------------------------------------------------
--	AI script functions
----------------------------------------------------------------------------*/

    /// Get unit-type.
extern UnitType* CclGetUnitType(SCM ptr);

/**
**	Append unit-type to request table.
**
**	@param type	Unit-type to be appended.
**	@param count	How many unit-types to build.
*/
local void InsertUnitTypeRequests(UnitType* type,int count)
{
    int n;

    if( AiPlayer->UnitTypeRequests ) {
	n=AiPlayer->UnitTypeRequestsCount;
	AiPlayer->UnitTypeRequests=realloc(AiPlayer->UnitTypeRequests,
		(n+1)*sizeof(*AiPlayer->UnitTypeRequests));
    } else {
	AiPlayer->UnitTypeRequests=malloc(sizeof(*AiPlayer->UnitTypeRequests));
	n=0;
    }
    AiPlayer->UnitTypeRequests[n].Table[0]=type;
    AiPlayer->UnitTypeRequests[n].Count=count;
    AiPlayer->UnitTypeRequestsCount=n+1;
}

/**
**	Find unit-type in request table.
**
**	@param type	Unit-type to be found.
*/
local AiUnitTypeTable* FindInUnitTypeRequests(const UnitType* type)
{
    int i;
    int n;

    n=AiPlayer->UnitTypeRequestsCount;
    for( i=0; i<n; ++i ) {
	if( AiPlayer->UnitTypeRequests[i].Table[0]==type ) {
	    return &AiPlayer->UnitTypeRequests[i];
	}
    }
    return NULL;
}

/**
**	Find unit-type in upgrade-to table.
**
**	@param type	Unit-type to be found.
*/
local int FindInUpgradeToRequests(const UnitType* type)
{
    int i;
    int n;

    n=AiPlayer->UnitTypeRequestsCount;
    for( i=0; i<n; ++i ) {
	if( AiPlayer->UpgradeToRequests[i]==type ) {
	    return 1;
	}
    }
    return 0;
}

/**
**	Append unit-type to request table.
**
**	@param type	Unit-type to be appended.
*/
local void InsertUpgradeToRequests(UnitType* type)
{
    int n;

    if( AiPlayer->UpgradeToRequests ) {
	n=AiPlayer->UpgradeToRequestsCount;
	AiPlayer->UpgradeToRequests=realloc(AiPlayer->UpgradeToRequests,
		(n+1)*sizeof(*AiPlayer->UpgradeToRequests));
    } else {
	AiPlayer->UpgradeToRequests=
		malloc(sizeof(*AiPlayer->UpgradeToRequests));
	n=0;
    }
    AiPlayer->UpgradeToRequests[n]=type;
    AiPlayer->UpgradeToRequestsCount=n+1;
}

/**
**	Append unit-type to request table.
**
**	@param upgrade	Upgrade to be appended.
*/
local void InsertResearchRequests(Upgrade* upgrade)
{
    int n;

    if( AiPlayer->ResearchRequests ) {
	n=AiPlayer->ResearchRequestsCount;
	AiPlayer->ResearchRequests=realloc(AiPlayer->ResearchRequests,
		(n+1)*sizeof(*AiPlayer->ResearchRequests));
    } else {
	AiPlayer->ResearchRequests=malloc(sizeof(*AiPlayer->ResearchRequests));
	n=0;
    }
    AiPlayer->ResearchRequests[n]=upgrade;
    AiPlayer->ResearchRequestsCount=n+1;
}

//----------------------------------------------------------------------------

/**
**	Get the race of the current AI player.
*/
local SCM CclAiGetRace(void)
{
    return gh_symbol2scm(AiPlayer->Player->RaceName);
}

/**
**	Get the number of cycles to sleep.
*/
local SCM CclAiGetSleepCycles(void)
{
    return gh_int2scm(AiSleepCycles);
}

//----------------------------------------------------------------------------

/**
**	Set debuging flag of AI script.
*/
local SCM CclAiDebug(SCM flag)
{
    if( gh_eq_p(flag,SCM_BOOL_F) ) {
	AiPlayer->ScriptDebug=0;
    } else {
	AiPlayer->ScriptDebug=1;
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
    InsertUnitTypeRequests(CclGetUnitType(value),1);

    return SCM_BOOL_F;
}

/**
**	Set the number of units.
**
**	@param value	Unit-type as string/symbol/object.
**	@param count	Number of unit-types requested.
**
**	@todo FIXME:	count==0 should remove the request.
*/
local SCM CclAiSet(SCM value,SCM count)
{
    AiUnitTypeTable* autt;
    UnitType* type;

    type=CclGetUnitType(value);
    if( (autt=FindInUnitTypeRequests(type)) ) {
	autt->Count=gh_scm2int(count);
	// FIXME: 0 should remove it.
    } else {
	InsertUnitTypeRequests(type,gh_scm2int(count));
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
    const AiUnitTypeTable* autt;
    const UnitType* type;
    const int* unit_types_count;
    int j;
    int n;

    type=CclGetUnitType(value);
    unit_types_count=AiPlayer->Player->UnitTypesCount;
    if( !(autt=FindInUnitTypeRequests(type)) ) {
	//
	//	Look if we have this unit-type.
	//
	if( unit_types_count[type->Type] ) {
	    return SCM_BOOL_F;
	}
	//
	//	Look if we have equivalent unit-types.
	//
	if( type->Type<AiHelpers.EquivCount && AiHelpers.Equiv[type->Type] ) {
	    DebugLevel3Fn("Equivalence for %s\n" _C_ type->Ident);
	    for( j=0; j<AiHelpers.Equiv[type->Type]->Count; ++j ) {
		if( unit_types_count[
			AiHelpers.Equiv[type->Type]->Table[j]->Type] ) {
		    return SCM_BOOL_F;
		}
	    }
	}

	//
	//	Look if we have an upgrade-to request.
	//
	if( FindInUpgradeToRequests(type) ) {
	    return SCM_BOOL_T;
	}
	DebugLevel0Fn("Broken? waiting on %s which wasn't requested.\n" _C_
		type->Ident);
	return SCM_BOOL_F;
    }

    //
    //	Add equivalent units
    //
    n=unit_types_count[type->Type];
    if( type->Type<AiHelpers.EquivCount && AiHelpers.Equiv[type->Type] ) {
	for( j=0; j<AiHelpers.Equiv[type->Type]->Count; ++j ) {
	    n+=unit_types_count[AiHelpers.Equiv[type->Type]->Table[j]->Type];
	}
    }

    // units available?
    DebugLevel3Fn("%d,%d\n" _C_ n _C_ autt->Count);

    if( n>=autt->Count ) {
	return SCM_BOOL_F;
    }

    return SCM_BOOL_T;
}

/**
**	Define a force, a groups of units.
**
**	@param list	Pairs of unit-types and counts.
*/
local SCM CclAiForce(SCM list)
{
    AiUnitType** prev;
    AiUnitType* aiut;
    UnitType* type;
    int count;
    int force;

    force=gh_scm2int(gh_car(list));
    if( force<0 || force>=AI_MAX_FORCES ) {
	errl("Force out of range",gh_car(list));
    }
    list=gh_cdr(list);

    while( !gh_null_p(list) ) {
	type=CclGetUnitType(gh_car(list));
	list=gh_cdr(list);
	count=gh_scm2int(gh_car(list));
	list=gh_cdr(list);

	if( !type ) {			// bulletproof
	    continue;
	}

	//
	//	Look if already in force.
	//
	for( prev=&AiPlayer->Force[force].UnitTypes; (aiut=*prev);
		prev=&aiut->Next ) {
	    if( aiut->Type==type ) {	// found
		if( count ) {
		    aiut->Want=count;
		} else {
		    *prev=aiut->Next;
		    free(aiut);
		}
		break;
	    }
	}

	//
	//	New type append it.
	//
	if( !aiut ) {
	    *prev=aiut=malloc(sizeof(*aiut));
	    aiut->Next=NULL;
	    aiut->Want=count;
	    aiut->Type=type;
	}
    }

    AiAssignFreeUnitsToForce();

    return SCM_BOOL_F;
}

/**
**	Define the role of a force.
**
**	@param value	Force number.
**	@param flag	Which role of the force.
*/
local SCM CclAiForceRole(SCM value,SCM flag)
{
    int force;

    force=gh_scm2int(value);
    if( force<0 || force>=AI_MAX_FORCES ) {
	errl("Force out of range",value);
    }
    if( gh_eq_p(flag,gh_symbol2scm("attack")) ) {
	AiPlayer->Force[force].Role=AiForceRoleAttack;
    } else if( gh_eq_p(flag,gh_symbol2scm("defend")) ) {
	AiPlayer->Force[force].Role=AiForceRoleDefend;
    } else {
	errl("Unknown force role",flag);
    }

    return SCM_BOOL_F;
}

/**
**	Check if a force ready.
**
**	@param value	Force number.
**	@return		#t if ready, #f otherwise.
*/
local SCM CclAiCheckForce(SCM value)
{
    int force;

    force=gh_scm2int(value);
    if( force<0 || force>=AI_MAX_FORCES ) {
	errl("Force out of range",value);
    }
    if( AiPlayer->Force[force].Completed ) {
	return SCM_BOOL_T;
    }

    return SCM_BOOL_F;
}

/**
**	Wait for a force ready.
**
**	@param value	Force number.
*/
local SCM CclAiWaitForce(SCM value)
{
    int force;

    force=gh_scm2int(value);
    if( force<0 || force>=AI_MAX_FORCES ) {
	errl("Force out of range",value);
    }
    if( AiPlayer->Force[force].Completed ) {
	return SCM_BOOL_F;
    }
    DebugLevel0Fn("Wait force\n");
#if 0
    // Debuging
    AiCleanForces();
    DebugCheck( AiPlayer->Force[force].Completed );
#endif

    return SCM_BOOL_T;
}

/**
**	Attack with force.
**
**	@param value	Force number.
*/
local SCM CclAiAttackWithForce(SCM value)
{
    int force;

    force=gh_scm2int(value);
    if( force<0 || force>=AI_MAX_FORCES ) {
	errl("Force out of range",value);
    }

    AiAttackWithForce(force);

    return SCM_BOOL_F;
}

/**
**	Sleep n cycles.
**
**	@param value	Number of cycles to delay.
*/
local SCM CclAiSleep(SCM value)
{
    int i;

    DebugLevel3Fn("%lu %d\n" _C_ GameCycle _C_ AiPlayer->SleepCycles);
    if( AiPlayer->SleepCycles ) {
	if( AiPlayer->SleepCycles<GameCycle ) {
	    AiPlayer->SleepCycles=0;
	    return SCM_BOOL_F;
	}
    } else {
	i=gh_scm2int(value);
	AiPlayer->SleepCycles=GameCycle+i;
    }

    return SCM_BOOL_T;
}

/**
**	Research an upgrade.
**
**	@param value	Upgrade as string/symbol/object.
*/
local SCM CclAiResearch(SCM value)
{
    const char* str;
    Upgrade* upgrade;

    // Be kind allow also strings or symbols
    if( (str=try_get_c_string(value)) ) {
	upgrade=UpgradeByIdent(str);
    } else {
	errl("Upgrade needed",value);
	return SCM_BOOL_F;
    }

    InsertResearchRequests(upgrade);

    return SCM_BOOL_F;
}

/**
**	Upgrade an unit to an new unit-type.
**
**	@param value	Unit-type as string/symbol/object.
*/
local SCM CclAiUpgradeTo(SCM value)
{
    UnitType* type;

    type=CclGetUnitType(value);
    InsertUpgradeToRequests(type);

    return SCM_BOOL_F;
}

/**
**	Simple restart the AI.
*/
local SCM CclAiRestart(void)
{
    AiPlayer->Script=AiPlayer->AiType->Script;

    return SCM_BOOL_T;
}

/**
**	Execute new script
*/
local SCM CclAiScript(SCM value)
{
    AiPlayer->Script=value;

    return SCM_BOOL_T;
}

/**
**	Return the player of the running AI.
**
**	@return		Player number of the AI.
*/
local SCM CclAiPlayer(void)
{
    return gh_int2scm(AiPlayer->Player->Player);
}

/**
**	Set AI player resource reserve.
**
**	@param vec	Resources vector
**	@return		Old resource vector
*/
local SCM CclAiSetReserve(SCM vec)
{
    int i;
    SCM old;

    old=cons_array(gh_int2scm(MaxCosts),NIL);
    for( i=0; i<MaxCosts; ++i ) {
	aset1(old,gh_int2scm(i),gh_int2scm(AiPlayer->Reserve[i]));
    }
    for( i=0; i<MaxCosts; ++i ) {
	AiPlayer->Reserve[i]=gh_scm2int(gh_vector_ref(vec,gh_int2scm(i)));
    }
    return old;
}

/**
**	Set AI player resource collect percent.
**
**	@param vec	Resources vector
**	@return		Old resource vector
*/
local SCM CclAiSetCollect(SCM vec)
{
    int i;
    SCM old;

    old=cons_array(gh_int2scm(MaxCosts),NIL);
    for( i=0; i<MaxCosts; ++i ) {
	aset1(old,gh_int2scm(i),gh_int2scm(AiPlayer->Collect[i]));
    }
    for( i=0; i<MaxCosts; ++i ) {
	AiPlayer->Collect[i]=gh_scm2int(gh_vector_ref(vec,gh_int2scm(i)));
    }
    return old;
}

/**
**	Dump some AI debug informations.
*/
local SCM CclAiDump(void)
{
    int i;
    int n;
    const AiUnitType* aut;
    const AiBuildQueue* queue;

    //
    //	Script
    //
    printf("------\n");
    for( i=0; i<MaxCosts; ++i ) {
	printf("%s(%4d) ",DEFAULT_NAMES[i],AiPlayer->Player->Resources[i]);
    }
    printf("\n");
    printf("%d:",AiPlayer->Player->Player);
    gh_display(gh_car(AiPlayer->Script));
    gh_newline();

    //
    //	Requests
    //
    n=AiPlayer->UnitTypeRequestsCount;
    printf("UnitTypeRequests(%d):\n",n);
    for( i=0; i<n; ++i ) {
	printf("%s ",AiPlayer->UnitTypeRequests[i].Table[0]->Ident);
    }
    printf("\n");
    n=AiPlayer->UpgradeToRequestsCount;
    printf("UpgradeToRequests(%d):\n",n);
    for( i=0; i<n; ++i ) {
	printf("%s ",AiPlayer->UpgradeToRequests[i]->Ident);
    }
    printf("\n");
    n=AiPlayer->ResearchRequestsCount;
    printf("ResearchRequests(%d):\n",n);
    for( i=0; i<n; ++i ) {
	printf("%s ",AiPlayer->ResearchRequests[i]->Ident);
    }
    printf("\n");

    //
    //	Building queue
    //
    printf("Building queue:\n");
    for( queue=AiPlayer->UnitTypeBuilded; queue; queue=queue->Next ) {
	printf("%s(%d/%d) ",queue->Type->Ident,queue->Made,queue->Want);
    }
    printf("\n");

    //
    //	PrintForce
    //
    for( i=0; i<AI_MAX_FORCES; ++i) {
	printf("Force(%d%s%s%s):\n",i,
		AiPlayer->Force[i].Completed ? ",complete" : ",recruit",
		AiPlayer->Force[i].Attacking ? ",attack" : "",
		AiPlayer->Force[i].Defending ? ",defend" : "");
	for( aut=AiPlayer->Force[i].UnitTypes; aut; aut=aut->Next ) {
	    printf("%s(%d) ",aut->Type->Ident,aut->Want);
	}
	printf("\n");
    }

    return SCM_BOOL_F;
}

/**
**	Define AI mapping from original number to internal symbol
**
**	@param list	List of all names.
*/
local SCM CclDefineAiWcNames(SCM list)
{
    int i;
    char** cp;

    if( (cp=AiTypeWcNames) ) {		// Free all old names
	while( *cp ) {
	    free(*cp++);
	}
	free(AiTypeWcNames);
    }

    //
    //	Get new table.
    //
    i=gh_length(list);
    AiTypeWcNames=cp=malloc((i+1)*sizeof(char*));
    while( i-- ) {
	*cp++=gh_scm2newstr(gh_car(list),NULL);
	list=gh_cdr(list);
    }
    *cp=NULL;

    return SCM_UNSPECIFIED;
}

/**
**	Define an AI player.
**
**	@param list	List of the AI Player.
*/
local SCM CclDefineAiPlayer(SCM list __attribute__((unused)))
{
    SCM value;
    SCM sublist;
    int i;
    char* str;
    PlayerAi* ai;

    DebugLevel0Fn("FIXME: Ai Player loading not supported\n");

    i=gh_scm2int(gh_car(list));
    list=gh_cdr(list);

    DebugCheck( i<0 || i>PlayerMax );
    DebugLevel0Fn("%p %d\n" _C_ Players[i].Ai _C_ Players[i].AiEnabled );
    // FIXME: loose this:
    // DebugCheck( Players[i].Ai || !Players[i].AiEnabled );

    ai=Players[i].Ai=calloc(1,sizeof(PlayerAi));

    //
    //	Parse the list:	(still everything could be changed!)
    //
    while( !gh_null_p(list) ) {

	value=gh_car(list);
	list=gh_cdr(list);

	if( gh_eq_p(value,gh_symbol2scm("ai-type")) ) {
	    AiType* ait;

	    str=gh_scm2newstr(gh_car(list),NULL);
	    for( ait=AiTypes; ait; ait=ait->Next ) {
		if( !strcmp(ait->Name,str) ) {
		    break;
		}
	    }
	    free(str);
	    if( !ait ) {
	       errl("ai-type not found",gh_car(list));
	    }
	    ai->AiType=ait;
	    ai->Script=ait->Script;
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("script")) ) {
	    sublist=gh_car(list);
	    value=gh_car(sublist);
	    sublist=gh_cdr(sublist);
	    if( gh_eq_p(value,gh_symbol2scm("aitypes")) ) {
		i=gh_scm2int(gh_car(sublist));
		while( i-- ) {
		    ai->Script=gh_cdr(ai->Script);
		}
	    } else {
		DebugLevel0Fn("FIXME: not written\n");
	    }
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("script-debug")) ) {
	    ai->ScriptDebug=gh_scm2bool(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("sleep-cycles")) ) {
	    i=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("force")) ) {
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("reserve")) ) {
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("used")) ) {
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("needed")) ) {
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("collect")) ) {
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("need-mask")) ) {
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("need-food")) ) {
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("unit-type")) ) {
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("upgrade")) ) {
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("research")) ) {
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("building")) ) {
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("repair-building")) ) {
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("repair-workers")) ) {
	    list=gh_cdr(list);
	} else {
	   // FIXME: this leaves a half initialized ai player
	   errl("Unsupported tag",value);
	}
    }

    return SCM_UNSPECIFIED;
}

#else

/**
**	Define helper for AI.
**
**	@param list	List of the AI player.
*/
local SCM CclDefineAiHelper(SCM list)
{
    return list;
}

/**
**	Define an AI engine.
**
**	@param list	List of the AI.
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
    gh_new_procedure0_0("ai:get-race",CclAiGetRace);
    gh_new_procedure0_0("ai:get-sleep-cycles",CclAiGetSleepCycles);

    gh_new_procedure1_0("ai:debug",CclAiDebug);
    gh_new_procedure1_0("ai:need",CclAiNeed);
    gh_new_procedure2_0("ai:set",CclAiSet);
    gh_new_procedure1_0("ai:wait",CclAiWait);
    gh_new_procedureN("ai:force",CclAiForce);
    gh_new_procedure2_0("ai:force-role",CclAiForceRole);
    gh_new_procedure1_0("ai:check-force",CclAiCheckForce);
    gh_new_procedure1_0("ai:wait-force",CclAiWaitForce);
    gh_new_procedure1_0("ai:attack-with-force",CclAiAttackWithForce);
    gh_new_procedure1_0("ai:sleep",CclAiSleep);
    gh_new_procedure1_0("ai:research",CclAiResearch);
    gh_new_procedure1_0("ai:upgrade-to",CclAiUpgradeTo);
    gh_new_procedure1_0("ai:script",CclAiScript);
    gh_new_procedure0_0("ai:restart",CclAiRestart);

    gh_new_procedure0_0("ai:player",CclAiPlayer);
    gh_new_procedure1_0("ai:set-reserve!",CclAiSetReserve);
    gh_new_procedure1_0("ai:set-collect!",CclAiSetCollect);

    gh_new_procedure0_0("ai:dump",CclAiDump);

    gh_new_procedureN("define-ai-wc-names",CclDefineAiWcNames);

    gh_new_procedureN("define-ai-player",CclDefineAiPlayer);
#endif
}

//@}
