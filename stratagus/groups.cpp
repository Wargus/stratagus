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
//
//	(c) Copyright 1999-2001 by Patrice Fortier and Lutz Sammer
//
//	$Id$

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
    Unit*	Units[NUM_UNITS_PER_GROUP];	/// Units in the group
    int		NumUnits;			/// How many units in the group
} UnitGroup;				/// group of units

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
        Groups[i].NumUnits=0;
    }
}

/**
 **	Return the number of units of group #num
 **
 **	@param num	Group number.
 **	@return		Returns the number of units in the group.
 */
global int GetNumberUnitsOfGroup(int num)
{
    return Groups[num].NumUnits;
}

/**
 **	Return the units of group #num
 **
 **	@param num	Group number.
 **	@return		Returns an array of all units in the group.
 */
global Unit** GetUnitsOfGroup(int num)
{
    return Groups[num].Units;
}

/**
 **	Clear contents of group #num
 **
 **	@param num	Group number.
 */
global void ClearGroup(int num)
{
    UnitGroup *group;
    int i;

    group=&Groups[num];
    for( i=0; i<group->NumUnits; i++ ) {
	group->Units[i]->GroupId=-1;
	DebugCheck( group->Units[i]->Destroyed );
    }
    group->NumUnits=0;
}

/**
 **	Add units to group #num contents from unit array "units"
 **
 **	@param units	Array of units to place into group.
 **	@param nunits	Number of units in array.
 **	@param num	Group number for storage.
 */
global void AddToGroup(Unit **units,int nunits,int num)
{
    UnitGroup *group;
    int i;

    DebugCheck(num>NUM_GROUPS);

    group=&Groups[num];
    for( i=0; group->NumUnits<NUM_UNITS_PER_GROUP && i<nunits; i++ ) {
	
        group->Units[group->NumUnits++]=units[i];
	units[i]->GroupId=num;
    }
}

/**
 **	Set group #num contents to unit array "units"
 **
 **	@param units	Array of units to place into group.
 **	@param nunits	Number of units in array.
 **	@param num	Group number for storage.
 */
global void SetGroup(Unit **units,int nunits,int num)
{
    DebugCheck(num>NUM_GROUPS || nunits>NUM_UNITS_PER_GROUP);

    ClearGroup(num);
    AddToGroup(units,nunits,num);
}

/**
 **	Remove unit from its group
 **
 **	@param unit	Unit to remove from group.
 */
global void RemoveUnitFromGroup(Unit *unit)
{
    UnitGroup *group;
    int num;
    int i;

    DebugCheck( unit->GroupId==-1 );	// unit doesn't belong to a group
    num=unit->GroupId;

    group=&Groups[num];
    for( i=0; group->Units[i]!=unit; i++ ) {
	;
    }

    DebugCheck( i>=group->NumUnits );	// oops not found

    // This is a clean way that will allow us to add a unit
    // to a group easily, or make an easy array walk...
    if( i<--group->NumUnits ) {
        group->Units[i]=group->Units[group->NumUnits];
    }

    unit->GroupId=-1;
}

//@}
