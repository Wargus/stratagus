//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name ai_resource.c	-	AI resource manager. */
//
//      (c) Copyright 2000-2002 by Lutz Sammer and Antonis Chaniotis.
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

#ifdef NEW_AI	// {

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"

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
    int j;
    int k;
    int err;
    const int* resources;
    const int* reserve;
    int* used;
    int nunits;
    Unit** units;
    const int* building_costs;


    // FIXME: the used costs shouldn't be calculated here
    used=AiPlayer->Used;
    for( j=1; j<MaxCosts; ++j ) {
	used[j]=0;
    }

    nunits=AiPlayer->Player->TotalNumUnits;
    units=AiPlayer->Player->Units;
    for( i=0; i<nunits; ++i ) {
	for( k=0; k<units[i]->OrderCount; ++k ) {
	    if( units[i]->Orders[k].Action==UnitActionBuild ) {
		building_costs=units[i]->Orders[k].Type->Stats[AiPlayer->Player->Player].Costs;
		for( j=1; j<MaxCosts; ++j ) {
		    used[j]+=building_costs[j];
		}
	    }
	}
    }


    err=0;
    resources=AiPlayer->Player->Resources;
    reserve=AiPlayer->Reserve;
    for( i=1; i<MaxCosts; ++i ) {
	if( resources[i]-used[i]<costs[i]-reserve[i] ) {
	    err|=1<<i;
	}
    }

    return err;
}

/**
**	Check if the AI player needs food.
**
**	It counts buildings in progress and units in training queues.
**
**	@param pai	AI player.
**	@param type	Unit-type that should be build.
**	@return		True if enought, false otherwise.
**
**	@todo	The number of food currently trained can be stored global
**		for faster use.
*/
local int AiCheckFood(const PlayerAi* pai,const UnitType* type)
{
    int remaining;
    const AiBuildQueue* queue;

    //
    //	Count food supplies under construction.
    //
    remaining=0;
    for( queue=pai->UnitTypeBuilded; queue; queue=queue->Next ) {
	if( queue->Type->Supply ) {
	    DebugLevel3Fn("Builded %d remain %d\n"
		_C_ queue->Made _C_ remaining);
	    remaining+=queue->Made*queue->Type->Supply;
	}
    }
    DebugLevel3Fn("Remain %d\n" _C_ remaining);
    //
    //	We are already out of food.
    //
    remaining+=pai->Player->Food-pai->Player->NumFoodUnits-type->Demand;
    DebugLevel3Fn("-Demand %d\n" _C_ remaining);
    if( remaining<0 ) {
	return 0;
    }

    //
    //	Count what we train.
    //
    for( queue=pai->UnitTypeBuilded; queue; queue=queue->Next ) {
	if( !queue->Type->Building ) {
	    DebugLevel3Fn("Trained %d remain %d\n"
		_C_ queue->Made _C_ remaining);
	    if( (remaining-=queue->Made*queue->Type->Demand)<0 ) {
		return 0;
	    }
	}
    }
    return 1;
}

/**
**	Check if the costs for an unit-type are available for the AI.
**
**	Take reserve and already used resources into account.
**
**	@param type	Unit-type to check the costs for.
**
**	@return		A bit field of the missing costs.
*/
local int AiCheckUnitTypeCosts(const UnitType* type)
{
    return AiCheckCosts(type->Stats[AiPlayer->Player->Player].Costs);
}

/**
**	Enemy units in distance.
**
**	@param unit	Find in distance for this unit.
**	@param range	Distance range to look.
**
**	@return		Number of enemy units.
*/
global int EnemyUnitsInDistance(const Unit* unit,unsigned range)
{
    const Unit* dest;
    const UnitType* type;
    Unit* table[UnitMax];
    unsigned x;
    unsigned y;
    unsigned n;
    unsigned i;
    int e;
    const Player* player;

    DebugLevel3Fn("(%d)%s\n" _C_ UnitNumber(unit) _C_ unit->Type->Ident);

    //
    //	Select all units in range.
    //
    x=unit->X;
    y=unit->Y;
    n=SelectUnits(x-range,y-range,x+range+1,y+range+1,table);

    player=unit->Player;
    type=unit->Type;

    //
    //	Find the enemy units which can attack
    //
    for( e=i=0; i<n; ++i ) {
	dest=table[i];
	//
	//	unusable unit
	//
	// FIXME: did SelectUnits already filter this.
	if( dest->Removed || dest->Invisible || !unit->HP
		|| !(dest->Visible&(1<<player->Player))
		|| dest->Orders[0].Action==UnitActionDie ) {
	    DebugLevel0Fn("NO\n");
	    continue;
	}

	if( !IsEnemy(player,dest) ) {	// a friend or neutral
	    continue;
	}

	//
	//	Unit can attack back?
	//
	if( CanTarget(dest->Type,type) ) {
	    ++e;
	}
    }

    return e;
}

/**
**	Check if we can build the building.
**
**	@param type	Unit that can build the building.
**	@param building	Building to be build.
**	@return		True if made, false if can't be made.
**
**	@note	We must check if the dependencies are fulfilled.
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
**	@note	We must check if the dependencies are fulfilled.
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
**	@note	We must check if the dependencies are fulfilled.
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
    n=table->Count;

    unit_count=AiPlayer->Player->UnitTypesCount;
    for( i=0; i<n; ++i ) {
	//
	//	The type for builder/trainer is available
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
**	@note	We must check if the dependencies are fulfilled.
*/
local int AiResearchUpgrade(const UnitType* type,Upgrade* what)
{
    Unit* table[UnitMax];
    Unit* unit;
    int nunits;
    int i;
    int num;

    DebugLevel3Fn("%s can research %s\n" _C_ type->Ident _C_ what->Ident);

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
	DebugLevel3Fn("Have an unit to research %d :)\n" _C_ UnitNumber(unit));

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
**	@note	We must check if the dependencies are fulfilled.
*/
local int AiUpgradeTo(const UnitType* type,UnitType* what)
{
    Unit* table[UnitMax];
    Unit* unit;
    int nunits;
    int i;
    int num;

    DebugLevel3Fn("%s can upgrade-to %s\n" _C_ type->Ident _C_ what->Ident);

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
	DebugLevel3Fn("Have an unit to upgrade-to %d :)\n" _C_
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

    DebugLevel3Fn("%d:%d %d %d\n" _C_ AiPlayer->Player->Player _C_
	    AiPlayer->Player->Resources[1] _C_
	    AiPlayer->Player->Resources[2] _C_
	    AiPlayer->Player->Resources[3]);

    // Food has the highest priority
    if( AiPlayer->NeedFood ) {
	if( !(AiPlayer->UnitTypeBuilded && AiPlayer->UnitTypeBuilded->Type->Supply) ) {
	    AiPlayer->NeedFood=0;
	    AiRequestFarms();
	}
    }

    //
    //	Look to the build requests, what can be done.
    //
    for( queue=AiPlayer->UnitTypeBuilded; queue; queue=queue->Next ) {
	if( queue->Want>queue->Made ) {
	    type=queue->Type;
	    DebugLevel3Fn("Must build: %s " _C_ type->Ident);

	    //
	    //	FIXME: must check if requirements are fulfilled.
	    //		Buildings can be destructed.

	    //
	    //	Check limits, AI should be broken if reached.
	    //
	    if( !PlayerCheckLimits(AiPlayer->Player,type) ) {
		DebugLevel2Fn("Unit limits reached\n");
		continue;
	    }

	    //
	    //	Check if we have enough food.
	    //
	    if( !type->Building ) {
		// Count future
		if(  !AiCheckFood(AiPlayer,type) ) {
		    DebugLevel3Fn("Need food\n");
		    AiPlayer->NeedFood=1;
		    AiRequestFarms();
		}
		// Current limit
		if( !PlayerCheckFood(AiPlayer->Player,type) ) {
		    continue;
		}
	    }

	    //
	    //	Check if resources available.
	    //
	    if( (c=AiCheckUnitTypeCosts(type)) ) {
		DebugLevel3("- no resources\n");
		AiPlayer->NeededMask|=c;
		//
		//	NOTE: we can continue and build things with lesser
		//		resource or other resource need!
		continue;
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
**	Find the nearest gold mine for unit.
**
**	@param unit	Pointer for source unit.
**
**	@return		Pointer to the nearest reachable gold mine.
**
**	@see FindGoldMine but this version doesn't check for explored tiles.
*/
local Unit* AiFindGoldMine(const Unit* unit)
{
    static const int xoffset[]={  0,-1,+1, 0, -1,+1,-1,+1 };
    static const int yoffset[]={ -1, 0, 0,+1, -1,-1,+1,+1 };
    struct {
	unsigned short X;
	unsigned short Y;
    } * points;
    int size;
    int x;
    int y;
    int rx;
    int ry;
    int mask;
    int wp;
    int rp;
    int ep;
    int i;
    int w;
    int n;
    unsigned char* m;
    unsigned char* matrix;
    const Unit* destu;
    Unit* mine;
    Unit* bestmine;
    int destx;
    int desty;
    int bestx;
    int besty;
    int bestd;

    destx=x=unit->X;
    desty=y=unit->Y;
    size=TheMap.Width*TheMap.Height/4;
    points=malloc(size*sizeof(*points));

    //
    //	Find the nearest gold depot
    //
    if( (destu=FindDeposit(unit->Player,x,y,GoldCost)) ) {
	NearestOfUnit(destu,x,y,&destx,&desty);
    }
    bestd=99999;
    IfDebug( bestx=besty=0; );		// keep the compiler happy

    //
    //	Make movement matrix. FIXME: can create smaller matrix.
    //
    matrix=CreateMatrix();
    w=TheMap.Width+2;
    matrix+=w+w+2;

    //
    //	Mark sight range as border. FIXME: matrix didn't need to be bigger.
    //
    n=unit->Stats->SightRange;
    rx=x-n;
    if( rx<0 ) {
	rx=0;
    }
    ep=x+n;
    if( ep>TheMap.Width ) {
	ep=TheMap.Width;
    }
    ry=y-n;
    if( ry<0 ) {
	ry=0;
    }
    wp=y+n;
    if( wp>TheMap.Height ) {
	wp=TheMap.Height;
    }
    for( i=rx; i<ep; ++i ) {		// top bottom line
	matrix[i+ry*w]=matrix[i+wp*w]=66;
    }
    for( i=ry+1; i<wp-1; ++i ) {
	matrix[rx+i*w]=matrix[ep+i*w]=66;
    }

#if 0
    matrix[x+n+(y+n)*w]=matrix[x-n+(y+n)*w]=
	matrix[x+n+(y-n)*w]=matrix[x-n+(y-n)*w]=66;
    for( i=n; i--; ) {
	// FIXME: marks out of map area
	DebugCheck( x-i+(y-n)*w<0 || x+i+(y+n)*w>w*TheMap.Hight );
	matrix[x+n+(y+i)*w]=matrix[x-n+(y+i)*w]=
	    matrix[x+n+(y-i)*w]=matrix[x-n+(y-i)*w]=
	    matrix[x-i+(y+n)*w]=matrix[x+i+(y+n)*w]=
	    matrix[x-i+(y-n)*w]=matrix[x+i+(y-n)*w]=66;
    }
#endif

    mask=UnitMovementMask(unit);

    points[0].X=x;
    points[0].Y=y;
    rp=0;
    matrix[x+y*w]=1;			// mark start point
    ep=wp=1;				// start with one point
    bestmine=NoUnitP;

    //
    //	Pop a point from stack, push all neighbors which could be entered.
    //
    for( ;; ) {
	while( rp!=ep ) {
	    rx=points[rp].X;
	    ry=points[rp].Y;
	    for( i=0; i<8; ++i ) {		// mark all neighbors
		x=rx+xoffset[i];
		y=ry+yoffset[i];
		m=matrix+x+y*w;
		if( *m ) {			// already checked
		    continue;
		}

		//
		//	Look if there is a mine
		//
		if ( (mine=GoldMineOnMap(x,y)) ) {
		    if( destu ) {
			n=max(abs(destx-x),abs(desty-y));
			if( n<bestd ) {
			    bestd=n;
			    bestx=x;
			    besty=y;
			    bestmine=mine;
			}
			*m=22;
		    } else {			// no goal take the first
			free(points);
			return mine;
		    }
		}

		if( CanMoveToMask(x,y,mask) ) {	// reachable
		    *m=1;
		    points[wp].X=x;		// push the point
		    points[wp].Y=y;
		    if( ++wp>=size ) {		// round about
			wp=0;
		    }
		} else {			// unreachable
		    *m=99;
		}
	    }
	    if( ++rp>=size ) {			// round about
		rp=0;
	    }
	}

	//
	//	Take best of this frame, if any.
	//
	if( bestd!=99999 ) {
	    free(points);
	    return bestmine;
	}

	//
	//	Continue with next frame.
	//
	if( rp==wp ) {			// unreachable, no more points available
	    break;
	}
	ep=wp;
    }

    DebugLevel3Fn("no mine in sight-range\n");

    free(points);
    return NoUnitP;
}

/**
**      Assign worker to mine gold.
**
**	@todo FIXME: JOHNS: docu What does used_mine do, for what is it good?
**
**      IDEA: If no way to goldmine, we must dig the way.
**      IDEA: If goldmine is on an other island, we must transport the workers.
*/
local int AiMineGold(Unit* unit)
{
    Unit* dest;

    DebugLevel3Fn("%d\n" _C_ UnitNumber(unit));
    dest = AiFindGoldMine(unit);
    if (!dest) {
	DebugLevel0Fn("goldmine not reachable by %s(%d,%d)\n"
		_C_ unit->Type->Ident _C_ unit->X _C_ unit->Y);
	return 0;
    }
    DebugCheck(unit->Type != UnitTypeHumanWorker
	    && unit->Type != UnitTypeOrcWorker);

    CommandMineGold(unit, dest, FlushCommands);

    return 1;
}

#if 0

/**
**      Assign worker to harvest.
**
**	@param unit	Find wood for this worker.
*/
local int AiHarvest(Unit * unit)
{
    int x, y, addx, addy, i, n, r, wx, wy, bestx, besty, cost;
    Unit *dest;

    DebugLevel3Fn("%d\n" _C_ UnitNumber(unit));
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
	DebugLevel3("To %d,%d\n" _C_ wx _C_ wy);
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
		DebugLevel3("Distance %d,%d %d\n" _C_ x _C_ y _C_ n);
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
		DebugLevel3("Distance %d,%d %d\n" _C_ x _C_ y _C_ n);
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
		DebugLevel3("Distance %d,%d %d\n" _C_ x _C_ y _C_ n);
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
		DebugLevel3("Distance %d,%d %d\n" _C_ x _C_ y _C_ n);
		if (n < cost && PlaceReachable(unit, x-1, y-1, 3)) {
		    cost = n;
		    bestx = x;
		    besty = y;
		}
	    }
	}
	if (cost != 99999) {
	    DebugLevel3Fn("wood on %d,%d\n" _C_ x _C_ y);
	    DebugCheck(unit->Type!=UnitTypeHumanWorker && unit->Type!=UnitTypeOrcWorker);
	    CommandHarvest(unit, bestx, besty,FlushCommands);
	    return 1;
	}
	++addy;
    }

    DebugLevel0Fn("no wood reachable by %s(%d,%d)\n");
	    _C_ unit->Type->Ident _C_ unit->X _C_ unit->Y);
    return 0;
}

#else

/**
**      Assign worker to harvest.
**
**	@param unit	Find wood for this worker.
**
**	@return		True if the work is going harvest.
*/
local int AiHarvest(Unit * unit)
{
    static const int xoffset[]={  0,-1,+1, 0, -1,+1,-1,+1 };
    static const int yoffset[]={ -1, 0, 0,+1, -1,-1,+1,+1 };
    struct {
	unsigned short X;
	unsigned short Y;
    } * points;
    int size;
    int x;
    int y;
    int rx;
    int ry;
    int mask;
    int wp;
    int rp;
    int ep;
    int i;
    int w;
    int n;
    unsigned char* m;
    unsigned char* matrix;
    const Unit* destu;
    int destx;
    int desty;
    int bestx;
    int besty;
    int bestd;

    x=unit->X;
    y=unit->Y;
    size=TheMap.Width*TheMap.Height/4;
    points=malloc(size*sizeof(*points));
	
    //
    //	Find the nearest wood depot
    //
    if( (destu=FindDeposit(unit->Player,x,y,WoodCost)) ) {
	NearestOfUnit(destu,x,y,&destx,&desty);
    }
    bestd=99999;
    bestx=besty=0;		// keep the compiler happy

    //
    //	Make movement matrix.
    //
    matrix=CreateMatrix();
    w=TheMap.Width+2;
    matrix+=w+w+2;

    points[0].X=x;
    points[0].Y=y;
    rp=0;
    matrix[x+y*w]=1;			// mark start point
    ep=wp=1;				// start with one point

    mask=UnitMovementMask(unit);

    //
    //	Pop a point from stack, push all neightbors which could be entered.
    //
    for( ;; ) {
	while( rp!=ep ) {
	    rx=points[rp].X;
	    ry=points[rp].Y;
	    for( i=0; i<8; ++i ) {		// mark all neighbors
		x=rx+xoffset[i];
		y=ry+yoffset[i];
		m=matrix+x+y*w;
		if( *m ) {			// already checked
		    continue;
		}

		//
		//	Look if there is wood
		//
		if ( CheckedForestOnMap(x,y) ) {
		    if( destu ) {
			n=max(abs(destx-x),abs(desty-y));
			if( n<bestd ) {
			    bestd=n;
			    bestx=x;
			    besty=y;
			}
			*m=22;
		    } else {			// no goal take the first
			DebugCheck(unit->Type!=UnitTypeHumanWorker
				&& unit->Type!=UnitTypeOrcWorker);
			CommandHarvest(unit,x,y,FlushCommands);
			free(points);
			return 1;
		    }
		}

		if( CanMoveToMask(x,y,mask) ) {	// reachable
		    *m=1;
		    points[wp].X=x;		// push the point
		    points[wp].Y=y;
		    if( ++wp>=size ) {		// round about
			wp=0;
		    }
		} else {			// unreachable
		    *m=99;
		}
	    }
	    if( ++rp>=size ) {			// round about
		rp=0;
	    }
	}

	//
	//	Take best of this frame, if any.
	//
	if( bestd!=99999 ) {
	    DebugCheck(unit->Type!=UnitTypeHumanWorker
		    && unit->Type!=UnitTypeOrcWorker);
	    CommandHarvest(unit, bestx, besty,FlushCommands);
	    free(points);
	    return 1;
	}

	//
	//	Continue with next frame.
	//
	if( rp==wp ) {			// unreachable, no more points available
	    break;
	}
	ep=wp;
    }

    DebugLevel0Fn("no wood in range by %s(%d,%d)\n"
	    _C_ unit->Type->Ident _C_ unit->X _C_ unit->Y);

    free(points);
    return 0;
}

#endif

/**
**      Assign worker to haul oil.
*/
local int AiHaulOil(Unit * unit)
{
    Unit *dest;

    DebugLevel3Fn("%d\n" _C_ UnitNumber(unit));
    dest = FindResource(unit->Player, unit->X, unit->Y ,unit->Type->ResourceHarvested);
    if (!dest) {
	DebugLevel0Fn("oil platform not reachable by %s(%d,%d)\n"
		_C_ unit->Type->Ident _C_ unit->X _C_ unit->Y);
	return 0;
    }

    DebugCheck( !unit->Type->Harvester );

    CommandResource(unit, dest,FlushCommands);

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
    Unit* units_with_resource[UnitMax][MaxCosts];   // Worker with resource
    int num_units_with_resource[MaxCosts];
    Unit* units_assigned[UnitMax][MaxCosts];	    // Worker assigned to resource
    int num_units_assigned[MaxCosts];
    Unit* units_unassigned[UnitMax][MaxCosts];	    // Unassigned workers
    int num_units_unassigned[MaxCosts];
    int total;					    // Total of workers
    int c;
    int i;
    int j;
    int o;
    int n;
    Unit** units;
    Unit* unit;
    int percent[MaxCosts];
    int percent_total;

    //
    //	Collect statistics about the current assignment
    //
    percent_total = 100;
    for( c=0; c<MaxCosts; c++ ) {
	num_units_with_resource[c]=0;
	num_units_assigned[c]=0;
	num_units_unassigned[c]=0;
	percent[c]=AiPlayer->Collect[c];
	if( (AiPlayer->NeededMask&(1<<c)) ) {	// Double percent if needed
	    percent_total+=percent[c];
	    percent[c]<<=1;
	}
    }

    n=AiPlayer->Player->TotalNumUnits;
    units=AiPlayer->Player->Units;
    for( i=0; i<n; i++ ) {
	unit=units[i];
	if( !unit->Type->CowerWorker && !unit->Type->Harvester ) {
	    continue;
	}

	//
	//	See if it's assigned already
	//
	switch( unit->Orders[0].Action ) {
	    case UnitActionMineGold:
		units_assigned[num_units_assigned[GoldCost]++][GoldCost]=unit;
		continue;
	    case UnitActionHarvest:
		units_assigned[num_units_assigned[WoodCost]++][WoodCost]=unit;
		continue;
	    case UnitActionResource:
		units_assigned[num_units_assigned[unit->Type->ResourceHarvested]++][unit->Type->ResourceHarvested]=unit;
		continue;
		// FIXME: the other resources
	    default:
		break;
	}

	//
	//	Look what the unit can do
	//
	for( c=0; c<MaxCosts; ++c ) {
	    int tn;
	    UnitType** types;

	    //
	    //	Send workers with resource home
	    //
	    if( c>=AiHelpers.WithGoodsCount || !AiHelpers.WithGoods[c] ) {
		continue;		// Nothing known about the resource
	    }
	    types=AiHelpers.WithGoods[c]->Table;
	    tn=AiHelpers.WithGoods[c]->Count;
	    for( j=0; j<tn; ++j ) {
		if( unit->Type==types[j] ) {
		    units_with_resource[num_units_with_resource[c]++][c]=unit;
		    if (unit->Orders[0].Action == UnitActionStill
			    && unit->OrderCount==1 ) {
			CommandReturnGoods(unit,NULL,FlushCommands);
		    }
		    break;
		}
	    }

	    //
	    //	Look if it is a worker
	    //
	    if( c>=AiHelpers.CollectCount || !AiHelpers.Collect[c] ) {
		continue;		// Nothing known about the resource
	    }
	    types=AiHelpers.Collect[c]->Table;
	    tn=AiHelpers.Collect[c]->Count;
	    for( j=0; j<tn; ++j ) {
		if( unit->Type==types[j] ) {
		    if (unit->Orders[0].Action == UnitActionStill
			    && unit->OrderCount==1 && !unit->Removed ) {
			for( c=0; c<MaxCosts; ++c ) {
			    int tn;
			    UnitType** types;
			    if( c>=AiHelpers.CollectCount || !AiHelpers.Collect[c] ) {
				continue;
			    }
			    types=AiHelpers.Collect[c]->Table;
			    tn=AiHelpers.Collect[c]->Count;
			    for( j=0; j<tn; ++j ) {
				if( unit->Type==types[j] ) {
				    units_unassigned[num_units_unassigned[c]++][c]=unit;
				}
			    }
			}
			break;
		    }
		}
	    }
	    if( j<tn ) {
		break;
	    }
	}
    }

    total=0;
    for( c=0; c<MaxCosts; ++c ) {
	total+=num_units_assigned[c]+num_units_with_resource[c];
	DebugLevel3Fn("Assigned %d = %d\n" _C_ c _C_ num_units_assigned[c]);
	DebugLevel3Fn("Resource %d = %d\n" _C_ c _C_ num_units_with_resource[c]);
    }
    DebugLevel3Fn("Unassigned %d of total %d\n" _C_ num_units_unassigned _C_ total);

    //
    //	Reassign workers
    //
    for( c=0; c<MaxCosts; ++c ) {
	if( num_units_unassigned[c]==0 ) {
	    if( percent[c] == 0 ) {
		continue;
	    }
	    for( i=0; i<MaxCosts; ++i ) {
		if( i==c ) {
		    continue;
		}
		if( (percent[i] < percent[c] &&
		    ((num_units_assigned[c]+num_units_with_resource[c]-1)*percent_total > total*percent[c] || 
		      num_units_assigned[c]+num_units_with_resource[c]==0)) ||
		    (num_units_assigned[i]+num_units_with_resource[i]-1)*percent_total > total*percent[i] ) {
		    int j;
		    for( j=num_units_assigned[i]-1; j>=0; --j ) {
			unit=units_assigned[j][i];
			if( unit && unit->SubAction < 64 ) {
			    units_assigned[j][i] = units_assigned[--num_units_assigned[i]][i];
			    --total;
			    units_unassigned[num_units_unassigned[c]++][c]=unit;
			    break;
			}
		    }
		}
	    }
	}
    }

    //
    //	Now assign the free workers.
    //
    for( n=0; n<MaxCosts; ++n ) {
	for( i=0; i<num_units_unassigned[n]; i++ ) {
	    int t;
	    int max;

	    unit=units_unassigned[i][n];

	    for( max=o=c=0; c<MaxCosts; ++c ) {
		DebugLevel3Fn("%d, %d, %d\n" _C_
		    (num_units_assigned[c]+num_units_with_resource[c])*percent_total _C_
		    percent[c] _C_
		    total*percent[c]);
		t=(num_units_assigned[c]+num_units_with_resource[c])*percent_total;
		if( t < total*percent[c] ) {
		    if( total*percent[c]-t > max ) {
			max=total*percent[c]-t;
			o=c;
		    }
		}
	    }

	    //
	    //	Look what the unit can do
	    //
	    for( t=0; t<MaxCosts; ++t ) {
		int tn;
		UnitType** types;

		c=(t+o)%MaxCosts;

		//
		//	Look if it is a worker for this resource
		//
		if( c>=AiHelpers.CollectCount || !AiHelpers.Collect[c] ) {
		    continue;		// Nothing known about the resource
		}
		types=AiHelpers.Collect[c]->Table;
		tn=AiHelpers.Collect[c]->Count;
		for( j=0; j<tn; ++j ) {
		    if( unit->Type==types[j] && unit->OrderCount==1 ) {
			switch( c ) {
			    int n1;
			    int n2;
			    case GoldCost:
				if( unit->Orders[0].Action==UnitActionMineGold || AiMineGold(unit) ) {
				    DebugLevel3Fn("Assigned to gold\n");
				    units_assigned[num_units_assigned[c]++][c]=unit;
    				    units_unassigned[i][c] = units_unassigned[--num_units_unassigned[c]][c];
				    for( n1=0; n1<MaxCosts; ++n1 ) {
					for( n2=0; n2<num_units_unassigned[n1]; ++n2 ) {
					    if( units_unassigned[n2][n1]==unit ) {
						units_unassigned[n2][n1]=units_unassigned[--num_units_unassigned[n1]][n1];
						break;
					    }
					}
				    }
				    ++total;
				}
				break;
			    case WoodCost:
				if( unit->Orders[0].Action==UnitActionHarvest || AiHarvest(unit) ) {
				    DebugLevel3Fn("Assigned to harvest\n");
				    units_assigned[num_units_assigned[c]++][c]=unit;
    				    units_unassigned[i][c] = units_unassigned[--num_units_unassigned[c]][c];
				    for( n1=0; n1<MaxCosts; ++n1 ) {
					for( n2=0; n2<num_units_unassigned[n1]; ++n2 ) {
					    if( units_unassigned[n2][n1]==unit ) {
						units_unassigned[n2][n1]=units_unassigned[--num_units_unassigned[n1]][n1];
						break;
					    }
					}
				    }
				    ++total;
				}
				break;
			    case OilCost:
				if( unit->Orders[0].Action==UnitActionResource || AiHaulOil(unit) ) {
				    DebugLevel3Fn("Assigned to oil\n");
				    units_assigned[num_units_assigned[c]++][c]=unit;
    				    units_unassigned[i][c] = units_unassigned[--num_units_unassigned[c]][c];
				    for( n1=0; n1<MaxCosts; ++n1 ) {
					for( n2=0; n2<num_units_unassigned[n1]; ++n2 ) {
					    if( units_unassigned[n2][n1]==unit ) {
						units_unassigned[n2][n1]=units_unassigned[--num_units_unassigned[n1]][n1];
						break;
					    }
					}
				    }
				    ++total;
				}
				break;
			    default:
				break;
			}
		    }
		}
	    }
	}
    }
}

/*----------------------------------------------------------------------------
--      WORKERS/REPAIR
----------------------------------------------------------------------------*/

/**
**	Check if we can repair the building.
**
**	@param type	Unit that can repair the building.
**	@param building	Building to be repaired.
**	@return		True if can repair, false if can't repair..
*/
local int AiRepairBuilding(const UnitType* type,Unit* building)
{
    Unit* table[UnitMax];
    Unit* unit;
    Unit* unit_temp;
    int distance[UnitMax];
    int rX;
    int rY;
    int r_temp;
    int index_temp;
    int nunits;
    int i;
    int j;
    int k;
    int num;

    DebugLevel3Fn("%s can repair %s\n" _C_ type->Ident _C_
	    building->Type->Ident);

    IfDebug( unit=NoUnitP; );
    //
    //  Remove all workers not mining. on the way building something
    //	FIXME: It is not clever to use workers with gold
    //  Idea: Antonis: Put the rest of the workers in a table in case
    //  miners can't reach but others can. This will be useful if AI becomes
    //  more flexible (e.g.: transports workers to an island)
    //	FIXME: too hardcoded, not nice, needs improvement.
    //  FIXME: too many workers repair the same building!

    // Selection of mining workers.
    nunits = FindPlayerUnitsByType(AiPlayer->Player,type,table);
    for (num = i = 0; i < nunits; i++) {
	unit = table[i];
    //if (unit->Orders[0].Action != UnitActionBuild && unit->OrderCount==1 ) {
	if ( unit->Orders[0].Action == UnitActionMineGold
		&& unit->OrderCount==1 ) {
	    table[num++] = unit;
	}
    }

    // Sort by distance loops -Antonis-
    for (i=0; i<num; ++i) {
	unit = table[i];
	//	FIXME: Probably calculated from top left corner of building
	if ((rX = unit->X - building->X) < 0) { rX = -rX; }
	if ((rY = unit->Y - building->Y) < 0) { rY = -rY; }
	if (rX<rY) {
	    distance[i] = rX;
	}
	else {
	    distance[i] = rY;
	}
    }
    for (i=0; i<num; ++i) {
	r_temp = distance[i];
	index_temp = i;
	for (j=i; j<num; ++j) {
	    if (distance[j] < r_temp) {
		r_temp = distance[j];
		index_temp = j;
	    }
	}
	if (index_temp > i) {
	    unit_temp = table[index_temp];
	    table[index_temp] = table[i];
	    distance[index_temp] = distance[i];
	    table[i] = unit_temp;
	    distance[i] = r_temp;	// May be omitted, here for completence
	}
    }

    // Check if building is reachable and try next trio of workers

    if ( (j=AiPlayer->TriedRepairWorkers[UnitNumber(building)]) > num) {
	j=AiPlayer->TriedRepairWorkers[UnitNumber(building)]=0;
    }

    for( k=i=j; i<num && i<j+3; ++i ) {

	unit=table[i];
	DebugLevel2Fn("Have an unit to repair %d :)\n" _C_ UnitNumber(unit));

	if( UnitReachable(unit,building,REPAIR_RANGE) ) {
	    CommandRepair(unit, 0, 0, building,FlushCommands);
	    return 1;
	}
	k=i;
    }
    AiPlayer->TriedRepairWorkers[UnitNumber(building)]=k+1;
    return 0;
}

/**
**	Check if we can repair this unit.
**
**	@param unit	Unit that must be repaired.
**	@return		True if made, false if can't be made.
*/
local int AiRepairUnit(Unit* unit)
{
    int i;
    int n;
    const UnitType* type;
    const int* unit_count;
    AiUnitTypeTable* const* tablep;
    const AiUnitTypeTable* table;

    n=AiHelpers.RepairCount;
    tablep=AiHelpers.Repair;
    type=unit->Type;
    if( type->Type>n ) {		// Oops not known.
	DebugLevel0Fn("Nothing known about `%s'\n" _C_ type->Ident);
	return 0;
    }
    table=tablep[type->Type];
    if( !table ) {			// Oops not known.
	DebugLevel0Fn("Nothing known about `%s'\n" _C_ type->Ident);
	return 0;
    }

    n=table->Count;
    unit_count=AiPlayer->Player->UnitTypesCount;
    for( i=0; i<n; ++i ) {
	//
	//	The type is available
	//
	if( unit_count[table->Table[i]->Type] ) {
	    if( AiRepairBuilding(table->Table[i],unit) ) {
		return 1;
	    }
	}
    }

    return 0;
}

/**
**	Look through the units, if an unit must be repaired.
*/
local void AiCheckRepair(void)
{
    int i,j,k;
    int n;
    int repair_flag;
    Unit* unit;

    n=AiPlayer->Player->TotalNumUnits;
    k=0;
	// Selector for next unit
    for( i=n; (i>0); --i ) {
	unit=AiPlayer->Player->Units[i];
	if (unit) {
	    if (UnitNumber(unit)==AiPlayer->LastRepairBuilding) {
		k=i+1;
	    }
	}
    }

    for( i=k; i<n; ++i ) {
	unit=AiPlayer->Player->Units[i];
	repair_flag=1;
	// Unit defekt?
	if( unit->Type->Building
		&& unit->Orders[0].Action!=UnitActionBuilded
		&& unit->Orders[0].Action!=UnitActionUpgradeTo
		&& unit->HP<unit->Stats->HitPoints ) {

	    DebugLevel0Fn("Have building to repair %d(%s)\n" _C_
		    UnitNumber(unit) _C_ unit->Type->Ident);

	    //
	    //	FIXME: Repair only buildings under control
	    //
	    if( EnemyUnitsInDistance(unit,unit->Stats->SightRange) ) {
		continue;
	    }

	    //
	    //	Must check, if there are enough resources
	    //
	    for( j=1; j<MaxCosts; ++j ) {
		if( unit->Stats->Costs[j]
			&& AiPlayer->Player->Resources[j]<99 ) {
		    repair_flag=0;
		}
	    }

	    //
	    //	Find a free worker, who can build this building can repair it?
	    //
	    if ( repair_flag ) {
		AiRepairUnit(unit);
		AiPlayer->LastRepairBuilding=UnitNumber(unit);
		return;
	    }
	}
    }
    AiPlayer->LastRepairBuilding=0;
}

/**
**	Add unit-type request to resource manager.
**
**	@param type	Unit type requested.
**	@param count	How many units.
**
**	@todo FIXME: should store the end of list and not search it.
*/
global void AiAddUnitTypeRequest(UnitType* type,int count)
{
    AiBuildQueue** queue;

    DebugLevel3Fn("%s %d\n" _C_ type->Ident _C_ count);

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
**	Entry point of resource manager, periodically called.
*/
global void AiResourceManager(void)
{
    //
    //	Check if something needs to be build / trained.
    //
    AiCheckingWork();
    //
    //	Look if we can build a farm in advance.
    //
    if( !AiPlayer->NeedFood
	    && AiPlayer->Player->NumFoodUnits==AiPlayer->Player->Food ) {
	DebugLevel1Fn("Farm in advance request\n");
	AiRequestFarms();
    }
    //
    //	Collect resources.
    //
    AiCollectResources();
    //
    //	Check repair.
    //
    AiCheckRepair();

    AiPlayer->NeededMask=0;
}

//@}

#endif // } NEW_AI
