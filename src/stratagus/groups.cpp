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
/**@name groups.c	-	The units' groups handling. */
/*
**	(c) Copyright 1999,2000 by Patrice Fortier
**
**	$Id$
*/

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>

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


/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/**
**	Defines a group of units.
*/
typedef struct _unit_group_ {
    Unit *units[NUM_UNITS_PER_GROUP];	/// Units in the group
    int num_units;			/// How many units in the group
} UnitGroup;	/// group of units

global UnitGroup Groups[NUM_GROUPS];	/// Number of groups predefined

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
 **	Initialize group part.
 */
global void InitGroups(void)
{
    int i;

    for( i=0; i<NUM_GROUPS; i++ ) {
        Groups[i].num_units=0;
    }
}

/**
 **	Return the number of units of group #num
 */
global int GetNumberUnitsOfGroup(int num)
{
    return Groups[num].num_units;
}

/**
 **	Return the units of group #num
 */
global Unit** GetUnitsOfGroup(int num)
{
    return Groups[num].units;
}

/**
 **	Clear contents of group #num
 */
global void ClearGroup(int num)
{
    UnitGroup *group;
    int i;

    group=&Groups[num];
    for( i=0; i<group->num_units; i++ ) {
	group->units[i]->GroupId=-1;
	group->units[i]=NoUnitP;
    }
    group->num_units=0;
}

/**
 **	Set group #num contents to unit array "units"
 */
global void SetGroup(Unit **units,int nunits,int num)
{
    UnitGroup *group;
    int i;

    DebugCheck(num>NUM_GROUPS);

    ClearGroup(num);

    group=&Groups[num];
    for( i=0; i<nunits; i++ ) {
        group->units[i]=units[i];
	group->units[i]->GroupId=num;
    }
    group->num_units=nunits;
}

/**
 **	Remove unit from its group
 */
global void RemoveUnitFromGroup(Unit *unit)
{
    UnitGroup *group;
    int num;
    int i;

    // unit doesn't belong to a group
    if( (num=unit->GroupId)==-1 ) {
        return;
    }

    group=&Groups[num];
    for( i=0; group->units[i]!=unit; i++ ) {
	;
    }

    DebugCheck( i>=group->num_units );

    // This is a clean way that will allow us to add a unit
    // to a group easily, or make an easy array walk...
    if( i<--group->num_units ) {
        group->units[i]=group->units[group->num_units];
    }
    group->units[group->num_units]=NoUnitP;

    unit->GroupId=-1;
}

//@}
