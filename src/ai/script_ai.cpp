//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name ccl_ai.c	-	The AI ccl functions. */
//
//      (c) Copyright 2000-2002 by Lutz Sammer and Ludovic Pollet
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//
//      $Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "unittype.h"
#include "ccl.h"
#include "ai.h"
#include "pathfinder.h"

#include "ai_local.h"

#include "ccl_helpers.h"


#if defined(USE_GUILE) || defined(USE_SIOD)
/*----------------------------------------------------------------------------
--	Forwards
----------------------------------------------------------------------------*/
/// Handle saving/loading a reference to an AiType.
local void IOAiTypePtr(SCM from, void* binaryform, void* para);
/// Handle saving/loading a reference to an AiScriptAction.
local void IOAiScriptActionPtr(SCM scmfrom, void* binaryform, void* para);
/// Handle saving/loading an array of int for ressources.
local void IORessourceArray(SCM scmfrom, void* binaryform, void* para);
/// Handle saving/loading a ressource mask
local void IORessourceMask(SCM scmfrom, void* binaryform, void* para);

/*----------------------------------------------------------------------------
--	Constants
----------------------------------------------------------------------------*/

/// Description of the AiActionEvaluation structure
static IOStructDef AiActionEvaluationStructDef = {
    "AiActionEvaluation",
    sizeof(AiActionEvaluation),
    -1,
    {
	{"`next", 		NULL, 		&((AiActionEvaluation*)0)->Next, 	NULL},
	{"ai-script-action",	&IOAiScriptActionPtr,&((AiActionEvaluation*)0)->aiScriptAction,NULL},
	{"gamecycle",		&IOInt,		&((AiActionEvaluation*)0)->gamecycle,	NULL},
	{"hotspot-x", 		&IOInt,		&((AiActionEvaluation*)0)->hotSpotX, 	NULL},
	{"hotspot-y", 		&IOInt,		&((AiActionEvaluation*)0)->hotSpotY,	NULL},
	{"hotspot-value", 	&IOInt,		&((AiActionEvaluation*)0)->hotSpotValue,NULL},
	{"value", 		&IOInt,		&((AiActionEvaluation*)0)->value,	NULL},
	{0, 0, 0, 0}
    }
};

static IOStructDef AiExplorationRequestStructDef = {
    "AiExplorationRequest",
    sizeof(AiExplorationRequest),
    -1,
    {
	{"`next", 		NULL, 		&((AiExplorationRequest*)0)->Next, 	NULL},
	{"gamecycle",		&IOInt,		&((AiExplorationRequest*)0)->Mask,	NULL},
	{"map-x", 		&IOInt,		&((AiExplorationRequest*)0)->X, 	NULL},
	{"map-y", 		&IOInt,		&((AiExplorationRequest*)0)->Y,		NULL},
	{0, 0, 0, 0}
    }
};

/// Description of the AiRunningScript structure
static IOStructDef AiRunningScriptStructDef = {
    "AiRunningScript",
    sizeof(AiRunningScript),
    AI_MAX_RUNNING_SCRIPTS,
    {
	{"script", 		&IOCcl,		&((AiRunningScript*)0)->Script, 	NULL},
	{"sleep-cycles", 	&IOInt,		&((AiRunningScript*)0)->SleepCycles, 	NULL},
	{"ident", 		&IOStrBuffer,	&((AiRunningScript*)0)->ident,		(void*) 10},
	{"hotspot-x",		&IOInt,		&((AiRunningScript*)0)->HotSpotX,	NULL},
	{"hotspot-y",		&IOInt,		&((AiRunningScript*)0)->HotSpotY,	NULL},
	{"hotspot-ray",		&IOInt,		&((AiRunningScript*)0)->HotSpotRay,	NULL},
	{"own-force",		&IOInt,		&((AiRunningScript*)0)->ownForce,	NULL},
	{"gauges",		&IOIntArrayPtr,	&((AiRunningScript*)0)->gauges,		(void*) GAUGE_NB},
	{0, 0, 0, 0}
    }
};

/// Description of the role flags
static IOFlagDef AiRoleFlag[] = {
    {"attack", AiForceRoleAttack}, {"defend", AiForceRoleDefend}, {0, 0}
};

/// Description of the populate flags
static IOFlagDef AiPopulateFlag[] = {
    {"dont-populate", AiForceDontPopulate}, {"from-scratch", AiForcePopulateFromScratch},
    {"from-attack", AiForcePopulateFromAttack}, {"any", AiForcePopulateAny}, {0, 0}
};

/// Description of the help flags
static IOFlagDef AiHelpFlag[] = {
    {"no-help", AiForceDontHelp}, {"force-help", AiForceHelpForce},
    {"full-help", AiForceHelpFull}, {0, 0}
};

/// Description of the AiUnitType structure
static IOStructDef AiUnitTypeStructDef = {
    "AiUnitType",
    sizeof(AiUnitType),
    -1,
    {
	{"'next", 		0, 		&((AiUnitType*)0)->Next,	0},
	{"type", 		&IOUnitTypePtr,	&((AiUnitType*)0)->Type, 	0},
	{"want", 		&IOInt,		&((AiUnitType*)0)->Want,	0},
	{0, 0, 0, 0}
    }
};

/// Description of the AiUnit structure
static IOStructDef AiUnitStructDef = {
    "AiUnit",
    sizeof(AiUnit),
    -1,
    {
	{"'next", 		NULL, 		&((AiUnit*)0)->Next,		0},
	{"unit", 		&IOUnitPtr,	&((AiUnit*)0)->Unit, 		0},
	{0, 0, 0, 0}
    }
};

/// Description of the AiForce structure
static IOStructDef AiForceStructDef = {
    "AiForce",
    sizeof(AiForce),
    AI_MAX_FORCES,
    {
	{"completed", 		&IOCharBool,	&((AiForce*)0)->Completed, 	0},
	{"attacking", 		&IOCharBool, 	&((AiForce*)0)->Attacking, 	0},
	{"role",		&IOCharFlag, 	&((AiForce*)0)->Role, 		&AiRoleFlag},
	{"populate-mode",	&IOCharFlag, 	&((AiForce*)0)->PopulateMode,	&AiPopulateFlag},
	{"units-reusable",	&IOCharBool, 	&((AiForce*)0)->UnitsReusable, 	0},
	{"help-mode",		&IOCharFlag, 	&((AiForce*)0)->HelpMode, 	&AiHelpFlag},
	{"unit-wants",		&IOLinkedList, 	&((AiForce*)0)->UnitTypes,	&AiUnitTypeStructDef},
	{"unit-presents",	&IOLinkedList, 	&((AiForce*)0)->Units, 		&AiUnitStructDef},
	{"attack-state",	&IOInt, 	&((AiForce*)0)->State, 		0},
	{"attack-goal-x",	&IOInt, 	&((AiForce*)0)->GoalX, 		0},
	{"attack-goal-y",	&IOInt, 	&((AiForce*)0)->GoalY, 		0},
	{"must-transport",	&IOBool, 	&((AiForce*)0)->MustTransport, 	0},
	{0, 0, 0, 0}
    }
};

/// Description of the AiBuildQueue structure/linked list
static IOStructDef AiBuildQueueStructDef = {
    "AiBuildQueue",
    sizeof(AiBuildQueue),
    -1,
    {
	{"`next", 		0, 		&((AiBuildQueue*)0)->Next, 	0},
	{"want", 		&IOInt,		&((AiBuildQueue*)0)->Want,	0},
	{"made", 		&IOInt,		&((AiBuildQueue*)0)->Made,	0},
	{"type", 		&IOUnitTypePtr,	&((AiBuildQueue*)0)->Type,	0},
	{0, 0, 0, 0}
    }
};

/// Description of the AiUnitTypeTable table in PlayerAi
static IOStructDef AiUnitTypeTableStructDef = {
    "AiUnitTypeTable",
    sizeof(AiUnitTypeTable),
    -1,
    {
	{"unittype", 		&IOUnitTypePtr,	&((AiUnitTypeTable*)0)->Table,	0},
	{"count", 		&IOInt,		&((AiUnitTypeTable*)0)->Count,	0},
	{0, 0, 0, 0}
    }
};


/// Description of the UnitTypeRequests table in PlayerAi
static IOStructDef UnitTypeRequestsTableDef = {
    "UnitTypeRequests",
    sizeof(AiUnitTypeTable),
    -1,
    {
	{"`ptr", 		0,		&((PlayerAi*)0)->UnitTypeRequests, 	0},
	{"`count", 		0,		&((PlayerAi*)0)->UnitTypeRequestsCount,	0},
	{"`items", 		&IOStruct,	0, 					&AiUnitTypeTableStructDef},
	{0, 0, 0, 0}
    }
};

/// Description of the UpgradeToRequests table in PlayerAi
static IOStructDef UpgradeToRequestsTableDef = {
    "UpgradeToRequests",
    sizeof(UnitType*),
    -1,
    {
	{"`ptr", 		0,		&((PlayerAi*)0)->UpgradeToRequests,	0},
	{"`count",		0,		&((PlayerAi*)0)->UpgradeToRequestsCount,0},
	{"`items",		&IOUnitTypePtr,	0,					0},
	{0, 0, 0, 0}
    }
};

/// Description of the ResearchRequests table in PlayerAi
static IOStructDef ResearchRequestsTableDef = {
    "ResearchRequests",
    sizeof(Upgrade*),
    -1,
    {
	{"`ptr",		0,		&((PlayerAi*)0)->ResearchRequests,	0},
	{"`count",		0,		&((PlayerAi*)0)->ResearchRequestsCount,	0},
	{"`items",		&IOUpgradePtr,	0,					0},
	{0, 0, 0, 0}
    }
};

/// Description of the PlayerAi structure
static IOStructDef PlayerAiStructDef = {
    "PlayerAi",
    sizeof(PlayerAi),
    -1,
    {
	{"player",		&IOPlayerPtr,	&((PlayerAi*)0)->Player,		0},
	{"ai-type",		&IOAiTypePtr,	&((PlayerAi*)0)->AiType,		0},
	{"scripts",		&IOStructArray,	&((PlayerAi*)0)->Scripts,		&AiRunningScriptStructDef},
	{"past-evaluations",	&IOLinkedList,	&((PlayerAi*)0)->FirstEvaluation,	&AiActionEvaluationStructDef},
	{"debug",		&IOBool,	&((PlayerAi*)0)->ScriptDebug,		0},
	{"auto-attack",		&IOBool,	&((PlayerAi*)0)->AutoAttack,		0},
	{"forces",		&IOStructArray,	&((PlayerAi*)0)->Force,			&AiForceStructDef},
	{"reserve",		&IORessourceArray,&((PlayerAi*)0)->Reserve, 		0},
	{"used",		&IORessourceArray,&((PlayerAi*)0)->Used,		0},
	{"needed",		&IORessourceArray,&((PlayerAi*)0)->Needed,		0},
	{"collect", 		&IORessourceArray,&((PlayerAi*)0)->Collect,		0},
	{"neededmask",		&IORessourceMask,&((PlayerAi*)0)->Reserve,		0},
	{"need-supply",		&IOBool,	&((PlayerAi*)0)->NeedSupply,		0},
	{"exploration-requests",&IOLinkedList,	&((PlayerAi*)0)->FirstExplorationRequest,&AiExplorationRequestStructDef},
	{"last-exploration",	&IOInt,		&((PlayerAi*)0)->LastExplorationGameCycle,0},
	{"unit-type-requests",	&IOTable,	0,					&UnitTypeRequestsTableDef},
	{"upgrade-to-requests",	&IOTable,	0,					&UpgradeToRequestsTableDef},
	{"research-requests",	&IOTable,	0,					&ResearchRequestsTableDef},
	{"unit-type-builded",	&IOLinkedList,	&((PlayerAi*)0)->UnitTypeBuilded,	&AiBuildQueueStructDef},
	{"last-repair-building",&IOInt,		&((PlayerAi*)0)->LastRepairBuilding,	0},
	{"tried-repair-worker",	&IOIntArray,	&((PlayerAi*)0)->TriedRepairWorkers,	(void*)UnitMax},
	{0, 0, 0, 0}
    }
};

/// Description of the PlayerAi structure
static IOStructDef AiTypeStructDef = {
    "AiType",
    sizeof(AiType),
    -1,
    {
	{"name", 		&IOString,	&((AiType*)0)->Name,		0},
	{"race", 		&IOString,	&((AiType*)0)->Race,		0},
	{"class",		&IOString,	&((AiType*)0)->Class,		0},
	{"script",		&IOCcl,		&((AiType*)0)->Script,		0},
	{0, 0, 0, 0}
    }
};

static IOStructDef AiScriptActionStructDef = {
    "AiScriptAction",
    sizeof(AiScriptAction),
    -1,
    {
	{"action",		&IOCcl,		&((AiScriptAction*)0)->Action,		0},
	{"defensive",		&IOBool,	&((AiScriptAction*)0)->Defensive,	0},
	{"offensive",		&IOBool,	&((AiScriptAction*)0)->Offensive,	0},
	{0, 0, 0, 0}
    }
};


/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Handle saving/loading a reference to an AiType ( AiType* ).
**	The null case is handled.
**
**	@param	scmform		When loading, the scm data to load
**	@param	binaryform	Pointer to the unit'ref ( AiType** )
**	@param	para		unused
*/
local void IOAiTypePtr(SCM from, void* binaryform, void* para)
{
    char buffer[512];
    char* str;
    AiType* cur;

    if (IOHandleNullPtr(from, binaryform)) {
	return;
    }
    if (IOLoadingMode) {
	str = gh_scm2newstr(from, 0);
	cur = AiTypes;
	while (cur) {
	    snprintf(buffer, 512, "%s-%s-%s", cur->Name, cur->Race, cur->Class);
	    if (!strcmp(str, buffer)) {
		*((AiType**)binaryform) = cur;
		return;
	    }
	    cur = cur->Next;
	}
	errl("unknown aitype ", from);
    } else {
	cur = *((AiType**)binaryform);

	snprintf(buffer, 512, "%s-%s-%s", cur->Name, cur->Race, cur->Class);
	CLprintf(IOOutFile, " \"%s\"", buffer);
    }
}


/**
**	Handle saving/loading a reference to an AiScriptAction.
**	The null case is handled.
**
**	@param	scmform		When loading, the scm data to load
**	@param	binaryform	Pointer to the unit'ref ( AiScriptAction** )
**	@param	para		unused
*/
local void IOAiScriptActionPtr(SCM scmfrom, void* binaryform, void* para)
{
    int slot;
    AiScriptAction* a;
    if (IOHandleNullPtr(scmfrom, binaryform)) {
	return;
    }
    if (IOLoadingMode) {
	slot = gh_scm2int(scmfrom);
	*((AiScriptAction**)binaryform) = AiScriptActions + slot;
    } else {
	a = *((AiScriptAction**)binaryform);
	CLprintf(IOOutFile, " %d", a - AiScriptActions);
    }
}

/// Handle loading an array of int for each ressource ( int[MAX_COSTS] )
local void IORessourceArray(SCM scmfrom, void* binaryform, void* para)
{
    IOIntArray(scmfrom, binaryform, (void*)MaxCosts);
}

/// Handle loading a mask for each ressource ( int[MAX_COSTS] )
local void IORessourceMask(SCM scmfrom, void* binaryform, void* para)
{
    int tmp[MaxCosts];
    int mask;
    int i;

    if (IOLoadingMode) {
	IOIntArray(scmfrom, tmp, (void*)MaxCosts);

	mask = 0;
	for (i = 0; i < MaxCosts; ++i) {
	    if (tmp[i]) {
		mask |= (1 << i);
	    }
	}

	*(int*)binaryform = mask;
    } else {
	mask = *(int*)binaryform;
	for (i = 0; i < MaxCosts; ++i) {
	    if (mask & (1 << i)) {
		tmp[i] = 1;
	    } else {
		tmp[i] = 0;
	    }
	}

	IOIntArray(scmfrom, tmp, (void*)MaxCosts);
    }
}

/**
**	Handle saving/loading a full PlayerAi structure.
**	The structure is allocated on the heap, filled from ccl, then completed.
**
**	@param	scmform		When loading, the scm data to load
**	@param	binaryform	Pointer to the PlayerAi'ref ( PlayerAi** )
**	@param	para		unused
*/
global void IOPlayerAiFullPtr(SCM form, void* binaryform, void* para)
{
    AiActionEvaluation* aa;
    PlayerAi** playerAi = (PlayerAi**)binaryform;

    IOStructPtr(form, binaryform, &PlayerAiStructDef);
    if (IOLoadingMode && (*playerAi)) {
	// Finalize the playerAi struct !
	// => last evaluation, evaluation count
	aa = (*playerAi)->FirstEvaluation;
	while (aa) {
	    (*playerAi)->LastEvaluation = aa;
	    (*playerAi)->EvaluationCount++;
	    aa = aa->Next;
	}
    }
}

global void IOAiTypeFullPtr(SCM form, void* binaryform, void* para)
{
    AiType** aiType = (AiType**)binaryform;
    IOStructPtr(form, binaryform, &AiTypeStructDef);
    if (IOLoadingMode && (*aiType)) {
	// Append the ai_type...
	(*aiType)->Next = AiTypes;
	AiTypes = (*aiType);
    }
}

global void IOAiScriptActionFull(SCM form, void* binaryform, void* para)
{
    /*AiScriptAction* asa = (AiScriptAction*)binaryform; */
    IOStruct(form, binaryform, &AiScriptActionStructDef);
}


#define INCOMPLETE_SIOD	1

#ifdef INCOMPLETE_SIOD

local SCM CclQuotient(SCM a, SCM b)
{
    int va, vb;
    va = gh_scm2int(a);
    vb = gh_scm2int(b);
    if (vb == 0) {
	errl("CclQuotient division by zero", b);
    }
    return gh_int2scm(va / vb);
}

local SCM CclOutput(SCM x)
{
    if (gh_null_p(x)) {
	printf(" '()");
	return SCM_BOOL_T;
    }
    if (gh_list_p(x)) {
	printf(" (");
	while (!gh_null_p(x)) {
	    CclOutput(gh_car(x));
	    x = gh_cdr(x);
	}
	printf(" )");
	return SCM_BOOL_T;
    }
    printf(" ");
    gh_display(x);
    return x;
}

#endif
#elif defined(USE_LUA)
#endif


/**
**	Setup AI helper table.
**
**	Expand the table if needed.
**
**	@param count	Pointer to number of elements in table.
**	@param table	Pointer to table with elements.
**	@param n	Index to insert new into table
*/
local void AiHelperSetupTable(int* count, AiUnitTypeTable*** table, int n)
{
    int i;

    ++n;
    if (n > (i = *count)) {
	if (*table) {
	    *table = realloc(*table, n * sizeof(AiUnitTypeTable*));
	    memset((*table) + i, 0, (n - i) * sizeof(AiUnitTypeTable*));
	} else {
	    *table = malloc(n * sizeof(AiUnitTypeTable*));
	    memset(*table, 0, n * sizeof(AiUnitTypeTable*));
	}
	*count = n;
    }
}

/**
**	Insert new unit-type element.
**
**	@param tablep	Pointer to table with elements.
**	@param base	Base type to insert into table.
*/
local void AiHelperInsert(AiUnitTypeTable** tablep, UnitType* base)
{
    int i;
    int n;
    AiUnitTypeTable* table;

    //
    //  New unit-type
    //
    if (!(table = *tablep)) {
	table = *tablep = malloc(sizeof(AiUnitTypeTable));
	table->Count = 1;
	table->Table[0] = base;
	return;
    }
    //
    //  Look if already known.
    //
    n = table->Count;
    for (i = 0; i < n; ++i) {
	if (table->Table[i] == base) {
	    return;
	}
    }

    //
    //  Append new base unit-type to units.
    //
    table = *tablep = realloc(table, sizeof(AiUnitTypeTable) + sizeof(UnitType*) * n);
    table->Count = n + 1;
    table->Table[n] = base;
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
#if defined(USE_GUILE) || defined(USE_SIOD)
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

#ifdef DEBUG
    type = NULL;
    upgrade = NULL;
    cost = 0;
#endif
    while (!gh_null_p(list)) {
	sub_list = gh_car(list);
	list = gh_cdr(list);

	//
	//      Type build,train,research/upgrade.
	//
	value = gh_car(sub_list);
	sub_list = gh_cdr(sub_list);
	if (gh_eq_p(value, gh_symbol2scm("build"))) {
	    what = 0;
	} else if (gh_eq_p(value, gh_symbol2scm("train"))) {
	    what = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("upgrade"))) {
	    what = 2;
	} else if (gh_eq_p(value, gh_symbol2scm("research"))) {
	    what = 3;
	} else if (gh_eq_p(value, gh_symbol2scm("unit-limit"))) {
	    what = 4;
	} else if (gh_eq_p(value, gh_symbol2scm("unit-equiv"))) {
	    what = 5;
	} else if (gh_eq_p(value, gh_symbol2scm("repair"))) {
	    what = 6;
	} else {
	    fprintf(stderr, "unknown tag\n");
	    continue;
	}

	//
	//      Get the base unit type, which could handle the action.
	//
	value = gh_car(sub_list);
	sub_list = gh_cdr(sub_list);

	// FIXME: support value as list!
	str = gh_scm2newstr(value, NULL);
	base = UnitTypeByIdent(str);
	if (!base) {
	    fprintf(stderr, "unknown unittype %s\n", str);
	    free(str);
	    continue;
	}
	DebugLevel3Fn("%s\n" _C_ base->Name);
	free(str);

	//
	//      Get the unit types, which could be produced
	//
	while (!gh_null_p(sub_list)) {
	    value = gh_car(sub_list);
	    sub_list = gh_cdr(sub_list);
	    str = gh_scm2newstr(value, NULL);
	    if (what == 3) {
		upgrade = UpgradeByIdent(str);
		if (!upgrade) {
		    fprintf(stderr, "unknown upgrade %s\n", str);
		    free(str);
		    continue;
		}
		DebugLevel3Fn("> %s\n" _C_ upgrade->Ident);
	    } else if (what == 4) {
		if (!strcmp("food", str)) {
		    cost = 0;
		} else {
		    fprintf(stderr, "unknown limit %s\n", str);
		    free(str);
		    continue;
		}
		DebugLevel3Fn("> %s\n" _C_ str);
	    } else {
		type = UnitTypeByIdent(str);
		if (!type) {
		    fprintf(stderr, "unknown unittype %s\n", str);
		    free(str);
		    continue;
		}
		DebugLevel3Fn("> %s\n" _C_ type->Name);
	    }
	    free(str);

	    switch (what) {
		case 0:		// build
		    AiHelperSetupTable(&AiHelpers.BuildCount, &AiHelpers.Build,
			type->Type);
		    AiHelperInsert(AiHelpers.Build + type->Type, base);
		    break;
		case 1:		// train
		    AiHelperSetupTable(&AiHelpers.TrainCount, &AiHelpers.Train,
			type->Type);
		    AiHelperInsert(AiHelpers.Train + type->Type, base);
		    break;
		case 2:		// upgrade
		    AiHelperSetupTable(&AiHelpers.UpgradeCount, &AiHelpers.Upgrade,
			type->Type);
		    AiHelperInsert(AiHelpers.Upgrade + type->Type, base);
		    break;
		case 3:		// research
		    AiHelperSetupTable(&AiHelpers.ResearchCount, &AiHelpers.Research,
			upgrade - Upgrades);
		    AiHelperInsert(AiHelpers.Research + (upgrade - Upgrades), base);
		    break;
		case 4:		// unit-limit
		    AiHelperSetupTable(&AiHelpers.UnitLimitCount, &AiHelpers.UnitLimit,
			cost);
		    AiHelperInsert(AiHelpers.UnitLimit + cost, base);
		    break;
		case 5:		// equivalence
		    AiHelperSetupTable(&AiHelpers.EquivCount, &AiHelpers.Equiv,
			base->Type);
		    AiHelperInsert(AiHelpers.Equiv + base->Type, type);

		    AiNewUnitTypeEquiv(base, type);
		    break;
		case 6:		// repair
		    AiHelperSetupTable(&AiHelpers.RepairCount, &AiHelpers.Repair,
			type->Type);
		    AiHelperInsert(AiHelpers.Repair + type->Type, base);
		    break;
	    }
	}
    }

    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDefineAiHelper(lua_State* l)
{
    const char* value;
    int what;
    UnitType* base;
    UnitType* type;
    Upgrade* upgrade;
    int cost;
    int args;
    int j;
    int subargs;
    int k;

#ifdef DEBUG
    type = NULL;
    upgrade = NULL;
    cost = 0;
#endif
    args = lua_gettop(l);
    for (j = 0; j < args; ++j) {
	if (!lua_istable(l, j + 1)) {
	    lua_pushstring(l, "incorrect argument");
	    lua_error(l);
	}
	subargs = luaL_getn(l, j + 1);
	k = 0;
	lua_rawgeti(l, j + 1, k + 1);
	value = LuaToString(l, -1);
	lua_pop(l, 1);
	++k;

	//
	//      Type build,train,research/upgrade.
	//
	if (!strcmp(value, "build")) {
	    what = 0;
	} else if (!strcmp(value, "train")) {
	    what = 1;
	} else if (!strcmp(value, "upgrade")) {
	    what = 2;
	} else if (!strcmp(value, "research")) {
	    what = 3;
	} else if (!strcmp(value, "unit-limit")) {
	    what = 4;
	} else if (!strcmp(value, "unit-equiv")) {
	    what = 5;
	} else if (!strcmp(value, "repair")) {
	    what = 6;
	} else {
	    lua_pushfstring(l, "unknown tag: %s", value);
	    lua_error(l);
	}

	//
	//      Get the base unit type, which could handle the action.
	//

	// FIXME: support value as list!
	lua_rawgeti(l, j + 1, k + 1);
	value = LuaToString(l, -1);
	lua_pop(l, 1);
	++k;
	base = UnitTypeByIdent(value);
	if (!base) {
	    lua_pushfstring(l, "unknown unittype: %s", value);
	    lua_error(l);
	}
	DebugLevel3Fn("%s\n" _C_ base->Name);

	//
	//      Get the unit types, which could be produced
	//
	for (; k < subargs; ++k) {
	    lua_rawgeti(l, j + 1, k + 1);
	    value = LuaToString(l, -1);
	    lua_pop(l, 1);
	    if (what == 3) {
		upgrade = UpgradeByIdent(value);
		if (!upgrade) {
		    lua_pushfstring(l, "unknown upgrade: %s", value);
		    lua_error(l);
		}
		DebugLevel3Fn("> %s\n" _C_ upgrade->Ident);
	    } else if (what == 4) {
		if (!strcmp("food", value)) {
		    cost = 0;
		} else {
		    lua_pushfstring(l, "unknown limit: %s", value);
		    lua_error(l);
		}
		DebugLevel3Fn("> %s\n" _C_ str);
	    } else {
		type = UnitTypeByIdent(value);
		if (!type) {
		    lua_pushfstring(l, "unknown unittype: %s", value);
		    lua_error(l);
		}
		DebugLevel3Fn("> %s\n" _C_ type->Name);
	    }

	    switch (what) {
		case 0:		// build
		    AiHelperSetupTable(&AiHelpers.BuildCount, &AiHelpers.Build,
			type->Type);
		    AiHelperInsert(AiHelpers.Build + type->Type, base);
		    break;
		case 1:		// train
		    AiHelperSetupTable(&AiHelpers.TrainCount, &AiHelpers.Train,
			type->Type);
		    AiHelperInsert(AiHelpers.Train + type->Type, base);
		    break;
		case 2:		// upgrade
		    AiHelperSetupTable(&AiHelpers.UpgradeCount, &AiHelpers.Upgrade,
			type->Type);
		    AiHelperInsert(AiHelpers.Upgrade + type->Type, base);
		    break;
		case 3:		// research
		    AiHelperSetupTable(&AiHelpers.ResearchCount, &AiHelpers.Research,
			upgrade - Upgrades);
		    AiHelperInsert(AiHelpers.Research + (upgrade - Upgrades), base);
		    break;
		case 4:		// unit-limit
		    AiHelperSetupTable(&AiHelpers.UnitLimitCount, &AiHelpers.UnitLimit,
			cost);
		    AiHelperInsert(AiHelpers.UnitLimit + cost, base);
		    break;
		case 5:		// equivalence
		    AiHelperSetupTable(&AiHelpers.EquivCount, &AiHelpers.Equiv,
			base->Type);
		    AiHelperInsert(AiHelpers.Equiv + base->Type, type);

		    AiNewUnitTypeEquiv(base, type);
		    break;
		case 6:		// repair
		    AiHelperSetupTable(&AiHelpers.RepairCount, &AiHelpers.Repair,
			type->Type);
		    AiHelperInsert(AiHelpers.Repair + type->Type, base);
		    break;
	    }
	}
    }

    return 0;
}
#endif

#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineAiAction(SCM type, SCM definition)
{
    AiScriptAction* aiScriptAction;

    aiScriptAction = AiScriptActions + AiScriptActionNum;
    ++AiScriptActionNum;

    memset(aiScriptAction, 0, sizeof(AiScriptAction));

    aiScriptAction->Action = definition;
    CclGcProtect(&aiScriptAction->Action);

    while (!gh_null_p(type)) {
	if (gh_eq_p(gh_car(type), gh_symbol2scm("defense"))) {
	    aiScriptAction->Defensive = 1;
	} else if (gh_eq_p(gh_car(type), gh_symbol2scm("attack"))) {
	    aiScriptAction->Offensive = 1;
	} else {
	    errl("Unsupported ai action type", gh_car(type));
	}
	type = gh_cdr(type);
    }

    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDefineAiAction(lua_State* l)
{
    AiScriptAction* aiScriptAction;
    const char* value;
    int args;
    int j;

    if (lua_gettop(l) != 2 || !lua_istable(l, 1) || !lua_istable(l, 2)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    aiScriptAction = AiScriptActions + AiScriptActionNum;
    ++AiScriptActionNum;

    memset(aiScriptAction, 0, sizeof(AiScriptAction));

//    aiScriptAction->Action = definition;
//    CclGcProtect(&aiScriptAction->Action);

    args = luaL_getn(l, 1);
    for (j = 0; j < args; ++j) {
	lua_rawgeti(l, 1, j + 1);
	value = LuaToString(l, -1);
	lua_pop(l, 1);
	if (!strcmp(value, "defense")) {
	    aiScriptAction->Defensive = 1;
	} else if (!strcmp(value, "attack")) {
	    aiScriptAction->Offensive = 1;
	} else {
	    lua_pushfstring(l, "Unsupported ai action type: %s", value);
	    lua_error(l);
	}
    }

    return 0;
}
#endif

/**
**	Define an AI engine.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineAi(SCM list)
{
    SCM value;
    char* str;
    AiType* aitype;
#ifdef DEBUG
    const AiType* ait;
#endif

    aitype = malloc(sizeof(AiType));
    aitype->Next = AiTypes;
    AiTypes = aitype;

    //
    //  AI Name
    //
    value = gh_car(list);
    list = gh_cdr(list);
    str = gh_scm2newstr(value, NULL);
    DebugLevel3Fn("%s\n" _C_ str);
    aitype->Name = str;

#ifdef DEBUG
    for (ait = AiTypes->Next; ait; ait = ait->Next) {
	if (!strcmp(aitype->Name, ait->Name)) {
	    DebugLevel0Fn("Warning two or more AI's with the same name '%s'\n" _C_ ait->
		Name);
	}
    }
#endif

    //
    //  AI Race
    //
    value = gh_car(list);
    list = gh_cdr(list);
    str = gh_scm2newstr(value, NULL);
    DebugLevel3Fn("%s\n" _C_ str);
    if (*str != '*') {
	aitype->Race = str;
    } else {
	aitype->Race = NULL;
	free(str);
    }

    //
    //  AI Class
    //
    value = gh_car(list);
    list = gh_cdr(list);
    str = gh_scm2newstr(value, NULL);
    DebugLevel3Fn("%s\n" _C_ str);
    aitype->Class = str;

    //
    //  AI Script
    //
    value = gh_car(list);
    list = gh_cdr(list);
    aitype->Script = value;

    // Protect the scheme script against GC garbage-collect.
    CclGcProtect(&aitype->Script);

    return list;
}
#elif defined(USE_LUA)
local int CclDefineAi(lua_State* l)
{
    const char* value;
    AiType* aitype;
#ifdef DEBUG
    const AiType* ait;
#endif
    int args;
    int j;

    args = lua_gettop(l);
    j = 0;

    aitype = malloc(sizeof(AiType));
    aitype->Next = AiTypes;
    AiTypes = aitype;

    //
    //  AI Name
    //
    aitype->Name = strdup(LuaToString(l, j + 1));
    ++j;
    DebugLevel3Fn("%s\n" _C_ aitype->Name);

#ifdef DEBUG
    for (ait = AiTypes->Next; ait; ait = ait->Next) {
	if (!strcmp(aitype->Name, ait->Name)) {
	    DebugLevel0Fn("Warning two or more AI's with the same name '%s'\n" _C_ ait->
		Name);
	}
    }
#endif

    //
    //  AI Race
    //
    value = LuaToString(l, j + 1);
    ++j;
    DebugLevel3Fn("%s\n" _C_ value);
    if (*value != '*') {
	aitype->Race = strdup(value);
    } else {
	aitype->Race = NULL;
    }

    //
    //  AI Class
    //
    aitype->Class = strdup(LuaToString(l, j + 1));
    ++j;
    DebugLevel3Fn("%s\n" _C_ aitype->Class);

    //
    //  AI Script
    //
//    aitype->Script = value;

    return 0;
}
#endif

/*----------------------------------------------------------------------------
--	AI script functions
----------------------------------------------------------------------------*/

#if defined(USE_GUILE) || defined(USE_SIOD)
    /// Get unit-type.
extern UnitType* CclGetUnitType(SCM ptr);
#elif defined(USE_LUA)
    /// Get unit-type.
extern UnitType* CclGetUnitType(lua_State* l);
#endif

/**
**	Append unit-type to request table.
**
**	@param type	Unit-type to be appended.
**	@param count	How many unit-types to build.
*/
local void InsertUnitTypeRequests(UnitType* type, int count)
{
    int n;

    if (AiPlayer->UnitTypeRequests) {
	n = AiPlayer->UnitTypeRequestsCount;
	AiPlayer->UnitTypeRequests = realloc(AiPlayer->UnitTypeRequests,
	    (n + 1) * sizeof(*AiPlayer->UnitTypeRequests));
    } else {
	AiPlayer->UnitTypeRequests = malloc(sizeof(*AiPlayer->UnitTypeRequests));
	n = 0;
    }
    AiPlayer->UnitTypeRequests[n].Table[0] = type;
    AiPlayer->UnitTypeRequests[n].Count = count;
    AiPlayer->UnitTypeRequestsCount = n + 1;
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

    n = AiPlayer->UnitTypeRequestsCount;
    for (i = 0; i < n; ++i) {
	if (AiPlayer->UnitTypeRequests[i].Table[0] == type) {
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

    n = AiPlayer->UpgradeToRequestsCount;
    for (i = 0; i < n; ++i) {
	if (AiPlayer->UpgradeToRequests[i] == type) {
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

    if (AiPlayer->UpgradeToRequests) {
	n = AiPlayer->UpgradeToRequestsCount;
	AiPlayer->UpgradeToRequests = realloc(AiPlayer->UpgradeToRequests,
	    (n + 1) * sizeof(*AiPlayer->UpgradeToRequests));
    } else {
	AiPlayer->UpgradeToRequests = malloc(sizeof(*AiPlayer->UpgradeToRequests));
	n = 0;
    }
    AiPlayer->UpgradeToRequests[n] = type;
    AiPlayer->UpgradeToRequestsCount = n + 1;
}

/**
**	Append unit-type to request table.
**
**	@param upgrade	Upgrade to be appended.
*/
local void InsertResearchRequests(Upgrade* upgrade)
{
    int n;

    if (AiPlayer->ResearchRequests) {
	n = AiPlayer->ResearchRequestsCount;
	AiPlayer->ResearchRequests = realloc(AiPlayer->ResearchRequests,
	    (n + 1) * sizeof(*AiPlayer->ResearchRequests));
    } else {
	AiPlayer->ResearchRequests = malloc(sizeof(*AiPlayer->ResearchRequests));
	n = 0;
    }
    AiPlayer->ResearchRequests[n] = upgrade;
    AiPlayer->ResearchRequestsCount = n + 1;
}

//----------------------------------------------------------------------------

/**
**	Get the race of the current AI player.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiGetRace(void)
{
    return gh_symbol2scm(AiPlayer->Player->RaceName);
}
#elif defined(USE_LUA)
local int CclAiGetRace(lua_State* l)
{
    if (lua_gettop(l) != 0) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    lua_pushstring(l, AiPlayer->Player->RaceName);
    return 1;
}
#endif

/**
**	Get the number of cycles to sleep.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiGetSleepCycles(void)
{
    return gh_int2scm(AiSleepCycles);
}
#elif defined(USE_LUA)
local int CclAiGetSleepCycles(lua_State* l)
{
    if (lua_gettop(l) != 0) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    lua_pushnumber(l, AiSleepCycles);
    return 1;
}
#endif

//----------------------------------------------------------------------------

/**
**	Set debuging flag of AI script.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiDebug(SCM flag)
{
    if (gh_eq_p(flag, SCM_BOOL_F)) {
	AiPlayer->ScriptDebug = 0;
    } else {
	AiPlayer->ScriptDebug = 1;
    }
    return SCM_BOOL_F;
}
#elif defined(USE_LUA)
local int CclAiDebug(lua_State* l)
{
    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    if (!LuaToBoolean(l, 1)) {
	AiPlayer->ScriptDebug = 0;
    } else {
	AiPlayer->ScriptDebug = 1;
    }
    lua_pushboolean(l, 0);
    return 1;
}
#endif

/**
**	Activate AI debugging for the given player(s)
**	Player can be
**		a number for a specific player
**		"self" for current human player (ai me)
**	 	"none" to disable
**
**	@param list the list of player to activate
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiDebugPlayer(SCM list)
{
    SCM item;
    int playerid;

    while (!gh_null_p(list)) {
	item = gh_car(list);
	list = gh_cdr(list);

	if (gh_eq_p(item, gh_symbol2scm("none"))) {
	    for (playerid = 0; playerid < NumPlayers; ++playerid) {
		if (!Players[playerid].AiEnabled || !Players[playerid].Ai) {
		    continue;
		}
		((PlayerAi*)Players[playerid].Ai)->ScriptDebug = 0;
	    }
	} else {
	    if (gh_eq_p(item, gh_symbol2scm("self"))) {
		if (!ThisPlayer) {
		    continue;
		}
		playerid = ThisPlayer->Player;
	    } else {
		playerid = gh_scm2int(item);
	    }

	    if (!Players[playerid].AiEnabled || !Players[playerid].Ai) {
		continue;
	    }
	    ((PlayerAi*)Players[playerid].Ai)->ScriptDebug = 1;
	}
    }
    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclAiDebugPlayer(lua_State* l)
{
    const char* item;
    int playerid;
    int args;
    int j;

    args = lua_gettop(l);
    for (j = 0; j < args; ++j) {
	if (lua_isstring(l, j + 1)) {
	    item = LuaToString(l, j + 1);
	} else {
	    item = NULL;
	}

	if (item && !strcmp(item, "none")) {
	    for (playerid = 0; playerid < NumPlayers; ++playerid) {
		if (!Players[playerid].AiEnabled || !Players[playerid].Ai) {
		    continue;
		}
		((PlayerAi*)Players[playerid].Ai)->ScriptDebug = 0;
	    }
	} else {
	    if (item && !strcmp(item, "self")) {
		if (!ThisPlayer) {
		    continue;
		}
		playerid = ThisPlayer->Player;
	    } else {
		playerid = LuaToNumber(l, j + 1);
	    }

	    if (!Players[playerid].AiEnabled || !Players[playerid].Ai) {
		continue;
	    }
	    ((PlayerAi*)Players[playerid].Ai)->ScriptDebug = 1;
	}
    }
    return 0;
}
#endif

/**
**	Need an unit.
**
**	@param value	Unit-type as string/symbol/object.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiNeed(SCM value)
{
    InsertUnitTypeRequests(CclGetUnitType(value), 1);

    return SCM_BOOL_F;
}
#elif defined(USE_LUA)
local int CclAiNeed(lua_State* l)
{
    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    InsertUnitTypeRequests(CclGetUnitType(l), 1);

    lua_pushboolean(l, 0);
    return 1;
}
#endif

/**
**	Set the number of units.
**
**	@param value	Unit-type as string/symbol/object.
**	@param count	Number of unit-types requested.
**
**	@todo FIXME:	count==0 should remove the request.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiSet(SCM value, SCM count)
{
    AiUnitTypeTable* autt;
    UnitType* type;

    type = CclGetUnitType(value);
    if ((autt = FindInUnitTypeRequests(type))) {
	autt->Count = gh_scm2int(count);
	// FIXME: 0 should remove it.
    } else {
	InsertUnitTypeRequests(type, gh_scm2int(count));
    }

    return SCM_BOOL_F;
}
#elif defined(USE_LUA)
local int CclAiSet(lua_State* l)
{
    AiUnitTypeTable* autt;
    UnitType* type;

    if (lua_gettop(l) != 2) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    lua_pushvalue(l, 1);
    type = CclGetUnitType(l);
    lua_pop(l, 1);
    if ((autt = FindInUnitTypeRequests(type))) {
	autt->Count = LuaToNumber(l, 2);
	// FIXME: 0 should remove it.
    } else {
	InsertUnitTypeRequests(type, LuaToNumber(l, 2));
    }

    lua_pushboolean(l, 0);
    return 1;
}
#endif

/**
**	Wait for an unit.
**
**	@param value	Unit-type as string/symbol/object.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiWait(SCM value)
{
    const AiUnitTypeTable* autt;
    const UnitType* type;
    const int* unit_types_count;
    int j;
    int n;

    type = CclGetUnitType(value);
    unit_types_count = AiPlayer->Player->UnitTypesCount;
    if (!(autt = FindInUnitTypeRequests(type))) {
	//
	//      Look if we have this unit-type.
	//
	if (unit_types_count[type->Type]) {
	    return SCM_BOOL_F;
	}

	//
	//      Look if we have equivalent unit-types.
	//
	if (type->Type < AiHelpers.EquivCount && AiHelpers.Equiv[type->Type]) {
	    DebugLevel3Fn("Equivalence for %s\n" _C_ type->Ident);
	    for (j = 0; j < AiHelpers.Equiv[type->Type]->Count; ++j) {
		if (unit_types_count[AiHelpers.Equiv[type->Type]->Table[j]->Type]) {
		    return SCM_BOOL_F;
		}
	    }
	}
	//
	//      Look if we have an upgrade-to request.
	//
	if (FindInUpgradeToRequests(type)) {
	    return SCM_BOOL_T;
	}
	DebugLevel0Fn("Broken? waiting on %s which wasn't requested.\n" _C_ type->Ident);
	return SCM_BOOL_F;
    }
    //
    //  Add equivalent units
    //
    n = unit_types_count[type->Type];
    if (type->Type < AiHelpers.EquivCount && AiHelpers.Equiv[type->Type]) {
	for (j = 0; j < AiHelpers.Equiv[type->Type]->Count; ++j) {
	    n += unit_types_count[AiHelpers.Equiv[type->Type]->Table[j]->Type];
	}
    }
    // units available?
    DebugLevel3Fn("%d,%d\n" _C_ n _C_ autt->Count);

    if (n >= autt->Count) {
	return SCM_BOOL_F;
    }

    return SCM_BOOL_T;
}
#elif defined(USE_LUA)
local int CclAiWait(lua_State* l)
{
    const AiUnitTypeTable* autt;
    const UnitType* type;
    const int* unit_types_count;
    int j;
    int n;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    type = CclGetUnitType(l);
    unit_types_count = AiPlayer->Player->UnitTypesCount;
    if (!(autt = FindInUnitTypeRequests(type))) {
	//
	//      Look if we have this unit-type.
	//
	if (unit_types_count[type->Type]) {
	    lua_pushboolean(l, 0);
	    return 1;
	}

	//
	//      Look if we have equivalent unit-types.
	//
	if (type->Type < AiHelpers.EquivCount && AiHelpers.Equiv[type->Type]) {
	    DebugLevel3Fn("Equivalence for %s\n" _C_ type->Ident);
	    for (j = 0; j < AiHelpers.Equiv[type->Type]->Count; ++j) {
		if (unit_types_count[AiHelpers.Equiv[type->Type]->Table[j]->Type]) {
		    lua_pushboolean(l, 0);
		    return 1;
		}
	    }
	}
	//
	//      Look if we have an upgrade-to request.
	//
	if (FindInUpgradeToRequests(type)) {
	    lua_pushboolean(l, 1);
	    return 1;
	}
	DebugLevel0Fn("Broken? waiting on %s which wasn't requested.\n" _C_ type->Ident);
	lua_pushboolean(l, 0);
	return 1;
    }
    //
    //  Add equivalent units
    //
    n = unit_types_count[type->Type];
    if (type->Type < AiHelpers.EquivCount && AiHelpers.Equiv[type->Type]) {
	for (j = 0; j < AiHelpers.Equiv[type->Type]->Count; ++j) {
	    n += unit_types_count[AiHelpers.Equiv[type->Type]->Table[j]->Type];
	}
    }
    // units available?
    DebugLevel3Fn("%d,%d\n" _C_ n _C_ autt->Count);

    if (n >= autt->Count) {
	lua_pushboolean(l, 0);
	return 1;
    }

    lua_pushboolean(l, 1);
    return 1;
}
#endif

/**
**	Give the number of the script specific force.
**
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiOwnForce(void)
{
    return gh_int2scm(AiScript->ownForce);
}
#elif defined(USE_LUA)
local int CclAiOwnForce(lua_State* l)
{
    if (lua_gettop(l) != 0) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    lua_pushnumber(l, AiScript->ownForce);
    return 1;
}
#endif

/**
**	Free a force ( requirements and current units )
**
**	@param s_force	Force to free.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiClearForce(SCM s_force)
{
    int force;

    force = gh_scm2int(s_force);
    if (force < 0 || force >= AI_MAX_FORCES) {
	errl("Force out of range", s_force);
    }

    AiEraseForce(force);
    return SCM_BOOL_F;

}
#elif defined(USE_LUA)
local int CclAiClearForce(lua_State* l)
{
    int force;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    force = LuaToNumber(l, 1);
    if (force < 0 || force >= AI_MAX_FORCES) {
	lua_pushfstring(l, "Force out of range: %d", force);
	lua_error(l);
    }

    AiEraseForce(force);
    lua_pushboolean(l, 0);
    return 1;

}
#endif

#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiGetForce(SCM list)
{
    int force;
    SCM rslt;
    AiUnitType* aiut;

    force = gh_scm2int(list);
    if (force < 0 || force >= AI_MAX_FORCES) {
	errl("Force out of range", list);
    }
    rslt = SCM_UNSPECIFIED;
    aiut = AiPlayer->Force[force].UnitTypes;
    while (aiut) {
	rslt =
	    cons(gh_symbol2scm(aiut->Type->Ident), cons(gh_int2scm(aiut->Want), rslt));
	aiut = aiut->Next;
    }
    CclOutput(rslt);
    return rslt;
}
#elif defined(USE_LUA)
local int CclAiGetForce(lua_State* l)
{
    int force;
    AiUnitType* aiut;
    int i;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    force = LuaToNumber(l, 1);
    if (force < 0 || force >= AI_MAX_FORCES) {
	lua_pushfstring(l, "Force out of range: %d", force);
	lua_error(l);
    }
    lua_newtable(l);
    i = 0;
    aiut = AiPlayer->Force[force].UnitTypes;
    while (aiut) {
	lua_pushstring(l, aiut->Type->Ident);
	lua_rawseti(l, -1, i + 1);
	++i;
	lua_pushnumber(l, aiut->Want);
	lua_rawseti(l, -1, i + 1);
	++i;
	aiut = aiut->Next;
    }
//    CclOutput(rslt);
    return 1;
}
#endif

/**
**	Return true if a force has no unit, fase otherwise.
**
**
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiForceEmpty(SCM list)
{
    int force;

    force = gh_scm2int(list);
    if (force < 0 || force >= AI_MAX_FORCES) {
	errl("Force out of range", list);
    }

    if (AiPlayer->Force[force].Units) {
	return SCM_BOOL_F;
    } else {
	return SCM_BOOL_T;
    }
}
#elif defined(USE_LUA)
local int CclAiForceEmpty(lua_State* l)
{
    int force;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    force = LuaToNumber(l, 1);
    if (force < 0 || force >= AI_MAX_FORCES) {
	lua_pushfstring(l, "Force out of range: %d", force);
	lua_error(l);
    }

    if (AiPlayer->Force[force].Units) {
	lua_pushboolean(l, 0);
	return 1;
    } else {
	lua_pushboolean(l, 1);
	return 1;
    }
}
#endif

#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiAdHocForce(SCM requirement, SCM scm_unittypes)
{
    int want[3];
    int* unittypes;
    int unittypecount;
    UnitType* unittype;
    char* str;
    int i;
    int rslt;

    for (i = 0; i < 3; ++i) {
	want[i] = gh_scm2int(gh_car(requirement));
	requirement = gh_cdr(requirement);
    }

    unittypecount = gh_length(scm_unittypes);
    if (unittypecount) {
	unittypes = (int*)malloc(sizeof(int) * unittypecount);

	for (i = 0; i < unittypecount; ++i) {
	    str = gh_scm2newstr(gh_car(scm_unittypes), NULL);
	    scm_unittypes = gh_cdr(scm_unittypes);

	    unittype = UnitTypeByIdent(str);
	    if (!unittype) {
		fprintf(stderr, "unknown unittype %s\n", str);
		free(str);
		--i;
		--unittypecount;
		continue;
	    }
	    free(str);

	    unittypes[i] = unittype->Type;
	}
    } else {
	unittypes = 0;
    }
    rslt = AiCreateSpecificForce(want, unittypes, unittypecount);
    if (unittypes) {
	free(unittypes);
    }

    if (rslt != -1) {
	return SCM_BOOL_T;
    } else {
	return SCM_BOOL_F;
    }
}
#elif defined(USE_LUA)
local int CclAiAdHocForce(lua_State* l)
{
    int want[3];
    int* unittypes;
    int unittypecount;
    UnitType* unittype;
    const char* str;
    int i;
    int rslt;

    if (lua_gettop(l) != 2 || !lua_istable(l, 1) || luaL_getn(l, 1) != 3) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    for (i = 0; i < 3; ++i) {
	lua_rawgeti(l, 1, i + 1);
	want[i] = LuaToNumber(l, -1);
	lua_pop(l, 1);
    }

    unittypecount = luaL_getn(l, 2);
    if (unittypecount) {
	unittypes = (int*)malloc(sizeof(int) * unittypecount);

	for (i = 0; i < unittypecount; ++i) {
	    lua_rawgeti(l, 2, i + 1);
	    str = LuaToString(l, -1);
	    lua_pop(l, 1);

	    unittype = UnitTypeByIdent(str);
	    if (!unittype) {
		lua_pushfstring(l, "unknown unittype: %s", str);
		lua_error(l);
	    }

	    unittypes[i] = unittype->Type;
	}
    } else {
	unittypes = 0;
    }
    rslt = AiCreateSpecificForce(want, unittypes, unittypecount);
    if (unittypes) {
	free(unittypes);
    }

    if (rslt != -1) {
	lua_pushboolean(l, 1);
	return 1;
    } else {
	lua_pushboolean(l, 0);
	return 1;
    }
}
#endif

#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiForceActive(SCM list)
{
    int force;
    AiUnit* unit;

    force = gh_scm2int(list);
    if (force < 0 || force >= AI_MAX_FORCES) {
	errl("Force out of range", list);
    }

    unit = AiPlayer->Force[force].Units;
    while (unit) {
	if (!UnitIdle(unit->Unit)) {
	    return SCM_BOOL_T;
	}
	unit = unit->Next;
    }
    return SCM_BOOL_F;
}
#elif defined(USE_LUA)
local int CclAiForceActive(lua_State* l)
{
    int force;
    AiUnit* unit;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    force = LuaToNumber(l, 1);
    if (force < 0 || force >= AI_MAX_FORCES) {
	lua_pushfstring(l, "Force out of range: %d", force);
	lua_error(l);
    }

    unit = AiPlayer->Force[force].Units;
    while (unit) {
	if (!UnitIdle(unit->Unit)) {
	    lua_pushboolean(l, 1);
	    return 1;
	}
	unit = unit->Next;
    }
    lua_pushboolean(l, 0);
    return 1;
}
#endif

/**
**	Define a force, a groups of units.
**	If force already exists, list is interpreted as a minimum...
**
**	@param list	Pairs of unit-types and counts.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiForce(SCM list)
{
    AiUnitType** prev;
    AiUnitType* aiut;
    UnitType* type;
    int count;
    int force;

    force = gh_scm2int(gh_car(list));
    if (force < 0 || force >= AI_MAX_FORCES) {
	errl("Force out of range", gh_car(list));
    }
    list = gh_cdr(list);

    while (!gh_null_p(list)) {
	type = CclGetUnitType(gh_car(list));
	list = gh_cdr(list);
	count = gh_scm2int(gh_car(list));
	list = gh_cdr(list);

	if (!type) {			// bulletproof
	    continue;
	}
	if (!count) {			// Don't care
	    continue;
	}

    	// Use the equivalent unittype.
    	type = UnitTypes[UnitTypeEquivs[type->Type]];

	//
	//      Look if already in force.
	//
	for (prev = &AiPlayer->Force[force].UnitTypes; (aiut = *prev);
		prev = &aiut->Next) {
	    if (UnitTypeEquivs[aiut->Type->Type] == type->Type) {	// found
		if (aiut->Want < count) {
		    aiut->Want = count;
		}

		if (!aiut->Want) {
		    *prev = aiut->Next;
		    free(aiut);
		}
		break;
	    }
	}

	//
	//      New type append it.
	//
	if (!aiut) {
	    *prev = aiut = malloc(sizeof(*aiut));
	    aiut->Next = NULL;
	    aiut->Want = count;
	    aiut->Type = type;
	}
    }

    AiAssignFreeUnitsToForce();

    return SCM_BOOL_F;
}
#elif defined(USE_LUA)
local int CclAiForce(lua_State* l)
{
    AiUnitType** prev;
    AiUnitType* aiut;
    UnitType* type;
    int count;
    int force;
    int args;
    int j;

    args = lua_gettop(l);
    j = 0;
    force = LuaToNumber(l, j + 1);
    ++j;
    if (force < 0 || force >= AI_MAX_FORCES) {
	lua_pushfstring(l, "Force out of range: %d", force);
	lua_error(l);
    }

    for (; j < args; ++j) {
	lua_pushvalue(l, j + 1);
	type = CclGetUnitType(l);
	lua_pop(l, 1);
	++j;
	count = LuaToNumber(l, j + 1);

	if (!type) {			// bulletproof
	    continue;
	}
	if (!count) {			// Don't care
	    continue;
	}

    	// Use the equivalent unittype.
    	type = UnitTypes[UnitTypeEquivs[type->Type]];

	//
	//      Look if already in force.
	//
	for (prev = &AiPlayer->Force[force].UnitTypes; (aiut = *prev);
		prev = &aiut->Next) {
	    if (UnitTypeEquivs[aiut->Type->Type] == type->Type) {	// found
		if (aiut->Want < count) {
		    aiut->Want = count;
		}

		if (!aiut->Want) {
		    *prev = aiut->Next;
		    free(aiut);
		}
		break;
	    }
	}

	//
	//      New type append it.
	//
	if (!aiut) {
	    *prev = aiut = malloc(sizeof(*aiut));
	    aiut->Next = NULL;
	    aiut->Want = count;
	    aiut->Type = type;
	}
    }

    AiAssignFreeUnitsToForce();

    lua_pushboolean(l, 0);
    return 1;
}
#endif

#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiForceTransfer(SCM sourceForce, SCM destForce)
{
    int src;
    int dst;

    src = gh_scm2int(sourceForce);
    dst = gh_scm2int(destForce);
    AiForceTransfert(src, dst);
    return SCM_BOOL_F;
}
#elif defined(USE_LUA)
local int CclAiForceTransfer(lua_State* l)
{
    int src;
    int dst;

    if (lua_gettop(l) != 2) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    src = LuaToNumber(l, 1);
    dst = LuaToNumber(l, 2);
    AiForceTransfert(src, dst);
    lua_pushboolean(l, 0);
    return 1;
}
#endif

/**
**	Wrapper of AiForceComplete.
**	Complete a force with existing units.
**
**	@param destForce	the force to complete
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiForceComplete(SCM destForce)
{
    int dst;

    dst = gh_scm2int(destForce);
    AiForceComplete(dst);
    return SCM_BOOL_F;
}
#elif defined(USE_LUA)
local int CclAiForceComplete(lua_State* l)
{
    int dst;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    dst = LuaToNumber(l, 1);
    AiForceComplete(dst);
    lua_pushboolean(l, 0);
    return 1;
}
#endif

/**
**	Define the role of a force.
**
**	@param value	Force number.
**	@param flag	Which role of the force.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiForceRole(SCM value, SCM flag)
{
    int force;

    force = gh_scm2int(value);
    if (force < 0 || force >= AI_MAX_FORCES) {
	errl("Force out of range", value);
    }
    if (gh_eq_p(flag, gh_symbol2scm("attack"))) {
	AiPlayer->Force[force].Role = AiForceRoleAttack;
    } else if (gh_eq_p(flag, gh_symbol2scm("defend"))) {
	AiPlayer->Force[force].Role = AiForceRoleDefend;
    } else {
	errl("Unknown force role", flag);
    }

    return SCM_BOOL_F;
}
#elif defined(USE_LUA)
local int CclAiForceRole(lua_State* l)
{
    int force;
    const char* flag;

    if (lua_gettop(l) != 2) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    force = LuaToNumber(l, 1);
    if (force < 0 || force >= AI_MAX_FORCES) {
	lua_pushfstring(l, "Force out of range", force);
	lua_error(l);
    }
    flag = LuaToString(l, 2);
    if (!strcmp(flag, "attack")) {
	AiPlayer->Force[force].Role = AiForceRoleAttack;
    } else if (!strcmp(flag, "defend")) {
	AiPlayer->Force[force].Role = AiForceRoleDefend;
    } else {
	lua_pushfstring(l, "Unknown force role", flag);
    }

    lua_pushboolean(l, 0);
    return 1;
}
#endif

/**
**	Return wether a force is defending or not.
**
**	@param	value	the force
**	@return 	true if the force is defending, false otherwise
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiIsForceDefending(SCM value)
{
    int force;

    force = gh_scm2int(value);
    if (force < 0 || force >= AI_MAX_FORCES) {
	errl("Force out of range", value);
    }

    if (AiPlayer->Force[force].Role == AiForceRoleDefend) {
	return SCM_BOOL_T;
    }
    return SCM_BOOL_F;
}
#elif defined(USE_LUA)
local int CclAiIsForceDefending(lua_State* l)
{
    int force;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    force = LuaToNumber(l, 1);
    if (force < 0 || force >= AI_MAX_FORCES) {
	lua_pushfstring(l, "Force out of range: %d", force);
	lua_error(l);
    }

    if (AiPlayer->Force[force].Role == AiForceRoleDefend) {
	lua_pushboolean(l, 1);
	return 1;
    }
    lua_pushboolean(l, 0);
    return 1;
}
#endif

/**
**	Check if the hotspot can be reached.
**	The parameter describe which element we are travelling in.
**
**	@param way	air, ground or sea
**	@return		true if the hotspot is reachable
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiCanReachHotSpot(SCM way)
{
#ifdef MAP_REGIONS
    ZoneSet targets = {0};
    ZoneSet sources = {0};
    ZoneSet transportable = {0};
    int i;
    Unit* unit;
    int transporterplace;
    UnitType* transporter;
    AiUnit* aiunit;

    if ((AiScript->HotSpotX == -1) || (AiScript->HotSpotY == -1) ||
	    (AiScript->HotSpotRay <= 0)) {
	return SCM_BOOL_T;
    }

    ZoneSetClear(&sources);
    transporter = 0;
    transporterplace = 0;
    for (i = 0; i < AiPlayer->Player->TotalNumUnits; ++i) {
	unit = AiPlayer->Player->Units[i];

	if (unit->Orders[0].Action == UnitActionDie) {
	    continue;
	}

	if (unit->Removed) {
	    continue;
	}

	if ((unit->X == -1) || (unit->Y == -1)) {
	    continue;
	}

	if (unit->Type->UnitType == UnitTypeNaval && unit->Type->Transporter) {
	    ZoneSetAddUnitZones(&sources, unit);
	    if ((!transporter) || (transporter->MaxOnBoard < unit->Type->MaxOnBoard)) {
		transporter = unit->Type;
	    }
	    transporterplace += unit->Type->MaxOnBoard;
	}
    }
    ZoneSetClear(&transportable);
    // Add land connected to transporter
    ZoneSetAddConnected(&transportable, &sources);
    // Add water as well
    ZoneSetAddSet(&transportable, &sources);


    aiunit = AiPlayer->Force[AiScript->ownForce].Units;

    while (aiunit) {
	switch(aiunit->Unit->Type->UnitType) {
	    case UnitTypeFly:
	    	break;
	    case UnitTypeNaval:
		if (!PlaceReachable(aiunit->Unit, AiScript->HotSpotX, AiScript->HotSpotY,
			1, 1, 0, aiunit->Unit->Type->_AttackRange)) {
		    return SCM_BOOL_F;
	    	}
		break;
	    case UnitTypeLand:

    		ZoneSetClear(&sources);
		ZoneSetAddUnitZones(&sources,aiunit->Unit);

		ZoneSetClear(&targets);
		ZoneSetAddGoalZones(&targets, aiunit->Unit, AiScript->HotSpotX - 4, AiScript->HotSpotY - 4,
		    9, 9, 0, 0);

		if (!ZoneSetHasIntersect(&targets, &sources) &&
			(!ZoneSetHasIntersect(&targets, &transportable) ||
			    !ZoneSetHasIntersect(&sources, &transportable))) {
		     return SCM_BOOL_F;
		}
		break;
	}
	aiunit = aiunit->Next;
    }

    return SCM_BOOL_T;
#else
    return SCM_BOOL_F;
#endif
}
#elif defined(USE_LUA)
local int CclAiCanReachHotSpot(lua_State* l)
{
#ifdef MAP_REGIONS
    ZoneSet targets = {0};
    ZoneSet sources = {0};
    ZoneSet transportable = {0};
    int i;
    Unit* unit;
    int transporterplace;
    UnitType* transporter;
    AiUnit* aiunit;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    if ((AiScript->HotSpotX == -1) || (AiScript->HotSpotY == -1) ||
	    (AiScript->HotSpotRay <= 0)) {
	lua_pushboolean(l, 1);
	return 1;
    }

    ZoneSetClear(&sources);
    transporter = 0;
    transporterplace = 0;
    for (i = 0; i < AiPlayer->Player->TotalNumUnits; ++i) {
	unit = AiPlayer->Player->Units[i];

	if (unit->Orders[0].Action == UnitActionDie) {
	    continue;
	}

	if (unit->Removed) {
	    continue;
	}

	if ((unit->X == -1) || (unit->Y == -1)) {
	    continue;
	}

	if (unit->Type->UnitType == UnitTypeNaval && unit->Type->Transporter) {
	    ZoneSetAddUnitZones(&sources, unit);
	    if ((!transporter) || (transporter->MaxOnBoard < unit->Type->MaxOnBoard)) {
		transporter = unit->Type;
	    }
	    transporterplace += unit->Type->MaxOnBoard;
	}
    }
    ZoneSetClear(&transportable);
    // Add land connected to transporter
    ZoneSetAddConnected(&transportable, &sources);
    // Add water as well
    ZoneSetAddSet(&transportable, &sources);


    aiunit = AiPlayer->Force[AiScript->ownForce].Units;

    while (aiunit) {
	switch(aiunit->Unit->Type->UnitType) {
	    case UnitTypeFly:
	    	break;
	    case UnitTypeNaval:
		if (!PlaceReachable(aiunit->Unit, AiScript->HotSpotX, AiScript->HotSpotY,
			1, 1, 0, aiunit->Unit->Type->_AttackRange)) {
		    lua_pushboolean(l, 0);
		    return 1;
	    	}
		break;
	    case UnitTypeLand:

    		ZoneSetClear(&sources);
		ZoneSetAddUnitZones(&sources,aiunit->Unit);

		ZoneSetClear(&targets);
		ZoneSetAddGoalZones(&targets, aiunit->Unit, AiScript->HotSpotX - 4,
		    AiScript->HotSpotY - 4, 9, 9, 0, 0);

		if (!ZoneSetHasIntersect(&targets, &sources) &&
			(!ZoneSetHasIntersect(&targets, &transportable) ||
			    !ZoneSetHasIntersect(&sources, &transportable))) {
		    lua_pushboolean(l, 0);
		    return 1;
		}
		break;
	}
	aiunit = aiunit->Next;
    }

    lua_pushboolean(l, 1);
    return 1;
#else
    lua_pushboolean(l, 0);
    return 1;
#endif
}
#endif


/**
**	Check if a force ready.
**
**	@param value	Force number.
**	@return		#t if ready, #f otherwise.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiCheckForce(SCM value)
{
    int force;

    force = gh_scm2int(value);
    if (force < 0 || force >= AI_MAX_FORCES) {
	errl("Force out of range", value);
    }
    if (AiPlayer->Force[force].Completed) {
	return SCM_BOOL_T;
    }
    return SCM_BOOL_F;
}
#elif defined(USE_LUA)
local int CclAiCheckForce(lua_State* l)
{
    int force;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    force = LuaToNumber(l, 1);
    if (force < 0 || force >= AI_MAX_FORCES) {
	lua_pushfstring(l, "Force out of range: %d", force);
    }
    if (AiPlayer->Force[force].Completed) {
	lua_pushboolean(l, 1);
	return 1;
    }
    lua_pushboolean(l, 0);
    return 1;
}
#endif

/**
** Evaluate the ressources needed to complete a force.
**
** @param s_force the force
**
** @return -1 if it is not possible ( upgrade missing )
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiEvaluateForceCost(SCM s_force)
{
    return gh_int2scm(AiEvaluateForceCost(gh_scm2int(s_force), 0));
}
#elif defined(USE_LUA)
local int CclAiEvaluateForceCost(lua_State* l)
{
    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    lua_pushnumber(l, AiEvaluateForceCost(LuaToNumber(l, 1), 0));
    return 1;
}
#endif

// Just hang...
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiIdle(void)
{
    return SCM_BOOL_T;
}
#elif defined(USE_LUA)
local int CclAiIdle(lua_State* l)
{
    if (lua_gettop(l) != 0) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    lua_pushboolean(l, 1);
    return 1;
}
#endif

/**
**	Wait for a force ready.
**
**	@param value	Force number.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiWaitForce(SCM value)
{
    int force;

    force = gh_scm2int(value);
    if (force < 0 || force >= AI_MAX_FORCES) {
	errl("Force out of range", value);
    }
    if (AiPlayer->Force[force].Completed) {
	return SCM_BOOL_F;
    }
    DebugLevel3Fn("Wait force %d\n" _C_ force);
#if 0
    // Debuging
    AiCleanForces();
    DebugCheck(AiPlayer->Force[force].Completed);
#endif

    return SCM_BOOL_T;
}
#elif defined(USE_LUA)
local int CclAiWaitForce(lua_State* l)
{
    int force;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    force = LuaToNumber(l, 1);
    if (force < 0 || force >= AI_MAX_FORCES) {
	lua_pushfstring(l, "Force out of range: %d", force);
	lua_error(l);
    }
    if (AiPlayer->Force[force].Completed) {
	lua_pushboolean(l, 0);
	return 1;
    }
    DebugLevel3Fn("Wait force %d\n" _C_ force);
#if 0
    // Debuging
    AiCleanForces();
    DebugCheck(AiPlayer->Force[force].Completed);
#endif

    lua_pushboolean(l, 1);
    return 1;
}
#endif

/**
**	Attack with force, on the current script hotspot.
**
**	@param value	Force number.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiHotSpotAttackWithForce(SCM value)
{
    int force;

    force = gh_scm2int(value);
    if (force < 0 || force >= AI_MAX_FORCES) {
	errl("Force out of range", value);
    }

    AiAttackWithForceAt(force, AiScript->HotSpotX, AiScript->HotSpotY);

    return SCM_BOOL_F;
}
#elif defined(USE_LUA)
local int CclAiHotSpotAttackWithForce(lua_State* l)
{
    int force;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    force = LuaToNumber(l, 1);
    if (force < 0 || force >= AI_MAX_FORCES) {
	lua_pushfstring(l, "Force out of range: %d", force);
	lua_error(l);
    }

    AiAttackWithForceAt(force, AiScript->HotSpotX, AiScript->HotSpotY);

    lua_pushboolean(l, 0);
    return 1;
}
#endif

/**
**
**
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiGroupForce(SCM value)
{
    int force;

    force = gh_scm2int(value);
    if (force < 0 || force >= AI_MAX_FORCES) {
	errl("Force out of range", value);
    }

    AiGroupForceNear(force, AiScript->HotSpotX, AiScript->HotSpotY);

    return SCM_BOOL_F;
}
#elif defined(USE_LUA)
local int CclAiGroupForce(lua_State* l)
{
    int force;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    force = LuaToNumber(l, 1);
    if (force < 0 || force >= AI_MAX_FORCES) {
	lua_pushfstring(l, "Force out of range: %d", force);
	lua_error(l);
    }

    AiGroupForceNear(force, AiScript->HotSpotX, AiScript->HotSpotY);

    lua_pushboolean(l, 0);
    return 1;
}
#endif

#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiForceHome(SCM value)
{
    int force;

    force = gh_scm2int(value);
    if (force < 0 || force >= AI_MAX_FORCES) {
	errl("Force out of range", value);
    }

    AiSendForceHome(force);

    return SCM_BOOL_F;
}
#elif defined(USE_LUA)
local int CclAiForceHome(lua_State* l)
{
    int force;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    force = LuaToNumber(l, 1);
    if (force < 0 || force >= AI_MAX_FORCES) {
	lua_pushfstring(l, "Force out of range: %d", force);
	lua_error(l);
    }

    AiSendForceHome(force);

    lua_pushboolean(l, 0);
    return 1;
}
#endif

/**
**	Wait for a force (like CclAiWaitForce), limited in time.
**
**	@param s_force	Force number.
**	@param s_wait	Maximum number of cycles to delay.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiTimedWaitForce(SCM s_force, SCM s_wait)
{
    SCM result;

    // return true after n call or when force is ready
    if (AiScript->SleepCycles) {
	if (AiScript->SleepCycles < GameCycle) {
	    AiScript->SleepCycles = 0;
	    return SCM_BOOL_F;
	}
    } else {
	AiScript->SleepCycles = GameCycle + gh_scm2int(s_wait);
    }
    result = CclAiWaitForce(s_force);
    if (result == SCM_BOOL_F) {
	AiScript->SleepCycles = 0;
    }
    return result;
}
#elif defined(USE_LUA)
local int CclAiTimedWaitForce(lua_State* l)
{
//    SCM result;

    if (lua_gettop(l) != 2) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    // return true after n call or when force is ready
    if (AiScript->SleepCycles) {
	if (AiScript->SleepCycles < GameCycle) {
	    AiScript->SleepCycles = 0;
	    lua_pushboolean(l, 0);
	    return 1;
	}
    } else {
	AiScript->SleepCycles = GameCycle + LuaToNumber(l, 2);
    }
//    result = CclAiWaitForce(s_force);
//    if (result == SCM_BOOL_F) {
//	AiScript->SleepCycles = 0;
//    }
    return 1;
}
#endif

/**
**	Sleep n cycles.
**
**	@param value	Number of cycles to delay.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiSleep(SCM value)
{
    int i;

    DebugLevel3Fn("%lu %d\n" _C_ GameCycle _C_ AiPlayer->SleepCycles);
    if (AiScript->SleepCycles) {
	if (AiScript->SleepCycles < GameCycle) {
	    AiScript->SleepCycles = 0;
	    return SCM_BOOL_F;
	}
    } else {
	i = gh_scm2int(value);
	AiScript->SleepCycles = GameCycle + i;
    }

    return SCM_BOOL_T;
}
#elif defined(USE_LUA)
local int CclAiSleep(lua_State* l)
{
    int i;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    DebugLevel3Fn("%lu %d\n" _C_ GameCycle _C_ AiPlayer->SleepCycles);
    if (AiScript->SleepCycles) {
	if (AiScript->SleepCycles < GameCycle) {
	    AiScript->SleepCycles = 0;
	    lua_pushboolean(l, 0);
	    return 1;
	}
    } else {
	i = LuaToNumber(l, 1);
	AiScript->SleepCycles = GameCycle + i;
    }

    lua_pushboolean(l, 1);
    return 1;
}
#endif

/**
**	Research an upgrade.
**
**	@param value	Upgrade as string/symbol/object.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiResearch(SCM value)
{
    const char* str;
    Upgrade* upgrade;

    // Be kind allow also strings or symbols
    if ((str = try_get_c_string(value))) {
	upgrade = UpgradeByIdent(str);
    } else {
	errl("Upgrade needed", value);
	return SCM_BOOL_F;
    }

    InsertResearchRequests(upgrade);

    return SCM_BOOL_F;
}
#elif defined(USE_LUA)
local int CclAiResearch(lua_State* l)
{
    const char* str;
    Upgrade* upgrade;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    if ((str = LuaToString(l, 1))) {
	upgrade = UpgradeByIdent(str);
    } else {
	lua_pushfstring(l, "Upgrade needed");
	lua_error(l);
    }

    InsertResearchRequests(upgrade);

    lua_pushboolean(l, 0);
    return 1;
}
#endif

/**
**	Upgrade an unit to an new unit-type.
**
**	@param value	Unit-type as string/symbol/object.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiUpgradeTo(SCM value)
{
    UnitType* type;

    type = CclGetUnitType(value);
    InsertUpgradeToRequests(type);

    return SCM_BOOL_F;
}
#elif defined(USE_LUA)
local int CclAiUpgradeTo(lua_State* l)
{
    UnitType* type;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    type = CclGetUnitType(l);
    InsertUpgradeToRequests(type);

    lua_pushboolean(l, 0);
    return 1;
}
#endif

#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiSetHotSpotRay(SCM value)
{
    return SCM_BOOL_F;
}
#elif defined(USE_LUA)
local int CclAiSetHotSpotRay(lua_State* l)
{
    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    lua_pushboolean(l, 0);
    return 1;
}
#endif

#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiComputeGauges(void)
{
    AiComputeCurrentScriptGauges();
    return SCM_BOOL_F;
}
#elif defined(USE_LUA)
local int CclAiComputeGauges(lua_State* l)
{
    if (lua_gettop(l) != 0) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    AiComputeCurrentScriptGauges();
    lua_pushboolean(l, 0);
    return 1;
}
#endif

#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiDebugGauges(void)
{
    AiDebugGauges();
    return SCM_BOOL_F;
}
#elif defined(USE_LUA)
local int CclAiDebugGauges(lua_State* l)
{
    if (lua_gettop(l) != 0) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    AiDebugGauges();
    lua_pushboolean(l, 0);
    return 1;
}
#endif

#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiGetGauge(SCM value)
{
    int gauge;

    if (gh_exact_p(value)) {
	gauge = gh_scm2int(value);
    } else {
	gauge = AiFindGaugeId(value);
	if (gauge == -1) {
	    errl("invalid gauge name", value);
	}
    }
    return gh_int2scm(AiGetGaugeValue(gauge));
}
#elif defined(USE_LUA)
local int CclAiGetGauge(lua_State* l)
{
    int gauge;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    if (lua_isnumber(l, 1)) {
	gauge = LuaToNumber(l, 1);
    } else {
	gauge = AiFindGaugeId(l);
	if (gauge == -1) {
	    lua_pushfstring(l, "invalid gauge name: %s", LuaToString(l, 1));
	    lua_error(l);
	}
    }
    lua_pushnumber(l, AiGetGaugeValue(gauge));
    return 1;
}
#endif

#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiGetUnitTypeForce(SCM value)
{
    UnitType* unitType;

    unitType = CclGetUnitType(value);

    return gh_int2scm(AiUnittypeForce(unitType));
}
#elif defined(USE_LUA)
local int CclAiGetUnitTypeForce(lua_State* l)
{
    UnitType* unitType;

    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    unitType = CclGetUnitType(l);

    lua_pushnumber(l, AiUnittypeForce(unitType));
    return 1;
}
#endif

/**
**	Simple restart the AI.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiRestart(void)
{
    int i;

    CclGcProtectedAssign(&AiPlayer->Scripts[0].Script, AiPlayer->AiType->Script);
    AiPlayer->Scripts[0].SleepCycles = 0;
    snprintf(AiPlayer->Scripts[0].ident, 10, "Main AI");
    for (i = 1; i < AI_MAX_RUNNING_SCRIPTS; ++i) {
	CclGcProtectedAssign(&AiPlayer->Scripts[i].Script, NIL);
	AiPlayer->Scripts[i].SleepCycles = 0;
	snprintf(AiPlayer->Scripts[i].ident, 10, "Empty");
    }
    return SCM_BOOL_T;
}
#elif defined(USE_LUA)
#endif

/**
**	Start an AI script at the given id.
**
**      @param script		The Id of the script to run ( main, defend, ... )
**	@param list		The script execution
**	@param hotSpotX		position of the hotspot (X)
**	@param hotSpotY		position of the hotspot (Y)
**	@param hotSpotRay	ray of the hotspot
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
global void AiRunScript(int script, SCM list, int hotSpotX, int hotSpotY, int hotSpotRay)
{
    CclGcProtectedAssign(&AiPlayer->Scripts[script].Script, list);
    AiPlayer->Scripts[script].SleepCycles = 0;
    snprintf(AiPlayer->Scripts[script].ident, 10, "AiRunScript");
    AiPlayer->Scripts[script].HotSpotX = hotSpotX;
    AiPlayer->Scripts[script].HotSpotY = hotSpotY;
    AiPlayer->Scripts[script].HotSpotRay = hotSpotRay;
    // FIXME : Compute gauges here ?
}
#elif defined(USE_LUA)
#endif

/**
**	Change the current script
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiSwitchTo(SCM value)
{
    CclGcProtectedAssign(&AiScript->Script, value);
    AiScript->SleepCycles = 0;
    return SCM_BOOL_T;
}
#elif defined(USE_LUA)
#endif

/**
**	Execute new main script
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiScript(SCM value)
{
    CclGcProtectedAssign(&AiPlayer->Scripts[0].Script, value);
    AiPlayer->Scripts[0].SleepCycles = 0;
    snprintf(AiPlayer->Scripts[0].ident, 10, "MainScript");
    return SCM_BOOL_T;
}
#elif defined(USE_LUA)
#endif

/**
**	Return the player of the running AI.
**
**	@return		Player number of the AI.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiPlayer(void)
{
    return gh_int2scm(AiPlayer->Player->Player);
}
#elif defined(USE_LUA)
local int CclAiPlayer(lua_State* l)
{
    if (lua_gettop(l) != 0) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    lua_pushnumber(l, AiPlayer->Player->Player);
    return 1;
}
#endif

/**
**	Set AI player resource reserve.
**
**	@param vec	Resources vector
**	@return		Old resource vector
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiSetReserve(SCM vec)
{
    int i;
    SCM old;

    old = cons_array(gh_int2scm(MaxCosts), 0);
    for (i = 0; i < MaxCosts; ++i) {
	aset1(old, gh_int2scm(i), gh_int2scm(AiPlayer->Reserve[i]));
    }
    for (i = 0; i < MaxCosts; ++i) {
	AiPlayer->Reserve[i] = gh_scm2int(gh_vector_ref(vec, gh_int2scm(i)));
    }
    return old;
}
#elif defined(USE_LUA)
local int CclAiSetReserve(lua_State* l)
{
    int i;

    if (lua_gettop(l) != 1 || !lua_istable(l, 1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    lua_newtable(l);
    for (i = 0; i < MaxCosts; ++i) {
	lua_pushnumber(l, AiPlayer->Reserve[i]);
	lua_rawseti(l, -1, i + 1);
    }
    for (i = 0; i < MaxCosts; ++i) {
	lua_rawgeti(l, 1, i + 1);
	AiPlayer->Reserve[i] = LuaToNumber(l, -1);
	lua_pop(l, 1);
    }
    return 1;
}
#endif

/**
**	Set AI player resource collect percent.
**
**	@param vec	Resources vector
**	@return		Old resource vector
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiSetCollect(SCM vec)
{
    int i;
    SCM old;

    old = cons_array(gh_int2scm(MaxCosts), 0);
    for (i = 0; i < MaxCosts; ++i) {
	aset1(old, gh_int2scm(i), gh_int2scm(AiPlayer->Collect[i]));
    }
    for (i = 0; i < MaxCosts; ++i) {
	AiPlayer->Collect[i] = gh_scm2int(gh_vector_ref(vec, gh_int2scm(i)));
    }
    return old;
}
#elif defined(USE_LUA)
local int CclAiSetCollect(lua_State* l)
{
    int i;

    if (lua_gettop(l) != 1 || !lua_istable(l, 1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    lua_newtable(l);
    for (i = 0; i < MaxCosts; ++i) {
	lua_pushnumber(l, AiPlayer->Collect[i]);
	lua_rawseti(l, -1, i + 1);
    }
    for (i = 0; i < MaxCosts; ++i) {
	lua_rawgeti(l, 1, i + 1);
	AiPlayer->Collect[i] = LuaToNumber(l, -1);
	lua_pop(l, 1);
    }
    return 1;
}
#endif

/**
**	Set the autoattack flag of the current AiPlayer
**
**	@param val	new value of the autoattack flag
**	@return		SCM_BOOL_F
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiSetAutoAttack(SCM val)
{
    AiPlayer->AutoAttack = gh_scm2bool(val);
    return SCM_BOOL_F;
}
#elif defined(USE_LUA)
local int CclAiSetAutoAttack(lua_State* l)
{
    if (lua_gettop(l) != 1) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    AiPlayer->AutoAttack = LuaToBoolean(l, 1);
    lua_pushboolean(l, 0);
    return 1;
}
#endif

/**
**	Dump some AI debug informations.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclAiDump(void)
{
    int i;
    int n;
    const AiUnitType* aut;
    const AiBuildQueue* queue;

    //
    //  Script
    //
    printf("------\n");
    for (i = 0; i < MaxCosts; ++i) {
	printf("%s(%4d) ", DefaultResourceNames[i], AiPlayer->Player->Resources[i]);
    }
    printf("\n");
    printf("%d:", AiPlayer->Player->Player);
    for (i = 0; i < AI_MAX_RUNNING_SCRIPTS; ++i) {
	gh_display(gh_car(AiPlayer->Scripts[i].Script));
    }
    gh_newline();

    //
    //  Requests
    //
    n = AiPlayer->UnitTypeRequestsCount;
    printf("UnitTypeRequests(%d):\n", n);
    for (i = 0; i < n; ++i) {
	printf("%s ", AiPlayer->UnitTypeRequests[i].Table[0]->Ident);
    }
    printf("\n");
    n = AiPlayer->UpgradeToRequestsCount;
    printf("UpgradeToRequests(%d):\n", n);
    for (i = 0; i < n; ++i) {
	printf("%s ", AiPlayer->UpgradeToRequests[i]->Ident);
    }
    printf("\n");
    n = AiPlayer->ResearchRequestsCount;
    printf("ResearchRequests(%d):\n", n);
    for (i = 0; i < n; ++i) {
	printf("%s ", AiPlayer->ResearchRequests[i]->Ident);
    }
    printf("\n");

    //
    //  Building queue
    //
    printf("Building queue:\n");
    for (queue = AiPlayer->UnitTypeBuilded; queue; queue = queue->Next) {
	printf("%s(%d/%d) ", queue->Type->Ident, queue->Made, queue->Want);
    }
    printf("\n");

    //
    //  PrintForce
    //
    for (i = 0; i < AI_MAX_FORCES; ++i) {
	printf("Force(%d%s%s):\n", i,
	    AiPlayer->Force[i].Completed ? ",complete" : ",recruit",
	    AiPlayer->Force[i].Attacking ? ",attack" : "");
	for (aut = AiPlayer->Force[i].UnitTypes; aut; aut = aut->Next) {
	    printf("%s(%d) ", aut->Type->Ident, aut->Want);
	}
	printf("\n");
    }

    return SCM_BOOL_F;
}
#elif defined(USE_LUA)
local int CclAiDump(lua_State* l)
{
    int i;
    int n;
    const AiUnitType* aut;
    const AiBuildQueue* queue;

    if (lua_gettop(l) != 0) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    //
    //  Script
    //
    printf("------\n");
    for (i = 0; i < MaxCosts; ++i) {
	printf("%s(%4d) ", DefaultResourceNames[i], AiPlayer->Player->Resources[i]);
    }
    printf("\n");
    printf("%d:", AiPlayer->Player->Player);
    for (i = 0; i < AI_MAX_RUNNING_SCRIPTS; ++i) {
//	gh_display(gh_car(AiPlayer->Scripts[i].Script));
    }
//    gh_newline();

    //
    //  Requests
    //
    n = AiPlayer->UnitTypeRequestsCount;
    printf("UnitTypeRequests(%d):\n", n);
    for (i = 0; i < n; ++i) {
	printf("%s ", AiPlayer->UnitTypeRequests[i].Table[0]->Ident);
    }
    printf("\n");
    n = AiPlayer->UpgradeToRequestsCount;
    printf("UpgradeToRequests(%d):\n", n);
    for (i = 0; i < n; ++i) {
	printf("%s ", AiPlayer->UpgradeToRequests[i]->Ident);
    }
    printf("\n");
    n = AiPlayer->ResearchRequestsCount;
    printf("ResearchRequests(%d):\n", n);
    for (i = 0; i < n; ++i) {
	printf("%s ", AiPlayer->ResearchRequests[i]->Ident);
    }
    printf("\n");

    //
    //  Building queue
    //
    printf("Building queue:\n");
    for (queue = AiPlayer->UnitTypeBuilded; queue; queue = queue->Next) {
	printf("%s(%d/%d) ", queue->Type->Ident, queue->Made, queue->Want);
    }
    printf("\n");

    //
    //  PrintForce
    //
    for (i = 0; i < AI_MAX_FORCES; ++i) {
	printf("Force(%d%s%s):\n", i,
	    AiPlayer->Force[i].Completed ? ",complete" : ",recruit",
	    AiPlayer->Force[i].Attacking ? ",attack" : "");
	for (aut = AiPlayer->Force[i].UnitTypes; aut; aut = aut->Next) {
	    printf("%s(%d) ", aut->Type->Ident, aut->Want);
	}
	printf("\n");
    }

    lua_pushboolean(l, 0);
    return 1;
}
#endif

/**
**	Define AI mapping from original number to internal symbol
**
**	@param list	List of all names.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineAiWcNames(SCM list)
{
    int i;
    char** cp;

    if ((cp = AiTypeWcNames)) {		// Free all old names
	while (*cp) {
	    free(*cp++);
	}
	free(AiTypeWcNames);
    }
    //
    //  Get new table.
    //
    i = gh_length(list);
    AiTypeWcNames = cp = malloc((i + 1) * sizeof(char*));
    while (i--) {
	*cp++ = gh_scm2newstr(gh_car(list), NULL);
	list = gh_cdr(list);
    }
    *cp = NULL;

    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDefineAiWcNames(lua_State* l)
{
    int i;
    int j;
    char** cp;

    if ((cp = AiTypeWcNames)) {	// Free all old names
	while (*cp) {
	    free(*cp++);
	}
	free(AiTypeWcNames);
    }

    //
    //	Get new table.
    //
    i = lua_gettop(l);
    AiTypeWcNames = cp = malloc((i + 1) * sizeof(char*));
    if (!cp) {
	fprintf(stderr, "out of memory.\n");
	ExitFatal(-1);
    }

    for (j = 0; j < i; ++j) {
	*cp++ = strdup(LuaToString(l, j + 1));
    }
    *cp = NULL;

    return 0;
}
#endif

/**
**	Get the default resource number
**
**	@param type	The name of the resource to lookup
**
**	@return		The number of the resource in DEFAULT_NAMES
*/
local int DefaultResourceNumber(const char* type)
{
    int i;

    for (i = 0; i < MaxCosts; ++i) {
	if (!strcmp(DefaultResourceNames[i], type)) {
	    return i;
	}
    }
    // Resource not found, should never happen
    DebugCheck(1);
    return -1;
}

/**
**	Define an AI player.
**
**	Format of the list is :
**	(	<player_id>
**		<value> <list>
**		[ <value> <list> [ ... ] ]
**	)
**
**	<value> can be :
**		ai-type	=> the name of the ai
**		script  => ???
**		script-debug => ???
**
**	@param list	List of the AI Player.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineAiPlayer(SCM list)
{
    //SCM value;
    //SCM sublist;
    int i;
    //char* str;
    PlayerAi *ai;

    IOLoadingMode = 1;

    IOPlayerAiFullPtr(gh_car(list), &ai, 0);

    i = ai->Player->Player;
    DebugCheck(i < 0 || i > PlayerMax);
    Players[i].Ai = ai;

    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
#endif

/**
**	Register CCL features for unit-type.
*/
global void AiCclRegister(void)
{
#if defined(USE_GUILE) || defined(USE_SIOD)
    // FIXME: Need to save memory here.
    // Loading all into memory isn't necessary.

    gh_new_procedureN("define-ai-helper", CclDefineAiHelper);
    gh_new_procedureN("define-ai", CclDefineAi);

#ifdef INCOMPLETE_SIOD
    gh_new_procedure2_0("quotient", CclQuotient);
    gh_new_procedure1_0("output", CclOutput);
#endif
    gh_new_procedure2_0("define-ai-action", CclDefineAiAction);

    gh_new_procedure0_0("ai:get-race", CclAiGetRace);
    gh_new_procedure0_0("ai:get-sleep-cycles", CclAiGetSleepCycles);

    gh_new_procedure1_0("ai:debug", CclAiDebug);
    gh_new_procedureN("ai:debug-player", CclAiDebugPlayer);
    gh_new_procedure1_0("ai:need", CclAiNeed);
    gh_new_procedure2_0("ai:set", CclAiSet);
    gh_new_procedure1_0("ai:wait", CclAiWait);

    gh_new_procedure0_0("ai:own-force", CclAiOwnForce);

    gh_new_procedure1_0("ai:get-force", CclAiGetForce);


    gh_new_procedureN("ai:force", CclAiForce);

    gh_new_procedure2_0("ai:adhoc-force", CclAiAdHocForce);

    gh_new_procedure1_0("ai:force-empty", CclAiForceEmpty);
    gh_new_procedure1_0("ai:force-active", CclAiForceActive);
    gh_new_procedure1_0("ai:force-list", CclAiForce);
    gh_new_procedure2_0("ai:force-transfer", CclAiForceTransfer);
    gh_new_procedure1_0("ai:force-complete", CclAiForceComplete);
    gh_new_procedure2_0("ai:force-role", CclAiForceRole);
    gh_new_procedure1_0("ai:is-force-defending", CclAiIsForceDefending);
    gh_new_procedure1_0("ai:check-force", CclAiCheckForce);
    gh_new_procedure1_0("ai:group-force", CclAiGroupForce);
    gh_new_procedure1_0("ai:wait-force", CclAiWaitForce);
    gh_new_procedure1_0("ai:clear-force", CclAiClearForce);
    gh_new_procedure1_0("ai:evaluate-force-cost", CclAiEvaluateForceCost);

    gh_new_procedure1_0("ai:can-reach-hotspot", CclAiCanReachHotSpot);

    gh_new_procedure1_0("ai:set-hotspot-ray", CclAiSetHotSpotRay);
    gh_new_procedure0_0("ai:compute-gauges", CclAiComputeGauges);
    gh_new_procedure0_0("ai:debug-gauges", CclAiDebugGauges);
    gh_new_procedure1_0("ai:get-gauge", CclAiGetGauge);
    gh_new_procedure1_0("ai:get-unittype-force", CclAiGetUnitTypeForce);

    gh_new_procedure0_0("ai:idle", CclAiIdle);
    gh_new_procedure2_0("ai:timed-wait-force", CclAiTimedWaitForce);
    gh_new_procedure1_0("ai:hotspot-attack-with-force", CclAiHotSpotAttackWithForce);
    gh_new_procedure1_0("ai:force-go-home", CclAiForceHome);
    gh_new_procedure1_0("ai:sleep", CclAiSleep);
    gh_new_procedure1_0("ai:research", CclAiResearch);
    gh_new_procedure1_0("ai:upgrade-to", CclAiUpgradeTo);
    gh_new_procedure1_0("ai:script", CclAiScript);
    gh_new_procedure1_0("ai:goto", CclAiSwitchTo);

    gh_new_procedure0_0("ai:restart", CclAiRestart);

    gh_new_procedure0_0("ai:player", CclAiPlayer);
    gh_new_procedure1_0("ai:set-reserve!", CclAiSetReserve);
    gh_new_procedure1_0("ai:set-collect!", CclAiSetCollect);
    gh_new_procedure1_0("ai:set-auto-attack", CclAiSetAutoAttack);

    gh_new_procedure0_0("ai:dump", CclAiDump);

    gh_new_procedureN("define-ai-wc-names", CclDefineAiWcNames);

    gh_new_procedureN("define-ai-player", CclDefineAiPlayer);
#elif defined(USE_LUA)
    // FIXME: Need to save memory here.
    // Loading all into memory isn't necessary.

    lua_register(Lua, "DefineAiHelper", CclDefineAiHelper);
    lua_register(Lua, "DefineAi", CclDefineAi);

    lua_register(Lua, "DefineAiAction", CclDefineAiAction);

    lua_register(Lua, "AiGetRace", CclAiGetRace);
    lua_register(Lua, "AiGetSleepCycles", CclAiGetSleepCycles);

    lua_register(Lua, "AiDebug", CclAiDebug);
    lua_register(Lua, "AiDebugPlayer", CclAiDebugPlayer);
    lua_register(Lua, "AiNeed", CclAiNeed);
    lua_register(Lua, "AiSet", CclAiSet);
    lua_register(Lua, "AiWait", CclAiWait);

    lua_register(Lua, "AiOwnForce", CclAiOwnForce);

    lua_register(Lua, "AiGetForce", CclAiGetForce);


    lua_register(Lua, "AiForce", CclAiForce);

    lua_register(Lua, "AiAdHocForce", CclAiAdHocForce);

    lua_register(Lua, "AiForceEmpty", CclAiForceEmpty);
    lua_register(Lua, "AiForceActive", CclAiForceActive);
    lua_register(Lua, "AiForceList", CclAiForce);
    lua_register(Lua, "AiForceTransfer", CclAiForceTransfer);
    lua_register(Lua, "AiForceComplete", CclAiForceComplete);
    lua_register(Lua, "AiForceRole", CclAiForceRole);
    lua_register(Lua, "AiIsForceDefending", CclAiIsForceDefending);
    lua_register(Lua, "AiCheckForce", CclAiCheckForce);
    lua_register(Lua, "AiGroupForce", CclAiGroupForce);
    lua_register(Lua, "AiWaitForce", CclAiWaitForce);
    lua_register(Lua, "AiClearForce", CclAiClearForce);
    lua_register(Lua, "AiEvaluateForceCost", CclAiEvaluateForceCost);

    lua_register(Lua, "AiCanReachHotSpot", CclAiCanReachHotSpot);

    lua_register(Lua, "AiSetHotSpotRay", CclAiSetHotSpotRay);
    lua_register(Lua, "AiComputeGauges", CclAiComputeGauges);
    lua_register(Lua, "AiDebugGauges", CclAiDebugGauges);
    lua_register(Lua, "AiGetGauge", CclAiGetGauge);
    lua_register(Lua, "AiGetUnitTypeForce", CclAiGetUnitTypeForce);

    lua_register(Lua, "AiIdle", CclAiIdle);
    lua_register(Lua, "AiTimedWaitForce", CclAiTimedWaitForce);
    lua_register(Lua, "AiHotspotAttackWithForce", CclAiHotSpotAttackWithForce);
    lua_register(Lua, "AiForceGoHome", CclAiForceHome);
    lua_register(Lua, "AiSleep", CclAiSleep);
    lua_register(Lua, "AiResearch", CclAiResearch);
    lua_register(Lua, "AiUpgradeTo", CclAiUpgradeTo);
//    lua_register(Lua, "AiScript", CclAiScript);
//    lua_register(Lua, "AiGoto", CclAiSwitchTo);

//    lua_register(Lua, "AiRestart", CclAiRestart);

    lua_register(Lua, "AiPlayer", CclAiPlayer);
    lua_register(Lua, "AiSetReserve!", CclAiSetReserve);
    lua_register(Lua, "AiSetCollect!", CclAiSetCollect);
    lua_register(Lua, "AiSetAutoAttack", CclAiSetAutoAttack);

    lua_register(Lua, "AiDump", CclAiDump);

    lua_register(Lua, "DefineAiWcNames", CclDefineAiWcNames);

//    lua_register(Lua, "DefineAiPlayer", CclDefineAiPlayer);
#endif
}


//@}
