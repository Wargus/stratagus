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
#include "map.h"
#include "pathfinder.h"
#include "ai_local.h"
#include "actions.h"

local int AiMakeUnit(UnitType* type);

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Check if the costs are available for the AI.
**
**	Take reserve and already used resources into account.
**
**	@param costs	Costs for something.
**
**	@return		A bit field of the missing costs.
*/
local int AiCheckCosts(const int* costs)
{
    int i;
    int err;
    const int* resources;
    const int* reserve;
    const int* used;

    err=0;
    resources=AiPlayer->Player->Resources;
    reserve=AiPlayer->Reserve;
    used=AiPlayer->Used;
    for( i=1; i<MaxCosts; ++i ) {
	if( resources[i]<costs[i]-reserve[i]-used[i] ) {
	    err|=1<<i;
	}
    }

    return err;
}

/**
**	Check if the costs for an unit-type are available for the AI.
**
**	Take reserve and already used resources into account.
**
**	@param costs	Costs for something.
**
**	@return		A bit field of the missing costs.
*/
local int AiCheckUnitTypeCosts(const UnitType* type)
{
    return AiCheckCosts(type->Stats[AiPlayer->Player->Player].Costs);
}

/**
**	Check if we can build the building.
**
**	@param type	Unit that can build the building.
**	@param building	Building to be build.
**	@return		True if made, false if can't be made.
**
**	@note 	We must check if the dependencies are fulfilled.
*/
local int AiBuildBuilding(const UnitType* type,UnitType* building)
{
    Unit* table[UnitMax];
    Unit* unit;
    int nunits;
    int i;
    int num;
    int x;
    int y;

    DebugLevel3Fn("%s can made %s\n" _C_ type->Ident _C_ building->Ident);

    IfDebug( unit=NoUnitP; );
    //
    //  Remove all workers on the way building something
    //
    nunits = FindPlayerUnitsByType(AiPlayer->Player,type,table);
    for (num = i = 0; i < nunits; i++) {
	unit = table[i];
	if (unit->Orders[0].Action != UnitActionBuild
		&& (unit->OrderCount==1
		    || unit->Orders[1].Action != UnitActionBuild) ) {
	    table[num++] = unit;
	}
    }

    for( i=0; i<num; ++i ) {

	unit=table[i];
	DebugLevel3Fn("Have an unit to build %d :)\n" _C_ UnitNumber(unit));

	//
	//  Find place on that could be build.
	//
	if ( !AiFindBuildingPlace(unit,building,&x,&y) ) {
	    continue;
	}

	DebugLevel3Fn("Have a building place %d,%d :)\n" _C_ x _C_ y);

	CommandBuildBuilding(unit, x, y, building,FlushCommands);

	return 1;
    }

    return 0;
}

/**
**	Build new units to reduce the food shortage.
*/
local void AiRequestFarms(void)
{
    int i;
    int n;
    int c;
    UnitType* type;
    AiBuildQueue* queue;
    int counter[UnitTypeMax];

    //
    //	Count the already made build requests.
    //
    memset(counter,0,sizeof(counter));
    for( queue=AiPlayer->UnitTypeBuilded; queue; queue=queue->Next ) {
	counter[queue->Type->Type]+=queue->Want;
    }

    //
    //	Check if we can build this?
    //
    n=AiHelpers.UnitLimit[0]->Count;
    for( i=0; i<n; ++i ) {
	type=AiHelpers.UnitLimit[0]->Table[i];
	if( counter[type->Type] ) {	// Already ordered.
	    return;
	}

	DebugLevel3Fn("Must build: %s " _C_ type->Ident);
	//
	//	Check if resources available.
	//
	if( (c=AiCheckUnitTypeCosts(type)) ) {
	    DebugLevel3("- no resources\n");
	    AiPlayer->NeededMask|=c;
	    return;
	} else {
	    DebugLevel3("- enough resources\n");
	    if( AiMakeUnit(type) ) {
		queue=malloc(sizeof(*AiPlayer->UnitTypeBuilded));
		queue->Next=AiPlayer->UnitTypeBuilded;
		queue->Type=type;
		queue->Want=1;
		queue->Made=1;
		AiPlayer->UnitTypeBuilded=queue;
	    }
	}
    }
}

/**
**	Check if we can train the unit.
**
**	@param type	Unit that can train the unit.
**	@param what	What to be trained.
**	@return		True if made, false if can't be made.
**
**	@note 	We must check if the dependencies are fulfilled.
*/
local int AiTrainUnit(const UnitType* type,UnitType* what)
{
    Unit* table[UnitMax];
    Unit* unit;
    int nunits;
    int i;
    int num;

    DebugLevel3Fn("%s can made %s\n" _C_ type->Ident _C_ what->Ident);

    IfDebug( unit=NoUnitP; );
    //
    //  Remove all units already doing something.
    //
    nunits = FindPlayerUnitsByType(AiPlayer->Player,type,table);
    for (num = i = 0; i < nunits; i++) {
	unit = table[i];
	if (unit->Orders[0].Action==UnitActionStill && unit->OrderCount==1 ) {
	    table[num++] = unit;
	}
    }

    for( i=0; i<num; ++i ) {

	unit=table[i];
	DebugLevel3Fn("Have an unit to train %d :)\n" _C_ UnitNumber(unit));

	CommandTrainUnit(unit, what,FlushCommands);

	return 1;
    }

    return 0;
}

/**
**	Check if we can make an unit-type.
**
**	@param type	Unit-type that must be made.
**	@return		True if made, false if can't be made.
**
**	@note 	We must check if the dependencies are fulfilled.
*/
local int AiMakeUnit(UnitType* type)
{
    int i;
    int n;
    const int* unit_count;
    AiUnitTypeTable* const* tablep;
    const AiUnitTypeTable* table;

    //
    //	Check if we have a place for building or an unit to build.
    //
    if( type->Building ) {
	n=AiHelpers.BuildCount;
	tablep=AiHelpers.Build;
    } else {
	n=AiHelpers.TrainCount;
	tablep=AiHelpers.Train;
    }
    if( type->Type>n ) {		// Oops not known.
	DebugLevel0Fn("Nothing known about `%s'\n" _C_ type->Ident);
	return 0;
    }
    table=tablep[type->Type];
    if( !table ) {			// Oops not known.
	DebugLevel0Fn("Nothing known about `%s'\n" _C_ type->Ident);
	return 0;
    }
    i=0;
    n=table->Count;

    unit_count=AiPlayer->Player->UnitTypesCount;
    for( i=0; i<n; ++i ) {
	//
	//	The type is available
	//
	if( unit_count[table->Table[i]->Type] ) {
	    if( type->Building ) {
		if( AiBuildBuilding(table->Table[i],type) ) {
		    return 1;
		}
	    } else {
		if( AiTrainUnit(table->Table[i],type) ) {
		    return 1;
		}
	    }
	}
    }

    return 0;
}

/**
**	Check if we can research the upgrade.
**
**	@param type	Unit that can research the upgrade.
**	@param what	What should be researched.
**	@return		True if made, false if can't be made.
**
**	@note 	We must check if the dependencies are fulfilled.
*/
local int AiResearchUpgrade(const UnitType* type,Upgrade* what)
{
    Unit* table[UnitMax];
    Unit* unit;
    int nunits;
    int i;
    int num;

    DebugLevel0Fn("%s can research %s\n" _C_ type->Ident _C_ what->Ident);

    IfDebug( unit=NoUnitP; );
    //
    //  Remove all units already doing something.
    //
    nunits = FindPlayerUnitsByType(AiPlayer->Player,type,table);
    for (num = i = 0; i < nunits; i++) {
	unit = table[i];
	if (unit->Orders[0].Action==UnitActionStill && unit->OrderCount==1 ) {
	    table[num++] = unit;
	}
    }

    for( i=0; i<num; ++i ) {

	unit=table[i];
	DebugLevel0Fn("Have an unit to research %d :)\n" _C_ UnitNumber(unit));

	CommandResearch(unit, what,FlushCommands);

	return 1;
    }

    return 0;
}

/**
**	Check if the research can be done.
*/
global void AiAddResearchRequest(Upgrade* upgrade)
{
    int i;
    int n;
    const int* unit_count;
    AiUnitTypeTable* const* tablep;
    const AiUnitTypeTable* table;

    //
    //	Check if resources are available.
    //
    if( (i=AiCheckCosts(upgrade->Costs)) ) {
	AiPlayer->NeededMask|=i;
	return;
    }

    //
    //	Check if we have a place for the upgrade to research
    //
    n=AiHelpers.ResearchCount;
    tablep=AiHelpers.Research;

    if( upgrade-Upgrades>n ) {		// Oops not known.
	DebugLevel0Fn("Nothing known about `%s'\n" _C_ upgrade->Ident);
	return;
    }
    table=tablep[upgrade-Upgrades];
    if( !table ) {			// Oops not known.
	DebugLevel0Fn("Nothing known about `%s'\n" _C_ upgrade->Ident);
	return;
    }
    i=0;
    n=table->Count;

    unit_count=AiPlayer->Player->UnitTypesCount;
    for( i=0; i<n; ++i ) {
	//
	//	The type is available
	//
	if( unit_count[table->Table[i]->Type] ) {
	    if( AiResearchUpgrade(table->Table[i],upgrade) ) {
		return;
	    }
	}
    }

    return;
}

/**
**	Check if we can upgrade to unit-type.
**
**	@param type	Unit that can upgrade to unit-type
**	@param what	To what should be upgraded.
**	@return		True if made, false if can't be made.
**
**	@note 	We must check if the dependencies are fulfilled.
*/
local int AiUpgradeTo(const UnitType* type,UnitType* what)
{
    Unit* table[UnitMax];
    Unit* unit;
    int nunits;
    int i;
    int num;

    DebugLevel0Fn("%s can upgrade-to %s\n" _C_ type->Ident _C_ what->Ident);

    IfDebug( unit=NoUnitP; );
    //
    //  Remove all units already doing something.
    //
    nunits = FindPlayerUnitsByType(AiPlayer->Player,type,table);
    for (num = i = 0; i < nunits; i++) {
	unit = table[i];
	if (unit->Orders[0].Action==UnitActionStill && unit->OrderCount==1 ) {
	    table[num++] = unit;
	}
    }

    for( i=0; i<num; ++i ) {

	unit=table[i];
	DebugLevel0Fn("Have an unit to upgrade-to %d :)\n" _C_
		UnitNumber(unit));

	CommandUpgradeTo(unit, what,FlushCommands);

	return 1;
    }

    return 0;
}

/**
**	Check if the upgrade-to can be done.
*/
global void AiAddUpgradeToRequest(UnitType* type)
{
    int i;
    int n;
    const int* unit_count;
    AiUnitTypeTable* const* tablep;
    const AiUnitTypeTable* table;

    //
    //	Check if resources are available.
    //
    if( (i=AiCheckUnitTypeCosts(type)) ) {
	AiPlayer->NeededMask|=i;
	return;
    }

    //
    //	Check if we have a place for the upgrade to.
    //
    n=AiHelpers.UpgradeCount;
    tablep=AiHelpers.Upgrade;

    if( type->Type>n ) {		// Oops not known.
	DebugLevel0Fn("Nothing known about `%s'\n" _C_ type->Ident);
	return;
    }
    table=tablep[type->Type];
    if( !table ) {			// Oops not known.
	DebugLevel0Fn("Nothing known about `%s'\n" _C_ type->Ident);
	return;
    }
    i=0;
    n=table->Count;

    unit_count=AiPlayer->Player->UnitTypesCount;
    for( i=0; i<n; ++i ) {
	//
	//	The type is available
	//
	if( unit_count[table->Table[i]->Type] ) {
	    if( AiUpgradeTo(table->Table[i],type) ) {
		return;
	    }
	}
    }

    return;
}

/**
**	Check what must be builded / trained.
*/
local void AiCheckingWork(void)
{
    int c;
    UnitType* type;
    AiBuildQueue* queue;

    DebugLevel2Fn("%d:%d %d %d\n" _C_ AiPlayer->Player->Player _C_
	    AiPlayer->Player->Resources[1] _C_
	    AiPlayer->Player->Resources[2] _C_
	    AiPlayer->Player->Resources[3]);

    if( AiPlayer->NeedFood ) {		// Food has the highest priority
	AiPlayer->NeedFood=0;
	AiRequestFarms();
    }

    //
    //	Look to the build requests, what can be done.
    //
    for( queue=AiPlayer->UnitTypeBuilded; queue; queue=queue->Next ) {
	if( queue->Want>queue->Made ) {
	    type=queue->Type;
	    DebugLevel3Fn("Must build: %s " _C_ type->Ident);

	    //
	    //	Check if we have enough food.
	    //
	    if( !type->Building && AiPlayer->Player->Food
			<=AiPlayer->Player->NumFoodUnits ) {
		DebugLevel3Fn("Need food\n");
		AiPlayer->NeedFood=1;
		AiRequestFarms();
		return;
	    }

	    //
	    //	Check if resources available.
	    //
	    if( (c=AiCheckUnitTypeCosts(type)) ) {
		DebugLevel3("- no resources\n");
		AiPlayer->NeededMask|=c;
		return;
	    } else {
		DebugLevel3("- enough resources\n");
		if( AiMakeUnit(type) ) {
		    ++queue->Made;
		}
	    }
	}
    }
}

/*----------------------------------------------------------------------------
--      WORKERS/RESOURCES
----------------------------------------------------------------------------*/

/**
**	Find the nearest gold mine for unit from x,y.
**
**	@param unit	Pointer for source unit.
**	@param x	X tile position to start.
**	@param y	Y tile position to start.
**
**	@return		Pointer to the nearest reachable gold mine.
**
**	@see FindGoldMine but this version uses a reachable gold-mine.
**
**	@note	This function is very expensive, reduce the calls to it.
**		Also we could sort the gold mines by the map distance and
**		try the nearest first.
**		Or we could do a flood fill search starting from x.y. The
**		first found gold mine is it.
*/
global Unit* AiFindGoldMine(const Unit* source,int x,int y)
{
    Unit** table;
    Unit* unit;
    Unit* best;
    int best_d;
    int d;

    best=NoUnitP;
    best_d=99999;
    for( table=Units; table<Units+NumUnits; table++ ) {
	unit=*table;
	// Want gold-mine and not dieing.
	if( !unit->Type->GoldMine || UnitUnusable(unit) ) {
	    continue;
	}
	
	//
	//	If we have already a shorter way, than the distance to the
	//	unit, we didn't need to check, if reachable.
	//
	if( MapDistanceBetweenUnits(source,unit)<best_d
		&& (d=UnitReachable(source,unit,1)) && d<best_d ) {
	    best_d=d;
	    best=unit;
	}
    }
    DebugLevel3Fn("%d %d,%d\n",UnitNumber(best),best->X,best->Y);

    return best;
}

/**
**      Assign worker to mine gold.
**
**      IDEA: If no way to goldmine, we must dig the way.
**      IDEA: If goldmine is on an other island, we must transport the workers.
**
**	@note	I can cache the gold-mine.
*/
local int AiMineGold(Unit * unit)
{
    Unit *dest;

    DebugLevel3Fn("%d\n", UnitNumber(unit));
    dest = AiFindGoldMine(unit, unit->X, unit->Y);
    if (!dest) {
	DebugLevel0Fn("goldmine not reachable\n");
	return 0;
    }
    DebugCheck(unit->Type!=UnitTypeHumanWorker && unit->Type!=UnitTypeOrcWorker);
    CommandMineGold(unit, dest,FlushCommands);

    return 1;
}

/**
**      Assign worker to harvest.
*/
local int AiHarvest(Unit * unit)
{
    int x, y, addx, addy, i, n, r, wx, wy, bestx, besty, cost;
    Unit *dest;

    DebugLevel3Fn("%d\n", UnitNumber(unit));
    x = unit->X;
    y = unit->Y;
    addx = unit->Type->TileWidth;
    addy = unit->Type->TileHeight;
    r = TheMap.Width;
    if (r < TheMap.Height) {
	r = TheMap.Height;
    }

    //  This is correct, but can this be written faster???
    if ((dest = FindWoodDeposit(unit->Player, x, y))) {
	NearestOfUnit(dest, x, y, &wx, &wy);
	DebugLevel3("To %d,%d\n", wx, wy);
    } else {
	wx = unit->X;
	wy = unit->Y;
    }
    cost = 99999;
    IfDebug(bestx = besty = 0; );	// keep the compiler happy

    // FIXME: if we reach the map borders we can go fast up, left, ...
    --x;
    while (addx <= r && addy <= r) {
	for (i = addy; i--; y++) {	// go down
	    if (CheckedForestOnMap(x, y)) {
		n = max(abs(wx - x), abs(wy - y));
		DebugLevel3("Distance %d,%d %d\n", x, y, n);
		if (n < cost && PlaceReachable(unit, x-1, y-1, 3)) {
		    cost = n;
		    bestx = x;
		    besty = y;
		}
	    }
	}
	++addx;
	for (i = addx; i--; x++) {	// go right
	    if (CheckedForestOnMap(x, y)) {
		n = max(abs(wx - x), abs(wy - y));
		DebugLevel3("Distance %d,%d %d\n", x, y, n);
		if (n < cost && PlaceReachable(unit, x-1, y-1, 3)) {
		    cost = n;
		    bestx = x;
		    besty = y;
		}
	    }
	}
	++addy;
	for (i = addy; i--; y--) {	// go up
	    if (CheckedForestOnMap(x, y)) {
		n = max(abs(wx - x), abs(wy - y));
		DebugLevel3("Distance %d,%d %d\n", x, y, n);
		if (n < cost && PlaceReachable(unit, x-1, y-1, 3)) {
		    cost = n;
		    bestx = x;
		    besty = y;
		}
	    }
	}
	++addx;
	for (i = addx; i--; x--) {	// go left
	    if (CheckedForestOnMap(x, y)) {
		n = max(abs(wx - x), abs(wy - y));
		DebugLevel3("Distance %d,%d %d\n", x, y, n);
		if (n < cost && PlaceReachable(unit, x-1, y-1, 3)) {
		    cost = n;
		    bestx = x;
		    besty = y;
		}
	    }
	}
	if (cost != 99999) {
	    DebugLevel3Fn("wood on %d,%d\n", x, y);
	    DebugCheck(unit->Type!=UnitTypeHumanWorker && unit->Type!=UnitTypeOrcWorker);
	    CommandHarvest(unit, bestx, besty,FlushCommands);
	    return 1;
	}
	++addy;
    }
    DebugLevel0Fn("no wood reachable\n");
    return 0;
}

/**
**      Assign worker to haul oil.
*/
local int AiHaulOil(Unit * unit)
{
    Unit *dest;

    DebugLevel3Fn("%d\n", UnitNumber(unit));
    dest = FindOilPlatform(unit->Player, unit->X, unit->Y);
    if (!dest) {
	DebugLevel0Fn("oil platform not reachable\n");
	return 0;
    }
    DebugCheck(unit->Type!=UnitTypeHumanTanker && unit->Type!=UnitTypeOrcTanker);
    CommandHaulOil(unit, dest,FlushCommands);

    return 1;
}

/**
**	Assign workers to collect resources.
**
**	If we have a shortage of a resource, let many workers collecting this.
**	If no shortage, split workers to all resources.
*/
local void AiCollectResources(void)
{
    int c;
    int i;
    int n;
    UnitType** types;
    Unit* table[UnitMax];
    int nunits;

    DebugLevel3Fn("Needed resources %x\n" _C_ AiPlayer->NeededMask);

    //
    //	Look through all costs, if needed.
    //
    for( c=0; c<OreCost; ++c ) {
	if( c>=AiHelpers.CollectCount || !AiHelpers.Collect[c]
		|| !(AiPlayer->NeededMask&(1<<c)) ) {
	    continue;
	}

	//
	//	Get all workers that can collect this resource.
	//
	types=AiHelpers.Collect[c]->Table;
	n=AiHelpers.Collect[c]->Count;
	nunits=0;
	for( i=0; i<n; ++i ) {
	    nunits += FindPlayerUnitsByType(AiPlayer->Player,
		    types[i],table+nunits);
	}
	DebugLevel3Fn("%s: units %d\n" _C_ DEFAULT_NAMES[c] _C_ nunits);

	//
	//	Assign the worker
	//
	for( i=0; i<nunits; ++i ) {
	    // Unit is already *very* busy
	    if (table[i]->Orders[0].Action != UnitActionBuild
		    && table[i]->OrderCount==1 ) {
		switch( c ) {
		    case GoldCost:
			if (table[i]->Orders[0].Action != UnitActionMineGold ) {
			    AiMineGold(table[i]);
			}
			break;
		    case WoodCost:
			if (table[i]->Orders[0].Action != UnitActionHarvest ) {
			    AiHarvest(table[i]);
			}
			break;

		    case OilCost:
			if (table[i]->Orders[0].Action != UnitActionHaulOil ) {
			    AiHaulOil(table[i]);
			}
			break;
		    default:
			DebugCheck( 1 );
		}
	    }
	}
    }

    //
    //	Assign the remaining unit.
    //
    for( c=0; c<OreCost; ++c ) {
	if( c>=AiHelpers.CollectCount || !AiHelpers.Collect[c] ) {
	    continue;
	}

	//
	//	Get all workers that can collect this resource.
	//
	types=AiHelpers.Collect[c]->Table;
	n=AiHelpers.Collect[c]->Count;
	nunits=0;
	for( i=0; i<n; ++i ) {
	    nunits += FindPlayerUnitsByType(AiPlayer->Player,
		    types[i],table+nunits);
	}
	DebugLevel3Fn("%s: units %d\n" _C_ DEFAULT_NAMES[c] _C_ nunits);

	//
	//	Assign the worker
	//
	for( i=0; i<nunits; ++i ) {
	    // Unit is already busy
	    if (table[i]->Orders[0].Action != UnitActionStill
		    || table[i]->OrderCount>1 ) {
		continue;
	    }
	    switch( c ) {
		case GoldCost:
		    AiMineGold(table[i]);
		    break;
		case WoodCost:
		    AiHarvest(table[i]);
		    break;
		case OilCost:
		    AiHaulOil(table[i]);
		    break;
		default:
		    DebugCheck( 1 );
	    }
	}
    }

    //
    //	Let all workers with resource return it.
    //
    nunits=0;
    for( c=0; c<OreCost; ++c ) {
	if( c>=AiHelpers.WithGoodsCount || !AiHelpers.WithGoods[c] ) {
	    continue;
	}
	types=AiHelpers.WithGoods[c]->Table;
	n=AiHelpers.WithGoods[c]->Count;
	for( i=0; i<n; ++i ) {
	    nunits+=FindPlayerUnitsByType(AiPlayer->Player,
		    types[i],table+nunits);
	}
    }
    DebugLevel3Fn("Return: units %d\n" _C_ nunits);

    //
    //	Assign the workers with goods
    //
    for( i=0; i<nunits; ++i ) {
	// Unit is already busy
	if (table[i]->Orders[0].Action != UnitActionStill
		|| table[i]->OrderCount>1 ) {
	    continue;
	}
	CommandReturnGoods(table[i],NULL,FlushCommands);
    }
}

/**
**	Add unit-type request to resource manager.
**
**	@param type	Unit type requested.
**	@param count	How many units.
*/
global void AiAddUnitTypeRequest(UnitType* type,int count)
{
    AiBuildQueue** queue;

    DebugLevel0Fn("%s %d\n" _C_ type->Ident _C_ count);

    //
    //	Find end of the list.
    //
    for( queue=&AiPlayer->UnitTypeBuilded; *queue; queue=&(*queue)->Next ) {
    }

    *queue=malloc(sizeof(*AiPlayer->UnitTypeBuilded));
    (*queue)->Next=NULL;
    (*queue)->Type=type;
    (*queue)->Want=count;
    (*queue)->Made=0;
}

/**
**	Entry point of resource manager, perodic called.
*/
global void AiResourceManager(void)
{
    //
    //	Check if something needs to be build / trained.
    //
    AiCheckingWork();
    //
    //	Collect resources.
    //
    AiCollectResources();

    AiPlayer->NeededMask=0;
}

//@}

#endif // } NEW_AI
