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
/**@name ccl_unittype.c	-	The unit-type ccl functions. */
//
//	(c) Copyright 1999-2003 by Lutz Sammer and Jimmy Salmon
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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
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
#include "spells.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

#ifdef DEBUG
extern int NoWarningUnitType;		/// quiet ident lookup.
#endif

global _AnimationsHash AnimationsHash;	/// Animations hash table

#if defined(USE_GUILE) || defined(USE_SIOD)
local ccl_smob_type_t SiodUnitTypeTag;		/// siod unit-type object
#elif defined(USE_LUA)
#endif

global char **BoolFlagName = NULL;	/// Name of user defined flag
global int NumberBoolFlag = 0;		/// Number of defined flags.

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
** 	Get the resource ID from a SCM object.
**
** 	@param value	SCM thingie
**	@return 	the resource id
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
global unsigned CclGetResourceByName(SCM value)
{
    int i;

    for (i = 0; i < MaxCosts; ++i) {
	if (gh_eq_p(value, gh_symbol2scm(DefaultResourceNames[i]))) {
	    return i;
	}
    }
    errl("Unsupported resource tag", value);
    return 0xABCDEF;
}
#elif defined(USE_LUA)
global unsigned CclGetResourceByName(lua_State* l)
{
    int i;
    const char* value;

    if (!lua_isstring(l, -1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    value = lua_tostring(l, -1);
    for (i = 0; i < MaxCosts; ++i) {
	if (!strcmp(value, DefaultResourceNames[i])) {
	    return i;
	}
    }
    lua_pushfstring(l, "Unsupported resource tag: %s", value);
    lua_error(l);
    return 0xABCDEF;
}
#endif

/**
**	Parse unit-type.
**
**	@note Should write a general parser for this.
**
**	@param list	List describing the unit-type.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineUnitType(SCM list)
{
    SCM value;
    SCM sublist;
    UnitType* type;
    UnitType* auxtype;
    ResourceInfo* res;
    char* str;
    int i;
    int redefine;

    //	Slot identifier

    str = gh_scm2newstr(gh_car(list), NULL);
    list = gh_cdr(list);

#ifdef DEBUG
    i = NoWarningUnitType;
    NoWarningUnitType = 1;
#endif
    type = UnitTypeByIdent(str);
#ifdef DEBUG
    NoWarningUnitType = i;
#endif
    if (type) {
	DebugLevel3Fn("Redefining unit-type `%s'\n" _C_ str);
	free(str);
	redefine = 1;
    } else {
	DebugLevel3Fn("Defining unit-type `%s'\n" _C_ str);
	type = NewUnitTypeSlot(str);
	redefine = 0;
	//Set some default values
	type->_RegenerationRate = 0;
	type->Selectable = 1;
    }
    type->BoolFlag = realloc(type->BoolFlag, NumberBoolFlag * sizeof (*type->BoolFlag));
    memset(type->BoolFlag, 0, NumberBoolFlag * sizeof (*type->BoolFlag));
    type->CanTargetFlag = realloc(type->CanTargetFlag, NumberBoolFlag * sizeof (*type->CanTargetFlag));
    memset(type->CanTargetFlag, 0, NumberBoolFlag * sizeof (*type->CanTargetFlag));

    type->NumDirections = 8;

    type->Property = SCM_UNSPECIFIED;

    //
    //	Parse the list:	(still everything could be changed!)
    //
    while (!gh_null_p(list)) {

	value = gh_car(list);
	list = gh_cdr(list);

	if (gh_eq_p(value, gh_symbol2scm("name"))) {
	    if (redefine) {
		free(type->Name);
	    }
	    type->Name = gh_scm2newstr(gh_car(list), NULL);
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("use"))) {
	    if (redefine) {
		free(type->SameSprite);
	    }
	    type->SameSprite = gh_scm2newstr(gh_car(list), NULL);
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("files"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		char* str;

		value = gh_car(sublist);
		sublist = gh_cdr(sublist);

		// FIXME: use a general get tileset function here!
		str = gh_scm2newstr(value, NULL);
		i = 0;
		if (strcmp(str, "default")) {
		    for (; i < NumTilesets; ++i) {
			if (!strcmp(str, Tilesets[i]->Ident) ||
				!strcmp(str, Tilesets[i]->Class)) {
			    break;
			}
		    }
		    if (i == NumTilesets) {
		       // FIXME: this leaves half initialized unit-type
		       errl("Unsupported tileset tag", value);
		    }
		}
		free(str);
		if (redefine) {
		    free(type->File[i]);
		}
		type->File[i] = gh_scm2newstr(gh_car(sublist), NULL);
		sublist = gh_cdr(sublist);
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("shadow"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);

		if (gh_eq_p(value, gh_symbol2scm("file"))) {
		    if (redefine) {
			free(type->ShadowFile);
		    }
		    type->ShadowFile = gh_scm2newstr(gh_car(sublist), NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("size"))) {
		    type->ShadowWidth = gh_scm2int(gh_car(gh_car(sublist)));
		    type->ShadowHeight = gh_scm2int(gh_car(gh_cdr(gh_car(sublist))));
		} else if (gh_eq_p(value, gh_symbol2scm("height"))) {
		} else if (gh_eq_p(value, gh_symbol2scm("offset"))) {
		    type->ShadowOffsetX = gh_scm2int(gh_car(gh_car(sublist)));
		    type->ShadowOffsetY = gh_scm2int(gh_car(gh_cdr(gh_car(sublist))));
		} else {
		    errl("Unsupported shadow tag", value);
		}
		sublist = gh_cdr(sublist);
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("size"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    type->Width = gh_scm2int(gh_car(sublist));
	    type->Height = gh_scm2int(gh_cadr(sublist));
	} else if (gh_eq_p(value, gh_symbol2scm("animations"))) {
	    type->Animations =
		AnimationsByIdent(str = gh_scm2newstr(gh_car(list), NULL));
	    free(str);
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("icon"))) {
	    if (redefine) {
		free(type->Icon.Name);
	    }
	    type->Icon.Name = gh_scm2newstr(gh_car(list), NULL);
	    type->Icon.Icon = NULL;
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("costs"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		type->_Costs[CclGetResourceByName(value)] = gh_scm2int(gh_car(sublist));
		sublist = gh_cdr(sublist);
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("improve-production"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		type->ImproveIncomes[CclGetResourceByName(value)] =
			DefaultIncomes[CclGetResourceByName(value)] + gh_scm2int(gh_car(sublist));
		sublist = gh_cdr(sublist);
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("construction"))) {
	    // FIXME: What if constructions aren't yet loaded?
	    str = gh_scm2newstr(gh_car(list), NULL);
	    type->Construction = ConstructionByIdent(str);
	    list = gh_cdr(list);
	    free(str);
	} else if (gh_eq_p(value, gh_symbol2scm("speed"))) {
	    type->_Speed = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("draw-level"))) {
	    type->DrawLevel = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("max-on-board"))) {
	    type->MaxOnBoard = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("hit-points"))) {
	    type->_HitPoints = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("regeneration-rate"))) {
	    type->_RegenerationRate = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("burn-percent"))) {
	    type->BurnPercent = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("burn-damage-rate"))) {
	    type->BurnDamageRate = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("max-mana"))) {
	    type->_MaxMana = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("tile-size"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    type->TileWidth = gh_scm2int(gh_car(sublist));
	    type->TileHeight = gh_scm2int(gh_cadr(sublist));
	} else if (gh_eq_p(value, gh_symbol2scm("must-build-on-top"))) {
	    str = gh_scm2newstr(gh_car(list), NULL);
	    auxtype = UnitTypeByIdent(str);
	    if (!auxtype) {
		DebugLevel0("Build on top of undefined unit \"%s\".\n" _C_ str);
		DebugCheck(1);
	    }
	    type->MustBuildOnTop = auxtype;
	    free(str);
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("not-selectable"))) {
	    type->Selectable = 0;
	} else if (gh_eq_p(value, gh_symbol2scm("neutral-minimap-color"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
#ifdef USE_SDL_SURFACE
	    type->NeutralMinimapColorRGB.r = gh_scm2int(gh_car(sublist));
	    type->NeutralMinimapColorRGB.g = gh_scm2int(gh_car(gh_cdr(sublist)));
	    type->NeutralMinimapColorRGB.b = gh_scm2int(gh_car(gh_cdr(gh_cdr(sublist))));
#else
	    type->NeutralMinimapColorRGB.D24.a = gh_scm2int(gh_car(sublist));
	    type->NeutralMinimapColorRGB.D24.b = gh_scm2int(gh_car(gh_cdr(sublist)));
	    type->NeutralMinimapColorRGB.D24.c = gh_scm2int(gh_car(gh_cdr(gh_cdr(sublist))));
#endif
	} else if (gh_eq_p(value, gh_symbol2scm("box-size"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    type->BoxWidth = gh_scm2int(gh_car(sublist));
	    type->BoxHeight = gh_scm2int(gh_cadr(sublist));
	} else if (gh_eq_p(value, gh_symbol2scm("num-directions"))) {
	    type->NumDirections = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("revealer"))) {
	    type->Revealer = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("sight-range"))) {
	    type->_SightRange = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("computer-reaction-range"))) {
	    type->ReactRangeComputer = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("person-reaction-range"))) {
	    type->ReactRangePerson = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("armor"))) {
	    type->_Armor = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("basic-damage"))) {
	    type->_BasicDamage = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("piercing-damage"))) {
	    type->_PiercingDamage = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("missile"))) {
	    type->Missile.Name = gh_scm2newstr(gh_car(list), NULL);
	    type->Missile.Missile = NULL;
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("min-attack-range"))) {
	    type->MinAttackRange = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("max-attack-range"))) {
	    type->_AttackRange = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("priority"))) {
	    type->Priority = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("annoy-computer-factor"))) {
	    type->AnnoyComputerFactor = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("decay-rate"))) {
	    type->DecayRate = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("points"))) {
	    type->Points = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("demand"))) {
	    type->Demand = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("supply"))) {
	    type->Supply = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("corpse"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    if (redefine) {
		free(type->CorpseName);
	    }
	    type->CorpseName = gh_scm2newstr(gh_car(sublist), NULL);
	    type->CorpseType = NULL;
	    type->CorpseScript = gh_scm2int(gh_cadr(sublist));
	} else if (gh_eq_p(value, gh_symbol2scm("explode-when-killed"))) {
	    type->ExplodeWhenKilled = 1;
	    type->Explosion.Name = gh_scm2newstr(gh_car(list), NULL);
	    type->Explosion.Missile = NULL;
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("type-land"))) {
	    type->UnitType = UnitTypeLand;
	} else if (gh_eq_p(value, gh_symbol2scm("type-fly"))) {
	    type->UnitType = UnitTypeFly;
	} else if (gh_eq_p(value, gh_symbol2scm("type-naval"))) {
	    type->UnitType = UnitTypeNaval;

	} else if (gh_eq_p(value, gh_symbol2scm("right-none"))) {
	    type->MouseAction = MouseActionNone;
	} else if (gh_eq_p(value, gh_symbol2scm("right-attack"))) {
	    type->MouseAction = MouseActionAttack;
	} else if (gh_eq_p(value, gh_symbol2scm("right-move"))) {
	    type->MouseAction = MouseActionMove;
	} else if (gh_eq_p(value, gh_symbol2scm("right-harvest"))) {
	    type->MouseAction = MouseActionHarvest;
	} else if (gh_eq_p(value, gh_symbol2scm("right-spell-cast"))) {
	    type->MouseAction = MouseActionSpellCast;
	} else if (gh_eq_p(value, gh_symbol2scm("right-sail"))) {
	    type->MouseAction = MouseActionSail;

	} else if (gh_eq_p(value, gh_symbol2scm("can-ground-attack"))) {
	    type->GroundAttack = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("can-attack"))) {
	    type->CanAttack = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("repair-range"))) {
	    type->RepairRange = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("repair-hp"))) {
	    type->RepairHP = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("repair-costs"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		type->RepairCosts[CclGetResourceByName(value)] = gh_scm2int(gh_car(sublist));
		sublist = gh_cdr(sublist);
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("can-target-land"))) {
	    type->CanTarget |= CanTargetLand;
	} else if (gh_eq_p(value, gh_symbol2scm("can-target-sea"))) {
	    type->CanTarget |= CanTargetSea;
	} else if (gh_eq_p(value, gh_symbol2scm("can-target-air"))) {
	    type->CanTarget |= CanTargetAir;

	} else if (gh_eq_p(value, gh_symbol2scm("building"))) {
	    type->Building = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("visible-under-fog"))) {
	    type->VisibleUnderFog = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("builder-outside"))) {
	    type->BuilderOutside = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("builder-lost"))) {
	    type->BuilderLost = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("auto-build-rate"))) {
	    type->AutoBuildRate = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("shore-building"))) {
	    type->ShoreBuilding = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("land-unit"))) {
	    type->LandUnit = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("air-unit"))) {
	    type->AirUnit = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("sea-unit"))) {
	    type->SeaUnit = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("random-movement-probability"))) {
	    type->RandomMovementProbability = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("clicks-to-explode"))) {
	    type->ClicksToExplode = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("permanent-cloak"))) {
	    type->PermanentCloak = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("detect-cloak"))) {
	    type->DetectCloak = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("transporter"))) {
	    type->Transporter = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("coward"))) {
	    type->Coward = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("can-gather-resource"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    res = (ResourceInfo*)malloc(sizeof(ResourceInfo));
	    memset(res, 0, sizeof(ResourceInfo));
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		if (gh_eq_p(value, gh_symbol2scm("resource-id"))) {
		    res->ResourceId = CclGetResourceByName(gh_car(sublist));
		    type->ResInfo[res->ResourceId] = res;
		    sublist = gh_cdr(sublist);
		} else if (gh_eq_p(value, gh_symbol2scm("resource-step"))) {
		    res->ResourceStep = gh_scm2int(gh_car(sublist));
		    sublist = gh_cdr(sublist);
		} else if (gh_eq_p(value, gh_symbol2scm("final-resource"))) {
		    res->FinalResource = CclGetResourceByName(gh_car(sublist));
		    sublist = gh_cdr(sublist);
		} else if (gh_eq_p(value, gh_symbol2scm("wait-at-resource"))) {
		    res->WaitAtResource = gh_scm2int(gh_car(sublist));
		    sublist = gh_cdr(sublist);
		} else if (gh_eq_p(value, gh_symbol2scm("wait-at-depot"))) {
		    res->WaitAtDepot = gh_scm2int(gh_car(sublist));
		    sublist = gh_cdr(sublist);
		} else if (gh_eq_p(value, gh_symbol2scm("resource-capacity"))) {
		    res->ResourceCapacity = gh_scm2int(gh_car(sublist));
		    sublist = gh_cdr(sublist);
		} else if (gh_eq_p(value, gh_symbol2scm("terrain-harvester"))) {
		    res->TerrainHarvester = 1;
		} else if (gh_eq_p(value, gh_symbol2scm("lose-resources"))) {
		    res->LoseResources = 1;
		} else if (gh_eq_p(value, gh_symbol2scm("harvest-from-outside"))) {
		    res->HarvestFromOutside = 1;
		} else if (gh_eq_p(value, gh_symbol2scm("file-when-empty"))) {
		    res->FileWhenEmpty = gh_scm2newstr(gh_car(sublist),0);
		    sublist = gh_cdr(sublist);
		} else if (gh_eq_p(value, gh_symbol2scm("file-when-loaded"))) {
		    res->FileWhenLoaded = gh_scm2newstr(gh_car(sublist),0);
		    sublist = gh_cdr(sublist);
		} else {
		   printf("\n%s\n",type->Name);
		   errl("Unsupported tag", value);
		   DebugCheck(1);
		}
	    }
	    type->Harvester = 1;
	    if (!res->FinalResource) {
		res->FinalResource = res->ResourceId;
	    }
	    DebugCheck(!res->ResourceId);
	} else if (gh_eq_p(value, gh_symbol2scm("gives-resource"))) {
	    type->GivesResource = CclGetResourceByName(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("max-workers"))) {
	    type->MaxWorkers = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("can-harvest"))) {
	    type->CanHarvest = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("can-store"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		type->CanStore[CclGetResourceByName(gh_car(sublist))] = 1;
		sublist = gh_cdr(sublist);
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("vanishes"))) {
	    type->Vanishes = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("can-cast-spell"))) {
	    //
	    //    Warning: can-cast-spell should only be used AFTER all spells
	    //    have been defined. FIXME: MaxSpellType=500 or something?
	    //
	    if (!type->CanCastSpell) {
		type->CanCastSpell = malloc(SpellTypeCount);
		memset(type->CanCastSpell, 0, SpellTypeCount);
	    }
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    type->Magic = 0;
	    while (!gh_null_p(sublist)) {
		int id;
		id = CclGetSpellByIdent(gh_car(sublist));
		DebugLevel3Fn("%d \n" _C_ id);
		if (id == -1) {
		    errl("Unknown spell type", gh_car(sublist));
		}
		type->CanCastSpell[id] = 1;
		sublist = gh_cdr(sublist);
		type->Magic = 1;
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("can-target-flag"))) {
	    //
	    //    Warning: can-target-flag should only be used AFTER all bool flags
	    //    have been defined.
	    //
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		for (i = 0; i < NumberBoolFlag; i++) {
		    if (gh_eq_p(value, gh_symbol2scm(BoolFlagName[i]))) {
		        type->CanTargetFlag[i] = Scm2Condition(gh_car(sublist));
		        sublist = gh_cdr(sublist);
		        break;
		    }
		}
		if (i != NumberBoolFlag) {
		    continue;
		}
		printf("\n%s\n", type->Name);
		errl("Unsupported flag tag for can-target-flag", value);
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("selectable-by-rectangle"))) {
	    type->SelectableByRectangle = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("teleporter"))) {
	    type->Teleporter = 1;
	} else if (gh_eq_p(value, gh_symbol2scm("sounds"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {

		value = gh_car(sublist);
		sublist = gh_cdr(sublist);

		if (gh_eq_p(value, gh_symbol2scm("selected"))) {
		    if (redefine) {
			free(type->Sound.Selected.Name);
		    }
		    type->Sound.Selected.Name = gh_scm2newstr(
			gh_car(sublist), NULL);
		    sublist = gh_cdr(sublist);
		} else if (gh_eq_p(value, gh_symbol2scm("acknowledge"))) {
		    if (redefine) {
			free(type->Sound.Acknowledgement.Name);
		    }
		    type->Sound.Acknowledgement.Name = gh_scm2newstr(
			gh_car(sublist), NULL);
		    sublist = gh_cdr(sublist);
		} else if (gh_eq_p(value, gh_symbol2scm("ready"))) {
		    if (redefine) {
			free(type->Sound.Ready.Name);
		    }
		    type->Sound.Ready.Name = gh_scm2newstr(
			gh_car(sublist), NULL);
		    sublist = gh_cdr(sublist);
		} else if (gh_eq_p(value, gh_symbol2scm("repair"))) {
		    if (redefine) {
			free(type->Sound.Repair.Name);
		    }
		    type->Sound.Repair.Name = gh_scm2newstr(
			gh_car(sublist), NULL);
		    sublist = gh_cdr(sublist);
		} else if (gh_eq_p(value, gh_symbol2scm("harvest"))) {
		    int res;
		    char* name;

		    name = gh_scm2newstr(gh_car(sublist), NULL);
		    sublist = gh_cdr(sublist);
		    for (res = 0; res < MaxCosts; ++res) {
			if (!strcmp(name, DefaultResourceNames[res])) {
			    break;
			}
		    }
		    if (res == MaxCosts) {
			errl("Resource not found", value);
		    }
		    free(name);
		    if (redefine) {
			free(type->Sound.Harvest[res].Name);
		    }
		    type->Sound.Harvest[res].Name = gh_scm2newstr(
			gh_car(sublist), NULL);
		    sublist = gh_cdr(sublist);
		} else if (gh_eq_p(value, gh_symbol2scm("help"))) {
		    if (redefine) {
			free(type->Sound.Help.Name);
		    }
		    type->Sound.Help.Name = gh_scm2newstr(
			gh_car(sublist), NULL);
		    sublist = gh_cdr(sublist);
		} else if (gh_eq_p(value, gh_symbol2scm("dead"))) {
		    if (redefine) {
			free(type->Sound.Dead.Name);
		    }
		    type->Sound.Dead.Name = gh_scm2newstr(
			gh_car(sublist), NULL);
		    sublist = gh_cdr(sublist);
		} else if (gh_eq_p(value, gh_symbol2scm("attack"))) {
		    if (redefine) {
			free(type->Weapon.Attack.Name);
		    }
		    type->Weapon.Attack.Name = gh_scm2newstr(
			gh_car(sublist), NULL);
		    sublist = gh_cdr(sublist);
		} else {
		    errl("Unsupported sound tag", value);
		}
	    }
	} else {
	    for (i = 0; i < NumberBoolFlag; i++) { // User defined bool flags
		if (gh_eq_p(value, gh_symbol2scm(BoolFlagName[i]))) {
		    type->BoolFlag[i] = 1;
		    break;
		}
	    }
	    if (i != NumberBoolFlag) {
		continue;
	    }
	    // FIXME: this leaves a half initialized unit-type
	    printf("\n%s\n",type->Name);
	    errl("Unsupported tag", value);
	    DebugCheck(1);
	}
    }

    // FIXME: try to simplify/combine the flags instead
    if (type->MouseAction == MouseActionAttack && !type->CanAttack) {
	printf("Unit-type `%s': right-attack is set, but can-attack is not\n", type->Name);
	// ugly way to show the line number
	errl("", SCM_UNSPECIFIED); 
    }

    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDefineUnitType(lua_State* l)
{
    const char* value;
//    SCM sublist;
    UnitType* type;
    UnitType* auxtype;
    ResourceInfo* res;
    char* str;
    int i;
    int redefine;
    int args;
    int j;
    int subargs;
    int k;

    args = lua_gettop(l);
    j = 0;

    //	Slot identifier
    if (!lua_isstring(l, j + 1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    str = strdup(lua_tostring(l, j + 1));
    ++j;

#ifdef DEBUG
    i = NoWarningUnitType;
    NoWarningUnitType = 1;
#endif
    type = UnitTypeByIdent(str);
#ifdef DEBUG
    NoWarningUnitType = i;
#endif
    if (type) {
	DebugLevel3Fn("Redefining unit-type `%s'\n" _C_ str);
	free(str);
	redefine = 1;
    } else {
	DebugLevel3Fn("Defining unit-type `%s'\n" _C_ str);
	type = NewUnitTypeSlot(str);
	redefine = 0;
	//Set some default values
	type->_RegenerationRate = 0;
	type->Selectable = 1;
    }
    type->BoolFlag = realloc(type->BoolFlag, NumberBoolFlag * sizeof (*type->BoolFlag));
    memset(type->BoolFlag, 0, NumberBoolFlag * sizeof (*type->BoolFlag));
    type->CanTargetFlag = realloc(type->CanTargetFlag, NumberBoolFlag * sizeof (*type->CanTargetFlag));
    memset(type->CanTargetFlag, 0, NumberBoolFlag * sizeof (*type->CanTargetFlag));

    type->NumDirections = 8;

//    type->Property = SCM_UNSPECIFIED;

    //
    //	Parse the list:	(still everything could be changed!)
    //
    for (; j < args; ++j) {
	if (!lua_isstring(l, j + 1)) {
	    lua_pushstring(l, "incorrect argument");
	    lua_error(l);
	}
	value = lua_tostring(l, j + 1);
	++j;
	if (!strcmp(value, "name")) {
	    if (!lua_isstring(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    if (redefine) {
		free(type->Name);
	    }
	    type->Name = strdup(lua_tostring(l, j + 1));
	} else if (!strcmp(value, "use")) {
	    if (!lua_isstring(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    if (redefine) {
		free(type->SameSprite);
	    }
	    type->SameSprite = strdup(lua_tostring(l, j + 1));
	} else if (!strcmp(value, "files")) {
	    if (!lua_istable(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    subargs = luaL_getn(l, j + 1);
	    for (k = 0; k < subargs; ++k) {
		lua_rawgeti(l, j + 1, k + 1);
		if (!lua_isstring(l, -1)) {
		    lua_pushstring(l, "incorrect argument");
		    lua_error(l);
		}
		value = lua_tostring(l, -1);
		lua_pop(l, 1);
		++k;

		// FIXME: use a general get tileset function here!
		i = 0;
		if (strcmp(value, "default")) {
		    for (; i < NumTilesets; ++i) {
			if (!strcmp(value, Tilesets[i]->Ident) ||
				!strcmp(value, Tilesets[i]->Class)) {
			    break;
			}
		    }
		    if (i == NumTilesets) {
		       // FIXME: this leaves half initialized unit-type
		       lua_pushfstring(l, "Unsupported tileset tag", value);
		       lua_error(l);
		    }
		}
		if (redefine) {
		    free(type->File[i]);
		}
		lua_rawgeti(l, j + 1, k + 1);
		if (!lua_isstring(l, -1)) {
		    lua_pushstring(l, "incorrect argument");
		    lua_error(l);
		}
		type->File[i] = strdup(lua_tostring(l, -1));
		lua_pop(l, 1);
	    }
	} else if (!strcmp(value, "shadow")) {
#if 0
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);

		if (gh_eq_p(value, "file")) {
		    if (redefine) {
			free(type->ShadowFile);
		    }
		    type->ShadowFile = gh_scm2newstr(gh_car(sublist), NULL);
		} else if (gh_eq_p(value, "size")) {
		    type->ShadowWidth = gh_scm2int(gh_car(gh_car(sublist)));
		    type->ShadowHeight = gh_scm2int(gh_car(gh_cdr(gh_car(sublist))));
		} else if (gh_eq_p(value, "height")) {
		} else if (gh_eq_p(value, "offset")) {
		    type->ShadowOffsetX = gh_scm2int(gh_car(gh_car(sublist)));
		    type->ShadowOffsetY = gh_scm2int(gh_car(gh_cdr(gh_car(sublist))));
		} else {
		    errl("Unsupported shadow tag", value);
		}
		sublist = gh_cdr(sublist);
	    }
#endif
	} else if (!strcmp(value, "size")) {
	    if (!lua_istable(l, j + 1) || luaL_getn(l, j + 1) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    lua_rawgeti(l, j + 1, 1);
	    if (!lua_isnumber(l, -1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->Width = lua_tonumber(l, -1);
	    lua_pop(l, 1);
	    lua_rawgeti(l, j + 1, 2);
	    if (!lua_isnumber(l, -1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->Height = lua_tonumber(l, -1);
	    lua_pop(l, 1);
	} else if (!strcmp(value, "animations")) {
	    if (!lua_isstring(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->Animations = AnimationsByIdent(lua_tostring(l, j + 1));
	} else if (!strcmp(value, "icon")) {
	    if (!lua_isstring(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    if (redefine) {
		free(type->Icon.Name);
	    }
	    type->Icon.Name = strdup(lua_tostring(l, j + 1));
	    type->Icon.Icon = NULL;
	} else if (!strcmp(value, "costs")) {
#if 0
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		type->_Costs[CclGetResourceByName(value)] = gh_scm2int(gh_car(sublist));
		sublist = gh_cdr(sublist);
	    }
#endif
	} else if (!strcmp(value, "improve-production")) {
#if 0
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		type->ImproveIncomes[CclGetResourceByName(value)] =
			DefaultIncomes[CclGetResourceByName(value)] + gh_scm2int(gh_car(sublist));
		sublist = gh_cdr(sublist);
	    }
#endif
	} else if (!strcmp(value, "construction")) {
	    if (!lua_isstring(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    // FIXME: What if constructions aren't yet loaded?
	    type->Construction = ConstructionByIdent(lua_tostring(l, j + 1));
	} else if (!strcmp(value, "speed")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->_Speed = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "draw-level")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->DrawLevel = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "max-on-board")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->MaxOnBoard = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "hit-points")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->_HitPoints = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "regeneration-rate")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->_RegenerationRate = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "burn-percent")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->BurnPercent = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "burn-damage-rate")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->BurnDamageRate = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "max-mana")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->_MaxMana = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "tile-size")) {
	    if (!lua_istable(l, j + 1) || luaL_getn(l, j + 1) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    lua_rawgeti(l, j + 1, 1);
	    if (!lua_isnumber(l, -1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->TileWidth = lua_tonumber(l, -1);
	    lua_pop(l, 1);
	    lua_rawgeti(l, j + 1, 2);
	    if (!lua_isnumber(l, -1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->TileHeight = lua_tonumber(l, -1);
	    lua_pop(l, 1);
	} else if (!strcmp(value, "must-build-on-top")) {
	    if (!lua_isstring(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    value = lua_tostring(l, j + 1);
	    auxtype = UnitTypeByIdent(value);
	    if (!auxtype) {
		DebugLevel0("Build on top of undefined unit \"%s\".\n" _C_ str);
		DebugCheck(1);
	    }
	    type->MustBuildOnTop = auxtype;
	} else if (!strcmp(value, "not-selectable")) {
	    type->Selectable = 0;
	    --j;
	} else if (!strcmp(value, "neutral-minimap-color")) {
#if 0
	    sublist = gh_car(list);
	    list = gh_cdr(list);
#ifdef USE_SDL_SURFACE
	    type->NeutralMinimapColorRGB.r = gh_scm2int(gh_car(sublist));
	    type->NeutralMinimapColorRGB.g = gh_scm2int(gh_car(gh_cdr(sublist)));
	    type->NeutralMinimapColorRGB.b = gh_scm2int(gh_car(gh_cdr(gh_cdr(sublist))));
#else
	    type->NeutralMinimapColorRGB.D24.a = gh_scm2int(gh_car(sublist));
	    type->NeutralMinimapColorRGB.D24.b = gh_scm2int(gh_car(gh_cdr(sublist)));
	    type->NeutralMinimapColorRGB.D24.c = gh_scm2int(gh_car(gh_cdr(gh_cdr(sublist))));
#endif
#endif
	} else if (!strcmp(value, "box-size")) {
	    if (!lua_istable(l, j + 1) || luaL_getn(l, j + 1) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    lua_rawgeti(l, j + 1, 1);
	    if (!lua_isnumber(l, -1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->BoxWidth = lua_tonumber(l, -1);
	    lua_pop(l, 1);
	    lua_rawgeti(l, j + 1, 2);
	    if (!lua_isnumber(l, -1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->BoxHeight = lua_tonumber(l, -1);
	    lua_pop(l, 1);
	} else if (!strcmp(value, "num-directions")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->NumDirections = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "revealer")) {
	    type->Revealer = 1;
	    --j;
	} else if (!strcmp(value, "sight-range")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->_SightRange = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "computer-reaction-range")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->ReactRangeComputer = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "person-reaction-range")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->ReactRangePerson = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "armor")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->_Armor = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "basic-damage")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->_BasicDamage = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "piercing-damage")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->_PiercingDamage = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "missile")) {
	    if (!lua_isstring(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->Missile.Name = strdup(lua_tostring(l, j + 1));
	    type->Missile.Missile = NULL;
	} else if (!strcmp(value, "min-attack-range")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->MinAttackRange = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "max-attack-range")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->_AttackRange = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "priority")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->Priority = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "annoy-computer-factor")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->AnnoyComputerFactor = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "decay-rate")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->DecayRate = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "points")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->Points = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "demand")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->Demand = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "supply")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->Supply = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "corpse")) {
#if 0
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    if (redefine) {
		free(type->CorpseName);
	    }
	    type->CorpseName = gh_scm2newstr(gh_car(sublist), NULL);
	    type->CorpseType = NULL;
	    type->CorpseScript = gh_scm2int(gh_cadr(sublist));
#endif
	} else if (!strcmp(value, "explode-when-killed")) {
	    if (!lua_isstring(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->ExplodeWhenKilled = 1;
	    type->Explosion.Name = strdup(lua_tostring(l, j + 1));
	    type->Explosion.Missile = NULL;
	} else if (!strcmp(value, "type-land")) {
	    type->UnitType = UnitTypeLand;
	    --j;
	} else if (!strcmp(value, "type-fly")) {
	    type->UnitType = UnitTypeFly;
	    --j;
	} else if (!strcmp(value, "type-naval")) {
	    type->UnitType = UnitTypeNaval;
	    --j;

	} else if (!strcmp(value, "right-none")) {
	    type->MouseAction = MouseActionNone;
	    --j;
	} else if (!strcmp(value, "right-attack")) {
	    type->MouseAction = MouseActionAttack;
	    --j;
	} else if (!strcmp(value, "right-move")) {
	    type->MouseAction = MouseActionMove;
	    --j;
	} else if (!strcmp(value, "right-harvest")) {
	    type->MouseAction = MouseActionHarvest;
	    --j;
	} else if (!strcmp(value, "right-spell-cast")) {
	    type->MouseAction = MouseActionSpellCast;
	    --j;
	} else if (!strcmp(value, "right-sail")) {
	    type->MouseAction = MouseActionSail;
	    --j;

	} else if (!strcmp(value, "can-ground-attack")) {
	    type->GroundAttack = 1;
	    --j;
	} else if (!strcmp(value, "can-attack")) {
	    type->CanAttack = 1;
	    --j;
	} else if (!strcmp(value, "repair-range")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->RepairRange = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "repair-hp")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->RepairHP = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "repair-costs")) {
#if 0
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		type->RepairCosts[CclGetResourceByName(value)] = gh_scm2int(gh_car(sublist));
		sublist = gh_cdr(sublist);
	    }
#endif
	} else if (!strcmp(value, "can-target-land")) {
	    type->CanTarget |= CanTargetLand;
	    --j;
	} else if (!strcmp(value, "can-target-sea")) {
	    type->CanTarget |= CanTargetSea;
	    --j;
	} else if (!strcmp(value, "can-target-air")) {
	    type->CanTarget |= CanTargetAir;
	    --j;

	} else if (!strcmp(value, "building")) {
	    type->Building = 1;
	    --j;
	} else if (!strcmp(value, "visible-under-fog")) {
	    type->VisibleUnderFog = 1;
	    --j;
	} else if (!strcmp(value, "builder-outside")) {
	    type->BuilderOutside = 1;
	    --j;
	} else if (!strcmp(value, "builder-lost")) {
	    type->BuilderLost = 1;
	    --j;
	} else if (!strcmp(value, "auto-build-rate")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->AutoBuildRate = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "shore-building")) {
	    type->ShoreBuilding = 1;
	    --j;
	} else if (!strcmp(value, "land-unit")) {
	    type->LandUnit = 1;
	    --j;
	} else if (!strcmp(value, "air-unit")) {
	    type->AirUnit = 1;
	    --j;
	} else if (!strcmp(value, "sea-unit")) {
	    type->SeaUnit = 1;
	    --j;
	} else if (!strcmp(value, "random-movement-probability")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->RandomMovementProbability = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "clicks-to-explode")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->ClicksToExplode = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "permanent-cloak")) {
	    type->PermanentCloak = 1;
	    --j;
	} else if (!strcmp(value, "detect-cloak")) {
	    type->DetectCloak = 1;
	    --j;
	} else if (!strcmp(value, "transporter")) {
	    type->Transporter = 1;
	    --j;
	} else if (!strcmp(value, "coward")) {
	    type->Coward = 1;
	    --j;
	} else if (!strcmp(value, "can-gather-resource")) {
	    res = (ResourceInfo*)malloc(sizeof(ResourceInfo));
	    memset(res, 0, sizeof(ResourceInfo));
#if 0
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		if (!strcmp(value, "resource-id")) {
		    res->ResourceId = CclGetResourceByName(gh_car(sublist));
		    type->ResInfo[res->ResourceId] = res;
		    sublist = gh_cdr(sublist);
		} else if (!strcmp(value, "resource-step")) {
		    res->ResourceStep = gh_scm2int(gh_car(sublist));
		    sublist = gh_cdr(sublist);
		} else if (!strcmp(value, "final-resource")) {
		    res->FinalResource = CclGetResourceByName(gh_car(sublist));
		    sublist = gh_cdr(sublist);
		} else if (!strcmp(value, "wait-at-resource")) {
		    res->WaitAtResource = gh_scm2int(gh_car(sublist));
		    sublist = gh_cdr(sublist);
		} else if (!strcmp(value, "wait-at-depot")) {
		    res->WaitAtDepot = gh_scm2int(gh_car(sublist));
		    sublist = gh_cdr(sublist);
		} else if (!strcmp(value, "resource-capacity")) {
		    res->ResourceCapacity = gh_scm2int(gh_car(sublist));
		    sublist = gh_cdr(sublist);
		} else if (!strcmp(value, "terrain-harvester")) {
		    res->TerrainHarvester = 1;
		} else if (!strcmp(value, "lose-resources")) {
		    res->LoseResources = 1;
		} else if (!strcmp(value, "harvest-from-outside")) {
		    res->HarvestFromOutside = 1;
		} else if (!strcmp(value, "file-when-empty")) {
		    res->FileWhenEmpty = gh_scm2newstr(gh_car(sublist),0);
		    sublist = gh_cdr(sublist);
		} else if (!strcmp(value, "file-when-loaded")) {
		    res->FileWhenLoaded = gh_scm2newstr(gh_car(sublist),0);
		    sublist = gh_cdr(sublist);
		} else {
		   printf("\n%s\n",type->Name);
		   errl("Unsupported tag", value);
		   DebugCheck(1);
		}
	    }
	    type->Harvester = 1;
	    if (!res->FinalResource) {
		res->FinalResource = res->ResourceId;
	    }
	    DebugCheck(!res->ResourceId);
#endif
	} else if (!strcmp(value, "gives-resource")) {
	    lua_pushvalue(l, j + 1);
	    type->GivesResource = CclGetResourceByName(l);
	    lua_pop(l, 1);
	} else if (!strcmp(value, "max-workers")) {
	    if (!lua_isnumber(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    type->MaxWorkers = lua_tonumber(l, j + 1);
	} else if (!strcmp(value, "can-harvest")) {
	    type->CanHarvest = 1;
	    --j;
	} else if (!strcmp(value, "can-store")) {
#if 0
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		type->CanStore[CclGetResourceByName(gh_car(sublist))] = 1;
		sublist = gh_cdr(sublist);
	    }
#endif
	} else if (!strcmp(value, "vanishes")) {
	    type->Vanishes = 1;
	    --j;
	} else if (!strcmp(value, "can-cast-spell")) {
#if 0
	    //
	    //    Warning: can-cast-spell should only be used AFTER all spells
	    //    have been defined. FIXME: MaxSpellType=500 or something?
	    //
	    if (!type->CanCastSpell) {
		type->CanCastSpell = malloc(SpellTypeCount);
		memset(type->CanCastSpell, 0, SpellTypeCount);
	    }
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    type->Magic = 0;
	    while (!gh_null_p(sublist)) {
		int id;
		id = CclGetSpellByIdent(gh_car(sublist));
		DebugLevel3Fn("%d \n" _C_ id);
		if (id == -1) {
		    errl("Unknown spell type", gh_car(sublist));
		}
		type->CanCastSpell[id] = 1;
		sublist = gh_cdr(sublist);
		type->Magic = 1;
	    }
#endif
	} else if (!strcmp(value, "can-target-flag")) {
#if 0
	    //
	    //    Warning: can-target-flag should only be used AFTER all bool flags
	    //    have been defined.
	    //
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);
		for (i = 0; i < NumberBoolFlag; i++) {
		    if (!strcmp(value, BoolFlagName[i]))) {
		        type->CanTargetFlag[i] = Scm2Condition(gh_car(sublist));
		        sublist = gh_cdr(sublist);
		        break;
		    }
		}
		if (i != NumberBoolFlag) {
		    continue;
		}
		printf("\n%s\n", type->Name);
		errl("Unsupported flag tag for can-target-flag", value);
	    }
#endif
	} else if (!strcmp(value, "selectable-by-rectangle")) {
	    type->SelectableByRectangle = 1;
	    --j;
	} else if (!strcmp(value, "teleporter")) {
	    type->Teleporter = 1;
	    --j;
	} else if (!strcmp(value, "sounds")) {
#if 0
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {

		value = gh_car(sublist);
		sublist = gh_cdr(sublist);

		if (!strcmp(value, "selected")) {
		    if (redefine) {
			free(type->Sound.Selected.Name);
		    }
		    type->Sound.Selected.Name = gh_scm2newstr(
			gh_car(sublist), NULL);
		    sublist = gh_cdr(sublist);
		} else if (!strcmp(value, "acknowledge")) {
		    if (redefine) {
			free(type->Sound.Acknowledgement.Name);
		    }
		    type->Sound.Acknowledgement.Name = gh_scm2newstr(
			gh_car(sublist), NULL);
		    sublist = gh_cdr(sublist);
		} else if (!strcmp(value, "ready")) {
		    if (redefine) {
			free(type->Sound.Ready.Name);
		    }
		    type->Sound.Ready.Name = gh_scm2newstr(
			gh_car(sublist), NULL);
		    sublist = gh_cdr(sublist);
		} else if (!strcmp(value, "repair")) {
		    if (redefine) {
			free(type->Sound.Repair.Name);
		    }
		    type->Sound.Repair.Name = gh_scm2newstr(
			gh_car(sublist), NULL);
		    sublist = gh_cdr(sublist);
		} else if (!strcmp(value, "harvest")) {
		    int res;
		    char* name;

		    name = gh_scm2newstr(gh_car(sublist), NULL);
		    sublist = gh_cdr(sublist);
		    for (res = 0; res < MaxCosts; ++res) {
			if (!strcmp(name, DefaultResourceNames[res])) {
			    break;
			}
		    }
		    if (res == MaxCosts) {
			errl("Resource not found", value);
		    }
		    free(name);
		    if (redefine) {
			free(type->Sound.Harvest[res].Name);
		    }
		    type->Sound.Harvest[res].Name = gh_scm2newstr(
			gh_car(sublist), NULL);
		    sublist = gh_cdr(sublist);
		} else if (!strcmp(value, "help")) {
		    if (redefine) {
			free(type->Sound.Help.Name);
		    }
		    type->Sound.Help.Name = gh_scm2newstr(
			gh_car(sublist), NULL);
		    sublist = gh_cdr(sublist);
		} else if (!strcmp(value, "dead")) {
		    if (redefine) {
			free(type->Sound.Dead.Name);
		    }
		    type->Sound.Dead.Name = gh_scm2newstr(
			gh_car(sublist), NULL);
		    sublist = gh_cdr(sublist);
		} else if (!strcmp(value, "attack")) {
		    if (redefine) {
			free(type->Weapon.Attack.Name);
		    }
		    type->Weapon.Attack.Name = gh_scm2newstr(
			gh_car(sublist), NULL);
		    sublist = gh_cdr(sublist);
		} else {
		    lua_pushfstring(l, "Unsupported sound tag: %s", value);
		    lua_error(l);
		}
	    }
#endif
	} else {
	    for (i = 0; i < NumberBoolFlag; ++i) { // User defined bool flags
		if (!strcmp(value, BoolFlagName[i])) {
		    type->BoolFlag[i] = 1;
		    --j;
		    break;
		}
	    }
            if (i != NumberBoolFlag) {
		continue;
	    }
	    // FIXME: this leaves a half initialized unit-type
	    printf("\n%s\n",type->Name);
	    lua_pushfstring(l, "Unsupported tag: %s", value);
	    lua_error(l);
	    DebugCheck(1);
	}
    }

    // FIXME: try to simplify/combine the flags instead
    if (type->MouseAction == MouseActionAttack && !type->CanAttack) {
	lua_pushfstring(l, "Unit-type `%s': right-attack is set, but can-attack is not\n", type->Name);
	lua_error(l);
    }

    return 0;
}
#endif

/**
**	Parse unit-stats.
**
**	@param list	List describing the unit-stats.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineUnitStats(SCM list)
{
    SCM value;
    //SCM data;
    SCM sublist;
    UnitType* type;
    UnitStats* stats;
    int i;
    char* str;

    type = UnitTypeByIdent(str = gh_scm2newstr(gh_car(list), NULL));
    DebugCheck(!type);
    
    free(str);
    list = gh_cdr(list);
    i = gh_scm2int(gh_car(list));
    DebugCheck(i >= PlayerMax);
    list = gh_cdr(list);

    stats = &type->Stats[i];

    //
    //	Parse the list:	(still everything could be changed!)
    //
    while (!gh_null_p(list)) {

	value = gh_car(list);
	list = gh_cdr(list);

	if (gh_eq_p(value, gh_symbol2scm("level"))) {
	    stats->Level = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("speed"))) {
	    stats->Speed = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("attack-range"))) {
	    stats->AttackRange = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("sight-range"))) {
	    stats->SightRange = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("armor"))) {
	    stats->Armor = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("basic-damage"))) {
	    stats->BasicDamage = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("piercing-damage"))) {
	    stats->PiercingDamage = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("hit-points"))) {
	    stats->HitPoints = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("regeneration-rate"))) {
	    stats->RegenerationRate = gh_scm2int(gh_car(list));
	    list = gh_cdr(list);
	} else if (gh_eq_p(value, gh_symbol2scm("costs"))) {
	    sublist = gh_car(list);
	    list = gh_cdr(list);
	    while (!gh_null_p(sublist)) {

		value = gh_car(sublist);
		sublist = gh_cdr(sublist);

		for (i = 0; i < MaxCosts; ++i) {
		    if (gh_eq_p(value, gh_symbol2scm(DefaultResourceNames[i]))) {
			stats->Costs[i] = gh_scm2int(gh_car(sublist));
			break;
		    }
		}
		if (i == MaxCosts) {
		   // FIXME: this leaves half initialized stats
		   errl("Unsupported tag", value);
		}
		sublist = gh_cdr(sublist);
	    }
	} else {
	   // FIXME: this leaves a half initialized unit
	   errl("Unsupported tag", value);
	}
    }

    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
#endif

// ----------------------------------------------------------------------------

/**
**	Access unit-type object
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
global UnitType* CclGetUnitType(SCM ptr)
{
    char* str;
    UnitType* type;

    // Be kind allow also strings or symbols
    if ((str = CclConvertToString(ptr)) != NULL)  {
        DebugLevel3("CclGetUnitType: %s\n"_C_ str);
        type = UnitTypeByIdent(str);
        free(str);
        return type;
    } else if (CclGetSmobType(ptr) == SiodUnitTypeTag)  {
        return CclGetSmobData(ptr);
    } else {
        errl("CclGetUnitType: not an unit-type", ptr);
        return 0;
    }
}
#elif defined(USE_LUA)
#endif

/**
**	Print the unit-type object
**
**	@param ptr	Scheme object.
**	@param f	Output structure.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local void CclUnitTypePrin1(SCM ptr, struct gen_printio* f)
{
#ifndef USE_GUILE
    char buf[1024];
    const UnitType* type;

    type = CclGetUnitType(ptr);

    if (type) {
        if (type->Ident) {
            sprintf(buf, "#<UnitType %p '%s'>", type, type->Ident);
        } else {
            sprintf(buf, "#<UnitType %p '(null)'>", type);
        }
    } else {
        sprintf(buf, "#<UnitType NULL>");
    }

    gput_st(f,buf);
#endif
}
#elif defined(USE_LUA)
#endif

/**
**	Get unit-type structure.
**
**	@param ident	Identifier for unit-type.
**
**	@return		Unit-type structure.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclUnitType(SCM ident)
{
    char* str;
    UnitType* type;
    
    str = CclConvertToString(ident);
    if (str) {
        type = UnitTypeByIdent(str);
        printf("CclUnitType: '%s' -> '%ld'\n", str, (long)type);
        free(str);
        return CclMakeSmobObj(SiodUnitTypeTag, type);
    } else {
        errl("CclUnitType: no unittype by ident: ", ident);
        return SCM_BOOL_F;
    }
}
#elif defined(USE_LUA)
#endif

/**
**	Get all unit-type structures.
**
**	@return		An array of all unit-type structures.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclUnitTypeArray(void)
{
    SCM array;
    SCM value;
    int i;

    array = cons_array(gh_int2scm(UnitTypeMax), NIL);

    for (i = 0; i < UnitTypeMax; ++i) {
      value = CclMakeSmobObj(SiodUnitTypeTag, &UnitTypes[i]);
      gh_vector_set_x(array, gh_int2scm(i), value);
    }
    return array;
}
#elif defined(USE_LUA)
#endif

/**
**	Get the ident of the unit-type structure.
**
**	@param ptr	Unit-type object.
**
**	@return		The identifier of the unit-type.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclGetUnitTypeIdent(SCM ptr)
{
    const UnitType* type;
    SCM value;

    type = CclGetUnitType(ptr);
    value = gh_str02scm(type->Ident);
    return value;
}
#elif defined(USE_LUA)
#endif

/**
**	Get the name of the unit-type structure.
**
**	@param ptr	Unit-type object.
**
**	@return		The name of the unit-type.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclGetUnitTypeName(SCM ptr)
{
    const UnitType* type;
    SCM value;

    type = CclGetUnitType(ptr);
    value = gh_str02scm(type->Name);
    return value;
}
#elif defined(USE_LUA)
#endif

/**
**	Set the name of the unit-type structure.
**
**	@param ptr	Unit-type object.
**	@param name	The name to set.
**
**	@return		The name of the unit-type.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetUnitTypeName(SCM ptr, SCM name)
{
    UnitType* type;

    type = CclGetUnitType(ptr);
    free(type->Name);
    type->Name = gh_scm2newstr(name, NULL);

    return name;
}
#elif defined(USE_LUA)
#endif

// FIXME: write the missing access functions

/**
**	Get the property of the unit-type structure.
**
**	@param ptr	Unit-type object.
**
**	@return		The property of the unit-type.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclGetUnitTypeProperty(SCM ptr)
{
    const UnitType* type;

    type = CclGetUnitType(ptr);
    return type->Property;
}
#elif defined(USE_LUA)
#endif

/**
**	Set the property of the unit-type structure.
**
**	@param ptr	Unit-type object.
**	@param property	The property to set.
**
**	@return		The property of the unit-type.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetUnitTypeProperty(SCM ptr, SCM property)
{
    UnitType* type;

    type = CclGetUnitType(ptr);

    if (!property) {
	DebugLevel0Fn("oops, my fault\n");
    }

    if (property != SCM_UNSPECIFIED && (SCM)type->Property == SCM_UNSPECIFIED) {
    	CclGcProtect((SCM*)&type->Property);
    } else if (property == SCM_UNSPECIFIED && (SCM)type->Property != SCM_UNSPECIFIED) {
	CclGcProtectedAssign((SCM*)&type->Property, property);
	CclGcUnprotect((SCM*)&type->Property);
    } else {
	CclGcProtectedAssign((SCM*)&type->Property, property);
    }

    return property;
}
#elif defined(USE_LUA)
#endif

/**
**	Define tileset mapping from original number to internal symbol
**
**	@param list	List of all names.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineUnitTypeWcNames(SCM list)
{
    int i;
    char** cp;

    if ((cp = UnitTypeWcNames)) {		// Free all old names
	while (*cp) {
	    free(*cp++);
	}
	free(UnitTypeWcNames);
    }

    //
    //	Get new table.
    //
    i = gh_length(list);
    UnitTypeWcNames = cp = malloc((i + 1) * sizeof(char*));
    while (i--) {
	*cp++ = gh_scm2newstr(gh_car(list), NULL);
	list = gh_cdr(list);
    }
    *cp = NULL;

    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDefineUnitTypeWcNames(lua_State* l)
{
    int i;
    int j;
    char** cp;

    if ((cp = UnitTypeWcNames)) {	// Free all old names
	while (*cp) {
	    free(*cp++);
	}
	free(UnitTypeWcNames);
    }

    //
    //	Get new table.
    //
    i = lua_gettop(l);
    UnitTypeWcNames = cp = malloc((i + 1) * sizeof(char*));
    if (!cp) {
	fprintf(stderr, "out of memory.\n");
	ExitFatal(-1);
    }

    for (j = 0; j < i; ++j) {
	if (!lua_isstring(l, j + 1)) {
	    lua_pushstring(l, "incorrect argument");
	    lua_error(l);
	}
	*cp++ = strdup(lua_tostring(l, j + 1));
    }
    *cp = NULL;

    return 0;
}
#endif

// ----------------------------------------------------------------------------

/**
**	Define an unit-type animations set.
**
**	@param list	Animations list.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineAnimations(SCM list)
{
    char* str;
    SCM id;
    SCM value;
    SCM resource;
    Animations* anims;
    Animation* anim;
    Animation* t;
    int i;
    int frame;

    resource = NULL;
    str = gh_scm2newstr(gh_car(list), NULL);
    list = gh_cdr(list);
    anims = calloc(1, sizeof(Animations));

    while (!gh_null_p(list)) {
	id = gh_car(list);
	list = gh_cdr(list);
	if (gh_eq_p(id, gh_symbol2scm("harvest"))) {
	    resource = gh_car(list);
	    list = gh_cdr(list);
	}
	value = gh_car(list);
	list = gh_cdr(list);

	t = anim = malloc(gh_length(value) * sizeof(Animation));
	frame = 0;
	while (!gh_null_p(value)) {
	    t->Flags = gh_scm2int(gh_vector_ref(gh_car(value), gh_int2scm(0)));
	    t->Pixel = gh_scm2int(gh_vector_ref(gh_car(value), gh_int2scm(1)));
	    t->Sleep = gh_scm2int(gh_vector_ref(gh_car(value), gh_int2scm(2)));
	    i = gh_scm2int(gh_vector_ref(gh_car(value), gh_int2scm(3)));
	    t->Frame = i - frame;
	    frame = i;
	    if (t->Flags & AnimationRestart) {
		frame = 0;
	    }
	    ++t;
	    value = gh_cdr(value);
	}
	t[-1].Flags |= 0x80;		// Marks end of list

	if (gh_eq_p(id, gh_symbol2scm("still"))) {
	    if (anims->Still) {
		free(anims->Still);
	    }
	    anims->Still = anim;
	} else if (gh_eq_p(id, gh_symbol2scm("move"))) {
	    if (anims->Move) {
		free(anims->Move);
	    }
	    anims->Move = anim;
	} else if (gh_eq_p(id, gh_symbol2scm("attack"))) {
	    if (anims->Attack) {
		free(anims->Attack);
	    }
	    anims->Attack = anim;
	} else if (gh_eq_p(id, gh_symbol2scm("repair"))) {
	    if (anims->Repair) {
		free(anims->Repair);
	    }
	    anims->Repair = anim;
	} else if (gh_eq_p(id, gh_symbol2scm("harvest"))) {
	    int res;
	    char* name;

	    name = gh_scm2newstr(resource, NULL);
	    for (res = 0; res < MaxCosts; ++res) {
		if (!strcmp(name, DefaultResourceNames[res])) {
		    break;
		}
	    }
	    if (res == MaxCosts) {
		errl("Resource not found", resource);
	    }
	    free(name);
	    if (anims->Harvest[res]) {
		free(anims->Harvest[res]);
	    }
	    anims->Harvest[res] = anim;
	} else if (gh_eq_p(id, gh_symbol2scm("die"))) {
	    if (anims->Die) {
		free(anims->Die);
	    }
	    anims->Die = anim;
	} else {
	    errl("Unsupported tag", id);
	}
    }

    *(Animations**)hash_add(AnimationsHash, str) = anims;
    free(str);

    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDefineAnimations(lua_State* l)
{
    const char* str;
    const char* id;
    const char* resource;
    Animations* anims;
    Animation* anim;
    Animation* t;
    int i;
    int frame;
    int args;
    int j;
    int subargs;
    int k;

    args = lua_gettop(l);
    j = 0;

    resource = NULL;
    if (!lua_isstring(l, j + 1)) {
	lua_pushstring(l, "incorrect argument");
	lua_error(l);
    }
    str = lua_tostring(l, j + 1);
    ++j;
    anims = calloc(1, sizeof(Animations));

    for (; j < args; ++j) {
	if (!lua_isstring(l, j + 1)) {
	    lua_pushstring(l, "incorrect argument");
	    lua_error(l);
	}
	id = lua_tostring(l, j + 1);
	++j;
	if (!strcmp(id, "harvest")) {
	    if (!lua_isstring(l, j + 1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    resource = lua_tostring(l, j + 1);
	    ++j;
	}

	if (!lua_istable(l, j + 1)) {
	    lua_pushstring(l, "incorrect argument");
	    lua_error(l);
	}
	subargs = luaL_getn(l, j + 1);
	t = anim = malloc(subargs * sizeof(Animation));
	frame = 0;
	for (k = 0; k < subargs; ++k) {
	    lua_rawgeti(l, j + 1, k + 1);
	    if (!lua_istable(l, -1) || luaL_getn(l, -1) != 4) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    lua_rawgeti(l, -1, 1);
	    if (!lua_isnumber(l, -1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    t->Flags = lua_tonumber(l, -1);
	    lua_pop(l, 1);
	    lua_rawgeti(l, -1, 2);
	    if (!lua_isnumber(l, -1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    t->Pixel = lua_tonumber(l, -1);
	    lua_pop(l, 1);
	    lua_rawgeti(l, -1, 3);
	    if (!lua_isnumber(l, -1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    t->Sleep = lua_tonumber(l, -1);
	    lua_pop(l, 1);
	    lua_rawgeti(l, -1, 4);
	    if (!lua_isnumber(l, -1)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	    }
	    i = lua_tonumber(l, -1);
	    lua_pop(l, 1);
	    t->Frame = i - frame;
	    frame = i;
	    if (t->Flags & AnimationRestart) {
		frame = 0;
	    }
	    ++t;
	    lua_pop(l, 1);
	}
	t[-1].Flags |= 0x80;		// Marks end of list

	if (!strcmp(id, "still")) {
	    if (anims->Still) {
		free(anims->Still);
	    }
	    anims->Still = anim;
	} else if (!strcmp(id, "move")) {
	    if (anims->Move) {
		free(anims->Move);
	    }
	    anims->Move = anim;
	} else if (!strcmp(id, "attack")) {
	    if (anims->Attack) {
		free(anims->Attack);
	    }
	    anims->Attack = anim;
	} else if (!strcmp(id, "repair")) {
	    if (anims->Repair) {
		free(anims->Repair);
	    }
	    anims->Repair = anim;
	} else if (!strcmp(id, "harvest")) {
	    int res;

	    for (res = 0; res < MaxCosts; ++res) {
		if (!strcmp(resource, DefaultResourceNames[res])) {
		    break;
		}
	    }
	    if (res == MaxCosts) {
		lua_pushfstring(l, "Resource not found: %s", resource);
		lua_error(l);
	    }
	    if (anims->Harvest[res]) {
		free(anims->Harvest[res]);
	    }
	    anims->Harvest[res] = anim;
	} else if (!strcmp(id, "die")) {
	    if (anims->Die) {
		free(anims->Die);
	    }
	    anims->Die = anim;
	} else {
	    lua_pushfstring(l, "Unsupported tag: %s", id);
	    lua_error(l);
	}
    }

    *(Animations**)hash_add(AnimationsHash, str) = anims;
    return 0;
}
#endif

/*
**	Define boolean flag.
**
**	@param list : list of flags' name.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineBoolFlags(SCM list)
{
    char* str;
    int i;

    if (NumberBoolFlag != 0) {
        DebugLevel0("Warning, Redefine Bool flags\n");
    }
    while (!gh_null_p(list)) {
        str = gh_scm2newstr(gh_car(list), NULL);
        list = gh_cdr(list);
        for (i = 0; i < NumberBoolFlag; ++i) {
            if (!strcmp(str, BoolFlagName[i])) {
                DebugLevel0("Warning, Bool flags already defined\n");
                break;
            }
	}
        if (i != NumberBoolFlag) {
            break;
	}
        BoolFlagName = realloc(BoolFlagName, (NumberBoolFlag + 1) * sizeof(*BoolFlagName));
        BoolFlagName[NumberBoolFlag++] = str;
    }
    return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDefineBoolFlags(lua_State* l)
{
    char* str;
    int i;
    int args;
    int j;

    if (NumberBoolFlag != 0) {
        DebugLevel0("Warning, Redefine Bool flags\n");
    }
    args = lua_gettop(l);
    for (j = 0; j < args; ++j) {
	if (!lua_isstring(l, j + 1)) {
	    lua_pushstring(l, "incorrect argument");
	    lua_error(l);
	}
        str = strdup(lua_tostring(l, j + 1));
        for (i = 0; i < NumberBoolFlag; ++i) {
            if (!strcmp(str, BoolFlagName[i])) {
                DebugLevel0("Warning, Bool flags already defined\n");
                break;
            }
	}
        if (i != NumberBoolFlag) {
            break;
	}
        BoolFlagName = realloc(BoolFlagName, (NumberBoolFlag + 1) * sizeof(*BoolFlagName));
        BoolFlagName[NumberBoolFlag++] = str;
    }
    return 0;
}
#endif

// ----------------------------------------------------------------------------

/**
**	Register CCL features for unit-type.
*/
global void UnitTypeCclRegister(void)
{
#if defined(USE_GUILE) || defined(USE_SIOD)
    gh_new_procedureN("define-unit-type", CclDefineUnitType);
    gh_new_procedureN("define-unit-stats", CclDefineUnitStats);
    gh_new_procedureN("define-bool-flags", CclDefineBoolFlags);

    SiodUnitTypeTag = CclMakeSmobType("UnitType");

#ifndef USE_GUILE
    set_print_hooks(SiodUnitTypeTag, CclUnitTypePrin1);
#endif 

    gh_new_procedure1_0("unit-type", CclUnitType);
    gh_new_procedure0_0("unit-type-array", CclUnitTypeArray);
    // unit type structure access
    gh_new_procedure1_0("get-unit-type-ident", CclGetUnitTypeIdent);
    gh_new_procedure1_0("get-unit-type-name", CclGetUnitTypeName);
    gh_new_procedure2_0("set-unit-type-name!", CclSetUnitTypeName);

    // FIXME: write the missing access functions

    gh_new_procedure1_0("get-unit-type-property", CclGetUnitTypeProperty);
    gh_new_procedure2_0("set-unit-type-property!", CclSetUnitTypeProperty);

    gh_new_procedureN("define-unittype-wc-names", CclDefineUnitTypeWcNames);

    gh_new_procedureN("define-animations", CclDefineAnimations);
#elif defined(USE_LUA)
    lua_register(Lua, "DefineUnitType", CclDefineUnitType);
//    lua_register(Lua, "DefineUnitStats", CclDefineUnitStats);
    lua_register(Lua, "DefineBoolFlags", CclDefineBoolFlags);

//    SiodUnitTypeTag = CclMakeSmobType("UnitType");

//    lua_register(Lua, "UnitType", CclUnitType);
//    lua_register(Lua, "UnitTypeArray", CclUnitTypeArray);
    // unit type structure access
//    lua_register(Lua, "GetUnitTypeIdent", CclGetUnitTypeIdent);
//    lua_register(Lua, "GetUnitTypeName", CclGetUnitTypeName);
//    lua_register(Lua, "SetUnitTypeName", CclSetUnitTypeName);

    // FIXME: write the missing access functions

//    lua_register(Lua, "GetUnitTypeProperty", CclGetUnitTypeProperty);
//    lua_register(Lua, "SetUnitTypeProperty", CclSetUnitTypeProperty);

    lua_register(Lua, "DefineUnitTypeWcNames", CclDefineUnitTypeWcNames);

    lua_register(Lua, "DefineAnimations", CclDefineAnimations);

#endif
}

//@}
