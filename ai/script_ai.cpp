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


/*----------------------------------------------------------------------------
--	Forwards
----------------------------------------------------------------------------*/
/// Handle saving/loading a reference to an AiType.
local void IOAiTypePtr(SCM from, void *binaryform, void *para);
/// Handle saving/loading a reference to an AiScriptAction.
local void IOAiScriptActionPtr(SCM scmfrom, void *binaryform, void *para);
/// Handle saving/loading an array of int for ressources.
local void IORessourceArray(SCM scmfrom, void *binaryform, void *para);
/// Handle saving/loading a ressource mask
local void IORessourceMask(SCM scmfrom, void *binaryform, void *para);

/*----------------------------------------------------------------------------
--	Constants
----------------------------------------------------------------------------*/

/// Description of the AiActionEvaluation structure 
static IOStructDef AiActionEvaluationStructDef = {
    "AiActionEvaluation",
    sizeof (AiActionEvaluation),
    -1,
    {
	{"`next", 		NULL, 		&((AiActionEvaluation *) 0)->Next, 	NULL},
	{"ai-script-action",	&IOAiScriptActionPtr,&((AiActionEvaluation *) 0)->aiScriptAction,NULL},
	{"gamecycle",		&IOInt,		&((AiActionEvaluation *) 0)->gamecycle,	NULL},
	{"hotspot-x", 		&IOInt,		&((AiActionEvaluation *) 0)->hotSpotX, 	NULL},
	{"hotspot-y", 		&IOInt,		&((AiActionEvaluation *) 0)->hotSpotY,	NULL},
	{"hotspot-value", 	&IOInt,		&((AiActionEvaluation *) 0)->hotSpotValue,NULL},
	{"value", 		&IOInt,		&((AiActionEvaluation *) 0)->value,	NULL},
	{0, 0, 0, 0}
    }
};

static IOStructDef AiExplorationRequestStructDef = {
    "AiExplorationRequest",
    sizeof (AiExplorationRequest),
    -1,
    {
	{"`next", 		NULL, 		&((AiExplorationRequest *) 0)->Next, 	NULL},	
	{"gamecycle",		&IOInt,		&((AiExplorationRequest *) 0)->Mask,	NULL},
	{"map-x", 		&IOInt,		&((AiExplorationRequest *) 0)->X, 	NULL},
	{"map-y", 		&IOInt,		&((AiExplorationRequest *) 0)->Y,	NULL},
	{0, 0, 0, 0}
    }
};

/// Description of the AiRunningScript structure
static IOStructDef AiRunningScriptStructDef = {
    "AiRunningScript",
    sizeof (AiRunningScript),
    AI_MAX_RUNNING_SCRIPTS,
    {
	{"script", 		&IOCcl,		&((AiRunningScript *) 0)->Script, 	NULL},
	{"sleep-cycles", 	&IOInt,		&((AiRunningScript *) 0)->SleepCycles, 	NULL},
	{"ident", 		&IOStrBuffer,	&((AiRunningScript *) 0)->ident,	(void *) 10},
	{"hotspot-x",		&IOInt,		&((AiRunningScript *) 0)->HotSpot_X,	NULL},
	{"hotspot-y",		&IOInt,		&((AiRunningScript *) 0)->HotSpot_Y,	NULL},
	{"hotspot-ray",		&IOInt,		&((AiRunningScript *) 0)->HotSpot_Ray,	NULL},
	{"own-force",		&IOInt,		&((AiRunningScript *) 0)->ownForce,	NULL},
	{"gauges",		&IOIntArrayPtr,	&((AiRunningScript *) 0)->gauges,	(void *) GAUGE_NB},
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
    sizeof (AiUnitType),
    -1,
    {
	{"'next", 		0, 		&((AiUnitType *) 0)->Next,		0},
	{"type", 		&IOUnitTypePtr,	&((AiUnitType *) 0)->Type, 		0},
	{"want", 		&IOInt,		&((AiUnitType *) 0)->Want,		0},
	{0, 0, 0, 0}
    }
};

/// Description of the AiUnit structure
static IOStructDef AiUnitStructDef = {
    "AiUnit",
    sizeof (AiUnit),
    -1,
    {
	{"'next", 		NULL, 		&((AiUnit *) 0)->Next,			0},
	{"unit", 		&IOUnitPtr,	&((AiUnit *) 0)->Unit, 			0},
	{0, 0, 0, 0}
    }
};

/// Description of the AiForce structure
static IOStructDef AiForceStructDef = {
    "AiForce",
    sizeof (AiForce),
    AI_MAX_FORCES,
    {
	{"completed", 		&IOCharBool,	&((AiForce *) 0)->Completed, 		0},
	{"attacking", 		&IOCharBool, 	&((AiForce *) 0)->Attacking, 		0},
	{"role",		&IOCharFlag, 	&((AiForce *) 0)->Role, 		&AiRoleFlag},
	{"populate-mode",	&IOCharFlag, 	&((AiForce *) 0)->PopulateMode,		&AiPopulateFlag},
	{"units-reusable",	&IOCharBool, 	&((AiForce *) 0)->UnitsReusable, 	0},
	{"help-mode",		&IOCharFlag, 	&((AiForce *) 0)->HelpMode, 		&AiHelpFlag},
	{"unit-wants",		&IOLinkedList, 	&((AiForce *) 0)->UnitTypes,		&AiUnitTypeStructDef},
	{"unit-presents",	&IOLinkedList, 	&((AiForce *) 0)->Units, 		&AiUnitStructDef},
	{"attack-state",	&IOInt, 	&((AiForce *) 0)->State, 		0},
	{"attack-goal-x",	&IOInt, 	&((AiForce *) 0)->GoalX, 		0},
	{"attack-goal-y",	&IOInt, 	&((AiForce *) 0)->GoalY, 		0},
	{"must-transport",	&IOBool, 	&((AiForce *) 0)->MustTransport, 	0},
	{0, 0, 0, 0}
    }
};

/// Description of the AiBuildQueue structure/linked list
static IOStructDef AiBuildQueueStructDef = {
    "AiBuildQueue",
    sizeof (AiBuildQueue),
    -1,
    {
	{"`next", 		0, 		&((AiBuildQueue *) 0)->Next, 		0},
	{"want", 		&IOInt,		&((AiBuildQueue *) 0)->Want,		0},
	{"made", 		&IOInt,		&((AiBuildQueue *) 0)->Made,		0},
	{"type", 		&IOUnitTypePtr,	&((AiBuildQueue *) 0)->Type,		0},
	{0, 0, 0, 0}
    }
};

/// Description of the AiUnitTypeTable table in PlayerAi
static IOStructDef AiUnitTypeTableStructDef = {
    "AiUnitTypeTable",
    sizeof (AiUnitTypeTable),
    -1,
    {
	{"unittype", 		&IOUnitTypePtr,	&((AiUnitTypeTable *) 0)->Table,	0},
	{"count", 		&IOInt,		&((AiUnitTypeTable *) 0)->Count,	0},
	{0, 0, 0, 0}
    }
};


/// Description of the UnitTypeRequests table in PlayerAi
static IOStructDef UnitTypeRequestsTableDef = {
    "UnitTypeRequests",
    sizeof (AiUnitTypeTable),
    -1,
    {
	{"`ptr", 		0,		&((PlayerAi *) 0)->UnitTypeRequests, 	0},
	{"`count", 		0,		&((PlayerAi *) 0)->UnitTypeRequestsCount,0},
	{"`items", 		&IOStruct,	0, 					&AiUnitTypeTableStructDef},
	{0, 0, 0, 0}
    }
};

/// Description of the UpgradeToRequests table in PlayerAi
static IOStructDef UpgradeToRequestsTableDef = {
    "UpgradeToRequests",
    sizeof (UnitType *),
    -1,
    {
	{"`ptr", 		0,		&((PlayerAi *) 0)->UpgradeToRequests,	0},
	{"`count",		0,		&((PlayerAi *) 0)->UpgradeToRequestsCount,0},
	{"`items",		&IOUnitTypePtr,	0,					0},
	{0, 0, 0, 0}
    }
};

/// Description of the ResearchRequests table in PlayerAi
static IOStructDef ResearchRequestsTableDef = {
    "ResearchRequests",
    sizeof (Upgrade *),
    -1,
    {
	{"`ptr",		0,		&((PlayerAi *) 0)->ResearchRequests,	0},
	{"`count",		0,		&((PlayerAi *) 0)->ResearchRequestsCount,0},
	{"`items",		&IOUpgradePtr,	0,					0},
	{0, 0, 0, 0}
    }
};

/// Description of the PlayerAi structure
static IOStructDef PlayerAiStructDef = {
    "PlayerAi",
    sizeof (PlayerAi),
    -1,
    {
	{"player",		&IOPlayerPtr,	&((PlayerAi *) 0)->Player,		0},
	{"ai-type",		&IOAiTypePtr,	&((PlayerAi *) 0)->AiType,		0},
	{"scripts",		&IOStructArray,	&((PlayerAi *) 0)->Scripts,		&AiRunningScriptStructDef},
	{"past-evaluations",	&IOLinkedList,	&((PlayerAi *) 0)->FirstEvaluation,	&AiActionEvaluationStructDef},
	{"debug",		&IOBool,	&((PlayerAi *) 0)->ScriptDebug,		0},
	{"auto-attack",		&IOBool,	&((PlayerAi *) 0)->AutoAttack,		0},
	{"forces",		&IOStructArray,	&((PlayerAi *) 0)->Force,		&AiForceStructDef},
	{"reserve",		&IORessourceArray,&((PlayerAi *) 0)->Reserve, 		0},
	{"used",		&IORessourceArray,&((PlayerAi *) 0)->Used,		0},
	{"needed",		&IORessourceArray,&((PlayerAi *) 0)->Needed,		0},
	{"collect", 		&IORessourceArray,&((PlayerAi *) 0)->Collect,		0},
	{"neededmask",		&IORessourceMask,&((PlayerAi *) 0)->Reserve,		0},
	{"need-food",		&IOBool,	&((PlayerAi *) 0)->NeedFood,		0},
	{"exploration-requests",&IOLinkedList,	&((PlayerAi *) 0)->FirstExplorationRequest,&AiExplorationRequestStructDef},
	{"last-exploration",	&IOInt,		&((PlayerAi *) 0)->LastExplorationGameCycle,0},
	{"unit-type-requests",	&IOTable,	0,					&UnitTypeRequestsTableDef},
	{"upgrade-to-requests",	&IOTable,	0,					&UpgradeToRequestsTableDef},
	{"research-requests",	&IOTable,	0,					&ResearchRequestsTableDef},
	{"unit-type-builded",	&IOLinkedList,	&((PlayerAi *) 0)->UnitTypeBuilded,	&AiBuildQueueStructDef},
	{"last-repair-building",&IOInt,		&((PlayerAi *) 0)->LastRepairBuilding,	0},
	{"tried-repair-worker",	&IOIntArray,	&((PlayerAi *) 0)->TriedRepairWorkers,	(void*)UnitMax},
	{0, 0, 0, 0}
    }
};

/// Description of the PlayerAi structure
static IOStructDef AiTypeStructDef = {
    "AiType",
    sizeof (AiType),
    -1,
    {
	{"name", 		&IOString,	&((AiType *) 0)->Name,			0},
	{"race", 		&IOString,	&((AiType *) 0)->Race,			0},
	{"class",		&IOString,	&((AiType *) 0)->Class,			0},
	{"script",		&IOCcl,		&((AiType *) 0)->Script,		0},
	{0, 0, 0, 0}
    }
};

static IOStructDef AiScriptActionStructDef = {
    "AiScriptAction",
    sizeof (AiScriptAction),
    -1,
    {
	{"action",		&IOCcl,		&((AiScriptAction *) 0)->Action,	0},
	{"defensive",		&IOBool,	&((AiScriptAction *) 0)->Defensive,	0},
	{"offensive",		&IOBool,	&((AiScriptAction *) 0)->Offensive,	0},
	{0, 0, 0, 0}
    }
};


/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Handle saving/loading a reference to an AiType ( AiType * ).
**	The null case is handled.
**
**	@param	scmform		When loading, the scm data to load
**	@param	binaryform	Pointer to the unit'ref ( AiType ** )
**	@param	para		unused
*/
local void IOAiTypePtr(SCM from, void *binaryform, void *para)
{
    char buffer[512];
    char *str;
    AiType *cur;

    if (IOHandleNullPtr(from, binaryform)) {
	return;
    }
    if (IOLoadingMode) {
	str = gh_scm2newstr(from, 0);
	cur = AiTypes;
	while (cur) {
	    snprintf(buffer, 512, "%s-%s-%s", cur->Name, cur->Race, cur->Class);
	    if (!strcmp(str, buffer)) {
		*((AiType **) binaryform) = cur;
		return;
	    }
	    cur = cur->Next;
	}
	errl("unknown aitype ", from);
    } else {
	cur = *((AiType **) binaryform);

	snprintf(buffer, 512, "%s-%s-%s", cur->Name, cur->Race, cur->Class);
	CLprintf(IOOutFile, " \"%s\"", buffer);
    }
}


/**
**	Handle saving/loading a reference to an AiScriptAction.
**	The null case is handled.
**
**	@param	scmform		When loading, the scm data to load
**	@param	binaryform	Pointer to the unit'ref ( AiScriptAction ** )
**	@param	para		unused
*/
local void IOAiScriptActionPtr(SCM scmfrom, void *binaryform, void *para)
{
    int slot;
    AiScriptAction *a;
    if (IOHandleNullPtr(scmfrom, binaryform)) {
	return;
    }
    if (IOLoadingMode) {
	slot = gh_scm2int(scmfrom);
	*((AiScriptAction **) binaryform) = AiScriptActions + slot;
    } else {
	a = *((AiScriptAction **) binaryform);
	CLprintf(IOOutFile, " %d", a - AiScriptActions);
    }
}

/// Handle loading an array of int for each ressource ( int[MAX_COSTS] )
local void IORessourceArray(SCM scmfrom, void *binaryform, void *para)
{
    IOIntArray(scmfrom, binaryform, (void *) MaxCosts);
}

/// Handle loading a mask for each ressource ( int[MAX_COSTS] )
local void IORessourceMask(SCM scmfrom, void *binaryform, void *para)
{
    int tmp[MaxCosts];
    int mask;
    int i;

    if (IOLoadingMode) {
	IOIntArray(scmfrom, tmp, (void *) MaxCosts);

	mask = 0;
	for (i = 0; i < MaxCosts; i++) {
	    if (tmp[i]) {
		mask |= (1 << i);
	    }
	}

	*(int *) binaryform = mask;
    } else {
	mask = *(int *) binaryform;
	for (i = 0; i < MaxCosts; i++) {
	    if (mask & (1 << i)) {
		tmp[i] = 1;
	    } else {
		tmp[i] = 0;
	    }
	}

	IOIntArray(scmfrom, tmp, (void *) MaxCosts);
    }
}

/**
**	Handle saving/loading a full PlayerAi structure.
**	The structure is allocated on the heap, filled from ccl, then completed.
**
**	@param	scmform		When loading, the scm data to load
**	@param	binaryform	Pointer to the PlayerAi'ref ( PlayerAi ** )
**	@param	para		unused
*/
global void IOPlayerAiFullPtr(SCM form, void *binaryform, void *para)
{
    AiActionEvaluation *aa;
    PlayerAi **playerAi = (PlayerAi **) binaryform;

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

global void IOAiTypeFullPtr(SCM form, void *binaryform, void *para)
{
    AiType **aiType = (AiType **) binaryform;
    IOStructPtr(form, binaryform, &AiTypeStructDef);
    if (IOLoadingMode && (*aiType)) {
	// Append the ai_type...
	(*aiType)->Next = AiTypes;
	AiTypes = (*aiType);
    }
}

global void IOAiScriptActionFull(SCM form, void *binaryform, void *para)
{
    /*AiScriptAction * asa = (AiScriptAction*) binaryform; */
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


/**
**	Setup AI helper table.
**
**	Expand the table if needed.
**
**	@param count	Pointer to number of elements in table.
**	@param table	Pointer to table with elements.
**	@param n	Index to insert new into table
*/
local void AiHelperSetupTable(int *count, AiUnitTypeTable *** table, int n)
{
    int i;

    ++n;
    if (n > (i = *count)) {
	if (*table) {
	    *table = realloc(*table, n * sizeof (AiUnitTypeTable *));
	    memset((*table) + i, 0, (n - i) * sizeof (AiUnitTypeTable *));
	} else {
	    *table = malloc(n * sizeof (AiUnitTypeTable *));
	    memset(*table, 0, n * sizeof (AiUnitTypeTable *));
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
local void AiHelperInsert(AiUnitTypeTable ** tablep, UnitType * base)
{
    int i;
    int n;
    AiUnitTypeTable *table;

    //
    //  New unit-type
    //
    if (!(table = *tablep)) {
	table = *tablep = malloc(sizeof (AiUnitTypeTable));
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
    table = *tablep = realloc(table, sizeof (AiUnitTypeTable) + sizeof (UnitType *) * n);
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
local SCM CclDefineAiHelper(SCM list)
{
    SCM sub_list;
    SCM value;
    int what;
    char *str;
    UnitType *base;
    UnitType *type;
    Upgrade *upgrade;
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

		    AiNewUnitTypeEquiv(base,type);
		    break;
		case 6:		// repair
		    AiHelperSetupTable(&AiHelpers.RepairCount, &AiHelpers.Repair,
			type->Type);
		    AiHelperInsert(AiHelpers.Repair + type->Type, base);
		    break;
	    }
	}
    }

    return list;
}

local SCM CclDefineAiAction(SCM type, SCM definition)
{
    AiScriptAction *aiScriptAction;

    aiScriptAction = AiScriptActions + AiScriptActionNum;
    AiScriptActionNum++;

    memset(aiScriptAction, 0, sizeof (AiScriptAction));

    aiScriptAction->Action = definition;
    CclGcProtect(aiScriptAction->Action);

    while (!gh_null_p(type)) {
	if (gh_eq_p(gh_car(type), gh_symbol2scm("defense"))) {
	    aiScriptAction->Defensive = 1;
	}
	if (gh_eq_p(gh_car(type), gh_symbol2scm("attack"))) {
	    aiScriptAction->Offensive = 1;
	}
	type = gh_cdr(type);
    }

    return SCM_UNSPECIFIED;
}

/**
**	Define an AI engine.
*/
local SCM CclDefineAi(SCM list)
{
    SCM value;
    char *str;
    AiType *aitype;
#ifdef DEBUG
    const AiType *ait;
#endif

    aitype = malloc(sizeof (AiType));
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
    CclGcProtect(value);

    return list;
}

/*----------------------------------------------------------------------------
--	AI script functions
----------------------------------------------------------------------------*/

    /// Get unit-type.
extern UnitType *CclGetUnitType(SCM ptr);

/**
**	Append unit-type to request table.
**
**	@param type	Unit-type to be appended.
**	@param count	How many unit-types to build.
*/
local void InsertUnitTypeRequests(UnitType * type, int count)
{
    int n;
    
    if (AiPlayer->UnitTypeRequests) {
	n = AiPlayer->UnitTypeRequestsCount;
	AiPlayer->UnitTypeRequests = realloc(AiPlayer->UnitTypeRequests,
	    (n + 1) * sizeof (*AiPlayer->UnitTypeRequests));
    } else {
	AiPlayer->UnitTypeRequests = malloc(sizeof (*AiPlayer->UnitTypeRequests));
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
local AiUnitTypeTable *FindInUnitTypeRequests(const UnitType * type)
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
local int FindInUpgradeToRequests(const UnitType * type)
{
    int i;
    int n;

    n = AiPlayer->UnitTypeRequestsCount;
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
local void InsertUpgradeToRequests(UnitType * type)
{
    int n;

    if (AiPlayer->UpgradeToRequests) {
	n = AiPlayer->UpgradeToRequestsCount;
	AiPlayer->UpgradeToRequests = realloc(AiPlayer->UpgradeToRequests,
	    (n + 1) * sizeof (*AiPlayer->UpgradeToRequests));
    } else {
	AiPlayer->UpgradeToRequests = malloc(sizeof (*AiPlayer->UpgradeToRequests));
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
local void InsertResearchRequests(Upgrade * upgrade)
{
    int n;

    if (AiPlayer->ResearchRequests) {
	n = AiPlayer->ResearchRequestsCount;
	AiPlayer->ResearchRequests = realloc(AiPlayer->ResearchRequests,
	    (n + 1) * sizeof (*AiPlayer->ResearchRequests));
    } else {
	AiPlayer->ResearchRequests = malloc(sizeof (*AiPlayer->ResearchRequests));
	n = 0;
    }
    AiPlayer->ResearchRequests[n] = upgrade;
    AiPlayer->ResearchRequestsCount = n + 1;
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
    if (gh_eq_p(flag, SCM_BOOL_F)) {
	AiPlayer->ScriptDebug = 0;
    } else {
	AiPlayer->ScriptDebug = 1;
    }
    return SCM_BOOL_F;
}

/**
**	Activate AI debugging for the given player(s)
**	Player can be
**		a number for a specific player 
**		"self" for current human player (ai me)
**	 	"none" to disable
**
**	@param list the list of player to activate
*/
local SCM CclAiDebugPlayer(SCM list)
{
    SCM item;
    int playerid;
    while (!gh_null_p(list)) {
	item = gh_car(list);
	list = gh_cdr(list);

	if (gh_eq_p(item, gh_symbol2scm("none"))) {
	    for (playerid = 0; playerid < NumPlayers; playerid++) {
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

/**
**	Need an unit.
**
**	@param value	Unit-type as string/symbol/object.
*/
local SCM CclAiNeed(SCM value)
{
    InsertUnitTypeRequests(CclGetUnitType(value), 1);

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
local SCM CclAiSet(SCM value, SCM count)
{
    AiUnitTypeTable *autt;
    UnitType *type;

    type = CclGetUnitType(value);
    if ((autt = FindInUnitTypeRequests(type))) {
	autt->Count = gh_scm2int(count);
	// FIXME: 0 should remove it.
    } else {
	InsertUnitTypeRequests(type, gh_scm2int(count));
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
    const AiUnitTypeTable *autt;
    const UnitType *type;
    const int *unit_types_count;
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

/**
**	Give the number of the script specific force.
**
*/
local SCM CclAiOwnForce(void)
{
    return gh_int2scm(AiScript->ownForce);
}

/**
**	Free a force ( requirements and current units )
**
**	@param s_force	Force to free.
*/
local SCM CclAiClearForce(SCM s_force)
{
    int force;
/*    AiUnitType * aiut,*next;
    AiUnit *aiu,*next_u;
*/
    force = gh_scm2int(s_force);
    if (force < 0 || force >= AI_MAX_FORCES) {
	errl("Force out of range", s_force);
    }
/*
    aiut=AiPlayer->Force[force].UnitTypes;
    while(aiut){
    	next=aiut->Next;
	free(aiut);
	aiut=next;
    }
    AiPlayer->Force[force].UnitTypes=0;

    aiu=AiPlayer->Force[force].Units;
    while(aiu){
	next_u=aiu->Next;
	free(aiu);
	aiu=next_u;
    }
    AiPlayer->Force[force].Units=0;
    
    AiAssignFreeUnitsToForce();
    */
    AiEraseForce(force);
    return SCM_BOOL_F;

}

local SCM CclAiGetForce(SCM list)
{
    int force;
    SCM rslt;
    AiUnitType *aiut;

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

/**
**	Return true if a force has no unit, fase otherwise.
**
**
*/
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

local SCM CclAiAdHocForce(SCM requirement, SCM scm_unittypes)
{
    int want[3];
    int *unittypes;
    int unittypecount;
    UnitType *unittype;
    char *str;
    int i;
    int rslt;

    for (i = 0; i < 3; i++) {
	want[i] = gh_scm2int(gh_car(requirement));
	requirement = gh_cdr(requirement);
    }

    unittypecount = gh_length(scm_unittypes);
    if (unittypecount) {
	unittypes = (int *) malloc(sizeof (int) * unittypecount);

	for (i = 0; i < unittypecount; i++) {

	    str = gh_scm2newstr(gh_car(scm_unittypes), NULL);
	    scm_unittypes = gh_cdr(scm_unittypes);


	    unittype = UnitTypeByIdent(str);
	    if (!unittype) {
		fprintf(stderr, "unknown unittype %s\n", str);
		free(str);
		i--;
		unittypecount--;
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

local SCM CclAiForceActive(SCM list)
{
    int force;

    force = gh_scm2int(list);
    if (force < 0 || force >= AI_MAX_FORCES) {
	errl("Force out of range", list);
    }

    if (AiPlayer->Force[force].Attacking) {
	return SCM_BOOL_T;
    } else {
	return SCM_BOOL_F;
    }
}

/**
**	Define a force, a groups of units.
**	If force already exists, list is interpreted as a minimum...
**
**	@param list	Pairs of unit-types and counts.
*/
local SCM CclAiForce(SCM list)
{
    AiUnitType **prev;
    AiUnitType *aiut;
    UnitType *type;
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
	    *prev = aiut = malloc(sizeof (*aiut));
	    aiut->Next = NULL;
	    aiut->Want = count;
	    aiut->Type = type;
	}
    }

    AiAssignFreeUnitsToForce();

    return SCM_BOOL_F;
}

local SCM CclAiForceTransfert(SCM sourceForce, SCM destForce)
{
    int src, dst;

    src = gh_scm2int(sourceForce);
    dst = gh_scm2int(destForce);
    AiForceTransfert(src, dst);
    return SCM_BOOL_F;
}

/**
**	Wrapper of AiForceComplete.
**	Complete a force with existing units.
**
**	@param destForce	the force to complete
*/
local SCM CclAiForceComplete(SCM destForce)
{
    int dst;

    dst = gh_scm2int(destForce);
    AiForceComplete(dst);
    return SCM_BOOL_F;
}

/**
**	Define the role of a force.
**
**	@param value	Force number.
**	@param flag	Which role of the force.
*/
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


/**
**	Return wether a force is defending or not.
**
**	@param	value	the force
**	@return 	true if the force is defending, false otherwise
*/
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


/**
**	Check if the hotspot can be reached.
**	The parameter describe which element we are travelling in.
**
**	@param way	air, ground or sea
**	@return		true if the hotspot is reachable
*/
local SCM CclAiCanReachHotSpot(SCM way)
{
    int wayid, i;
    int x, y, w, h;
    Unit *unit;


    if ((AiScript->HotSpot_X == -1) || (AiScript->HotSpot_Y == -1)
	|| (AiScript->HotSpot_Ray <= 0)) {
	return SCM_BOOL_T;
    }

    wayid = -1;
    // Air is always accessible...
    if (gh_eq_p(way, gh_symbol2scm("air"))) {
	wayid = 0;
    }
    if (gh_eq_p(way, gh_symbol2scm("ground"))) {
	wayid = 1;
    }
    if (gh_eq_p(way, gh_symbol2scm("sea"))) {
	wayid = 2;
    }

    if (wayid == -1) {
	errl("Unknown way ", way);
    }
    // Calculate a rectangle...
    // FIXME : Why 2 ?
    x = AiScript->HotSpot_X - 2;
    y = AiScript->HotSpot_Y - 2;
    w = 5;
    h = 5;

    // clip it
    if (x < 0) {
	w += x;
	x = 0;
    }
    if (y < 0) {
	h += y;
	y = 0;
    }
    if (x + w >= TheMap.Width) {
	w = TheMap.Width - x;
    }

    if (y + h >= TheMap.Height) {
	h = TheMap.Height - y;
    }
    // Dummy check...
    if ((w <= 0) || (h <= 0)) {
	return SCM_BOOL_F;
    }
    // Air is no problem...
    if (wayid == 0) {
	return SCM_BOOL_T;
    }
    // Find a unit which can move, check if it can access the hotspot.
    for (i = 0; i < AiPlayer->Player->TotalNumUnits; i++) {
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

	if (unit->Type->Building) {
	    continue;
	}

	if ((wayid == 1) && (unit->Type->UnitType != UnitTypeLand)) {
	    continue;
	}

	if ((wayid == 2) && (unit->Type->UnitType != UnitTypeNaval)) {
	    continue;
	}
	// Ok, unit is a possible test
	if (PlaceReachable(unit, x, y, w, h, 2)) {
	    return SCM_BOOL_T;
	}
	return SCM_BOOL_F;
    }

    // Fall back : no unit at all ? Use the starting point...
    DebugLevel3Fn("CclAiCanReach failled to use a test unit...\n");
    return SCM_BOOL_T;
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

    force = gh_scm2int(value);
    if (force < 0 || force >= AI_MAX_FORCES) {
	errl("Force out of range", value);
    }
    if (AiPlayer->Force[force].Completed) {
	return SCM_BOOL_T;
    }
    return SCM_BOOL_F;
}

/**
** Evaluate the ressources needed to complete a force.
**
** @param s_force the force
**
** @return -1 if it is not possible ( upgrade missing )
*/
local SCM CclAiEvaluateForceCost(SCM s_force)
{
    return gh_int2scm(AiEvaluateForceCost(gh_scm2int(s_force), 0));
}

// Just hang...
local SCM CclAiIdle(void)
{
    return SCM_BOOL_T;
}

/**
**	Wait for a force ready.
**
**	@param value	Force number.
*/
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

/**
**	Attack with force.
**
**	@param value	Force number.
*/
local SCM CclAiAttackWithForce(SCM value)
{
    int force;

    force = gh_scm2int(value);
    if (force < 0 || force >= AI_MAX_FORCES) {
	errl("Force out of range", value);
    }

    AiAttackWithForce(force);

    return SCM_BOOL_F;
}

/**
**	Attack with force, on the current script hotspot.
**
**	@param value	Force number.
*/
local SCM CclAiHotSpotAttackWithForce(SCM value)
{
    int force;

    force = gh_scm2int(value);
    if (force < 0 || force >= AI_MAX_FORCES) {
	errl("Force out of range", value);
    }

    AiAttackWithForceAt(force, AiScript->HotSpot_X, AiScript->HotSpot_Y);

    return SCM_BOOL_F;
}

/**
**
**
*/
local SCM CclAiGroupForce(SCM value)
{
    int force;

    force = gh_scm2int(value);
    if (force < 0 || force >= AI_MAX_FORCES) {
	errl("Force out of range", value);
    }

    AiGroupForceNear(force, AiScript->HotSpot_X, AiScript->HotSpot_Y);

    return SCM_BOOL_F;
}

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

/**
**	Wait for a force (like CclAiWaitForce), limited in time.
**
**	@param s_force	Force number.
**	@param s_wait	Maximum number of cycles to delay.
*/
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

/**
**	Sleep n cycles.
**
**	@param value	Number of cycles to delay.
*/
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

/**
**	Research an upgrade.
**
**	@param value	Upgrade as string/symbol/object.
*/
local SCM CclAiResearch(SCM value)
{
    const char *str;
    Upgrade *upgrade;

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

/**
**	Upgrade an unit to an new unit-type.
**
**	@param value	Unit-type as string/symbol/object.
*/
local SCM CclAiUpgradeTo(SCM value)
{
    UnitType *type;

    type = CclGetUnitType(value);
    InsertUpgradeToRequests(type);

    return SCM_BOOL_F;
}

local SCM CclAiSetHotSpotRay(SCM value)
{
    return SCM_BOOL_F;
}

local SCM CclAiComputeGauges(void)
{
    AiComputeCurrentScriptGauges();
    return SCM_BOOL_F;
}

local SCM CclAiDebugGauges(void)
{
    AiDebugGauges();
    return SCM_BOOL_F;
}

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

local SCM CclGetUnitTypeForce(SCM value)
{
    UnitType *unitType;

    unitType = CclGetUnitType(value);

    return gh_int2scm(AiUnittypeForce(unitType));
}

/**
**	Simple restart the AI.
*/
local SCM CclAiRestart(void)
{
    int i;
    AiPlayer->Scripts[0].Script = AiPlayer->AiType->Script;
    AiPlayer->Scripts[0].SleepCycles = 0;
    snprintf(AiPlayer->Scripts[0].ident, 10, "Main AI");
    for (i = 1; i < AI_MAX_RUNNING_SCRIPTS; i++) {
	AiPlayer->Scripts[i].Script = NIL;
	AiPlayer->Scripts[i].SleepCycles = 0;
	snprintf(AiPlayer->Scripts[i].ident, 10, "Empty");
    }
    return SCM_BOOL_T;
}

/**
**	Start an AI script at the given id.
**
**      @param script		The Id of the script to run ( main, defend, ... )
**	@param list		The script execution
**	@param hotSpotX		position of the hotspot (X)
**	@param hotSpotY		position of the hotspot (Y)
**	@param hotSpotRay	ray of the hotspot
*/
global void AiRunScript(int script, SCM list, int hotSpotX, int hotSpotY, int hotSpotRay)
{
    AiPlayer->Scripts[script].Script = list;
    AiPlayer->Scripts[script].SleepCycles = 0;
    snprintf(AiPlayer->Scripts[script].ident, 10, "AiRunScript");
    AiPlayer->Scripts[script].HotSpot_X = hotSpotX;
    AiPlayer->Scripts[script].HotSpot_Y = hotSpotY;
    AiPlayer->Scripts[script].HotSpot_Ray = hotSpotRay;
    // TODO : Compute gauges here ?    
}

/**
**	Change the current script
*/
local SCM CclAiSwitchTo(SCM value)
{
    AiScript->Script = value;
    AiScript->SleepCycles = 0;
    return SCM_BOOL_T;
}

/**
**	Execute new main script
*/
local SCM CclAiScript(SCM value)
{
    int i;
    AiPlayer->Scripts[0].Script = value;
    AiPlayer->Scripts[0].SleepCycles = 0;
    snprintf(AiPlayer->Scripts[0].ident, 10, "MainScript");
    for (i = 1; i < AI_MAX_RUNNING_SCRIPTS; i++) {
	AiPlayer->Scripts[i].Script = NIL;
	AiPlayer->Scripts[i].SleepCycles = 0;
	AiEraseForce(AiPlayer->Scripts[i].ownForce);
	snprintf(AiPlayer->Scripts[i].ident, 10, "Empty");	
    }
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

    old = cons_array(gh_int2scm(MaxCosts), 0);
    for (i = 0; i < MaxCosts; ++i) {
	aset1(old, gh_int2scm(i), gh_int2scm(AiPlayer->Reserve[i]));
    }
    for (i = 0; i < MaxCosts; ++i) {
	AiPlayer->Reserve[i] = gh_scm2int(gh_vector_ref(vec, gh_int2scm(i)));
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

    old = cons_array(gh_int2scm(MaxCosts), 0);
    for (i = 0; i < MaxCosts; ++i) {
	aset1(old, gh_int2scm(i), gh_int2scm(AiPlayer->Collect[i]));
    }
    for (i = 0; i < MaxCosts; ++i) {
	AiPlayer->Collect[i] = gh_scm2int(gh_vector_ref(vec, gh_int2scm(i)));
    }
    return old;
}

/**
**	Set the autoattack flag of the current AiPlayer
**
**	@param val	new value of the autoattack flag
**	@return		SCM_BOOL_F
*/
local SCM CclAiSetAutoAttack(SCM val)
{
    AiPlayer->AutoAttack = gh_scm2bool(val);
    return SCM_BOOL_F;
}

/**
**	Dump some AI debug informations.
*/
local SCM CclAiDump(void)
{
    int i;
    int n;
    const AiUnitType *aut;
    const AiBuildQueue *queue;

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

/**
**	Define AI mapping from original number to internal symbol
**
**	@param list	List of all names.
*/
local SCM CclDefineAiWcNames(SCM list)
{
    int i;
    char **cp;

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
    AiTypeWcNames = cp = malloc((i + 1) * sizeof (char *));
    while (i--) {
	*cp++ = gh_scm2newstr(gh_car(list), NULL);
	list = gh_cdr(list);
    }
    *cp = NULL;

    return SCM_UNSPECIFIED;
}

/**
**	Get the default resource number
**
**	@param type	The name of the resource to lookup
**
**	@return		The number of the resource in DEFAULT_NAMES
*/
local int DefaultResourceNumber(const char *type)
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

#if 0
    i = gh_scm2int(gh_car(list));
    list = gh_cdr(list);

    DebugCheck(i < 0 || i > PlayerMax);
    DebugLevel0Fn("%p %d\n" _C_ Players[i].Ai _C_ Players[i].AiEnabled);
    // FIXME: loose this:
    // DebugCheck( Players[i].Ai || !Players[i].AiEnabled );

    ai = Players[i].Ai = calloc(1, sizeof (PlayerAi));
    ai->Player = &Players[i];

    snprintf(AiPlayer->Scripts[0].ident, 10, "Empty");
    for (i = 1; i < AI_MAX_RUNNING_SCRIPTS; i++) {
	ai->Scripts[i].Script = NIL;
	ai->Scripts[i].SleepCycles = 0;
	snprintf(AiPlayer->Scripts[i].ident, 10, "Empty");
    }

    //
    //  Parse the list: (still everything could be changed!)
    //
    while (!gh_null_p(list)) {

	value = gh_car(list);
	list = gh_cdr(list);

	if (gh_eq_p(value, gh_symbol2scm("ai-type"))) {
	    AiType *ait;

	    str = gh_scm2newstr(gh_car(list), NULL);
	    for (ait = AiTypes; ait; ait = ait->Next) {
		if (!strcmp(ait->Name, str)) {
		    break;
		}
	    }
	    free(str);
	    if (!ait) {
		errl("ai-type not found", gh_car(list));
	    }
	    ai->AiType = ait;
	    ai->Scripts[0].Script = ait->Script;
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("script"))) {
	    sublist = gh_car(list);
	    value = gh_car(sublist);
	    sublist = gh_cdr(sublist);
	    if (gh_eq_p(value, gh_symbol2scm("aitypes"))) {
		i = gh_scm2int(gh_car(sublist));
		while (i--) {
		    ai->Scripts[0].Script = gh_cdr(ai->Scripts[0].Script);
		}
	    } else {
		DebugLevel0Fn("FIXME: not written\n");
	    }
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("script-debug"))) {
	    ai->ScriptDebug = gh_scm2bool(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("sleep-cycles"))) {
	    ai->Scripts[0].SleepCycles = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("force"))) {
	    sublist = gh_car(list);
	    value = gh_car(sublist);
	    sublist = gh_cdr(sublist);
	    i = gh_scm2int(value);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		if (gh_eq_p(value, gh_symbol2scm("complete"))) {
		    ai->Force[i].Completed = 1;
		} else if (gh_eq_p(value, gh_symbol2scm("recruit"))) {
		    ai->Force[i].Completed = 0;
		} else if (gh_eq_p(value, gh_symbol2scm("attack"))) {
		    ai->Force[i].Attacking = 1;
		} else if (gh_eq_p(value, gh_symbol2scm("role"))) {
		    if (gh_eq_p(gh_car(sublist), gh_symbol2scm("attack"))) {
			ai->Force[i].Role = AiForceRoleAttack;
		    } else if (gh_eq_p(gh_car(sublist), gh_symbol2scm("defend"))) {
			ai->Force[i].Role = AiForceRoleDefend;
		    }
		    sublist = gh_cdr(sublist);
		} else if (gh_eq_p(value, gh_symbol2scm("types"))) {
		    AiUnitType **queue;
		    SCM subsublist;
		    subsublist = gh_car(sublist);
		    queue = &ai->Force[i].UnitTypes;
		    while (!gh_null_p(subsublist)) {
			int num;
			char *ident;
			value = gh_car(subsublist);
			subsublist = gh_cdr(subsublist);
			num = gh_scm2int(value);
			value = gh_car(subsublist);
			subsublist = gh_cdr(subsublist);
			ident = get_c_string(value);
			*queue = malloc(sizeof (AiUnitType));
			(*queue)->Next = NULL;
			(*queue)->Want = num;
			(*queue)->Type = UnitTypeByIdent(ident);
			queue = &(*queue)->Next;
		    }
		    sublist = gh_cdr(sublist);
		} else if (gh_eq_p(value, gh_symbol2scm("units"))) {
		    AiUnit **queue;
		    SCM subsublist;
		    subsublist = gh_car(sublist);
		    queue = &ai->Force[i].Units;
		    while (!gh_null_p(subsublist)) {
			int num;
			char *ident;
			value = gh_car(subsublist);
			subsublist = gh_cdr(subsublist);
			num = gh_scm2int(value);
			value = gh_car(subsublist);
			subsublist = gh_cdr(subsublist);
			ident = get_c_string(value);
			*queue = malloc(sizeof (AiUnit));
			(*queue)->Next = NULL;
			(*queue)->Unit = UnitSlots[num];
			queue = &(*queue)->Next;
		    }
		}
	    }
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("reserve"))) {
	    sublist = gh_car(list);
	    while (!gh_null_p(sublist)) {
		char *type;
		int num;
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		type = get_c_string(value);
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		num = gh_scm2int(value);
		ai->Reserve[DefaultResourceNumber(type)] = num;
	    }
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("used"))) {
	    sublist = gh_car(list);
	    while (!gh_null_p(sublist)) {
		char *type;
		int num;
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		type = get_c_string(value);
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		num = gh_scm2int(value);
		ai->Used[DefaultResourceNumber(type)] = num;
	    }
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("needed"))) {
	    sublist = gh_car(list);
	    while (!gh_null_p(sublist)) {
		char *type;
		int num;
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		type = get_c_string(value);
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		num = gh_scm2int(value);
		ai->Needed[DefaultResourceNumber(type)] = num;
	    }
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("collect"))) {
	    sublist = gh_car(list);
	    while (!gh_null_p(sublist)) {
		char *type;
		int num;
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		type = get_c_string(value);
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		num = gh_scm2int(value);
		ai->Collect[DefaultResourceNumber(type)] = num;
	    }
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("need-mask"))) {
	    sublist = gh_car(list);
	    while (!gh_null_p(sublist)) {
		char *type;
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		type = get_c_string(value);
		ai->NeededMask |= (1 << DefaultResourceNumber(type));
	    }
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("need-food"))) {
	    ai->NeedFood = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("unit-type"))) {
	    sublist = gh_car(list);
	    i = 0;
	    if (gh_length(sublist)) {
		ai->UnitTypeRequests =
		    malloc(gh_length(sublist) / 2 * sizeof (AiUnitTypeTable));
	    }
	    while (!gh_null_p(sublist)) {
		char *ident;
		int count;
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		ident = get_c_string(value);
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		count = gh_scm2int(value);
		ai->UnitTypeRequests[i].Table[0] = UnitTypeByIdent(ident);
		ai->UnitTypeRequests[i].Count = count;
		++i;
	    }
	    ai->UnitTypeRequestsCount = i;
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("upgrade"))) {
	    sublist = gh_car(list);
	    i = 0;
	    if (gh_length(sublist)) {
		ai->UpgradeToRequests = malloc(gh_length(sublist) * sizeof (UnitType *));
	    }
	    while (!gh_null_p(sublist)) {
		char *ident;
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		ident = get_c_string(value);
		ai->UpgradeToRequests[i] = UnitTypeByIdent(ident);
		++i;
	    }
	    ai->UpgradeToRequestsCount = i;
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("research"))) {
	    sublist = gh_car(list);
	    i = 0;
	    if (gh_length(sublist)) {
		ai->ResearchRequests = malloc(gh_length(sublist) * sizeof (Upgrade *));
	    }
	    while (!gh_null_p(sublist)) {
		char *ident;
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		ident = get_c_string(value);
		ai->ResearchRequests[i] = UpgradeByIdent(ident);
		++i;
	    }
	    ai->ResearchRequestsCount = i;
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("building"))) {
	    AiBuildQueue **queue;
	    sublist = gh_car(list);
	    queue = &ai->UnitTypeBuilded;
	    while (!gh_null_p(sublist)) {
		char *ident;
		int made;
		int want;
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		ident = get_c_string(value);
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		made = gh_scm2int(value);
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		want = gh_scm2int(value);
		*queue = malloc(sizeof (AiBuildQueue));
		(*queue)->Next = NULL;
		(*queue)->Type = UnitTypeByIdent(ident);
		(*queue)->Want = want;
		(*queue)->Made = made;
		queue = &(*queue)->Next;
	    }
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("repair-building"))) {
	    ai->LastRepairBuilding = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("repair-workers"))) {
	    sublist = gh_car(list);
	    while (!gh_null_p(sublist)) {
		int num;
		int workers;
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		num = gh_scm2int(value);
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		workers = gh_scm2int(value);
		ai->TriedRepairWorkers[num] = workers;
		++i;
	    }
	    list = gh_cdr(list);
	} else {
	    // FIXME: this leaves a half initialized ai player
	    errl("Unsupported tag", value);
	}
    }
#endif
    return SCM_UNSPECIFIED;
}

/**
**	Register CCL features for unit-type.
*/
global void AiCclRegister(void)
{
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
    gh_new_procedure2_0("ai:force-transfer", CclAiForceTransfert);
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
    gh_new_procedure1_0("ai:get-unittype-force", CclGetUnitTypeForce);

    gh_new_procedure0_0("ai:idle", CclAiIdle);
    gh_new_procedure2_0("ai:timed-wait-force", CclAiTimedWaitForce);
    gh_new_procedure1_0("ai:attack-with-force", CclAiAttackWithForce);
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
}


//@}
