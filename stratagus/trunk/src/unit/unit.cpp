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
/**@name unit.c		-	The units. */
//
//	(c) Copyright 1998-2001 by Lutz Sammer
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the License,
//	or (at your option) any later version.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#include "freecraft.h"

#include "video.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "tileset.h"
#include "map.h"
#include "actions.h"
#include "sound_server.h"
#include "missile.h"
#include "interface.h"
#include "sound.h"
#include "ai.h"
#include "pathfinder.h"
#include "network.h"
#include "ui.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

#define LimitSearch 1			/// Limit the search.

global Unit* UnitSlots[MAX_UNIT_SLOTS];	/// All possible units
global Unit** UnitSlotFree;		/// First free unit slot
local Unit* ReleasedHead;		/// List of released units.
local Unit** ReleasedTail;		/// List tail of released units.

global Unit* Units[MAX_UNIT_SLOTS];	/// Array of used slots
global int NumUnits;			/// Number of slots used

global int HitPointRegeneration;	/// Enable hit point regeneration for all units
global int EnableTrainingQueue;		/// Config: training queues enabled

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Initial memory allocation for Units.
*/
global void InitUnitsMemory(void)
{
    Unit** slot;

    // Initialize the "list" of free unit slots

    slot=UnitSlots+MAX_UNIT_SLOTS;
    *--slot=NULL;			// leave the last slot free as no marker
    *--slot=NULL;
    do {
	slot[-1]=(void*)slot;
    } while( --slot>UnitSlots );
    UnitSlotFree=slot;

    ReleasedTail=&ReleasedHead;		// list of unfreed units.
    NumUnits=0;
}

#if 0
/**
**	Free the memory for an unit slot. Update the global slot table.
**	The memory should only be freed, if all references are dropped.
**
**	@param unit	Pointer to unit.
*/
global void FreeUnitMemory(Unit* unit)
{
    Unit** slot;

    //
    //	Remove from slot table
    //
    slot=UnitSlots+unit->Slot;
    DebugCheck( *slot!=unit );

    *slot=(void*)UnitSlotFree;
    free(unit);
}
#endif

/**
**	Release an unit.
**
**	The unit is only released, if all references are dropped.
**
**	@param unit	Pointer to unit.
*/
global void ReleaseUnit(Unit* unit)
{
    DebugLevel2Fn("%d:Unit %p %d `%s'\n",FrameCounter,
	    unit,UnitNumber(unit),unit->Type->Ident);

    DebugCheck( !unit->Type );		// already free.
    DebugCheck( unit->OrderCount!=1 );
    RefsDebugCheck( unit->Orders[0].Goal );

    //
    //	First release, remove from lists/tables.
    //
    if( !unit->Destroyed ) {
	Unit* temp;

	//
	//	Remove the unit from the global units table.
	//
	DebugCheck( *unit->UnitSlot!=unit );
	temp=Units[--NumUnits];
	Units[NumUnits]=NULL;
	temp->UnitSlot=unit->UnitSlot;
	*unit->UnitSlot=temp;
	//
	//	Are more references remaining?
	//
	unit->Destroyed=1;		// mark as destroyed
	RefsDebugCheck( !unit->Refs );
	if( --unit->Refs>0 ) {
	    DebugLevel2Fn("%d:More references of %d #%d\n",FrameCounter
		    ,UnitNumber(unit),unit->Refs);
	    return;
	}
    }

    RefsDebugCheck( unit->Refs );

#ifdef UNIT_ON_MAP
    if( 0 ) {		// debug check
	Unit* list;

	list=TheMap.Fields[unit->Y*TheMap.Width+unit->X].Here.Units;
	while( list ) {			// find the unit
	    if( list==unit ) {
		abort();
	    }
	    list=list->Next;
	}
    }
#endif
    //
    //	No more references remaining, but the network could have an order
    //	on the way. We must wait a little time before we could free the
    //	memory.
    //
    *ReleasedTail=unit;
    ReleasedTail=&unit->Next;
    unit->Refs=FrameCounter+NetworkMaxLag;	// could be reuse after this.
    IfDebug(
	DebugLevel2Fn("%d:No more references %d\n",
		FrameCounter,UnitNumber(unit));
	unit->Type=NULL;			// for debugging.
    );
}

/**
**	Create a new unit.
**
**	@param type	Pointer to unit-type.
**	@param player	Pointer to owning player.
**
**	@return		Pointer to created unit.
*/
global Unit* MakeUnit(UnitType* type,Player* player)
{
    Unit* unit;
    Unit** slot;

    DebugCheck( !player );	// Current code didn't support no player

    DebugLevel3Fn("%s(%d)\n",type->Name,player-Players);

    //
    //	Game unit limit reached.
    //
    if( NumUnits>=UnitMax ) {
	DebugLevel0Fn("Over all unit limit (%d) reached.\n" _C_ UnitMax);
	// FIXME: Hoping this is checked everywhere.
	return NoUnitP;
    }

    //
    //	Can use released unit?
    //
    if( ReleasedHead && ReleasedHead->Refs<FrameCounter ) {
	unit=ReleasedHead;
	ReleasedHead=unit->Next;
	if( ReleasedTail==&unit->Next ) {	// last element
	    ReleasedTail=&ReleasedHead;
	}
	DebugLevel2Fn("%d:Release %p %d\n",FrameCounter,unit,UnitNumber(unit));
	slot=UnitSlots+unit->Slot;
	memset(unit,0,sizeof(*unit));
	// FIXME: can release here more slots, reducing memory needs.
    } else {
	//
	//	Allocate structure
	//
	if( !(slot=UnitSlotFree) ) {	// should not happen!
	    DebugLevel0Fn("Maximum of units reached\n");
	    return NoUnitP;
	}
	UnitSlotFree=(void*)*slot;
	*slot=unit=calloc(1,sizeof(*unit));
    }
    unit->Slot=slot-UnitSlots;		// back index
    unit->Refs=1;

    //
    //	Build all unit table
    //
    unit->UnitSlot=&Units[NumUnits];	// back pointer
    Units[NumUnits++]=unit;

    //
    //	Build player unit table
    //
    if( player ) {
	unit->PlayerSlot=player->Units+player->TotalNumUnits++;
	*unit->PlayerSlot=unit;

	player->UnitTypesCount[type->Type]++;
    }

    DebugLevel3Fn("%p %d\n",unit,UnitNumber(unit));

    //
    //	Initialise unit structure (must be zero filled!)
    //
    unit->Type=type;

    unit->SeenFrame=0xFF;
    if( type->Demand ) {
        player->NumFoodUnits+=type->Demand;	// food needed
	if( player==ThisPlayer ) {
	    MustRedraw|=RedrawResources;	// update food
	}
    }
    if( type->Building ) {
	player->NumBuildings++;
    } else {
        unit->Direction=(MyRand()>>8)&0xFF;	// random heading
    }
    unit->Player=player;
    unit->Stats=&type->Stats[unit->Player->Player];

    if( type->CanCastSpell ) {
	unit->Mana=MAGIC_FOR_NEW_UNITS;
    }
    unit->HP=unit->Stats->HitPoints;
    unit->Active=1;

    unit->GroupId=-1;

    unit->Wait=1;
    unit->Reset=1;
    unit->Removed=1;

    if( type->Submarine ) {
	unit->Visible=0;		// Invisible as default
    } else {
	unit->Visible=-1;		// Visible as default
    }

    unit->Rs=MyRand()%100; // used for random fancy buildings and other things

    unit->OrderCount=1;
    unit->Orders[0].Action=UnitActionStill;
    DebugCheck( unit->Orders[0].Goal );
    unit->NewOrder.Action=UnitActionStill;
    DebugCheck( unit->NewOrder.Goal );
    unit->SavedOrder.Action=UnitActionStill;
    DebugCheck( unit->SavedOrder.Goal );

    DebugCheck( NoUnitP );		// Init fails if NoUnitP!=0

    return unit;
}

/**
**	Place unit on map.
**
**	@param unit	Unit to be placed.
**	@param x	X map tile position.
**	@param y	Y map tile position.
*/
global void PlaceUnit(Unit* unit,int x,int y)
{
    const UnitType* type;
    int h;
    int w;

    DebugCheck( !unit->Removed || unit->Destroyed );

    type=unit->Type;
    //
    //	Sea and air units are 2 tiles aligned
    //
    if( type->UnitType==UnitTypeFly || type->UnitType==UnitTypeNaval ) {
	x&=~1;
	y&=~1;
    }

    unit->X=x;
    unit->Y=y;

    //
    //	Place unit on the map.
    //
    if( type->Building ) {
	//
	//	Mark building on movement map.
	//		buildings that could be entered have no HP!
	//		on Oilpatch could be build.
	//

	// FIXME: should be done more general.
	if( unit->HP ) {
	    for( h=type->TileHeight; h--; ) {
		for( w=type->TileWidth; w--; ) {
		    TheMap.Fields[x+w+(y+h)*TheMap.Width].Flags
			    |=MapFieldBuilding;
		}
	    }
	} else if( !type->OilPatch ) {
	    for( h=type->TileHeight; h--; ) {
		for( w=type->TileWidth; w--; ) {
		    TheMap.Fields[x+w+(y+h)*TheMap.Width].Flags
			    |=MapFieldNoBuilding;
		}
	    }
	}
    } else {
	unsigned flags;

	flags=UnitFieldFlags(unit);
	for( h=type->TileHeight; h--; ) {
	    for( w=type->TileWidth; w--; ) {
		TheMap.Fields[x+w+(y+h)*TheMap.Width].Flags|=flags;
	    }
	}
    }

    x+=unit->Type->TileWidth/2;
    y+=unit->Type->TileHeight/2;

    //
    //	Units under construction have no sight range.
    //
    if( !unit->Constructed ) {
#ifdef NEW_FOW
	//
	//	Update fog of war.
	//
	MapMarkSight(unit->Player,x,y,unit->Stats->SightRange);
#else
	//
	//	Update fog of war, if unit belongs to player on this computer
	//
	if( unit->Player==ThisPlayer ) {
	    MapMarkSight(x,y,unit->Stats->SightRange);
	}
#endif
	if( type->CanSeeSubmarine ) {
	    MarkSubmarineSeen(unit->Player,x,y,unit->Stats->SightRange);
	}
    }

    unit->Removed=0;
    UnitCacheInsert(unit);

    MustRedraw|=RedrawMinimap;
    CheckUnitToBeDrawn(unit);
}

/**
**	Create new unit and place on map.
**
**	@param x	X map tile position.
**	@param y	Y map tile position.
**	@param type	Pointer to unit-type.
**	@param player	Pointer to owning player.
**
**	@return		Pointer to created unit.
*/
global Unit* MakeUnitAndPlace(int x,int y,UnitType* type,Player* player)
{
    Unit* unit;

    unit=MakeUnit(type,player);

    if( type->Building ) {
	//
	//	fancy buildings: mirror buildings (but shadows not correct)
	//
	if ( FancyBuildings && unit->Rs > 50 ) {
	    unit->Frame |= 128;
	}
    }

    PlaceUnit(unit,x,y);

    return unit;
}

/**
**	Remove unit from map.
**
**	Update selection.
**	Update panels.
**	Update map.
**
**	@param unit	Pointer to unit.
*/
global void RemoveUnit(Unit* unit)
{
    int h;
    int w;
    const UnitType* type;

    if( unit->Removed ) {		// could happen!
	// If unit is removed (inside) and building is destroyed.
	return;
    }
    unit->Removed=1;

    //  Remove unit from the current selection
    if( unit->Selected ) {
        if( NumSelected==1 ) {		//  Remove building cursor
	    CancelBuildingMode();
	}
	UnSelectUnit(unit);
	MustRedraw|=RedrawPanels;
	UpdateButtonPanel();
    }

    //  Remove unit from its group
    if( unit->GroupId!=-1 ) {
        RemoveUnitFromGroup(unit);
    }

    // Unit is seen as under cursor
    if( unit==UnitUnderCursor ) {
	UnitUnderCursor=NULL;
    }

    //
    //	Update map
    //
    type=unit->Type;
    if( type->Building ) {
	//
	//	Unmark building on movement map.
	//		buildings that could be entered have no HP!
	//		on Oilpatch could be build.
	//

	// FIXME: should be done more general.
	if( unit->Stats->HitPoints ) {
	    for( h=type->TileHeight; h--; ) {
		for( w=type->TileWidth; w--; ) {
		    TheMap.Fields[unit->X+w+(unit->Y+h)*TheMap.Width].Flags
			    &=~MapFieldBuilding;
		}
	    }
	} else if( !type->OilPatch ) {
	    for( h=type->TileHeight; h--; ) {
		for( w=type->TileWidth; w--; ) {
		    TheMap.Fields[unit->X+w+(unit->Y+h)*TheMap.Width].Flags
			    &=~MapFieldNoBuilding;
		}
	    }
	}
    } else {
	unsigned flags;

	flags=~UnitFieldFlags(unit);
	for( h=type->TileHeight; h--; ) {
	    for( w=type->TileWidth; w--; ) {
		TheMap.Fields[unit->X+w+(unit->Y+h)*TheMap.Width].Flags&=flags;
	    }
	}
    }

    DebugLevel3Fn("%d %p %p\n",UnitNumber(unit),unit,unit->Next);
    UnitCacheRemove(unit);
#ifdef UNIT_ON_MAP
    if( 0 ) {
	Unit* list;

	list=TheMap.Fields[unit->Y*TheMap.Width+unit->X].Here.Units;
	while( list ) {				// find the unit
	    if( list==unit ) {
		DebugLevel0Fn("%d\n",UnitNumber(unit));
		abort();
		break;
	    }
	    list=list->Next;
	}
    }
#endif

    MustRedraw|=RedrawMinimap;
    CheckUnitToBeDrawn(unit);
}

/**
**	Update informations for lost units.
**
**	@param unit	Pointer to unit.
*/
global void UnitLost(Unit* unit)
{
    Unit* temp;
    const UnitType* type;
    Player* player;
    int i;

    DebugCheck( !unit );

    type=unit->Type;
    player=unit->Player;

    //
    //	Call back to AI, for killed units.
    //
    if( player && player->Ai ) {
	AiUnitKilled(unit);
    }

    //
    //	Remove the unit from the player's units table.
    //
    if( player ) {
	DebugCheck( *unit->PlayerSlot!=unit );
	temp=player->Units[--player->TotalNumUnits];
	player->Units[player->TotalNumUnits]=NULL;
	temp->PlayerSlot=unit->PlayerSlot;
	*unit->PlayerSlot=temp;
    }

    //
    //	Handle unit demand. (Currently only food supported.)
    //
    if( type->Demand ) {
	player->NumFoodUnits-=type->Demand;
	if( player==ThisPlayer ) {
	    MustRedraw|=RedrawResources;	// update food
	}
    }

    if( unit->Orders[0].Action!=UnitActionBuilded ) {
	player->UnitTypesCount[type->Type]--;
    }

    if( unit->Type->Building ) {
	// FIXME: This should be complete rewritten
	// FIXME: Slow and new members are available
	// FIXME: most redraws only needed for player==ThisPlayer

	// Still under construction
	if( unit->Orders[0].Action!=UnitActionBuilded ) {
	    if( type->Supply ) {
		player->Food-=type->Supply;
		if( player==ThisPlayer ) {
		    MustRedraw |= RedrawResources;
		}
	    }
	    if( type==UnitTypeByIdent("unit-elven-lumber-mill")
		    || type==UnitTypeByIdent("unit-troll-lumber-mill") ) {

		if( !(HaveUnitTypeByIdent(player,"unit-elven-lumber-mill")
			+HaveUnitTypeByIdent(player
				,"unit-troll-lumber-mill")) ) {
		    player->Incomes[WoodCost]=DEFAULT_INCOMES[WoodCost];
		    if( player==ThisPlayer ) {
			MustRedraw |= RedrawInfoPanel;
		    }
		}
	    } else if( type==UnitTypeByIdent("unit-human-refinery")
		    || type==UnitTypeByIdent("unit-orc-refinery") ) {
		if( !(HaveUnitTypeByIdent(player,"unit-human-refinery")
			+HaveUnitTypeByIdent(player,"unit-orc-refinery")) ) {
		    player->Incomes[OilCost]=DEFAULT_INCOMES[OilCost];
		    if( player==ThisPlayer ) {
			MustRedraw |= RedrawInfoPanel;
		    }
		}
	    } else if( type==UnitTypeByIdent("unit-keep")
		    || type==UnitTypeByIdent("unit-stronghold")
		    || type==UnitTypeByIdent("unit-castle")
		    || type==UnitTypeByIdent("unit-fortress") ) {
		if( !(HaveUnitTypeByIdent(player,"unit-castle")
			+HaveUnitTypeByIdent(player,"unit-fortress")) ) {
		    player->Incomes[GoldCost]=DEFAULT_INCOMES[GoldCost]+10;
		    if( !(HaveUnitTypeByIdent(player,"unit-keep")
			    +HaveUnitTypeByIdent(player,"unit-stronghold")) ) {
			player->Incomes[GoldCost]=DEFAULT_INCOMES[GoldCost];
		    }
		    if( player==ThisPlayer ) {
			MustRedraw |= RedrawInfoPanel;
		    }
		}
	    }
	}
	player->NumBuildings--;
    }

    //
    //	Handle research cancels.
    //
    if( unit->Orders[0].Action == UnitActionResearch ) {
	unit->Player->UpgradeTimers.Upgrades[unit->Data.Research.Upgrade
		-Upgrades]=0;
    }

    DebugLevel3Fn("Lost %s(%d)\n",unit->Type->Ident,UnitNumber(unit));

    //
    //	Destroy oil-platform, must re-make oil patch.
    //
    if( type->GivesOil && unit->Value>0 ) {
	// NOTE: I wasn't sure the best UnitType/Player
	// NOTE: This should really NOT be hardcoded?!
	temp=MakeUnitAndPlace(unit->X,unit->Y
		,UnitTypeByIdent("unit-oil-patch"),&Players[15]);
	temp->Value=unit->Value;
    }

    //
    //	Release all references of the unit.
    //
    for( i=unit->OrderCount; i-->0; ) {
	if( unit->Orders[i].Goal ) {
	    RefsDebugCheck( !unit->Orders[i].Goal->Refs );
	    if( !--unit->Orders[i].Goal->Refs ) {
		RefsDebugCheck( !unit->Orders[i].Goal->Destroyed );
		ReleaseUnit(unit->Orders[i].Goal);
	    }
	    unit->Orders[i].Goal=NoUnitP;
	}
	unit->OrderCount=1;
    }
    if( unit->NewOrder.Goal ) {
	RefsDebugCheck( !unit->NewOrder.Goal->Refs );
	if( !--unit->NewOrder.Goal->Refs ) {
	    DebugCheck( !unit->NewOrder.Goal->Destroyed );
	    ReleaseUnit(unit->NewOrder.Goal);
	}
	unit->NewOrder.Goal=NoUnitP;
    }
    if( unit->SavedOrder.Goal ) {
	RefsDebugCheck( !unit->SavedOrder.Goal->Refs );
	if( !--unit->SavedOrder.Goal->Refs ) {
	    DebugCheck( !unit->SavedOrder.Goal->Destroyed );
	    ReleaseUnit(unit->SavedOrder.Goal);
	}
	unit->SavedOrder.Goal=NoUnitP;
    }

    DebugCheck( player->NumFoodUnits > UnitMax);
    DebugCheck( player->NumBuildings > UnitMax);
    DebugCheck( player->UnitTypesCount[type->Type] > UnitMax);
}

/**
**	Update for new unit. Food and income ...
**
**	@param unit	New unit pointer.
**	@param upgrade	True unit was upgraded.
*/
global void UpdateForNewUnit(const Unit* unit,int upgrade)
{
    const UnitType* type;
    Player* player;

    DebugLevel3Fn("unit %d (%d)\n",UnitNumber(unit),unit->Type->Type);

    player=unit->Player;
    type=unit->Type;

    //
    //	Handle unit demand. (Currently only food supported.)
    //
    if( type->Supply && !upgrade ) {
	player->Food+=type->Supply;
	if( player==ThisPlayer ) {
	    MustRedraw|=RedrawResources;	// update food
	}
    }

    //
    //	Update food and resources
    //
    if( unit->Type->Building ) {

	// FIXME: this is slow, remove the UnitTypeByIdent.
	// FIXME: any ideas to generalize this problems?

	if( type==UnitTypeByIdent("unit-elven-lumber-mill")
		|| type==UnitTypeByIdent("unit-troll-lumber-mill") ) {
	    player->Incomes[WoodCost]=DEFAULT_INCOMES[WoodCost]+25;
	    if( player==ThisPlayer ) {
		MustRedraw |= RedrawInfoPanel;
	    }
	} else if( type==UnitTypeByIdent("unit-human-refinery")
		|| type==UnitTypeByIdent("unit-orc-refinery") ) {
	    player->Incomes[OilCost]=DEFAULT_INCOMES[OilCost]+25;
	    if( player==ThisPlayer ) {
		MustRedraw |= RedrawInfoPanel;
	    }
	} else if( type==UnitTypeByIdent("unit-keep")
		|| type==UnitTypeByIdent("unit-stronghold") ) {
	    if( player->Incomes[GoldCost]==DEFAULT_INCOMES[GoldCost] ) {
		player->Incomes[GoldCost]=DEFAULT_INCOMES[GoldCost]+10;
		if( player==ThisPlayer ) {
		    MustRedraw |= RedrawInfoPanel;
		}
	    }
	} else if( type==UnitTypeByIdent("unit-castle")
		|| type==UnitTypeByIdent("unit-fortress") ) {
	    if( player->Incomes[GoldCost]!=DEFAULT_INCOMES[GoldCost]+20 ) {
		player->Incomes[GoldCost]=DEFAULT_INCOMES[GoldCost]+20;
		if( player==ThisPlayer ) {
		    MustRedraw |= RedrawInfoPanel;
		}
	    }
	}
    }
}

/**
**	Find nearest point of unit.
**
**	@param unit	Pointer to unit.
**	FIXME:		more docs from here
*/
global void NearestOfUnit(const Unit* unit,int tx,int ty,int *dx,int *dy)
{
    int x;
    int y;

    x=unit->X;
    y=unit->Y;

    DebugLevel3("Nearest of (%d,%d) - (%d,%d)\n",tx,ty,x,y);
    if( tx>=x+unit->Type->TileWidth ) {
	*dx=x+unit->Type->TileWidth-1;
    } else if( tx<x ) {
	*dx=x;
    } else {
	*dx=tx;
    }
    if( ty>=y+unit->Type->TileHeight ) {
	*dy=y+unit->Type->TileHeight-1;
    } else if( ty<y ) {
	*dy=y;
    } else {
	*dy=ty;
    }

    DebugLevel3Fn("Goal is (%d,%d)\n",*dx,*dy);
}

/**
**	Mark submarine seen by an submarine detector.
**
**	@param player	Player pointer that can see the submarine
**	@param x	X map tile center position
**	@param y	Y map tile center position
**	@param r	Range arround center
*/
global void MarkSubmarineSeen(const Player* player,int x,int y,int r)
{
    Unit* table[UnitMax];
    int n;
    int i;

    n=SelectUnits(x-r,y-r,x+r,y+r,table);
    for( i=0; i<n; ++i ) {
	table[i]->Visible|=(1<<player->Player);
    }
}

/**
**	Returns true, if unit is visible for this player on the map.
**	An unit is visible, if any field could be seen.
**
**	@warning	This is only true for ::ThisPlayer.
**
**	@param unit	Unit to be checked.
**	@return		True if visible, false otherwise.
*/
global int UnitVisibleOnMap(const Unit* unit)
{
#ifdef NEW_FOW
    unsigned x;
    unsigned y;
    int w;
    int w0;
    int h;

    DebugCheck( !unit->Type );	// FIXME: Can this happen, if yes it is a bug
    //DebugCheck( unit->Player==ThisPlayer );

    //
    //	Unit invisible (by spell), removed or submarine.
    //
    if ( unit->Invisible || unit->Removed
	    || !(unit->Visible&(1<<ThisPlayer->Player)) ) {
	return 0;
    }

    x = unit->X;
    y = unit->Y;
    w = w0 = unit->Type->TileWidth;
    h = unit->Type->TileHeight;

    //
    //	Check if visible, not under fog of war.
    //		FIXME: need only check the boundary, not the complete rectangle.
    //
    for( ; h-->0; ) {
	for( w=w0; w-->0; ) {
	    if( IsMapFieldVisible(x+w,y+h) ) {
		return 1;
	    }
	}
    }

    return 0;
#else
    unsigned x;
    unsigned y;
    int w;
    int w0;
    int h;

    DebugCheck( !unit->Type );	// FIXME: Can this happen, if yes it is a bug

    //
    //	Unit invisible (by spell), removed or submarine.
    //
    if ( unit->Invisible || unit->Removed
	    || !(unit->Visible&(1<<ThisPlayer->Player)) ) {
	return 0;
    }

    x = unit->X;
    y = unit->Y;
    w = w0 = unit->Type->TileWidth;
    h = unit->Type->TileHeight;

    //
    //	Check if visible, not under fog of war.
    //		FIXME: need only check the boundary, not the complete rectangle.
    //
    for( ; h-->0; ) {
	for( w=w0; w-->0; ) {
	    if( IsMapFieldVisible(x+w,y+h) ) {
		return 1;
	    }
	}
    }

    return 0;
#endif
}

/**
**	Returns true, if unit is known on the map. Special case for buildings.
**
**	@param unit	Unit to be checked.
**	@return		True if known, false otherwise.
*/
global int UnitKnownOnMap(const Unit* unit)
{
    unsigned x;
    unsigned y;
    int w;
    int w0;
    int h;

    DebugCheck( !unit->Type );	// FIXME: Can this happen, if yes it is a bug

    if( unit->Player != ThisPlayer ) {
	//FIXME: vladi: should handle teams and shared vision
	// Invisible by spell
	if ( unit->Invisible ) {
	    return 0;
	}
	// Visible submarine
	if ( !(unit->Visible&(1<<ThisPlayer->Player)) ) {
	    return 0;
	}
    }

    //
    //	Check if visible on screen.
    //		FIXME: This could be better checked, tells to much!
    //		FIXME: This is needed to show moving units.
    //		FIXME: flyers disappears to fast.
    //
    x = unit->X;
    y = unit->Y;
    w = w0 = unit->Type->TileWidth;
    h = unit->Type->TileHeight;
    //
    //	Check explored or if visible (building) under fog of war.
    //		FIXME: need only check the boundary, not the complete rectangle.
    //
    for( ; h-->0; ) {
	for( w=w0; w-->0; ) {
	    if( IsMapFieldVisible(x+w,y+h)
		    || (unit->Type->Building && unit->SeenFrame!=0xFF
			&& IsMapFieldExplored(x+w,y+h)) ) {
		return 1;
	    }
	}
    }

    return 0;
}

/**
**	Returns true, if unit is visible on current map view.
**
**	@param unit	Unit to be checked.
**	@return		True if visible, false otherwise.
*/
global int UnitVisibleOnScreen(const Unit* unit)
{
    unsigned x;
    unsigned y;
    int w;
    int w0;
    int h;

    DebugCheck( !unit->Type );	// FIXME: Can this happen, if yes it is a bug

    if (!ThisPlayer) {
	//FIXME: ARI: Added here for early game setup state by
	//	MakeAndPlaceUnit() from LoadMap(). ThisPlayer not yet set,
	//	so don't show anything until first real map-draw.
	return 0;
    }

    if( unit->Player != ThisPlayer ) {
	//FIXME: vladi: should handle teams and shared vision
	// Invisible by spell
	if ( unit->Invisible ) {
	    return 0;
	}
	// Visible submarine
	if ( !(unit->Visible&(1<<ThisPlayer->Player)) ) {
	    return 0;
	}
    }

    //
    //	Check if visible on screen.
    //		FIXME: This could be better checked, tells to much!
    //		FIXME: This is needed to show moving units.
    //		FIXME: flyers disappears to fast.
    //
    x = unit->X;
    y = unit->Y;
    w = w0 = unit->Type->TileWidth;
    h = unit->Type->TileHeight;
    if( (x+w) < MapX || x > (MapX+MapWidth)
	    || (y+h) < MapY || y > (MapY+MapHeight) ) {
	return 0;
    }

    //
    //	Check explored or if visible (building) under fog of war.
    //		FIXME: need only check the boundary, not the complete rectangle.
    //
    for( ; h-->0; ) {
	for( w=w0; w-->0; ) {
	    if( IsMapFieldVisible(x+w,y+h)
		    || (unit->Type->Building && unit->SeenFrame!=0xFF
			&& IsMapFieldExplored(x+w,y+h)) ) {
		return 1;
	    }
	}
    }

    return 0;
}

/**
**      StephanR: Get area of tiles covered by unit, including its displacement
**
**      @param unit     Unit to be checked and set.
**      @return         sx,sy,ex,ey defining area in Map
*/
global void GetUnitMapArea( const Unit* unit,
                            int *sx, int *sy, int *ex, int *ey )
{
    *sx = unit->X - (unit->IX < 0);
    *ex = *sx + unit->Type->TileWidth - !unit->IX;
    *sy = unit->Y - (unit->IY < 0);
    *ey = *sy + unit->Type->TileHeight - !unit->IY;
}

/**
**      Check and sets if unit must be drawn on screen-map
**
**      @param unit     Unit to be checked.
**      @return         True if map marked to be drawn, false otherwise.
*/
global int CheckUnitToBeDrawn(const Unit * unit)
{
#ifdef NEW_MAPDRAW
    int sx, sy, ex, ey;

    // in debug-mode check unsupported displacement exceeding an entire Tile
    // FIXME: displacement could always be made positive and smaller than Tile
#if NEW_MAPDRAW > 1
    if (unit->IX <= -TileSizeX || unit->IX >= TileSizeX
	    || unit->IY <= -TileSizeY || unit->IY >= TileSizeY) {
	printf("internal error in CheckUnitToBeDrawn\n");
    }
#endif

    GetUnitMapArea(unit, &sx, &sy, &ex, &ey);

    // FIXME: extra tiles added here for attached statusbar/mana/shadow/..
    sx--;
    sy--;
    ex++;
    ey++;

    if (MarkDrawAreaMap(sx, sy, ex, ey)) {
	//  MustRedraw|=RedrawMinimap;
	return 1;
    }
#else
    if (UnitVisibleOnScreen(unit)) {
	MustRedraw |= RedrawMap;
	return 1;
    }
#endif
    return 0;
}

// FIXME: perhaps I should write a function UnitSelectable?

/**
**	Increment mana of all magic units. Called each second.
**	Also clears the blink flag.
**
**	NOTE: we could build a table of all magic units reducing cpu use.
*/
//FIXME: vladi: the doc says incrementing mana is done by 1 per second
//       the spells effect can be decremented at the same time and this
//       will reduse calls to this function to one time per second only!
global void UnitIncrementMana(void)
{
    Unit** table;
    Unit* unit;
    int flag;

    for( table=Units; table<Units+NumUnits; table++ ) {
	unit=*table;
	// FIXME: This isn't the correct position or the correct function name
	if( unit->Blink ) {		// clear blink flag
	    --unit->Blink;
	}

	if( unit->Type->CanCastSpell && unit->Mana!=255 ) {
	    unit->Mana++;

	    // some frames delayed done my color cycling
	    if( 0 ) {
                CheckUnitToBeDrawn(unit);
	    }
	    if( unit->Selected ) {
		MustRedraw|=RedrawInfoPanel;
	    }
	}

	//
	//	Look if the time to live is over.
	//
	if( unit->TTL && unit->TTL<FrameCounter ) {
	    DebugLevel0Fn("Unit must die %d %d!\n",unit->TTL,FrameCounter);
	    //if( !--unit->HP ) { FIXME: must reduce hp the last seconds of life
		LetUnitDie(unit);
	    //}
	    // FIXME: this can modify my table, some units are than skipped!
	    continue;
	}

	// some frames delayed done my color cycling
	flag=1;
	//
	// decrease spells effects time, if end redraw unit.
	//
	if ( unit->Bloodlust ) {
	    unit->Bloodlust--;
	    if( !flag && !unit->Bloodlust ) {
                flag=CheckUnitToBeDrawn(unit);
	    }
	}
	if ( unit->Haste ) {
	    unit->Haste--;
	    if( !flag && !unit->Haste ) {
                flag=CheckUnitToBeDrawn(unit);
	    }
	}
	if ( unit->Slow ) {
	    unit->Slow--;
	    if( !flag && !unit->Slow ) {
                flag=CheckUnitToBeDrawn(unit);
	    }
	}
	if ( unit->Invisible ) {
	    unit->Invisible--;
	    if( !flag && !unit->Invisible ) {
                flag=CheckUnitToBeDrawn(unit);
	    }
	}
	if ( unit->UnholyArmor ) {
	    unit->UnholyArmor--;
	    if( !flag && !unit->UnholyArmor ) {
                flag=CheckUnitToBeDrawn(unit);
	    }
	}
	DebugLevel3Fn("%d:%d,%d,%d,%d,%d\n",UnitNumber(unit),
		unit->Bloodlust,unit->Haste,unit->Slow,unit->Invisible,
		unit->UnholyArmor);

	if (  unit->Type->Submarine ) {
	    if( !flag && (unit->Visible&(1<<ThisPlayer->Player)) ) {
                flag=CheckUnitToBeDrawn(unit);
	    }
	    unit->Visible=0;
	}
    }
}

/**
**	Increment health of all regenerating units. Called each second.
**
**	@note:	We could build a table of all regenerating units reducing cpu
**		use. Also the berserker unit-type and upgrade regeneration
**		should be initialized in some init function.
**		Any idea how to handle this more general? It whould be nice
**		to have more units that could regenerate.
*/
global void UnitIncrementHealth(void)
{
    Unit** table;
    Unit* unit;
    static UnitType* berserker;
    static int regeneration;

    if( !berserker ) {			// FIXME: can move to init code!
	berserker=UnitTypeByIdent("unit-berserker");
	regeneration=UpgradeIdByIdent("upgrade-berserker-regeneration");
    }

    for( table=Units; table<Units+NumUnits; table++ ) {
	unit=*table;
	if (HitPointRegeneration &&  unit->HP<unit->Stats->HitPoints) {
	    ++unit->HP;

	    if( 0 ) {		// some frames delayed done my color cycling
		CheckUnitToBeDrawn(unit);
	    }
	    if( unit->Selected ) {
		MustRedraw|=RedrawInfoPanel;
	    }
	}
	if( unit->Type==berserker
		&& unit->HP<unit->Stats->HitPoints
		&& UpgradeIdAllowed(unit->Player,regeneration)=='R' ) {
	    ++unit->HP;			// FIXME: how fast do we regenerate

	    // some frames delayed done my color cycling
	    if( 0 ) {
                CheckUnitToBeDrawn(unit);
	    }
	    if( unit->Selected ) {
		MustRedraw|=RedrawInfoPanel;
	    }
	}
    }
}

/**
**	Change the unit's owner
**
**	@param oldplayer	Old owning player.
**	@param newplayer	New owning player.
**
**	@todo	FIXME: I think here are some failures, if building is build
**		what is with the unit inside? or a main hall with workers
**		inside?
*/
global void ChangeUnitOwner(Unit* unit,Player* oldplayer,Player* newplayer)
{
    int i;

    // For st*rcr*ft (dark archons),
    if( unit->Type->Transporter ) {
        for( i=0; i<MAX_UNITS_ONBOARD; i++) {
	    if( unit->OnBoard[i] ) {
	        ChangeUnitOwner(unit->OnBoard[i],oldplayer,newplayer);
	    }
	}
    }

    //
    //	Must change food/gold and other.
    //
    UnitLost(unit);

#if 0
    //	Remove from old player table

    temp=oldplayer->Units[--oldplayer->TotalNumUnits];
    oldplayer->Units[oldplayer->TotalNumUnits]=NULL;
    temp->PlayerSlot=unit->PlayerSlot;
    *unit->PlayerSlot=temp;
#endif

    //
    //	Now the new side!
    //

    //	Insert into new player table.

    unit->PlayerSlot=newplayer->Units+newplayer->TotalNumUnits++;
    *unit->PlayerSlot=unit;

    unit->Player=newplayer;

    //
    //	Must change food/gold and other.
    //
    if( unit->Type->GivesOil ) {
	DebugLevel0Fn("oil platform transfer unsupported\n");
    }
    if( !unit->Type->Building ) {
	newplayer->NumFoodUnits+=unit->Type->Demand;
	if( newplayer==ThisPlayer ) {
	    MustRedraw|=RedrawResources;// update food
	}
    } else {
	newplayer->NumBuildings++;
    }
    newplayer->UnitTypesCount[unit->Type->Type]++;

    UpdateForNewUnit(unit,0);
}

/**
**	Change the owner of all units of a player.
**
**	@param oldplayer	Old owning player.
**	@param newplayer	New owning player.
*/
local void ChangePlayerOwner(Player* oldplayer,Player* newplayer)
{
    Unit* table[UnitMax];
    Unit* unit;
    int i;
    int n;

    // NOTE: table is changed.
    n=oldplayer->TotalNumUnits;
    memcpy(table,oldplayer->Units,n*sizeof(Unit*));
    for( i=0; i<n; i++ ) {
	unit=table[i];
	ChangeUnitOwner(unit,oldplayer,newplayer);
    }
}

/**
**	Rescue units.
*/
global void RescueUnits(void)
{
    Player* p;
    Unit* unit;
    Unit* table[UnitMax];
    Unit* near[UnitMax];
    int n;
    int i;
    int j;
    int l;
    static int norescue;

    if( norescue ) {			// all possible units are rescued
	return;
    }
    norescue=1;

    //
    //	Look if player could be rescued.
    //
    for( p=Players; p<Players+NumPlayers; ++p ) {
	if( p->Type!=PlayerRescuePassive && p->Type!=PlayerRescueActive ) {
	    continue;
	}
	if( p->TotalNumUnits ) {
	    norescue=0;
	    // NOTE: table is changed.
	    l=p->TotalNumUnits;
	    memcpy(table,p->Units,l*sizeof(Unit*));
	    for( j=0; j<l; j++ ) {
		unit=table[j];
		DebugLevel3("Checking %d\n",UnitNumber(unit));
#ifdef UNIT_ON_MAP
		// FIXME: could be done faster?
#endif
		// FIXME: I hope SelectUnits checks bounds?
		// FIXME: Yes, but caller should check.
		// NOTE: +1 right,bottom isn't inclusive :(
		if( unit->Type->UnitType==UnitTypeLand ) {
		    n=SelectUnits(
			unit->X-1,unit->Y-1,
			unit->X+unit->Type->TileWidth+1,
			unit->Y+unit->Type->TileHeight+1,near);
		} else {
		    n=SelectUnits(
			unit->X-2,unit->Y-2,
			unit->X+unit->Type->TileWidth+2,
			unit->Y+unit->Type->TileHeight+2,near);
		}
		//
		//	Look if human near the unit.
		//
		for( i=0; i<n; ++i ) {
		    if( near[i]->Player->Type==PlayerHuman ) {
			ChangeUnitOwner(unit,unit->Player,near[i]->Player);
			// FIXME: more races?
			if( unit->Player->Race==PlayerRaceHuman ) {
			    PlayGameSound(GameSounds.HumanRescue.Sound
				    ,MaxSampleVolume);
			} else {
			    PlayGameSound(GameSounds.OrcRescue.Sound
				    ,MaxSampleVolume);
			}
			//
			//	City center converts complete race
			//	NOTE: I use a trick here, centers could
			//		store gold.
			if( unit->Type->StoresGold ) {
			    ChangePlayerOwner(p,unit->Player);
			}
			break;
		    }
		}
	    }
	}
    }
}

/*----------------------------------------------------------------------------
--	Unit headings
----------------------------------------------------------------------------*/

/**
**	Fast arc tangent function.
**
**	@param val	atan argument
**
**	@return		atan(val)
*/
local int myatan(int val)
{
    static int init;
    static unsigned char atan_table[2608];

    if( val>=2608 ) {
	return 63;
    }
    if( !init ) {
	for( ; init<2608; ++init ) {
	    atan_table[init]=atan((double)init/64)*(64*4/6.2831853);
	}
    }

    return atan_table[val];
}

/**
**	Convert direction to heading.
**
**	@param delta_x	Delta X.
**	@param delta_y	Delta Y.
**
**	@return		Angle (0..255)
*/
global int DirectionToHeading(int delta_x,int delta_y)
{
    //
    //	Check which quadrant.
    //
    if( delta_x>0 ) {
	if( delta_y<0 ) {	// Quadrant 1?
	    return myatan((delta_x*64)/-delta_y);
	}
				// Quadrant 2?
	return myatan((delta_y*64)/delta_x)+64;
    }
    if( delta_y>0 ) {		// Quadrant 3?
	return myatan((delta_x*-64)/delta_y)+64*2;
    }
    if( delta_x ) {		// Quadrant 4.
	return myatan((delta_y*-64)/-delta_x)+64*3;
    }
    return 0;
}

/**
**	Update sprite frame for new heading.
*/
global void UnitUpdateHeading(Unit* unit)
{
    int dir;

    // FIXME: depends on the possible unit directions wc 8, sc 32
    unit->Frame&=127;
    unit->Frame/=5;
    unit->Frame*=5;		// Remove heading, keep animation frame
    dir=((unit->Direction+NextDirection/2)&0xFF)/NextDirection;
    if( dir<=LookingS/NextDirection ) {	// north->east->south
	unit->Frame+=dir;
    } else {
	// Note: 128 is the flag for flip graphic in X.
	unit->Frame+=128+256/NextDirection-dir;
    }
}

/**
**	Change unit heading/frame from delta direction x,y.
*
**	@param unit	Unit for new direction looking.
**	@param dx	X map tile delta direction.
**	@param dy	Y map tile delta direction.
*/
global void UnitHeadingFromDeltaXY(Unit* unit,int dx,int dy)
{
    unit->Direction=DirectionToHeading(dx,dy);
    UnitUpdateHeading(unit);
}

/*----------------------------------------------------------------------------
--	Drop out units
----------------------------------------------------------------------------*/

/**
**	Reappear unit on map.
**
**	@param unit	Unit to drop out.
**	@param heading	Direction in which the unit should appear.
**	@param addx	Tile size in x.
**	@param addy	Tile size in y.
*/
global void DropOutOnSide(Unit* unit,int heading,int addx,int addy)
{
    int x;
    int y;
    int i;
    int mask;

    DebugCheck( !unit->Removed );

    x=unit->X;
    y=unit->Y;
    mask=UnitMovementMask(unit);

    if( heading<LookingNE || heading>LookingNW) {
	x+=addx-1;
	--y;
	goto startn;
    }
    if( heading<LookingSE ) {
	x+=addx;
	y+=addy-1;
	goto starte;
    }
    if( heading<LookingSW ) {
	y+=addy;
	goto starts;
    }
    --x;
    goto startw;

    // FIXME: don't search outside of the map
    for( ;; ) {
startw:
	for( i=addy; i--; y++ ) {
#ifdef NEW_SHIPS
	    if( unit->Type->UnitType!=UnitTypeLand && ((x&1) || (y&1)) ) {
		continue;
	    }
#endif
	    if( CheckedCanMoveToMask(x,y,mask) ) {
		goto found;
	    }
	}
	++addx;
starts:
	for( i=addx; i--; x++ ) {
#ifdef NEW_SHIPS
	    if( unit->Type->UnitType!=UnitTypeLand && ((x&1) || (y&1)) ) {
		continue;
	    }
#endif
	    if( CheckedCanMoveToMask(x,y,mask) ) {
		goto found;
	    }
	}
	++addy;
starte:
	for( i=addy; i--; y-- ) {
#ifdef NEW_SHIPS
	    if( unit->Type->UnitType!=UnitTypeLand && ((x&1) || (y&1)) ) {
		continue;
	    }
#endif
	    if( CheckedCanMoveToMask(x,y,mask) ) {
		goto found;
	    }
	}
	++addx;
startn:
	for( i=addx; i--; x-- ) {
#ifdef NEW_SHIPS
	    if( unit->Type->UnitType!=UnitTypeLand && ((x&1) || (y&1)) ) {
		continue;
	    }
#endif
	    if( CheckedCanMoveToMask(x,y,mask) ) {
		goto found;
	    }
	}
	++addy;
    }

found:
    unit->X=x;
    unit->Y=y;

    unit->Wait=1;		// should be correct unit has still action

    // FIXME: Should I use PlaceUnit here?
    UnitCacheInsert(unit);
    // FIXME: This only works with 1x1 big units
    DebugCheck( unit->Type->TileWidth!=1 || unit->Type->TileHeight!=1 );
    TheMap.Fields[x+y*TheMap.Width].Flags|=UnitFieldFlags(unit);

    unit->Removed=0;

    x+=unit->Type->TileWidth/2;
    y+=unit->Type->TileHeight/2;
#ifdef NEW_FOW
    //
    //	Update fog of war.
    //
    MapMarkSight(unit->Player,x,y,unit->Stats->SightRange);
#else
    //
    //	Update fog of war, if unit belongs to player on this computer
    //
    if( unit->Player==ThisPlayer ) {
	MapMarkSight(x,y,unit->Stats->SightRange);
    }
#endif
    if( unit->Type->CanSeeSubmarine ) {
	MarkSubmarineSeen(unit->Player,x,y,unit->Stats->SightRange);
    }

    MustRedraw|=RedrawMinimap;
    CheckUnitToBeDrawn(unit);
}

/**
**	Reappear unit on map nearest to x,y.
**
**	@param unit	Unit to drop out.
**	@param gx	Goal X map tile position.
**	@param gy	Goal Y map tile position.
**	@param addx	Tile size in x.
**	@param addy	Tile size in y.
*/
global void DropOutNearest(Unit* unit,int gx,int gy,int addx,int addy)
{
    int x;
    int y;
    int i;
    int n;
    int bestx;
    int besty;
    int bestd;
    int mask;

    DebugLevel3Fn("%d\n",UnitNumber(unit));
    DebugCheck( !unit->Removed );

    x=unit->X;
    y=unit->Y;
    mask=UnitMovementMask(unit);

    bestd=99999;
    IfDebug( bestx=besty=0; );		// keep the compiler happy

    // FIXME: if we reach the map borders we can go fast up, left, ...
    --x;
    for( ;; ) {
	for( i=addy; i--; y++ ) {	// go down
	    if( CheckedCanMoveToMask(x,y,mask) ) {
		n=MapDistance(gx,gy,x,y);
		DebugLevel3("Distance %d,%d %d\n",x,y,n);
		if( n<bestd ) {
		    bestd=n;
		    bestx=x;
		    besty=y;
		}
	    }
	}
	++addx;
	for( i=addx; i--; x++ ) {	// go right
	    if( CheckedCanMoveToMask(x,y,mask) ) {
		n=MapDistance(gx,gy,x,y);
		DebugLevel3("Distance %d,%d %d\n",x,y,n);
		if( n<bestd ) {
		    bestd=n;
		    bestx=x;
		    besty=y;
		}
	    }
	}
	++addy;
	for( i=addy; i--; y-- ) {	// go up
	    if( CheckedCanMoveToMask(x,y,mask) ) {
		n=MapDistance(gx,gy,x,y);
		DebugLevel3("Distance %d,%d %d\n",x,y,n);
		if( n<bestd ) {
		    bestd=n;
		    bestx=x;
		    besty=y;
		}
	    }
	}
	++addx;
	for( i=addx; i--; x-- ) {	// go left
	    if( CheckedCanMoveToMask(x,y,mask) ) {
		n=MapDistance(gx,gy,x,y);
		DebugLevel3("Distance %d,%d %d\n",x,y,n);
		if( n<bestd ) {
		    bestd=n;
		    bestx=x;
		    besty=y;
		}
	    }
	}
	if( bestd!=99999 ) {
	    unit->X=bestx;
	    unit->Y=besty;

	    unit->Wait=1;		// unit should have action still
	    // FIXME: Should I use PlaceUnit here?

	    // FIXME: This only works with 1x1 big units
	    DebugCheck( unit->Type->TileWidth!=1 || unit->Type->TileHeight!=1 );
	    TheMap.Fields[bestx+besty*TheMap.Width].Flags|=UnitFieldFlags(unit);

	    unit->Removed=0;
	    UnitCacheInsert(unit);

	    // {FIXME: Should make a general function for this
	    bestx+=unit->Type->TileWidth/2;
	    besty+=unit->Type->TileHeight/2;
#ifdef NEW_FOW
	    //
	    //	Update fog of war.
	    //
	    MapMarkSight(unit->Player,bestx,besty,unit->Stats->SightRange);
#else
	    //
	    //	Update fog of war, if unit belongs to player on this computer
	    //
	    if( unit->Player==ThisPlayer ) {
		MapMarkSight(bestx,besty,unit->Stats->SightRange);
	    }
#endif
	    if( unit->Type->CanSeeSubmarine ) {
		MarkSubmarineSeen(unit->Player,bestx,besty,
			unit->Stats->SightRange);
	    }
	    // }FIXME: Should make a general function for this

	    MustRedraw|=RedrawMinimap;
            CheckUnitToBeDrawn(unit);
	    return;
	}
	++addy;
    }
}

/**
**	Drop out all units inside unit.
**
**	@param source	All units inside source are dropped out.
*/
global void DropOutAll(const Unit* source)
{
    // FIXME: Rewrite this use source->Next;
    Unit** table;
    Unit* unit;

    for( table=Units; table<Units+NumUnits; table++ ) {
	unit=*table;
	if( unit->Removed && unit->X==source->X && unit->Y==source->Y ) {
	    DropOutOnSide(unit,LookingW
		    ,source->Type->TileWidth,source->Type->TileHeight);
	    DebugCheck( unit->Orders[0].Goal );
	    unit->Orders[0].Action=UnitActionStill;
	    unit->Wait=unit->Reset=1;
	    unit->SubAction=0;
	}
    }
}

/*----------------------------------------------------------------------------
--	Building units
----------------------------------------------------------------------------*/

/**
**	Can build unit here.
**		Hall to near to goldmine.
**		Refinery or shipyard to near to oil patch.
**
**	@param type	unit-type to be checked.
**	@param x	Map X position.
**	@param y	Map Y position.
**	@return		True if could build here, otherwise false.
*/
global int CanBuildHere(const UnitType* type,unsigned x,unsigned y)
{
    Unit* table[UnitMax];
    int n;
    int i;
    Unit* unit;
    int dx;
    int dy;

    //
    //	Can't build outside the map
    //
    if( x+type->TileWidth>TheMap.Width ) {
	return 0;
    }
    if( y+type->TileHeight>TheMap.Height ) {
	return 0;
    }

    if( type->StoresGold ) {
	//
	//	Gold deposit can't be build too near to gold-mine.
	//
	// FIXME: use unit-cache here.
        int i;

	for( i=0; i<NumUnits; i++ ) {
	    unit=Units[i];
	    if( unit->Type->GoldMine ) {
		DebugLevel3("Check goldmine %d,%d\n",unit->X,unit->Y);
		if( unit->X<x ) {
		    dx=x-unit->X-unit->Type->TileWidth;
		} else {
		    dx=unit->X-x-type->TileWidth;
		}
		if( unit->Y<y ) {
		    dy=y-unit->Y-unit->Type->TileHeight;
		} else {
		    dy=unit->Y-y-type->TileHeight;
		}
		DebugLevel3("Distance %d,%d\n",dx,dy);
		if( dx<GOLDMINE_DISTANCE && dy<GOLDMINE_DISTANCE ) {
		    return 0;
		}
	    }
	}
	return 1;
    }

    // Must be checked before oil!
    if( type->ShoreBuilding ) {
	int h;
	int w;

	DebugLevel3("Shore building\n");
	// Need atleast one coast tile
	for( h=type->TileHeight; h--; ) {
	    for( w=type->TileWidth; w--; ) {
		if( TheMap.Fields[x+w+(y+h)*TheMap.Width].Flags
			    &MapFieldCoastAllowed ) {
		    goto next;
		}
	    }
	}
	return 0;
    }

next:
    if( type->StoresOil ) {
	//
	//	Oil deposit can't be build too near to oil-patch.
	//
	// FIXME: use unit-cache here.
	int i;

	for( i=0; i<NumUnits; i++ ) {
	    unit=Units[i];
	    if( unit->Type->OilPatch ) {
	      DebugLevel3("Check oilpatch %d,%d\n"
			  ,unit->X,unit->Y);
	      if( unit->X<x ) {
		dx=x-unit->X-unit->Type->TileWidth;
	      } else {
		dx=unit->X-x-type->TileWidth;
	      }
	      if( unit->Y<y ) {
		dy=y-unit->Y-unit->Type->TileHeight;
	      } else {
		dy=unit->Y-y-type->TileHeight;
	      }
	      DebugLevel3("Distance %d,%d\n",dx,dy);
	      if( dx<OILPATCH_DISTANCE && dy<OILPATCH_DISTANCE ) {
		return 0;
	      }
	    }
	}
    }

    if( type->GivesOil ) {
	//
	//	Oil platform could only be build on oil-patch.
	//
	// FIXME: Can I use here OilPatchOnMap?
	n=UnitCacheSelect(x,y,x+1,y+1,table);
	for( i=0; i<n; ++i ) {
	    if( !table[i]->Type->OilPatch ) {
		continue;
	    }
	    if( table[i]->X==x && table[i]->Y==y ) {
		return 1;
	    }
	}

	return 0;
    }

    return 1;
}

/**
**	Can build on this point.
*/
global int CanBuildOn(int x,int y,int mask)
{
    if( x<0 || y<0 || x>=TheMap.Width || y>=TheMap.Height ) {
	return 0;
    }
    return (TheMap.Fields[x+y*TheMap.Width].Flags&mask) ? 0 : 1;
}

/**
**	Can build unit-type on this point.
**
**	@param unit	Worker that want to build the building.
**	@param type	Building unit-type.
**	@param x	X tile map position.
**	@param y	Y tile map position.
**	@return		True if the building could be build..
*/
global int CanBuildUnitType(const Unit* unit,const UnitType* type,int x,int y)
{
    int w;
    int h;
    int j;
    int mask;

    //
    //	Remove unit that is building!
    //
    j=UnitFieldFlags(unit);
    // This only works with 1x1 big units
    TheMap.Fields[unit->X+unit->Y*TheMap.Width].Flags&=~j;

    // FIXME: Should be moved into unittype structure, and allow more types.
    if( type->ShoreBuilding ) {
	mask=MapFieldLandUnit
		| MapFieldSeaUnit
		| MapFieldBuilding	// already occuppied
		| MapFieldWall
		| MapFieldRocks
		| MapFieldForest	// wall,rock,forest not 100% clear?
		| MapFieldLandAllowed	// can't build on this
		//| MapFieldUnpassable	// FIXME: I think shouldn't be used
		| MapFieldNoBuilding;
    } else switch( type->UnitType ) {
	case UnitTypeLand:
	    mask=MapFieldLandUnit
		| MapFieldBuilding	// already occuppied
		| MapFieldWall
		| MapFieldRocks
		| MapFieldForest	// wall,rock,forest not 100% clear?
		| MapFieldCoastAllowed
		| MapFieldWaterAllowed	// can't build on this
		| MapFieldUnpassable	// FIXME: I think shouldn't be used
		| MapFieldNoBuilding;
	    break;
	case UnitTypeNaval:
	    mask=MapFieldSeaUnit
		| MapFieldBuilding	// already occuppied
		| MapFieldCoastAllowed
		| MapFieldLandAllowed	// can't build on this
		| MapFieldUnpassable	// FIXME: I think shouldn't be used
		| MapFieldNoBuilding;
	    break;
	case UnitTypeFly:
	default:
	    DebugLevel1Fn("Were moves this unit?\n");
	    return 0;
    }

    for( h=type->TileHeight; h--; ) {
	for( w=type->TileWidth; w--; ) {
	    if( !CanBuildOn(x+w,y+h,mask) ) {
		TheMap.Fields[unit->X+unit->Y*TheMap.Width].Flags|=j;
		return 0;
	    }
	}
    }
    TheMap.Fields[unit->X+unit->Y*TheMap.Width].Flags|=j;

    //
    //	We can build here: check distance to gold mine/oil patch!
    //
    return CanBuildHere(type,x,y);
}

/*----------------------------------------------------------------------------
--	Finding units
----------------------------------------------------------------------------*/

/**
**	Find the nearest gold mine for unit from x,y.
**
**	@param unit	Pointer for source unit.
**	@param x	X tile position to start.
**	@param y	Y tile position to start.
**
**	@return		Pointer to the nearest gold mine.
*/
global Unit* FindGoldMine(const Unit* source __attribute__((unused)),
	int x,int y)
{
    Unit** table;
    Unit* unit;
    Unit* best;
    int best_d;
    int d;

    //	FIXME:	this is not the best one
    //		We need the deposit with the shortest way!
    //		At least it must be reachable!
    //		Should use the same pathfinder flood fill, like the attacking
    //		code.

    best=NoUnitP;
    best_d=99999;
    for( table=Units; table<Units+NumUnits; table++ ) {
	unit=*table;
	// Want gold-mine and not dieing.
	if( !unit->Type->GoldMine || UnitUnusable(unit) ) {
	    continue;
	}
	d=MapDistanceToUnit(x,y,unit);
	// FIXME: UnitReachable(source,unit) didn't work unit still in building
	if( d<best_d /* && UnitReachable(source,unit) */ ) {
	    best_d=d;
	    best=unit;
	}
    }
    DebugLevel3Fn("%d %d,%d\n",UnitNumber(best),best->X,best->Y);

    if( LimitSearch && (best_d>TheMap.Width/5 || best_d>TheMap.Height/5) ) {
	return NoUnitP;
    }
    return best;
}

/**
**	Find gold deposit.
**
**	@param unit	Pointer for source unit.
**	@param player	A deposit owning this player
**	@param x	X tile position to start.
**	@param y	Y tile position to start.
**
**	@return		Pointer to the nearest gold mine.
*/
global Unit* FindGoldDeposit(const Unit* source,int x,int y)
{
    Unit** table;
    Unit* unit;
    Unit* best;
    int best_d;
    int d;
    const Player* player;

    //	FIXME:	this is not the best one
    //		We need the deposit with the shortest way!
    //		At least it must be reachable!
    //		Should use the same pathfinder flood fill, like the attacking
    //		code.
    player=source->Player;

    best=NoUnitP;
    best_d=99999;
    for( table=player->Units; table<player->Units+player->TotalNumUnits;
		table++ ) {
	unit=*table;
	// Want gold deposit and not dieing.
	if( !unit->Type->StoresGold || UnitUnusable(unit) ) {
	    continue;
	}
	d=MapDistanceToUnit(x,y,unit);
	// FIXME: UnitReachable(source,unit) didn't work unit still in building
	if( d<best_d /* && UnitReachable(source,unit) */ ) {
	    best_d=d;
	    best=unit;
	}
    }
    DebugLevel3Fn("%d %d,%d\n",UnitNumber(best),best->X,best->Y);
    return best;
}

/**
**	Find wood deposit.
**	@param player	A deposit owning this player
*/
global Unit* FindWoodDeposit(const Player* player,int x,int y)
{
    Unit* unit;
    Unit* best;
    Unit** units;
    int nunits;
    int best_d;
    int d,i;

    //	FIXME:	this is not the best one
    //		We need the deposit with the shortest way!
    //		At least it must be reachable!
    //	FIXME:	Could we use unit-cache to find it faster?
    //

    best=NoUnitP;
    best_d=99999;
    nunits=player->TotalNumUnits;
    units=player->Units;
    for( i=0; i<nunits; i++ ) {
	unit=units[i];
	if( UnitUnusable(unit) ) {
	    continue;
	}
	// Want wood-deposit
	if( unit->Type->StoresWood || unit->Type->StoresGold ) {
	    d=MapDistanceToUnit(x,y,unit);
	    if( d<best_d ) {
		best_d=d;
		best=unit;
	    }
	}
    }

    DebugLevel3Fn("%d %d,%d\n",UnitNumber(best),best->X,best->Y);
    return best;
}

#if 0
/**
**	Find wood in sight range.
**
**	@param unit	Unit that needs wood.
**	@param px	OUT: Map X position of wood.
**	@param py	OUT: Map Y position of wood.
**
**	@return		True if wood was found.
*/
global int FindWoodInSight(Unit* unit,int* px,int* py)
{
    int x;
    int y;
    int addx;
    int addy;
    int i;
    int n;
    int r;
    int wx;
    int wy;
    int bestx;
    int besty;
    int bestd;
    Unit* destu;

    DebugLevel3Fn("%d %d,%d\n",UnitNumber(unit),unit->X,unit->Y);

    x=unit->X;
    y=unit->Y;
    addx=unit->Type->TileWidth;
    addy=unit->Type->TileHeight;
    r=unit->Stats->SightRange*2;

    //
    //	This is correct, but can this be written faster???
    //
    if( (destu=FindWoodDeposit(unit->Player,x,y)) ) {
	NearestOfUnit(destu,x,y,&wx,&wy);
	DebugLevel3("To %d,%d\n",wx,wy);
    } else {
	wx=unit->X;
	wy=unit->Y;
    }
    bestd=99999;
    IfDebug( bestx=besty=0; );		// keep the compiler happy

    // FIXME: don't mark outside of the map
    --x;
    while( addx<=r && addy<=r ) {
	for( i=addy; i--; y++ ) {
	    if( CheckedForestOnMap(x,y) ) {
		n=max(abs(wx-x),abs(wy-y));
		DebugLevel3("Distance %d,%d %d\n",x,y,n);
		if( n<bestd ) {
		    bestd=n;
		    bestx=x;
		    besty=y;
		}
	    }
	}
	++addx;
	for( i=addx; i--; x++ ) {
	    if( CheckedForestOnMap(x,y) ) {
		n=max(abs(wx-x),abs(wy-y));
		DebugLevel3("Distance %d,%d %d\n",x,y,n);
		if( n<bestd ) {
		    bestd=n;
		    bestx=x;
		    besty=y;
		}
	    }
	}
	++addy;
	for( i=addy; i--; y-- ) {
	    if( CheckedForestOnMap(x,y) ) {
		n=max(abs(wx-x),abs(wy-y));
		DebugLevel3("Distance %d,%d %d\n",x,y,n);
		if( n<bestd ) {
		    bestd=n;
		    bestx=x;
		    besty=y;
		}
	    }
	}
	++addx;
	for( i=addx; i--; x-- ) {
	    if( CheckedForestOnMap(x,y) ) {
		n=max(abs(wx-x),abs(wy-y));
		DebugLevel3("Distance %d,%d %d\n",x,y,n);
		if( n<bestd ) {
		    bestd=n;
		    bestx=x;
		    besty=y;
		}
	    }
	}
	if( bestd!=99999 ) {
	    DebugLevel3Fn("wood on %d,%d\n",x,y);
	    *px=bestx;
	    *py=besty;
	    return 1;
	}
	++addy;
    }

    DebugLevel3Fn("no wood in sight-range\n");
    return 0;
}

#else

/**
**	Find wood in sight range.
**
**	@param unit	Unit that needs wood.
**	@param px	OUT: Map X position of wood.
**	@param py	OUT: Map Y position of wood.
**
**	@return		True if wood was found.
*/
global int FindWoodInSight(const Unit* unit,int* px,int* py)
{
    static const int xoffset[]={  0,-1,+1, 0, -1,+1,-1,+1 };
    static const int yoffset[]={ -1, 0, 0,+1, -1,-1,+1,+1 };
    struct {
	unsigned short X;
	unsigned short Y;
    } points[MaxMapWidth*MaxMapHeight/4];
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

    destx=x=unit->X;
    desty=y=unit->Y;

    //
    //	Find the nearest wood depot
    //
    if( (destu=FindWoodDeposit(unit->Player,x,y)) ) {
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
    matrix[x+n+(y+n)*w]=matrix[x-n+(y+n)*w]=
	matrix[x+n+(y-n)*w]=matrix[x-n+(y-n)*w]=66;
    for( i=n; i--; ) {
	matrix[x+n+(y+i)*w]=matrix[x-n+(y+i)*w]=
	    matrix[x+n+(y-i)*w]=matrix[x-n+(y-i)*w]=
	    matrix[x-i+(y+n)*w]=matrix[x+i+(y+n)*w]=
	    matrix[x-i+(y-n)*w]=matrix[x+i+(y-n)*w]=66;
    }

    mask=UnitMovementMask(unit);

    points[0].X=x;
    points[0].Y=y;
    rp=0;
    matrix[x+y*w]=1;			// mark start point
    ep=wp=1;				// start with one point

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
		if ( ForestOnMap(x,y) ) {
		    if( destu ) {
			n=max(abs(destx-x),abs(desty-y));
			if( n<bestd ) {
			    bestd=n;
			    bestx=x;
			    besty=y;
			}
			*m=22;
		    } else {			// no goal take the first
			*px=x;
			*py=y;
			return 1;
		    }
		}

		if( CanMoveToMask(x,y,mask) ) {	// reachable
		    *m=1;
		    points[wp].X=x;		// push the point
		    points[wp].Y=y;
		    if( ++wp>=sizeof(points)/sizeof(*points) ) {// round about
			wp=0;
		    }
		} else {			// unreachable
		    *m=99;
		}
	    }

	    if( ++rp>=sizeof(points)/sizeof(*points) ) {	// round about
		rp=0;
	    }
	}

	//
	//	Take best of this frame, if any.
	//
	if( bestd!=99999 ) {
	    *px=bestx;
	    *py=besty;
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

    DebugLevel3Fn("no wood in sight-range\n");

    return 0;
}

#endif

/**
**	Find oil platform.
**
**	@param player	A deposit owning this player
**	@param x	Nearest to X position.
**	@param y	Nearest to Y position.
**
**	@return		NoUnitP or oil platform unit
*/
global Unit* FindOilPlatform(const Player* player,int x,int y)
{
    Unit* unit;
    Unit* best;
    Unit** units;
    int nunits;
    int best_d;
    int d,i;

    //	FIXME:	this is not the best one
    //		We need the deposit with the shortest way!
    //		At least it must be reachable!
    //

    best=NoUnitP;
    best_d=99999;
    nunits=player->TotalNumUnits;
    units=player->Units;
    for( i=0; i<nunits; i++ ) {
	unit=units[i];
	if( UnitUnusable(unit) ) {
	    continue;
	}
	// Want platform
	if( unit->Type->GivesOil ) {
	    d=MapDistanceToUnit(x,y,unit);
	    if( d<best_d ) {
		best_d=d;
		best=unit;
	    }
	}
    }

    DebugLevel3Fn("%d %d,%d\n",UnitNumber(best),best->X,best->Y);
    if( LimitSearch && (best_d>TheMap.Width/5 || best_d>TheMap.Height/5) ) {
	return NoUnitP;
    }
    return best;
}

/**
**	Find oil deposit.
**
**	@param source	A deposit for this unit.
**	@param x	Nearest to X position.
**	@param y	Nearest to Y position.
**
**	@return		NoUnitP or oil deposit unit
*/
global Unit* FindOilDeposit(const Unit* source,int x,int y)
{
    Unit* unit;
    Unit* best;
    Unit** units;
    int nunits;
    int best_d;
    int d,i;
    const Player* player;

    //	FIXME:	this is not the best one
    //		We need the deposit with the shortest way!
    //		At least it must be reachable!
    //	FIXME:	Could we use unit-cache to find it faster?
    //
    player=source->Player;

    best=NoUnitP;
    best_d=INT_MAX;
    nunits=player->TotalNumUnits;
    units=player->Units;
    for( i=0; i<nunits; i++ ) {
	unit=units[i];
	if( UnitUnusable(unit) ) {
	    continue;
	}
	// Want oil-deposit
	if( unit->Type->StoresOil ) {
	    d=MapDistanceToUnit(x,y,unit);
	    if( d<best_d
		    /*&& (d=UnitReachable(source,unit,1)) && d<best_d*/ ) {
		best_d=d;
		best=unit;
	    }
	}
    }

    DebugLevel3Fn("%d %d,%d\n",UnitNumber(best),best->X,best->Y);
    return best;
}

/*----------------------------------------------------------------------------
--	Select units
----------------------------------------------------------------------------*/

/**
**	Unit on map screen.
**
**	Select units on screen. (x,y are in pixels relative to map 0,0).
**
**	More units on same position.
**		Cycle through units. ounit is the old one.
**		First take highest unit.
**
**	FIXME: If no unit, we could select near units?
**
**	@param ounit	Old selected unit.
**	@param x	X pixel position.
**	@param y	Y pixel position.
**
**	@return		An unit on X,Y position.
*/
global Unit* UnitOnScreen(Unit* ounit,unsigned x,unsigned y)
{
    Unit** table;
    Unit* unit;
    Unit* nunit;
    Unit* funit;			// first possible unit
    UnitType* type;
    int flag;				// flag take next unit
    unsigned gx;
    unsigned gy;

    funit=NULL;
    nunit=NULL;
    flag=0;
    if( !ounit ) {			// no old on this position
	flag=1;
    }
    for( table=Units; table<Units+NumUnits; table++ ) {
	unit=*table;
	// We don't use UnitUnusable() to be able to select
	// a building under construction.
	if( unit->Removed || unit->Orders[0].Action==UnitActionDie ) {
	    continue;
	}
	type=unit->Type;

	//
	//	Check if mouse is over the unit.
	//
	gx=unit->X*TileSizeX+unit->IX;
	if( x+(type->BoxWidth-type->TileWidth*TileSizeX)/2<gx ) {
	    continue;
	}
	if( x>gx+(type->TileWidth*TileSizeX+type->BoxWidth)/2 ) {
	    continue;
	}

	gy=unit->Y*TileSizeY+unit->IY;
	if( y+(type->BoxHeight-type->TileHeight*TileSizeY)/2<gy ) {
	    continue;
	}
	if( y>gy+(type->TileHeight*TileSizeY+type->BoxHeight)/2 ) {
	    continue;
	}

	//
	//	This could be taken.
	//
	if( flag ) {
	    return unit;
	}
	if( unit==ounit ) {
	    flag=1;
	} else if( !funit ) {
	    funit=unit;
	}
	nunit=unit;
    }

    if( flag && funit ) {
	return funit;
    }
    return nunit;
}

/**
**	Let an unit die.
**
**	@param unit	Unit to be destroyed.
*/
global void LetUnitDie(Unit* unit)
{
    const UnitType* type;

    unit->HP=0;
    unit->Moving=0;

    type=unit->Type;

    //
    //	Oil patch or removed units,  just remove.
    //
    if( type->OilPatch || unit->Removed ) {
	RemoveUnit(unit);
	UnitLost(unit);
	ReleaseUnit(unit);
	return;
    }

    PlayUnitSound(unit,VoiceDying);

    //
    //	Catapults,... explodes.
    //
    if( type->ExplodeWhenKilled ) {
	// FIXME: make it configurable?
	MakeMissile(MissileTypeExplosion
	    ,unit->X*TileSizeX+type->TileWidth*TileSizeX/2
	    ,unit->Y*TileSizeY+type->TileHeight*TileSizeY/2
	    ,0,0);
	RemoveUnit(unit);
	UnitLost(unit);
	ReleaseUnit(unit);
	return;
    }

    //
    //	Building,... explodes.
    //
    if( type->Building ) {
	MakeMissile(MissileTypeByIdent("missile-explosion")
	    ,unit->X*TileSizeX+type->TileWidth*TileSizeX/2
	    ,unit->Y*TileSizeY+type->TileHeight*TileSizeY/2
	    ,0,0);

	//
	//	Building with units inside?
	//
	if( type->GoldMine
		|| type->StoresGold || type->StoresWood
		|| type->GivesOil || type->StoresOil
		|| unit->Orders[0].Action==UnitActionBuilded ) {
	    DestroyAllInside(unit);
	}

	RemoveUnit(unit);
	UnitLost(unit);

	// FIXME: buildings should get a die sequence

	if( type->CorpseType ) {
	    unit->State=unit->Type->CorpseScript;
	    type=unit->Type=type->CorpseType;

	    unit->IX=(type->Width-VideoGraphicWidth(type->Sprite))/2;
	    unit->IY=(type->Height-VideoGraphicHeight(type->Sprite))/2;

	    unit->SubAction=0;
	    unit->Removed=0;
	    unit->Frame=0;
	    unit->Orders[0].Action=UnitActionDie;

	    DebugCheck( !unit->Type->Animations
		    || !unit->Type->Animations->Die );
	    UnitShowAnimation(unit,unit->Type->Animations->Die);
	    DebugLevel0Fn("Frame %d\n",unit->Frame);

	    return;
	}

	// no corpse available
	ReleaseUnit(unit);
	return;
    }

    // FIXME: units in transporters should die without corpes...
    if( unit->Type->Transporter ) { // Transporters loose their units
        //FIXME: vladi: it could be usefull if transport is near land
	//       to unload instead of destroying all units in it... ?
	DestroyAllInside(unit);
    }

    RemoveUnit(unit);
    UnitLost(unit);

    // FIXME: ugly trick unit-peon-with-gold ... has no die sequence.
    if( type==UnitTypeHumanWorkerWithGold
	    || type==UnitTypeHumanWorkerWithWood ) {
	unit->Type=UnitTypeHumanWorker;
    } else if( type==UnitTypeOrcWorkerWithGold
	    || type==UnitTypeOrcWorkerWithWood ) {
	unit->Type=UnitTypeOrcWorker;
    }

    //
    //	Unit has death animation.
    //

    // Not good: UnitUpdateHeading(unit);
    unit->SubAction=0;
    unit->Removed=0;
    unit->State=0;
    unit->Reset=0;
    unit->Wait=1;
    unit->Orders[0].Action=UnitActionDie;
}

/**
**	Destroy all units inside unit.
*/
global void DestroyAllInside(Unit* source)
{
    Unit* unit;
    int i;

    //
    // Destroy all units in Transporters
    //
    if( source->Type->Transporter ) {
        for( i=0; i<MAX_UNITS_ONBOARD; i++) {
	    // FIXME: check if valid pointer
	    if( (unit=source->OnBoard[i]) ) {
		// FIXME: no corpse!
	        // LetUnitDie(unit);
		RemoveUnit(unit);
		UnitLost(unit);
		ReleaseUnit(unit);
	    }
	}
	return;
    }

    //
    // Destroy the peon in building under construction...
    //
    if( source->Orders[0].Action==UnitActionBuilded
	    && source->Data.Builded.Worker ) {
	LetUnitDie(source->Data.Builded.Worker);
	return;
    }

    // FIXME: should use a better methode, linking all units in a building
    // FIXME: f.e. with the next pointer.
    //
    // Destroy all units in buildings or Resources (mines...)
    //
    for( i=0; i<NumUnits; i++ ) {
	unit=Units[i];
	if( !unit->Removed ) {		// not an unit inside
	    continue;
	}
	if( unit->X==source->X && unit->Y==source->Y ) {
	    LetUnitDie(unit);
	}
    }
}

/*----------------------------------------------------------------------------
--	Unit AI
----------------------------------------------------------------------------*/

/**
**	Unit is hit by missile or other damage.
**
**	@param attacker	Unit that attacks.
**	@param target	Unit that is hit.
**	@param damage	How many damage to take.
*/
global void HitUnit(Unit* attacker,Unit* target,int damage)
{
    UnitType* type;
    Unit* goal;

    if( damage==0 ) {			// Can now happen by splash damage
	DebugLevel0Fn("Warning no damage\n"); 
	return;
    }

    DebugCheck( damage==0 || target->HP==0 || target->Type->Vanishes );

    if ( target->UnholyArmor > 0 ) {
	// vladi: units with active UnholyArmour are invulnerable
	return;
    }

    type=target->Type;
    if( !target->Attacked ) {
	// NOTE: perhaps this should also be moved into the notify?
	if( target->Player==ThisPlayer ) {
	    static int LastFrame;
	    static int LastX;
	    static int LastY;

	    //
	    //	One help cry each 2 second is enough
	    //	If on same area ignore it for 2 minutes.
	    //
	    if( LastFrame<FrameCounter ) {
		if( LastFrame+FRAMES_PER_SECOND*120<FrameCounter ||
			target->X<LastX-14 || target->X>LastX+14 
			    || target->Y<LastY-14 || target->Y>LastY+14  ) {
		    LastFrame=FrameCounter+FRAMES_PER_SECOND*2;
		    LastX=target->X;
		    LastY=target->Y;
		    PlayUnitSound(target,VoiceHelpMe);
		}
	    }
	}
	NotifyPlayer(target->Player,NotifyRed,target->X,target->Y,
		"%s attacked",target->Type->Ident);
	if( target->Player->AiEnabled ) {
	    AiHelpMe(attacker,target);
	}
    }
    target->Attacked=7;

    if( target->HP<=damage ) {	// unit is killed or destroyed
	if( attacker ) {
	    attacker->Player->Score+=target->Type->Points;
#ifdef USE_HP_FOR_XP
	    attacker->XP+=target->HP;
#else
	    attacker->XP+=target->Type->Points;
#endif
	    ++attacker->Kills;
	}
	LetUnitDie(target);
	return;
    }
    target->HP-=damage;		// UNSIGNED!
#ifdef USE_HP_FOR_XP
    if( attacker ) {
	attacker->XP+=damage;
    }
#endif

#if 0
    // FIXME: want to show hits.
    if( type->Organic ) {
	MakeMissile(MissileBlood
		,target->X*TileSizeX+TileSizeX/2
		,target->Y*TileSizeY+TileSizeY/2,0,0);
    }
    if( type->Building ) {
	MakeMissile(MissileSmallFire
		,target->X*TileSizeX
			+(type->TileWidth*TileSizeX)/2
		,target->Y*TileSizeY
			+(type->TileHeight*TileSizeY)/2
		,0,0);
    }
#endif
    if( type->Building && !target->Burning ) {
	int f;
	Missile* missile;

	f=(100*target->HP)/target->Stats->HitPoints;
	if( f>75) {
	    ; // No fire for this
	} else if( f>50 ) {
	    missile=MakeMissile(MissileTypeSmallFire
		    ,target->X*TileSizeX
			    +(type->TileWidth*TileSizeX)/2
		    ,target->Y*TileSizeY
			    +(type->TileHeight*TileSizeY)/2
			    -TileSizeY
		    ,0,0);
	    missile->SourceUnit=target;
	    target->Burning=1;
	    ++target->Refs;
	} else {
	    missile=MakeMissile(MissileTypeBigFire
		    ,target->X*TileSizeX
			    +(type->TileWidth*TileSizeX)/2
		    ,target->Y*TileSizeY
			    +(type->TileHeight*TileSizeY)/2
			    -TileSizeY
		    ,0,0);
	    missile->SourceUnit=target;
	    target->Burning=1;
	    ++target->Refs;
	}
    }

    //
    //	FIXME: call others for help.
    //

    //
    //	Unit is working?
    //
    if( target->Orders[0].Action!=UnitActionStill ) {
	return;
    }

    //
    //	Attack units in range (which or the attacker?)
    //
    if( !type->CowerWorker && !type->CowerMage ) {
	if( type->CanAttack && !type->Tower ) {
	    goal=AttackUnitsInReactRange(target);
	    if( goal ) {
		if( target->SavedOrder.Action==UnitActionStill ) {
		    // FIXME: should rewrite command handling
		    CommandAttack(target,target->X,target->Y,NoUnitP,
			    FlushCommands);
		    target->SavedOrder=target->Orders[1];
		}
		CommandAttack(target,goal->X,goal->Y,NoUnitP,FlushCommands);
		return;
	    }
	}
    }

    //
    //	FIXME: Can't attack run away.
    //
    if( !type->Building ) {
	DebugLevel0Fn("FIXME: run away!\n");
    }
}

/*----------------------------------------------------------------------------
--	Conflicts
----------------------------------------------------------------------------*/

/**
**	Returns the map distance between two points.
**
**	@param x1	X map tile position.
**	@param y1	Y map tile position.
**	@param x2	X map tile position.
**	@param y2	Y map tile position.
**
**	@return		The distance between in tiles.
*/
global int MapDistance(int x1,int y1,int x2,int y2)
{
    return max(abs(x1-x2),abs(y1-y2));
}

/**
**	Returns the map distance between two points with unit type.
**
**	@param x1	X map tile position.
**	@param y1	Y map tile position.
**	@param type	Unit type to take into account.
**	@param x2	X map tile position.
**	@param y2	Y map tile position.
**
**	@return		The distance between in tiles.
*/
global int MapDistanceToType(int x1,int y1,const UnitType* type,int x2,int y2)
{
    int dx;
    int dy;

    if( x1<=x2 ) {
	dx=x2-x1;
    } else {
	dx=x1-x2-type->TileWidth+1;
	if( dx<0 ) {
	    dx=0;
	}
    }

    if( y1<=y2 ) {
	dy=y2-y1;
    } else {
	dy=y1-y2-type->TileHeight+1;
	if( dy<0 ) {
	    dy=0;
	}
    }

    DebugLevel3("\tDistance %d,%d -> %d,%d = %d\n"
	    ,x1,y1,x2,y2,(dy<dx) ? dx : dy);

    return (dy<dx) ? dx : dy;
}

/**
**	Returns the map distance to unit.
**
**	@param x	X map tile position.
**	@param y	Y map tile position.
**	@param dest	Distance to this unit.
**
**	@return		The distance between in tiles.
*/
global int MapDistanceToUnit(int x,int y,const Unit* dest)
{
    return MapDistanceToType(x,y,dest->Type,dest->X,dest->Y);
}

/**
**	Returns the map distance between two units.
**
**	@param src	Distance from this unit.
**	@param dst	Distance  to  this unit.
**
**	@return		The distance between in tiles.
*/
global int MapDistanceBetweenUnits(const Unit* src,const Unit* dst)
{
    int dx;
    int dy;
    int x1;
    int x2;
    int y1;
    int y2;

    x1=src->X;
    y1=src->Y;
    x2=dst->X;
    y2=dst->Y;

    if( x1+src->Type->TileWidth<=x2 ) {
	dx=x2-x1-src->Type->TileWidth;
	if( dx<0 ) {
	    dx=0;
	}
    } else {
	dx=x1-x2-dst->Type->TileWidth;
	if( dx<0 ) {
	    dx=0;
	}
    }

    if( y1+src->Type->TileHeight<=y2 ) {
	dy=y2-y1-src->Type->TileHeight;
    } else {
	dy=y1-y2-dst->Type->TileHeight;
	if( dy<0 ) {
	    dy=0;
	}
    }

    return (dy<dx) ? dx : dy;
}

/**
** compute the distance from the view point to a given point.
**	@param x	X map tile position.
**	@param y	Y map tile position.
*/
//FIXME: is it the correct place to put this?
global int ViewPointDistance(int x,int y)
{
    int x_v;
    int y_v;
    // first compute the view point coordinate
    x_v=MapX+MapWidth/2;
    y_v=MapY+MapHeight/2;
    // then use MapDistance
    return MapDistance(x_v,y_v,x,y);
}

/**
** compute the distance from the view point to a given unit.
**	@param x	X map tile position.
**	@param y	Y map tile position.
**	@param dest	Distance to this unit.
*/
//FIXME: is it the correct place to put this?
global int ViewPointDistanceToUnit(const Unit* dest)
{
    int x_v;
    int y_v;
    // first compute the view point coordinate
    x_v=MapX+MapWidth/2;
    y_v=MapY+MapHeight/2;
    // then use MapDistanceToUnit
    return MapDistanceToUnit(x_v,y_v,dest);
}

/**
**	Check if unit is an enemy.
**
**	@param player	The source player.
**	@param dest	The destination unit.
**
**	@return		Returns true, if the destination unit is an enemy.
*/
global int IsEnemy(const Player* player,const Unit* dest)
{
    return player->Enemy&(1<<dest->Player->Player);
}

/**
**	Check if unit is allied.
**
**	@param player	The source player.
**	@param dest	The destination unit.
**
**	@return		Returns true, if the destination unit is allied.
*/
global int IsAllied(const Player* player,const Unit* dest)
{
    return player->Allied&(1<<dest->Player->Player);
}

/**
**	Can the source unit attack the destination unit.
**
**	@param source	Unit type pointer of the attacker.
**	@param dest	Unit type pointer of the target.
*/
global int CanTarget(const UnitType* source,const UnitType* dest)
{
    if( dest->UnitType==UnitTypeLand ) {
	if( dest->ShoreBuilding ) {
	    return source->CanTarget&(CanTargetLand|CanTargetSea);
	}
	return source->CanTarget&CanTargetLand;
    }
    if( dest->UnitType==UnitTypeFly ) {
	return source->CanTarget&CanTargetAir;
    }
    if( dest->UnitType==UnitTypeNaval ) {
	return source->CanTarget&CanTargetSea;
    }
    return 0;
}

/*----------------------------------------------------------------------------
--	SAVE/LOAD
----------------------------------------------------------------------------*/

/**
**	Generate a unit reference, a printable unique string for unit.
*/
global char* UnitReference(const Unit* unit)
{
    char* ref;

    ref=malloc(10);
    sprintf(ref,"U%04X",UnitNumber(unit));
    return ref;
}

/**
**	Save an order.
**
**	@param order	Order who should be saved.
**	@param file	Output file.
*/
local void SaveOrder(const Order* order,FILE* file)
{
    char* ref;

    fprintf(file,"'(");
    switch( order->Action ) {
	case UnitActionNone:
	    fprintf(file,"action-none");
	    break;

	case UnitActionStill:
	    fprintf(file,"action-still");
	    break;
	case UnitActionStandGround:
	    fprintf(file,"action-stand-ground");
	    break;
	case UnitActionFollow:
	    fprintf(file,"action-follow");
	    break;
	case UnitActionMove:
	    fprintf(file,"action-move");
	    break;
	case UnitActionAttack:
	    fprintf(file,"action-attack");
	    break;
	case UnitActionAttackGround:
	    fprintf(file,"action-attack-ground");
	    break;
	case UnitActionDie:
	    fprintf(file,"action-die");
	    break;

	case UnitActionSpellCast:
	    fprintf(file,"action-spell-cast");
	    break;

	case UnitActionTrain:
	    fprintf(file,"action-train");
	    break;
	case UnitActionUpgradeTo:
	    fprintf(file,"action-upgrade-to");
	    break;
	case UnitActionResearch:
	    fprintf(file,"action-research");
	    break;
	case UnitActionBuilded:
	    fprintf(file,"action-builded");
	    break;

	case UnitActionBoard:
	    fprintf(file,"action-board");
	    break;
	case UnitActionUnload:
	    fprintf(file,"action-unload");
	    break;
	case UnitActionPatrol:
	    fprintf(file,"action-patrol");
	    break;
	case UnitActionBuild:
	    fprintf(file,"action-build");
	    break;

	case UnitActionRepair:
	    fprintf(file,"action-repair");
	    break;
	case UnitActionHarvest:
	    fprintf(file,"action-harvest");
	    break;
	case UnitActionMineGold:
	    fprintf(file,"action-mine-gold");
	    break;
	case UnitActionMineOre:
	    fprintf(file,"action-mine-ore");
	    break;
	case UnitActionMineCoal:
	    fprintf(file,"action-mine-coal");
	    break;
	case UnitActionQuarryStone:
	    fprintf(file,"action-quarry-stone");
	    break;
	case UnitActionHaulOil:
	    fprintf(file,"action-haul-oil");
	    break;
	case UnitActionReturnGoods:
	    fprintf(file,"action-return-goods");
	    break;

	case UnitActionDemolish:
	    fprintf(file,"action-demolish");
	    break;
    }
    fprintf(file," flags %d",order->Flags);
    fprintf(file," range (%d %d)",order->RangeX,order->RangeY);
    if( order->Goal ) {
	fprintf(file," goal %s",ref=UnitReference(order->Goal));
	free(ref);
    }
    fprintf(file," tile (%d %d)",order->X,order->Y);
    if( order->Type ) {
	fprintf(file," type %s",order->Type->Ident);
    }
    if( order->Arg1 ) {
	// patrol=pos, research=upgrade, spell=spell
	switch( order->Action ) {
	    case UnitActionPatrol:
		fprintf(file," patrol (%d %d)",
			(int)order->Arg1>>16,(int)order->Arg1&0xFFFF);
		break;
	    case UnitActionSpellCast:
		fprintf(file," spell %s",((SpellType*)order->Arg1)->Ident);
		break;
	    case UnitActionResearch:
		fprintf(file," upgrade %s",((Upgrade*)order->Arg1)->Ident);
		break;
	    default:
		fprintf(file," arg1 %08X",(int)order->Arg1);
		break;
	}
    }
    fprintf(file,")");
}

/**
**	Save the state of an unit to file.
**
**	@param unit	Unit pointer to be saved.
**	@param file	Output file.
*/
global void SaveUnit(const Unit* unit,FILE* file)
{
    char* ref;
    int i;

    fprintf(file,"\n(unit '%s ",ref=UnitReference(unit));
    free(ref);
    // Needed to create the unit slot
    fprintf(file,"'type '%s ",unit->Type->Ident);
    fprintf(file,"'player %d\n  ",unit->Player->Player);

    if( unit->Next ) {
	fprintf(file,"'next '%s ",ref=UnitReference(unit->Next));
	free(ref);
    }

    fprintf(file,"'tile '(%d %d) ",unit->X,unit->Y);
    for( i=0; i<PlayerMax; ++i ) {
	if( &unit->Type->Stats[i]==unit->Stats ) {
	    fprintf(file,"'stats %d\n  ",i);
	    break;
	}
    }
    if( i==PlayerMax ) {
	fprintf(file,"'stats 'S%08X\n  ",(int)unit->Stats);
    }
    fprintf(file,"'pixel '(%d %d) ",unit->IX,unit->IY);
    fprintf(file,"'%sframe %d ",
	    unit->Frame&128 ? "flipped-" : "" ,unit->Frame&127);
    if( unit->SeenFrame!=0xFF ) {
	fprintf(file,"'%sseen %d ",
		unit->SeenFrame&128 ? "flipped-" : "" ,unit->SeenFrame&127);
    } else {
	fprintf(file,"'not-seen ");
    }
    fprintf(file,"'direction %d\n  ",(unit->Direction*360)/256);
    fprintf(file,"'attacked %d\n ",unit->Attacked);
    if( unit->Burning ) {
	fprintf(file," 'burning");
    }
    if( unit->Destroyed ) {
	fprintf(file," 'destroyed");
    }
    if( unit->Removed ) {
	fprintf(file," 'removed");
    }
    if( unit->Selected ) {
	fprintf(file," 'selected");
    }
    fprintf(file," 'visible \"");
    for( i=0; i<PlayerMax; ++i ) {
	fputc((unit->Visible&(1<<i)) ? 'X' : '_',file);
    }
    fprintf(file,"\"\n ");
    if( unit->Constructed ) {
	fprintf(file," 'constructed");
    }
    if( unit->Active ) {
	fprintf(file," 'active");
    }
    fprintf(file," 'mana %d",unit->Mana);
    fprintf(file," 'hp %d",unit->HP);
    fprintf(file," 'xp %d",unit->XP);
    fprintf(file," 'kills %d\n  ",unit->Kills);

    fprintf(file,"'ttl %d ",unit->TTL);
    fprintf(file,"'bloodlust %d ",unit->Bloodlust);
    fprintf(file,"'haste %d ",unit->Haste);
    fprintf(file,"'slow %d\n  ",unit->Slow);
    fprintf(file,"'invisible %d ",unit->Invisible);
    fprintf(file,"'flame-shield %d ",unit->FlameShield);
    fprintf(file,"'unholy-armor %d\n  ",unit->UnholyArmor);

    fprintf(file,"'group-id %d\n  ",unit->GroupId);
    fprintf(file,"'last-group %d\n  ",unit->LastGroup);

    fprintf(file,"'sub-action %d ",unit->SubAction);
    fprintf(file,"'wait %d ",unit->Wait);
    fprintf(file,"'state %d",unit->State);
    if( unit->Reset ) {
	fprintf(file," 'reset");
    }
    fprintf(file,"\n  'blink %d",unit->Blink);
    if( unit->Moving ) {
	fprintf(file," 'moving");
    }
    fprintf(file," 'rs %d",unit->Rs);
    if( unit->Revealer ) {
	fprintf(file," 'revealer");
    }
    fprintf(file,"\n  'on-board #(");
    for( i=0; i<MAX_UNITS_ONBOARD; ++i ) {
	if( unit->OnBoard[i] ) {
	    fprintf(file,"'%s",ref=UnitReference(unit->OnBoard[i]));
	    free(ref);
	} else {
	    fprintf(file,"()");
	}
	if( i<MAX_UNITS_ONBOARD-1 ) {
	    fputc(' ',file);
	}
    }
    fprintf(file,")\n  ");
    fprintf(file,"'order-count %d\n  ",unit->OrderCount);
    fprintf(file,"'order-flush %d\n  ",unit->OrderFlush);
    fprintf(file,"'orders #(");
    for( i=0; i<MAX_ORDERS; ++i ) {
	fprintf(file,"\n    ");
	SaveOrder(&unit->Orders[i],file);
    }
    fprintf(file,")\n  'saved-order ");
    SaveOrder(&unit->SavedOrder,file);
    fprintf(file,"\n  'new-order ");
    SaveOrder(&unit->NewOrder,file);

    //
    //	Order data part
    //
    switch( unit->Orders[0].Action ) {
	case UnitActionStill:
	    break;
	case UnitActionBuilded:
	    fprintf(file,"\n  'data-builded '(worker %s",
		    ref=UnitReference(unit->Data.Builded.Worker));
	    free(ref);
	    fprintf(file," sum %d add %d val %d sub %d",
		    unit->Data.Builded.Sum,unit->Data.Builded.Add,
		    unit->Data.Builded.Val,unit->Data.Builded.Sub);
	    if( unit->Data.Builded.Cancel ) {
		fprintf(file,"cancel");
	    }
	    fprintf(file,")");
	    break;
	case UnitActionResearch:
	    DebugLevel0Fn("FIXME: not written\n");
	    fprintf(file,"\n  'data-reseach 'FIXME");
	    break;
	case UnitActionUpgradeTo:
	    DebugLevel0Fn("FIXME: not written\n");
	    fprintf(file,"\n  'data-upgrade-to 'FIXME");
	    break;
	case UnitActionTrain:
	    DebugLevel0Fn("FIXME: not written\n");
	    fprintf(file,"\n  'data-train 'FIXME");
	    break;
	default:
	    fprintf(file,"\n  'data-move '(");
	    if( unit->Data.Move.Fast ) {
		fprintf(file,"fast ");
	    }
	    if( unit->Data.Move.Length ) {
		fprintf(file,"with-path ");
	    }
	    fprintf(file,")");
	    break;
    }

    fprintf(file,")\n");
}

/**
**	Save state of units to file.
**
**	@param file	Output file.
*/
global void SaveUnits(FILE* file)
{
    Unit** table;

    fprintf(file,"\n;;; -----------------------------------------\n");
    fprintf(file,";;; MODULE: units $Id$\n\n");

    //
    //	Local variables
    //
    fprintf(file,"(set-hitpoint-regeneration! #%s)\n",
	    HitPointRegeneration ? "t" : "f");
    if( FancyBuildings ) {
	fprintf(file,"(fancy-buildings)");
    }
    fprintf(file,";;(set-fancy-buildings! #%s)\n",
	    FancyBuildings ? "t" : "f");

    for( table=Units; table<&Units[NumUnits]; ++table ) {
	SaveUnit(*table,file);
    }
}

/*----------------------------------------------------------------------------
--	Initialize/Cleanup
----------------------------------------------------------------------------*/

/**
**	Initialize unit module.
*/
global void InitUnits(void)
{
}

/**
**	Cleanup unit module.
*/
global void CleanUnits(void)
{
    Unit** table;
    Unit* unit;

    //
    //	Free memory for all units in unit table.
    //
    for( table=Units; table<&Units[NumUnits]; ++table ) {
	free(*table);
	*table=NULL;
    }

    //
    //	Release memory of units in release queue.
    //
    while( (unit=ReleasedHead) ) {
	ReleasedHead=unit->Next;
	free(unit);
    }

    InitUnitsMemory();

    HitPointRegeneration=0;
    FancyBuildings=0;
}

//@}
