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
/**@name unittype.c	-	The unit types. */
//
//	(c) Copyright 1998-2003 by Lutz Sammer and Jimmy Salmon
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
#include <ctype.h>

#include "stratagus.h"
#include "video.h"
#include "tileset.h"
#include "map.h"
#include "sound_id.h"
#include "unitsound.h"
#include "construct.h"
#include "unittype.h"
#include "player.h"
#include "missile.h"
#include "ccl.h"

#include "etlib/hash.h"

#include "myendian.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/**
**	Unit-type type definition
*/
global const char UnitTypeType[] = "unit-type";

IfDebug(
global int NoWarningUnitType;			/// quiet ident lookup
);

global UnitType* UnitTypes[UnitTypeMax];	/// unit-types definition
global int NumUnitTypes;			/// number of unit-types made

/*
**	Next unit type are used hardcoded in the source.
**
**	FIXME: find a way to make it configurable!
*/
global UnitType*UnitTypeOrcTanker;		/// Orc tanker unit type pointer
global UnitType*UnitTypeHumanTanker;		/// Human tanker unit type pointer
global UnitType*UnitTypeOrcTankerFull;		/// Orc tanker full unit type pointer
global UnitType*UnitTypeHumanTankerFull;	/// Human tanker full unit type pointer
global UnitType*UnitTypeHumanWorker;		/// Human worker
global UnitType*UnitTypeOrcWorker;		/// Orc worker
global UnitType*UnitTypeHumanWorkerWithGold;	/// Human worker with gold
global UnitType*UnitTypeOrcWorkerWithGold;	/// Orc worker with gold
global UnitType*UnitTypeHumanWorkerWithWood;	/// Human worker with wood
global UnitType*UnitTypeOrcWorkerWithWood;	/// Orc worker with wood
global UnitType*UnitTypeHumanWall;		/// Human wall
global UnitType*UnitTypeOrcWall;		/// Orc wall
global UnitType*UnitTypeCritter;		/// Critter unit type pointer
global UnitType*UnitTypeBerserker;		/// Berserker for berserker regeneration

/**
**	Mapping of W*rCr*ft number to our internal unit-type symbol.
**	The numbers are used in puds.
*/
global char** UnitTypeWcNames;

#ifdef DOXYGEN                          // no real code, only for document

/**
**	Lookup table for unit-type names
*/
local UnitType* UnitTypeHash[UnitTypeMax];

#else

/**
**	Lookup table for unit-type names
*/
local hashtable(UnitType*,UnitTypeMax) UnitTypeHash;

#endif

/**
**	Default resources for a new player.
*/
global int DefaultResources[MaxCosts];

/**
**	Default resources for a new player with low resources.
*/
global int DefaultResourcesLow[MaxCosts];

/**
**	Default resources for a new player with mid resources.
*/
global int DefaultResourcesMedium[MaxCosts];

/**
**	Default resources for a new player with high resources.
*/
global int DefaultResourcesHigh[MaxCosts];

/**
**	Default incomes for a new player.
*/
global int DefaultIncomes[MaxCosts];

/**
**	Default action for the resources.
*/
global char* DefaultActions[MaxCosts];

/**
**	Default names for the resources.
*/
global char* DefaultResourceNames[MaxCosts];

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Update the player stats for changed unit types.
**      @param reset indicates wether default value should be set to each stat ( level, upgrades )
*/
global void UpdateStats(int reset)
{
    UnitType* type;
    UnitStats* stats;
    unsigned player;
    unsigned i;
    unsigned j;

    //
    //  Update players stats
    //
    for (j = 0; j<NumUnitTypes; ++j) {
	type = UnitTypes[j];
        if (reset){
	    // LUDO : FIXME : reset loading of player stats !
	    for (player = 0; player < PlayerMax; ++player) {
		stats = &type->Stats[player];
		stats->AttackRange = type->_AttackRange;
		stats->SightRange = type->_SightRange;
		stats->Armor = type->_Armor;
		stats->BasicDamage = type->_BasicDamage;
		stats->PiercingDamage = type->_PiercingDamage;
		stats->Speed = type->_Speed;
		stats->HitPoints = type->_HitPoints;
		for (i = 0; i < MaxCosts; ++i) {
		    stats->Costs[i] = type->_Costs[i];
		}
		if (type->Building) {
		    stats->Level = 0;	// Disables level display
		} else {
		    stats->Level = 1;
		}
	    }
	}

	//
	//      As side effect we calculate the movement flags/mask here.
	//
	switch (type->UnitType) {
	    case UnitTypeLand:		// on land
		type->MovementMask =
		    MapFieldLandUnit
		    | MapFieldSeaUnit
		    | MapFieldBuilding	// already occuppied
		    | MapFieldCoastAllowed
		    | MapFieldWaterAllowed	// can't move on this
		    | MapFieldUnpassable;
		break;
	    case UnitTypeFly:		// in air
		type->MovementMask =
		    MapFieldAirUnit;	// already occuppied
		break;
	    case UnitTypeNaval:		// on water
		if (type->Transporter) {
		    type->MovementMask =
			MapFieldLandUnit
			| MapFieldSeaUnit
			| MapFieldBuilding	// already occuppied
			| MapFieldLandAllowed;	// can't move on this
		    // Johns: MapFieldUnpassable only for land units?
		} else {
		    type->MovementMask =
			MapFieldLandUnit
			| MapFieldSeaUnit
			| MapFieldBuilding	// already occuppied
			| MapFieldCoastAllowed
			| MapFieldLandAllowed	// can't move on this
			| MapFieldUnpassable;
		}
		break;
	    default:
		DebugLevel1Fn("Where moves this unit?\n");
		type->MovementMask = 0;
		break;
	}
	if( type->Building || type->ShoreBuilding ) {
	    // Shore building is something special.
	    if( type->ShoreBuilding ) {
		type->MovementMask =
		    MapFieldLandUnit
		    | MapFieldSeaUnit
		    | MapFieldBuilding	// already occuppied
		    | MapFieldLandAllowed;	// can't build on this
	    }
	    type->MovementMask |= MapFieldNoBuilding;
	    //
	    //	A little chaos, buildings without HP can be entered.
	    //	The oil-patch is a very special case.
	    //
	    if( type->_HitPoints ) {
		type->FieldFlags = MapFieldBuilding;
	    } else {
		type->FieldFlags = MapFieldNoBuilding;
	    }
	} else switch (type->UnitType) {
	    case UnitTypeLand:		// on land
		type->FieldFlags = MapFieldLandUnit;
		break;
	    case UnitTypeFly:		// in air
		type->FieldFlags = MapFieldAirUnit;
		break;
	    case UnitTypeNaval:		// on water
		type->FieldFlags = MapFieldSeaUnit;
		break;
	    default:
		DebugLevel1Fn("Where moves this unit?\n");
		type->FieldFlags = 0;
		break;
	}
    }
}

    /// Macro to fetch an 8bit value, to have some looking 8/16/32 bit funcs.
#define Fetch8(p)   (*((unsigned char*)(p))++)

/**
**	Parse UDTA area from puds.
**
**	@param udta	Pointer to udta area.
**	@param length	length of udta area.
*/
global void ParsePudUDTA(const char* udta,int length __attribute__((unused)))
{
    int i;
    int v;
    const char* start;
    UnitType* unittype;

    // FIXME: not the fastest, remove UnitTypeByWcNum from loops!
    IfDebug(
	if( length!=5694 && length!=5948 ) {
	    DebugLevel0("\n***\n");
	    DebugLevel0Fn("%d\n" _C_ length);
	    DebugLevel0("***\n\n");
	}
    )
    start=udta;

    for( i=0; i<110; ++i ) {		// overlap frames
	unittype=UnitTypeByWcNum(i);
	v=FetchLE16(udta);
	unittype->Construction=ConstructionByWcNum(v);
    }
    for( i=0; i<508; ++i ) {		// skip obsolete data
	v=FetchLE16(udta);
    }
    for( i=0; i<110; ++i ) {		// sight range
	unittype=UnitTypeByWcNum(i);
	v=FetchLE32(udta);
	unittype->_SightRange=v;
    }
    for( i=0; i<110; ++i ) {		// hit points
	unittype=UnitTypeByWcNum(i);
	v=FetchLE16(udta);
	unittype->_HitPoints=v;
    }
    for( i=0; i<110; ++i ) {		// Flag if unit is magic
	unittype=UnitTypeByWcNum(i);
	v=Fetch8(udta);
	unittype->Magic=v;
    }
    for( i=0; i<110; ++i ) {		// Build time * 6 = one second FRAMES
	unittype=UnitTypeByWcNum(i);
	v=Fetch8(udta);
	unittype->_Costs[TimeCost]=v;
    }
    for( i=0; i<110; ++i ) {		// Gold cost / 10
	unittype=UnitTypeByWcNum(i);
	v=Fetch8(udta);
	unittype->_Costs[GoldCost]=v*10;
    }
    for( i=0; i<110; ++i ) {		// Lumber cost / 10
	unittype=UnitTypeByWcNum(i);
	v=Fetch8(udta);
	unittype->_Costs[WoodCost]=v*10;
    }
    for( i=0; i<110; ++i ) {		// Oil cost / 10
	unittype=UnitTypeByWcNum(i);
	v=Fetch8(udta);
	unittype->_Costs[OilCost]=v*10;
    }
    for( i=0; i<110; ++i ) {		// Unit size in tiles
	unittype=UnitTypeByWcNum(i);
	v=FetchLE16(udta);
	unittype->TileWidth=v;
	v=FetchLE16(udta);
	unittype->TileHeight=v;
    }
    for( i=0; i<110; ++i ) {		// Box size in pixel
	unittype=UnitTypeByWcNum(i);
	v=FetchLE16(udta);
	unittype->BoxWidth=v;
	v=FetchLE16(udta);
	unittype->BoxHeight=v;
    }

    for( i=0; i<110; ++i ) {		// Attack range
	unittype=UnitTypeByWcNum(i);
	v=Fetch8(udta);
	unittype->_AttackRange=v;
    }
    for( i=0; i<110; ++i ) {		// React range
	unittype=UnitTypeByWcNum(i);
	v=Fetch8(udta);
	unittype->ReactRangeComputer=v;
    }
    for( i=0; i<110; ++i ) {		// React range
	unittype=UnitTypeByWcNum(i);
	v=Fetch8(udta);
	unittype->ReactRangePerson=v;
    }
    for( i=0; i<110; ++i ) {		// Armor
	unittype=UnitTypeByWcNum(i);
	v=Fetch8(udta);
	unittype->_Armor=v;
    }
    for( i=0; i<110; ++i ) {		// Selectable via rectangle
	unittype=UnitTypeByWcNum(i);
	v=Fetch8(udta);
	unittype->SelectableByRectangle=v!=0;
    }
    for( i=0; i<110; ++i ) {		// Priority
	unittype=UnitTypeByWcNum(i);
	v=Fetch8(udta);
	unittype->Priority=v;
    }
    for( i=0; i<110; ++i ) {		// Basic damage
	unittype=UnitTypeByWcNum(i);
	v=Fetch8(udta);
	unittype->_BasicDamage=v;
    }
    for( i=0; i<110; ++i ) {		// Piercing damage
	unittype=UnitTypeByWcNum(i);
	v=Fetch8(udta);
	unittype->_PiercingDamage=v;
    }
    for( i=0; i<110; ++i ) {		// Weapons upgradable
	unittype=UnitTypeByWcNum(i);
	v=Fetch8(udta);
	unittype->WeaponsUpgradable=v;
    }
    for( i=0; i<110; ++i ) {		// Armor upgradable
	unittype=UnitTypeByWcNum(i);
	v=Fetch8(udta);
	unittype->ArmorUpgradable=v;
    }
    for( i=0; i<110; ++i ) {		// Missile Weapon
	unittype=UnitTypeByWcNum(i);
	v=Fetch8(udta);
	unittype->Missile.Name=strdup(MissileTypeWcNames[v]);
	DebugCheck( unittype->Missile.Missile );
    }
    for( i=0; i<110; ++i ) {		// Unit type
	unittype=UnitTypeByWcNum(i);
	v=Fetch8(udta);
	unittype->UnitType=v;
    }
    for( i=0; i<110; ++i ) {		// Decay rate * 6 = secs
	unittype=UnitTypeByWcNum(i);
	v=Fetch8(udta);
	unittype->DecayRate=v;
    }
    for( i=0; i<110; ++i ) {		// Annoy computer factor
	unittype=UnitTypeByWcNum(i);
	v=Fetch8(udta);
	unittype->AnnoyComputerFactor=v;
    }
    for( i=0; i<58; ++i ) {		// 2nd mouse button action
	unittype=UnitTypeByWcNum(i);
	v=Fetch8(udta);
	unittype->MouseAction=v;
    }
    for( ; i<110; ++i ) {		// 2nd mouse button action
	unittype=UnitTypeByWcNum(i);
	unittype->MouseAction=0;
    }
    for( i=0; i<110; ++i ) {		// Point value for killing unit
	unittype=UnitTypeByWcNum(i);
	v=FetchLE16(udta);
	unittype->Points=v;
    }
    for( i=0; i<110; ++i ) {		// Can target (1 land, 2 sea, 4 air)
	unittype=UnitTypeByWcNum(i);
	v=Fetch8(udta);
	unittype->CanTarget=v;
    }

    for( i=0; i<110; ++i ) {		// Flags
	unittype=UnitTypeByWcNum(i);
	v=FetchLE32(udta);
    /// Nice looking bit macro
#define BIT(b,v)	(((v>>b))&1)
	unittype->LandUnit=BIT(0,v);
	unittype->AirUnit=BIT(1,v);
	unittype->ExplodeWhenKilled=BIT(2,v);
	unittype->SeaUnit=BIT(3,v);
	unittype->Critter=BIT(4,v);
	unittype->Building=BIT(5,v);
	unittype->Submarine=BIT(6,v);
	unittype->CanSeeSubmarine=BIT(7,v);
	unittype->CowerWorker=BIT(8,v);
	unittype->Tanker=BIT(9,v);
	unittype->Transporter=BIT(10,v);
	unittype->CanStore[GoldCost]=BIT(12,v);
	unittype->Vanishes=BIT(13,v);
	unittype->GroundAttack=BIT(14,v);
	unittype->IsUndead=BIT(15,v);
	unittype->ShoreBuilding=BIT(16,v);
	unittype->CanCastSpell=BIT(17,v);
	unittype->CanStore[WoodCost]=BIT(18,v);
	unittype->CanAttack=BIT(19,v);
	unittype->Tower=BIT(20,v);
	unittype->Hero=BIT(23,v);
	unittype->CanStore[OilCost]=BIT(24,v);
	unittype->Volatile=BIT(25,v);
	unittype->CowerMage=BIT(26,v);
	unittype->Organic=BIT(27,v);
	
	if (BIT(11,v)||BIT(21,v)) {
	    unittype->GivesResource=OilCost;
	}
	if (BIT(22,v)) {
	    unittype->GivesResource=GoldCost;
	}

#ifdef DEBUG
	if( BIT(28,v) )	DebugLevel0("Unused bit 28 used in %d\n" _C_ i);
	if( BIT(29,v) )	DebugLevel0("Unused bit 29 used in %d\n" _C_ i);
	if( BIT(30,v) )	DebugLevel0("Unused bit 30 used in %d\n" _C_ i);
	if( BIT(31,v) )	DebugLevel0("Unused bit 31 used in %d\n" _C_ i);
#endif
#undef BIT
	//
	//	Unit type checks.
	//
	if( unittype->CanCastSpell && !unittype->_MaxMana ) {
	    DebugLevel0Fn("%s: Need max mana value\n" _C_ unittype->Ident);
	    unittype->_MaxMana=255;
	}
    }

    // FIXME: peon applies also to peon-with-gold and peon-with-wood
    // FIXME: oil-tanker applies also to oil-tanker-full

    DebugLevel0("\tUDTA used %d bytes\n" _C_ udta-start);

    UpdateStats(1);
}

/**
**	Get the animations structure by ident.
**
**	@param ident	Identifier for the animation.
**	@return		Pointer to the animation structure.
**
**	@todo	Remove the use of scheme symbols to store, use own hash.
*/
global Animations* AnimationsByIdent(const char* ident)
{
    Animations** tmp;

    tmp=(Animations**)hash_find(AnimationsHash,ident);
    if( tmp ) {
	return *tmp;
    }
    DebugLevel0Fn("Warning animation `%s' not found\n" _C_ ident);
    return NULL;
}

/**
**	Save state of an animition set to file.
**
**	@param name	Animation name.
**	@param anim	Save this animation.
**	@param file	Output file.
*/
local void SaveAnimation(const char* name,const Animation* anim,FILE* file)
{
    int i;
    int p;
    int frame;
    const Animation* temp;

    if( anim ) {
	//
	//	Calculate the wait sum and pixels count.
	//
	i=p=0;
	for( temp=anim; ; ++temp ) {
	    i+=temp->Sleep&0xFF;
	    p+=temp->Pixel;
	    if( temp->Flags&AnimationEnd ) {
		break;
	    }
	    if( temp->Flags&AnimationRestart ) {
		i=p=0;
	    }
	}

	fprintf(file,"\n  '%s '(\t; #%d",name,i);
	if( p ) {
	    fprintf(file," P%d",p);
	}

	temp=anim;
	frame=0;
	for( ;; ) {
	    fprintf(file,"\n    ");
	    for( i=0; i<4; ++i ) {
		if( i ) {
		    fputc(' ',file);
		}
		frame+=temp->Frame;
		fprintf(file,"#(%2d %d %3d %3d)",
			temp->Flags&0x7F,temp->Pixel,temp->Sleep&0xFF,frame);
		if( temp->Flags&AnimationRestart ) {
		    frame=0;
		}
		if( temp->Flags&AnimationEnd ) {
		    fprintf(file,")");
		    return;
		}
		++temp;
	    }
	}
    }
}

/**
**	Save state of an unit-stats to file.
**
**	@param stats	Unit-stats to save.
**	@param ident	Unit-type ident.
**	@param plynr	Player number.
**	@param file	Output file.
*/
local void SaveUnitStats(const UnitStats* stats,const char* ident,int plynr,
	FILE* file)
{
    DebugCheck(plynr>=PlayerMax);
    int j;

    fprintf(file,"(define-unit-stats '%s %d\n  ",ident,plynr);
    fprintf(file,"'level %d ",stats->Level);
    fprintf(file,"'speed %d ",stats->Speed);
    fprintf(file,"'attack-range %d ",stats->AttackRange);
    fprintf(file,"'sight-range %d\n  ",stats->SightRange);
    fprintf(file,"'armor %d ",stats->Armor);
    fprintf(file,"'basic-damage %d ",stats->BasicDamage);
    fprintf(file,"'piercing-damage %d ",stats->PiercingDamage);
    fprintf(file,"'hit-points %d\n  ",stats->HitPoints);
    fprintf(file,"'costs '(");
    for( j=0; j<MaxCosts; ++j ) {
	if( j ) {
//	    if( j==MaxCosts/2 ) {
//		fputs("\n    ",file);
//	    } else {
		fputc(' ',file);
//	    }
	}
	fprintf(file,"%s %d",DefaultResourceNames[j],stats->Costs[j]);
    }

    fprintf(file,") )\n");
}

/**
**	Save state of the unit-type table to file.
**
**	@param file	Output file.
*/
global void SaveUnitTypes(FILE* file)
{
    int i;
    int j;

    fprintf(file,"\n;;; -----------------------------------------\n");
    fprintf(file,";;; MODULE: unittypes $Id$\n\n");

    //	Save all stats

    for( i=0; i<NumUnitTypes; ++i ) {
	fputc('\n',file);
	for( j=0; j<PlayerMax; ++j ) {
	    SaveUnitStats(&UnitTypes[i]->Stats[j],UnitTypes[i]->Ident,j,file);
	}
    }
}

/**
**	Find unit-type by identifier.
**
**	@param ident	The unit-type identifier.
**	@return		Unit-type pointer.
*/
global UnitType* UnitTypeByIdent(const char* ident)
{
    UnitType* const* type;

    type=(UnitType* const*)hash_find(UnitTypeHash,ident);
    if( type ) {
	return *type;
    }

    IfDebug(
	if( !NoWarningUnitType ) {
	    DebugLevel0Fn("Name `%s' not found\n" _C_ ident);
	}
    );

    return NULL;
}

/**
**	Find unit-type by wc number.
**
**	@param num	The unit-type number used in f.e. puds.
**	@return		Unit-type pointer.
*/
global UnitType* UnitTypeByWcNum(unsigned num)
{
    return UnitTypeByIdent(UnitTypeWcNames[num]);
}

/**
**	Allocate an empty unit-type slot.
**
**	@param ident	Identifier to identify the slot (malloced by caller!).
**
**	@return		New allocated (zeroed) unit-type pointer.
*/
global UnitType* NewUnitTypeSlot(char* ident)
{
    UnitType* type;

    type=malloc(sizeof(UnitType));
    if( !type ) {
	fprintf(stderr,"Out of memory\n");
	ExitFatal(-1);
    }
    memset(type,0,sizeof(UnitType));
    type->Type=NumUnitTypes;
    type->Ident=ident;
    UnitTypes[NumUnitTypes++]=type;
    //
    //	Rehash.
    //
    *(UnitType**)hash_add(UnitTypeHash,type->Ident)=type;
    return type;
}

/**
**	Draw unit-type on map.
**
**	@param type	Unit-type pointer.
**	@param frame	Animation frame of unit-type.
**	@param x	Screen X pixel postion to draw unit-type.
**	@param y	Screen Y pixel postion to draw unit-type.
**
**	@todo	Do screen position caculation in high level.
**		Better way to handle in x mirrored sprites.
*/
global void DrawUnitType(const UnitType* type,int frame,int x,int y)
{
    // FIXME: move this calculation to high level.
    x-=(type->Width-type->TileWidth*TileSizeX)/2;
    y-=(type->Height-type->TileHeight*TileSizeY)/2;

    // FIXME: This is a hack for mirrored sprites
    if( frame<0 ) {
	VideoDrawClipX(type->Sprite,-frame,x,y);
    } else {
	VideoDrawClip(type->Sprite,frame,x,y);
    }
}

/**
**	Init unit types.
*/
global void InitUnitTypes(int reset_player_stats)
{
    int type;

    for( type=0; type<NumUnitTypes; ++type ) {
	//
	//	Initialize:
	//
	DebugCheck( UnitTypes[type]->Type!=type );
	//
	//	Add idents to hash.
	//
	*(UnitType**)hash_add(UnitTypeHash,UnitTypes[type]->Ident)
		=UnitTypes[type];
	//
	//	Hardcoded incomes, FIXME: should be moved to some configs.
	//
	if( !strcmp(UnitTypes[type]->Ident,"unit-elven-lumber-mill") ) {
	    UnitTypes[type]->ImproveIncomes[WoodCost]=
		    DefaultIncomes[WoodCost]+25;
	}
	if( !strcmp(UnitTypes[type]->Ident,"unit-troll-lumber-mill") ) {
	    UnitTypes[type]->ImproveIncomes[WoodCost]=
		    DefaultIncomes[WoodCost]+25;
	}
	if( !strcmp(UnitTypes[type]->Ident,"unit-human-refinery") ) {
	    UnitTypes[type]->ImproveIncomes[OilCost]=
		    DefaultIncomes[OilCost]+25;
	}
	if( !strcmp(UnitTypes[type]->Ident,"unit-orc-refinery") ) {
	    UnitTypes[type]->ImproveIncomes[OilCost]=
		    DefaultIncomes[OilCost]+25;
	}
	if( !strcmp(UnitTypes[type]->Ident,"unit-keep") ) {
	    UnitTypes[type]->ImproveIncomes[GoldCost]=
		    DefaultIncomes[GoldCost]+10;
	}
	if( !strcmp(UnitTypes[type]->Ident,"unit-stronghold") ) {
	    UnitTypes[type]->ImproveIncomes[GoldCost]=
		    DefaultIncomes[GoldCost]+10;
	}
	if( !strcmp(UnitTypes[type]->Ident,"unit-castle") ) {
	    UnitTypes[type]->ImproveIncomes[GoldCost]=
		    DefaultIncomes[GoldCost]+20;
	}
	if( !strcmp(UnitTypes[type]->Ident,"unit-fortress") ) {
	    UnitTypes[type]->ImproveIncomes[GoldCost]=
		    DefaultIncomes[GoldCost]+20;
	}
    }

    // LUDO : called after game is loaded -> don't reset stats !
    UpdateStats(reset_player_stats);	// Calculate the stats

    //
    //	Setup hardcoded unit types. FIXME: should be moved to some configs.
    //
    UnitTypeHumanTanker=UnitTypeByIdent("unit-human-oil-tanker");
    UnitTypeOrcTanker=UnitTypeByIdent("unit-orc-oil-tanker");
    UnitTypeHumanTankerFull=UnitTypeByIdent("unit-human-oil-tanker-full");
    UnitTypeOrcTankerFull=UnitTypeByIdent("unit-orc-oil-tanker-full");
    UnitTypeHumanWorker=UnitTypeByIdent("unit-peasant");
    UnitTypeOrcWorker=UnitTypeByIdent("unit-peon");
    UnitTypeHumanWorkerWithGold=UnitTypeByIdent("unit-peasant-with-gold");
    UnitTypeOrcWorkerWithGold=UnitTypeByIdent("unit-peon-with-gold");
    UnitTypeHumanWorkerWithWood=UnitTypeByIdent("unit-peasant-with-wood");
    UnitTypeOrcWorkerWithWood=UnitTypeByIdent("unit-peon-with-wood");
    UnitTypeHumanWall=UnitTypeByIdent("unit-human-wall");
    UnitTypeOrcWall=UnitTypeByIdent("unit-orc-wall");
    UnitTypeCritter=UnitTypeByIdent("unit-critter");
    UnitTypeBerserker=UnitTypeByIdent("unit-berserker");
}

/**
**	Load the graphics for the unit-types.
*/
global void LoadUnitTypes(void)
{
    UnitType* type;
    const char* file;
    int i;

    for( i=0; i<NumUnitTypes; ++i ) {
	type=UnitTypes[i];
	if( (file=type->ShadowFile) ) {
	    char *buf;
	    buf=alloca(strlen(file)+9+1);
	    file=strcat(strcpy(buf,"graphics/"),file);
	    ShowLoadProgress("\tUnit `%s'\n",file);
	    type->ShadowSprite=LoadSprite(file,type->ShadowWidth,type->ShadowHeight);
	}

	//
	//	Unit-type uses the same sprite as an other.
	//
	if( type->SameSprite ) {
	    continue;
	}

	//
	//	FIXME: must handle terrain different!
	//

	file=type->File[TheMap.Terrain];
	if( !file ) {			// default one
	    file=type->File[0];
	}
	if( file ) {
	    char* buf;

	    buf=alloca(strlen(file)+9+1);
	    file=strcat(strcpy(buf,"graphics/"),file);
	    ShowLoadProgress("\tUnit `%s'\n",file);
	    type->Sprite=LoadSprite(file,type->Width,type->Height);
	}
    }

    for( i=0; i<NumUnitTypes; ++i ) {
	type=UnitTypes[i];
	//
	//	Unit-type uses the same sprite as an other.
	//
	if( type->SameSprite ) {
	    const UnitType* unittype;

	    unittype=UnitTypeByIdent(type->SameSprite);
	    if( !unittype ) {
		PrintFunction();
		fprintf(stdout,"Unit-type %s not found\n" ,type->SameSprite);
		ExitFatal(-1);
	    }
	    type->Sprite=unittype->Sprite;
	}

	//
	//	Lookup icons.
	//
	type->Icon.Icon=IconByIdent(type->Icon.Name);
	//
	//	Lookup missiles.
	//
	type->Missile.Missile=MissileTypeByIdent(type->Missile.Name);
	//
	//	Lookup corpse.
	//
	if( type->CorpseName ) {
	    type->CorpseType=UnitTypeByIdent(type->CorpseName);
	}

	// FIXME: should i copy the animations of same graphics?
    }

    // FIXME: must copy unit data from peon/peasant to with gold/wood
    // FIXME: must copy unit data from tanker to tanker full
}

/**
**	Cleanup the unit-type module.
*/
global void CleanUnitTypes(void)
{
    UnitType* type;
    void** ptr;
    int i;
    int j;

    DebugLevel0Fn("FIXME: icon, sounds not freed.\n");

    //
    //	Mapping the original unit-type numbers in puds to our internal strings
    //
    if( (ptr=(void**)UnitTypeWcNames) ) {	// Free all old names
	while( *ptr ) {
	    free(*ptr++);
	}
	free(UnitTypeWcNames);

	UnitTypeWcNames=NULL;
    }

    //	FIXME: scheme contains references on this structure.
    //	Clean all animations.

    for( i=0; i<NumUnitTypes; ++i )
    {
	type=UnitTypes[i];
	Animations* anims;

	if( !(anims=type->Animations) ) {	// Must be handled?
	    continue;
	}
    	for( j=i; j<NumUnitTypes; ++j ) {	// Remove all uses.
	    if( anims==UnitTypes[j]->Animations ) {
		UnitTypes[j]->Animations=NULL;
	    }
	}
	type->Animations=NULL;
	if( anims->Still ) {
	    free(anims->Still);
	}
	if( anims->Move ) {
	    free(anims->Move);
	}
	if( anims->Attack ) {
	    free(anims->Attack);
	}
	if( anims->Die ) {
	    free(anims->Die);
	}
	free(anims);
    }

    //	Clean all unit-types

    for( i=0;i<NumUnitTypes;++i ) {
	type=UnitTypes[i];
	hash_del(UnitTypeHash,type->Ident);

	DebugCheck( !type->Ident );
	free(type->Ident);
	DebugCheck( !type->Name );
	free(type->Name);

    	if( type->SameSprite ) {
	    free(type->SameSprite);
	}
	if( type->File[0] ) {
	    free(type->File[0]);
	}
	if( type->File[1] ) {
	    free(type->File[1]);
	}
	if( type->File[2] ) {
	    free(type->File[2]);
	}
	if( type->File[3] ) {
	    free(type->File[3]);
	}
	if( type->Icon.Name ) {
	    free(type->Icon.Name);
	}
	if( type->Missile.Name ) {
	    free(type->Missile.Name);
	}
	if( type->CorpseName ) {
	    free(type->CorpseName);
	}

	//
	//	FIXME: Sounds can't be freed, they still stuck in sound hash.
	//
	if( type->Sound.Selected.Name ) {
	    free(type->Sound.Selected.Name);
	}
	if( type->Sound.Acknowledgement.Name ) {
	    free(type->Sound.Acknowledgement.Name);
	}
	if( type->Sound.Ready.Name ) {
	    free(type->Sound.Ready.Name);
	}
	if( type->Sound.Help.Name ) {
	    free(type->Sound.Help.Name);
	}
	if( type->Sound.Dead.Name ) {
	    free(type->Sound.Dead.Name);
	}
	if( type->Weapon.Attack.Name ) {
	    free(type->Weapon.Attack.Name);
	}

	if( !type->SameSprite ) {	// our own graphics
	    VideoSaveFree(type->Sprite);
	}
#ifdef USE_OPENGL
	for( i=0; i<PlayerMax; ++i ) {
	    VideoSaveFree(type->PlayerColorSprite[i]);
	}
#endif
	free(UnitTypes[i]);
	UnitTypes[i]=0;
    }
    NumUnitTypes=0;

    //
    //	Clean hardcoded unit types.
    //
    UnitTypeHumanTanker=NULL;
    UnitTypeOrcTanker=NULL;
    UnitTypeHumanTankerFull=NULL;
    UnitTypeOrcTankerFull=NULL;
    UnitTypeHumanWorker=NULL;
    UnitTypeOrcWorker=NULL;
    UnitTypeHumanWorkerWithGold=NULL;
    UnitTypeOrcWorkerWithGold=NULL;
    UnitTypeHumanWorkerWithWood=NULL;
    UnitTypeOrcWorkerWithWood=NULL;
    UnitTypeHumanWall=NULL;
    UnitTypeOrcWall=NULL;
    UnitTypeCritter=NULL;
    UnitTypeBerserker=NULL;
}

//@}
