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
**      Find free building place.
**
**	@param worker	Worker to build building.
**	@param type	Type of building.
**	@param dx	Pointer for X position returned.
**	@param dy	Pointer for Y position returned.
**	@return		True if place found, false if no found.
*/
global int AiFindBuildingPlace(const Unit * worker, const UnitType * type,
	int *dx, int *dy)
{
    int wx, wy, x, y, addx, addy;
    int end, state;

    wx = worker->X;
    wy = worker->Y;
    x = wx;
    y = wy;
    addx = 1;
    addy = 1;

    state = 0;
    end = y + addy - 1;
    for (;;) {				// test rectangles arround the place
	switch (state) {
	case 0:
	    if (y++ == end) {
		++state;
		end = x + addx++;
	    }
	    break;
	case 1:
	    if (x++ == end) {
		++state;
		end = y - addy++;
	    }
	    break;
	case 2:
	    if (y-- == end) {
		++state;
		end = x - addx++;
	    }
	    break;
	case 3:
	    if (x-- == end) {
		state = 0;
		end = y + addy++;
	    }
	    break;
	}

	// FIXME: this check outside the map could be speeded up.
	if (y < 0 || x < 0 || y >= TheMap.Height || x >= TheMap.Width) {
	    continue;
	}
	if (CanBuildUnitType(worker, type, x, y) 
		&& PlaceReachable(worker, x, y, 1)) {
	    *dx=x;
	    *dy=y;
	    return 1;
	}
    }
    return 0;
}

/**
**	Check if we can build the building.
**
**	@param type	Unit that can build the building.
**	@param building	Building to be build.
**	@return		True if made, false if can't be made.
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

    DebugLevel0Fn("%s can made %s\n" _C_ type->Ident _C_ building->Ident);

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

    if( !num ) {			// No available unit.
	return 0;
    }

    DebugLevel0Fn("Have an unit to build :)\n");

    //
    //  Find place on that could be build.
    //
    if ( !AiFindBuildingPlace(unit,building,&x,&y) ) {
	return 0;
    }

    DebugLevel0Fn("Have a building place :)\n");

    CommandBuildBuilding(unit, x, y, building,FlushCommands);

    return 1;
}

/**
**	Check if we can make an unit-type.
**
**	@param type	Unit-type that must be made.
**	@return		True if made, false if can't be made.
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
	    if( AiBuildBuilding(table->Table[i],type) ) {
		return 1;
	    }
	}
    }

    return 0;
}

/**
**	Check what must be builded / trained.
*/
local void AiCheckingWork(void)
{
    int i;
    int n;
    int c;
    UnitType* type;

    n=AiPlayer->BuildedCount;
    for( i=0; i<n; ++i ) {
	if( AiPlayer->UnitTypeBuilded[i].Want
		>AiPlayer->UnitTypeBuilded[i].Made ) {
	    type=AiPlayer->UnitTypeBuilded[i].Type;
	    DebugLevel0Fn("Must build: %s " _C_ type->Ident);
	    //
	    //	Check if resources available.
	    //
	    if( (c=AiCheckUnitTypeCosts(type)) ) {
		DebugLevel0("- no resources\n");
		AiPlayer->NeededMask=c;
		return;
	    } else {
		DebugLevel0("- enough resources\n");
		if( AiMakeUnit(type) ) {
		    ++AiPlayer->UnitTypeBuilded[i].Made;
		}
	    }
	}
    }
}

/*----------------------------------------------------------------------------
--      WORKERS/RESOURCES
----------------------------------------------------------------------------*/

/**
**      Assign worker to mine gold.
**
**      IDEA: If no way to goldmine, we must dig the way.
**      IDEA: If goldmine is on an other island, we must transport the workers.
*/
local void AiMineGold(Unit * unit)
{
    Unit *dest;

    DebugLevel3Fn("%d\n", UnitNumber(unit));
    dest = FindGoldMine(unit, unit->X, unit->Y);
    if (!dest) {
	DebugLevel0Fn("goldmine not reachable\n");
	return;
    }
    CommandMineGold(unit, dest,FlushCommands);
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
		if (n < cost && PlaceReachable(unit, x, y, 1)) {
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
		if (n < cost && PlaceReachable(unit, x, y, 1)) {
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
		if (n < cost && PlaceReachable(unit, x, y, 1)) {
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
		if (n < cost && PlaceReachable(unit, x, y, 1)) {
		    cost = n;
		    bestx = x;
		    besty = y;
		}
	    }
	}
	if (cost != 99999) {
	    DebugLevel3Fn("wood on %d,%d\n", x, y);
	    CommandHarvest(unit, bestx, besty,FlushCommands);
	    return 1;
	}
	++addy;
    }
    DebugLevel0Fn("no wood reachable\n");
    return 0;
}

/**
**	Assign workers to collect resources.
*/
local void AiCollectResources(void)
{
    int c;
    int i;
    int n;
    UnitType** types;
    Unit* table[UnitMax];
    int nunits;

    DebugLevel0Fn("%x\n",AiPlayer->NeededMask);

    //
    //	Look through all costs, if needed.
    //
    for( c=0; c<OreCost; ++c ) {
	if( c>=AiHelpers.CollectCount || !AiHelpers.Collect[c]
		|| !(AiPlayer->NeededMask&(1<<c)) ) {
	    continue;
	}
	types=AiHelpers.Collect[c]->Table;
	n=AiHelpers.Collect[c]->Count;
	nunits=0;
	for( i=0; i<n; ++i ) {
	    nunits += FindPlayerUnitsByType(AiPlayer->Player,
		    types[i],table+nunits);
	}
	DebugLevel0Fn("%s: units %d\n",DEFAULT_NAMES[c],nunits);

	//
	//	Assign the worker
	//
	for( i=0; i<nunits; ++i ) {
	    // Unit is already *very* busy
	    if (table[i]->Orders[0].Action != UnitActionBuild
		    && (table[i]->OrderCount==1
			|| table[i]->Orders[1].Action != UnitActionBuild) ) {
		switch( c ) {
		    case 1:
			if (table[i]->Orders[0].Action != UnitActionMineGold ) {
			    AiMineGold(table[i]);
			}
			break;
		    case 2:
			if (table[i]->Orders[0].Action != UnitActionHarvest ) {
			    AiHarvest(table[i]);
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
	types=AiHelpers.Collect[c]->Table;
	n=AiHelpers.Collect[c]->Count;
	nunits=0;
	for( i=0; i<n; ++i ) {
	    nunits += FindPlayerUnitsByType(AiPlayer->Player,
		    types[i],table+nunits);
	}
	DebugLevel0Fn("%s: units %d\n",DEFAULT_NAMES[c],nunits);

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
		case 1:
		    AiMineGold(table[i]);
		    break;
		case 2:
		    AiHarvest(table[i]);
		    break;
		default:
		    DebugCheck( 1 );
	    }
	}
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
    int n;

    DebugLevel0Fn("%s %d\n" _C_ type->Ident _C_ count);
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
    //
    //	Check if something needs to be build / trained.
    //
    AiCheckingWork();
    //
    //	Collect resources.
    //
    AiCollectResources();
}

//@}

#endif // } NEW_AI
