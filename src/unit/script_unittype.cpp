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
local SCM CclDefineUnitType(SCM list)
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
	// FIXME: lose memory, old content isn't freed.
    } else {
	type=NewUnitTypeSlot(str);
    }

    type->NumDirections=8;
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

		// FIXME: use a general get tileset function here!
		i=0;
		if( !gh_eq_p(value,gh_symbol2scm("default")) ) {
		    for( ; i<TilesetMax; ++i ) {
			if( gh_eq_p(value,gh_symbol2scm(Tilesets[i]->Ident)) ||
				gh_eq_p(value,
				    gh_symbol2scm(Tilesets[i]->Class)) ) {
			    break;
			}
		    }
		    if( i==TilesetMax ) {
		       // FIXME: this leaves half initialized unit-type
		       errl("Unsupported tileset tag",value);
		    }
		}
		type->File[i]=gh_scm2newstr(gh_car(sublist),NULL);
		sublist=gh_cdr(sublist);
	    }
	} else if( gh_eq_p(value,gh_symbol2scm("shadow")) ) {
	    sublist=gh_car(list);
	    list=gh_cdr(list);
	    while( !gh_null_p(sublist) ) {
		value=gh_car(sublist);
		sublist=gh_cdr(sublist);

		if( gh_eq_p(value,gh_symbol2scm("file")) ) {
		    type->ShadowFile=gh_scm2newstr(gh_car(sublist),NULL);
		} else if( gh_eq_p(value,gh_symbol2scm("width")) ) {
		    type->ShadowWidth=gh_scm2int(gh_car(sublist));
		} else if( gh_eq_p(value,gh_symbol2scm("height")) ) {
		    type->ShadowHeight=gh_scm2int(gh_car(sublist));
		} else {
		    errl("Unsupported shadow tag",value);
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
		    if( gh_eq_p(value,gh_symbol2scm(DefaultResourceNames[i])) ) {
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
	} else if( gh_eq_p(value,gh_symbol2scm("max-mana")) ) {
	    type->_MaxMana=gh_scm2int(gh_car(list));
	    if( type->_MaxMana>MaxMana ) {
		DebugLevel0Fn("Too much mana %d\n" _C_ type->_MaxMana);
		type->_MaxMana=MaxMana;
	    }
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
	} else if( gh_eq_p(value,gh_symbol2scm("num-directions")) ) {
	    type->NumDirections=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
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

    //
    //	Unit type checks.
    //
    if( type->CanCastSpell && !type->_MaxMana ) {
	DebugLevel0Fn("%s: Need max mana value\n" _C_ type->Ident);
	type->_MaxMana=1;
    }

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
		    if( gh_eq_p(value,gh_symbol2scm(DefaultResourceNames[i])) ) {
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
    free(str);

    return SCM_UNSPECIFIED;
}

// ----------------------------------------------------------------------------

/**
**	Register CCL features for unit-type.
*/
global void UnitTypeCclRegister(void)
{
    gh_new_procedureN("define-unit-type",CclDefineUnitType);
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
