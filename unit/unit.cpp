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
/**@name unit.c		-	The units. */
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
#include <string.h>
#include <limits.h>
#include <math.h>

#include "stratagus.h"

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
global Unit* DestroyedBuildings;	/// List of DestroyedBuildings
global Unit* CorpseList;		/// List of Corpses On Map

global int HitPointRegeneration;	/// Hit point regeneration for all units
global int XpDamage;			/// Hit point regeneration for all units
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
    DestroyedBuildings=NULL;
    CorpseList=NULL;
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

	// Mark building as can't be destroyed, since it's still seen
	if( unit->Type->Building ) {
	    int i;
	    int x;
	    int y;
	    int w;
	    int w0;
	    int h;

	    // Mark the Destroyed building, with who has seen it destroyed.
	    x = unit->X;
	    y = unit->Y;
	    unit->Visible = 0x0000;
	    for( i=0; i < PlayerMax; i++ ) {
		w = w0 = unit->Type->TileWidth;
		h = unit->Type->TileHeight;
		for( ; h-->0; ) {
		    for( w=w0; w-->0; ) {
			if( !IsMapFieldVisible(&Players[i],x+w,y+h)
				&& IsMapFieldExplored(&Players[i],x+w,y+h)
				&& Players[i].Type == PlayerPerson ) {
			    unit->Visible |= (1 << i);
			}
		    }
		}
	    }
	}

	if( unit->Type->Building && unit->Visible != 0x0000 ) {
	    return;
	}
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

    // Update Corpse Cache
    if( unit->Orders[0].Action == UnitActionDie ) {
	if( unit->Type->Building ) {
	    DeadBuildingCacheRemove(unit);
	} else {
	    CorpseCacheRemove(unit);
	}
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
	    // unit->Type=NULL;			// for debugging.
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
	if (symbol_boundp(fun, NIL)) {
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
    if (!type->PermanentCloak) {
	unit->Visible = -1;		// Visible as default
    }

    unit->Rs = MyRand() % 100;		// used for fancy buildings and others

    unit->OrderCount = 1;		// No orders
    unit->Orders[0].Action = UnitActionStill;
    unit->Orders[0].X=unit->Orders[0].Y=-1;
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
    UnitType* type;

    type=unit->Type;

    //
    //	Build player unit table    
    //
    if( player && !type->Vanishes && unit->Orders[0].Action!=UnitActionDie ) {
	unit->PlayerSlot=player->Units+player->TotalNumUnits++;
	if( type->_HitPoints!=0 ) {
	    if( type->Building ) {
		// FIXME: support more races
		if( type!=UnitTypeOrcWall && type!=UnitTypeHumanWall ) {
		    player->TotalBuildings++;
		}
	    } else {
		player->TotalUnits++;
	    }
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
	// FIXME: support more races
	if( type!=UnitTypeOrcWall && type!=UnitTypeHumanWall ) {
	    player->NumBuildings++;
	}
    }

    unit->Player=player;
    unit->Stats=&type->Stats[unit->Player->Player];
    unit->Colors=&player->UnitColors;
    unit->HP = unit->Stats->HitPoints;

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

    //DebugCheck(!player);		// Current code didn't support no player

    unit = AllocUnit();
    InitUnit(unit, type);

    // Only Assign if a Player was specified
    if ( player ) {
	AssignUnitToPlayer(unit, player);
    }

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

    x+=unit->Type->TileWidth/2;
    y+=unit->Type->TileHeight/2;

    //
    //	Units under construction have no sight range.
    //
    if( !unit->Constructed ) {
	//
	//	Update fog of war, if unit belongs to player on this computer
	//
	if( unit->Container && unit->Removed ) {
	    MapUnmarkUnitOnBoardSight(unit,unit->Container);
	}
	if (unit->Container) {
	    RemoveUnitFromContainer(unit);
	}
	unit->CurrentSightRange=unit->Stats->SightRange;
	MapMarkUnitSight(unit);

	if( type->DetectCloak ) {
	    MapDetectCloakedUnits(unit);
	}
    }

    unit->Removed=0;
    UnitCacheInsert(unit);

    MustRedraw|=RedrawMinimap;
    CheckUnitToBeDrawn(unit);
    UnitMarkSeen(unit);
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

/*
 ** 	Add unit to a container. It only updates linked list stuff
 **
 **	@param unit	Pointer to unit.
 *	@param host	Pointer to container.
 */
global void AddUnitInContainer(Unit* unit, Unit* host)
{
    if (unit->Container) {
	DebugLevel0Fn("Unit is already contained.\n");
	exit(0);
    }
    unit->Container=host;
    if (host->InsideCount==0) {
	unit->NextContained=unit->PrevContained=unit;
    } else {
	unit->NextContained=host->UnitInside;
	unit->PrevContained=host->UnitInside->PrevContained;
	host->UnitInside->PrevContained->NextContained=unit;
	host->UnitInside->PrevContained=unit;
    }
    host->UnitInside=unit;
    ++host->InsideCount;
}

/*
 ** 	Remove unit from a container. It only updates linked list stuff
 **
 **	@param unit	Pointer to unit.
 */
global void RemoveUnitFromContainer(Unit* unit)
{
    Unit* host;
    host=unit->Container;
    if (!unit->Container) {
	DebugLevel0Fn("Unit not contained.\n");
	exit(0);
    }
    if (host->InsideCount==0) {
	DebugLevel0Fn("host's inside count reached -1.");
	exit(0);
    }
    host->InsideCount--;
    unit->NextContained->PrevContained=unit->PrevContained;
    unit->PrevContained->NextContained=unit->NextContained;
    if (host->InsideCount==0) {
	host->UnitInside=NoUnitP;
    } else {
	if (host->UnitInside==unit)
	    host->UnitInside=unit->NextContained;
    }
    unit->Container=NoUnitP;
}

/*
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

    if( unit->Removed && unit->Container ) {
	MapUnmarkUnitOnBoardSight(unit,unit->Container);
    } else {
	MapUnmarkUnitSight(unit);
    }
    if( host ) {
	unit->CurrentSightRange=host->CurrentSightRange;
	MapMarkUnitOnBoardSight(unit,host);
	AddUnitInContainer(unit,host);
    }

    if( unit->Removed ) {		// could happen!
	// If unit is removed (inside) and building is destroyed.
	return;
    }
    unit->Removed=1;
    //  Remove unit from the current selection
    if( unit->Selected ) {
#ifndef NEW_UI
	if( NumSelected==1 ) {		//  Remove building cursor
	    CancelBuildingMode();
	}
	MustRedraw|=RedrawPanels;
#endif
	UnSelectUnit(unit);
	SelectionChanged();
    }

    // Unit is seen as under cursor
    if( unit==UnitUnderCursor ) {
	UnitUnderCursor=NULL;
    }

    // FIXME: unit is tracked?

    type=unit->Type;

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
	    // FIXME: support more races
	    if( type!=UnitTypeOrcWall && type!=UnitTypeHumanWall ) {
		player->NumBuildings--;
	    }
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

    //	Destroy resource-platform, must re-make resource patch.
    if( type->MustBuildOnTop && unit->Value>0 ) {
	temp=MakeUnitAndPlace(unit->X,unit->Y,type->MustBuildOnTop,&Players[15]);
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
 **	Returns true, if unit is visible for this player on the map.
 **	An unit is visible, if any field could be seen.
 **
 **	@warning	This is only true for ::ThisPlayer.
 **
 **	@param unit	Unit to be checked.
 **	@return		True if visible, false otherwise.
 */
global int BuildingVisibleOnMap(const Unit* unit)
{
    int x;
    int y;
    int w;
    int w0;
    int h;

    DebugCheck( !unit->Type );	// FIXME: Can this happen, if yes it is a bug
    
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
 **	FIXME: docu
 **
 **	@param x	x location to check if building is on, and mark as seen
 **	@param y	y location to check if building is on, and mark as seen
 */
global void UnitsMarkSeen(int x,int y)
{
    int n;
    Unit* units[UnitMax];
    Unit* unit;

    if( IsMapFieldVisible(ThisPlayer, x, y) ) {
	n = SelectUnitsOnTile(x,y,units);
	DebugLevel3Fn("I can see %d units from here.\n" _C_ n);
	// FIXME: need to handle Dead buldings
	while( n ) {
	    unit=units[n-1];
	    if (unit->SeenFrame==UnitNotSeen) {
		DebugLevel3Fn("unit %d at %d,%d first seen at %lu.\n" _C_ unit->Slot _C_ unit->X _C_ unit->Y _C_ GameCycle);
	    }
	    unit->SeenIY=unit->IY;
	    unit->SeenIX=unit->IX;
	    unit->SeenFrame = unit->Frame;
	    unit->SeenType = unit->Type;
	    unit->SeenState = (unit->Orders[0].Action==UnitActionBuilded) |
		((unit->Orders[0].Action==UnitActionUpgradeTo) << 1);
	    if( unit->Orders[0].Action==UnitActionDie ) {
		unit->SeenState = 3;
	    }
	    unit->SeenConstructed = unit->Constructed;
	    unit->SeenDestroyed = unit->Destroyed;
	    --n;
	}
    }
}

/**
 **	FIXME: docu
 **
 **	@param unit	pointer to the unit to check if seen
 */
global void UnitMarkSeen(Unit* unit)
{
    int x;
    int y;

    // Update Building Seen
    if( !unit->Type ) {
	DebugLevel0Fn("UnitMarkSeen: Type is NULL\n" );
	return;
    }
    for (x=0; x<unit->Type->TileWidth; x++) {
	for (y=0; y<unit->Type->TileHeight; y++) {
	    if( IsMapFieldVisible(ThisPlayer,unit->X+x,unit->Y+y) ) {
		unit->SeenIY=unit->IY;
		unit->SeenIX=unit->IX;
		if (unit->SeenFrame==UnitNotSeen) {
		    DebugLevel3Fn("unit %d at %d,%d first seen at %lu.\n" _C_ unit->Slot _C_ unit->X _C_ unit->Y _C_ GameCycle);
		}
		unit->SeenFrame = unit->Frame;
		unit->SeenType = unit->Type;
		unit->SeenState = (unit->Orders[0].Action==UnitActionBuilded) |
		    ((unit->Orders[0].Action==UnitActionUpgradeTo) << 1);
		if( unit->Orders[0].Action==UnitActionDie ) {
		    unit->SeenState = 3;
		}
		unit->SeenConstructed = unit->Constructed;
		unit->SeenDestroyed = unit->Destroyed;
		x=unit->Type->TileWidth;
		y=unit->Type->TileHeight;
		//  If we found one visible square, END.
		break;
	    }
	}
    }
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

    // FIXME: Need to be able to see enemy submarines seen by my shared vision
    //		partners
    if( ThisPlayer != unit->Player &&
	    !(unit->Player->SharedVision&(1<<ThisPlayer->Player) &&
		ThisPlayer->SharedVision&(1<<unit->Player->Player)) ) {
	// Invisible by spell
	if ( unit->Invisible ) {
	    return 0;
	}
	// Visible submarine
	if ( !(unit->Visible&(1<<ThisPlayer->Player)) && !unit->Type->Building ) {
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
    if (!unit->Removed && UnitVisibleOnScreen(unit)) {
	int x;
	int y;
	int w;
	int h;

	// FIXME: Inaccurate dimension to take unit's extras into account..
	//        Should be solved by adding each unit extra as separate decoration
	x = Map2ScreenX(unit->X)+unit->IX-10;
	y = Map2ScreenY(unit->Y)+unit->IY-10;
	w = unit->Type->Width+20;
	h = unit->Type->Height+20;

	if (unit->deco) {
	    // FIXME: its very expensive to remove+add a decoration to satify a
	    //        new location, a decoration update function should be added
	    Deco *d = unit->deco;
	    if ( d->x != x || d->y != y || d->w != w || d->h != h ) {
		DecorationRemove( unit->deco );
		AddUnitDeco((Unit *)unit, x, y, w, h);
	    }
	    else {
		DecorationMark(unit->deco);
	    }
	}
	else {
	    AddUnitDeco((Unit *)unit, x, y, w, h);
	}

	return 1;
    } else if (unit->deco) {
	// not longer visible: so remove from auto decoration redraw
	Unit *u = (Unit *)unit;
	DecorationRemove(unit->deco);
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

/**
 **	Do the runestone work each second.
 */
global void DoRunestones(void)
{
    Unit* units[UnitMax];
    Unit* stones[UnitMax];
    int nstones;
    int nunits;
    int i;
    int j;
    unsigned tmp;

    // FIXME: Don't use UnitTypeByIdent during runtime
    nstones = FindUnitsByType(UnitTypeByIdent("unit-runestone"),stones);
    for (i = 0; i < nstones; ++i) {
	// Get all the units around the runestone
	nunits = SelectUnits(stones[i]->X - stones[i]->Stats->SightRange,
		stones[i]->Y - stones[i]->Stats->SightRange,
		stones[i]->X + stones[i]->Stats->SightRange+1,
		stones[i]->Y + stones[i]->Stats->SightRange+1,
		units);
	// Runestone Mana and HP on units, 2 every time
	for (j = 0; j < nunits; ++j) {
	    if (units[j] == stones[i]) {
		continue;
	    }

	    // Restore HP in everything but buildings (even in other player's units)
	    if (!units[j]->Type->Building && units[j]->Type->Organic && 
		    units[j]->HP != units[j]->Stats->HitPoints ) {
		tmp = units[j]->Stats->HitPoints - units[j]->HP;
		if (tmp > 2) {
		    tmp = 2;
		}
		units[j]->HP += tmp;
	    }

	    // Restore mana in all magical units
	    if(units[j]->Type->CanCastSpell && units[j]->Mana != MaxMana)  {	
		tmp = MaxMana - units[j]->Mana;
		if (tmp > 2) {
		    tmp = 2;
		}
		units[j]->Mana += tmp;
	    }
	}
    }
}

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

	if (  unit->Type->PermanentCloak ) {
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
	if( unit->Type->DetectCloak && !unit->Removed &&
		unit->Orders[0].Action!=UnitActionBuilded ) {
	    MapDetectCloakedUnits(unit);
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
 **	@param newplayer	New owning player.
 **
 **	@todo	FIXME: I think here are some failures, if building is build
 **		what is with the unit inside? or a main hall with workers
 **		inside?
 */
global void ChangeUnitOwner(Unit* unit,Player* newplayer)
{
    int i;
    Unit* uins;
    Player* oldplayer;

    oldplayer=unit->Player;

    // This shouldn't happen
    if (oldplayer==newplayer) {
	DebugLevel0Fn("Change the unit owner to the same player???\n");
	return;
    }

    // Rescue all units in buildings/transporters.
    uins=unit->UnitInside;
    for( i=unit->InsideCount; i; --i,uins=uins->NextContained) {
	ChangeUnitOwner(uins,newplayer);
    }

    //
    //	Must change food/gold and other.
    //
    UnitLost(unit);

    //  Adjust Orders to remove Attack Order
    //  Mainly to protect peasants who are building.
    //  FIXME: What's the point in this code? It just causes a crash when
    //  FIXME: an unit is moving (the unit stops when between map cells.)

    /*for( i=0; i < MAX_ORDERS; i++) {
      if (unit->Orders[i].Action==UnitActionAttack ||
      unit->Orders[i].Action==UnitActionAttackGround) {
    //Now see if it's an enemy..
    //FIXME:Just Stops attacking at the moment
    printf("Stopped attack for a/an %s,\n",unit->Type->Name);
    unit->Orders[i].Action=UnitActionStill;
    unit->SubAction=unit->State=0;
    break;
    }
    }*/

    //
    //	Now the new side!
    //

    //	Insert into new player table.

    unit->PlayerSlot=newplayer->Units+newplayer->TotalNumUnits++;
    if( unit->Type->_HitPoints!=0 ) {
	if( unit->Type->Building ) {
	    newplayer->TotalBuildings++;
	}
	else {
	    newplayer->TotalUnits++;
	}
    }
    *unit->PlayerSlot=unit;

    if ( unit->Removed && unit->Container ) {
	MapUnmarkUnitOnBoardSight(unit,unit->Next);
	unit->Player=newplayer;
	MapMarkUnitOnBoardSight(unit,unit->Next);
    } else {
	MapUnmarkUnitSight(unit);
	unit->Player=newplayer;
	MapMarkUnitSight(unit);
    }

    unit->Stats=&unit->Type->Stats[newplayer->Player];
    //
    //	Must change food/gold and other.
    //
    if( unit->Type->GivesResource ) {
	DebugLevel0Fn("Resource transfer not supported\n");
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
	// Don't save the unit again(can happen when inside a town hall)
	if (unit->Player==newplayer) {
	    continue;
	}
	ChangeUnitOwner(unit,newplayer);
	unit->Blink=5;
	unit->RescuedFrom=oldplayer;
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
		// Do not rescue removed units. Units inside something are
		// rescued by ChangeUnitOwner
		if (unit->Removed) {
		    continue;
		}
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
		    if( around[i]->Type->CanAttack &&
			    IsAllied(unit->Player,around[i]) ) {
			//
			//	City center converts complete race
			//	NOTE: I use a trick here, centers could
			//		store gold. FIXME!!!
			if( unit->Type->CanStore[GoldCost] ) {
			    ChangePlayerOwner(p,around[i]->Player);
			    break;
			}
			unit->RescuedFrom=unit->Player;
			ChangeUnitOwner(unit,around[i]->Player);
			unit->Blink=5;
			PlayGameSound(GameSounds.Rescue[unit->Player->Race].Sound
				,MaxSampleVolume);
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

    //FIXME: vladi: this debug check fails when used for teleporting...
    //DebugCheck( !unit->Removed );

    // FIXME: better and quicker solution, to find the building.
    if( unit->Container ) {
	x=unit->Container->X;
	y=unit->Container->Y;
    } else {
	x=unit->X;
	y=unit->Y;
	// n0b0dy: yes, when training an unit.
    }


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

    DebugLevel3Fn("%d\n" _C_ UnitNumber(unit));
    DebugCheck( !unit->Removed );

    // FIXME: better and quicker solution, to find the building.
    x=y=-1;
    if( unit->Container ) {
	x=unit->Container->X;
	y=unit->Container->Y;
    } else {
	x=unit->X;
	y=unit->Y;
    }

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
    Unit* unit;
    int i;

    unit=source->UnitInside;
    for( i=source->InsideCount; i; --i,unit=unit->NextContained ) {
	DropOutOnSide(unit,LookingW
		,source->Type->TileWidth,source->Type->TileHeight);
	DebugCheck( unit->Orders[0].Goal );
	unit->Orders[0].Action=UnitActionStill;
	unit->Wait=unit->Reset=1;
	unit->SubAction=0;
    }
    DebugLevel0Fn("Drop out %d of %d\n" _C_ i _C_ source->Data.Resource.Active);
}

/*----------------------------------------------------------------------------
  --	Building units
  ----------------------------------------------------------------------------*/

/**
 **		Can build unit here.
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
    int w;
    int h;
    int resource;
    int found;

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
	if( type->GivesResource==OilCost ) {
	    // FIXME: Better ideas? type->OnlyPlaceable on odd tiles? Yuck.
	    // Oil patches and platforms can only be placed on even tiles
	    if( !(x&1 && y&1) ) {
		return 0;
	    }
	    n=UnitCacheSelect(x,y,x+type->TileWidth,y+type->TileHeight,table);
	    for( i=0; i<n; ++i ) {
		if( table[i]->Type->GivesResource==OilCost ) {
		    return 0;
		}
	    }
	} else if( type->UnitType==UnitTypeFly || type->UnitType==UnitTypeNaval ) {
	    // Flyers and naval units can only be placed on odd tiles
	    if( x&1 || y&1 ) {
		return 0;
	    }
	}
    }

    // Must be checked before oil!
    if( type->ShoreBuilding ) {
	found=0;

	DebugLevel3("Shore building\n");
	// Need atleast one coast tile
	for( h=type->TileHeight; h--; ) {
	    for( w=type->TileWidth; w--; ) {
		if( TheMap.Fields[x+w+(y+h)*TheMap.Width].Flags
			&MapFieldCoastAllowed ) {
		    h=w=0;
		    found=1;
		}
	    }
	}
	if ( !found ) {
	    return 0;
	}
    }
    
    //	resource deposit can't be build too near to resource
    // FIXME: use unit-cache here.
    for( i=0; i<NumUnits; i++ ) {
	unit=Units[i];
	for (resource=1;resource<MaxCosts;resource++) {
	    if (( type->CanStore[resource] && unit->Type->GivesResource==resource )||
		    ( unit->Type->CanStore[resource] && type->GivesResource==resource )) {
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
		if( dx<RESOURCE_DISTANCE && dy<RESOURCE_DISTANCE ) {
		    return 0;
		}
	    }
	}
    }

    if( type->MustBuildOnTop && !EditorRunning) {
	// Resource platform could only be build on resource patch.
	n=UnitCacheSelect(x,y,x+1,y+1,table);
	for( i=0; i<n; ++i ) {
	    if( table[i]->Type!=type->MustBuildOnTop ) {
		continue;
	    }
	    if( table[i]->Orders[0].Action==UnitActionBuilded ) {
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

    // Terrain Flags don't matter.
    if ( type->MustBuildOnTop ) {
	return CanBuildHere(type,x,y);
    }

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
 **	Find the closest piece of wood for an unit.
 **
 **	@param unit	The unit.
 **	@param x	OUT: Map X position of tile.
 **	@param y	OUT: Map Y position of tile.
 */
global int FindWoodInSight(const Unit* unit,int* x,int* y)
{
    return FindTerrainType(UnitMovementMask(unit),0,MapFieldForest,9999,
	    unit->Player,unit->X,unit->Y,x,y);
}

/**
 **	Find the closest piece of terrain with the given flags.
 **
 **	@param movemask	The movement mask to reach that location. 
 **	@param resmask	Result tile mask.
 **	@param rvresult Return a tile that doesn't match. 
 **	@param range	Maximum distance for the search.
 **	@param player	Only search fields explored by player
 **	@param x	Map X start position for the search.
 **	@param y	Map Y start position for the search.
 **
 **	@param px	OUT: Map X position of tile.
 **	@param py	OUT: Map Y position of tile.
 **
 **	@note		Movement mask can be 0xFFFFFFFF to have no effect
 **			Range is not circular, but square.
 **			Player is ignored if nil(search the entire map)
 **			Use rvresult if you search for a til;e that doesn't
 **			match resmask. Like for a tile where an unit can go
 **			with it's movement mask.
 **
 **	@return		True if wood was found.
 */
global int FindTerrainType(int movemask,int resmask,int rvresult,int range,
	const Player *player,int x,int y,int* px,int* py)
{
    static const int xoffset[]={  0,-1,+1, 0, -1,+1,-1,+1 };
    static const int yoffset[]={ -1, 0, 0,+1, -1,-1,+1,+1 };
    struct {
	unsigned short X;
	unsigned short Y;
    } * points;
    int size;
    int rx;
    int ry;
    int wp;
    int rp;
    int ep;
    int i;
    int w;
    unsigned char* m;
    unsigned char* matrix;
    int destx;
    int desty;
    int cdist;

    destx=x;
    desty=y;
    size=min(TheMap.Width*TheMap.Height/4,range*range*5);
    points=malloc(size*sizeof(*points));

    //	Make movement matrix. FIXME: can create smaller matrix.
    matrix=CreateMatrix();
    w=TheMap.Width+2;
    matrix+=w+w+2;
    points[0].X=x;
    points[0].Y=y;
    rp=0;
    matrix[x+y*w]=1;			// mark start point
    ep=wp=1;				// start with one point
    cdist=0;				// current distance is 0

    //
    //	Pop a point from stack, push all neighbors which could be entered.
    //
    for( ;; ) {
	while( rp!=ep ) {
	    rx=points[rp].X;
	    ry=points[rp].Y;
	    DebugLevel3("%d,%d\n" _C_ rx _C_ ry);
	    for( i=0; i<8; ++i ) {		// mark all neighbors
		x=rx+xoffset[i];
		y=ry+yoffset[i];
		//  Make sure we don't leave the map.
		if (x<0||y<0||x>=TheMap.Width||y>=TheMap.Height) {
		    continue; 
		}
		m=matrix+x+y*w;
		//  Check if visited or unexplored
		if( *m || (player&&!IsMapFieldExplored(player,x,y))) {
		    continue;
		}
		//	Look if found what was required.
		if ( rvresult?CanMoveToMask(x,y,resmask):!CanMoveToMask(x,y,resmask) ) {
		    *px=x;
		    *py=y;
		    DebugLevel3("Found it! %X %X\n" _C_ TheMap.Fields[x+y*TheMap.Width].Flags _C_ resmask);
		    return 1;
		}
		if( CanMoveToMask(x,y,movemask) ) {	// reachable
		    *m=1;
		    points[wp].X=x;		// push the point
		    points[wp].Y=y;
		    if( ++wp>=size ) {		// round about
			wp=0;
		    }
		    if (wp==ep) {
			//  We are out of points, give up!
			DebugLevel0Fn("Ran out of points the hard way, beware.\n");
			break;
		    }
		} else {			// unreachable
		    *m=99;
		}
	    }
	    if( ++rp>=size ) {			// round about
		rp=0;
	    }
	}
	cdist++;
	if( rp==wp||cdist>=range ) {			// unreachable, no more points available
	    break;
	}
	//	Continue with next set.
	ep=wp;
    }
    free(points);
    return 0;
}

/**
 **	Find Resource.
 **
 **	@param unit	The unit that wants to find a resource.
 **	@param x	Closest to x
 **	@param y	Closest to y
 **	@param range    Maximum distance to the resource.
 **	@param resource The resource id.
 **
 **	@note 		This will return an usable resource building that
 **			belongs to "player" or is neutral.
 **
 **	@return		NoUnitP or resource unit
 */
global Unit* FindResource(const Unit * unit,int x,int y,int range,int resource)
{
    static const int xoffset[]={  0,-1,+1, 0, -1,+1,-1,+1 };
    static const int yoffset[]={ -1, 0, 0,+1, -1,-1,+1,+1 };
    struct {
	unsigned short X;
	unsigned short Y;
    } * points;
    int size;
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
    int bestd;
    int cdist;

    destx=x;
    desty=y;
    size=min(TheMap.Width*TheMap.Height/4,range*range*5);
    points=malloc(size*sizeof(*points));

    //	Find the nearest gold depot
    if( (destu=FindDeposit(unit,x,y,range,resource)) ) {
	NearestOfUnit(destu,x,y,&destx,&desty);
    }
    bestd=99999;
    //	Make movement matrix. FIXME: can create smaller matrix.
    matrix=CreateMatrix();
    w=TheMap.Width+2;
    matrix+=w+w+2;
    //  Unit movement mask
    mask=UnitMovementMask(unit);
    //  Ignore all units along the way. Might seem wierd, but otherwise
    //  peasants would lock at a mine with a lot of workers.
    mask&=~(MapFieldLandUnit|MapFieldSeaUnit|MapFieldAirUnit);
    points[0].X=x;
    points[0].Y=y;
    rp=0;
    matrix[x+y*w]=1;			// mark start point
    ep=wp=1;				// start with one point
    cdist=0;				// current distance is 0
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

		if (!IsMapFieldExplored(unit->Player,x,y)) { // Unknown.
		    continue;
		}

		//
		//	Look if there is a mine
		//
		if ((mine=ResourceOnMap(x,y,resource))&&
			(mine->Type->CanHarvest)&&
			((mine->Player->Player==PlayerMax-1)||
			(mine->Player==unit->Player)||
			(IsAllied(unit->Player,mine)))) {
		    if( destu ) {
			n=max(abs(destx-x),abs(desty-y));
			if( n<bestd ) {
			    bestd=n;
			    bestmine=mine;
			}
			*m=99;
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
		    if (wp==ep) {
			//  We are out of points, give up!
			break;
		    }
		} else {			// unreachable
		    *m=99;
		}
	    }
	    if( ++rp>=size ) {			// round about
		rp=0;
	    }
	}
	//	Take best of this frame, if any.
	if( bestd!=99999 ) {
	    free(points);
	    return bestmine;
	}
	cdist++;
	if( rp==wp||cdist>=range ) {			// unreachable, no more points available
	    break;
	}
	//	Continue with next set.
	ep=wp;
    }
    DebugLevel3Fn("no resource found\n");
    free(points);
    return NoUnitP;
}

/**
 **	Find deposit. This will find a deposit for a resource 
 **
 **	@param unit	The unit that wants to find a resource.
 **	@param x	Closest to x
 **	@param y	Closest to y
 **	@param range    Maximum distance to the deposit.
 **	@param resource	Resource to find deposit from.
 **
 **	@note		This will return a reachable allied depot.
 **
 **	@return		NoUnitP or deposit unit
 */
global Unit* FindDeposit(const Unit* unit,int x,int y,int range,int resource)
{
    static const int xoffset[]={  0,-1,+1, 0, -1,+1,-1,+1 };
    static const int yoffset[]={ -1, 0, 0,+1, -1,-1,+1,+1 };
    struct {
	unsigned short X;
	unsigned short Y;
    } * points;
    int size;
    int rx;
    int ry;
    int mask;
    int wp;
    int rp;
    int ep;
    int i;
    int w;
    int nodes_searched;
    unsigned char* m;
    unsigned char* matrix;
    Unit* depot;
    int destx;
    int desty;
    int cdist;

    nodes_searched=0;

    destx=x;
    desty=y;
    size=min(TheMap.Width*TheMap.Height/4,range*range*5);
    points=malloc(size*sizeof(*points));

    //	Make movement matrix. FIXME: can create smaller matrix.
    matrix=CreateMatrix();
    w=TheMap.Width+2;
    matrix+=w+w+2;
    //  Unit movement mask
    mask=UnitMovementMask(unit);
    //  Ignore all units along the way. Might seem wierd, but otherwise
    //  peasants would lock at a mine with a lot of workers.
    mask&=~(MapFieldLandUnit|MapFieldSeaUnit|MapFieldAirUnit|MapFieldBuilding);
    points[0].X=x;
    points[0].Y=y;
    rp=0;
    matrix[x+y*w]=1;			// mark start point
    ep=wp=1;				// start with one point
    cdist=0;				// current distance is 0

    DebugLevel3Fn("Searching for a deposit(%d,%d,%d,%d,%s)" _C_ UnitNumber(unit) _C_ x _C_ y _C_ range _C_ DefaultResourceNames[resource]);
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
		nodes_searched++;
		DebugLevel3("(%d,%d) " _C_ x _C_ y);
		//  Make sure we don't leave the map.
		if (x<0||y<0||x>=TheMap.Width||y>=TheMap.Height) {
		    continue; 
		}
		m=matrix+x+y*w;
		//  Check if visited or unexplored
		if( *m || !IsMapFieldExplored(unit->Player,x,y)) {
		    continue;
		}
		//
		//	Look if there is a mine
		//
		if (  (depot=ResourceDepositOnMap(x,y,resource)) &&
			( (IsAllied(unit->Player,depot)) ||
			(unit->Player==depot->Player) ) ) {
		    free(points);
		    DebugLevel3("Found a resource deposit at %d,%d\n" _C_ x _C_ y);
		    return depot;
		}
		if( CanMoveToMask(x,y,mask) ) {	// reachable
		    *m=1;
		    points[wp].X=x;		// push the point
		    points[wp].Y=y;
		    if( ++wp>=size ) {		// round about
			wp=0;
		    }
		    if (wp==ep) {
			//  We are out of points, give up!
			DebugLevel0Fn("Ran out of points the hard way, beware.\n");
			break;
		    }
		} else {			// unreachable
		    *m=99;
		}
	    }
	    if( ++rp>=size ) {			// round about
		rp=0;
	    }
	}
	cdist++;
	if( rp==wp||cdist>=range ) {			// unreachable, no more points available
	    break;
	}
	//	Continue with next set.
	ep=wp;
    }
    DebugLevel3("No resource deposit found, after we searched %d nodes.\n" _C_ nodes_searched);
    free(points);
    return NoUnitP;
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
	if( unit->Type->Coward && !unit->Removed ) {
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
    unit->TTL=0;

    type=unit->Type;

    //	removed units,  just remove.
    if( unit->Removed ) {
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
	MakeMissile(type->Explosion.Missile
		,unit->X*TileSizeX+type->TileWidth*TileSizeX/2
		,unit->Y*TileSizeY+type->TileHeight*TileSizeY/2
		,0,0);
    }

    //
    //	Building,...
    //
    if( type->Building ) {
	//
	//	Building with units inside?
	//
	//
	//	During resource build, the worker holds the resource amount,
	//	but if canceling building the platform, the worker is already
	//	outside.
	if( type->GivesResource
		&& unit->Orders[0].Action==UnitActionBuilded
		&& unit->Data.Builded.Worker ) {
	    // Restore value for oil-patch
	    unit->Value=unit->Data.Builded.Worker->Value;
	}
	DestroyAllInside(unit);

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
	    unit->Visible = 0xffff;
	    DeadBuildingCacheInsert(unit);	//Insert into corpse list
	    // FIXME: (mr-russ) Hack to make sure we see our own building destroyed
	    MapMarkUnitSight(unit);
	    UnitMarkSeen(unit);
	    MapUnmarkUnitSight(unit);
	    UnitMarkSeen(unit);
	    return;
	}

	// no corpse available
	// FIXME: (mr-russ) Hack to make sure we see our own building destroyed
	MapMarkUnitSight(unit);
	UnitMarkSeen(unit);
	MapUnmarkUnitSight(unit);
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
    CorpseCacheInsert(unit);	//Insert into corpse list

    if( unit->Type->CorpseType ) {
	unit->CurrentSightRange=unit->Type->CorpseType->Stats->SightRange;
    } else {
	unit->CurrentSightRange=0;
    }
    MapMarkUnitSight(unit);
}

/**
 **	Destroy all units inside unit.
 */
global void DestroyAllInside(Unit* source)
{
    Unit* unit;
    int i;

    // FIXME: n0b0dy: No corpses, is that correct behaviour?
    // No Corpses, we are inside something, and we can't be seen
    // FIXME: mr-russ: support for units inside units/buildings?
    unit=source->UnitInside;
    for( i=source->InsideCount; i; --i,unit=unit->NextContained ) {
	RemoveUnit(unit,NULL);
	UnitLost(unit);
	UnitClearOrders(unit);
	ReleaseUnit(unit);
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
	//  increase scores of the attacker, but not if attacking it's own units.
	//  prevents cheating by killing your own units.
	if( attacker && (target->Player->Enemy&(1<<attacker->Player->Player))) {
	    attacker->Player->Score+=target->Type->Points;
	    if( type->Building ) {
		attacker->Player->TotalRazings++;
	    } else {
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
    target->HP-=damage;
#ifdef USE_HP_FOR_XP
    if( attacker ) {
	attacker->XP+=damage;
    }
#endif

    // FIXME: this is dumb. I made repairers capture. crap.
    // david: capture enemy buildings
    // Only worker types can capture.
    // Still possible to destroy building if not careful (too many attackers)
    if( EnableBuildingCapture && attacker
	    && type->Building && target->HP<=damage*3
	    && IsEnemy(attacker->Player,target)
	    && attacker->Type->RepairRange ) {
	ChangeUnitOwner(target,attacker->Player);
	CommandStopUnit(attacker);	// Attacker shouldn't continue attack!
    }

    if( UnitVisibleOnMap(target) || ReplayRevealMap ) {
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
    if( !type->Coward ) {
	if( type->CanAttack && target->Stats->Speed ) {
	    if( RevealAttacker && CanTarget(target->Type,attacker->Type)) {  // Reveal Unit that is attacking
		goal=attacker;
	    } else {
		goal=AttackUnitsInReactRange(target);
	    }
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
	    _C_ x1 _C_ y1 _C_ x2 _C_ y2 _C_ (dy<dx) ? dx : dy );

    return isqrt(dy*dy+dx*dx);
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
	dx=x2-x1-src->Type->TileWidth+1;
	if( dx<0 ) {
	    dx=0;
	}
    } else {
	dx=x1-x2-dst->Type->TileWidth+1;
	if( dx<0 ) {
	    dx=0;
	}
    }

    if( y1+src->Type->TileHeight<=y2 ) {
	dy=y2-y1-src->Type->TileHeight+1;
    } else {
	dy=y1-y2-dst->Type->TileHeight+1;
	if( dy<0 ) {
	    dy=0;
	}
    }

    return isqrt(dy*dy+dx*dx);
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
    //  Hack for snipers, can only target organic units.
    if( !dest->Organic && source->Sniper) {
	return 0;
    }
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
local void SaveOrder(const Order* order,CLFile* file)
{
    char* ref;

    CLprintf(file,"(");
    switch( order->Action ) {
	case UnitActionNone:
	    CLprintf(file,"action-none");
	    break;

	case UnitActionStill:
	    CLprintf(file,"action-still");
	    break;
	case UnitActionStandGround:
	    CLprintf(file,"action-stand-ground");
	    break;
	case UnitActionFollow:
	    CLprintf(file,"action-follow");
	    break;
	case UnitActionMove:
	    CLprintf(file,"action-move");
	    break;
	case UnitActionAttack:
	    CLprintf(file,"action-attack");
	    break;
	case UnitActionAttackGround:
	    CLprintf(file,"action-attack-ground");
	    break;
	case UnitActionDie:
	    CLprintf(file,"action-die");
	    break;

	case UnitActionSpellCast:
	    CLprintf(file,"action-spell-cast");
	    break;

	case UnitActionTrain:
	    CLprintf(file,"action-train");
	    break;
	case UnitActionUpgradeTo:
	    CLprintf(file,"action-upgrade-to");
	    break;
	case UnitActionResearch:
	    CLprintf(file,"action-research");
	    break;
	case UnitActionBuilded:
	    CLprintf(file,"action-builded");
	    break;

	case UnitActionBoard:
	    CLprintf(file,"action-board");
	    break;
	case UnitActionUnload:
	    CLprintf(file,"action-unload");
	    break;
	case UnitActionPatrol:
	    CLprintf(file,"action-patrol");
	    break;
	case UnitActionBuild:
	    CLprintf(file,"action-build");
	    break;

	case UnitActionRepair:
	    CLprintf(file,"action-repair");
	    break;
	case UnitActionResource:
	    CLprintf(file,"action-resource");
	    break;
	case UnitActionReturnGoods:
	    CLprintf(file,"action-return-goods");
	    break;

	case UnitActionDemolish:
	    CLprintf(file,"action-demolish");
	    break;

	default:
	    DebugLevel0Fn("Unknown action in order\n");
    }
    CLprintf(file," flags %d",order->Flags);
    CLprintf(file," range (%d %d)",order->RangeX,order->RangeY);
    if( order->Goal ) {
	if (order->Goal->Destroyed) {
	    /* this unit is destroyed so it's not in the global unit
	     * array - this means it won't be saved!!! */
	    printf ("FIXME: storing destroyed Goal - loading will fail.\n");
	}
	CLprintf(file," goal %s",ref=UnitReference(order->Goal));
	free(ref);
    }
    CLprintf(file," tile (%d %d)",order->X,order->Y);
    if( order->Type ) {
	CLprintf(file," type %s",order->Type->Ident);
    }
    if( order->Arg1 ) {
	// patrol=pos, research=upgrade, spell=spell
	switch( order->Action ) {
	    case UnitActionPatrol:
		CLprintf(file," patrol (%d %d)",
			(int)order->Arg1>>16,(int)order->Arg1&0xFFFF);
		break;
	    case UnitActionSpellCast:
		CLprintf(file," spell %s",((SpellType*)order->Arg1)->Ident);
		break;
	    case UnitActionResearch:
		CLprintf(file," upgrade %s",((Upgrade*)order->Arg1)->Ident);
		break;
	    default:
		CLprintf(file," arg1 %d",(int)order->Arg1);
		break;
	}
    }
    CLprintf(file,")");
}

/**
 **	Save the state of an unit to file.
 **
 **	@param unit	Unit pointer to be saved.
 **	@param file	Output file.
 */
global void SaveUnit(const Unit* unit,CLFile* file)
{
    char* ref;
    Unit* uins;
    int i;

    CLprintf(file,"\n(unit %d ",UnitNumber(unit));

    // 'type and 'player must be first, needed to create the unit slot
    CLprintf(file,"'type '%s ",unit->Type->Ident);
    if( unit->SeenType ) {
	CLprintf(file,"'seen-type '%s ",unit->SeenType->Ident);
    }

    CLprintf(file,"'player %d\n  ",unit->Player->Player);

    if( unit->Name ) {
	CLprintf(file,"'name \"%s\" ",unit->Name);
    }

    if( unit->Next ) {
	CLprintf(file,"'next '%d ",UnitNumber(unit->Next));
    }

    CLprintf(file,"'tile '(%d %d) ",unit->X,unit->Y);
#if 0
    /* latimerius: why is this so complex? */
    // JOHNS: An unit can be owned by a new player and have still the old stats
    for( i=0; i<PlayerMax; ++i ) {
	if( &unit->Type->Stats[i]==unit->Stats ) {
	    CLprintf(file,"'stats %d\n  ",i);
	    break;
	}
    }
    /* latimerius: what's the point of storing a pointer value anyway? */
    if( i==PlayerMax ) {
	CLprintf(file,"'stats 'S%08X\n  ",(int)unit->Stats);
    }
#else
    CLprintf(file, "'stats %d\n  " ,unit->Player->Player);
#endif
    CLprintf(file,"'pixel '(%d %d) ",unit->IX,unit->IY);
    CLprintf(file,"'seen-pixel '(%d %d) ",unit->SeenIX,unit->SeenIY);
    CLprintf(file,"'%sframe %d ",
	    unit->Frame<0 ? "flipped-" : "" ,unit->Frame<0?-unit->Frame:unit->Frame);
    if( unit->SeenFrame!=UnitNotSeen ) {
	CLprintf(file,"'%sseen %d ",
		unit->SeenFrame<0 ? "flipped-" : "" ,unit->SeenFrame<0?-unit->SeenFrame:unit->SeenFrame);
    } else {
	CLprintf(file,"'not-seen ");
    }
    CLprintf(file,"'direction %d\n  ",unit->Direction);
    CLprintf(file,"'attacked %d\n ",unit->Attacked);
    CLprintf(file," 'current-sight-range %d",unit->CurrentSightRange);
    if( unit->Burning ) {
	CLprintf(file," 'burning");
    }
    if( unit->Destroyed ) {
	CLprintf(file," 'destroyed");
    }
    if( unit->SeenDestroyed ) {
	CLprintf(file," 'seen-destroyed");
    }
    if( unit->Removed ) {
	CLprintf(file," 'removed");
    }
    if( unit->Selected ) {
	CLprintf(file," 'selected");
    }
    if( unit->RescuedFrom ) {
	CLprintf(file," 'rescued-from %d",unit->RescuedFrom->Player);
    }
    // n0b0dy: How is this usefull?
    // mr-russ: You can't always load units in order, it saved the information
    // so you can load a unit who's Container hasn't been loaded yet.
    // SEE unit loading code.
    if( unit->Container && unit->Removed ) {
	CLprintf(file," 'host-info '(%d %d %d %d) ",
	    unit->Next->X,unit->Next->Y,
	    unit->Next->Type->TileWidth,
	    unit->Next->Type->TileHeight);
    }
    CLprintf(file," 'visible \"");
    for( i=0; i<PlayerMax; ++i ) {
	CLprintf(file,"%c",(unit->Visible&(1<<i)) ? 'X' : '_');
    }
    CLprintf(file,"\"\n ");
    if( unit->Constructed ) {
	CLprintf(file," 'constructed");
    }
    if( unit->SeenConstructed ) {
	CLprintf(file," 'seen-constructed");
    }
    CLprintf(file," 'seen-state %d ",unit->SeenState);
    if( unit->Active ) {
	CLprintf(file," 'active");
    }
    CLprintf(file," 'mana %d",unit->Mana);
    CLprintf(file," 'hp %d",unit->HP);
    CLprintf(file," 'xp %d",unit->XP);
    CLprintf(file," 'kills %d\n  ",unit->Kills);

    CLprintf(file,"'ttl %lu ",unit->TTL);
    CLprintf(file,"'bloodlust %d ",unit->Bloodlust);
    CLprintf(file,"'haste %d ",unit->Haste);
    CLprintf(file,"'slow %d\n  ",unit->Slow);
    CLprintf(file,"'invisible %d ",unit->Invisible);
    CLprintf(file,"'flame-shield %d ",unit->FlameShield);
    CLprintf(file,"'unholy-armor %d\n  ",unit->UnholyArmor);

    CLprintf(file,"'group-id %d\n  ",unit->GroupId);
    CLprintf(file,"'last-group %d\n  ",unit->LastGroup);

    CLprintf(file,"'value %d\n  ",unit->Value);
    if (unit->CurrentResource) {
	CLprintf(file,"'current-resource '%s\n  ",DefaultResourceNames[unit->CurrentResource]);
    }

    CLprintf(file,"'sub-action %d ",unit->SubAction);
    CLprintf(file,"'wait %d ",unit->Wait);
    CLprintf(file,"'state %d",unit->State);
    if( unit->Reset ) {
	CLprintf(file," 'reset");
    }
    CLprintf(file,"\n  'blink %d",unit->Blink);
    if( unit->Moving ) {
	CLprintf(file," 'moving");
    }
    CLprintf(file," 'rs %d",unit->Rs);
    CLprintf(file," 'units-contained-count %d",unit->InsideCount);
    CLprintf(file,"\n  'units-contained #(");
    uins=unit->UnitInside;
    for( i=unit->InsideCount; i; --i,uins=uins->NextContained ) {
	CLprintf(file,"%s",ref=UnitReference(uins));
	if( i>1 ) {
	    CLprintf(file," ");
	}
    }
    CLprintf(file,")\n  ");
    CLprintf(file,"'order-count %d\n  ",unit->OrderCount);
    CLprintf(file,"'order-flush %d\n  ",unit->OrderFlush);
    CLprintf(file,"'orders #(");
    for( i=0; i<MAX_ORDERS; ++i ) {
	CLprintf(file,"\n    ");
	SaveOrder(&unit->Orders[i],file);
    }
    CLprintf(file,")\n  'saved-order '");
    SaveOrder(&unit->SavedOrder,file);
    CLprintf(file,"\n  'new-order '");
    SaveOrder(&unit->NewOrder,file);

    //
    //	Order data part
    //
    switch( unit->Orders[0].Action ) {
	case UnitActionStill:
	    // FIXME: support other resource types
	    if( unit->Type->GivesResource ) {
		CLprintf(file," 'resource-active %d",unit->Data.Resource.Active);
	    }
	    break;
	case UnitActionResource:
	    CLprintf(file," 'data-res-worker '(time-to-harvest %d",unit->Data.ResWorker.TimeToHarvest);
	    if (unit->Data.ResWorker.DoneHarvesting) {
		CLprintf(file," done-harvesting");
	    }
	    CLprintf(file,")");
	    break;
	case UnitActionBuilded:
	    {
		ConstructionFrame* cframe;
		int frame;

		cframe=unit->Type->Construction->Frames;
		frame=0;
		while( cframe!=unit->Data.Builded.Frame ) {
		    cframe=cframe->Next;
		    ++frame;
		}
		CLprintf(file,"\n  'data-builded '(worker %s",
			ref=UnitReference(unit->Data.Builded.Worker));
		free(ref);
		CLprintf(file," progress %d frame %d",
			unit->Data.Builded.Progress,frame);
		if( unit->Data.Builded.Cancel ) {
		    CLprintf(file," cancel");
		}
		CLprintf(file,")");
		break;
	    }
	case UnitActionResearch:
	    CLprintf(file,"\n  'data-research '(");
	    CLprintf(file,"ident %s", unit->Data.Research.Upgrade->Ident);
	    CLprintf(file,")");
	    break;
	case UnitActionUpgradeTo:
	    CLprintf(file,"\n  'data-upgrade-to '(");
	    CLprintf(file,"ticks %d",unit->Data.UpgradeTo.Ticks);
	    CLprintf(file,")");
	    break;
	case UnitActionTrain:
	    CLprintf(file,"\n  'data-train '(");
	    CLprintf(file,"ticks %d ",unit->Data.Train.Ticks);
	    CLprintf(file,"count %d ",unit->Data.Train.Count);
	    CLprintf(file,"queue #(");
	    for (i=0; i<MAX_UNIT_TRAIN; i++) {
		if (i<unit->Data.Train.Count) {
		    CLprintf(file,"%s ",unit->Data.Train.What[i]->Ident);
		} else {
		    /* this slot is currently unused */
		    CLprintf(file,"unit-none ");
		}
	    }
	    CLprintf(file, "))");
	    break;
	default:
	    CLprintf(file,"\n  'data-move '(");
	    if( unit->Data.Move.Fast ) {
		CLprintf(file,"fast ");
	    }
	    if( unit->Data.Move.Length ) {
		CLprintf(file,"path #(");
		for( i=0; i<unit->Data.Move.Length; ++i ) {
		    CLprintf(file,"%d ", unit->Data.Move.Path[i]);
		}
		CLprintf(file,")");
	    }
	    CLprintf(file,")");
	    break;
    }

    if( unit->Goal ) {
	CLprintf(file,"\n  'goal %d",UnitNumber(unit->Goal));
    }
    if( unit->AutoCastSpell ) {
	CLprintf(file,"\n  'auto-cast '%s",unit->AutoCastSpell->Ident);
    }

    CLprintf(file,")\n");
}

/**
 **	Save state of units to file.
 **
 **	@param file	Output file.
 */
global void SaveUnits(CLFile* file)
{
    Unit** table;
    int i;
    unsigned char SlotUsage[MAX_UNIT_SLOTS/8 + 1];
    int InRun, RunStart;

    CLprintf(file,"\n;;; -----------------------------------------\n");
    CLprintf(file,";;; MODULE: units $Id$\n\n");

    //
    //	Local variables
    //
    CLprintf(file,"(set-hitpoint-regeneration! #%s)\n",
	    HitPointRegeneration ? "t" : "f");
    CLprintf(file,"(set-xp-damage! #%s)\n",
	    XpDamage ? "t" : "f");
    CLprintf(file,"(set-fancy-buildings! #%s)\n",
	    FancyBuildings ? "t" : "f");
    CLprintf(file,"(set-training-queue! #%s)\n",
	    EnableTrainingQueue ? "t" : "f");

    CLprintf (file, "; Unit slot usage bitmap\n");
    CLprintf (file, "(slot-usage '(");

    memset (SlotUsage, 0, MAX_UNIT_SLOTS/8 + 1);
    for (i=0; i<NumUnits; i++) {
	int slot = Units[i]->Slot;
	SlotUsage[slot/8] |= 1 << (slot%8);
    }
#if 0
    /* the old way */
    for (i=0; i<MAX_UNIT_SLOTS/8 + 1; i++) {
	CLprintf (file, " %d", SlotUsage[i]);
	if ( (i+1) % 16 == 0 )		// 16 numbers per line
	    CLprintf (file, "\n");
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
		CLprintf (file, "%d ", i-1);
	    } else {
		CLprintf (file, "%d - %d ", RunStart, i-1);
	    }
	}
    }
#endif

    CLprintf (file, "))\n");

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
