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

global Unit* UnitSlots[MAX_UNIT_SLOTS];	/// All possible units
global Unit** UnitSlotFree;		/// First free unit slot
local Unit* ReleasedHead;		/// List of released units.
local Unit** ReleasedTail;		/// List tail of released units.

global Unit* Units[MAX_UNIT_SLOTS];	/// Array of used slots
global int NumUnits;			/// Number of slots used

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Initial memory allocation for Units.
*/
global void InitUnitsMemory(void)
{
    Unit** slot;

    // Initiallize the "list" of free unit slots

    slot=UnitSlots+MAX_UNIT_SLOTS;
    *--slot=NULL;			// leave the last slot free as no marker
    *--slot=NULL;
    do {
	slot[-1]=(void*)slot;
    } while( --slot>UnitSlots );
    UnitSlotFree=slot;

    ReleasedTail=&ReleasedHead;		// list of unfreed units.
}

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

/**
**	Release an unit.
**
**	The unit is only released, if all references are dropped.
**
**	@param unit	Pointer to unit.
*/
global void ReleaseUnit(Unit* unit)
{
    DebugLevel2Fn("%d:Unit %p %Zd `%s'\n",FrameCounter,
	    unit,UnitNumber(unit),unit->Type->Ident);

    DebugCheck( !unit->Type );		// already free.

    //
    //	First release, remove from lists/tables.
    //
    if( !unit->Destroyed ) {
	Player* player;
	Unit* temp;

	//
	//	Remove the unit from the player's units table.
	//
	if( (player=unit->Player) ) {
	    DebugCheck( *unit->PlayerSlot!=unit );
	    temp=player->Units[--player->TotalNumUnits];
	    player->Units[player->TotalNumUnits]=NULL;
	    temp->PlayerSlot=unit->PlayerSlot;
	    *unit->PlayerSlot=temp;
	}
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
	    DebugLevel2Fn("%d:More references of %Zd #%d\n",FrameCounter
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
	DebugLevel2Fn("%d:No more references %Zd\n",
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

    DebugLevel3Fn("%s(%Zd)\n",type->Name,player-Players);

    //
    //	Can use released unit?
    //
    // FIXME: releasing disabled until references are working correct.
    if( 1 && ReleasedHead && ReleasedHead->Refs<FrameCounter ) {
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

    DebugLevel3Fn("%p %Zd\n",unit,UnitNumber(unit));

    //
    //	Initialise unit structure (must be zero filled!)
    //
    unit->Type=type;

    unit->SeenFrame=-1;
    if( !type->Building ) {
        unit->Direction=(MyRand()>>8)&0xFF;// random heading
        player->NumFoodUnits++;		// food needed
	if( player==ThisPlayer ) {
	    MustRedraw|=RedrawResources;// update food
	}
    } else {
	player->NumBuildings++;
    }
    unit->Player=player;
    unit->Stats=&type->Stats[unit->Player->Player];

    if( type->CanCastSpell ) {
	unit->Mana=MAGIC_FOR_NEW_UNITS;
    }
    unit->HP=type->Stats[player->Player].HitPoints;

    unit->GroupId=-1;

    unit->Wait=1;
    unit->Reset=1;

    unit->Rs=MyRand()%100; // used for random fancy buildings and other things
    // DEFAULT! unit->Revealer = 0;		// FOW revealer

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
    int h;
    int w;

    unit=MakeUnit(type,player);

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

	//
	//	fancy buildings: mirror buildings (but shadows not correct)
	//
	if ( FancyBuildings && unit->Rs > 50 ) {
	    unit->Frame |= 128;
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

#ifdef NEW_FOW
    //
    //	Update fog of war.
    //
    MapMarkSight(unit->Player,x,y,unit->Stats->SightRange);
#else
    //
    //	Update fog of war, if unit belongs to player on this computer
    //
    if( player==ThisPlayer ) {
	MapMarkSight(x,y,unit->Stats->SightRange);
    }
#endif

    UnitCacheInsert(unit);

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

    DebugLevel3Fn("%Zd %p %p\n",UnitNumber(unit),unit,unit->Next);
    UnitCacheRemove(unit);
#ifdef UNIT_ON_MAP
    if( 0 ) {
	Unit* list;

	list=TheMap.Fields[unit->Y*TheMap.Width+unit->X].Here.Units;
	while( list ) {				// find the unit
	    if( list==unit ) {
		DebugLevel0Fn("%Zd\n",UnitNumber(unit));
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
global void UnitLost(const Unit* unit)
{
    Unit* temp;
    UnitType* type;
    Player* player;

    DebugCheck( !unit );

    type=unit->Type;
    player=unit->Player;
    if( unit->Type->Building ) {
	// FIXME: This should be complete rewritten
	// FIXME: Slow and new members are available
	// FIXME: most redraws only needed for player==ThisPlayer

	// Still under construction
	// FIXME: could use unit::Constructed?
	if( unit->Orders[0].Action!=UnitActionBuilded ) {
	    if( type==UnitTypeHumanFarm || type==UnitTypeOrcFarm ) {
		player->Food-=4;
		MustRedraw |= RedrawResources;
	    } else if( type==UnitTypeByIdent("unit-town-hall")
		    || type==UnitTypeByIdent("unit-great-hall") ) {
		player->Food--;
		MustRedraw |= RedrawResources;
	    } else if( type==UnitTypeByIdent("unit-elven-lumber-mill")
		    || type==UnitTypeByIdent("unit-troll-lumber-mill") ) {

		if( !(HaveUnitTypeByIdent(player,"unit-elven-lumber-mill")
			+HaveUnitTypeByIdent(player
				,"unit-elven-lumber-mill")) ) {
		    player->Incomes[WoodCost]=DEFAULT_INCOMES[WoodCost]+25;
		    MustRedraw |= RedrawInfoPanel;
		}
	    } else if( type==UnitTypeByIdent("unit-human-refinery")
		    || type==UnitTypeByIdent("unit-orc-refinery") ) {
		if( !(HaveUnitTypeByIdent(player,"unit-human-refinery")
			+HaveUnitTypeByIdent(player,"unit-orc-refinery")) ) {
		    player->Incomes[OilCost]=DEFAULT_INCOMES[OilCost]+25;
		    MustRedraw |= RedrawInfoPanel;
		}
	    } else if( type==UnitTypeByIdent("unit-keep")
		    || type==UnitTypeByIdent("unit-stronghold")
		    || type==UnitTypeByIdent("unit-castle")
		    || type==UnitTypeByIdent("unit-fortress") ) {
		player->Food--;
		MustRedraw |= RedrawResources;
		if( !(HaveUnitTypeByIdent(player,"unit-castle")
			+HaveUnitTypeByIdent(player,"unit-fortress")) ) {
		    player->Incomes[GoldCost]=DEFAULT_INCOMES[GoldCost]+10;
		    if( !(HaveUnitTypeByIdent(player,"unit-keep")
			    +HaveUnitTypeByIdent(player,"unit-stronghold")) ) {
			player->Incomes[GoldCost]=DEFAULT_INCOMES[GoldCost];
		    }
		    MustRedraw |= RedrawInfoPanel;
		}
	    }
	}
	player->NumBuildings--;
    } else {
	player->NumFoodUnits--;
	if( player==ThisPlayer ) {
	    MustRedraw|=RedrawResources;	// update food
	}
    }

    DebugLevel3Fn("Lost %s(%Zd)\n",unit->Type->Ident,UnitNumber(unit));

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

    player->UnitTypesCount[type->Type]--;

    DebugCheck( player->NumFoodUnits < 0 );
    DebugCheck( player->NumBuildings < 0 );
    DebugCheck( player->UnitTypesCount[type->Type] < 0 );
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

    DebugLevel3Fn("unit %Zd (%d)\n"
	    ,UnitNumber(unit),unit->Type->Type);

    player=unit->Player;
    type=unit->Type;
    //
    //	Update food and resources
    //
    if( unit->Type->Building ) {
	// FIXME: this is slow, remove the UnitTypeByIdent.
	// FIXME: any ideas to generalize this problems?
	if( type==UnitTypeByIdent("unit-farm")
		|| type==UnitTypeByIdent("unit-pig-farm") ) {
	    player->Food+=4;
            MustRedraw |= RedrawResources;
	} else if( type==UnitTypeByIdent("unit-town-hall")
		|| type==UnitTypeByIdent("unit-great-hall") ) {
	    player->Food++;
	    MustRedraw |= RedrawResources;
	} else if( type==UnitTypeByIdent("unit-elven-lumber-mill")
		|| type==UnitTypeByIdent("unit-troll-lumber-mill") ) {
	    player->Incomes[WoodCost]=DEFAULT_INCOMES[WoodCost]+25;
	    MustRedraw |= RedrawInfoPanel;
	} else if( type==UnitTypeByIdent("unit-human-refinery")
		|| type==UnitTypeByIdent("unit-orc-refinery") ) {
	    player->Incomes[OilCost]=DEFAULT_INCOMES[OilCost]+25;
	    MustRedraw |= RedrawInfoPanel;
	} else if( type==UnitTypeByIdent("unit-keep")
		|| type==UnitTypeByIdent("unit-stronghold") ) {
	    if( !upgrade ) {
		player->Food++;
		MustRedraw |= RedrawResources;
	    }
	    if( player->Incomes[GoldCost]==DEFAULT_INCOMES[GoldCost] ) {
		player->Incomes[GoldCost]=DEFAULT_INCOMES[GoldCost]+10;
		MustRedraw |= RedrawInfoPanel;
	    }
	} else if( type==UnitTypeByIdent("unit-castle")
		|| type==UnitTypeByIdent("unit-fortress") ) {
	    if( !upgrade ) {
		player->Food++;
		MustRedraw |= RedrawResources;
	    }
	    if( player->Incomes[GoldCost]!=DEFAULT_INCOMES[GoldCost]+20 ) {
		player->Incomes[GoldCost]=DEFAULT_INCOMES[GoldCost]+20;
		MustRedraw |= RedrawInfoPanel;
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
**	Return true if unit is visible on screen.
**
**	@param unit	Unit to be checked.
**	@return		True if visible, false otherwise.
*/
global int UnitVisible(const Unit* unit)
{
#ifdef NEW_FOW
    unsigned x;
    unsigned y;
    unsigned w;
    unsigned h;
    unsigned i;
    unsigned m;
    MapField* mf;

    if ( unit->Invisible && unit->Player != ThisPlayer )
      {
      //FIXME: vladi: should handle teams and shared vision
      return 0;
      }

    //
    //	Check if visible on screen
    //
    x = unit->X;
    y = unit->Y;
    w = unit->Type->TileWidth;
    h = unit->Type->TileHeight;
    if( x+w > MapX && x < MapX+MapWidth &&
	    y+h > MapY && y < MapY+MapHeight ) {
	//
	//	Check explored and if visible under fog of war.
	//	Building could always be seen under fog of war.
	//	FIXME: only known buildings are visible SceenFrame!=-1.
	//
	mf=TheMap.Fields+y*TheMap.Width+x;
	m=1<<ThisPlayer->Player;
	while( h-- ) {
	    for( i=w; i--; ) {
		if( (mf->Explored&m) && (unit->Type->Building
			    || (mf->Visible&m)) ) {
		    return 1;
		}
		mf++;
	    }
	    mf+=TheMap.Width-w;
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

    if ( unit->Invisible && unit->Player != ThisPlayer ) {
	//FIXME: vladi: should handle teams and shared vision
	return 0;
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
#endif
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
global int CheckUnitToBeDrawn(const Unit* unit)
{
  #ifdef NEW_MAPDRAW
    int sx,sy,ex,ey;

    // in debug-mode check unsupported displacement exceeding an entire Tile
    // FIXME: displacement could always be made positive and smaller than Tile
    #if NEW_MAPDRAW > 1
      if ( unit->IX <= -TileSizeX || unit->IX >= TileSizeX ||
           unit->IY <= -TileSizeY || unit->IY >= TileSizeY )
        printf( "internal error in CheckUnitToBeDrawn\n" );
    #endif

    GetUnitMapArea( unit, &sx, &sy, &ex, &ey );

    // FIXME: extra tiles added here for attached statusbar/mana/shadow/..
      sx--;sy--;ex++;ey++;

    if ( MarkDrawAreaMap( sx, sy, ex, ey ) ) {
    //  MustRedraw|=RedrawMinimap;
      return 1;
    }
  #else
    if( UnitVisible( unit ) ) {
      MustRedraw|=RedrawMap;
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
	// decrease spells effects time
	//
	if ( unit->Bloodlust > 0 )
	  unit->Bloodlust--;
	if ( unit->Haste > 0 )
	  unit->Haste--;
	if ( unit->Slow > 0 )
	  unit->Slow--;
	if ( unit->Invisible > 0 )
	  unit->Invisible--;
	if ( unit->UnholyArmor > 0 )
	  unit->UnholyArmor--;
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
*/
global void ChangeUnitOwner(Unit* unit,Player* oldplayer,Player* newplayer)
{
    Unit* temp;
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

    //	Remove from old player table

    temp=oldplayer->Units[--oldplayer->TotalNumUnits];
    oldplayer->Units[oldplayer->TotalNumUnits]=NULL;
    temp->PlayerSlot=unit->PlayerSlot;
    *unit->PlayerSlot=temp;

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
        newplayer->NumFoodUnits++;	// food needed
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
    Unit* table[MAX_UNITS];
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
    Unit* table[MAX_UNITS];
    Unit* near[MAX_UNITS];
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
		DebugLevel3("Checking %Zd\n",UnitNumber(unit));
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
    unit->Frame*=5;
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

    x=unit->X;
    y=unit->Y;
    DebugLevel3("\tAdd: %d %d,%d\n",heading,addx,addy);
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
    UnitCacheInsert(unit);
    // FIXME: This only works with 1x1 big units
    TheMap.Fields[x+y*TheMap.Width].Flags|=UnitFieldFlags(unit);

    //unit->Orders[0].Action=UnitActionStill;
    //DebugCheck( unit->SubAction );

    if( unit->Wait!=1 ) {
	unit->Wait=1;
	DebugLevel2Fn("Check this\n");
    }
    unit->Removed=0;

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

    DebugLevel3Fn("%Zd\n",UnitNumber(unit));

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

	    // FIXME: This only works with 1x1 big units
	    DebugCheck( unit->Type->TileWidth!=1 || unit->Type->TileHeight!=1 );
	    TheMap.Fields[bestx+besty*TheMap.Width].Flags|=UnitFieldFlags(unit);

	    unit->Removed=0;
	    UnitCacheInsert(unit);

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
	if( !unit->Removed ) {		// unusable unit
	    continue;
	}
	if( unit->X==source->X && unit->Y==source->Y ) {
	    DropOutOnSide(unit,LookingW
		    ,source->Type->TileWidth,source->Type->TileHeight);
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
    Unit* table[MAX_UNITS];
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
global Unit* FindGoldMine(const Unit* source,int x,int y)
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
    DebugLevel3Fn("%Zd %d,%d\n",UnitNumber(best),best->X,best->Y);
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
    DebugLevel3Fn("%Zd %d,%d\n",UnitNumber(best),best->X,best->Y);
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

    DebugLevel3Fn("%Zd %d,%d\n",UnitNumber(best),best->X,best->Y);
    return best;
}

/**
**	Find wood in sight range.
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

    DebugLevel3Fn("%Zd %d,%d\n",UnitNumber(unit),unit->X,unit->Y);

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

    DebugLevel3Fn("%Zd %d,%d\n",UnitNumber(best),best->X,best->Y);
    return best;
}

/**
**	Find oil deposit.
**
**	@param player	A deposit of this player
**	@param x	Nearest to X position.
**	@param y	Nearest to Y position.
**
**	@return		NoUnitP or oil deposit unit
*/
global Unit* FindOilDeposit(const Player* player,int x,int y)
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
	// Want oil-deposit
	if( unit->Type->StoresOil ) {
	    d=MapDistanceToUnit(x,y,unit);
	    if( d<best_d ) {
		best_d=d;
		best=unit;
	    }
	}
    }

    DebugLevel3Fn("%Zd %d,%d\n",UnitNumber(best),best->X,best->Y);
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

/*----------------------------------------------------------------------------
--	Unit AI
----------------------------------------------------------------------------*/

/**
**	Destroy an unit.
**
**	@param unit	Unit to be destroyed.
*/
global void DestroyUnit(Unit* unit)
{
    UnitType* type;
    int i;

    unit->HP=0;
    unit->Moving=0;

    MustRedraw|=RedrawResources; // for food usage indicator

    //
    //	Release all references
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
	// FIXME: make it configurable? remove ident lookup
	MakeMissile(MissileTypeByIdent("missile-explosion")
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

	if( (type=type->CorpseType) ) {
	    unit->State=unit->Type->CorpseScript;
	    unit->Type=type;

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
	        // DestroyUnit(unit);
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
	DestroyUnit(source->Data.Builded.Worker);
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
	    DestroyUnit(unit);
	}
    }
}

/**
**	Unit is hit by missile.
**
**	@param unit	Unit that is hit.
**	@param damage	How many damage to take.
*/
global void HitUnit(Unit* unit,int damage)
{
    UnitType* type;
    Unit* goal;

    DebugCheck( damage==0 || unit->HP==0 || unit->Type->Vanishes );

    if ( unit->UnholyArmor > 0 ) {
	// vladi: units with active UnholyArmour are invulnerable
	return;
    }

    type=unit->Type;
    if( !unit->Attacked ) {
	if( unit->Player==ThisPlayer ) {
	    static int LastFrame;

	    //
	    //	One help cry each 2 second is enough
	    //	FIXME: Should this be moved into the sound part???
	    //
	    if( LastFrame<FrameCounter ) {
		LastFrame=FrameCounter+FRAMES_PER_SECOND*2;
		PlayUnitSound(unit,VoiceHelpMe);
	    }
	}
	if( unit->Player->AiEnabled ) {
	    AiHelpMe(unit);
	}
    }
    unit->Attacked=7;

    if( unit->HP<=damage ) {	// unit is killed or destroyed
	DestroyUnit(unit);
	return;
    }
    unit->HP-=damage;		// UNSIGNED!

#if 0
    if( type->Organic ) {
	MakeMissile(MissileBlood
		,unit->X*TileSizeX+TileSizeX/2
		,unit->Y*TileSizeY+TileSizeY/2,0,0);
    }
    if( type->Building ) {
	MakeMissile(MissileSmallFire
		,unit->X*TileSizeX
			+(type->TileWidth*TileSizeX)/2
		,unit->Y*TileSizeY
			+(type->TileHeight*TileSizeY)/2
		,0,0);
    }
#endif
    if( type->Building && !unit->Burning ) {
	int f;
	Missile* missile;

	f=(100*unit->HP)/unit->Stats->HitPoints;
	if( f>75) {
	    ; // No fire for this
	} else if( f>50 ) {
	    missile=MakeMissile(MissileTypeSmallFire
		    ,unit->X*TileSizeX
			    +(type->TileWidth*TileSizeX)/2
		    ,unit->Y*TileSizeY
			    +(type->TileHeight*TileSizeY)/2
			    -TileSizeY
		    ,0,0);
	    missile->SourceUnit=unit;
	    unit->Burning=1;
	    ++unit->Refs;
	} else {
	    missile=MakeMissile(MissileTypeBigFire
		    ,unit->X*TileSizeX
			    +(type->TileWidth*TileSizeX)/2
		    ,unit->Y*TileSizeY
			    +(type->TileHeight*TileSizeY)/2
			    -TileSizeY
		    ,0,0);
	    missile->SourceUnit=unit;
	    unit->Burning=1;
	    ++unit->Refs;
	}
    }

    //
    //	FIXME: call others for help.
    //

    //
    //	Unit is working?
    //
    if( unit->Orders[0].Action!=UnitActionStill ) {
	return;
    }

    //
    //	Attack units in range (which or the attacker?)
    //
    if( !type->CowerWorker && !type->CowerMage ) {
	if( type->CanAttack && !type->Tower ) {
	    goal=AttackUnitsInReactRange(unit);
	    if( goal ) {
		// FIXME: should rewrite command handling
		CommandAttack(unit,unit->X,unit->Y,NULL,FlushCommands);
		unit->SavedOrder=unit->Orders[1];
		CommandAttack(unit,goal->X,goal->Y,NoUnitP,FlushCommands);
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
global int ViewPointDistance(int x,int y) {
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
global int ViewPointDistanceToUnit(Unit* dest) {
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
    sprintf(ref,"U%08ZX",UnitNumber(unit));
    return ref;
}

/**
**	Save the state of an unit to file.
*/
global void SaveUnit(const Unit* unit,FILE* file)
{
    DebugLevel0Fn("FIXME: not written\n");
}

/**
**	Save state of units to file.
**
**	@param file	Output file.
*/
global void SaveUnits(FILE* file)
{
    Unit** unit;

    fprintf(file,"\n;;; -----------------------------------------\n");
    fprintf(file,";;; MODULE: units $Id$\n");
    for( unit=Units; unit<&Units[NumUnits]; ++unit ) {
	SaveUnit(*unit,file);
    }
}

//@}
