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
/**@name unit_find.c	-	The find/select for units. */
//
//	(c) Copyright 1998-2003 by Lutz Sammer
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
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

#include "stratagus.h"
#include "video.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "missile.h"
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
#define PRIORITY_FACTOR		0x00010000
#define HEALTH_FACTOR		0x00000001
#define DISTANCE_FACTOR		0x00100000
#define INRANGE_FACTOR		0x00010000
#define INRANGE_BONUS		0x00100000
#define CANATTACK_BONUS		0x01000000

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
    int num;
    int nunits;
    int i;

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
global Unit* UnitOnMapTile(int tx,int ty)
{
    Unit* table[UnitMax];
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
global Unit* RepairableOnMapTile(int tx,int ty)
{
    Unit* table[UnitMax];
    int n;
    int i;

    n=SelectUnitsOnTile(tx,ty,table);
    for( i=0; i<n; ++i ) {
	// FIXME: could use more or less for repair? Repair of ships/catapults.
	// Only repairable if target is a building or tansporter and it's HP is
	// not at max
	if( (table[i]->Type->Building || table[i]->Type->Transporter)
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
global Unit* TargetOnMapTile(const Unit* source,int tx,int ty)
{
    Unit* table[UnitMax];
    Unit* unit;
    Unit* best;
    const UnitType* type;
    int n;
    int i;

    n=SelectUnitsOnTile(tx,ty,table);
    best=NoUnitP;
    for( i=0; i<n; ++i ) {
	unit=table[i];
	// unusable unit ?
	// if( UnitUnusable(unit) ) can't attack constructions
	// FIXME: did SelectUnitsOnTile already filter this?
	// Invisible and not Visible
	if( unit->Removed || unit->Invisible || !unit->HP
		|| !(unit->Visible&(1<<source->Player->Player))
		|| unit->Orders[0].Action==UnitActionDie ) {
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
global Unit* TransporterOnMapTile(int tx,int ty)
{
    Unit* table[UnitMax];
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

/**
**	Returns true if screen map position (x,y) is inside of
**	the rectangle where the unit's sprite is drawn.
**
**	@param unit	Pointer to unit whose sprite is to be tested
**	@param x	X position on screen map, pixel-based
**	@param y	Y position on screen map, pixel-based
**
**	@return		true if (x,y) is inside the unit's sprite
*/
local int InsideUnitSprite(const Unit* unit,int x,int y)
{
    int ux;
    int uy;		// position at which unit's sprite is currently drawn
    const UnitType* type;

    type=unit->Type;
    ux = TileSizeX*unit->X + unit->IX;
    ux -= (type->BoxWidth - TileSizeX*type->TileWidth)/2;
    uy = TileSizeY*unit->Y + unit->IY;
    uy -= (type->BoxHeight - TileSizeY*type->TileHeight)/2;

    if (x < ux || x >= ux+type->BoxWidth
	    || y < uy ||  y >= uy+type->BoxHeight) {
	return 0;
    } else {
	return 1;
    }
}

/**
**	Searches for unit whose sprite is drawn at (x,y) pixel map position.
**
**	@param x	X position on screen map, pixel-based.
**	@param y	Y position on screen map, pixel-based.
**
**	@return		Returns unit found at this pixel map coordinates
*/
global Unit* UnitOnScreenMapPosition(int x,int y)
{
    Unit* table[UnitMax];
    int tx;
    int ty;
    int n;
    int i;

    //	this code runs quite often (e.g. upon each motion notify) so this
    //	little optimization could be appropriate

    tx = x / TileSizeX;
    ty = y / TileSizeY;

    //	fast path, should work most of the time
    n = SelectUnitsOnTile(tx, ty, table);
    for( i=0; i<n; ++i ) {
	if( !table[i]->Type->Vanishes && InsideUnitSprite(table[i], x, y)) {
	    return table[i];
	}
    }

    //	if we got here we have to search for our unit in the neighborhood

    //	ships and flyers could be 2 fields away
    n = UnitCacheSelect(tx-2,ty-2, tx+2, ty+2, table);
    for( i=0; i<n; ++i ) {
	if( !table[i]->Type->Vanishes && InsideUnitSprite(table[i], x, y)) {
	    return table[i];
	}
    }

    return NoUnitP;
}

/**
**	Repairable unit on screen map position.
**
**	@param x	X position on screen map, pixel-based.
**	@param y	Y position on screen map, pixel-based.
**
**	@return		Returns repairable unit found on screen map position.
*/
global Unit* RepairableOnScreenMapPosition(int x,int y)
{
    Unit* table[UnitMax];
    int tx;
    int ty;
    int n;
    int i;

    tx = x / TileSizeX;
    ty = y / TileSizeY;

    n = UnitCacheSelect(tx-2,ty-2, tx+2, ty+2, table);
    for( i=0; i<n; ++i ) {
	// FIXME: could use more or less for repair? Repair of ships/catapults.
	// Only repairable if target is a building or tansporter and it's HP is
	// not at max
	if( (table[i]->Type->Building || table[i]->Type->Transporter)
		&& table[i]->HP < table[i]->Stats->HitPoints ) {
	    if (InsideUnitSprite(table[i], x, y)) {
		return table[i];
	    }
	}
    }
    return NoUnitP;
}

/**
**	Choose target at pixel map coordinates.
**
**	@param source	Unit which wants to attack.
**	@param x	X position on the display map, pixel-based.
**	@param y	Y position on the display map, pixel-based.
**
**	@return		Returns ideal target
*/
global Unit* TargetOnScreenMapPosition(const Unit* source,int x,int y)
{
    Unit* table[UnitMax];
    Unit* unit;
    Unit* best;
    int tx;
    int ty;
    int n;
    int i;

    //	this code runs upon right button action only so it can affort being a
    //	little inefficient.

    tx = x / TileSizeX;
    ty = y / TileSizeY;

    //	 ships and flyers could be 2 fields away
    n = UnitCacheSelect(tx-2,ty-2, tx+2, ty+2, table);
    best=NoUnitP;
    for( i=0; i<n; ++i ) {
	unit=table[i];
	// unusable unit ?
	// if( UnitUnusable(unit) ) can't attack constructions
	// FIXME: did SelectUnitsOnTile already filter this?
	// Invisible and not Visible
	if( unit->Removed || unit->Invisible
		|| !(unit->Visible&(1<<source->Player->Player))
		|| unit->Orders[0].Action==UnitActionDie ) {
	    continue;
	}
	if ( !InsideUnitSprite(table[i], x, y)) {
	    continue;
	}
	if( !CanTarget(source->Type,unit->Type) ) {
	    continue;
	}
	//
	//	Choose the best target.
	//
	if( !best || best->Type->Priority < unit->Type->Priority ) {
	    best=unit;
	}
    }
    return best;
}

/**
**	Transporter unit on screen map position.
**
**	@param x	X position on screen map, pixel-based.
**	@param y	Y position on screen map, pixel-based.
**
**	@return		Returns transporter unit found on tile.
*/
global Unit* TransporterOnScreenMapPosition(int x,int y)
{
    Unit* table[UnitMax];
    int tx;
    int ty;
    int n;
    int i;

    tx = x / TileSizeX;
    ty = y / TileSizeY;

//  n=SelectUnitsOnTile(tx,ty,table);
    n = UnitCacheSelect(tx-2,ty-2, tx+2, ty+2, table);
    for( i=0; i<n; ++i ) {
	if( table[i]->Type->Transporter && InsideUnitSprite(table[i], x, y)) {
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
    Unit* table[UnitMax];
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
**	Oil patch on map tile
**
**	@param tx	X position on map, tile-based.
**	@param ty	Y position on map, tile-based.
**
**	@return		Returns the oil patch if found, or NoUnitP.
*/
global Unit* OilPatchOnMap(int tx,int ty)
{
    Unit* table[UnitMax];
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
    Unit* table[UnitMax];
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
**	Resource deposit on map tile
**
**	@param tx	X position on map, tile-based.
**	@param ty	Y position on map, tile-based.
**	@param resource	resource type.
**
**	@return		Returns the deposit if found, or NoUnitP.
*/
global Unit* ResourceDepositOnMap(int tx,int ty,int resource)
{
    Unit* table[UnitMax];
    int i;
    int n;

    n=SelectUnitsOnTile(tx,ty,table);
    for( i=0; i<n; ++i ) {
	if( UnitUnusable(table[i]) ) {
	    continue;
	}
	if( table[i]->Type->Stores[resource] ) {
	    return table[i];
	}
    }
    return NoUnitP;
}

/*----------------------------------------------------------------------------
--	Finding units for attack
----------------------------------------------------------------------------*/

/**
**	Attack units in distance, with large missile
**
**		Choose the best target, that can be attacked. It takes into
**		account allied unit which could be hit by the missile
**
**	@param u	Find in distance for this unit.
**	@param range	Distance range to look.
**
**	@return		Unit to be attacked.
**
**	@note	This could be improved, for better performance / better trade.
**      @note   Limited to attack range smaller than 16.
**	@note	Will be moved to unit_ai.c soon.
*/
local Unit* FindRangeAttack(Unit* u, int range)
{
    int x, y, n, cost,d,effective_hp,enemy_count;
    int missile_range,attackrange,hp_damage_evaluate;
    int good[32][32], bad[32][32];
    Unit* table[UnitMax];
    Unit* dest;
    const UnitType* dtype;
    const UnitType* type;
    const Player* player;
    int xx, yy;
    int best_x, best_y, best_cost;
    int i, sbad, sgood;
    Unit* best;

    type = u->Type;
    player = u->Player;

    //  If catapult, count units near the target...
    //      FIXME : make it configurable
    //

    missile_range = type->Missile.Missile->Range + range - 1;
    attackrange=u->Stats->AttackRange;
    // Evaluation of possible damage...
    hp_damage_evaluate=u->Stats->BasicDamage+u->Stats->PiercingDamage;

    DebugCheck(2 * missile_range + 1 >= 32);

    x = u->X;
    y = u->Y;
    n = SelectUnits(x - missile_range, y - missile_range,
	x + missile_range + 1, y + missile_range + 1, table);

    if (!n) {
	return NoUnitP;
    }

    for (y = 0; y < 2 * missile_range + 1; y++) {
	for (x = 0; x < 2 * missile_range + 1; x++) {
	    good[y][x] = 0;
	    bad[y][x] = 0;
	}
    }

    enemy_count=0;
    // FILL good/bad...
    for (i = 0; i < n; i++) {
	dest = table[i];
	dtype = dest->Type;
	//
	//      unusable unit
	//
	// FIXME: did SelectUnits already filter this.
	if (dest->Removed || dest->Invisible || !u->HP
		|| !(dest->Visible & (1 << player->Player))
		|| dest->Orders[0].Action == UnitActionDie) {
	    table[i] = 0;
	    continue;
	}

        // won't be a target...
        if (!CanTarget(type, dtype)) {	// can't be attacked.
	    table[i] = 0;
	    continue;
	}

	if (!IsEnemy(player, dest)) {	// a friend or neutral
	    table[i] = 0;

	    // Calc a negative cost
            // The gost is more important when the unit would be killed
            // by our fire.

            // It costs ( is positive ) if hp_damage_evaluate>dest->HP ... )
	    // FIXME : assume that PRIORITY_FACTOR>HEALTH_FACTOR
	    cost = HEALTH_FACTOR*(2*hp_damage_evaluate-dest->HP) / (dtype->TileWidth * dtype->TileWidth);
	    if (cost < 1) {
		cost = 1;
	    }
	    cost = (-cost);
	} else {
	    //
	    //  Calculate the costs to attack the unit.
	    //  Unit with the smallest attack costs will be taken.
	    //
	    cost = 0;
	    //
	    //  Priority 0-255
	    //
	    cost += dtype->Priority * PRIORITY_FACTOR;
	    //
	    //  Remaining HP (Health) 0-65535
	    //
	    // Give a boost to unit we can kill in one shoot only

	    //
	    // calculate HP which will remain in the enemy unit, after hit
	    //
	    effective_hp=(dest->HP-2*hp_damage_evaluate);

	    //
	    // Unit we won't kill are evaluated the same
	    //
	    if (effective_hp>0){
	      effective_hp=0;
	    }

	    //
	    // Unit we are sure to kill are all evaluated the same ( except PRIORITY )
	    //
	    if (effective_hp<(-hp_damage_evaluate)){
	      effective_hp=(-hp_damage_evaluate);
	    }

	    //
	    // Here, effective_hp vary from -hp_damage_evaluate ( unit will be killed) to 0 ( unit can't be killed )
	    // => we prefer killing rather than only hitting...
	    //
	    cost += (-effective_hp) * HEALTH_FACTOR;

	    //
	    //  Unit can attack back.
	    //
	    if (CanTarget(dtype, type)) {
		cost += CANATTACK_BONUS;
	    }

            //
            // the cost may be divided accros multiple cells
            //
	    cost=cost / (dtype->TileWidth * dtype->TileWidth);
	    if (cost < 1) {
		cost = 1;
	    }
     	    d=MapDistanceBetweenUnits(u,dest);

            // FIXME: we don't support moving away!
            if((d<type->MinAttackRange)||(!UnitReachable(u,dest,attackrange))) {
	    	table[i]=0;
	    } else {
	    	enemy_count++;
	    }
	}

	x = dest->X - u->X + missile_range+1;
	y = dest->Y - u->Y + missile_range+1;

	// Mark the good/bad array...
	for (xx = 0; xx < dtype->TileWidth; xx++) {
	    for (yy = 0; yy < dtype->TileWidth; yy++) {
		if ((x + xx < 0) || (y + yy < 0)
			|| (x + xx >= 2 * missile_range + 1)
			|| (y + yy >= 2 * missile_range + 1)) {
		    continue;
		}
		if (cost < 0) {
		    good[y + yy][x + xx] -= cost;
		} else {
		    bad[y + yy][x + xx] += cost;
		}
	    }
	}
    }

    if (!enemy_count) {
	return NoUnitP;
    }

    // Find the best area...
    // The target which provide the best bad/good ratio is choosen...
    best_x = -1;
    best_y = -1;
    best_cost = -1;
    best = NoUnitP;
    for (i = 0; i < n; i++) {
	if (!table[i]) {
	    continue;
	}
	dest = table[i];
	dtype = dest->Type;

	// put in x-y the real point which will be hit...
	// ( only meaningfull when dtype->TileWidth>1 )
        if (u->X<dest->X){
	    x=dest->X;
	} else if (u->X>dest->X+dtype->TileWidth-1){
	    x=dest->X+dtype->TileWidth-1;
	} else {
	    x=u->X;
	}

	if (u->Y<dest->Y){
	    y=dest->Y;
	} else if(u->Y>dest->Y+dtype->TileWidth-1){
	    y=dest->Y+dtype->TileWidth-1;
	} else {
	    y=u->Y;
	}

 	// Make x,y relative to u->x...
	x = x - u->X + missile_range+1;
	y = y - u->Y + missile_range+1;

	sbad = 0;
	sgood = 0;
	for (yy = -(type->Missile.Missile->Range - 1);
	    yy <= type->Missile.Missile->Range - 1; yy++) {
	    for (xx = -(type->Missile.Missile->Range - 1);
		xx <= type->Missile.Missile->Range - 1; xx++) {
		if ((x + xx < 0) || (y + yy < 0)
			|| ((x + xx) >= 2 * missile_range + 1)
			|| ((y + yy) >= 2 * missile_range + 1)) {
		    continue;
		}

		sbad += bad[y + yy][x + xx];
		sgood += good[y + yy][x + xx];
		if ((!yy) && (!xx)) {
		    sbad += bad[y + yy][x + xx];
		    sgood += good[y + yy][x + xx];
		}
	    }
	}

	// don't consider small damages...
	if (sgood < 20) {
	    sgood = 20;
	}

	cost = sbad / sgood;
	if (cost > best_cost) {
	    best_cost = cost;
	    best = dest;
	}
    }
    return best;
}

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
**
**	@note	This could be improved, for better performance.
*/
global Unit* AttackUnitsInDistance(Unit* unit,int range)
{
    const Unit* dest;
    const UnitType* type;
    const UnitType* dtype;
    Unit* table[UnitMax];
    int x;
    int y;
    int n;
    int i;
    int d;
    int attackrange;
    int cost;
    const Player* player;
    const Unit* best_unit;
    int best_cost;
    int tried_units;
    int reachable;

    DebugLevel3Fn("(%d)%s\n" _C_ UnitNumber(unit) _C_ unit->Type->Ident);

    // if necessary, take possible damage on allied units into account...
    if (unit->Type->Missile.Missile->Range>1
	    && (range+unit->Type->Missile.Missile->Range<15)) {
        return FindRangeAttack(unit,range);
    }

    //
    //	Select all units in range.
    //
    x=unit->X;
    y=unit->Y;
    n=SelectUnits(x-range,y-range,x+range+1,y+range+1,table);

    best_unit=NoUnitP;
    best_cost=INT_MAX;
    tried_units=0;

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
	if( dest->Removed || dest->Invisible || !unit->HP
		|| !(dest->Visible&(1<<player->Player))
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
	cost-=dtype->Priority*PRIORITY_FACTOR;
	//
	//	Remaining HP (Health) 0-65535
	//
	cost+=dest->HP*HEALTH_FACTOR;
	//
	//	Unit in attack range?
	//
	d=MapDistanceBetweenUnits(unit,dest);
	if( d<type->MinAttackRange ) {	// FIXME: we don't support moving away!
	    continue;
	}
	if( d<attackrange && d>type->MinAttackRange ) {
	    cost+=d*INRANGE_FACTOR;
	    cost-=INRANGE_BONUS;
	} else {
	    cost+=d*DISTANCE_FACTOR;
	}
	//
	//	Unit can attack back.
	//
	if( CanTarget(dtype,type) ) {
	    cost-=CANATTACK_BONUS;
	}

	DebugLevel3Fn("%s -> %s\t%08x\n" _C_ type->Ident _C_ dtype->Ident _C_ cost);
	//
	//	Take this target?
	//
	// If we failed 5 times, we probably can't get anything
	if( tried_units >= 5 && best_cost == INT_MAX ) {
	    best_unit=NULL;
	    break;
	}
	if( cost<best_cost ) {
	    reachable = UnitReachable(unit,dest,attackrange);
	    if( d<attackrange || reachable ) {
		best_unit=dest;
		best_cost=cost;
	    } else {
		tried_units++;
	    }
	}
    }

/*
    if( best_unit ) {
	DebugLevel3Fn("Attacking (%d)%s -> %s\n" _C_ UnitNumber(unit) _C_
		unit->Type->Ident _C_ best_unit->Type->Ident);
    }
*/

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
global Unit* AttackUnitsInRange(Unit* unit)
{
    //
    //	Only units which can attack.
    //
    IfDebug(

	if( !unit->Type->CanAttack && !unit->Type->Tower ) {
	    DebugLevel0Fn("Should be handled by caller?\n");
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
global Unit* AttackUnitsInReactRange(Unit* unit)
{
    int range;
    const UnitType* type;

    //
    //	Only units which can attack.
    //
    type=unit->Type;
    IfDebug(
	if( !type->CanAttack && !type->Tower ) {
	    DebugLevel0Fn("Should be handled by caller?\n");
	    abort();
	    return NoUnitP;
	}
    );

    if( unit->Player->Type==PlayerPerson ) {
	range=type->ReactRangePerson;
    } else {
	range=type->ReactRangeComputer;
    }

    return AttackUnitsInDistance(unit,range);
}

//@}
