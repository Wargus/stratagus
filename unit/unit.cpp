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
/*
**	(c) Copyright 1998-2000 by Lutz Sammer
**
**	$Id$
*/

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
#include "sound_id.h"
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

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

#ifdef NEW_UNIT
global Unit* UnitSlots[MAX_UNIT_SLOTS];	/// All possible units
global Unit** UnitSlotFree; 		/// First free unit slot
local Unit* ReleasedHead;		/// List of released units.
local Unit** ReleasedTail;		/// List tail of released units.

global Unit* Units[MAX_UNIT_SLOTS];	/// Array of used slots
global int NumUnits;			/// Number of slots used

#else
// The pool is currently hardcoded to MAX_UNITS
global Unit* UnitsPool;			/// All units in play

global int NumUnits;			/// Number of slots used
global Unit** Units;			/// Array of used slots

local Unit** FreeUnits;			/// Array of free slots
local int NumFreeUnits;			/// Number of free slots
#endif

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Initial memory allocation for Units.
*/
global void InitUnitsMemory(void)
{
#ifdef NEW_UNIT
    Unit** slot;

    // Initiallize the "list" of free unit slots

    slot=UnitSlots+MAX_UNIT_SLOTS;
    *--slot=NULL;			// leave the last slot free as no marker
    *--slot=NULL;
    do {
	slot[-1]=(void*)slot;
    } while( --slot>UnitSlots );
    UnitSlotFree=slot;

    ReleasedTail=&ReleasedHead;
#else
    int i;

    UnitsPool=(Unit*)calloc(MAX_UNITS,sizeof(Unit));
    if( !UnitsPool ) {
	fprintf(stderr,__FUNCTION__": Out of memory\n");
	exit(-1);
    }

    Units=(Unit**)calloc(MAX_UNITS,sizeof(Unit*));
    if( !Units ) {
	fprintf(stderr,__FUNCTION__": Out of memory\n");
	exit(-1);
    }

    NumUnits=0;

    // Initiallize the "list" of free unit slots
    // FIXME: Should build a free list
    FreeUnits=(Unit**)malloc(MAX_UNITS*sizeof(Unit*));
    if( !FreeUnits ) {
	fprintf(stderr,__FUNCTION__": Out of memory\n");
	exit(-1);
    }

    NumFreeUnits=i=MAX_UNITS;
    while( i-->0 ) {
	FreeUnits[i]=&UnitsPool[MAX_UNITS-1-i];
    }
#endif
}

/**
**	Free the memory for an unit slot. Update all units tables.
**	Memory is only freed, if all references are dropped.
**
**	@param unit	Pointer to unit.
*/
global void FreeUnitMemory(Unit* unit)
{
#ifdef NEW_UNIT
    Unit** slot;

    //
    //	Remove from slot table
    //
    slot=UnitSlots+unit->Slot;
    DebugCheck( *slot!=unit );

    *slot=(void*)UnitSlotFree;
    free(unit);
#else
    Unit** tmp;
    unsigned tmp_id;
    Player* player;

    player=unit->Player;
    tmp_id=unit->Id;
    memset(unit,0,sizeof(Unit));	// zero is needed for the code
    unit->Id=++tmp_id;			// slot's id for networking code

    // Remove the unit from the player's units list.
    // FIXME: a backpointer is faster

    // looking for the unit slot...
    for( tmp=player->Units; *tmp!=unit; tmp++ ) {
	;
    }
    *tmp=player->Units[--player->TotalNumUnits];
    player->Units[player->TotalNumUnits]=NULL;

    // Update the 'used slots' array
    // FIXME: a backpointer is faster

    // looking for the unit slot...
    for( tmp=Units; *tmp!=unit; tmp++ ) {
	;
    }
    *tmp=Units[--NumUnits];
    Units[NumUnits]=NULL;

    // Update the 'free slots' array
    FreeUnits[NumFreeUnits++]=unit;
#endif
}

/**
**	Release an unit.
**
**	The unit is only released if all references are dropped.
**
**	@param unit	Pointer to unit.
*/
global void ReleaseUnit(Unit* unit)
{
#ifdef NEW_UNIT
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
    }
    //
    //	Are more references remaining?
    //
    if( unit->Refs-->1 ) {
	unit->Destroyed=1;		// mark as destroyed

	DebugLevel0(__FUNCTION__": more references\n");
	return;
    }
    //
    //	No more references remaining, but the network could have an order
    //	on the way.
    //
    *ReleasedTail=unit;
    ReleasedTail=&unit->Next;
    unit->Refs=FrameCounter+NetworkMaxLag;	// could be reuse after this.
    unit->Type=NULL;			// for debugging.
#else
    FreeUnitMemory(unit);
#endif
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
#ifdef NEW_UNIT
    Unit** slot;
#endif

    IfDebug(
	if( type>UnitTypes+sizeof(UnitTypes)/sizeof(*UnitTypes)
		|| type<UnitTypes ) {
	    fprintf(stderr,__FUNCTION__": Illegal type\n");
	    return NoUnitP;
	}
    );

    DebugLevel3(__FUNCTION__": %s(%Zd)\n",type->Name,player-Players);

#ifdef NEW_UNIT
    //
    //	Can use released unit?
    //
    if( ReleasedHead && ReleasedHead->Refs<FrameCounter ) {
	unit=ReleasedHead;
	ReleasedHead=unit->Next;
	memset(unit,0,sizeof(*unit));
	DebugLevel0(__FUNCTION__": release %p\n",unit);
	// FIXME: can release here more slots.
    } else {
	//
	//	Allocate structure
	//
	if( !(slot=UnitSlotFree) ) {	// should not happen!
	    DebugLevel0(__FUNCTION__": Maximum of units reached\n");
	    return NoUnitP;
	}
	UnitSlotFree=(void*)*slot;
	*slot=unit=calloc(1,sizeof(Unit));
	unit->Refs=1;
	unit->Slot=slot-UnitSlots;		// back index
    }

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

    DebugLevel3(__FUNCTION__": %p %Zd\n",unit,UnitNumber(unit));
#else
    if( NumFreeUnits ) {
	unit=FreeUnits[--NumFreeUnits];
	FreeUnits[NumFreeUnits]=NULL;
	Units[NumUnits++]=unit;
    } else {				// NumUnits == MAX_UNITS
	DebugLevel0("Maximum of units reached\n");
	return NoUnitP;
    }

    DebugLevel3(__FUNCTION__": %p %Zd\n",unit,UnitNumber(unit));

    player->Units[player->TotalNumUnits++]=unit;
    player->UnitTypesCount[type->Type]++;
#endif

    //
    //	Initialise unit structure (must be zero filled!)
    //
    unit->Type=type;

    unit->SeenFrame=-1;
    if( !type->Building ) {
#ifdef NEW_HEADING
        unit->Direction=(MyRand()>>8)&0xFF;// random heading
#else
        unit->Heading=(MyRand()>>13)&7;	// random heading
#endif
        player->NumFoodUnits++;		// food needed
	if( player==ThisPlayer ) {
	    MustRedraw|=RedrawResources;// update food
	}
    } else {
	player->NumBuildings++;
    }
    unit->Player=player;
    unit->Stats=&type->Stats[unit->Player->Player];

    if( type->CowerWorker ) {
	unit->WoodToHarvest=CHOP_FOR_WOOD;
    }
    if( type->CanCastSpell ) {
	unit->Mana=MAGIC_FOR_NEW_UNITS;
    }
    unit->HP=type->Stats[player->Player].HitPoints;

    unit->GroupId=-1;

    DebugCheck( NoUnitP );		// Init fails if NoUnitP!=0

    unit->Wait=1;
    unit->Reset=1;

    unit->Rs=MyRand()%100; // used for random fancy buildings and other things

    unit->Command.Action=UnitActionStill;
    unit->PendCommand.Action=UnitActionStill;

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

    //
    //	Update fog of war, if unit belongs to player on this computer
    //
    if( player==ThisPlayer ) {
	MapMarkSight(x,y,unit->Stats->SightRange);
    }

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

	IfDebug(
	    // FIXME: Removing buildings without HP.
	    if( !type->Stats[unit->Player->Player].HitPoints
		    && !type->OilPatch ) {
		DebugLevel0(__FUNCTION__": internal error\n");
	    }
	);

	for( h=type->TileHeight; h--; ) {
	    for( w=type->TileWidth; w--; ) {
		TheMap.Fields[unit->X+w+(unit->Y+h)*TheMap.Width].Flags
			&=~MapFieldBuilding;
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

    UnitCacheRemove(unit);

    MustRedraw|=RedrawMinimap;
    if( UnitVisible(unit) ) {
	MustRedraw|=RedrawMap;
    }
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
	if( unit->Command.Action!=UnitActionBuilded ) {
	    if( type==UnitTypeByIdent("unit-farm")
		    || type==UnitTypeByIdent("unit-pig-farm") ) {
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
	MustRedraw|=RedrawResources;	// update food
    }

    //
    //	Destroy oil-platform, must update oil.
    //
    if( type->GivesOil ) {
	temp=OilPatchOnMap(unit->X,unit->Y);
	if( temp ) {
	    // JOHNS: copy remaining oil value
	    temp->Value=unit->Value;
	    temp->Removed=0;
	} else {
	    DebugLevel0(__FUNCTION__": Update no oil-patch %s\n",type->Ident);
	}
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

    DebugLevel3(__FUNCTION__": unit %Zd (%d)\n"
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

    DebugLevel3(__FUNCTION__": Goal is (%d,%d)\n",*dx,*dy);
}

/**
**	Return true if unit is visible on screen.
**
**	@param unit	Unit to be checked.
**	@return		True if visible, false otherwise.
*/
global int UnitVisible(const Unit* unit)
{
    unsigned x;
    unsigned y;
    unsigned w;
    unsigned h;

    IfDebug(
	if (!unit->Type) {	// FIXME: Can this happen, if yes it is a bug
	    abort();
	    return 0;
	}
    );

    //
    //	Check if visible on screen
    //
    x = unit->X;
    y = unit->Y;
    w = unit->Type->TileWidth  - 1; // x+width is outside building rect
    h = unit->Type->TileHeight - 1;
    if( (x+w) < MapX || x > (MapX+MapWidth) ||
	    (y+h) < MapY || y > (MapY+MapHeight) ) {
	return 0;
    }

    //
    //	Check explored or if visible under fog of war.
    //	FIXME: isn't it enough to see a field of the building?
    //
    if( !(TheMap.Fields[y*TheMap.Width+x].Flags&MapFieldExplored)
	    || (!TheMap.NoFogOfWar
		&& !unit->Type->Building
		&& !(TheMap.Fields[y*TheMap.Width+x].Flags&MapFieldVisible)) ) {
	return 0;
    }
    return 1;
}

/**
**	Increment mana of all magic units. Called each second.
**	Also clears the blink flag.
*/
global void UnitIncrementMana(void)
{
#ifdef NEW_UNIT
    Unit** table;
    Unit* unit;

    for( table=Units; table<Units+NumUnits; table++ ) {
	unit=*table;
	// FIXME: This isn't the correct position or the correct function name
	if( unit->Blink ) {		// clear blink flag
	    --unit->Blink;
	}

	if( unit->Type->CanCastSpell ) {
	    if( unit->Mana!=255 ) {
		unit->Mana++;
		/* Done currently by color cycle!
		// FIXME: if mana is shown on map
		if( UnitVisible(unit) ) {
		    MustRedraw|=RedrawMap;
		}
		*/
		if( unit->Selected ) {
		    MustRedraw|=RedrawInfoPanel;
		}
	    }
	}
    }
#else
    Unit* unit;
    int i;

    for( i=0; i< NumUnits; i++) {
	unit=Units[i];
	if( unit->Type->CanCastSpell ) {
	    if( unit->Mana!=255 ) {
		unit->Mana++;
		/* Done by color cycle!
		if( UnitVisible(unit) ) {
		    MustRedraw|=RedrawMap;
		}
		if( unit->Selected ) {
		    MustRedraw|=RedrawInfoPanel;
		}
		*/
	    }
	}
	// FIXME: This isn't the correct position or the correct function name
	if( unit->Blink ) {		// clear blink flag
	    --unit->Blink;
	}
    }
#endif
}

/**
**	Increment health of all regenerating units. Called each second.
*/
global void UnitIncrementHealth(void)
{
#ifdef NEW_UNIT
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
	}
    }
#else
    Unit* unit;
    int i;
    static UnitType* berserker;
    static int regeneration;

    if( !berserker ) {
	berserker=UnitTypeByIdent("unit-berserker");
	regeneration=UpgradeIdByIdent("upgrade-berserker-regeneration");
    }

    for( i=0; i< NumUnits; i++) {
	unit=Units[i];
	if( unit->Type==berserker
		&& unit->HP<unit->Stats->HP
		&& UpgradeIdAllowed(unit->Player,regeneration)=='R' ) {
	    ++unit->HP;			// FIXME: how fast do we regenerate
	}
    }
#endif
}

/**
**	Change the unit's owner
**
**	@param oldplayer	Old owning player.
**	@param newplayer	New owning player.
*/
global void ChangeUnitOwner(Unit* unit,Player* oldplayer,Player* newplayer)
{
#ifdef NEW_UNIT
    Unit* temp;
    int i;
#else
    Unit** tmp;
    int i;
#endif

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

#ifdef NEW_UNIT
    //	Remove from old player table

    temp=oldplayer->Units[--oldplayer->TotalNumUnits];
    oldplayer->Units[oldplayer->TotalNumUnits]=NULL;
    temp->PlayerSlot=unit->PlayerSlot;
    *unit->PlayerSlot=temp;

    //	Insert into new player table.

    unit->PlayerSlot=newplayer->Units+newplayer->TotalNumUnits++;
    *unit->PlayerSlot=unit;
#else
    newplayer->Units[newplayer->TotalNumUnits++]=unit;

    // looking for the unit slot...
    for( tmp=oldplayer->Units; *tmp!=unit; tmp++ ) {
	;
    }
    *tmp=oldplayer->Units[--oldplayer->TotalNumUnits];
    oldplayer->Units[oldplayer->TotalNumUnits]=NULL;
#endif

    unit->Player=newplayer;

    //
    //	Must change food/gold and other.
    //
    if( unit->Type->GivesOil ) {
	DebugLevel0(__FUNCTION__":FIXME: oil platform transfer unsupported\n");
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

    if( norescue ) {
	return;
    }
    norescue=1;
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
		// NOTE: I hope SelectUnits checks bounds?
		n=SelectUnits(
			unit->X-1,unit->Y-1,
			unit->X+unit->Type->TileWidth+1,
			unit->Y+unit->Type->TileHeight+1,near);
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
*	Fast arc tangent function.
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
#ifdef NEW_HEADING
    // FIXME: depends on the possible unit directions wc 8, sc 32
    unit->Frame&=127;
    unit->Frame/=5;
    unit->Frame*=5;
    if( unit->Direction<=LookingS ) {	// north->east->south
	DebugLevel3(__FUNCTION__": %Zd>%d\n",UnitNumber(unit),unit->Direction);
	unit->Frame+=unit->Direction/32;
    } else {
	DebugLevel3(__FUNCTION__": %Zd<%d\n",UnitNumber(unit),unit->Direction);
	unit->Frame+=128+256/32-unit->Direction/32;
    }
#else
    switch( unit->Heading ) {
	case HeadingN:
	case HeadingNE:
	case HeadingE:
	case HeadingSE:
	case HeadingS:
	    DebugLevel3(__FUNCTION__": %Zd>%d\n"
		    ,UnitNumber(unit),unit->Heading);
	    unit->Frame=unit->Heading;
	    break;
	case HeadingSW:			// this are mirros
	case HeadingW:
	case HeadingNW:
	    DebugLevel3(__FUNCTION__": %Zd<%d\n"
		    ,UnitNumber(unit),unit->Heading);
	    unit->Frame=128+1+HeadingNW-unit->Heading;
	    break;
    }
#endif
}

#if 0
/**
**	Heading from x1,y1 to x2,y2.
*/
global int HeadingFromXY2XY(int x,int y,int dx,int dy)
{
#ifdef NEW_HEADING
    return DirectionToHeading(dx-x,dy-y);
#else
    DebugLevel3("Heading %d,%d -> %d,%d\n",x,y,dx,dy);
    x=dx-x;
    y=dy-y;
    if( x==0 ) {
	if( y<0 ) {
	    return HeadingN;
	} else {
	    return HeadingS;
	}
    } else if( x>0 ) {
	if( y<0 ) {
	    return HeadingNE;
	} else if( y==0 ) {
	    return HeadingE;
	} else {
	    return HeadingSE;
	}
    } else {
	if( y<0 ) {
	    return HeadingNW;
	} else if( y==0 ) {
	    return HeadingW;
	} else {
	    return HeadingSW;
	}
    }
#endif
}
#endif

/**
**	Change unit heading/frame from delta direction x,y.
*
**	@param unit	Unit for new direction looking.
**	@param dx	X map tile delta direction.
**	@param dy	Y map tile delta direction.
*/
global void UnitHeadingFromDeltaXY(Unit* unit,int dx,int dy)
{
#ifdef NEW_HEADING
    unit->Direction=DirectionToHeading(dx,dy);
    UnitUpdateHeading(unit);
#else
    int heading;
    int frame;

    // Set new heading:
    if( dx==0 ) {
	if( dy<0 ) {
	    heading=HeadingN;
	} else {
	    heading=HeadingS;
	}
	frame=heading;
    } else if( dx>0 ) {
	if( dy<0 ) {
	    heading=HeadingNE;
	} else if( dy==0 ) {
	    heading=HeadingE;
	} else {
	    heading=HeadingSE;
	}
	frame=heading;
    } else {
	if( dy<0 ) {
	    heading=HeadingNW;
	} else if( dy==0 ) {
	    heading=HeadingW;
	} else {
	    heading=HeadingSW;
	}
	frame=128+1+HeadingNW-heading;
    }
    unit->Heading=heading;
    unit->Frame=frame;
#endif
}

/*----------------------------------------------------------------------------
--	Drop out units
----------------------------------------------------------------------------*/

/**
**	Reapear unit on map.
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

#ifdef NEW_HEADING
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
#else
    switch( heading ) {
	case HeadingN:
	case HeadingNE:
	    x+=addx-1;
	    --y;
	    goto startn;
	case HeadingNW:
	case HeadingW:
	    --x;
	    goto startw;
	case HeadingSW:
	case HeadingS:
	    y+=addy;
	    goto starts;
	case HeadingSE:
	case HeadingE:
	    x+=addx;
	    y+=addy-1;
	    goto starte;
    }
#endif

    // FIXME: don't search outside of the map
    for( ;; ) {
startw:
	for( i=addy; i--; y++ ) {
	    if( CheckedCanMoveToMask(x,y,mask) ) {
		goto found;
	    }
	}
	++addx;
starts:
	for( i=addx; i--; x++ ) {
	    if( CheckedCanMoveToMask(x,y,mask) ) {
		goto found;
	    }
	}
	++addy;
starte:
	for( i=addy; i--; y-- ) {
	    if( CheckedCanMoveToMask(x,y,mask) ) {
		goto found;
	    }
	}
	++addx;
startn:
	for( i=addx; i--; x-- ) {
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

    unit->Command.Action=UnitActionStill;
    if( unit->Wait!=1 ) {
	unit->Wait=1;
	DebugLevel3("Check this\n");
    }
    unit->Removed=0;

    //	Update fog of war, if unit belongs to player on this computer
    if( unit->Player==ThisPlayer ) {
	MapMarkSight(x,y,unit->Stats->SightRange);
    }

    MustRedraw|=RedrawMinimap;
    if( UnitVisible(unit) ) {
	MustRedraw|=RedrawMaps;
    }
}

/**
**	Reapear unit on map nearest to x,y.
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

    DebugLevel3(__FUNCTION__": %d\n",unit);

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
	    TheMap.Fields[bestx+besty*TheMap.Width].Flags|=UnitFieldFlags(unit);

	    unit->Command.Action=UnitActionStill;
	    if( unit->Wait!=1 ) {
		unit->Wait=1;
		DebugLevel3("Check this\n");
	    }
	    unit->Removed=0;
	    UnitCacheInsert(unit);

	    if( unit->Player==ThisPlayer ) {
		MapMarkSight(bestx,besty,unit->Stats->SightRange);
	    }

	    MustRedraw|=RedrawMinimap;
	    if( UnitVisible(unit) ) {
		MustRedraw|=RedrawMaps;
	    }
	    return;
	}
	++addy;
    }
}

/**
**	Drop out all units inside unit.
*/
global void DropOutAll(const Unit* source)
{
#ifdef NEW_UNIT
    // FIXME: Rewrite this use source->Next;
    Unit** table;
    Unit* unit;

    for( table=Units; table<Units+NumUnits; table++ ) {
	unit=*table;
	if( !unit->Removed ) {		// unusable unit
	    continue;
	}
	if( unit->X==source->X && unit->Y==source->Y ) {
#ifdef NEW_HEADING
	    DropOutOnSide(unit,LookingW
		    ,source->Type->TileWidth,source->Type->TileHeight);
#else
	    DropOutOnSide(unit,HeadingW
		    ,source->Type->TileWidth,source->Type->TileHeight);
#endif
	}
    }
#else
    Unit* unit;
    int i;

    for( i=0; i<NumUnits; i++ ) {
	unit=Units[i];
	if( !unit->Removed ) {		// unusable unit
	    continue;
	}
	if( unit->X==source->X
		&& unit->Y==source->Y ) {
	    DropOutOnSide(unit,HeadingW
		    ,source->Type->TileWidth
		    ,source->Type->TileHeight);
	}
    }
#endif
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
global int CanBuildHere(UnitType* type,unsigned x,unsigned y)
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
#ifdef NEW_UNIT
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
#else
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
#endif
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
#ifdef NEW_UNIT
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
#else
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
#endif
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
global int CanBuildUnitType(Unit* unit,UnitType* type,int x,int y)
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
	    DebugLevel1(__FUNCTION__": Were moves this unit?\n");
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
**	@returns	Pointer to the nearest gold mine.
*/
global Unit* FindGoldMine(const Unit* source,int x,int y)
{
#ifdef NEW_UNIT
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
	if( d<best_d && UnitReachable(source,unit) ) {
	    best_d=d;
	    best=unit;
	}
    }
    DebugLevel3(__FUNCTION__": %Zd %d,%d\n",UnitNumber(best),best->X,best->Y);
    return best;
#else
    Unit* unit;
    Unit* best;
    int best_d;
    int d,i;

    //	FIXME:	this is not the best one
    //		We need the deposit with the shortest way!
    //		At least it must be reachable!
    //

    best=NoUnitP;
    best_d=99999;
    for( i=0; i<NumUnits; i++ ) {
	unit=Units[i];
	if( UnitUnusable(unit) )
	    continue;
	// Want gold-mine
	if( unit->Type->GoldMine ) {
	    d=MapDistanceToUnit(x,y,unit);
	    if( d<best_d ) {
		best_d=d;
		best=unit;
	    }
	}
    }
    DebugLevel3(__FUNCTION__": %Zd %d,%d\n",UnitNumber(best),best->X,best->Y);
    return best;
#endif
}

/**
**	Find gold deposit.
**
**	@param unit	Pointer for source unit.
**	@param player	A deposit owning this player
**	@param x	X tile position to start.
**	@param y	Y tile position to start.
**
**	@returns	Pointer to the nearest gold mine.
*/
global Unit* FindGoldDeposit(const Unit* source,int x,int y)
{
#ifdef NEW_UNIT
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
	if( d<best_d && UnitReachable(source,unit) ) {
	    best_d=d;
	    best=unit;
	}
    }
    DebugLevel3(__FUNCTION__": %Zd %d,%d\n",UnitNumber(best),best->X,best->Y);
    return best;
#else
    Unit* unit;
    Unit* best;
    Unit*const * units;
    const Player* player;
    int nunits;
    int best_d;
    int d,i;

    //	FIXME:	this is not the best one
    //		We need the deposit with the shortest way!
    //		At least it must be reachable!

    best=NoUnitP;
    best_d=99999;
    player=source->Player;
    nunits=player->TotalNumUnits;
    units=player->Units;
    for( i=0; i<nunits; i++ ) {
	unit=units[i];
	if( UnitUnusable(unit) ) {
	    continue;
	}
	// Want gold-deposit
	if( unit->Type->StoresGold ) {
	    d=MapDistanceToUnit(x,y,unit);
	    if( d<best_d ) {
		best_d=d;
		best=unit;
	    }
	}
    }

    DebugLevel3(__FUNCTION__": %Zd %d,%d\n",UnitNumber(best),best->X,best->Y);
    return best;
#endif
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

    DebugLevel3(__FUNCTION__": %Zd %d,%d\n",UnitNumber(best),best->X,best->Y);
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

    DebugLevel3(__FUNCTION__": %d %d,%d\n",unit,unit->X,unit->Y);

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
	    DebugLevel3(__FUNCTION__": wood on %d,%d\n",x,y);
	    *px=bestx;
	    *py=besty;
	    return 1;
	}
	++addy;
    }

    DebugLevel3(__FUNCTION__": no wood in sight-range\n");
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

    DebugLevel3(__FUNCTION__": %d %d,%d\n",best-Units,best->X,best->Y);
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

    DebugLevel3(__FUNCTION__": %d %d,%d\n",best-Units,best->X,best->Y);
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
#ifdef NEW_UNIT
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
	if( unit->Removed || unit->Command.Action==UnitActionDie ) {
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
#else
    Unit* unit;
    Unit* nunit;
    Unit* funit;			// first possible unit
    UnitType* type;
    int flag;				// flag take next unit
    unsigned gx;
    unsigned gy;
    int i;

    funit=NULL;
    nunit=NULL;
    flag=0;
    if( !ounit ) {			// no old on this position
	flag=1;
    }
    for( i=0; i<NumUnits; i++ ) {
	unit=Units[i];
	// We don't use UnitUnusable() to be able to select
	// a building under construction.
	if( unit->Removed || unit->Command.Action==UnitActionDie ) {
	    continue;
	}
	type=unit->Type;

	//
	//	Check if mouse over unit.
	//
	gx=unit->X*TileSizeX+unit->IX;
	if( x<=gx+(type->TileWidth*TileSizeX-type->BoxWidth)/2) {
	    continue;
	}
	if( x>gx+(type->TileWidth*TileSizeX+type->BoxWidth)/2 ) {
	    continue;
	}

	gy=unit->Y*TileSizeY+unit->IY;
	if( y<=gy+(type->TileHeight*TileSizeY-type->BoxHeight)/2) {
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
#endif
}

/*----------------------------------------------------------------------------
--	Unit AI
----------------------------------------------------------------------------*/

/**
**	Destroy a unit.
**
**	@param unit	Unit to be destroyed.
*/
global void DestroyUnit(Unit* unit)
{
    UnitType* type;

    unit->HP=0;
    unit->Moving=0;

    type=unit->Type;
    PlayUnitSound(unit,VoiceDying);

    MustRedraw|=RedrawResources; // for food usage indicator

    //
    //	Oil patch or removed units,  just remove.
    //
    if( type->OilPatch || unit->Removed ) {
	RemoveUnit(unit);
	UnitLost(unit);
	ReleaseUnit(unit);
	return;
    }

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
		|| unit->Command.Action==UnitActionBuilded ) {
	    DestroyAllInside(unit);
	}

	RemoveUnit(unit);
	UnitLost(unit);

	// FIXME: buildings should get a die sequence

	if( (type=type->CorpseType) ) {
	    unit->State=unit->Type->CorpseScript;
	    unit->Type=type;

	    unit->IX=(type->Width-type->RleSprite->Width)/2;
	    unit->IY=(type->Height-type->RleSprite->Height)/2;

	    unit->SubAction=0;
	    unit->Removed=0;
	    unit->Frame=0;
	    unit->Command.Action=UnitActionDie;

	    DebugCheck( !unit->Type->Animations
		    || !unit->Type->Animations->Die );
	    if( unit->NextCount ) {
		DebugLevel0(__FUNCTION__": NextCount = %d\n",unit->NextCount);
	    }
	    unit->NextCount=0;
	    UnitShowAnimation(unit,unit->Type->Animations->Die);
	    DebugLevel0(__FUNCTION__": Frame %d\n",unit->Frame);

	    return;
	}

	// no corpse available
	ReleaseUnit(unit);
	return;
    }

    // FIXME: units in transporters should die without corpes...
    if( unit->Type->Transporter ) { // Transporters loose their units
        DestroyAllInside(unit);
    }

    RemoveUnit(unit);
    UnitLost(unit);

    // FIXME: ugly trick unit-peon-with-gold ... has no die sequence.
    if( type==UnitTypeByIdent("unit-peon-with-gold")
	    || type==UnitTypeByIdent("unit-peon-with-wood") ) {
	unit->Type=UnitTypeByIdent("unit-peon");
    } else if( type==UnitTypeByIdent("unit-peasant-with-gold")
	    || type==UnitTypeByIdent("unit-peasant-with-wood") ) {
	unit->Type=UnitTypeByIdent("unit-peasant");
    }

    //
    //	Unit has no death animation.
    //

    // Not good: UnitUpdateHeading(unit);
    unit->SubAction=0;
    unit->Removed=0;
    unit->State=0;
    unit->Reset=0;
    unit->Wait=1;
    unit->Command.Action=UnitActionDie;
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
    if( source->Type->Building
	    && source->Command.Action==UnitActionBuilded
	    && source->Command.Data.Builded.Worker ) {
	DestroyUnit(source->Command.Data.Builded.Worker);
	return;
    }

#ifdef NEW_UNIT
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
#else
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
#endif
}

/**
**	Unit is hit by missile.
*/
global void HitUnit(Unit* unit,int damage)
{
    UnitType* type;
    Unit* goal;

    DebugCheck( damage==0 || unit->HP==0 || unit->Type->Vanishes );

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
	unit->Attacked=1;
    }

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

    if( unit->Command.Action!=UnitActionStill ) {
	return;
    }

    //
    //	Attack units in range (which or the attacker?)
    //
    if( !type->CowerWorker && !type->CowerMage ) {
	if( type->CanAttack && !type->Tower ) {
	    goal=AttackUnitsInReactRange(unit);
	    if( goal ) {
		CommandAttack(unit,goal->X,goal->Y,goal,0);
		// FIXME: whow what a chaos (johns)
		//unit->SubAction|=2; (removed produces error)
		return;
	    }
	}
    }

    //
    //	FIXME: Can't attack run away.
    //
}

/*----------------------------------------------------------------------------
--	Conflicts
----------------------------------------------------------------------------*/

/**
**	Returns the distance between two points.
**
**	@param x1	X map tile position.
**	@param y1	Y map tile position.
**	@param x2	X map tile position.
**	@param y2	Y map tile position.
*/
global int MapDistance(int x1,int y1,int x2,int y2)
{
    return max(abs(x1-x2),abs(y1-y2));
}

/**
**	Returns the distance to unit type.
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
**	Returns the distance to unit.
**
**	@param x	X map tile position.
**	@param y	Y map tile position.
**	@param dest	Distance to this unit.
*/
global int MapDistanceToUnit(int x,int y,const Unit* dest)
{
    IfDebug(
	if( !dest ) {			// FIXME: should be handled by caller
	    DebugLevel0(__FUNCTION__": NULL pointer\n");
	    return InfiniteDistance;
	}
    );
    return MapDistanceToType(x,y,dest->Type,dest->X,dest->Y);
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

#if 0
/**
**	Check if unit is an enemy.
**
**	FIXME: Should use a bit-field for enemies.
*/
global int IsEnemy(const Player* player,const Unit* dest)
{
    Player* dest_player;

    //	Neutral unit
    if( player->Player==PlayerNumNeutral
	    || (dest_player=dest->Player)->Player==PlayerNumNeutral ) {
	return 0;
    }

    //	Same team
    if( player->Team==dest_player->Team ) {
	return 0;
    }
    return 1;
}
#else
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
#endif

/**
**	Can the source unit attack the destionation unit.
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
local char* UnitReference(const Unit* unit)
{
    char* ref;

    ref=malloc(10);
    sprintf(ref,"U%08ZX",UnitNumber(unit));
    return ref;
}

/**
**	Save the current command of an unit.
*/
local void SaveCommand(const Command* command,FILE* file)
{
    char* ref;

    fprintf(file,"\t(");
    switch( command->Action ) {
	case UnitActionNone:
	    fprintf(file,"'none");
	    break;
	case UnitActionStill:
	    fprintf(file,"'still");
	    if( command->Data.Move.Goal ) {
		ref=UnitReference(command->Data.Move.Goal);
		fprintf(file," %s",ref);
		free(ref);
	    }
	    break;
	case UnitActionStandGround:
	    fprintf(file,"'stand-ground");
	    if( command->Data.Move.Goal ) {
		ref=UnitReference(command->Data.Move.Goal);
		fprintf(file," %s",ref);
		free(ref);
	    }
	    break;
	case UnitActionFollow:
	    fprintf(file,"'follow");
	    ref=UnitReference(command->Data.Move.Goal);
	    fprintf(file," %s",ref);
	    fprintf(file," %d",command->Data.Move.Range);
	    free(ref);
	    break;
	case UnitActionMove:
	    fprintf(file,"'move");
	    fprintf(file," (%d %d)"
		,command->Data.Move.SX,command->Data.Move.SY);
	    fprintf(file," (%d %d)"
		,command->Data.Move.DX,command->Data.Move.DY);
	    if( command->Data.Move.Goal ) {
		ref=UnitReference(command->Data.Move.Goal);
		fprintf(file," %s",ref);
		free(ref);
	    }
	    fprintf(file," %d",command->Data.Move.Range);
	    break;
	case UnitActionAttack:
	    fprintf(file,"'attack");
	    fprintf(file," (%d %d)"
		,command->Data.Move.SX,command->Data.Move.SY);
	    fprintf(file," (%d %d)"
		,command->Data.Move.DX,command->Data.Move.DY);
	    if( command->Data.Move.Goal ) {
		ref=UnitReference(command->Data.Move.Goal);
		fprintf(file," %s",ref);
		free(ref);
	    }
	    fprintf(file," %d",command->Data.Move.Range);
	    break;
	case UnitActionDie:
	    fprintf(file,"'die");
	    fprintf(file," \"FIXME:\"");
	    break;
	case UnitActionTrain:
	    fprintf(file,"'train");
	    fprintf(file," \"FIXME:\"");
	    break;
	case UnitActionUpgradeTo:
	    fprintf(file,"'upgrade-to");
	    fprintf(file," \"FIXME:\"");
	    break;
	/*
	case UnitActionUpgrade:
	    fprintf(file,"'upgrade");
	    fprintf(file," \"FIXME:\"");
	    break;
	*/
	case UnitActionResearch:
	    fprintf(file,"'research");
	    fprintf(file," \"FIXME:\"");
	    break;
	case UnitActionBuilded:
	    fprintf(file,"'builded");
	    fprintf(file," \"FIXME:\"");
	    break;
	case UnitActionBoard:
	    fprintf(file,"'board");
	    fprintf(file," \"FIXME:\"");
	    break;
	case UnitActionUnload:
	    fprintf(file,"'unload");
	    fprintf(file," \"FIXME:\"");
	    break;
	case UnitActionPatrol:
	    fprintf(file,"'patrol");
	    fprintf(file," \"FIXME:\"");
	    break;
	case UnitActionBuild:
	    fprintf(file,"'build");
	    fprintf(file," \"FIXME:\"");
	    break;
	case UnitActionRepair:
	    fprintf(file,"'repair");
	    fprintf(file," \"FIXME:\"");
	    break;
	case UnitActionHarvest:
	    fprintf(file,"'harvest");
	    fprintf(file," \"FIXME:\"");
	    break;
	case UnitActionMineGold:
	    fprintf(file,"'mine-gold");
	    fprintf(file," \"FIXME:\"");
	    break;
	case UnitActionMineOre:
	    fprintf(file,"'mine-ore");
	    fprintf(file," \"FIXME:\"");
	    break;
	case UnitActionMineCoal:
	    fprintf(file,"'mine-coal");
	    fprintf(file," \"FIXME:\"");
	    break;
	case UnitActionQuarryStone:
	    fprintf(file,"'break-stone");
	    fprintf(file," \"FIXME:\"");
	    break;
	case UnitActionHaulOil:
	    fprintf(file,"'haul-oil");
	    fprintf(file," \"FIXME:\"");
	    break;
	case UnitActionReturnGoods:
	    fprintf(file,"'return-goods");
	    fprintf(file," \"FIXME:\"");
	    break;
	case UnitActionDemolish:
	    fprintf(file,"'demolish");
	    fprintf(file," \"FIXME:\"");
	    break;
    }
    fprintf(file,")\n");
}

/**
**	Save the state of an unit to file.
*/
global void SaveUnit(const Unit* unit,FILE* file)
{
    char* ref;
    int i;

    ref=UnitReference(unit);
    fprintf(file,"(unit %s ",ref);
    free(ref);

    // FIXME: This part has nobody (=johns) updated!
    fprintf(file,"'%s ;;; \"%s\"\n",unit->Type->Ident,unit->Type->Name);
    fprintf(file,"\t(%d %d) ",unit->X,unit->Y);
    fprintf(file,"(%d %d) ",unit->IX,unit->IY);
    fprintf(file,"%d ",unit->Frame);
#ifdef NEW_HEADING
    fprintf(file,"%d\n",(unit->Direction*360)/256);
#else
    switch( unit->Heading ) {
	case HeadingN:	fprintf(file,"0\n");	break;
	case HeadingNE:	fprintf(file,"45\n");	break;
	case HeadingE:	fprintf(file,"90\n");	break;
	case HeadingSE:	fprintf(file,"135\n");break;
	case HeadingS:	fprintf(file,"180\n");break;
	case HeadingSW:	fprintf(file,"225\n");break;
	case HeadingW:	fprintf(file,"270\n");break;
	case HeadingNW:	fprintf(file,"315\n");break;
    }
#endif
    if( unit->Attacked ) {
	fprintf(file,"\t'attacked\n");
    }
    /*
    if( unit->Visible ) {
	fprintf(file,"\t'visible\n");
    }
    */
    if( unit->Removed ) {
	fprintf(file,"\t'removed\n");
    }
    if( unit->Selected ) {
	fprintf(file,"\t'selected\n");
    }
    if( unit->Constructed ) {
	fprintf(file,"\t'constructed\n");
    }
    fprintf(file,"\t%d ",unit->Player->Player);
    fprintf(file,"%d ",unit->Mana);
    fprintf(file,"%d ",unit->HP);
    fprintf(file,"(%d %d %d %d %d)\n"
	    ,unit->Bloodlust
	    ,unit->Haste
	    ,unit->Slow
	    ,unit->Invisible
	    ,unit->Shield);
    fprintf(file,"\t%d ",unit->GroupId);
    fprintf(file,"%d\n",unit->Value);

    fprintf(file,"\t(%d %d %d%s)\n"
	,unit->State
	,unit->SubAction
	,unit->Wait
	,unit->Reset ? " 'reset" : "");

    SaveCommand(&unit->Command,file);
    fprintf(file,"\t( ");
    for( i=0; i<unit->NextCount; ++i ) {	// Save all commands
	SaveCommand(unit->NextCommand+i,file);
    }
    fprintf(file,"  ))\n");
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
