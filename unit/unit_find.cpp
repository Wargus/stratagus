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
/**@name unit_find.c	-	The find/select for units. */
//
//	(c) Copyright 1998-2001 by Lutz Sammer
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "freecraft.h"
#include "video.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "interface.h"
#include "tileset.h"
#include "map.h"
#include "pathfinder.h"

/*----------------------------------------------------------------------------
--	Defines
----------------------------------------------------------------------------*/

/*
**	Configuration of the small (unit) AI.
*/
#define PRIORITY_FACTOR		(0x00010000*100)
#define HEALTH_FACTOR		(0x00000001*100)
#define DISTANCE_FACTOR		(0x00100000*100)
#define INRANGE_FACTOR		(0x00010000*100)
#define INRANGE_BONUS		(0x00100000*100)
#define CANATTACK_BONUS		(0x01000000*100)

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Select units in rectangle range.
**
**	@param x1	Left column of selection rectangle
**	@param y1	Top row of selection rectangle
**	@param x2	Right column of selection rectangle
**	@param y2	Bottom row of selection rectangle
**	@param table	All units in the selection rectangle
**
**	@return		Returns the number of units found
*/
global int SelectUnits(int x1,int y1,int x2,int y2,Unit** table)
{
    if ( x1 == x2 && y1 == y2 ) {
	return UnitCacheOnTile(x1,y1,table);
    } else {
	return UnitCacheSelect(x1,y1,x2,y2,table);
    }
}

/**
**	Select units on tile.
**
**	@param x	Map X tile position
**	@param y	Map Y tile position
**	@param table	All units in the selection rectangle
**
**	@return		Returns the number of units found
*/
global int SelectUnitsOnTile(int x,int y,Unit** table)
{
    return UnitCacheOnTile(x,y,table);
}

/**
**	Find all units of type.
**
**	@param type	type of unit requested
**	@param table	table in which we have to store the units
**
**	@return		Returns the number of units found.
*/
global int FindUnitsByType(const UnitType* type,Unit** table)
{
    Unit* unit;
    int i;
    int num;

    for( num=i=0; i<NumUnits; i++ ) {
	unit=Units[i];
	if( unit->Type==type && !UnitUnusable(unit) ) {
	    table[num++]=unit;
	}
    }
    return num;
}

/**
**	Find all units of type.
**
**	@param player	we're looking for the units of this player
**	@param type	type of unit requested
**	@param table	table in which we have to store the units
**
**	@return		Returns the number of units found.
*/
global int FindPlayerUnitsByType(const Player* player,const UnitType* type
	,Unit** table)
{
    Unit* unit;
    Unit** units;
    int num,nunits,i;

    nunits=player->TotalNumUnits;
    units=player->Units;
    // FIXME: Can't abort if all units are found: UnitTypeCount
    for( num=0,i=0; i<nunits; i++ ) {
	unit=units[i];
	if( unit->Type==type && !UnitUnusable(unit) ) {
	    table[num++]=unit;
	}
    }
    return num;
}

/**
**	Unit on map tile, no special prefered.
**
**	@param tx	X position on map, tile-based.
**	@param ty	Y position on map, tile-based.
**
**	@return		Returns first found unit on tile.
*/
global Unit* UnitOnMapTile(unsigned tx,unsigned ty)
{
    Unit* table[MAX_UNITS];
    int n;
    int i;

    n=SelectUnitsOnTile(tx,ty,table);
    for( i=0; i<n; ++i ) {
        // Note: this is less restrictive than UnitActionDie...
        // Is it normal?
	if( table[i]->Type->Vanishes ) {
	    continue;
	}
	return table[i];
    }

    return NoUnitP;
}

/**
**	Repairable unit on map tile.
**
**	@param tx	X position on map, tile-based.
**	@param ty	Y position on map, tile-based.
**
**	@return		Returns repairable unit found on tile.
*/
global Unit* RepairableOnMapTile(unsigned tx,unsigned ty)
{
    Unit* table[MAX_UNITS];
    int n;
    int i;

    n=SelectUnitsOnTile(tx,ty,table);
    for( i=0; i<n; ++i ) {
	// FIXME: could use more or less for repair? Repair of ships/catapults.
	// Only repairable if target is a building and it's HP is not at max
	if( table[i]->Type->Building
		&& table[i]->HP < table[i]->Stats->HitPoints ) {
	    return table[i];
	}
    }
    return NoUnitP;
}

/**
**	Choose target on map tile.
**
**	@param source	Unit which want to attack.
**	@param tx	X position on map, tile-based.
**	@param ty	Y position on map, tile-based.
**
**	@return		Returns ideal target on map tile.
*/
global Unit* TargetOnMapTile(Unit* source,unsigned tx,unsigned ty)
{
    Unit* table[MAX_UNITS];
    Unit* unit;
    Unit* best;
    UnitType* type;
    int n;
    int i;

    n=SelectUnitsOnTile(tx,ty,table);
    best=NoUnitP;
    for( i=0; i<n; ++i ) {
	unit=table[i];
	// unusable unit ?
	// if( UnitUnusable(unit) ) can't attack constructions
	// FIXME: did SelectUnitsOnTile already filter this?
	if( unit->Removed || unit->Orders[0].Action==UnitActionDie ) {
	    continue;
	}
	type=unit->Type;
	if( tx<unit->X || tx>=unit->X+type->TileWidth
		|| ty<unit->Y || ty>=unit->Y+type->TileHeight ) {
	    continue;
	}
	if( !CanTarget(source->Type,unit->Type) ) {
	    continue;
	}

	//
	//	Choose the best target.
	//
	if( !best || best->Type->Priority<unit->Type->Priority ) {
	    best=unit;
	}
    }
    return best;
}

/**
**	Transporter unit on map tile.
**
**	@param tx	X position on map, tile-based.
**	@param ty	Y position on map, tile-based.
**
**	@return		Returns transporter unit found on tile.
*/
global Unit* TransporterOnMapTile(unsigned tx,unsigned ty)
{
    Unit* table[MAX_UNITS];
    int n;
    int i;

    n=SelectUnitsOnTile(tx,ty,table);
    for( i=0; i<n; ++i ) {
	if( table[i]->Type->Transporter ) {
	    return table[i];
	}
    }
    return NoUnitP;
}

/*----------------------------------------------------------------------------
--	Finding special units
----------------------------------------------------------------------------*/

/**
**	Gold mine on map tile
**
**	@param tx	X position on map, tile-based.
**	@param ty	Y position on map, tile-based.
**
**	@return		Returns the gold mine if found, or NoUnitP.
*/
global Unit* GoldMineOnMap(int tx,int ty)
{
    Unit* table[MAX_UNITS];
    int i;
    int n;

    n=SelectUnitsOnTile(tx,ty,table);
    for( i=0; i<n; ++i ) {
	if( UnitUnusable(table[i]) ) {
	    continue;
	}
	if( table[i]->Type->GoldMine ) {
	    return table[i];
	}
    }
    return NoUnitP;
}

/**
**	Gold deposit on map tile
**
**	@param tx	X position on map, tile-based.
**	@param ty	Y position on map, tile-based.
**
**	@return		Returns the gold deposit if found, or NoUnitP.
*/
global Unit* GoldDepositOnMap(int tx,int ty)
{
    Unit* table[MAX_UNITS];
    int i;
    int n;

    n=SelectUnitsOnTile(tx,ty,table);
    for( i=0; i<n; ++i ) {
	if( UnitUnusable(table[i]) ) {
	    continue;
	}
	if( table[i]->Type->StoresGold ) {
	    return table[i];
	}
    }
    return NoUnitP;
}

/**
**	Oil patch on map tile
**
**	@param tx	X position on map, tile-based.
**	@param ty	Y position on map, tile-based.
**
**	@return		Returns the oil patch if found, or NoUnitP.
*/
global Unit* OilPatchOnMap(int tx,int ty)
{
    Unit* table[MAX_UNITS];
    int i;
    int n;

    n=SelectUnitsOnTile(tx,ty,table);
    for( i=0; i<n; ++i ) {
	if( table[i]->Type->OilPatch ) {
	    return table[i];
	}
    }
    return NoUnitP;
}

/**
**	Oil platform on map tile
**
**	@param tx	X position on map, tile-based.
**	@param ty	Y position on map, tile-based.
**
**	@return		Returns the oil platform if found, or NoUnitP.
*/
global Unit* PlatformOnMap(int tx,int ty)
{
    Unit* table[MAX_UNITS];
    int i;
    int n;

    n=SelectUnitsOnTile(tx,ty,table);
    for( i=0; i<n; ++i ) {
	if( UnitUnusable(table[i]) ) {
	    continue;
	}
	if( table[i]->Type->GivesOil ) {
	    return table[i];
	}
    }
    return NoUnitP;
}

/**
**	Oil deposit on map tile
**
**	@param tx	X position on map, tile-based.
**	@param ty	Y position on map, tile-based.
**
**	@return		Returns the oil deposit if found, or NoUnitP.
*/
global Unit* OilDepositOnMap(int tx,int ty)
{
    Unit* table[MAX_UNITS];
    int i;
    int n;

    n=SelectUnitsOnTile(tx,ty,table);
    for( i=0; i<n; ++i ) {
	if( UnitUnusable(table[i]) ) {
	    continue;
	}
	if( table[i]->Type->StoresOil ) {
	    return table[i];
	}
    }
    return NoUnitP;
}

/**
**	Wood deposit on map tile
**
**	@param tx	X position on map, tile-based.
**	@param ty	Y position on map, tile-based.
**
**	@return		Returns the wood deposit if found, or NoUnitP.
*/
global Unit* WoodDepositOnMap(int tx,int ty)
{
    Unit* table[MAX_UNITS];
    int i;
    int n;

    n=SelectUnitsOnTile(tx,ty,table);
    for( i=0; i<n; ++i ) {
	if( UnitUnusable(table[i]) ) {
	    continue;
	}
	if( table[i]->Type->StoresWood || table[i]->Type->StoresGold ) {
	    return table[i];
	}
    }
    return NoUnitP;
}

/*----------------------------------------------------------------------------
--	Finding units for attack
----------------------------------------------------------------------------*/

/**
**	Attack units in distance.
**
**		If the unit can attack must be handled by caller.
**		Choose the best target, that can be attacked.
**
**	@param unit	Find in distance for this unit.
**	@param range	Distance range to look.
**
**	@return		Unit to be attacked.
*/
global Unit* AttackUnitsInDistance(const Unit* unit,unsigned range)
{
    const Unit* dest;
    const UnitType* type;
    const UnitType* dtype;
    Unit* table[MAX_UNITS];
    unsigned x;
    unsigned y;
    unsigned n;
    unsigned i;
    unsigned d;
    int attackrange;
    int cost;
    const Player* player;
    const Unit* best_unit;
    int best_cost;

    //
    //	Select all units in range.
    //
    x=unit->X;
    y=unit->Y;
    n=SelectUnits(x-range,y-range,x+range+1,y+range+1,table);

    best_unit=NoUnitP;
    best_cost=INT_MAX;

    player=unit->Player;
    type=unit->Type;
    attackrange=unit->Stats->AttackRange;
    //
    //	Find the best unit to attack
    //
    for( i=0; i<n; ++i ) {
	dest=table[i];
	//
	//	unusable unit
	//
	// FIXME: did SelectUnits already filter this.
	if( dest->Removed || dest->Invisible
		|| dest->Orders[0].Action==UnitActionDie ) {
	    continue;
	}

	if( !IsEnemy(player,dest) ) {	// a friend or neutral
	    continue;
	}

	dtype=dest->Type;
	if( !CanTarget(type,dtype) ) {	// can't be attacked.
	    continue;
	}

	//
	//	Calculate the costs to attack the unit.
	//	Unit with the smallest attack costs will be taken.
	//
	cost=0;
	//
	//	Priority 0-255
	//
	cost-=dtype->Priority*PRIORITY_FACTOR/100;
	//
	//	Remaining HP (Health) 0-65535
	//
	cost+=dest->HP*HEALTH_FACTOR/100;
	//
	//	Unit in attack range?
	//
	d=MapDistanceBetweenUnits(unit,dest);
	if( d<type->MinAttackRange ) {	// FIXME: we don't support moving away!
	    continue;
	}
	if( d<attackrange && d>type->MinAttackRange ) {
	    cost+=d*INRANGE_FACTOR/100;
	    cost-=INRANGE_BONUS/100;
	} else {
	    cost+=d*DISTANCE_FACTOR/100;
	}
	//
	//	Unit can attack back.
	//
	if( CanTarget(dtype,type) ) {
	    cost-=CANATTACK_BONUS/100;
	}

	DebugLevel3Fn("%s -> %s\t%08x\n",type->Ident,dtype->Ident,cost);
	//
	//	Take this target?
	//
	if( cost<best_cost && UnitReachable(unit,dest,attackrange) ) {
	    best_unit=dest;
	    best_cost=cost;
	}
    }

    // FIXME: No idea how to make this correct, without cast!!
    return (Unit*)best_unit;
}

/**
**	Attack units in attack range.
**
**	@param unit	Find unit in attack range for this unit.
**
**	@return		Pointer to unit which should be attacked.
*/
global Unit* AttackUnitsInRange(const Unit* unit)
{
    //
    //	Only units which can attack.
    //
    IfDebug(

	if( !unit->Type->CanAttack && !unit->Type->Tower ) {
	    DebugLevel0("Should be handled by caller?\n");
	    abort();
	    return NoUnitP;
	}
    );

    return AttackUnitsInDistance(unit,unit->Stats->AttackRange);
}

/**
**	Attack units in reaction range.
**
**	@param unit	Find unit in reaction range for this unit.
**
**	@return		Pointer to unit which should be attacked.
*/
global Unit* AttackUnitsInReactRange(const Unit* unit)
{
    int range;
    const UnitType* type;

    //
    //	Only units which can attack.
    //
    type=unit->Type;
    IfDebug(
	if( !type->CanAttack && !type->Tower ) {
	    DebugLevel0("Should be handled by caller?\n");
	    abort();
	    return NoUnitP;
	}
    );

    if( unit->Player->Type==PlayerHuman ) {
	range=type->ReactRangeHuman;
    } else {
	range=type->ReactRangeComputer;
    }

    return AttackUnitsInDistance(unit,range);
}

//@}
