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
/**@name ccl_unittype.c	-	The unit-type ccl functions. */
//
//	(c) Copyright 1999-2002 by Lutz Sammer
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

#include "freecraft.h"
#include "video.h"
#include "tileset.h"
#include "map.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "icons.h"
#include "missile.h"
#include "ccl.h"
#include "construct.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

IfDebug(
extern int NoWarningUnitType;		/// quiet ident lookup.
);

local long SiodUnitTypeTag;		/// siod unit-type object

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Parse unit-type.
**
**	@note Should write a general parser for this.
**
**	@param list	List describing the unit-type.
*/
local SCM CclDefineNewUnitType(SCM list)
{
    SCM value;
    SCM sublist;
    UnitType* type;
    char* str;
    int i;

    //	Slot identifier

    str=gh_scm2newstr(gh_car(list),NULL);
    list=gh_cdr(list);

    IfDebug( i=NoWarningUnitType; NoWarningUnitType=1; );
    type=UnitTypeByIdent(str);
    IfDebug( NoWarningUnitType=i; );
    if( type ) {
	DebugLevel0Fn("Redefining unit-type `%s'\n" _C_ str);
	free(str);
	// FIXME: loose memory, old content isn't freed.
    } else {
	type=NewUnitTypeSlot(str);
    }

    //
    //	Parse the list:	(still everything could be changed!)
    //
    while( !gh_null_p(list) ) {

	value=gh_car(list);
	list=gh_cdr(list);

	if( gh_eq_p(value,gh_symbol2scm("name")) ) {
	    type->Name=gh_scm2newstr(gh_car(list),NULL);
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("use")) ) {
	    type->SameSprite=gh_scm2newstr(gh_car(list),NULL);
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("files")) ) {
	    sublist=gh_car(list);
	    list=gh_cdr(list);
	    while( !gh_null_p(sublist) ) {

		value=gh_car(sublist);
		sublist=gh_cdr(sublist);

		for( i=0; i<TilesetMax; ++i ) {
		    if( gh_eq_p(value,gh_symbol2scm(Tilesets[i]->Ident)) ) {
			type->File[i]=gh_scm2newstr(gh_car(sublist),NULL);
			break;
		    }
		}
		if( i==TilesetMax ) {
		   // FIXME: this leaves half initialized unit-type
		   errl("Unsupported tileset tag",value);
		}
		sublist=gh_cdr(sublist);
	    }
	} else if( gh_eq_p(value,gh_symbol2scm("size")) ) {
	    sublist=gh_car(list);
	    list=gh_cdr(list);
	    type->Width=gh_scm2int(gh_car(sublist));
	    type->Height=gh_scm2int(gh_cadr(sublist));
	} else if( gh_eq_p(value,gh_symbol2scm("animations")) ) {
	    type->Animations=
		AnimationsByIdent(str=gh_scm2newstr(gh_car(list),NULL));
	    free(str);
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("icon")) ) {
	    type->Icon.Name=gh_scm2newstr(gh_car(list),NULL);
	    type->Icon.Icon=NULL;
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("costs")) ) {
	    sublist=gh_car(list);
	    list=gh_cdr(list);
	    while( !gh_null_p(sublist) ) {

		value=gh_car(sublist);
		sublist=gh_cdr(sublist);

		for( i=0; i<MaxCosts; ++i ) {
		    if( gh_eq_p(value,gh_symbol2scm((char*)DEFAULT_NAMES[i])) ) {
			type->_Costs[i]=gh_scm2int(gh_car(sublist));
			break;
		    }
		}
		if( i==MaxCosts ) {
		   // FIXME: this leaves half initialized unit-type
		   errl("Unsupported resource tag",value);
		}
		sublist=gh_cdr(sublist);
	    }
	} else if( gh_eq_p(value,gh_symbol2scm("construction")) ) {
	    // FIXME: What if constructions arn't yet loaded?
	    str=gh_scm2newstr(gh_car(list),NULL);
	    type->Construction=ConstructionByIdent(str);
	    list=gh_cdr(list);
	    free(str);
	} else if( gh_eq_p(value,gh_symbol2scm("speed")) ) {
	    type->_Speed=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("hit-points")) ) {
	    type->_HitPoints=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("magic")) ) {
	    type->Magic=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("tile-size")) ) {
	    sublist=gh_car(list);
	    list=gh_cdr(list);
	    type->TileWidth=gh_scm2int(gh_car(sublist));
	    type->TileHeight=gh_scm2int(gh_cadr(sublist));
	} else if( gh_eq_p(value,gh_symbol2scm("box-size")) ) {
	    sublist=gh_car(list);
	    list=gh_cdr(list);
	    type->BoxWidth=gh_scm2int(gh_car(sublist));
	    type->BoxHeight=gh_scm2int(gh_cadr(sublist));
	} else if( gh_eq_p(value,gh_symbol2scm("sight-range")) ) {
	    type->_SightRange=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("computer-reaction-range")) ) {
	    type->ReactRangeComputer=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("person-reaction-range")) ) {
	    type->ReactRangePerson=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("armor")) ) {
	    type->_Armor=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("basic-damage")) ) {
	    type->_BasicDamage=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("piercing-damage")) ) {
	    type->_PiercingDamage=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("missile")) ) {
	    type->Missile.Name=gh_scm2newstr(gh_car(list),NULL);
	    type->Missile.Missile=NULL;
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("min-attack-range")) ) {
	    type->MinAttackRange=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("max-attack-range")) ) {
	    type->_AttackRange=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("weapons-upgradable")) ) {
	    type->WeaponsUpgradable=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("armor-upgradable")) ) {
	    type->ArmorUpgradable=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("priority")) ) {
	    type->Priority=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("annoy-computer-factor")) ) {
	    type->AnnoyComputerFactor=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("decay-rate")) ) {
	    type->DecayRate=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("points")) ) {
	    type->Points=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("demand")) ) {
	    type->Demand=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("supply")) ) {
	    type->Supply=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("corpse")) ) {
	    sublist=gh_car(list);
	    list=gh_cdr(list);
	    type->CorpseName=gh_scm2newstr(gh_car(sublist),NULL);
	    type->CorpseType=NULL;
	    type->CorpseScript=gh_scm2int(gh_cadr(sublist));
	} else if( gh_eq_p(value,gh_symbol2scm("explode-when-killed")) ) {
	    type->ExplodeWhenKilled=1;
	    list=gh_cdr(list);

	} else if( gh_eq_p(value,gh_symbol2scm("type-land")) ) {
	    type->UnitType=UnitTypeLand;
	} else if( gh_eq_p(value,gh_symbol2scm("type-fly")) ) {
	    type->UnitType=UnitTypeFly;
	} else if( gh_eq_p(value,gh_symbol2scm("type-naval")) ) {
	    type->UnitType=UnitTypeNaval;

	} else if( gh_eq_p(value,gh_symbol2scm("right-none")) ) {
	    type->MouseAction=MouseActionNone;
	} else if( gh_eq_p(value,gh_symbol2scm("right-attack")) ) {
	    type->MouseAction=MouseActionAttack;
	} else if( gh_eq_p(value,gh_symbol2scm("right-move")) ) {
	    type->MouseAction=MouseActionMove;
	} else if( gh_eq_p(value,gh_symbol2scm("right-harvest")) ) {
	    type->MouseAction=MouseActionHarvest;
	} else if( gh_eq_p(value,gh_symbol2scm("right-haul-oil")) ) {
	    type->MouseAction=MouseActionHaulOil;
	} else if( gh_eq_p(value,gh_symbol2scm("right-demolish")) ) {
	    type->MouseAction=MouseActionDemolish;
	} else if( gh_eq_p(value,gh_symbol2scm("right-sail")) ) {
	    type->MouseAction=MouseActionSail;

	} else if( gh_eq_p(value,gh_symbol2scm("can-ground-attack")) ) {
	    type->GroundAttack=1;
	} else if( gh_eq_p(value,gh_symbol2scm("can-attack")) ) {
	    type->CanAttack=1;

	} else if( gh_eq_p(value,gh_symbol2scm("can-target-land")) ) {
	    type->CanTarget|=CanTargetLand;
	} else if( gh_eq_p(value,gh_symbol2scm("can-target-sea")) ) {
	    type->CanTarget|=CanTargetSea;
	} else if( gh_eq_p(value,gh_symbol2scm("can-target-air")) ) {
	    type->CanTarget|=CanTargetAir;

	} else if( gh_eq_p(value,gh_symbol2scm("building")) ) {
	    type->Building=1;
	} else if( gh_eq_p(value,gh_symbol2scm("shore-building")) ) {
	    type->ShoreBuilding=1;
	} else if( gh_eq_p(value,gh_symbol2scm("land-unit")) ) {
	    type->LandUnit=1;
	} else if( gh_eq_p(value,gh_symbol2scm("air-unit")) ) {
	    type->AirUnit=1;
	} else if( gh_eq_p(value,gh_symbol2scm("sea-unit")) ) {
	    type->SeaUnit=1;
	} else if( gh_eq_p(value,gh_symbol2scm("critter")) ) {
	    type->Critter=1;
	} else if( gh_eq_p(value,gh_symbol2scm("submarine")) ) {
	    type->Submarine=1;
	} else if( gh_eq_p(value,gh_symbol2scm("can-see-submarine")) ) {
	    type->CanSeeSubmarine=1;
	} else if( gh_eq_p(value,gh_symbol2scm("transporter")) ) {
	    type->Transporter=1;
	} else if( gh_eq_p(value,gh_symbol2scm("cower-worker")) ) {
	    type->CowerWorker=1;
	} else if( gh_eq_p(value,gh_symbol2scm("tanker")) ) {
	    type->Tanker=1;
	} else if( gh_eq_p(value,gh_symbol2scm("gives-oil")) ) {
	    type->GivesOil=1;
	} else if( gh_eq_p(value,gh_symbol2scm("stores-gold")) ) {
	    type->StoresGold=1;
	} else if( gh_eq_p(value,gh_symbol2scm("stores-wood")) ) {
	    type->StoresWood=1;
	} else if( gh_eq_p(value,gh_symbol2scm("oil-patch")) ) {
	    type->OilPatch=1;
	} else if( gh_eq_p(value,gh_symbol2scm("gives-gold")) ) {
	    type->GoldMine=1;
	} else if( gh_eq_p(value,gh_symbol2scm("stores-oil")) ) {
	    type->StoresOil=1;
	} else if( gh_eq_p(value,gh_symbol2scm("vanishes")) ) {
	    type->Vanishes=1;
	} else if( gh_eq_p(value,gh_symbol2scm("tower")) ) {
	    type->Tower=1;
	} else if( gh_eq_p(value,gh_symbol2scm("hero")) ) {
	    type->Hero=1;
	} else if( gh_eq_p(value,gh_symbol2scm("volatile")) ) {
	    type->Volatile=1;
	} else if( gh_eq_p(value,gh_symbol2scm("cower-mage")) ) {
	    type->CowerMage=1;
	} else if( gh_eq_p(value,gh_symbol2scm("isundead")) ) {
	    type->IsUndead=1;
	} else if( gh_eq_p(value,gh_symbol2scm("can-cast-spell")) ) {
	    type->CanCastSpell=1;
	} else if( gh_eq_p(value,gh_symbol2scm("organic")) ) {
	    type->Organic=1;
	} else if( gh_eq_p(value,gh_symbol2scm("selectable-by-rectangle")) ) {
	    type->SelectableByRectangle=1;
	} else if( gh_eq_p(value,gh_symbol2scm("teleporter")) ) {
	    type->Teleporter=1;
	} else if( gh_eq_p(value,gh_symbol2scm("sounds")) ) {
	    sublist=gh_car(list);
	    list=gh_cdr(list);
	    while( !gh_null_p(sublist) ) {

		value=gh_car(sublist);
		sublist=gh_cdr(sublist);

		if( gh_eq_p(value,gh_symbol2scm("selected")) ) {
		    type->Sound.Selected.Name=gh_scm2newstr(
			gh_car(sublist),NULL);
		    sublist=gh_cdr(sublist);
		} else if( gh_eq_p(value,gh_symbol2scm("acknowledge")) ) {
		    type->Sound.Acknowledgement.Name=gh_scm2newstr(
			gh_car(sublist),NULL);
		    sublist=gh_cdr(sublist);
		} else if( gh_eq_p(value,gh_symbol2scm("ready")) ) {
		    type->Sound.Ready.Name=gh_scm2newstr(
			gh_car(sublist),NULL);
		    sublist=gh_cdr(sublist);
		} else if( gh_eq_p(value,gh_symbol2scm("help")) ) {
		    type->Sound.Help.Name=gh_scm2newstr(
			gh_car(sublist),NULL);
		    sublist=gh_cdr(sublist);
		} else if( gh_eq_p(value,gh_symbol2scm("dead")) ) {
		    type->Sound.Dead.Name=gh_scm2newstr(
			gh_car(sublist),NULL);
		    sublist=gh_cdr(sublist);
		} else if( gh_eq_p(value,gh_symbol2scm("attack")) ) {
		    type->Weapon.Attack.Name=gh_scm2newstr(
			gh_car(sublist),NULL);
		    sublist=gh_cdr(sublist);
		} else {
		    errl("Unsupported sound tag",value);
		}
	    }
	} else {
	   // FIXME: this leaves a half initialized unit-type
	   errl("Unsupported tag",value);
	}
    }

    return SCM_UNSPECIFIED;
}

/**
**	Parse unit-type.
**
**	@note FIXME: This should be changed to a more readable and useable
**	format. I thinking of an tagged format 'size and this should be
**	parsed by a general parser.
**
**	@param list	List describing the unit-type.
*/
local SCM CclDefineOldUnitType(SCM list)
{
    SCM value;
    SCM temp;
    char* str;
    UnitType* type;
    int i;
    int j;
    int n;

    DebugLevel0Fn("This version will be soon removed.\n");

    //	Slot identifier

    value=gh_car(list);
    str=gh_scm2newstr(value,NULL);
    IfDebug( i=NoWarningUnitType; NoWarningUnitType=1; );
    type=UnitTypeByIdent(str);
    IfDebug( NoWarningUnitType=i; );
    if( type ) {
	DebugLevel0Fn("Redefining unit-type `%s'\n" _C_ str);
	free(str);
    } else {
	type=NewUnitTypeSlot(str);
    }

    //	Name
    list=gh_cdr(list);
    value=gh_car(list);
    str=gh_scm2newstr(value,NULL);
    DebugLevel3("\tName: %s\n" _C_ str);
    free(type->Name);
    type->Name=str;

    //	Graphic

    list=gh_cdr(list);
    value=gh_car(list);
    if( gh_symbol_p(value) || gh_string_p(value) ) {
	str=gh_scm2newstr(value,NULL);
	DebugLevel3("\tSame-Sprite: %s\n" _C_ str);
	free(type->SameSprite);
	type->SameSprite=str;
	free(type->File[0]);
	type->File[0]=NULL;
	free(type->File[1]);
	type->File[1]=NULL;
	free(type->File[2]);
	type->File[2]=NULL;
	free(type->File[3]);
	type->File[3]=NULL;
    } else {
	if( gh_vector_length(value)!=4 ) {
	    fprintf(stderr,"Wrong vector length\n");
	}
	free(type->SameSprite);
	type->SameSprite=NULL;

	temp=gh_vector_ref(value,gh_int2scm(0));
	if( gh_null_p(temp) ) {
	    str=NULL;
	} else {
	    str=gh_scm2newstr(temp,NULL);
	}
	DebugLevel3("\tFile-0: %s\n" _C_ str);
	free(type->File[0]);
	type->File[0]=str;

	temp=gh_vector_ref(value,gh_int2scm(1));
	if( gh_null_p(temp) ) {
	    str=NULL;
	} else {
	    str=gh_scm2newstr(temp,NULL);
	}
	DebugLevel3("\tFile-1: %s\n" _C_ str);
	free(type->File[1]);
	type->File[1]=str;

	temp=gh_vector_ref(value,gh_int2scm(2));
	if( gh_null_p(temp) ) {
	    str=NULL;
	} else {
	    str=gh_scm2newstr(temp,NULL);
	}
	DebugLevel3("\tFile-2: %s\n" _C_ str);
	free(type->File[2]);
	type->File[2]=str;

	temp=gh_vector_ref(value,gh_int2scm(3));
	if( gh_null_p(temp) ) {
	    str=NULL;
	} else {
	    str=gh_scm2newstr(temp,NULL);
	}
	DebugLevel3("\tFile-3: %s\n" _C_ str);
	free(type->File[3]);
	type->File[3]=str;
    }

    // Graphic Size

    list=gh_cdr(list);
    value=gh_car(list);

    temp=gh_car(value);
    type->Width=gh_scm2int(temp);
    temp=gh_car(gh_cdr(value));
    type->Height=gh_scm2int(temp);
    DebugLevel3("\tGraphic: %d,%d\n" _C_ type->Width _C_ type->Height);

    // Animations

    list=gh_cdr(list);
    value=gh_car(list);
    str=gh_scm2newstr(value,NULL);
    type->Animations=AnimationsByIdent(str);
    free(str);

    // Icon

    list=gh_cdr(list);
    value=gh_car(list);
    str=gh_scm2newstr(value,NULL);
    DebugLevel3("\tIcon: %s\n" _C_ str);

    free(type->Icon.Name);
    type->Icon.Name=str;

    // Speed

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tSpeed: %d\n" _C_ i);
    type->_Speed=i;

    // Overlay frame

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tOverlay frame: %d\n" _C_ i);
    type->Construction=ConstructionByWcNum(i);

    // Sight range

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tSight range: %d\n" _C_ i);
    type->_SightRange=i;

    // Hitpoints

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tHitpoints: %d\n" _C_ i);
    type->_HitPoints=i;

    // Magic

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tMagic: %d\n" _C_ i);
    type->Magic=i;

    // Costs

    list=gh_cdr(list);
    value=gh_car(list);
    n=gh_vector_length(value);
    if( n<4 || n>MaxCosts ) {
	fprintf(stderr,"Wrong vector length\n");
	if( n>MaxCosts ) {
	    n=MaxCosts;
	}
    }

    for( j=0; j<n; ++j ) {
	temp=gh_vector_ref(value,gh_int2scm(j));
	i=gh_scm2int(temp);
	type->_Costs[j]=i;
    }
    while( j<MaxCosts ) {
	type->_Costs[j++]=0;
    }

    DebugLevel3("\tCosts: %d,%d,%d,%d\n"
	    _C_ type->_Costs[TimeCost] _C_ type->_Costs[GoldCost]
	    _C_ type->_Costs[WoodCost] _C_ type->_Costs[OilCost]);

    // Tile Size

    list=gh_cdr(list);
    value=gh_car(list);

    temp=gh_car(value);
    type->TileWidth=gh_scm2int(temp);
    temp=gh_car(gh_cdr(value));
    type->TileHeight=gh_scm2int(temp);
    DebugLevel3("\tTile: %d,%d\n" _C_ type->TileWidth _C_ type->TileHeight);

    // Box Size

    list=gh_cdr(list);
    value=gh_car(list);

    temp=gh_car(value);
    type->BoxWidth=gh_scm2int(temp);
    temp=gh_car(gh_cdr(value));
    type->BoxHeight=gh_scm2int(temp);
    DebugLevel3("\tBox: %d,%d\n" _C_ type->BoxWidth _C_ type->BoxHeight);

    // Minimal attack range

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tMinimal AttackRange: %d\n" _C_ i);
    type->MinAttackRange=i;

    // Attack range

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tAttackRange: %d\n" _C_ i);
    type->_AttackRange=i;

    // Reaction range computer

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tReaction range computer: %d\n" _C_ i);
    type->ReactRangeComputer=i;

    // Reaction range player

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tReaction range player: %d\n" _C_ i);
    type->ReactRangePerson=i;

    // Armor

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tArmor: %d\n" _C_ i);
    type->_Armor=i;

    // Priority

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tPriority: %d\n" _C_ i);
    type->Priority=i;

    // Basic damage

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tBasic damage: %d\n" _C_ i);
    type->_BasicDamage=i;

    // Piercing damage

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tPiercing damage: %d\n" _C_ i);
    type->_PiercingDamage=i;

    // Weapons upgradable

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tWeaponsUpgradable: %d\n" _C_ i);
    type->WeaponsUpgradable=i;

    // Armor upgradable

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tArmorUpgradable: %d\n" _C_ i);
    type->ArmorUpgradable=i;

    // Decay rate

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tDecay rate: %d\n" _C_ i);
    type->DecayRate=i;

    // Annoy computer factor

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tAnnoy computer factor: %d\n" _C_ i);
    type->AnnoyComputerFactor=i;

    // Points

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tPoints: %d\n" _C_ i);
    type->Points=i;

    // Food demand

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tFood demand: %d\n" _C_ i);
    type->Demand=i;

    // Food supply

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tFood supply: %d\n" _C_ i);
    type->Supply=i;

    // Missile

    list=gh_cdr(list);
    value=gh_car(list);
    str=gh_scm2newstr(value,NULL);
    DebugLevel3("\tMissile: %s\n" _C_ str);

    free(type->Missile.Name);
    type->Missile.Name=str;

    // Corpse

    list=gh_cdr(list);
    temp=gh_car(list);

    free(type->CorpseName);
    type->CorpseName=NULL;
    type->CorpseType=NULL;
    type->CorpseScript=0;
    if( !gh_null_p(temp) ) {
	value=gh_car(temp);
	temp=gh_cdr(temp);
	str=gh_scm2newstr(value,NULL);
	DebugLevel3("\tCorpse: %s\n" _C_ str);
	type->CorpseName=str;

	value=gh_car(temp);
	i=gh_scm2int(value);
	type->CorpseScript=i;
    }

    // Flags
    type->UnitType=0;			// reset all
    type->MouseAction=0;
    type->CanTarget=0;
    type->LandUnit=0;
    type->AirUnit=0;
    type->SeaUnit=0;
    type->ExplodeWhenKilled=0;
    type->Critter=0;
    type->Building=0;
    type->Submarine=0;
    type->CanSeeSubmarine=0;
    type->CowerWorker=0;
    type->Tanker=0;
    type->Transporter=0;
    type->GivesOil=0;
    type->StoresGold=0;
    type->Vanishes=0;
    type->GroundAttack=0;
    type->IsUndead=0;
    type->ShoreBuilding=0;
    type->CanCastSpell=0;
    type->StoresWood=0;
    type->CanAttack=0;
    type->Tower=0;
    type->OilPatch=0;
    type->GoldMine=0;
    type->Hero=0;
    type->StoresOil=0;
    type->Volatile=0;
    type->CowerMage=0;
    type->Organic=0;
    type->SelectableByRectangle=0;
    type->Teleporter=0;

    while( !gh_null_p(list) ) {
	list=gh_cdr(list);
	value=gh_car(list);
	if( gh_vector_p(value) ) {
	    break;
	}

	if( gh_eq_p(value,gh_symbol2scm("type-land")) ) {
	    type->UnitType=UnitTypeLand;
	} else if( gh_eq_p(value,gh_symbol2scm("type-fly")) ) {
	    type->UnitType=UnitTypeFly;
	} else if( gh_eq_p(value,gh_symbol2scm("type-naval")) ) {
	    type->UnitType=UnitTypeNaval;

	} else if( gh_eq_p(value,gh_symbol2scm("right-none")) ) {
	    type->MouseAction=MouseActionNone;
	} else if( gh_eq_p(value,gh_symbol2scm("right-attack")) ) {
	    type->MouseAction=MouseActionAttack;
	} else if( gh_eq_p(value,gh_symbol2scm("right-move")) ) {
	    type->MouseAction=MouseActionMove;
	} else if( gh_eq_p(value,gh_symbol2scm("right-harvest")) ) {
	    type->MouseAction=MouseActionHarvest;
	} else if( gh_eq_p(value,gh_symbol2scm("right-haul-oil")) ) {
	    type->MouseAction=MouseActionHaulOil;
	} else if( gh_eq_p(value,gh_symbol2scm("right-demolish")) ) {
	    type->MouseAction=MouseActionDemolish;
	} else if( gh_eq_p(value,gh_symbol2scm("right-sail")) ) {
	    type->MouseAction=MouseActionSail;

	} else if( gh_eq_p(value,gh_symbol2scm("can-target-land")) ) {
	    type->CanTarget|=CanTargetLand;
	} else if( gh_eq_p(value,gh_symbol2scm("can-target-sea")) ) {
	    type->CanTarget|=CanTargetSea;
	} else if( gh_eq_p(value,gh_symbol2scm("can-target-air")) ) {
	    type->CanTarget|=CanTargetAir;

	} else if( gh_eq_p(value,gh_symbol2scm("land-unit")) ) {
	    type->LandUnit=1;
	} else if( gh_eq_p(value,gh_symbol2scm("air-unit")) ) {
	    type->AirUnit=1;
	} else if( gh_eq_p(value,gh_symbol2scm("sea-unit")) ) {
	    type->SeaUnit=1;

	} else if( gh_eq_p(value,gh_symbol2scm("explode-when-killed")) ) {
	    type->ExplodeWhenKilled=1;
	} else if( gh_eq_p(value,gh_symbol2scm("critter")) ) {
	    type->Critter=1;
	} else if( gh_eq_p(value,gh_symbol2scm("building")) ) {
	    type->Building=1;
	} else if( gh_eq_p(value,gh_symbol2scm("submarine")) ) {
	    type->Submarine=1;
	} else if( gh_eq_p(value,gh_symbol2scm("can-see-submarine")) ) {
	    type->CanSeeSubmarine=1;
	} else if( gh_eq_p(value,gh_symbol2scm("cower-worker")) ) {
	    type->CowerWorker=1;
	} else if( gh_eq_p(value,gh_symbol2scm("tanker")) ) {
	    type->Tanker=1;
	} else if( gh_eq_p(value,gh_symbol2scm("transporter")) ) {
	    type->Transporter=1;
	} else if( gh_eq_p(value,gh_symbol2scm("gives-oil")) ) {
	    type->GivesOil=1;
	} else if( gh_eq_p(value,gh_symbol2scm("stores-gold")) ) {
	    type->StoresGold=1;
	} else if( gh_eq_p(value,gh_symbol2scm("vanishes")) ) {
	    type->Vanishes=1;
	} else if( gh_eq_p(value,gh_symbol2scm("can-ground-attack")) ) {
	    type->GroundAttack=1;
	} else if( gh_eq_p(value,gh_symbol2scm("isundead")) ) {
	    type->IsUndead=1;
	} else if( gh_eq_p(value,gh_symbol2scm("shore-building")) ) {
	    type->ShoreBuilding=1;
	} else if( gh_eq_p(value,gh_symbol2scm("can-cast-spell")) ) {
	    type->CanCastSpell=1;
	} else if( gh_eq_p(value,gh_symbol2scm("stores-wood")) ) {
	    type->StoresWood=1;
	} else if( gh_eq_p(value,gh_symbol2scm("can-attack")) ) {
	    type->CanAttack=1;
	} else if( gh_eq_p(value,gh_symbol2scm("tower")) ) {
	    type->Tower=1;
	} else if( gh_eq_p(value,gh_symbol2scm("oil-patch")) ) {
	    type->OilPatch=1;
	} else if( gh_eq_p(value,gh_symbol2scm("gives-gold")) ) {
	    type->GoldMine=1;
	} else if( gh_eq_p(value,gh_symbol2scm("hero")) ) {
	    type->Hero=1;
	} else if( gh_eq_p(value,gh_symbol2scm("stores-oil")) ) {
	    type->StoresOil=1;
	} else if( gh_eq_p(value,gh_symbol2scm("volatile")) ) {
	    type->Volatile=1;
	} else if( gh_eq_p(value,gh_symbol2scm("cower-mage")) ) {
	    type->CowerMage=1;
	} else if( gh_eq_p(value,gh_symbol2scm("organic")) ) {
	    type->Organic=1;
	} else if( gh_eq_p(value,gh_symbol2scm("selectable-by-rectangle")) ) {
	    type->SelectableByRectangle=1;
	} else if( gh_eq_p(value,gh_symbol2scm("teleporter")) ) {
	    type->Teleporter=1;

	} else {
	    str=gh_scm2newstr(value,NULL);
	    fprintf(stderr,"Unsupported flag %s\n",str);
	    free(str);
	}
    }

    // Sounds

    if( gh_vector_length(value)!=5 ) {
	fprintf(stderr,"Wrong vector length\n");
    }

    temp=gh_vector_ref(value,gh_int2scm(0));
    if( gh_null_p(temp) ) {
	str=NULL;
    } else {
	str=gh_scm2newstr(temp,NULL);
    }
    free(type->Sound.Selected.Name);
    type->Sound.Selected.Name=str;

    temp=gh_vector_ref(value,gh_int2scm(1));
    if( gh_null_p(temp) ) {
	str=NULL;
    } else {
	str=gh_scm2newstr(temp,NULL);
    }
    free(type->Sound.Acknowledgement.Name);
    type->Sound.Acknowledgement.Name=str;

    temp=gh_vector_ref(value,gh_int2scm(2));
    if( gh_null_p(temp) ) {
	str=NULL;
    } else {
	str=gh_scm2newstr(temp,NULL);
    }
    free(type->Sound.Ready.Name);
    type->Sound.Ready.Name=str;

    temp=gh_vector_ref(value,gh_int2scm(3));
    if( gh_null_p(temp) ) {
	str=NULL;
    } else {
	str=gh_scm2newstr(temp,NULL);
    }
    free(type->Sound.Help.Name);
    type->Sound.Help.Name=str;

    temp=gh_vector_ref(value,gh_int2scm(4));
    if( gh_null_p(temp) ) {
	str=NULL;
    } else {
	str=gh_scm2newstr(temp,NULL);
    }
    free(type->Sound.Dead.Name);
    type->Sound.Dead.Name=str;

    list=gh_cdr(list);
    value=gh_car(list);

    if( gh_null_p(value) ) {
	str=NULL;
    } else {
	str=gh_scm2newstr(value,NULL);
    }
    free(type->Weapon.Attack.Name);
    type->Weapon.Attack.Name=str;

    return SCM_UNSPECIFIED;
}

/**
**	Parse unit-stats.
**
**	@param list	List describing the unit-stats.
*/
local SCM CclDefineUnitStats(SCM list)
{
    SCM value;
    //SCM data;
    SCM sublist;
    UnitType* type;
    UnitStats* stats;
    int i;
    char* str;

    type=UnitTypeByIdent(str=gh_scm2newstr(gh_car(list),NULL));
    free(str);
    list=gh_cdr(list);
    i=gh_scm2int(gh_car(list));
    list=gh_cdr(list);
    stats=&type->Stats[i];

    //
    //	Parse the list:	(still everything could be changed!)
    //
    while( !gh_null_p(list) ) {

	value=gh_car(list);
	list=gh_cdr(list);

	if( gh_eq_p(value,gh_symbol2scm("level")) ) {
	    stats->Level=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("speed")) ) {
	    stats->Speed=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("attack-range")) ) {
	    stats->AttackRange=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("sight-range")) ) {
	    stats->SightRange=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("armor")) ) {
	    stats->Armor=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("basic-damage")) ) {
	    stats->BasicDamage=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("piercing-damage")) ) {
	    stats->PiercingDamage=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("hit-points")) ) {
	    stats->HitPoints=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("costs")) ) {
	    sublist=gh_car(list);
	    list=gh_cdr(list);
	    while( !gh_null_p(sublist) ) {

		value=gh_car(sublist);
		sublist=gh_cdr(sublist);

		for( i=0; i<MaxCosts; ++i ) {
		    if( gh_eq_p(value,gh_symbol2scm((char*)DEFAULT_NAMES[i])) ) {
			stats->Costs[i]=gh_scm2int(gh_car(sublist));
			break;
		    }
		}
		if( i==MaxCosts ) {
		   // FIXME: this leaves half initialized stats
		   errl("Unsupported tag",value);
		}
		sublist=gh_cdr(sublist);
	    }
	} else {
	   // FIXME: this leaves a half initialized unit
	   errl("Unsupported tag",value);
	}
    }

    return SCM_UNSPECIFIED;
}

// ----------------------------------------------------------------------------

/**
**	Access unit-type object
*/
global UnitType* CclGetUnitType(SCM ptr)
{
    const char* str;

    // Be kind allow also strings or symbols
    if( (str=try_get_c_string(ptr)) ) {
	return UnitTypeByIdent(str);
    }
    if( NTYPEP(ptr,SiodUnitTypeTag) ) {
	errl("not an unit-type",ptr);
    }
    return (UnitType*)CAR(ptr);
}

/**
**	Print the unit-type object
**
**	@param ptr	Scheme object.
**	@param f	Output structure.
*/
local void CclUnitTypePrin1(SCM ptr,struct gen_printio* f)
{
    char buf[1024];
    const UnitType* type;

    type=CclGetUnitType(ptr);
    sprintf(buf,"#<UnitType %p %s>",type,type->Ident);
    gput_st(f,buf);
}

/**
**	Get unit-type structure.
**
**	@param ident	Identifier for unit-type.
**
**	@return		Unit-type structure.
*/
local SCM CclUnitType(SCM ident)
{
    const char* str;
    const UnitType* type;
    SCM value;

    str=get_c_string(ident);

    type=UnitTypeByIdent(str);

    value=cons(NIL,NIL);
    value->type=SiodUnitTypeTag;
    CAR(value)=(SCM)type;

    return value;
}

/**
**	Get all unit-type structures.
**
**	@return		An array of all unit-type structures.
*/
local SCM CclUnitTypeArray(void)
{
    SCM array;
    SCM value;
    int i;

    array=cons_array(flocons(UnitTypeMax),NIL);

    for( i=0; i<UnitTypeMax; ++i ) {
	value=cons(NIL,NIL);
	value->type=SiodUnitTypeTag;
	CAR(value)=(SCM)&UnitTypes[i];
	array->storage_as.lisp_array.data[i]=value;
    }
    return array;
}

/**
**	Get the ident of the unit-type structure.
**
**	@param ptr	Unit-type object.
**
**	@return		The identifier of the unit-type.
*/
local SCM CclGetUnitTypeIdent(SCM ptr)
{
    const UnitType* type;
    SCM value;

    type=CclGetUnitType(ptr);
    value=gh_str02scm(type->Ident);
    return value;
}

/**
**	Get the name of the unit-type structure.
**
**	@param ptr	Unit-type object.
**
**	@return		The name of the unit-type.
*/
local SCM CclGetUnitTypeName(SCM ptr)
{
    const UnitType* type;
    SCM value;

    type=CclGetUnitType(ptr);
    value=gh_str02scm(type->Name);
    return value;
}

/**
**	Set the name of the unit-type structure.
**
**	@param ptr	Unit-type object.
**	@param name	The name to set.
**
**	@return		The name of the unit-type.
*/
local SCM CclSetUnitTypeName(SCM ptr,SCM name)
{
    UnitType* type;

    type=CclGetUnitType(ptr);
    free(type->Name);
    type->Name=gh_scm2newstr(name,NULL);

    return name;
}

// FIXME: write the missing access functions

/**
**	Get the property of the unit-type structure.
**
**	@param ptr	Unit-type object.
**
**	@return		The property of the unit-type.
*/
local SCM CclGetUnitTypeProperty(SCM ptr)
{
    const UnitType* type;

    type=CclGetUnitType(ptr);
    return type->Property;
}

/**
**	Set the property of the unit-type structure.
**
**	@param ptr	Unit-type object.
**	@param property	The property to set.
**
**	@return		The property of the unit-type.
*/
local SCM CclSetUnitTypeProperty(SCM ptr,SCM property)
{
    UnitType* type;

    type=CclGetUnitType(ptr);

    if( type->Property ) {
	// FIXME: old value must be unprotected!!
    }
    if( !property ) {
	DebugLevel0Fn("oops, my fault\n");
    }
    type->Property=property;
    CclGcProtect(type->Property);

    return property;
}

/**
**	Define tileset mapping from original number to internal symbol
**
**	@param list	List of all names.
*/
local SCM CclDefineUnitTypeWcNames(SCM list)
{
    int i;
    char** cp;

    if( (cp=UnitTypeWcNames) ) {		// Free all old names
	while( *cp ) {
	    free(*cp++);
	}
	free(UnitTypeWcNames);
    }

    //
    //	Get new table.
    //
    i=gh_length(list);
    UnitTypeWcNames=cp=malloc((i+1)*sizeof(char*));
    while( i-- ) {
	*cp++=gh_scm2newstr(gh_car(list),NULL);
	list=gh_cdr(list);
    }
    *cp=NULL;

    return SCM_UNSPECIFIED;
}

// ----------------------------------------------------------------------------

/**
**	Define an unit-type animations set.
**
**	@param list	Animations list.
*/
local SCM CclDefineAnimations(SCM list)
{
    char* str;
    SCM id;
    SCM value;
    Animations* anims;
    Animation* anim;
    Animation* t;
    int i;
    int frame;

    str=gh_scm2newstr(gh_car(list),NULL);
    list=gh_cdr(list);
    anims=calloc(1,sizeof(Animations));

    while( !gh_null_p(list) ) {
	id=gh_car(list);
	list=gh_cdr(list);
	value=gh_car(list);
	list=gh_cdr(list);

	t=anim=malloc(gh_length(value)*sizeof(Animation));
	frame=0;
	while( !gh_null_p(value) ) {
	    t->Flags=gh_scm2int(gh_vector_ref(gh_car(value),gh_int2scm(0)));
	    t->Pixel=gh_scm2int(gh_vector_ref(gh_car(value),gh_int2scm(1)));
	    t->Sleep=gh_scm2int(gh_vector_ref(gh_car(value),gh_int2scm(2)));
	    i=gh_scm2int(gh_vector_ref(gh_car(value),gh_int2scm(3)));
	    t->Frame=i-frame;
	    frame=i;
	    if( t->Flags&AnimationRestart ) {
		frame=0;
	    }
	    ++t;
	    value=gh_cdr(value);
	}
	t[-1].Flags|=0x80;		// Marks end of list

	if( gh_eq_p(id,gh_symbol2scm("still")) ) {
	    if( anims->Still ) {
		free(anims->Still);
	    }
	    anims->Still=anim;
	} else if( gh_eq_p(id,gh_symbol2scm("move")) ) {
	    if( anims->Move ) {
		free(anims->Move);
	    }
	    anims->Move=anim;
	} else if( gh_eq_p(id,gh_symbol2scm("attack")) ) {
	    if( anims->Attack ) {
		free(anims->Attack);
	    }
	    anims->Attack=anim;
	} else if( gh_eq_p(id,gh_symbol2scm("die")) ) {
	    if( anims->Die ) {
		free(anims->Die);
	    }
	    anims->Die=anim;
	} else {
	    errl("Unsupported tag",id);
	}
    }

    // I generate a scheme variable containing the pointer!
    gh_define(str,gh_int2scm((int)anims));

    return SCM_UNSPECIFIED;
}

// ----------------------------------------------------------------------------

/**
**	Register CCL features for unit-type.
*/
global void UnitTypeCclRegister(void)
{
    //gh_new_procedureN("define-old-unit-type",CclDefineOldUnitType);
    gh_new_procedureN("define-new-unit-type",CclDefineNewUnitType);
    gh_new_procedureN("define-unit-stats",CclDefineUnitStats);

    SiodUnitTypeTag=allocate_user_tc();
    set_print_hooks(SiodUnitTypeTag,CclUnitTypePrin1);

    gh_new_procedure1_0("unit-type",CclUnitType);
    gh_new_procedure0_0("unit-type-array",CclUnitTypeArray);
    // unit type structure access
    gh_new_procedure1_0("get-unit-type-ident",CclGetUnitTypeIdent);
    gh_new_procedure1_0("get-unit-type-name",CclGetUnitTypeName);
    gh_new_procedure2_0("set-unit-type-name!",CclSetUnitTypeName);

    // FIXME: write the missing access functions

    gh_new_procedure1_0("get-unit-type-property",CclGetUnitTypeProperty);
    gh_new_procedure2_0("set-unit-type-property!",CclSetUnitTypeProperty);

    gh_new_procedureN("define-unittype-wc-names",CclDefineUnitTypeWcNames);

    gh_new_procedureN("define-animations",CclDefineAnimations);
}

//@}
