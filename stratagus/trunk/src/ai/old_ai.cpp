//   ___________                     _________                _____  __
//   \_   _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//        \/                \/     \/        \/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         FreeCraft - A free fantasy real time strategy game engine
//
/**@name ai.c           -       The computer player AI main file. */
//
//      (c) Copyright 1998-2001 by Lutz Sammer
//
//      $Id$

//@{

#ifndef NEW_AI				// {

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freecraft.h"
#include "video.h"
#include "sound_id.h"

#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "tileset.h"
#include "map.h"
#include "pathfinder.h"
#include "actions.h"
#include "ai.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/*
**	Names for the unit-type table slots as used in puds.
**
**	@todo: Would be soon global removed.
**	FIXME: This will soon be broken, @see UnitTypes.
*/
#define	UnitFootman		0x00
#define UnitBallista		0x04
#define UnitKnight		0x06
#define UnitArcher		0x08
#define UnitFarm		0x3A
#define UnitBarracksHuman	0x3C
#define UnitScoutTowerHuman	0x40
#define UnitTownHall		0x4A
#define UnitElvenLumberMill	0x4C
#define UnitBlacksmithHuman	0x52
#define UnitKeep		0x58
#define UnitCastle		0x5A

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--      Ai tables
----------------------------------------------------------------------------*/

global int AiSleep = 0;			/// Ai sleeps # frames
global int AiTimeFactor = 100;		/// Adjust the AI build times
global int AiCostFactor = 100;		/// Adjust the AI costs

/**
**	All possible AI control command types.
*/
enum _ai_command_type_
{
    AiCmdBuild,				/// unit must be built
    AiCmdCreature,			/// creature must be built
    AiCmdArmy,				/// attack the opponent
};

/**
**	Define the AI commands.
*/
typedef struct _ai_command_
{
    int Command;			/// The command it self
    int Unit;				/// Unittype to Build, unless AiCmdArmy,
					/// then Units to stay home
    int Number;				/// Number of Units
}
AiCommand;

typedef struct _ai_force_
{
    int Worker;
    int Footman;
    int Archer;
    int Ballista;
    int Knight;
    int GryphonRider;
}
AiForce;

#define X(num) 0

local AiForce ForceAtHome[] = {
//      pea ftm  arc  bal  kni  gry
    {3, 1, 0, 0, 0, 0,},
    {9, 2, 0, 0, 0, 0,},
    {9, 3, 1, 0, 0, 0,},
    {15, 5, 2, 0, 0, 0,},
};

local AiForce ForceAttacking[] = {
//      pea ftm  arc  bal  kni  gry
    {0, 0, 0, 0, 0, 0,},
    {0, 1, 0, 0, 0, 0,},
    {0, 3, 1, 0, 0, 0,},
    {0, 5, 2, 0, 0, 0,},
    {0, 7, 4, 2, 0, 0,},
};

//Makes it less Confusing
#define Wave(num) num

/**
**      Default AI command table:
*/
local AiCommand AiTable1[] = {
    //               AtHome    Attacking    UnitType            Qty
    {AiCmdArmy, Wave(0), Wave(1),},
    {AiCmdArmy, Wave(1), Wave(2),},
    {AiCmdBuild, UnitBarracksHuman, 2,},
    {AiCmdArmy, Wave(2), Wave(3),},
    {AiCmdArmy, Wave(3), Wave(4),},
    {AiCmdBuild, UnitScoutTowerHuman, 2,},
    {0, 0, 0}
};

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/**
**	Define the AI goals.
**	All conditions the AI must check each frame.
*/
typedef struct _ai_goal_ AiGoal;

struct _ai_goal_
{
    AiGoal *Next;			/// double linked list of all goals
    AiGoal *Prev;			/// double linked list of all goals

    int Action;				/// action to be done for goal
    int Number;				/// number of units
    int Unit;				/// type of unit
};

/**
**	This are the variables for the AI players.
**
**	@see Players
*/
typedef struct _player_ai_
{
    Player *Player;			/// player number in Players
    AiGoal *GoalHead;			/// start of double linked list
    AiGoal *GoalNil1;			/// dummy end of double linked list
    AiGoal *GoalTail;			/// end of double linked list

    unsigned Build[(UnitTypeInternalMax + BitsOf(unsigned) - 1)
		   / BitsOf(unsigned)];	/// flags what units are currently build

    Unit *MainHall;			/// home point

    int NeedGold;			/// gold resources needed
    int NeedWood;			/// wood resources needed
    int NeedOil;			/// oil resources needed

    int NoGold;				/// there is no gold
    int NoWood;				/// there is no wood
    int NoOil;				/// there is no oil

    int CurrentAttack;			/// army at attack
    int CurrentHome;			/// army at home

    int DoCommands;			/// true: process new commands
    int CmdIndex;			/// current command
    AiCommand *Commands;		/// command table
} PlayerAi;

local PlayerAi *AiPlayer;		/// current ai player
local PlayerAi Ais[PlayerMax];		/// storage for all players

/*----------------------------------------------------------------------------
--   Finding units
----------------------------------------------------------------------------*/

/**
**  Chooses which Race the building/unit needs to be.
**	FIXME: this is soon broken, must find better way!!!!
*/
local int AiChooseRace(int type)
{
    if (AiPlayer->Player->Race == PlayerRaceHuman) {
	if (type % 2 == 0)
	    return type;
	return type - 1;
    }
    if (type % 2 == 1)
	return type;
    return type + 1;
}

/**
**      Find all free workers of the current player.
*/
local int AiFindFreeWorkers(Unit ** table)
{
    int nunits;
    int num;
    int i;
    Unit *unit;

    nunits = FindPlayerUnitsByType(AiPlayer->Player,
				   UnitTypes +
				   AiChooseRace(UnitTypeHumanWorker->Type),
				   table);
    /*
       nunits += FindPlayerUnitsByType(AiPlayer->Player,
       UnitTypes+AiChooseRace(UnitTypeHumanWorkerWithGold->Type),
       table+nunits);
       nunits += FindPlayerUnitsByType(AiPlayer->Player,
       UnitTypes+AiChooseRace(UnitTypeHumanWorkerWithWood->Type),
       table+nunits);
     */

    //
    //  Remove all workers on the way building something
    //
    for (num = i = 0; i < nunits; i++) {
	unit = table[i];
#ifdef NEW_ORDERS
	if (unit->Orders[0].Action != UnitActionBuild
		&& (unit->OrderCount==1
		    || unit->Orders[1].Action != UnitActionBuild) ) {
	    table[num++] = unit;
	}
#else
	if (unit->Command.Action != UnitActionBuild
	    && unit->NextCommand[0].Action != UnitActionBuild) {
	    table[num++] = unit;
	}
#endif
    }
    return num;
}

/**
**      Find all halls of the current player.
*/
local int AiFindHalls(Unit ** table)
{
    Unit *unit;
    UnitType *type;
    Unit **units;
    int num, nunits, i;

    nunits = AiPlayer->Player->TotalNumUnits;
    units = AiPlayer->Player->Units;
    for (num = 0, i = 0; i < nunits; i++) {
	unit = units[i];
	if (UnitUnusable(unit)) {
	    continue;
	}
	type = unit->Type;
	// Speeds up the tests by avoiding the next serie
	// as a mobile unit can't be a hall...
	if (type->Building && (type->Type == AiChooseRace(UnitTownHall)
			       || type->Type == AiChooseRace(UnitKeep)
			       || type->Type == AiChooseRace(UnitCastle))) {
	    table[num++] = unit;
	}
    }
    return num;
}

/*============================================================================
==       Counting TOOLS
============================================================================*/

//      IDEAS:  This could be cached or precalculated.

/**
**      Numbers of all units of current AI player counted.
*/
local int UnitTypesCount[UnitTypeInternalMax];

/**
**      Count the units of the current Ai player.
**
**      @see UnitTypesCount
*/
local void AiCountUnits(void)
{
    Unit **units;
    int nunits, i;

    // FIXME: No longer needed done by MakeUnit...
    memset(UnitTypesCount, 0, sizeof(UnitTypesCount));
    nunits = AiPlayer->Player->TotalNumUnits;
    units = AiPlayer->Player->Units;
    for (i = 0; i < nunits; i++) {
	++UnitTypesCount[units[i]->Type->Type];
    }
}

/*----------------------------------------------------------------------------
--      Building flags
----------------------------------------------------------------------------*/

/**
**      Are we currently building a unit of that type.
**      @param type     The unit type tested.
*/
local int AiBuildingUnitType(int type)
{
    return AiPlayer->Build[type / BitsOf(*AiPlayer->Build)]
	    & (1 << (type % BitsOf(*AiPlayer->Build)));
}

/**
**      Mark that building a new type is in progress.
**      @param type     The unit type set.
*/
local void AiMarkBuildUnitType(int type)
{
    AiPlayer->Build[type / BitsOf(*AiPlayer->Build)]
	    |= (1 << (type % BitsOf(*AiPlayer->Build)));
}

/**
**      Clear that building a new type is in progress.
**      @param type     The unit type cleared.
*/
local void AiClearBuildUnitType(int type)
{
    AiPlayer->Build[type / BitsOf(*AiPlayer->Build)]
	    &= ~(1 << (type % BitsOf(*AiPlayer->Build)));
}

/*----------------------------------------------------------------------------
--      Resource management
----------------------------------------------------------------------------*/

/**
**   Check for Resources
*/
local int AiNeedResources(const UnitType * type)
{
    int err;

    Player *player;

    player = AiPlayer->Player;
    if ((err = PlayerCheckUnitType(player, type))) {
	if (err & (1 << GoldCost)) {
	    DebugLevel3Fn("%Zd Need gold\n", AiPlayer->Player - Players);
	    AiPlayer->NeedGold = 1;
	}
	if (err & (1 << WoodCost)) {
	    DebugLevel3Fn("%Zd Need wood\n", AiPlayer->Player - Players);
	    AiPlayer->NeedWood = 1;
	}
	if (err & (1 << OilCost)) {
	    DebugLevel3Fn("%Zd Need oil\n", AiPlayer->Player - Players);
	    AiPlayer->NeedOil = 1;
	}
	// FIXME: more resources!!!
	return 1;
    }
    return 0;
}

/*----------------------------------------------------------------------------
--      BUILDING
----------------------------------------------------------------------------*/

/**
**      Place near gold mine.
**
**      FIXME: can move own units out of the way!!
**
**      00000000000000111               This are the best places
**      01111111111110111               0: Is x,y of the hall
**      01111111111110111
**      01111111111110111
**      0111.........0111
**      0111.........0111
**      0111.........0111
**      0111...xxx...0111
**      0111...xxx...0111
**      0111...xxx...0111
**      0111.........0111
**      0111.........0111
**      0111.........0111
**      00000000000000111
**      11111111111111111
**      11111111111111111
**      11111111111111111
*/
local int AiNearGoldmine(Unit * goldmine, Unit * worker, int type, int *dx,
			 int *dy)
{
    int wx, wy, x, y, addx, addy, i, best_x, best_y, d, cost;

    wx = worker->X;
    wy = worker->Y;
    x = goldmine->X;
    y = goldmine->Y;
    addx = goldmine->Type->TileWidth;
    addy = goldmine->Type->TileHeight;
    DebugLevel3("%d,%d\n", UnitTypes[type].TileWidth,
		UnitTypes[type].TileHeight);
    x -= UnitTypes[type].TileWidth;	// this should correct it
    y -= UnitTypes[type].TileHeight - 1;
    addx += UnitTypes[type].TileWidth - 1;
    addy += UnitTypes[type].TileHeight - 1;
    DebugLevel3("XXXXX: %d,%d\n", addx, addy);
    cost = 99999;
    IfDebug(best_x = best_y = 0;
	    );				// remove compiler warning
    for (;;) {				// test rectangles arround the mine
	for (i = addy; i--; y++) {
	    DebugLevel3("\t\tTest %3d,%3d\n", x, y);
	    if (CanBuildUnitType(worker, &UnitTypes[type], x, y)) {
		d = MapDistanceToType(wx, wy, &UnitTypes[type], x, y);
		DebugLevel3("\t\t%3d,%3d -> %3d,%3d = %3d\n", wx, wy, x, y, d);
		if (d < cost) {
		    cost = d;
		    best_x = x;
		    best_y = y;
		}
	    }
	}
	++addx;
	for (i = addx; i--; x++) {
	    DebugLevel3("\t\tTest %3d,%3d\n", x, y);
	    if (CanBuildUnitType(worker, &UnitTypes[type], x, y)) {
		d = MapDistanceToType(wx, wy, &UnitTypes[type], x, y);
		DebugLevel3("\t\t%3d,%3d -> %3d,%3d = %3d\n", wx, wy, x, y, d);
		if (d < cost) {
		    cost = d;
		    best_x = x;
		    best_y = y;
		}
	    }
	}
	++addy;
	for (i = addy; i--; y--) {
	    DebugLevel3("\t\tTest %3d,%3d\n", x, y);
	    if (CanBuildUnitType(worker, &UnitTypes[type], x, y)) {
		d = MapDistanceToType(wx, wy, &UnitTypes[type], x, y);
		DebugLevel3("\t\t%3d,%3d -> %3d,%3d = %3d\n", wx, wy, x, y, d);
		if (d < cost) {
		    cost = d;
		    best_x = x;
		    best_y = y;
		}
	    }
	}
	++addx;
	for (i = addx; i--; x--) {
	    DebugLevel3("\t\tTest %3d,%3d\n", x, y);
	    if (CanBuildUnitType(worker, &UnitTypes[type], x, y)) {
		d = MapDistanceToType(wx, wy, &UnitTypes[type], x, y);
		DebugLevel3("\t\t%3d,%3d -> %3d,%3d = %3d\n", wx, wy, x, y, d);
		if (d < cost) {
		    cost = d;
		    best_x = x;
		    best_y = y;
		}
	    }
	}
	if (cost != 99999) {
	    DebugLevel3("\tBuild at %d,%d\n", best_x, best_y);
	    *dx = best_x;
	    *dy = best_y;
	    return 0;
	}
	++addy;
    }
    abort();				// shouldn't be reached
    return 1;
}

/**
**      Find free building place near hall.
**
**      FIXME: can move own units out of the way!!
*/
global int AiNearHall(Unit * hall, Unit * worker, UnitType * type, int *dx,
		      int *dy)
{
    int wx, wy, x, y, addx, addy, num_goldmine, g;
    int end, state, best_x, best_y, d, cost;
    Unit *goldmines[MAX_UNITS];

    DebugLevel3Fn("hall %d %d,%d\n", UnitNumber(hall), hall->X, hall->Y);

    if (!hall) {			// No hall take units place.
	DebugLevel3Fn("Called without hall\n");
	*dx = worker->X;
	*dy = worker->Y;
	return 99998;
    }

    num_goldmine = FindUnitsByType(UnitTypeGoldMine, goldmines);
    DebugLevel3("\tGoldmines %d\n", num_goldmine);
    wx = worker->X;
    wy = worker->Y;
    x = hall->X;
    y = hall->Y;
    addx = hall->Type->TileWidth;
    addy = hall->Type->TileHeight;
    cost = 99999;
    IfDebug(best_x = best_y = 0;
	    );				// remove compiler warning
    DebugLevel3("%d,%d\n", type->TileWidth, type->TileHeight);
    /// leave two fields free!
#define SPACE 2		/// Space arround buildings
    x -= type->TileWidth + SPACE;	// this should correct it
    y -= type->TileHeight + SPACE - 1;
    addx += type->TileWidth + SPACE + 1;
    addy += type->TileHeight + SPACE + 1;
#undef SPACE
    state = 0;
    end = y + addy - 1;
    for (;;) {				// test rectangles arround the hall
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
		if (cost != 99999) {
		    DebugLevel3("\tBuild at %d,%d\n", best_x, best_y);
		    *dx = best_x;
		    *dy = best_y;
		    return cost;
		}
	    }
	    break;
	}
	// FIXME: this check outside the map could be speeded up.
	if (y < 0 || x < 0 || y >= TheMap.Height || x >= TheMap.Width) {
	    DebugLevel3("\t\tSkip %3d,%3d\n", x, y);
	    continue;
	}
	DebugLevel3("\t\tTest %3d,%3d\n", x, y);
	if (CanBuildUnitType(worker, type, x, y)) {
	    if (x == hall->X || y == hall->Y || x == hall->X + 1
		|| y == hall->Y + 1 || x == hall->X + 2 || y == hall->Y + 2
		|| x == hall->X - 2 || y == hall->Y - 2 || x == hall->X - 1
		|| y == hall->Y - 1) {
		continue;
	    }
	    // Check if too near to goldmine
	    for (g = 0; g < num_goldmine; ++g) {
		if (MapDistanceToType
		    (x, y, UnitTypeGoldMine, goldmines[g]->X,
		     goldmines[g]->Y) < 4) {
		    break;
		}
	    }
	    if (g != num_goldmine) {
		continue;
	    }				//too near goldmine
	    d = MapDistanceToType(wx, wy, type, x, y);
	    DebugLevel3("\t\t%3d,%3d -> %3d,%3d = %3d\n", wx, wy, x, y, d);
	    if (d < cost) {
		cost = d;
		best_x = x;
		best_y = y;
	    }
	}
    }
    return 0;
}

/**
**      Find the best place for a hall.
**
**      Returns:
**              -1:     impossible
**               0:     done
**
**
**      Original:       build at current place!
**
**      The best place:
**              1) near to goldmine.
**              !2) near to wood.
**              3) near to worker and must be reachable.
**              !4) no enemy near it.
**              !5) no hall already near
*/
local int AiBuildHall(int type)
{
    Unit *workers[MAX_UNITS];
    Unit *goldmines[MAX_UNITS];
    int num_goldmine, g, best_w, best_g, best_x, best_y, cost;
    int x, y, d, num_worker, w;

    DebugLevel3Fn("\n");

    //  Find all available peon/peasants.
    num_worker = AiFindFreeWorkers(workers);
    DebugLevel3("\tWorkers %d\n", num_worker);
    if (!num_worker) {
	return -1;
    }					// QUESTION: only not working ??

    //  Find all goldmines.
    num_goldmine = FindUnitsByType(UnitTypeGoldMine, goldmines);
    DebugLevel3("\tGoldmines %d\n", num_goldmine);
    if (!num_goldmine) {
	return -1;
    }
    //
    //  Find best place.
    //  FIXME: this isn't the best, can later write a better routine
    //

    //  Find worker with the shortest distance to a gold-mine.
    cost = 99999;
    IfDebug(best_w = best_g = 0;
	    );				// remove compiler warning
    for (w = 0; w < num_worker; ++w) {
	x = workers[w]->X;
	y = workers[w]->Y;
	for (g = 0; g < num_goldmine; ++g) {
	    d = MapDistanceToUnit(x, y, goldmines[g]);
	    DebugLevel3("\t\t%3d,%3d -> %3d,%3d = %3d\n", x, y,
			goldmines[g]->X, goldmines[g]->Y, d);
	    if (d < cost && UnitReachable(workers[w], goldmines[g], 1)) {
		best_w = w;
		best_g = g;
		cost = d;
	    }
	}
    }
    // Did use the first if no could be moved.
    DebugLevel3("\tWorker %Zd %d,%d -> Goldmine %Zd %d,%d\n",
		UnitNumber(workers[best_w])
		, workers[best_w]->X, workers[best_w]->Y,
		UnitNumber(goldmines[best_g])
		, goldmines[best_g]->X, goldmines[best_g]->Y);

    //  Find the nearest buildable place near the gold-mine.
    if (AiNearGoldmine(goldmines[best_g]
		       , workers[best_w], type, &best_x, &best_y)) {
	return -1;
    }
    CommandBuildBuilding(workers[best_w], best_x, best_y, &UnitTypes[type],FlushCommands);
    AiMarkBuildUnitType(type);
    return 0;
}

/**
**      Find place for farm/barracks/etc..
**
**      Returns:
**              -1:     impossible
**               0:     done
**
**
**      The best place:
**              1) near to home hall.
**              2) near to worker and must be reachable.
**              3) didn't block the way to the goldmine.
**              !4) no enemy near it.
*/
local int AiBuildBuilding(int type)
{
    Unit *workers[MAX_UNITS];
    int num_worker, cost, best_w, best_x, best_y, x, y, d, w;

    DebugLevel3Fn("(%d)\n", type);

    //  Find all available peon/peasants.
    num_worker = AiFindFreeWorkers(workers);
    DebugLevel3("\tWorkers %d\n", num_worker);
    if (!num_worker) {
	return -1;
    }
    //  Build near to main hall.
    //if( !AiPlayer->MainHall ) {return -1;}

    //  FIXME: use them as defence like humans.
    //  FIXME: don't block the way to the gold mine

    //  Find best worker near a building place near home hall.
    cost = 99999;
    IfDebug(best_w = best_x = best_y = 0;
	    );				// keep the compiler happy
    for (w = 0; w < num_worker; ++w) {
	if ((d =
	     AiNearHall(AiPlayer->MainHall, workers[w], &UnitTypes[type], &x,
			&y))) {
	    if (d < cost && PlaceReachable(workers[w], x, y, 1)) {
		// JOHNS: ?if(x != y)
		{
		    cost = d;
		    best_w = w;
		    best_x = x;
		    best_y = y;
		}
	    }
	}
    }
    if (cost != 99999) {
	DebugLevel3Fn("at %d,%d\n", best_x, best_y);
	CommandBuildBuilding(workers[best_w], best_x, best_y, &UnitTypes[type],
			     1);
	AiMarkBuildUnitType(type);
	return 0;
    }
    return -1;
}

/*----------------------------------------------------------------------------
--      Train
----------------------------------------------------------------------------*/

/**
**      Train Creature.
**
**              Returns:        -1 impossible
**                               1 wait
**                               0 started
*/
local int AiTrainCreature(int type)
{
    Unit *units[MAX_UNITS];
    int nunits;
    Player *player;

    DebugLevel3Fn("\n");
    if (type == AiChooseRace(UnitTypeHumanWorker->Type)) {
	nunits = AiFindHalls(units);
    } else {
	nunits = FindPlayerUnitsByType(AiPlayer->Player,
	    UnitTypes + AiChooseRace(UnitBarracksHuman),
	    //FIXME: jon: AiChooseTrainer(type),
	    units);
    }
    if (!nunits) {
	return -1;
    }
    while (nunits--) {
#ifdef NEW_ORDERS
	if (units[nunits]->Orders[0].Action != UnitActionStill) {
	    continue;
	}
#else
	if (units[nunits]->Command.Action != UnitActionStill) {
	    continue;
	}
#endif
	player = AiPlayer->Player;
	CommandTrainUnit(units[nunits], &UnitTypes[type],FlushCommands);
	// FIXME: and if not possible?
	AiMarkBuildUnitType(type);
	return 0;
    }
    return 1;
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
	// FIXME: now not really, no goldmine.
	DebugLevel0Fn("goldmine not reachable\n");
	//AiPlayer->NoGold=1;
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
    AiPlayer->NoWood = 1;

    // FIXME: if we reach the map borders we can go fast up, left, ...
    --x;
    while (addx <= r && addy <= r) {
	for (i = addy; i--; y++) {	// go down
	    if (CheckedForestOnMap(x, y)) {
		AiPlayer->NoWood = 0;
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
		AiPlayer->NoWood = 0;
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
		AiPlayer->NoWood = 0;
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
		AiPlayer->NoWood = 0;
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
    DebugLevel0Fn("no wood on map\n");
    return 0;
}

/**
**      Assigned workers: let them do something.
*/
local void AiAssignWorker(void)
{
    Unit *workers[MAX_UNITS];
    int num_worker, num_gold, num_wood, num_repair, num_still;
    int action, type, w;

    DebugLevel3Fn("\n");

    //  Count workers
    num_worker = AiFindFreeWorkers(workers);
    if (num_worker) {
	num_still = num_gold = num_wood = num_repair = 0;
	for (w = 0; w < num_worker; ++w) {
#ifdef NEW_ORDERS
	    action = workers[w]->Orders[0].Action;
#else
	    action = workers[w]->Command.Action;
#endif
	    switch (action) {
	    case UnitActionStill:
		++num_still;
		break;
	    case UnitActionMineGold:
		++num_gold;
		break;
	    case UnitActionHarvest:
		++num_wood;
		break;
	    case UnitActionRepair:
		++num_repair;
		break;
	    default:
		DebugLevel0("%d\n", action);
		break;
	    }
	}

	DebugLevel3("Ai: Player %Zd: ", AiPlayer->Player - Players);
	DebugLevel3("Workers %d, Gold %d, Wood %d, Repair %d, Still %d.\n",
		    num_worker, num_gold, num_wood, num_repair, num_still);

	if (AiPlayer->NeedGold && AiPlayer->NeedWood) {
	    DebugLevel3("Ai: Player %Zd need gold and wood\n",
			AiPlayer->Player - Players);
	    //      Assign half to wood and gold.
	    if (num_still) {		// assign the non-working
		for (w = 0; w < num_worker; ++w) {
		    type = workers[w]->Type->Type;
#ifdef NEW_ORDERS
		    action = workers[w]->Orders[0].Action;
#else
		    action = workers[w]->Command.Action;
#endif
		    if (action == UnitActionStill) {
			if (type ==
			    AiChooseRace(UnitTypeHumanWorkerWithGold->Type)
			    || type ==
			    AiChooseRace(UnitTypeHumanWorkerWithWood->Type)) {
			    CommandReturnGoods(workers[w],NoUnitP,FlushCommands);
			} else {
			    if (num_gold <= num_wood) {
				AiMineGold(workers[w]);
				++num_gold;
			    } else {
				AiHarvest(workers[w]);
				++num_wood;
			    }
			}
		    }
		}
	    }
	    return;			//Continue
	}

	if (AiPlayer->NeedGold) {
	    DebugLevel3("Ai: Player %Zd need gold\n",
			AiPlayer->Player - Players);
	    //      Assign all to mine gold.
	    for (w = 0; w < num_worker; ++w) {
		type = workers[w]->Type->Type;
		if (type == AiChooseRace(UnitTypeHumanWorkerWithWood->Type)
		    || type ==
		    AiChooseRace(UnitTypeHumanWorkerWithGold->Type)) {
		    CommandReturnGoods(workers[w],NoUnitP,FlushCommands);
		} else {
		    // FIXME: don't interrupt chopping
#ifdef NEW_ORDERS
		    action = workers[w]->Orders[0].Action;
#else
		    action = workers[w]->Command.Action;
#endif
		    if (action == UnitActionStill
			|| action == UnitActionHarvest) {
			AiMineGold(workers[w]);
			DebugLevel3("\tAdd worker to gold\n");
		    }
		}
	    }
	    return;
	}

	if (AiPlayer->NeedWood) {
	    DebugLevel3("Ai: Player %Zd need wood\n",
			AiPlayer->Player - Players);
	    //      Assign all to harvest wood.
	    for (w = 0; w < num_worker; ++w) {
		type = workers[w]->Type->Type;
		if (type == AiChooseRace(UnitTypeHumanWorkerWithWood->Type)
		    || type ==
		    AiChooseRace(UnitTypeHumanWorkerWithGold->Type)) {
		    CommandReturnGoods(workers[w],NoUnitP,FlushCommands);
		} else {
#ifdef NEW_ORDERS
		    action = workers[w]->Orders[0].Action;
#else
		    action = workers[w]->Command.Action;
#endif
		    if (action == UnitActionStill
			|| action == UnitActionMineGold) {
			AiHarvest(workers[w]);
		    }
		}
	    }
	    return;
	}
	//  Unassigned workers: let them do something.
	//          FIXME: if there is no gold/no wood forget this!
	if (num_still) {		// assign the non working
	    for (w = 0; w < num_worker; ++w) {
#ifdef NEW_ORDERS
		action = workers[w]->Orders[0].Action;
#else
		action = workers[w]->Command.Action;
#endif
		if (action == UnitActionStill) {
		    type = workers[w]->Type->Type;
		    if (type == AiChooseRace(UnitTypeHumanWorkerWithGold->Type)
			|| type ==
			AiChooseRace(UnitTypeHumanWorkerWithWood->Type)) {
			if (AiPlayer->MainHall) {
			    CommandReturnGoods(workers[w],NoUnitP,FlushCommands);
			}
			continue;
		    }
		    if (!AiPlayer->NoGold && num_gold <= num_wood) {
			AiMineGold(workers[w]);
			++num_gold;
		    } else if (!AiPlayer->NoWood) {
			AiHarvest(workers[w]);
			++num_wood;
		    }
		}
	    }
	    // FIXME: reassign units !!
	}
    }
#if 1
    //
    //  Send standing workers home.
    //
    num_worker =
	    FindPlayerUnitsByType(AiPlayer->Player,
				  UnitTypes +
				  AiChooseRace(UnitTypeHumanWorkerWithGold->
					       Type), workers);
    DebugLevel3("Gold %d\n", num_worker);
    if (num_worker) {			// assign the non working
	if (AiPlayer->MainHall) {
	    for (w = 0; w < num_worker; ++w) {
#ifdef NEW_ORDERS
		action = workers[w]->Orders[0].Action;
#else
		action = workers[w]->Command.Action;
#endif
		if (action == UnitActionStill) {
		    CommandReturnGoods(workers[w],NoUnitP,FlushCommands);
		}
	    }
	}
    }
    num_worker =
	    FindPlayerUnitsByType(AiPlayer->Player,
				  UnitTypes +
				  AiChooseRace(UnitTypeHumanWorkerWithWood->
					       Type), workers);
    DebugLevel3("Wood %d\n", num_worker);
    if (num_worker) {			// assign the non working
	if (AiPlayer->MainHall) {
	    for (w = 0; w < num_worker; ++w) {
#ifdef NEW_ORDERS
		action = workers[w]->Orders[0].Action;
#else
		action = workers[w]->Command.Action;
#endif
		if (action == UnitActionStill) {
		    CommandReturnGoods(workers[w],NoUnitP,FlushCommands);
		}
	    }
	}
    }
#endif
}

/*----------------------------------------------------------------------------
--      GOAL
----------------------------------------------------------------------------*/

/**
**	Show Goal
*/
local void AiShowGoal(const char *function, const AiGoal * goal)
{
    IfDebug(printf("\t%s:%d ", function, AiPlayer - Ais); switch (goal->Action) {
case AiCmdBuild:			/// unit must be built
printf("build %d*%s\n", goal->Number, UnitTypes[goal->Unit].Ident); break; case AiCmdCreature:	/// creature must be built
printf("train %d*%s\n", goal->Number, UnitTypes[goal->Unit].Ident); break; case AiCmdArmy:	/// attack the opponent
	    printf("army %d*%d\n", goal->Number, goal->Unit); break;}

    ) ;
}

/**
**      Insert a new goal into the Ai player goal list.
*/
local void AiNewGoal(int action, int number, int type)
{
    AiGoal *goal;

    if (AiPlayer->GoalHead != 0)
	if (AiPlayer->GoalHead->Action == action
	    && AiPlayer->GoalHead->Number == number
	    && AiPlayer->GoalHead->Unit == type) {
	    return;
	}

    goal = malloc(sizeof(AiGoal));
    goal->Next = AiPlayer->GoalHead;
    goal->Prev = (AiGoal *) & AiPlayer->GoalHead;
    AiPlayer->GoalHead->Prev = goal;
    goal->Action = action;
    goal->Number = number;
    goal->Unit = type;
    AiPlayer->GoalHead = goal;

    AiShowGoal(__FUNCTION__, goal);
}

/**
**      Delete a goal from the Ai player goal list.
*/
local void AiDelGoal(AiGoal * goal)
{
    AiShowGoal(__FUNCTION__, goal);
    goal->Next->Prev = goal->Prev;
    goal->Prev->Next = goal->Next;
    free(goal);
}

/**
**   Check for Food
**   Returns TRUE if not enough food.
*/
local int AiNeedFood(const UnitType * type)
{
    Player *player;

    player = AiPlayer->Player;
    if (!PlayerCheckFood(player, type)) {
	// already building new food (farm or hall)
	if (AiBuildingUnitType(AiChooseRace(UnitFarm))
	    || AiBuildingUnitType(AiChooseRace(UnitTownHall))) {
	    return 1;
	}
	AiNewGoal(AiCmdBuild, UnitTypesCount[AiChooseRace(UnitFarm)] + 1,
		  AiChooseRace(UnitFarm));
	return 1;
    }
    return 0;
}

/*
**   Check to see if building exists
**   Returns TRUE if building doesn't exist
*/
local int AiNoBuilding(int type)
{
    type = AiChooseRace(type);
    if (type == AiChooseRace(UnitTownHall)) {
	if (!UnitTypesCount[type]
	    && !UnitTypesCount[AiChooseRace(UnitKeep)]
	    && !UnitTypesCount[AiChooseRace(UnitCastle)]) {
	    if (!AiBuildingUnitType(type))
		AiNewGoal(AiCmdBuild, 1, type);
	    return 1;
	}
	return 0;
    }
    if (!UnitTypesCount[type]) {
	if (!AiBuildingUnitType(type))
	    AiNewGoal(AiCmdBuild, 1, type);
	return 1;
    }
    return 0;
}

/**
**  Check to see if building is needed.
**  Returns TRUE if building needed and building doesn't exist.
*/
local int AiNeedBuilding(int type)
{
    UnitType *typep;

    type = AiChooseRace(type);
    // FIXME: johns should use dependence rules some time
    typep = UnitTypes + type;
    if (typep == UnitTypeHumanWorker || typep == UnitTypeOrcWorker) {
	if (AiNoBuilding(UnitTownHall))
	    return 1;
    } else if (typep == UnitTypeByIdent("unit-ballista")
	       || typep == UnitTypeByIdent("unit-catapult")) {
	if (AiNoBuilding(UnitTownHall))
	    return 1;
	if (AiNoBuilding(UnitBlacksmithHuman))
	    return 1;
    } else if (typep == UnitTypeByIdent("unit-archer")
	       || typep == UnitTypeByIdent("unit-axethrower")) {
	if (AiNoBuilding(UnitTownHall))
	    return 1;
	if (AiNoBuilding(UnitElvenLumberMill))
	    return 1;
    } else if (typep == UnitTypeByIdent("unit-footman")
	       || typep == UnitTypeByIdent("unit-grunt")) {
	if (AiNoBuilding(UnitTownHall))
	    return 1;
	if (AiNoBuilding(UnitBarracksHuman))
	    return 1;
    }
    return 0;
}

/**
**      We need a number of units.
**      Returns TRUE when number is reached.
*/
local int AiCommandBuild(int type, int number, int action)
{
    DebugLevel3Fn("%d, %d, %d\n", type, number, action);
    if (number == 0)
	return 1;
    type = AiChooseRace(type);
    if (AiBuildingUnitType(type)) {
	return 0;
    }					//already training
    AiCountUnits();
    if (type == AiChooseRace(UnitTypeHumanWorker->Type)) {
	if ((UnitTypesCount[type]
	     + UnitTypesCount[AiChooseRace(UnitTypeHumanWorkerWithGold->Type)]
	     + UnitTypesCount[AiChooseRace(UnitTypeHumanWorkerWithWood->Type)])
	    >= number)
	    return 1;
    } else {
	if (UnitTypesCount[type] >= number)
	    return 1;
    }
    if (AiNeedBuilding(type))
	return 0;
    if (AiNeedResources(&UnitTypes[type]))
	return 0;
    if (action == AiCmdCreature) {
	if (AiNeedFood(&UnitTypes[type]))
	    return 0;
	AiTrainCreature(type);
	return 0;
    }
    if (type == AiChooseRace(UnitTownHall)) {
	AiBuildHall(type);
    } else {
	AiBuildBuilding(type);
    }
    return 0;
}

/**
**   Tell the Ai to start Attacking
*/
local int AiCommandAttack(int unittype, int attack, int home)
{
    int nunits, i;
    Unit *enemy;
    Unit *table[MAX_UNITS];

    nunits = FindPlayerUnitsByType(AiPlayer->Player,
				   UnitTypes + AiChooseRace(unittype), table);
    if (nunits < attack + home)
	return 0;
    for (i = 0; i < attack; i++) {
	enemy = AttackUnitsInDistance(table[i], 1000);
	if (enemy) {
	    CommandAttack(table[i], enemy->X, enemy->Y, NULL,FlushCommands);
	}
    }
    return 1;
}

/**
**    Create an Attack Force
**    Returns TRUE when Force Complete;
*/
local int AiCommandArmy(int home, int attack)
{
    if (!AiCommandBuild
	(UnitTypeHumanWorker->Type,
	 ForceAtHome[home].Worker + ForceAttacking[attack].Worker,
	 AiCmdCreature)) {
	return 0;
    }
    if (!AiCommandBuild
	(UnitFootman,
	 ForceAtHome[home].Footman + ForceAttacking[attack].Footman,
	 AiCmdCreature)) {
	return 0;
    }
    if (!AiCommandBuild
	(UnitArcher, ForceAtHome[home].Archer + ForceAttacking[attack].Archer,
	 AiCmdCreature)) {
	return 0;
    }
    if (!AiCommandBuild
	(UnitBallista,
	 ForceAtHome[home].Ballista + ForceAttacking[attack].Ballista,
	 AiCmdCreature)) {
	return 0;
    }
    AiPlayer->CurrentAttack = attack;
    AiPlayer->CurrentHome = home;
    return 1;
}

/**
**      Next goal.
*/
local void AiNextGoal(void)
{
    AiGoal *goal, *temp;

    AiPlayer->NeedGold = AiPlayer->NeedWood = AiPlayer->NeedOil = 0;
    for (goal = AiPlayer->GoalHead; (temp = goal->Next); goal = temp) {
	if (goal->Action == AiCmdArmy) {
	    if (AiCommandArmy(goal->Unit, goal->Number)) {
		AiDelGoal(goal);
	    }
	} else {
	    if (goal->Unit == -1) {
		DebugLevel0Fn("Oops\n");
		continue;
	    }
	    if (AiCommandBuild(goal->Unit, goal->Number, goal->Action)) {
		AiDelGoal(goal);
	    }
	}
    }
    if (AiPlayer->NeedGold || AiPlayer->NeedWood) {
	AiAssignWorker();
    }
}

/*----------------------------------------------------------------------------
--      CALL BACKS
----------------------------------------------------------------------------*/

/**
**   Called if a Unit is Attacked
**
**   @param unit    Pointer to unit that is being attacked.
**
*/
global void AiHelpMe(Unit * unit)
{
    DebugLevel0("HELP %d %d", unit->X, unit->Y);
}

/**
**      Called if work complete (Buildings).
**
**      @param unit     Pointer to unit what builds the building.
**      @param what     Pointer to unit building that was build.
*/
global void AiWorkComplete(Unit * unit, Unit * what)
{
    DebugLevel3("Ai: Player %Zd: %Zd Work %Zd complete\n",
		unit->Player - Players, UnitNumber(unit), UnitNumber(what));
    // FIXME: correct position
    if (unit->Player->Type == PlayerHuman) {
	return;
    }
    AiPlayer = &Ais[unit->Player->Player];
    AiClearBuildUnitType(what->Type->Type);
    if (!AiPlayer->MainHall && what->Type->Type == AiChooseRace(UnitTownHall)) {
	AiPlayer->MainHall = what;
    }
    AiAssignWorker();
}

/**
**      Called if building can't be build.
**
**      @param unit     Pointer to unit what builds the building.
**      @param what     Pointer to unit-type.
*/
global void AiCanNotBuild(Unit * unit, const UnitType * what)
{
    DebugLevel1("Ai: Player %Zd: %Zd Can't build %d at %d,%d\n",
		unit->Player - Players, UnitNumber(unit), what->Type, unit->X,
		unit->Y);
    // FIXME: correct position
    if (unit->Player->Type == PlayerHuman) {
	return;
    }
    AiPlayer = &Ais[unit->Player->Player];
    AiClearBuildUnitType(what->Type);
    AiNewGoal(AiCmdBuild, 1, what->Type);
}

/**
**      Called if building place can't be reached.
**
**      @param unit     Pointer to unit what builds the building.
**      @param what     Pointer to unit-type.
*/
global void AiCanNotReach(Unit * unit, const UnitType * what)
{
    DebugLevel3("Ai: Player %Zd: %Zd Can't reach %d at %d,%d\n",
		unit->Player - Players, UnitNumber(unit), what->Type, unit->X,
		unit->Y);
    // FIXME: correct position
    if (unit->Player->Type == PlayerHuman) {
	return;
    }
    AiPlayer = &Ais[unit->Player->Player];
    AiClearBuildUnitType(what->Type);
    AiNewGoal(AiCmdBuild, 1, what->Type);
    //CommandBuildBuilding(unit,unit->X,unit->Y,&UnitTypes[UnitWallHuman],1);
    //FIXME: should be a better way than above line.
}

/**
**      Called if training of an unit is completed.
**
**      @param unit     Pointer to unit.
**      @param what     Pointer to type.
*/
global void AiTrainingComplete(Unit * unit, Unit * what)
{
    DebugLevel3("Ai: Player %Zd: %Zd Training %Zd complete\n",
		unit->Player - Players, UnitNumber(unit), UnitNumber(what));
    // FIXME: correct position
    if (unit->Player->Type == PlayerHuman) {
	return;
    }
    // FIXME: Should I put an AiPlayer pointer into the player struct?
    AiPlayer = &Ais[unit->Player->Player];
    if (what->Type->CowerWorker) {
	AiAssignWorker();
    }
    AiClearBuildUnitType(what->Type->Type);
}

/**
**      This is called for each player, each frame.
**
**      @param player   The player structure pointer.
*/
global void AiEachFrame(Player * player)
{
    AiCommand command;

    if (AiSleep) {			// wait some time. FIXME: should become
	// an AI command :)
	--AiSleep;
	return;
    }

    AiPlayer = player->Ai;
    DebugLevel3Fn("Player %d\n", player->Player);
    command = AiPlayer->Commands[AiPlayer->CmdIndex];
    if (AiPlayer->GoalHead->Next == 0) {
	if (command.Number == 0 && command.Command == 0) {
	    DebugLevel3("Ai Starting Over\n");
	    //AiPlayer->CmdIndex = 0;
	    AiPlayer->DoCommands = 0;
	}
	if (AiPlayer->DoCommands) {
	    AiNewGoal(command.Command, command.Number, command.Unit);
	    AiPlayer->CmdIndex++;
	}
    }
    AiNextGoal();
}

/**
**      This called for each player, each second.
**
**      @param player   The player structure pointer.
*/
global void AiEachSecond(Player * player)
{
    DebugLevel3Fn("Player %d\n", player->Player);
    if (AiSleep) {			// wait some time. FIXME: see above
	return;
    }
    AiPlayer = player->Ai;		//  Prepare ai.
    AiAssignWorker();
    if (AiCommandAttack
	(UnitBallista, ForceAttacking[AiPlayer->CurrentAttack].Ballista,
	 ForceAtHome[AiPlayer->CurrentHome].Ballista))
	if (AiCommandAttack
	    (UnitArcher, ForceAttacking[AiPlayer->CurrentAttack].Archer,
	     ForceAtHome[AiPlayer->CurrentHome].Archer))
	    if (AiCommandAttack
		(UnitFootman, ForceAttacking[AiPlayer->CurrentAttack].Footman,
		 ForceAtHome[AiPlayer->CurrentHome].Footman))
		AiPlayer->CurrentAttack = 0;
}

/**
**      Setup all at start.
**
**      @param player   The player structure pointer.
*/
global void AiInit(Player * player)
{
    int i;
    int j;
    PlayerAi *aip;
    Unit *units[MAX_UNITS];

    DebugLevel2Fn("Player %d\n" _C_ player->Player);
    player->Ai = aip = &Ais[player->Player];
    aip->Player = player;

    for (i = 0; i < UnitTypeInternalMax / (sizeof(int) * 8); ++i) {
	aip->Build[i] = 0;
    }
    aip->GoalHead = (AiGoal *) & aip->GoalNil1;
    aip->GoalNil1 = (AiGoal *) 0;
    aip->GoalTail = (AiGoal *) & aip->GoalHead;
    aip->MainHall = NoUnitP;
    aip->NeedGold = 0;
    aip->NeedWood = 0;
    aip->NeedOil = 0;
    aip->NoGold = 0;
    aip->NoWood = 0;
    aip->NoOil = 0;
    aip->CurrentAttack = 0;
    aip->CurrentHome = 0;
    aip->DoCommands = 1;
    aip->CmdIndex = 0;
    aip->Commands = AiTable1;

    //
    //  Adjust costs for AI Player
    //
    for (i = 0; UnitTypes[i].OType; ++i) {
	for (j = 0; j < MaxCosts; ++j) {
	    UnitTypes[i].Stats[player->Player].Costs[j] *=
		    j == TimeCost ? AiTimeFactor : AiCostFactor;
	    UnitTypes[i].Stats[player->Player].Costs[j] /= 100;
	}
    }

    AiPlayer = aip;
    if (AiFindHalls(units)) {		// without this nothing works
	aip->MainHall = units[0];
    }
}

#endif // } NEW_AI

//@}
