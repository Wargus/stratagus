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
/*
**	(c) Copyright 1999,2000 by Lutz Sammer
**
**	$Id$
*/

//@{

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"

#if defined(USE_CCL) || defined(USE_CCL2) // {

#include "video.h"
#include "tileset.h"
#include "map.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "icons.h"
#include "missile.h"
#include "ccl.h"

#ifdef USE_CCL2

local long SiodUnitTypeTag;		/// siod unit type object

#endif

/**
**	Parse unit-type.
**
**	@param list	List describing unit type.
*/
local SCM CclDefineUnitType(SCM list)
{
    SCM value;
    SCM temp;
    char* str;
    UnitType* type;
    int i;
    int j;
    int n;
    static int hack_index;

    //	Slot identifier

    value=gh_car(list);
    str=gh_scm2newstr(value,NULL);
    DebugLevel3("UnitType: %s\n",str);
    type=UnitTypeByIdent(str);
    if( !type ) {
	// FIXME: must generate a new unit type
	type=&UnitTypes[hack_index++];
    }

    //	Name

    list=gh_cdr(list);
    value=gh_car(list);
    str=gh_scm2newstr(value,NULL);
    DebugLevel3("\tName: %s\n",str);
    CclFree(type->Name);
    type->Name=str;

    //	Graphic

    list=gh_cdr(list);
    value=gh_car(list);
    if( gh_symbol_p(value) || gh_string_p(value) ) {
	str=gh_scm2newstr(value,NULL);
	DebugLevel3("\tSame-Sprite: %s\n",str);
	CclFree(type->SameSprite);
	type->SameSprite=str;
	CclFree(type->File[0]);
	type->File[0]=NULL;
	CclFree(type->File[1]);
	type->File[1]=NULL;
	CclFree(type->File[2]);
	type->File[2]=NULL;
	CclFree(type->File[3]);
	type->File[3]=NULL;
    } else {
	if( gh_vector_length(value)!=4 ) {
	    fprintf(stderr,"Wrong vector length\n");
	}
	CclFree(type->SameSprite);
	type->SameSprite=NULL;

	temp=gh_vector_ref(value,gh_int2scm(0));
	if( gh_null_p(temp) ) {
	    str=NULL;
	} else {
	    str=gh_scm2newstr(temp,NULL);
	}
	DebugLevel3("\tFile-0: %s\n",str);
	CclFree(type->File[0]);
	type->File[0]=str;

	temp=gh_vector_ref(value,gh_int2scm(1));
	if( gh_null_p(temp) ) {
	    str=NULL;
	} else {
	    str=gh_scm2newstr(temp,NULL);
	}
	DebugLevel3("\tFile-1: %s\n",str);
	CclFree(type->File[1]);
	type->File[1]=str;

	temp=gh_vector_ref(value,gh_int2scm(2));
	if( gh_null_p(temp) ) {
	    str=NULL;
	} else {
	    str=gh_scm2newstr(temp,NULL);
	}
	DebugLevel3("\tFile-2: %s\n",str);
	CclFree(type->File[2]);
	type->File[2]=str;

	temp=gh_vector_ref(value,gh_int2scm(3));
	if( gh_null_p(temp) ) {
	    str=NULL;
	} else {
	    str=gh_scm2newstr(temp,NULL);
	}
	DebugLevel3("\tFile-3: %s\n",str);
	CclFree(type->File[3]);
	type->File[3]=str;
    }

    // Graphic Size

    list=gh_cdr(list);
    value=gh_car(list);

    temp=gh_car(value);
    type->Width=gh_scm2int(temp);
    temp=gh_car(gh_cdr(value));
    type->Height=gh_scm2int(temp);
    DebugLevel3("\tGraphic: %d,%d\n",type->Width,type->Height);

    // Animations

    list=gh_cdr(list);
    value=gh_car(list);
    str=gh_scm2newstr(value,NULL);
    DebugLevel3("\tAnimation: %s\n",str);
    free(str);

    // FIXME: animation parsing not written

    // Icon

    list=gh_cdr(list);
    value=gh_car(list);
    str=gh_scm2newstr(value,NULL);
    DebugLevel3("\tIcon: %s\n",str);

    CclFree(type->Icon.Name);
    type->Icon.Name=str;

    // Speed

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tSpeed: %d\n",i);
    type->_Speed=i;

    // Overlay frame

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tOverlay frame: %d\n",i);
    type->OverlapFrame=i;

    // Sight range

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tSight range: %d\n",i);
    type->_SightRange=i;

    // Hitpoints

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tHitpoints: %d\n",i);
    type->_HitPoints=i;

    // Magic

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tMagic: %d\n",i);
    type->Magic=i;

    // Costs

    list=gh_cdr(list);
    value=gh_car(list);
    n=gh_vector_length(value);
    if( n<4 || n>6 ) {
	fprintf(stderr,"Wrong vector length\n");
	if( n>6 ) {
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
	    ,type->_Costs[TimeCost],type->_Costs[GoldCost]
	    ,type->_Costs[WoodCost],type->_Costs[OilCost]);

    // Tile Size

    list=gh_cdr(list);
    value=gh_car(list);

    temp=gh_car(value);
    type->TileWidth=gh_scm2int(temp);
    temp=gh_car(gh_cdr(value));
    type->TileHeight=gh_scm2int(temp);
    DebugLevel3("\tTile: %d,%d\n",type->TileWidth,type->TileHeight);

    // Box Size

    list=gh_cdr(list);
    value=gh_car(list);

    temp=gh_car(value);
    type->BoxWidth=gh_scm2int(temp);
    temp=gh_car(gh_cdr(value));
    type->BoxHeight=gh_scm2int(temp);
    DebugLevel3("\tBox: %d,%d\n",type->BoxWidth,type->BoxHeight);

    // Attack range

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tAttackRange: %d\n",i);
    type->_AttackRange=i;

    // Reaction range computer

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tReaction range computer: %d\n",i);
    type->ReactRangeComputer=i;

    // Reaction range player

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tReaction range player: %d\n",i);
    type->ReactRangeHuman=i;

    // Armor

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tArmor: %d\n",i);
    type->_Armor=i;

    // Priority

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tPriority: %d\n",i);
    type->Priority=i;

    // Basic damage

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tBasic damage: %d\n",i);
    type->_BasicDamage=i;

    // Piercing damage

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tPiercing damage: %d\n",i);
    type->_PiercingDamage=i;

    // Weapons upgradable

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tWeaponsUpgradable: %d\n",i);
    type->WeaponsUpgradable=i;

    // Armor upgradable

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tArmorUpgradable: %d\n",i);
    type->ArmorUpgradable=i;

    // Decay rate

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tDecay rate: %d\n",i);
    type->DecayRate=i;

    // Annoy computer factor

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tAnnoy computer factor: %d\n",i);
    type->AnnoyComputerFactor=i;

    // Points

    list=gh_cdr(list);
    value=gh_car(list);
    i=gh_scm2int(value);
    DebugLevel3("\tPoints: %d\n",i);
    type->Points=i;

    // Missile

    list=gh_cdr(list);
    value=gh_car(list);
    str=gh_scm2newstr(value,NULL);
    DebugLevel3("\tMissile: %s\n",str);

    CclFree(type->Missile.Name);
    type->Missile.Name=str;

    // Flags

    type->Corpse=0;			// reset all
    type->UnitType=0;
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
    type->CowerPeon=0;
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
    type->Explodes=0;
    type->CowerMage=0;
    type->Organic=0;
    type->SelectableByRectangle=0;

    while( !gh_null_p(list) ) {
	list=gh_cdr(list);
	value=gh_car(list);
	if( gh_vector_p(value) ) {
	    break;
	}

	// FIXME: slow! any better idea?
	if( gh_eq_p(value,gh_symbol2scm("corpse-none")) ) {
	    type->Corpse=CorpseNone;
	} else if( gh_eq_p(value,gh_symbol2scm("corpse-human")) ) {
	    type->Corpse=CorpseHuman;
	} else if( gh_eq_p(value,gh_symbol2scm("corpse-orc")) ) {
	    type->Corpse=CorpseOrc;
	} else if( gh_eq_p(value,gh_symbol2scm("corpse-ship")) ) {
	    type->Corpse=CorpseShip;
	} else if( gh_eq_p(value,gh_symbol2scm("corpse-land-site")) ) {
	    type->Corpse=CorpseLandSite;
	} else if( gh_eq_p(value,gh_symbol2scm("corpse-water-site")) ) {
	    type->Corpse=CorpseWaterSite;

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
	    type->CanSeeSubmarine=0;
	} else if( gh_eq_p(value,gh_symbol2scm("cower-peon")) ) {
	    type->CowerPeon=1;
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
	    type->Explodes=1;
	} else if( gh_eq_p(value,gh_symbol2scm("cower-mage")) ) {
	    type->CowerMage=1;
	} else if( gh_eq_p(value,gh_symbol2scm("organic")) ) {
	    type->Organic=1;
	} else if( gh_eq_p(value,gh_symbol2scm("selectable-by-rectangle")) ) {
	    type->SelectableByRectangle=1;

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
    CclFree(type->Sound.Selected.Name);
    type->Sound.Selected.Name=str;

    temp=gh_vector_ref(value,gh_int2scm(1));
    if( gh_null_p(temp) ) {
	str=NULL;
    } else {
	str=gh_scm2newstr(temp,NULL);
    }
    CclFree(type->Sound.Acknowledgement.Name);
    type->Sound.Acknowledgement.Name=str;

    temp=gh_vector_ref(value,gh_int2scm(2));
    if( gh_null_p(temp) ) {
	str=NULL;
    } else {
	str=gh_scm2newstr(temp,NULL);
    }
    CclFree(type->Sound.Ready.Name);
    type->Sound.Ready.Name=str;

    temp=gh_vector_ref(value,gh_int2scm(3));
    if( gh_null_p(temp) ) {
	str=NULL;
    } else {
	str=gh_scm2newstr(temp,NULL);
    }
    CclFree(type->Sound.Help.Name);
    type->Sound.Help.Name=str;

    temp=gh_vector_ref(value,gh_int2scm(4));
    if( gh_null_p(temp) ) {
	str=NULL;
    } else {
	str=gh_scm2newstr(temp,NULL);
    }
    CclFree(type->Sound.Dead.Name);
    type->Sound.Dead.Name=str;

    list=gh_cdr(list);
    value=gh_car(list);

    if( gh_null_p(value) ) {
	str=NULL;
    } else {
	str=gh_scm2newstr(value,NULL);
    }
    CclFree(type->Weapon.Attack.Name);
    type->Weapon.Attack.Name=str;

    return list;
}

#ifdef DEBUG

/**
**	Generate C - table for UnitTypes.
*/
local SCM CclPrintUnitTypeTable(void)
{
    PrintUnitTypeTable();

    return SCM_UNSPECIFIED;
}

#endif

#ifdef USE_CCL2

/** 
 ** Get a single animation sequence.
 */
local Animation * GetSingleAnimation(SCM list){
  SCM partlist;
  Animation * finallist;
  size_t length;
  int current_location = 0;

  if(!gh_list_p(list))
    return NULL;

  //printf("list %p %d",list,gh_list_p(list));
  length = gh_length(list);

  finallist = (Animation *)calloc(sizeof(Animation),length);
  while(list){
    SCM value;
    Animation anim;
    int Flags,Pixel,Sleep,Frame;
    partlist = gh_car(list);
    //printf("lenpart %ld, lenlist %ld\n",gh_length(partlist),gh_length(list));
    list = gh_cdr(list);
    value = gh_car(partlist);

    Flags = gh_scm2int(value);
    partlist = gh_cdr(partlist);
    value = gh_car(partlist);
    
    Pixel = gh_scm2int(value);
    partlist = gh_cdr(partlist);
    value = gh_car(partlist);

    Sleep = gh_scm2int(value);
    partlist = gh_cdr(partlist);
    value = gh_car(partlist);
    
    Frame = gh_scm2int(value);
    //printf("(F%d,P%d,S%d,F%d)",Flags,Pixel,Sleep,Frame);
    anim.Flags = Flags;
    anim.Pixel = Pixel;
    anim.Sleep = Sleep;
    anim.Frame = Frame;
    finallist[current_location++] = anim;
  }
  //printf("\n");
  
  return finallist;

}

global Animation ** UnitDie;
global Animation ** UnitCorpse;
global Animation ** UnitStillAnimation;

/**
 ** Get animation data
 */
local SCM CclAnimType(SCM list)
{
  int i;
  UnitType* unittype;

  list = gh_car(list);
  {
    int length = gh_length(list)-1;
    Animation ** whole_animation = 
      (Animation **)calloc(sizeof(Animation*),length);
    int type = gh_scm2int(gh_car(list));
    int cur = 0;

    //printf("length %d,type %d\n",length,type);

    list = gh_cdr(list);
    while(list){
      //printf("%ld ",gh_length(gh_car(list)));
      whole_animation[cur++] = GetSingleAnimation(gh_car(list));
      list = gh_cdr(list);
      
    }
    
    switch( type ){
    case 0:
      UnitStillAnimation = whole_animation;
      for( i=0; i<UnitTypeInternalMax; ++i ) {
	unittype=UnitTypeByWcNum(i);
	if( !unittype->Animations ) {
	    unittype->Animations=calloc(sizeof(*unittype->Animations),1);
	}
	CclFree(unittype->Animations->Still);
        unittype->Animations->Still=whole_animation[i];
      }
      DebugLevel2("Loading UnitStillAnimation\n");
      break;
    case 1:
      for( i=0; i<UnitTypeInternalMax; ++i ) {
	unittype=UnitTypeByWcNum(i);
	if( !unittype->Animations ) {
	    unittype->Animations=calloc(sizeof(*unittype->Animations),1);
	}
	CclFree(unittype->Animations->Move);
	unittype->Animations->Move=whole_animation[i];
      }
      DebugLevel2("Loading UnitMoveAnimation\n");
      break;
    case 2:
      UnitDie = whole_animation;
      for( i=0; i<UnitTypeInternalMax; ++i ) {
	unittype=UnitTypeByWcNum(i);
	if( !unittype->Animations ) {
	    unittype->Animations=calloc(sizeof(*unittype->Animations),1);
	}
	CclFree(unittype->Animations->Die);
	unittype->Animations->Die=whole_animation[i];
      }
      DebugLevel2("Loading UnitDie\n");
      break;
    case 3:
      UnitCorpse = whole_animation;
      for( i=0; i<UnitTypeInternalMax; ++i ) {
	unittype=UnitTypeByWcNum(i);
	if( !unittype->Animations ) {
	    unittype->Animations=calloc(sizeof(*unittype->Animations),1);
	}
	// FIXME: corpse
      }
      DebugLevel2("Loading UnitCorpse\n");
      break;
    case 4:
      for( i=0; i<UnitTypeInternalMax; ++i ) {
	unittype=UnitTypeByWcNum(i);
	if( !unittype->Animations ) {
	    unittype->Animations=calloc(sizeof(*unittype->Animations),1);
	}
	CclFree(unittype->Animations->Attack);
	unittype->Animations->Attack=whole_animation[i];
      }
      DebugLevel2("Loading UnitAttack\n");
      break;
    default:
      DebugLevel1(__FUNCTION__":Unknown unit-type %d\n",type);
      break;
    }
    return list;

  }
}

/**
**	Access unit type object
*/
local UnitType* CclGetUnitType(SCM ptr)
{
    const char* str;

    // Be kind allow also strings or symbols
    if( (str=try_get_c_string(ptr)) ) {
	return UnitTypeByIdent(str);
    }
    if( NTYPEP(ptr,SiodUnitTypeTag) ) {
	err("not an unit type",ptr);
    }
    return (UnitType*)CAR(ptr);
}

/**
**	Print the unit type object
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
**	Get unit type structure.
**
**	@param ident	Identifier for unit type.
**
**	@returns	Unit type structure.
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
**	Get all unit type structures.
**
**	@returns	An array of all unit type structures.
*/
local SCM CclUnitTypeArray(void)
{
    SCM array;
    SCM value;
    int i;

    array=cons_array(flocons(UnitTypeInternalMax),NIL);

    for( i=0; i<UnitTypeInternalMax; ++i ) {
	value=cons(NIL,NIL);
	value->type=SiodUnitTypeTag;
	CAR(value)=(SCM)&UnitTypes[i];
	array->storage_as.lisp_array.data[i]=value;
    }
    return array;
}

/**
**	Get the ident of the unit type structure.
**
**	@param ptr	Unit type object.
**
**	@returns	The identifier of the unit type.
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
**	Get the name of the unit type structure.
**
**	@param ptr	Unit type object.
**
**	@returns	The name of the unit type.
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
**	Set the name of the unit type structure.
**
**	@param ptr	Unit type object.
**	@param name	The name to set.
**
**	@returns	The name of the unit type.
*/
local SCM CclSetUnitTypeName(SCM ptr,SCM name)
{
    UnitType* type;

    type=CclGetUnitType(ptr);
    CclFree(type->Name);
    type->Name=gh_scm2newstr(name,NULL);

    return name;
}

// FIXME: write the missing access functions

/**
**	Get the property of the unit type structure.
**
**	@param ptr	Unit type object.
**
**	@returns	The property of the unit type.
*/
local SCM CclGetUnitTypeProperty(SCM ptr)
{
    const UnitType* type;

    type=CclGetUnitType(ptr);
    return type->Property;
}

/**
**	Set the property of the unit type structure.
**
**	@param ptr	Unit type object.
**	@param name	The property to set.
**
**	@returns	The property of the unit type.
*/
local SCM CclSetUnitTypeProperty(SCM ptr,SCM property)
{
    UnitType* type;

    type=CclGetUnitType(ptr);

    if( !type->Property ) {		// first time use, protect for gc.
	gc_protect(type->Property);
    }
    if( !property ) {
	DebugLevel0(__FUNCTION__": oops, my fault\n");
    }
    type->Property=property;

    return property;
}

#endif

/**
**	Register CCL features for unit-type.
*/
global void UnitTypeCclRegister(void)
{
    gh_new_procedureN("define-unit-type",CclDefineUnitType);
#ifdef DEBUG
    gh_new_procedure0_0("print-unit-type-table",CclPrintUnitTypeTable);
#endif

#ifdef USE_CCL2
    SiodUnitTypeTag=allocate_user_tc();
    set_print_hooks(SiodUnitTypeTag,CclUnitTypePrin1);

    gh_new_procedure1_0("unit-type",CclUnitType);
    gh_new_procedure0_0("unit-type-array",CclUnitTypeArray);
    // unit structure access
    gh_new_procedure1_0("get-unit-type-ident",CclGetUnitTypeIdent);
    gh_new_procedure1_0("get-unit-type-name",CclGetUnitTypeName);
    gh_new_procedure2_0("set-unit-type-name!",CclSetUnitTypeName);

    // FIXME: write the missing access functions

    gh_new_procedure1_0("get-unit-type-property",CclGetUnitTypeProperty);
    gh_new_procedure2_0("set-unit-type-property!",CclSetUnitTypeProperty);

    gh_new_procedureN("anim-type",CclAnimType);
#endif
    InitUnitTypes();
}

#endif	// } USE_CCL && USE_CCL2

//@}
