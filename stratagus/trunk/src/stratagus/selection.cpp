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
/**@name selection.c	-	The units' selection. */
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

global int NumSelected;			/// Number of selected units
global Unit* Selected[MaxSelectable] = {
    NoUnitP,NoUnitP,NoUnitP,
    NoUnitP,NoUnitP,NoUnitP,
    NoUnitP,NoUnitP,NoUnitP
};					/// All selected units

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
 **	Unselect all the units in the current selection
 */
global void UnSelectAll(void)
{
    while( NumSelected ) {
        Selected[--NumSelected]->Selected=0;
        Selected[NumSelected]=NoUnitP;	// FIXME: only needed for old code
    }
}

/**
 **	Replace a group of selected units by an other group of units.
 **
 **	@param units	Array of units to be selected.
 **	@param count	Number of units in array to be selected.
 */
global void ChangeSelectedUnits(Unit** units,int count)
{
    int i;

    DebugCheck( count>MaxSelectable );

    UnSelectAll();
    for( i=0; i<count; i++ ) {
        Selected[i]=units[i];
	Selected[i]->Selected=1;
    }
    NumSelected=count;
}

/**
 **	Add a unit to the other selected units.
 **
 **	@param unit	Pointer to unit to add.
 **	@return		false if NumSelected == MaxSelectable or
 **			unit is already selected, true otherwise.
 */
global int SelectUnit(Unit* unit)
{
    if ( unit->Revealer )
    {
	return 0; // Revealers cannot be selected
    }
    
    if( NumSelected == MaxSelectable ) {
        return 0;
    }

    if( unit->Selected ) {
        return 0;
    }

    Selected[NumSelected++]=unit;
    unit->Selected=1;
    return 1;
}

/**
 **	Select a single unit, unselecting the previous ones
 **
 **	@param unit	Pointer to unit to be selected.
 */
global void SelectSingleUnit(Unit* unit)
{
    ChangeSelectedUnits(&unit,1);
}

/**
 **	Unselect unit
 **
 **	@param unit	Pointer to unit to be unselected.
 */
global void UnSelectUnit(Unit* unit)
{
    int i;

    if( !unit->Selected ) {
        return;
    }

    for( i=0; Selected[i]!=unit; i++) {
	;
    }
    DebugCheck( i>=NumSelected );

    if( i<--NumSelected ) {
        Selected[i]=Selected[NumSelected];
    }
    Selected[NumSelected]=NoUnitP;	// FIXME: only needed for old code
    unit->Selected=0;
}

/**
 **	Toggle the selection of a unit in a group of selected units
 **
 **	@param unit	Pointer to unit to be toggled.
 **	@return		0 if unselected, 1 otherwise
 */
global int ToggleSelectUnit(Unit* unit)
{
    if( unit->Selected ) {
        UnSelectUnit(unit);
	return 0;
    }
    SelectUnit(unit);
    return 1;
}

/**
 **	Select units from a particular type and belonging to the local player.
 **
 **	The base is included in the selection and defines
 **	the type of the other units to be selected.
 **
 **	@param base	Select all units of same type.
 **	@return		Number of units found, 0 means selection unchanged
 **
 **	FIXME: 0 can't happen. Maybe when scripting will use it?
 **
 **	FIXME: should always select the nearest 9 units to the base!
 */
global int SelectUnitsByType(Unit* base)
{
    Unit* unit;
    Unit* table[MAX_UNITS];
    const UnitType* type;
    int r;
    int i;

    type=base->Type;

    DebugLevel3Fn(" (%d)\n",base->UnitType->Type);

    // select all visible units.
    r=SelectUnits(MapX-1,MapY-1,MapX+MapWidth+1,MapY+MapHeight+1,table);

    // if unit is a cadaver or hidden (not on map)
    // no unit can be selected.
    if( base->Removed || base->Command.Action==UnitActionDie ) {
        return 0;
    }

    UnSelectAll();
    Selected[0]=base;
    base->Selected=1;
    NumSelected=1;

    // if unit isn't belonging to the player, or is a static unit
    // (like a building), only 1 unit can be selected at the same time.
    if( base->Player!=ThisPlayer || !type->SelectableByRectangle ) {
        return NumSelected;
    }

    //
    //  Search for other visible units of the same type
    //
    // FIXME: peon/peasant with gold/wood & co are considered from
    //        different type... idem for tankers
    for( i=0; i<r; ++i ) {
	unit=table[i];
	if( unit->Player!=ThisPlayer || unit->Type!=type ) {
	    continue;
	}
	if( UnitUnusable(unit) ) {  // guess SelectUnits doesn't check this
	    continue;
	}
	if( unit==base ) {  // no need to have the same unit twice :)
	    continue;
	}
	Selected[NumSelected++]=unit;
	unit->Selected=1;
	if( NumSelected==MaxSelectable ) {
	    break;
	}
    }

    return NumSelected;
}

/**
 **     Change selected units to units from group #group_number
 **     Doesn't change the selection if the group has no unit.
 **
 **	@param group_number	number of the group to be selected.
 **	@return			number of units in the group.
 */
global int SelectGroup(int group_number)
{
    int nunits;

    DebugCheck(group_number>NUM_GROUPS);

    if( !(nunits=GetNumberUnitsOfGroup(group_number)) ) {
        return 0;
    }

    ChangeSelectedUnits(GetUnitsOfGroup(group_number),nunits);
    return NumSelected;
}

/**
 **     Select units from group of a particular unit.
 **     Doesn't change the selection if the group has no unit,
 **     or the unit doesn't belong to any group.
 **
 **	@param unit	unit belonging to the group to be selected.
 **	@return		0 if the unit doesn't belong to a group,
 **			or the number of units in the group.
 */
global int SelectGroupFromUnit(Unit *unit)
{
    if( unit->GroupId==-1 ) {
        return 0;
    }
    return SelectGroup(unit->GroupId);
}

/**
 **	Select the units selecteable by rectangle in a local table.
 **	Act like a filter: The source table is modified.
 **	Return the original table if no unit is found.
 **
 **	@param table		Input/Output table of units.
 **	@param num_units	Number of units in input table.
 **	@return			the number of units found.
 */
local int SelectOrganicUnitsInTable(Unit** table,int num_units)
{
    Unit* unit;
    int n,i;

    for( n=i=0; i<num_units; i++ ) {
	unit=table[i];
	if( unit->Player!=ThisPlayer || !unit->Type->SelectableByRectangle ) {
	    continue;
	}
	if( UnitUnusable(unit) ) {  // guess SelectUnits doesn't check this
	    continue;
	}
	table[n++]=unit;
	if( n==MaxSelectable ) {
	    break;
	}
    }
    return n;
}

/**
 **     Add the units in the rectangle to the current selection
 **
 **	@param tx	X start of selection rectangle in tile coordinates
 **	@param ty	Y start of selection rectangle in tile coordinates
 **	@param w	Selection rectangle width.
 **	@param h	Selection rectangle height.
 **	@return		the _total_ number of units selected.
 */
global int AddSelectedUnitsInRectangle(int tx,int ty,int w,int h)
{
    Unit* table[MAX_UNITS];
    int toggle_num;
    int n,i;

    //  If there is no selected unit yet, do a simple selection.
    if( !NumSelected ) {
        return SelectUnitsInRectangle(tx, ty, w, h);
    }

    //  If no unit in rectangle area... do nothing
    if( !(toggle_num=SelectUnits(tx,ty,tx+w+1,ty+h+1,table)) ) {
        return NumSelected;
    }

    //  Check if the original selected unit (if it's alone) is ours, 
    //  and can be selectable by rectangle.
    //  In this case, do nothing.
    if( NumSelected == 1 &&
        ( Selected[0]->Player!=ThisPlayer ||
            !Selected[0]->Type->SelectableByRectangle )) {
        return NumSelected;
    }

    //  Now we should only have mobile (organic) units belonging to us,
    //  so if there's no such units in the rectangle, do nothing.
    if( !(n=SelectOrganicUnitsInTable(table,toggle_num)) ) {
        return NumSelected;
    }

    for( i=0; i<n && NumSelected<MaxSelectable; i++ ) {
        SelectUnit(table[i]);
    }
    return NumSelected;
}

/**
 **	Select units in a rectangle. Proceed in order in none found:
 **	  - select local player mobile units
 **	  - select one local player static unit (random)
 **	  - select one neutral unit (critter, mine...)
 **	  - select one enemy unit (random)
 **
 **	@param tx	X start of selection rectangle in tile coordinates
 **	@param ty	Y start of selection rectangle in tile coordinates
 **	@param w	Selection rectangle width.
 **	@param h	Selection rectangle height.
 **	@return		the number of units found.
 */
global int SelectUnitsInRectangle(int tx,int ty,int w,int h)
{
    Unit* unit;
    Unit* table[MAX_UNITS];
    UnitType* type;
    int r;
    int n;
    int i;

    DebugLevel3Fn(" (%d,%d,%d,%d)\n",tx,ty,w,h);

    r=SelectUnits(tx,ty,tx+w+1,ty+h+1,table);

    //
    //	1) search for the player units selectable with rectangle
    //
    if( (n=SelectOrganicUnitsInTable(table,r)) ) {
        ChangeSelectedUnits(table,n);
	return n;
    }

    //
    //	2) If no unit found, try a player's unit not selectable by rectangle
    //
    for( i=0; i<r; ++i ) {
        unit=table[i];
	if( unit->Player!=ThisPlayer ) {
	    continue;
	}
	// FIXME: Can we get this?
	if( !unit->Removed && unit->Command.Action!=UnitActionDie ) { 
	    SelectSingleUnit(unit);
	    return 1;
	}
    }

    //
    //	3) If no unit found, try a resource or a neutral critter
    //
    for( i=0; i<r; ++i ) {
        unit=table[i];
	// Unit visible FIXME: write function UnitSelectable
	if( !UnitVisible(unit) ) {
	    continue;
	}
	type=unit->Type;
	// Buildings are visible but not selectable
	if( type->Building
		&& !(TheMap.Fields[
		    unit->Y*TheMap.Width+unit->X].Flags&MapFieldVisible) ) {
	    // FIXME: isn't it enough to see a field of the building?
	    continue;
	}
	if( type->Critter || type->GoldMine
	      || (type->OilPatch && !unit->Removed) ) {  // no oil platform!
	    SelectSingleUnit(unit);
	    return 1;
	}
    }

    //
    //	4) If no unit found, select an enemy unit (first found)
    //
    for( i=0; i<r; ++i ) {
        unit=table[i];
	// Unit visible FIXME: write function UnitSelectable
	if( !UnitVisible(unit) ) {
	    continue;
	}
	// Buildings are visible but not selectable
	if( unit->Type->Building
		&& !(TheMap.Fields[
		    unit->Y*TheMap.Width+unit->X].Flags&MapFieldVisible) ) {
	    // FIXME: isn't it enough to see a field of the building?
	    continue;
	}
	if( !unit->Removed && unit->Command.Action!=UnitActionDie ) {
	    SelectSingleUnit(unit);
	    return 1;
	}
    }

    return 0;
}

//@}
