//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __|
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name unit.c		-	The units. */
//
//	(c) Copyright 1998-2002 by Lutz Sammer
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
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
#include "ccl.h"
#include "editor.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

#ifndef LimitSearch
#define LimitSearch 1			/// Limit the search
#endif

global Unit* UnitSlots[MAX_UNIT_SLOTS];	/// All possible units
global Unit** UnitSlotFree;		/// First free unit slot
local Unit* ReleasedHead;		/// List of released units.
local Unit** ReleasedTail;		/// List tail of released units.

global Unit* Units[MAX_UNIT_SLOTS];	/// Array of used slots
global int NumUnits;			/// Number of slots used

global int HitPointRegeneration;	/// Hit point regeneration for all units
global int XpDamage;				/// Hit point regeneration for all units
global char EnableTrainingQueue;	/// Config: training queues enabled
global char EnableBuildingCapture;	/// Config: capture buildings enabled
global char RevealAttacker;		/// Config: reveal attacker enabled

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Initial memory allocation for units.
*/
global void InitUnitsMemory(void)
{
    Unit** slot;

    // Initialize the "list" of free unit slots

    slot=UnitSlots+MAX_UNIT_SLOTS;
    *--slot=NULL;			// leave last slot free as no marker
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
    DebugLevel2Fn("%lu:Unit %p %d `%s'\n" _C_ GameCycle _C_
	    unit _C_ UnitNumber(unit) _C_ unit->Type->Ident);

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
	temp->UnitSlot=unit->UnitSlot;
	*unit->UnitSlot=temp;
	Units[NumUnits]=NULL;
	//
	//	Are more references remaining?
	//
	unit->Destroyed=1;		// mark as destroyed
	RefsDebugCheck( !unit->Refs );
	if( --unit->Refs>0 ) {
	    DebugLevel2Fn("%lu:More references of %d #%d\n" _C_ GameCycle
		    _C_ UnitNumber(unit) _C_ unit->Refs);
	    return;
	}
#ifdef HIERARCHIC_PATHFINDER
	PfHierReleaseData(unit);
#endif
    }

    RefsDebugCheck( unit->Refs );

    //
    //	Free used memory
    //
    if( unit->Name ) {
	free(unit->Name);
    }

    //
    //	No more references remaining, but the network could have an order
    //	on the way. We must wait a little time before we could free the
    //	memory.
    //
    *ReleasedTail=unit;
    ReleasedTail=&unit->Next;
    unit->Refs=GameCycle+NetworkMaxLag;	// could be reuse after this time
    IfDebug(
	DebugLevel2Fn("%lu:No more references %d\n" _C_
		GameCycle _C_ UnitNumber(unit));
	unit->Type=NULL;			// for debugging.
    );
}

/**
**	FIXME: Docu
*/
local Unit *AllocUnit(void)
{
    Unit* unit;
    Unit** slot;
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
    if( ReleasedHead && (unsigned)ReleasedHead->Refs<GameCycle ) {
	unit=ReleasedHead;
	ReleasedHead=unit->Next;
	if( ReleasedTail==&unit->Next ) {	// last element
	    ReleasedTail=&ReleasedHead;
	}
	DebugLevel2Fn("%lu:Release %p %d\n" _C_ GameCycle _C_ unit _C_ UnitNumber(unit));
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
    return unit;
}

/**
**	Initialize the unit slot with default values.
**
**	@param unit	Unit pointer (allocated zero filled)
**	@param type	Unit-type
*/
global void InitUnit(Unit* unit, UnitType* type)
{
    // Refs need to be *increased* by 1, not *set* to 1, because if InitUnit()
    // is called from game loading code, Refs can already have a nonzero
    // value (thanks to forward references in the save file).  Incrementing
    // should not matter during in-game unit creation because in that case
    // Refs is 0 anyway.

    ++unit->Refs;

    //
    //  Build all unit table
    //
    unit->UnitSlot = &Units[NumUnits];	// back pointer
    Units[NumUnits++] = unit;

    DebugLevel3Fn("%p %d\n" _C_ unit _C_ UnitNumber(unit));

    //
    //  Initialise unit structure (must be zero filled!)
    //
    unit->Type = type;
    unit->SeenFrame = UnitNotSeen;		// Unit isn't yet seen

    // FIXME: this is not needed for load+save, must move to other place
    if (1) {				// Call CCL for name generation
	SCM fun;

	fun = gh_symbol2scm("gen-unit-name");
	if (!gh_null_p(symbol_boundp(fun, NIL))) {
	    SCM value;

	    value = symbol_value(fun, NIL);
	    if (!gh_null_p(value)) {
		value = gh_apply(value, cons(gh_symbol2scm(type->Ident), NIL));
		unit->Name = gh_scm2newstr(value, NULL);
	    }
	}
    }

    if (!type->Building && type->Sprite
	    && VideoGraphicFrames(type->Sprite) > 5) {
	unit->Direction = (MyRand() >> 8) & 0xFF;	// random heading
	UnitUpdateHeading(unit);
    }

    if (type->CanCastSpell) {
	unit->Mana = (type->_MaxMana * MAGIC_FOR_NEW_UNITS) / 100;
    }
    unit->Active = 1;

    unit->Wait = 1;
    unit->Reset = 1;
    unit->Removed = 1;

    // Invisible as default for submarines
    if (!type->Submarine) {
	unit->Visible = -1;		// Visible as default
    }

    unit->Rs = MyRand() % 100;		// used for fancy buildings and others

    unit->OrderCount = 1;		// No orders
    unit->Orders[0].Action = UnitActionStill;
    DebugCheck(unit->Orders[0].Goal);
    unit->NewOrder.Action = UnitActionStill;
    DebugCheck(unit->NewOrder.Goal);
    unit->SavedOrder.Action = UnitActionStill;
    DebugCheck(unit->SavedOrder.Goal);

    DebugCheck(NoUnitP);		// Init fails if NoUnitP!=0
}

/**
**	FIXME: Docu
*/
global void AssignUnitToPlayer(Unit *unit, Player *player)
{
    UnitType *type;
 
    type = unit->Type;

    //
    //	Build player unit table    
    //
    if (player && !type->Vanishes && unit->Orders[0].Action != UnitActionDie) {
	unit->PlayerSlot=player->Units+player->TotalNumUnits++;
	if( type->Building ) {
	    player->TotalBuildings++;
	}
	else {
	    player->TotalUnits++;
	}
	*unit->PlayerSlot=unit;

	player->UnitTypesCount[type->Type]++;
    }

    if( type->Demand ) {
        player->NumFoodUnits+=type->Demand;	// food needed
	if( player==ThisPlayer ) {
	    MustRedraw|=RedrawResources;	// update food
	}
    }
    if( type->Building ) {
	player->NumBuildings++;
    }

    unit->Player=player;
    unit->Stats=&type->Stats[unit->Player->Player];
    unit->Colors=player->UnitColors;
}

/**
**	Create a new unit.
**
**	@param type	Pointer to unit-type.
**	@param player	Pointer to owning player.
**
**	@return		Pointer to created unit.
*/
global Unit* MakeUnit(UnitType* type, Player* player)
{
    Unit* unit;

    DebugCheck(!player);		// Current code didn't support no player

    unit = AllocUnit();
    InitUnit(unit, type);
    AssignUnitToPlayer(unit, player);

    /* now we can finish the player-specific initialization of our unit */
    /* NOTE: this used to be a part of AssignUnitToPlayer() but had to be
     * moved out since it clobbered HP read from a saved file during game
     * load. */
    unit->HP = unit->Stats->HitPoints;

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
    unsigned flags;

    DebugCheck( !unit->Removed || unit->Destroyed );

    type=unit->Type;

#ifdef NEW_SHIPS
    //
    //	Sea and air units are 2 tiles aligned
    //
    if( type->UnitType==UnitTypeFly || type->UnitType==UnitTypeNaval ) {
	x&=~1;
	y&=~1;
    }
#endif

    unit->X=x;
    unit->Y=y;

#if 0
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

	flags=unit->Type->FieldFlags;
	for( h=type->TileHeight; h--; ) {
	    for( w=type->TileWidth; w--; ) {
		TheMap.Fields[x+w+(y+h)*TheMap.Width].Flags|=flags;
	    }
	}
    }
#else

    //
    //	Place unit on the map, mark the field with the FieldFlags.
    //
    flags=type->FieldFlags;
    for( h=type->TileHeight; h--; ) {
	for( w=type->TileWidth; w--; ) {
	    TheMap.Fields[x+w+(y+h)*TheMap.Width].Flags|=flags;
	}
    }

#ifdef not_needed_JOHNS_HIERARCHIC_PATHFINDER
    //
    //	Update hierarchic pathfinder structures.
    //
    if( type->Building ) {
	PfHierMapChangedCallback(x, y,
	    x + type->TileWidth - 1, y + type->TileHeight - 1);
    }
#endif

#endif

    x+=unit->Type->TileWidth/2;
    y+=unit->Type->TileHeight/2;

    //
    //	Units under construction have no sight range.
    //
    if( !unit->Constructed ) {
#ifdef NEW_FOW
	//
	//	Update fog of war, if unit belongs to player on this computer
	//
	if( unit->Next && !unit->Next->Data.Builded.Cancel && unit->Removed ) {
	    MapUnmarkSight(unit->Player,unit->Next->X+unit->Next->Type->TileWidth/2
				,unit->Next->Y+unit->Next->Type->TileHeight/2
				,unit->CurrentSightRange);
	}
	unit->Next = NULL;
	unit->CurrentSightRange=unit->Type->Stats->SightRange;
	MapMarkSight(unit->Player,x,y,unit->CurrentSightRange);
#else
	if( unit->Player==ThisPlayer || 
	    (ThisPlayer && IsSharedVision(ThisPlayer,unit)) ) {
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

    //
    //	Building oil-platform, must remove oil-patch.
    //
    if( type->GivesOil ) {
	Unit* temp;

	if( (temp=OilPatchOnMap(x,y)) ) {
	    DebugCheck( !temp );
	    unit->Value=temp->Value;
	    // oil patch should NOT make sound, handled by let unit die
	    LetUnitDie(temp);		// Destroy oil patch
	} else {
	    DebugLevel0Fn("No oil-patch to remove.\n");
	}
    }
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
	    unit->Frame = -unit->Frame;
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
**	@param host	Pointer to housing unit.
*/
global void RemoveUnit(Unit* unit, Unit* host)
{
    int h;
    int w;
    const UnitType* type;
    unsigned flags;

#ifdef NEW_FOW
    if ( !(host &&
	unit->X+unit->Type->TileWidth/2 == host->X+host->Type->TileWidth/2 &&
	unit->Y+unit->Type->TileHeight/2 == host->Y+host->Type->TileWidth/2 &&
	unit->CurrentSightRange == unit->CurrentSightRange) ) {
	    MapUnmarkSight(unit->Player,unit->X+unit->Type->TileWidth/2
				,unit->Y+unit->Type->TileHeight/2
				,unit->CurrentSightRange);
	    if( host ) {
		unit->CurrentSightRange=host->CurrentSightRange;
		MapMarkSight(unit->Player,host->X+host->Type->TileWidth/2,
				host->Y+host->Type->TileWidth/2,
				unit->CurrentSightRange);
	    }
    }
#endif

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

    // Unit is seen as under cursor
    if( unit==UnitUnderCursor ) {
	UnitUnderCursor=NULL;
    }

    // FIXME: unit is tracked?

    type=unit->Type;
#if 0
    //
    //	Update map
    //
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
#ifdef HIERARCHIC_PATHFINDER
	//
	//	Update hierarchic pathfinder structures.
	//
	PfHierMapChangedCallback (unit->X, unit->Y,
		    unit->X + unit->Type->TileWidth - 1,
		    unit->Y + unit->Type->TileHeight - 1);
#endif
    } else {
	unsigned flags;

	flags=~UnitFieldFlags(unit);
	for( h=type->TileHeight; h--; ) {
	    for( w=type->TileWidth; w--; ) {
		TheMap.Fields[unit->X+w+(unit->Y+h)*TheMap.Width].Flags&=flags;
	    }
	}
    }
#else

    //
    //	Update map
    //
    flags=~type->FieldFlags;
    for( h=type->TileHeight; h--; ) {
	for( w=type->TileWidth; w--; ) {
	    TheMap.Fields[unit->X+w+(unit->Y+h)*TheMap.Width].Flags&=flags;
	}
    }
#ifdef HIERARCHIC_PATHFINDER
    //
    //	Update hierarchic pathfinder structures.
    //
    if( type->Building ) {
	PfHierMapChangedCallback(unit->X, unit->Y,
	    unit->X + type->TileWidth - 1, unit->Y + type->TileHeight - 1);
    }
#endif

#endif

    DebugLevel3Fn("%d %p %p\n" _C_ UnitNumber(unit) _C_ unit _C_ unit->Next);
    UnitCacheRemove(unit);
    // UnitCache uses Next, need to set next again
    unit->Next=host;
     
    MustRedraw|=RedrawMinimap;
    CheckUnitToBeDrawn(unit);
}

/**
**	Update informations for lost units.
**
**	@param unit	Pointer to unit.
**
**	@note Also called by ChangeUnitOwner
*/
global void UnitLost(Unit* unit)
{
    Unit* temp;
    const UnitType* type;
    Player* player;
    int i;

    DebugCheck( !unit );

    player=unit->Player;
    DebugCheck( !player );		// Next code didn't support no player!

    //
    //	Call back to AI, for killed or lost units.
    //
    if( player && player->Ai ) {
	AiUnitKilled(unit);
    }

    //
    //  Remove unit from its groups
    //
    if( unit->GroupId ) {
        RemoveUnitFromGroups(unit);
    }

    //
    //	Remove the unit from the player's units table.
    //
    type=unit->Type;
    if( player && !type->Vanishes ) {
	DebugCheck( *unit->PlayerSlot!=unit );
	temp=player->Units[--player->TotalNumUnits];
	temp->PlayerSlot=unit->PlayerSlot;
	*unit->PlayerSlot=temp;
	player->Units[player->TotalNumUnits]=NULL;

	if( unit->Type->Building ) {
	    player->NumBuildings--;
	}

	if( unit->Orders[0].Action!=UnitActionBuilded ) {
	    player->UnitTypesCount[type->Type]--;
	}
    }


    //
    //	Handle unit demand. (Currently only food supported.)
    //
    if( type->Demand ) {
	player->NumFoodUnits-=type->Demand;
	if( player==ThisPlayer ) {
	    MustRedraw|=RedrawResources;	// update food
	    // FIXME: MustRedraw |= RedrawFood;
	}
    }

    //
    //	Update informations.
    //
    if( unit->Orders[0].Action!=UnitActionBuilded ) {
	if( type->Supply ) {			// supply
	    player->Food-=type->Supply;
	    if( player==ThisPlayer ) {
		MustRedraw |= RedrawResources;
		// FIXME: MustRedraw |= RedrawFood;
	    }
	}

	//
	//	Handle income improvements, look if a player looses a building
	//	which have given him a better income, find the next lesser
	//	income.
	//
	for( i=1; i<MaxCosts; ++i ) {
	    if( type->ImproveIncomes[i]==player->Incomes[i] ) {
		int m;
		int j;

		m=DefaultIncomes[i];
		for( j=0; j<player->TotalNumUnits; ++j ) {
		    if( m<player->Units[j]->Type->ImproveIncomes[i] ) {
			m=player->Units[j]->Type->ImproveIncomes[i];
		    }
		}
		player->Incomes[WoodCost]=m;
		if( player==ThisPlayer ) {
		    MustRedraw |= RedrawInfoPanel;
		}
	    }
	}
    }

    //
    //	Handle research cancels.
    //
    if( unit->Orders[0].Action == UnitActionResearch ) {
	unit->Player->UpgradeTimers.Upgrades[unit->Data.Research.Upgrade
		-Upgrades]=0;
    }

    DebugLevel3Fn("Lost %s(%d)\n" _C_ unit->Type->Ident _C_ UnitNumber(unit));

    //
    //	Destroy oil-platform, must re-make oil patch.
    //
    if( type->GivesOil && unit->Value>0 ) {
	// NOTE: I wasn't sure the best UnitType/Player
	// NOTE: This should really NOT be hardcoded?!
	temp=MakeUnitAndPlace(unit->X,unit->Y,UnitTypeOilPatch,&Players[15]);
	temp->Value=unit->Value;
    }
    DebugCheck( player->NumFoodUnits > UnitMax);
    DebugCheck( player->NumBuildings > UnitMax);
    DebugCheck( player->TotalNumUnits > UnitMax);
    DebugCheck( player->UnitTypesCount[type->Type] > UnitMax);
}

/**
**	FIXME: Docu
*/
global void UnitClearOrders(Unit *unit)
{
    int i;
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
    unit->Orders[0].Action=UnitActionStill;
    unit->SubAction=unit->State=0;
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
    int u;

    player=unit->Player;
    type=unit->Type;

    //
    //	Handle unit supply. (Currently only food supported.)
    //		Note an upgraded unit can't give more supply.
    //
    if( type->Supply && !upgrade ) {
	player->Food+=type->Supply;
	if( player==ThisPlayer ) {
	    MustRedraw|=RedrawResources;	// update food
	}
    }

    //
    //	Update resources
    //
    for( u=1; u<MaxCosts; ++u ) {
	if( player->Incomes[u]<unit->Type->ImproveIncomes[u] ) {
	    player->Incomes[u]=unit->Type->ImproveIncomes[u];
	    if( player==ThisPlayer ) {
		MustRedraw |= RedrawInfoPanel;
	    }
	}
    }
}

/**
**	Find nearest point of unit.
**
**	@param unit	Pointer to unit.
**	@param tx	X tile map postion.
**	@param ty	Y tile map postion.
**	@param dx	Out: nearest point X tile map postion to (tx,ty).
**	@param dy	Out: nearest point Y tile map postion to (tx,ty).
*/
global void NearestOfUnit(const Unit* unit,int tx,int ty,int *dx,int *dy)
{
    int x;
    int y;

    x=unit->X;
    y=unit->Y;

    DebugLevel3("Nearest of (%d,%d) - (%d,%d)\n" _C_ tx _C_ ty _C_ x _C_ y);
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

    DebugLevel3Fn("Goal is (%d,%d)\n" _C_ *dx _C_ *dy);
}

/**
**	Mark submarine seen by a submarine detector.
**
**	@param player	Player pointer that can see the submarine
**	@param x	X map tile center position
**	@param y	Y map tile center position
**	@param r	Range around center
**
**	@note
**		All units are marked as visible, not only submarines.
*/
global void MarkSubmarineSeen(const Player* player,int x,int y,int r)
{
    Unit* table[UnitMax];
    int n;
    int i;
    int pm;

    n=SelectUnits(x-r,y-r,x+r,y+r,table);
    pm=(1<<player->Player);
    for( i=0; i<n; ++i ) {
	table[i]->Visible|=pm;
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
    int x;
    int y;
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
	    if( IsMapFieldVisible(ThisPlayer,x+w,y+h) ) {
		return 1;
	    }
	}
    }

    return 0;
}

/**
**	Returns true, if unit is known on the map. Special case for buildings.
**
**	@param unit	Unit to be checked.
**	@return		True if known, false otherwise.
*/
global int UnitKnownOnMap(const Unit* unit)
{
    int x;
    int y;
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
	    if( IsMapFieldVisible(ThisPlayer,x+w,y+h)
		    || (unit->Type->Building && unit->SeenFrame!=UnitNotSeen
			&& IsMapFieldExplored(ThisPlayer,x+w,y+h)) ) {
		return 1;
	    }
	}
    }

    return 0;
}

/**
**	Returns true, if unit is visible in viewport.
**
**	@param vp	Viewport number.
**	@param unit	Unit to be checked.
**	@return		True if visible, false otherwise.
*/
global int UnitVisibleInViewport(const Viewport* vp, const Unit* unit)
{
    int x;
    int y;
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

    if( ThisPlayer != unit->Player &&
	!(unit->Player->SharedVision&(1<<ThisPlayer->Player) &&
	ThisPlayer->SharedVision&(1<<unit->Player->Player)) ) {
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
    if( (x+w) < vp->MapX || x > (vp->MapX+vp->MapWidth)
	    || (y+h) < vp->MapY || y > (vp->MapY+vp->MapHeight) ) {
	return 0;
    }

    //
    //	Check explored or if visible (building) under fog of war.
    //		FIXME: need only check the boundary, not the complete rectangle.
    //
    for( ; h-->0; ) {
	for( w=w0; w-->0; ) {
	    if( IsMapFieldVisible(ThisPlayer,x+w,y+h) || ReplayRevealMap
		    || (unit->Type->Building && unit->SeenFrame!=UnitNotSeen
			&& IsMapFieldExplored(ThisPlayer,x+w,y+h)) ) {
		return 1;
	    }
	}
    }

    return 0;
}

/**
**	Returns true, if unit is visible on current map view (any viewport).
**
**	@param unit	Unit to be checked.
**	@return		True if visible, false otherwise.
*/
global int UnitVisibleOnScreen(const Unit* unit)
{
    const Viewport* vp;

    for (vp = TheUI.Viewports; vp < TheUI.Viewports + TheUI.NumViewports;
	    vp++) {
	if (UnitVisibleInViewport(vp, unit)) {
	    return 1;
	}
    }
    return 0;
}

/**
**      Get area of map tiles covered by unit, including its displacement.
**
**      @param unit     Unit to be checked and set.
**	@param sx	Out: Top left X tile map postion.
**	@param sy	Out: Top left Y tile map postion.
**	@param ex	Out: Bottom right X tile map postion.
**	@param ey	Out: Bottom right Y tile map postion.
**
**      @return		sx,sy,ex,ey defining area in Map
*/
global void GetUnitMapArea(const Unit* unit, int *sx, int *sy, int *ex, int *ey)
{
    *sx = unit->X - (unit->IX < 0);
    *ex = *sx + unit->Type->TileWidth - !unit->IX;
    *sy = unit->Y - (unit->IY < 0);
    *ey = *sy + unit->Type->TileHeight - !unit->IY;
}

#ifdef NEW_DECODRAW
/**
**	Decoration redraw function that will redraw an unit (no building) for
**	set clip rectangle by decoration mechanism.
**
**	@param data	Unit pointer to be drawn
*/
local void DecoUnitDraw(void* data)
{
    Unit* unit;

    unit = (Unit*) data;
    DebugCheck(unit->Removed);
    DebugCheck(!UnitVisibleOnScreen(unit));

    DrawUnit(unit);
}

/**
**	Decoration redraw function that will redraw a building for
**	set clip rectangle by decoration mechanism.
**
**	@param data	Unit pointer to be drawn
*/
local void DecoBuildingDraw(void* data)
{
    Unit *unit;

    unit = (Unit*) data;
    DebugCheck(unit->Removed);
    DebugCheck(!UnitVisibleOnScreen(unit));

    DrawBuilding(unit);
}

/**
**	Create decoration for any unit-type
**
**	@param u	an unit which is visible on screen
**      @param x	x pixel position on screen of left-top
**      @param y	y pixel position on screen of left-top
**      @param w	width in pixels of area to be drawn from (x,y)
**      @param h	height in pixels of area to be drawn from (x,y)
*/
local void AddUnitDeco(Unit* u, int x, int y, int w, int h)
{
    if (u->Type->Building) {
	u->deco = DecorationAdd(u, DecoBuildingDraw, LevBuilding, x, y, w, h);
    } else if (u->Type->UnitType == UnitTypeFly) {
	u->deco = DecorationAdd(u, DecoUnitDraw, LevSkylow, x, y, w, h);
    } else {
	u->deco = DecorationAdd(u, DecoUnitDraw, LevCarLow, x, y, w, h);
    }
}
#endif

/**
**      Check and sets if unit must be drawn on screen-map
**
**      @param unit     Unit to be checked.
**      @return         True if map marked to be drawn, false otherwise.
*/
global int CheckUnitToBeDrawn(const Unit* unit)
{
#ifdef NEW_MAPDRAW
    int sx;
    int sy;
    int ex;
    int ey;

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
#ifdef NEW_DECODRAW
    if ( !unit->Removed && UnitVisibleOnScreen(unit) ) {
    // FIXME: Inaccurate dimension to take unit's extras into account..
    //        Should be solved by adding each unit extra as separate decoration
        int x = Map2ScreenX(unit->X)+unit->IX-10;
        int y = Map2ScreenY(unit->Y)+unit->IY-10;
        int w = unit->Type->Width+20;
        int h = unit->Type->Height+20;

        if ( unit->deco )
        {
        // FIXME: its very expensive to remove+add a decoration to satify a
        //        new location, a decoration update function should be added
          Deco *d = unit->deco;
          if ( d->x != x || d->y != y || d->w != w || d->h != h )
          {
            DecorationRemove( unit->deco );
            AddUnitDeco( (Unit *)unit, x, y, w, h );
          }
          else DecorationMark( unit->deco );
        }
        else AddUnitDeco( (Unit *)unit, x, y, w, h );

	return 1;
    }
    else if ( unit->deco )
    {
    // not longer visible: so remove from auto decoration redraw
      Unit *u = (Unit *)unit;
      DecorationRemove( unit->deco );
      u->deco = NULL;
    }
#else
    if (UnitVisibleOnScreen(unit)) {
	MustRedraw |= RedrawMap;
	return 1;
    }
#endif
#endif
    return 0;
}

#ifdef HIERARCHIC_PATHFINDER	// {

// hack
#include "../pathfinder/pf_lowlevel.h"

/**
**	FIXME: Docu
*/
global int UnitGetNextPathSegment(const Unit* unit, int *dx, int *dy)
{
    int segment;
    int shift;
    int neighbor;

    if (unit->Data.Move.Length <= 0) {
	return 0;
	// *dx and *dy returned to the caller are invalid
    }

    segment = unit->Data.Move.Length - 1;
    shift = segment % 2 ? 4 : 0;
    neighbor = (unit->Data.Move.Path[segment / 2] >> shift) & 0xf;
    *dx = Neighbor[neighbor].dx;
    *dy = Neighbor[neighbor].dy;
    return 1;
}

#endif // } HIERARCHIC_PATHFINDER

// FIXME: perhaps I should write a function UnitSelectable?

/**
**	Increment mana of all magic units. Called each second.
**	Also clears the blink flag and handles submarines.
**
**	@note	we could build a table of all magic units reducing cpu use.
**
**	@todo FIXME: Split this into more functions, to make the use clearer
**		or rename the function.
*/
//FIXME: vladi: the doc says incrementing mana is done by 1 per second
//       the spells effect can be decremented at the same time and this
//       will reduse calls to this function to one time per second only!
//	johns: We must also walk through all units = also overhead.
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

	if( unit->Type->CanCastSpell && unit->Mana!=unit->Type->_MaxMana ) {
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
	if( unit->TTL && unit->TTL<GameCycle ) {
	    DebugLevel0Fn("Unit must die %lu %lu!\n" _C_ unit->TTL
		    _C_ GameCycle);
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
	DebugLevel3Fn("%d:%d,%d,%d,%d,%d\n" _C_ UnitNumber(unit) _C_
		unit->Bloodlust _C_ unit->Haste _C_ unit->Slow _C_
		unit->Invisible _C_ unit->UnholyArmor);

	if (  unit->Type->Submarine ) {
	    if( !flag && (unit->Visible&(1<<ThisPlayer->Player)) ) {
                flag=CheckUnitToBeDrawn(unit);
	    }
	    unit->Visible=0;
	}
    }

    //
    //	Step 2) Mark all submarines that could be seen.
    //		Take units that can see sub marines, aren't under construction
    //		and are on the map.
    //
    for( table=Units; table<Units+NumUnits; table++ ) {
	unit=*table;
	if( unit->Type->CanSeeSubmarine && !unit->Removed &&
		unit->Orders[0].Action!=UnitActionBuilded ) {
	    MarkSubmarineSeen(unit->Player,unit->X+unit->Type->TileWidth/2,
		unit->Y+unit->Type->TileHeight/2,unit->Stats->SightRange);
	}
    }
}

/**
**	Increment health of all regenerating units. Called each second.
**
**	@note:	We could build a table of all regenerating units reducing cpu
**		use.
**		Any idea how to handle this more general? It whould be nice
**		to have more units that could regenerate.
*/
global void UnitIncrementHealth(void)
{
    Unit** table;
    Unit* unit;
    int regeneration;

    // FIXME: move to init code! (Can't be done here, load/save!)
    regeneration=UpgradeIdByIdent("upgrade-berserker-regeneration");

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
	if( unit->Type==UnitTypeBerserker
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
**	@param unit		Unit which should be consigned.
**	@param oldplayer	Old owning player.
**	@param newplayer	New owning player.
**
**	@todo	FIXME: I think here are some failures, if building is build
**		what is with the unit inside? or a main hall with workers
**		inside?
**		Parameter old player is redunant?
*/
global void ChangeUnitOwner(Unit* unit,Player* oldplayer,Player* newplayer)
{
    int i;

    DebugCheck( unit->Player!=oldplayer );

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

    //Adjust Orders to remove Attack Order
    //Mainly to protect peasants who are building.
    for( i=0; i < MAX_ORDERS; i++) {
        if (unit->Orders[i].Action==UnitActionAttack ||
            unit->Orders[i].Action==UnitActionAttackGround) {
            //Now see if it's an enemy..
            //FIXME:Just Stops attacking at the moment
               unit->Orders[i].Action=UnitActionStill;
               unit->SubAction=unit->State=0;
               break;
        }
    }
    //
    //	Now the new side!
    //

    //	Insert into new player table.

    unit->PlayerSlot=newplayer->Units+newplayer->TotalNumUnits++;
    if( unit->Type->Building ) {
	newplayer->TotalBuildings++;
    }
    else {
	newplayer->TotalUnits++;
    }
    *unit->PlayerSlot=unit;

    unit->Player=newplayer;
#ifdef NEW_FOW
    MapUnmarkSight(oldplayer,unit->X+unit->Type->TileWidth/2
	,unit->Y+unit->Type->TileHeight/2
	,unit->Stats->SightRange);
    MapMarkSight(unit->Player,unit->X+unit->Type->TileWidth/2
	,unit->Y+unit->Type->TileHeight/2
	,unit->Stats->SightRange);
#endif

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
	unit->Blink=5;
	unit->Rescued=1;
    }
}

/**
**	Rescue units.
**
**	Look through all rescueable players, if they could be rescued.
*/
global void RescueUnits(void)
{
    Player* p;
    Unit* unit;
    Unit* table[UnitMax];
    Unit* around[UnitMax];
    int n;
    int i;
    int j;
    int l;

    if( NoRescueCheck ) {		// all possible units are rescued
	return;
    }
    NoRescueCheck=1;

    //
    //	Look if player could be rescued.
    //
    for( p=Players; p<Players+NumPlayers; ++p ) {
	if( p->Type!=PlayerRescuePassive && p->Type!=PlayerRescueActive ) {
	    continue;
	}
	if( p->TotalNumUnits ) {
	    NoRescueCheck=0;
	    // NOTE: table is changed.
	    l=p->TotalNumUnits;
	    memcpy(table,p->Units,l*sizeof(Unit*));
	    for( j=0; j<l; j++ ) {
		unit=table[j];
		DebugLevel3("Checking %d(%s)" _C_ UnitNumber(unit) _C_
			unit->Type->Ident);
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
			unit->Y+unit->Type->TileHeight+1,around);
		} else {
		    n=SelectUnits(
			unit->X-2,unit->Y-2,
			unit->X+unit->Type->TileWidth+2,
			unit->Y+unit->Type->TileHeight+2,around);
		}
		DebugLevel3(" = %d\n" _C_ n);
		//
		//	Look if ally near the unit.
		//
		for( i=0; i<n; ++i ) {
#if 0
		    if( around[i]->Type->CanAttack &&
			    around[i]->Player->Type==PlayerPerson ) {
#endif
		    if( around[i]->Type->CanAttack &&
			    IsAllied(unit->Player,around[i]) ) {
			ChangeUnitOwner(unit,unit->Player,around[i]->Player);
			unit->Blink=5;
			unit->Rescued=1;
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
	    atan_table[init]=
		(unsigned char)(atan((double)init/64)*(64*4/6.2831853));
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
    int nextdir;

    if( unit->Frame<0 ) {
	unit->Frame=-unit->Frame;
    }
    unit->Frame/=unit->Type->NumDirections/2+1;
    unit->Frame*=unit->Type->NumDirections/2+1;
    // Remove heading, keep animation frame

    nextdir=256/unit->Type->NumDirections;
    dir=((unit->Direction+nextdir/2)&0xFF)/nextdir;
    if( dir<=LookingS/nextdir ) {	// north->east->south
	unit->Frame+=dir;
    } else {
	unit->Frame+=256/nextdir-dir;
	unit->Frame=-unit->Frame;
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
#ifndef NEW_FOW
    int n;
    int nb;
    Unit* table[UnitMax];
#endif

    //FIXME: vladi: this debug check fails when used for teleporting...
    //DebugCheck( !unit->Removed );

    // FIXME: better and quicker solution, to find the building.
    x=y=-1;
#ifdef NEW_FOW
    if( unit->Next ) {
	x=unit->Next->X;
	y=unit->Next->Y;
    } else {
	x=unit->X;
	y=unit->Y;
	DebugLevel0Fn("No building?\n");
    }
#else
    n=SelectUnitsOnTile(unit->X,unit->Y,table);
    for( nb=i=0; i<n; ++i ) {
	if( UnitUnusable(table[i]) ) {
	    continue;
	}
	if( table[i]->Type->Building ) {
	    nb++;
	    x=table[i]->X;
	    y=table[i]->Y;
	}
    }
    if (!nb) {	//Check if there actually is a building.
	DebugLevel0Fn("No building?\n");
	x=unit->X;
	y=unit->Y;
    }
#endif
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
    unit->Wait=1;		// should be correct unit has still action

    PlaceUnit(unit, x, y);
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
    int bestx;
    int besty;
    int bestd;
    int mask;
    int n;
#ifndef NEW_FOW
    Unit* table[UnitMax];
#endif

    DebugLevel3Fn("%d\n" _C_ UnitNumber(unit));
    DebugCheck( !unit->Removed );

    // FIXME: better and quicker solution, to find the building.
    x=y=-1;
#ifdef NEW_FOW
    if( unit->Next ) {
	x=unit->Next->X;
	y=unit->Next->Y;
    } else {
	DebugLevel0Fn("No building?\n");
	x=unit->X;
	y=unit->Y;
    }
#else
    n=SelectUnitsOnTile(unit->X,unit->Y,table);
    for( i=0; i<n; ++i ) {
	if( UnitUnusable(table[i]) ) {
	    continue;
	}
	if( table[i]->Type->Building ) {
	    x=table[i]->X;
	    y=table[i]->Y;
	}
    }
#endif
    DebugCheck( x==-1 || y==-1 );
    mask=UnitMovementMask(unit);

    bestd=99999;
    IfDebug( bestx=besty=0; );		// keep the compiler happy

    // FIXME: if we reach the map borders we can go fast up, left, ...
    --x;
    for( ;; ) {
	for( i=addy; i--; y++ ) {	// go down
	    if( CheckedCanMoveToMask(x,y,mask) ) {
		n=MapDistance(gx,gy,x,y);
		DebugLevel3("Distance %d,%d %d\n" _C_ x _C_ y _C_ n);
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
		DebugLevel3("Distance %d,%d %d\n" _C_ x _C_ y _C_ n);
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
		DebugLevel3("Distance %d,%d %d\n" _C_ x _C_ y _C_ n);
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
		DebugLevel3("Distance %d,%d %d\n" _C_ x _C_ y _C_ n);
		if( n<bestd ) {
		    bestd=n;
		    bestx=x;
		    besty=y;
		}
	    }
	}
	if( bestd!=99999 ) {
	    unit->Wait=1;		// unit should have action still
	    PlaceUnit(unit,bestx,besty);
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
    // FIXME: above is wrong, NEW_FOW use Next in another way.
    Unit** table;
    Unit* unit;
    int i;

    i=0;
    for( table=Units; table<Units+NumUnits; table++ ) {
	unit=*table;
	if( unit->Removed && unit->X==source->X && unit->Y==source->Y ) {
	    ++i;
	    DropOutOnSide(unit,LookingW
		,source->Type->TileWidth,source->Type->TileHeight);
	    DebugCheck( unit->Orders[0].Goal );
	    unit->Orders[0].Action=UnitActionStill;
	    unit->Wait=unit->Reset=1;
	    unit->SubAction=0;
	}
    }
    DebugLevel0Fn("Drop out %d of %d\n" _C_ i _C_ source->Data.Resource.Active);
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
global int CanBuildHere(const UnitType* type,int x,int y)
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

    if( EditorRunning ) {
	if( type->OilPatch || type->GivesOil ) {
	    // Oil patches and platforms can only be placed on even tiles
	    if( !(x&1 && y&1) ) {
		return 0;
	    }
	    // Don't allow oil patches on oil patches
	    if( type->OilPatch ) {
		n=UnitCacheSelect(x,y,x+type->TileWidth,y+type->TileHeight,table);
		for( i=0; i<n; ++i ) {
		    if( table[i]->Type->OilPatch ) {
			return 0;
		    }
		}
	    }
	} else if( type->UnitType==UnitTypeFly || type->UnitType==UnitTypeNaval ) {
	    // Flyers and naval units can only be placed on odd tiles
	    if( x&1 || y&1 ) {
		return 0;
	    }
	}
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
		DebugLevel3("Check goldmine %d,%d\n" _C_ unit->X _C_ unit->Y);
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
		DebugLevel3("Distance %d,%d\n" _C_ dx _C_ dy);
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
	//	Oil deposit can't be build too near to oil-patch or platform.
	//
	// FIXME: use unit-cache here.
	int i;

	for( i=0; i<NumUnits; i++ ) {
	    unit=Units[i];
	    if( unit->Type->OilPatch || unit->Type->GivesOil ) {
	      DebugLevel3("Check oilpatch %d,%d\n"
			  _C_ unit->X _C_ unit->Y);
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
	      DebugLevel3("Distance %d,%d\n" _C_ dx _C_ dy);
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
**	@param unit	Worker that want to build the building or NULL.
**	@param type	Building unit-type.
**	@param x	X tile map position.
**	@param y	Y tile map position.
**	@return		True if the building could be build..
**
**	@todo can't handle building units !1x1, needs a rewrite.
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
    IfDebug( j=0; );
    if( unit ) {
	// FIXME: This only works with 1x1 big units
	DebugCheck( unit->Type->TileWidth!=1 || unit->Type->TileHeight!=1 );
	j=unit->Type->FieldFlags;
	TheMap.Fields[unit->X+unit->Y*TheMap.Width].Flags&=~j;
    }

#if 0
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
    } else if( type->Building ) {
	switch( type->UnitType ) {
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
	    mask=MapFieldAirUnit;	// already occuppied
	    break;
	default:
	    DebugLevel1Fn("Were moves this unit?\n");
	    if( unit ) {
		TheMap.Fields[unit->X+unit->Y*TheMap.Width].Flags|=j;
	    }
	    return 0;
	}
    } else switch( type->UnitType ) {
	case UnitTypeLand:
	    mask=MapFieldLandUnit
		| MapFieldBuilding	// already occuppied
		| MapFieldWall
		| MapFieldRocks
		| MapFieldForest	// wall,rock,forest not 100% clear?
		| MapFieldCoastAllowed
		| MapFieldWaterAllowed	// can't build on this
		| MapFieldUnpassable;	// FIXME: I think shouldn't be used
	    break;
	case UnitTypeNaval:
	    mask=MapFieldSeaUnit
		| MapFieldBuilding	// already occuppied
		| MapFieldCoastAllowed
		| MapFieldLandAllowed	// can't build on this
		| MapFieldUnpassable;	// FIXME: I think shouldn't be used
	    break;
	case UnitTypeFly:
	    mask=MapFieldAirUnit;	// already occuppied
	    break;
	default:
	    DebugLevel1Fn("Were moves this unit?\n");
	    if( unit ) {
		TheMap.Fields[unit->X+unit->Y*TheMap.Width].Flags|=j;
	    }
	    return 0;
    }
#else

    mask = type->MovementMask;

#endif

    for( h=type->TileHeight; h--; ) {
	for( w=type->TileWidth; w--; ) {
	    if( !CanBuildOn(x+w,y+h,mask) ) {
		if( unit ) {
		    TheMap.Fields[unit->X+unit->Y*TheMap.Width].Flags|=j;
		}
		return 0;
	    }
	}
    }
    if( unit ) {
	TheMap.Fields[unit->X+unit->Y*TheMap.Width].Flags|=j;
    }

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
**	@param source	Pointer for source unit.
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
	// FIXME: UnitReachable didn't work with unit inside
	if( d<best_d /* && (d=UnitReachable(source,unit,1)) && d<best_d */ ) {
	    best_d=d;
	    best=unit;
	}
    }
    DebugLevel3Fn("%d %d,%d\n" _C_ UnitNumber(best) _C_ best->X _C_ best->Y);

    if( LimitSearch && (best_d>TheMap.Width/5 || best_d>TheMap.Height/5) ) {
	return NoUnitP;
    }
    return best;
}

/**
**	Find gold deposit (where we can deliver gold).
**
**	@param source	Pointer for source unit.
**	@param x	X tile position to start.
**	@param y	Y tile position to start.
**
**	@return		Pointer to the nearest gold depot.
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
	// FIXME: UnitReachable didn't work with unit inside
	if( d<best_d /* && (d=UnitReachable(source,unit,1)) && d<best_d */ ) {
	    best_d=d;
	    best=unit;
	}
    }
    DebugLevel3Fn("%d %d,%d\n" _C_ best?UnitNumber(best):-1 _C_
                               best?best->X:-1 _C_ best?best->Y:-1);
    return best;
}

/**
**	Find wood deposit.
**
**	@param player	A deposit owning this player
**	@param x	X tile position to start.
**	@param y	Y tile position to start.
**
**	@return		Pointer to the nearest wood depot.
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

    DebugLevel3Fn("%d %d,%d\n" _C_ best?UnitNumber(best):-1 _C_
                               best?best->X:-1 _C_ best?best->Y:-1);
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

    DebugLevel3Fn("%d %d,%d\n" _C_ UnitNumber(unit) _C_ unit->X _C_ unit->Y);

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
	DebugLevel3("To %d,%d\n" _C_ wx _C_ wy);
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
		DebugLevel3("Distance %d,%d %d\n" _C_ x _C_ y _C_ n);
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
		DebugLevel3("Distance %d,%d %d\n" _C_ x _C_ y _C_ n);
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
		DebugLevel3("Distance %d,%d %d\n" _C_ x _C_ y _C_ n);
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
		DebugLevel3("Distance %d,%d %d\n" _C_ x _C_ y _C_ n);
		if( n<bestd ) {
		    bestd=n;
		    bestx=x;
		    besty=y;
		}
	    }
	}
	if( bestd!=99999 ) {
	    DebugLevel3Fn("wood on %d,%d\n" _C_ x _C_ y);
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

    destx=x=unit->X;
    desty=y=unit->Y;
    size=TheMap.Width*TheMap.Height/4;
    points=alloca(size*sizeof(*points));
	

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
#ifdef NEW_FOW
		if ( ForestOnMap(x,y) && IsMapFieldExplored(unit->Player,x,y) ) {
#else
		if ( ForestOnMap(x,y) ) {
#endif
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
			if( ++wp>=size ) {			// round about
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
    int d;
    int i;

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

    DebugLevel3Fn("%d %d,%d\n" _C_ best?UnitNumber(best):-1 _C_
                               best?best->X:-1 _C_ best?best->Y:-1);
    /*	Oil platforms are our own, they should be known
    if( LimitSearch && (best_d>TheMap.Width/5 || best_d>TheMap.Height/5) ) {
	return NoUnitP;
    }
    */
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
		    // FIXME: UnitReachable didn't work with unit inside
		    /*&& (d=UnitReachable(source,unit,1)) && d<best_d*/ ) {
		best_d=d;
		best=unit;
	    }
	}
    }

    DebugLevel3Fn("%d %d,%d\n" _C_ best?UnitNumber(best):-1 _C_
                               best?best->X:-1 _C_ best?best->Y:-1);
    return best;
}

/**
**	Find the next idle worker
**
**	@param player	Player's units to search through
**	@param last	Previous idle worker selected
**
**	@return		NoUnitP or next idle worker
*/
global Unit* FindIdleWorker(const Player* player,const Unit* last)
{
    Unit* unit;
    Unit** units;
    Unit* FirstUnitFound;
    int nunits;
    int i;
    int SelectNextUnit;

    FirstUnitFound=NoUnitP;
    if( last==NoUnitP ) SelectNextUnit=1;
    else SelectNextUnit=0;

    nunits=player->TotalNumUnits;
    units=player->Units;

    for( i=0; i<nunits; i++ ) {
	unit=units[i];
	if( unit->Type->CowerWorker && !unit->Removed ) {
	    if( unit->Orders[0].Action==UnitActionStill ) {
		if( SelectNextUnit && !IsOnlySelected(unit) ) {
		    return unit;
		}
		if( FirstUnitFound==NULL ) {
		    FirstUnitFound=unit;
		}
	    }
	}
	if( unit==last ) {
	    SelectNextUnit=1;
	}
    }

    if( FirstUnitFound!=NoUnitP && !IsOnlySelected(FirstUnitFound) ) {
	return FirstUnitFound;
    }

    return NoUnitP;
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
global Unit* UnitOnScreen(Unit* ounit,int x,int y)
{
    Unit** table;
    Unit* unit;
    Unit* nunit;
    Unit* funit;			// first possible unit
    UnitType* type;
    int flag;				// flag take next unit
    int gx;
    int gy;

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
	RemoveUnit(unit,NULL);
	UnitLost(unit);
	UnitClearOrders(unit);
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
	RemoveUnit(unit,NULL);
	UnitLost(unit);
	UnitClearOrders(unit);
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
	    //
	    //	During oil platform build, the worker holds the oil value,
	    //	but if canceling building the platform, the worker is already
	    //	outside.
	    if( type->GivesOil
			&& unit->Orders[0].Action==UnitActionBuilded
			&& unit->Data.Builded.Worker ) {
		// Restore value for oil-patch
		unit->Value=unit->Data.Builded.Worker->Value;
	    }
	    DestroyAllInside(unit);
	}

	RemoveUnit(unit,NULL);
	UnitLost(unit);
	UnitClearOrders(unit);

	// FIXME: buildings should get a die sequence

	if( type->CorpseType ) {
	    unit->State=unit->Type->CorpseScript;
	    type=unit->Type=type->CorpseType;

	    unit->IX=(type->Width-VideoGraphicWidth(type->Sprite))/2;
	    unit->IY=(type->Height-VideoGraphicHeight(type->Sprite))/2;

	    unit->SubAction=0;
	    //unit->Removed=0;
	    unit->Frame=0;
	    unit->Orders[0].Action=UnitActionDie;

	    DebugCheck( !unit->Type->Animations
		    || !unit->Type->Animations->Die );
	    UnitShowAnimation(unit,unit->Type->Animations->Die);
	    DebugLevel0Fn("Frame %d\n" _C_ unit->Frame);
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

    RemoveUnit(unit,NULL);
    UnitLost(unit);
    UnitClearOrders(unit);

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
    //unit->Removed=0;
    unit->State=0;
    unit->Reset=0;
    unit->Wait=1;
    unit->Orders[0].Action=UnitActionDie;
#ifdef NEW_FOW
    if( unit->Type->CorpseType ) {
	unit->CurrentSightRange=unit->Type->CorpseType->Stats->SightRange;
    } else {
	unit->CurrentSightRange=1;
    }
    MapMarkSight(unit->Player,unit->X,unit->Y,unit->CurrentSightRange);
#endif
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
		RemoveUnit(unit,NULL);
		UnitLost(unit);
    		UnitClearOrders(unit);
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

    if( !damage ) {			// Can now happen by splash damage
	DebugLevel0Fn("Warning no damage, try to fix by caller?\n");
	return;
    }

    DebugCheck( damage==0 || target->HP==0 || target->Type->Vanishes );

    if ( target->UnholyArmor > 0 ) {
	// vladi: units with active UnholyArmour are invulnerable
	return;
    }

    if ( target->Removed ) {
	DebugLevel0Fn("Removed target hit\n");
	return;
    }

    if( GodMode ) {
	if( attacker && attacker->Player==ThisPlayer ) {
	    damage=255;
	}
	if( target->Player==ThisPlayer ) {
	    damage=0;
	}
    }

    type=target->Type;
    if( !target->Attacked ) {
	// NOTE: perhaps this should also be moved into the notify?
	if( target->Player==ThisPlayer ) {
	    // FIXME: Problem with load+save and restart.
	    static unsigned long LastCycle;
	    static int LastX;
	    static int LastY;

	    //
	    //	One help cry each 2 second is enough
	    //	If on same area ignore it for 2 minutes.
	    //
	    if( LastCycle<GameCycle ) {
		if( LastCycle+CYCLES_PER_SECOND*120<GameCycle ||
			target->X<LastX-14 || target->X>LastX+14
			    || target->Y<LastY-14 || target->Y>LastY+14  ) {
		    LastCycle=GameCycle+CYCLES_PER_SECOND*2;
		    LastX=target->X;
		    LastY=target->Y;
		    PlayUnitSound(target,VoiceHelpMe);
		}
	    }
	}
	NotifyPlayer(target->Player,NotifyRed,target->X,target->Y,
		"%s attacked",target->Type->Name);
	if( target->Player->AiEnabled ) {
	    AiHelpMe(attacker,target);
	}
    }
    target->Attacked=7;

    if( target->HP<=damage ) {	// unit is killed or destroyed
	if( attacker ) {
	    attacker->Player->Score+=target->Type->Points;
	    if( type->Building ) {
		attacker->Player->TotalRazings++;
	    }
	    else {
		attacker->Player->TotalKills++;
	    }
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

    // david: capture enemy buildings
    // Only worker types can capture.
    // Still possible to destroy building if not careful (too many attackers)
    if( EnableBuildingCapture && attacker
	    && type->Building && target->HP<=damage*3
	    && IsEnemy(attacker->Player,target)
	    && (attacker->Type==UnitTypeOrcWorker
		|| attacker->Type==UnitTypeHumanWorker) ) {
	ChangeUnitOwner(target, target->Player, attacker->Player);
	CommandStopUnit(attacker);	// Attacker shouldn't continue attack!
    }

    if( UnitVisibleOnMap(target) ) {
	MakeLocalMissile(MissileTypeHit,
	    target->X*TileSizeX+target->Type->TileWidth*TileSizeX/2,
	    target->Y*TileSizeY+target->Type->TileHeight*TileSizeY/2,
	    target->X*TileSizeX+target->Type->TileWidth*TileSizeX/2+3,
	    target->Y*TileSizeY+target->Type->TileHeight*TileSizeY/2
		    -MissileTypeHit->Range)->Damage=-damage;
    }

#if 0
    // FIXME: want to show hits.
    if( type->Organic ) {
	MakeMissile(MissileBlood
		,target->X*TileSizeX+TileSizeX/2
		,target->Y*TileSizeY+TileSizeY/2,0,0);
    }
    if( type->Building ) {
	MakeMissile(MissileSmallFire
		,target->X*TileSizeX+(type->TileWidth*TileSizeX)/2
		,target->Y*TileSizeY+(type->TileHeight*TileSizeY)/2
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
		    ,target->X*TileSizeX+(type->TileWidth*TileSizeX)/2
		    ,target->Y*TileSizeY+(type->TileHeight*TileSizeY)/2
			    -TileSizeY
		    ,0,0);
	    missile->SourceUnit=target;
	    target->Burning=1;
	    ++target->Refs;
	} else {
	    missile=MakeMissile(MissileTypeBigFire
		    ,target->X*TileSizeX+(type->TileWidth*TileSizeX)/2
		    ,target->Y*TileSizeY+(type->TileHeight*TileSizeY)/2
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
    return isqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
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
	    _C_ x1 _C_ y1 _C_ x2 _C_ y2 _C_ isqrt(dx*dx+dy*dy));

    return isqrt(dx*dx+dy*dy);
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

    return isqrt(dx*dx+dy*dy);
}

/**
**	Compute the distance from the view point to a given point.
**
**	@param x	X map tile position.
**	@param y	Y map tile position.
**
**	@todo
**		FIXME: is it the correct place to put this function in?
*/
global int ViewPointDistance(int x, int y)
{
    const Viewport *vp;

    // first compute the view point coordinate
    vp = TheUI.SelectedViewport;

    // then use MapDistance
    return MapDistance(vp->MapX + vp->MapWidth / 2,
	vp->MapY + vp->MapHeight / 2, x, y);
}

/**
**	Compute the distance from the view point to a given unit.
**
**	@param dest	Distance to this unit.
**
**	@todo
**		FIXME: is it the correct place to put this function in?
*/
global int ViewPointDistanceToUnit(const Unit* dest)
{
    const Viewport* vp;

    // first compute the view point coordinate
    vp = TheUI.SelectedViewport;
    // then use MapDistanceToUnit
    return MapDistanceToUnit(vp->MapX + vp->MapWidth / 2,
	    vp->MapY + vp->MapHeight / 2, dest);
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
**	Check if unit is shared vision.
**
**	@param player	The source player.
**	@param dest	The destination unit.
**
**	@return		Returns true, if the destination unit is shared
**			vision.
*/
global int IsSharedVision(const Player* player,const Unit* dest)
{
    return (player->SharedVision&(1<<dest->Player->Player)) &&
	   (dest->Player->SharedVision&(1<<player->Player));
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

    fprintf(file,"(");
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

	default:
	    DebugLevel0Fn("Unknown action in order\n");
    }
    fprintf(file," flags %d",order->Flags);
    fprintf(file," range (%d %d)",order->RangeX,order->RangeY);
    if( order->Goal ) {
	if (order->Goal->Destroyed) {
	    /* this unit is destroyed so it's not in the global unit
	     * array - this means it won't be saved!!! */
	    printf ("FIXME: storing destroyed Goal - loading will fail.\n");
	}
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
	    case UnitActionMineGold:
		fprintf(file," mine (%d %d)",
			(int)order->Arg1>>16,(int)order->Arg1&0xFFFF);
		break;
	    default:
		fprintf(file," arg1 %d",(int)order->Arg1);
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

    fprintf(file,"\n(unit %d ",UnitNumber(unit));

    // 'type and 'player must be first, needed to create the unit slot
    fprintf(file,"'type '%s ",unit->Type->Ident);
    fprintf(file,"'seen-type '%s ",unit->Type->Ident);
    fprintf(file,"'player %d\n  ",unit->Player->Player);

    if( unit->Name ) {
	fprintf(file,"'name \"%s\" ",unit->Name);
    }

    if( unit->Next ) {
	fprintf(file,"'next '%d ",UnitNumber(unit->Next));
    }

    fprintf(file,"'tile '(%d %d) ",unit->X,unit->Y);
#if 0
    /* latimerius: why is this so complex? */
    // JOHNS: An unit can be owned by a new player and have still the old stats
    for( i=0; i<PlayerMax; ++i ) {
	if( &unit->Type->Stats[i]==unit->Stats ) {
	    fprintf(file,"'stats %d\n  ",i);
	    break;
	}
    }
    /* latimerius: what's the point of storing a pointer value anyway? */
    if( i==PlayerMax ) {
	fprintf(file,"'stats 'S%08X\n  ",(int)unit->Stats);
    }
#else
    fprintf (file, "'stats %d\n  " ,unit->Player->Player);
#endif
    fprintf(file,"'pixel '(%d %d) ",unit->IX,unit->IY);
    fprintf(file,"'%sframe %d ",
	    unit->Frame<0 ? "flipped-" : "" ,unit->Frame<0?-unit->Frame:unit->Frame);
    if( unit->SeenFrame!=UnitNotSeen ) {
	fprintf(file,"'%sseen %d ",
		unit->SeenFrame<0 ? "flipped-" : "" ,unit->SeenFrame<0?-unit->SeenFrame:unit->SeenFrame);
    } else {
	fprintf(file,"'not-seen ");
    }
    fprintf(file,"'direction %d\n  ",unit->Direction);
    fprintf(file,"'attacked %d\n ",unit->Attacked);
#ifdef NEW_FOW
    fprintf(file," 'current-sight-range %d",unit->CurrentSightRange);
#endif
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
#ifdef NEW_FOW
    if( unit->Next && unit->Removed ) {
	fprintf(file," 'host-tile '(%d %d) ",
		unit->Next->X+unit->Next->Type->TileWidth/2,
		unit->Next->Y+unit->Next->Type->TileHeight/2);
    }
#endif
    fprintf(file," 'visible \"");
    for( i=0; i<PlayerMax; ++i ) {
	fputc((unit->Visible&(1<<i)) ? 'X' : '_',file);
    }
    fprintf(file,"\"\n ");
    if( unit->Constructed ) {
	fprintf(file," 'constructed");
    }
    if( unit->SeenConstructed ) {
	fprintf(file," 'seen-constructed");
    }
    fprintf(file," 'seen-state %d ",unit->SeenState);
    if( unit->Active ) {
	fprintf(file," 'active");
    }
    fprintf(file," 'mana %d",unit->Mana);
    fprintf(file," 'hp %d",unit->HP);
    fprintf(file," 'xp %d",unit->XP);
    fprintf(file," 'kills %d\n  ",unit->Kills);

    fprintf(file,"'ttl %lu ",unit->TTL);
    fprintf(file,"'bloodlust %d ",unit->Bloodlust);
    fprintf(file,"'haste %d ",unit->Haste);
    fprintf(file,"'slow %d\n  ",unit->Slow);
    fprintf(file,"'invisible %d ",unit->Invisible);
    fprintf(file,"'flame-shield %d ",unit->FlameShield);
    fprintf(file,"'unholy-armor %d\n  ",unit->UnholyArmor);

    fprintf(file,"'group-id %d\n  ",unit->GroupId);
    fprintf(file,"'last-group %d\n  ",unit->LastGroup);

    fprintf(file,"'value %d\n  ",unit->Value);

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
	    fprintf(file,"%s",ref=UnitReference(unit->OnBoard[i]));
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
    fprintf(file,")\n  'saved-order '");
    SaveOrder(&unit->SavedOrder,file);
    fprintf(file,"\n  'new-order '");
    SaveOrder(&unit->NewOrder,file);

    //
    //	Order data part
    //
    switch( unit->Orders[0].Action ) {
	case UnitActionStill:
	    // FIXME: support other resource types
	    if( unit->Type->GoldMine || unit->Type->GivesOil ) {
		fprintf(file," 'resource-active %d",unit->Data.Resource.Active);
	    }
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
	    fprintf(file,"\n  'data-research '(");
	    fprintf(file,"ident %s", unit->Data.Research.Upgrade->Ident);
	    fprintf(file,")");
	    break;
	case UnitActionUpgradeTo:
	    fprintf(file,"\n  'data-upgrade-to '(");
	    fprintf(file,"ticks %d", unit->Data.UpgradeTo.Ticks);
	    fprintf(file,")");
	    break;
	case UnitActionTrain:
	    fprintf(file,"\n  'data-train '(");
	    fprintf (file, "ticks %d ", unit->Data.Train.Ticks);
	    fprintf (file, "count %d ", unit->Data.Train.Count);
	    fprintf (file, "queue #(");
	    for (i=0; i<MAX_UNIT_TRAIN; i++) {
		if (i < unit->Data.Train.Count) {
		    fprintf (file, "%s ", unit->Data.Train.What[i]->Ident);
		} else {
		    /* this slot is currently unused */
		    fprintf (file, "unit-none ");
		}
	    }
	    fprintf (file, "))");
	    break;
	default:
	    fprintf(file,"\n  'data-move '(");
	    if( unit->Data.Move.Fast ) {
		fprintf(file,"fast ");
	    }
	    if( unit->Data.Move.Length ) {
		fprintf(file,"path #(");
		for( i=0; i<unit->Data.Move.Length; ++i ) {
		    fprintf(file,"%d ", unit->Data.Move.Path[i]);
		}
		fprintf(file,")");
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
    int i;
    unsigned char SlotUsage[MAX_UNIT_SLOTS/8 + 1];
    int InRun, RunStart;

    fprintf(file,"\n;;; -----------------------------------------\n");
    fprintf(file,";;; MODULE: units $Id$\n\n");

    //
    //	Local variables
    //
    fprintf(file,"(set-hitpoint-regeneration! #%s)\n",
	    HitPointRegeneration ? "t" : "f");
    fprintf(file,"(set-xp-damage! #%s)\n",
	    XpDamage ? "t" : "f");
    fprintf(file,"(set-fancy-buildings! #%s)\n",
	    FancyBuildings ? "t" : "f");
    fprintf(file,"(set-training-queue! #%s)\n",
	    EnableTrainingQueue ? "t" : "f");

    fprintf (file, "; Unit slot usage bitmap\n");
    fprintf (file, "(slot-usage '(");

    memset (SlotUsage, 0, MAX_UNIT_SLOTS/8 + 1);
    for (i=0; i<NumUnits; i++) {
	int slot = Units[i]->Slot;
	SlotUsage[slot/8] |= 1 << (slot%8);
    }
#if 0
    /* the old way */
    for (i=0; i<MAX_UNIT_SLOTS/8 + 1; i++) {
	fprintf (file, " %d", SlotUsage[i]);
	if ( (i+1) % 16 == 0 )		// 16 numbers per line
	    fprintf (file, "\n");
    }

#else
#define SlotUsed(slot)	( SlotUsage[(slot)/8] & (1 << ((slot)%8)) )
    RunStart = InRun = 0;
    for (i=0; i<MAX_UNIT_SLOTS; i++) {
	if ( !InRun && SlotUsed (i) ) {
	    InRun = 1;
	    RunStart = i;
	}
	if ( !SlotUsed (i) && InRun) {
	    InRun = 0;
	    if (i-1 == RunStart) {
		fprintf (file, "%d ", i-1);
	    } else {
		fprintf (file, "%d - %d ", RunStart, i-1);
	    }
	}
    }
#endif

    fprintf (file, "))\n");

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
	if( (*table)->Name ) {
	    free((*table)->Name);
	}
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
    XpDamage=0;
    FancyBuildings=0;
}

//@}
