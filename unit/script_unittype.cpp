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
--		Includes
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
--		Variables
----------------------------------------------------------------------------*/

#ifdef DEBUG
extern int NoWarningUnitType;				/// quiet ident lookup.
#endif

global _AnimationsHash AnimationsHash;		/// Animations hash table

#if defined(USE_GUILE) || defined(USE_SIOD)
local ccl_smob_type_t SiodUnitTypeTag;				/// siod unit-type object
#elif defined(USE_LUA)
#endif

global char** BoolFlagName = NULL;		/// Name of user defined flag
global int NumberBoolFlag = 0;				/// Number of defined flags.

/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

/**
** 		Get the resource ID from a SCM object.
**
** 		@param value		SCM thingie
**		@return 		the resource id
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

	value = LuaToString(l, -1);
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
**		Parse unit-type.
**
**		@note Should write a general parser for this.
**
**		@param list		List describing the unit-type.
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

	//		Slot identifier

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

	//
	//		Parse the list:		(still everything could be changed!)
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
					   // This leaves half initialized unit-type
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
			//	Warning: can-cast-spell should only be used AFTER all spells
			//	have been defined. FIXME: MaxSpellType=500 or something?
			//
			if (!type->CanCastSpell) {
				type->CanCastSpell = malloc(SpellTypeCount);
				memset(type->CanCastSpell, 0, SpellTypeCount);
			}
			sublist = gh_car(list);
			list = gh_cdr(list);
			if (gh_null_p(sublist)) { // empty list
				free(type->CanCastSpell);
				type->CanCastSpell = NULL;
			}
			while (!gh_null_p(sublist)) {
				int id;
				id = CclGetSpellByIdent(gh_car(sublist));
				DebugLevel3Fn("%d \n" _C_ id);
				if (id == -1) {
					errl("Unknown spell type", gh_car(sublist));
				}
				type->CanCastSpell[id] = 1;
				sublist = gh_cdr(sublist);
			}
		} else if (gh_eq_p(value, gh_symbol2scm("can-target-flag"))) {
			//
			//	Warning: can-target-flag should only be used AFTER all bool flags
			//	have been defined.
			//
			sublist = gh_car(list);
			list = gh_cdr(list);
			while (!gh_null_p(sublist)) {
				value = gh_car(sublist);
				sublist = gh_cdr(sublist);
				for (i = 0; i < NumberBoolFlag; ++i) {
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
			// This leaves a half initialized unit-type
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
	UnitType* type;
	UnitType* auxtype;
	ResourceInfo* res;
	char* str;
	int i;
	int redefine;
	int subargs;
	int k;

	if (lua_gettop(l) != 2 || !lua_istable(l, 2)) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	//		Slot identifier
	str = strdup(LuaToString(l, 1));

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

	//
	//		Parse the list:		(still everything could be changed!)
	//
	lua_pushnil(l);
	while (lua_next(l, 2)) {
		value = LuaToString(l, -2);
		if (!strcmp(value, "Name")) {
			if (redefine) {
				free(type->Name);
			}
			type->Name = strdup(LuaToString(l, -1));
		} else if (!strcmp(value, "Use")) {
			if (redefine) {
				free(type->SameSprite);
			}
			type->SameSprite = strdup(LuaToString(l, -1));
		} else if (!strcmp(value, "Files")) {
			if (!lua_istable(l, -1)) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				value = LuaToString(l, -1);
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
					   // This leaves half initialized unit-type
					   lua_pushfstring(l, "Unsupported tileset tag", value);
					   lua_error(l);
					}
				}
				if (redefine) {
					free(type->File[i]);
				}
				lua_rawgeti(l, -1, k + 1);
				type->File[i] = strdup(LuaToString(l, -1));
				lua_pop(l, 1);
			}
		} else if (!strcmp(value, "Shadow")) {
			if (!lua_istable(l, -1)) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;

				if (!strcmp(value, "file")) {
					if (redefine) {
						free(type->ShadowFile);
					}
					lua_rawgeti(l, -1, k + 1);
					type->ShadowFile = strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				} else if (!strcmp(value, "size")) {
					lua_rawgeti(l, -1, k + 1);
					if (!lua_istable(l, -1)) {
						lua_pushstring(l, "incorrect argument");
						lua_error(l);
					}
					lua_rawgeti(l, -1, 1);
					type->ShadowWidth = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_rawgeti(l, -1, 2);
					type->ShadowHeight = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_pop(l, 1);
				} else if (!strcmp(value, "height")) {
				} else if (!strcmp(value, "offset")) {
					lua_rawgeti(l, -1, k + 1);
					if (!lua_istable(l, -1)) {
						lua_pushstring(l, "incorrect argument");
						lua_error(l);
					}
					lua_rawgeti(l, -1, 1);
					type->ShadowOffsetX = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_rawgeti(l, -1, 2);
					type->ShadowOffsetY = LuaToNumber(l, -1);
					lua_pop(l, 1);
					lua_pop(l, 1);
				} else {
					lua_pushfstring(l, "Unsupported shadow tag: %s", value);
					lua_error(l);
				}
			}
		} else if (!strcmp(value, "Size")) {
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			lua_rawgeti(l, -1, 1);
			type->Width = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			type->Height = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "Animations")) {
			type->Animations = AnimationsByIdent(LuaToString(l, -1));
		} else if (!strcmp(value, "Icon")) {
			if (redefine) {
				free(type->Icon.Name);
			}
			type->Icon.Name = strdup(LuaToString(l, -1));
			type->Icon.Icon = NULL;
		} else if (!strcmp(value, "Costs")) {
			if (!lua_istable(l, -1)) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				int res;

				lua_rawgeti(l, -1, k + 1);
				res = CclGetResourceByName(l);
				lua_pop(l, 1);
				++k;
				lua_rawgeti(l, -1, k + 1);
				type->_Costs[res] = LuaToNumber(l, -1);
				lua_pop(l, 1);
			}
		} else if (!strcmp(value, "ImproveProduction")) {
			if (!lua_istable(l, -1)) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				int res;

				lua_rawgeti(l, -1, k + 1);
				res = CclGetResourceByName(l);
				lua_pop(l, 1);
				++k;
				lua_rawgeti(l, -1, k + 1);
				type->ImproveIncomes[res] = DefaultIncomes[res] + LuaToNumber(l, -1);
				lua_pop(l, 1);
			}
		} else if (!strcmp(value, "Construction")) {
			// FIXME: What if constructions aren't yet loaded?
			type->Construction = ConstructionByIdent(LuaToString(l, -1));
		} else if (!strcmp(value, "Speed")) {
			type->_Speed = LuaToNumber(l, -1);
		} else if (!strcmp(value, "DrawLevel")) {
			type->DrawLevel = LuaToNumber(l, -1);
		} else if (!strcmp(value, "MaxOnBoard")) {
			type->MaxOnBoard = LuaToNumber(l, -1);
		} else if (!strcmp(value, "HitPoints")) {
			type->_HitPoints = LuaToNumber(l, -1);
		} else if (!strcmp(value, "RegenerationRate")) {
			type->_RegenerationRate = LuaToNumber(l, -1);
		} else if (!strcmp(value, "BurnPercent")) {
			type->BurnPercent = LuaToNumber(l, -1);
		} else if (!strcmp(value, "BurnDamageRate")) {
			type->BurnDamageRate = LuaToNumber(l, -1);
		} else if (!strcmp(value, "MaxMana")) {
			type->_MaxMana = LuaToNumber(l, -1);
		} else if (!strcmp(value, "TileSize")) {
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			lua_rawgeti(l, -1, 1);
			type->TileWidth = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			type->TileHeight = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "MustBuildOnTop")) {
			value = LuaToString(l, -1);
			auxtype = UnitTypeByIdent(value);
			if (!auxtype) {
				DebugLevel0("Build on top of undefined unit \"%s\".\n" _C_ str);
				DebugCheck(1);
			}
			type->MustBuildOnTop = auxtype;
		} else if (!strcmp(value, "Selectable")) {
			type->Selectable = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "NeutralMinimapColor")) {
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 3) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
#ifdef USE_SDL_SURFACE
			lua_rawgeti(l, -1, 1);
			type->NeutralMinimapColorRGB.r = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			type->NeutralMinimapColorRGB.g = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 3);
			type->NeutralMinimapColorRGB.b = LuaToNumber(l, -1);
			lua_pop(l, 1);
#else
			lua_rawgeti(l, -1, 1);
			type->NeutralMinimapColorRGB.D24.a = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			type->NeutralMinimapColorRGB.D24.b = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 3);
			type->NeutralMinimapColorRGB.D24.c = LuaToNumber(l, -1);
			lua_pop(l, 1);
#endif
		} else if (!strcmp(value, "BoxSize")) {
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			lua_rawgeti(l, -1, 1);
			type->BoxWidth = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			type->BoxHeight = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "NumDirections")) {
			type->NumDirections = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Revealer")) {
			type->Revealer = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "SightRange")) {
			type->_SightRange = LuaToNumber(l, -1);
		} else if (!strcmp(value, "ComputerReactionRange")) {
			type->ReactRangeComputer = LuaToNumber(l, -1);
		} else if (!strcmp(value, "PersonReactionRange")) {
			type->ReactRangePerson = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Armor")) {
			type->_Armor = LuaToNumber(l, -1);
		} else if (!strcmp(value, "BasicDamage")) {
			type->_BasicDamage = LuaToNumber(l, -1);
		} else if (!strcmp(value, "PiercingDamage")) {
			type->_PiercingDamage = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Missile")) {
			type->Missile.Name = strdup(LuaToString(l, -1));
			type->Missile.Missile = NULL;
		} else if (!strcmp(value, "MinAttackRange")) {
			type->MinAttackRange = LuaToNumber(l, -1);
		} else if (!strcmp(value, "MaxAttackRange")) {
			type->_AttackRange = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Priority")) {
			type->Priority = LuaToNumber(l, -1);
		} else if (!strcmp(value, "AnnoyComputerFactor")) {
			type->AnnoyComputerFactor = LuaToNumber(l, -1);
		} else if (!strcmp(value, "DecayRate")) {
			type->DecayRate = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Points")) {
			type->Points = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Demand")) {
			type->Demand = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Supply")) {
			type->Supply = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Corpse")) {
			if (!lua_istable(l, -1) || luaL_getn(l, -1) != 2) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			if (redefine) {
				free(type->CorpseName);
			}
			lua_rawgeti(l, -1, 1);
			type->CorpseName = strdup(LuaToString(l, -1));
			lua_pop(l, 1);
			type->CorpseType = NULL;
			lua_rawgeti(l, -1, 2);
			type->CorpseScript = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "ExplodeWhenKilled")) {
			type->ExplodeWhenKilled = 1;
			type->Explosion.Name = strdup(LuaToString(l, -1));
			type->Explosion.Missile = NULL;
		} else if (!strcmp(value, "Type")) {
			value = LuaToString(l, -1);
			if (!strcmp(value, "land")) {
				type->UnitType = UnitTypeLand;
			} else if (!strcmp(value, "fly")) {
				type->UnitType = UnitTypeFly;
			} else if (!strcmp(value, "naval")) {
				type->UnitType = UnitTypeNaval;
			} else {
				lua_pushfstring(l, "Unsupported Type: %s", value);
				lua_error(l);
			}

		} else if (!strcmp(value, "RightMouseAction")) {
			value = LuaToString(l, -1);
			if (!strcmp(value, "none")) {
				type->MouseAction = MouseActionNone;
			} else if (!strcmp(value, "attack")) {
				type->MouseAction = MouseActionAttack;
			} else if (!strcmp(value, "move")) {
				type->MouseAction = MouseActionMove;
			} else if (!strcmp(value, "harvest")) {
				type->MouseAction = MouseActionHarvest;
			} else if (!strcmp(value, "spell-cast")) {
				type->MouseAction = MouseActionSpellCast;
			} else if (!strcmp(value, "sail")) {
				type->MouseAction = MouseActionSail;
			} else {
				lua_pushfstring(l, "Unsupported RightMouseAction: %s", value);
				lua_error(l);
			}

		} else if (!strcmp(value, "CanGroundAttack")) {
			type->GroundAttack = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "CanAttack")) {
			type->CanAttack = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "RepairRange")) {
			type->RepairRange = LuaToNumber(l, -1);
		} else if (!strcmp(value, "RepairHp")) {
			type->RepairHP = LuaToNumber(l, -1);
		} else if (!strcmp(value, "RepairCosts")) {
			if (!lua_istable(l, -1)) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				int res;

				lua_rawgeti(l, -1, k + 1);
				res = CclGetResourceByName(l);
				lua_pop(l, 1);
				++k;
				lua_rawgeti(l, -1, k + 1);
				type->RepairCosts[res] = LuaToNumber(l, -1);
				lua_pop(l, 1);
			}
		} else if (!strcmp(value, "CanTargetLand")) {
			if (LuaToBoolean(l, -1)) {
				type->CanTarget |= CanTargetLand;
			} else {
				type->CanTarget &= ~CanTargetLand;
			}
		} else if (!strcmp(value, "CanTargetSea")) {
			if (LuaToBoolean(l, -1)) {
				type->CanTarget |= CanTargetSea;
			} else {
				type->CanTarget &= ~CanTargetSea;
			}
		} else if (!strcmp(value, "CanTargetAir")) {
			if (LuaToBoolean(l, -1)) {
				type->CanTarget |= CanTargetAir;
			} else {
				type->CanTarget &= ~CanTargetAir;
			}

		} else if (!strcmp(value, "Building")) {
			type->Building = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "VisibleUnderFog")) {
			type->VisibleUnderFog = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "BuilderOutside")) {
			type->BuilderOutside = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "BuilderLost")) {
			type->BuilderLost = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "AutoBuildRate")) {
			type->AutoBuildRate = LuaToNumber(l, -1);
		} else if (!strcmp(value, "ShoreBuilding")) {
			type->ShoreBuilding = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "LandUnit")) {
			type->LandUnit = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "AirUnit")) {
			type->AirUnit = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "SeaUnit")) {
			type->SeaUnit = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "RandomMovementProbability")) {
			type->RandomMovementProbability = LuaToNumber(l, -1);
		} else if (!strcmp(value, "ClicksToExplode")) {
			type->ClicksToExplode = LuaToNumber(l, -1);
		} else if (!strcmp(value, "PermanentCloak")) {
			type->PermanentCloak = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "DetectCloak")) {
			type->DetectCloak = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "Transporter")) {
			type->Transporter = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "Coward")) {
			type->Coward = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "CanGatherResources")) {
			int args;
			int j;

			args = luaL_getn(l, -1);
			for (j = 0; j < args; ++j) {
				lua_rawgeti(l, -1, j + 1);
				res = (ResourceInfo*)malloc(sizeof(ResourceInfo));
				memset(res, 0, sizeof(ResourceInfo));
				if (!lua_istable(l, -1)) {
					lua_pushstring(l, "incorrect argument");
					lua_error(l);
				}
				subargs = luaL_getn(l, -1);
				for (k = 0; k < subargs; ++k) {
					lua_rawgeti(l, -1, k + 1);
					value = LuaToString(l, -1);
					lua_pop(l, 1);
					++k;
					if (!strcmp(value, "resource-id")) {
						lua_rawgeti(l, -1, k + 1);
						res->ResourceId = CclGetResourceByName(l);
						lua_pop(l, 1);
						type->ResInfo[res->ResourceId] = res;
					} else if (!strcmp(value, "resource-step")) {
						lua_rawgeti(l, -1, k + 1);
						res->ResourceStep = LuaToNumber(l, -1);
						lua_pop(l, 1);
					} else if (!strcmp(value, "final-resource")) {
						lua_rawgeti(l, -1, k + 1);
						res->FinalResource = CclGetResourceByName(l);
						lua_pop(l, 1);
					} else if (!strcmp(value, "wait-at-resource")) {
						lua_rawgeti(l, -1, k + 1);
						res->WaitAtResource = LuaToNumber(l, -1);
						lua_pop(l, 1);
					} else if (!strcmp(value, "wait-at-depot")) {
						lua_rawgeti(l, -1, k + 1);
						res->WaitAtDepot = LuaToNumber(l, -1);
						lua_pop(l, 1);
					} else if (!strcmp(value, "resource-capacity")) {
						lua_rawgeti(l, -1, k + 1);
						res->ResourceCapacity = LuaToNumber(l, -1);
						lua_pop(l, 1);
					} else if (!strcmp(value, "terrain-harvester")) {
						res->TerrainHarvester = 1;
						--k;
					} else if (!strcmp(value, "lose-resources")) {
						res->LoseResources = 1;
						--k;
					} else if (!strcmp(value, "harvest-from-outside")) {
						res->HarvestFromOutside = 1;
						--k;
					} else if (!strcmp(value, "file-when-empty")) {
						lua_rawgeti(l, -1, k + 1);
						res->FileWhenEmpty = strdup(LuaToString(l, -1));
						lua_pop(l, 1);
					} else if (!strcmp(value, "file-when-loaded")) {
						lua_rawgeti(l, -1, k + 1);
						res->FileWhenLoaded = strdup(LuaToString(l, -1));
						lua_pop(l, 1);
					} else {
					   printf("\n%s\n",type->Name);
					   lua_pushfstring(l, "Unsupported tag: %s", value);
					   lua_error(l);
					   DebugCheck(1);
					}
				}
				if (!res->FinalResource) {
					res->FinalResource = res->ResourceId;
				}
				DebugCheck(!res->ResourceId);
				lua_pop(l, 1);
			}
			type->Harvester = 1;
		} else if (!strcmp(value, "GivesResource")) {
			lua_pushvalue(l, -1);
			type->GivesResource = CclGetResourceByName(l);
			lua_pop(l, 1);
		} else if (!strcmp(value, "CanHarvest")) {
			type->CanHarvest = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "CanStore")) {
			if (!lua_istable(l, -1)) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				type->CanStore[CclGetResourceByName(l)] = 1;
				lua_pop(l, 1);
			}
		} else if (!strcmp(value, "Vanishes")) {
			type->Vanishes = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "CanCastSpell")) {
			if (!lua_istable(l, -1)) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			//
			//	Warning: can-cast-spell should only be used AFTER all spells
			//	have been defined. FIXME: MaxSpellType=500 or something?
			//
			if (!type->CanCastSpell) {
				type->CanCastSpell = malloc(SpellTypeCount);
				memset(type->CanCastSpell, 0, SpellTypeCount);
			}
			subargs = luaL_getn(l, -1);
			if (subargs == 0) {
				free(type->CanCastSpell);
				type->CanCastSpell = NULL;

			}
			for (k = 0; k < subargs; ++k) {
				int id;

				lua_rawgeti(l, -1, k + 1);
				value = LuaToString(l, -1);
				id = CclGetSpellByIdent(l);
				lua_pop(l, 1);
				DebugLevel3Fn("%d \n" _C_ id);
				if (id == -1) {
					lua_pushfstring(l, "Unknown spell type: %s", value);
					lua_error(l);
				}
				type->CanCastSpell[id] = 1;
			}
		} else if (!strcmp(value, "CanTargetFlag")) {
			//
			//	Warning: can-target-flag should only be used AFTER all bool flags
			//	have been defined.
			//
			if (!lua_istable(l, -1)) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;
				for (i = 0; i < NumberBoolFlag; ++i) {
					if (!strcmp(value, BoolFlagName[i])) {
						lua_rawgeti(l, -1, k + 1);
						value = LuaToString(l, -1);
						lua_pop(l, 1);
						type->CanTargetFlag[i] = Ccl2Condition(l, value);
						break;
					}
				}
				if (i != NumberBoolFlag) {
					continue;
				}
				printf("\n%s\n", type->Name);
				lua_pushfstring(l, "Unsupported flag tag for can-target-flag: %s", value);
				lua_error(l);
			}
		} else if (!strcmp(value, "SelectableByRectangle")) {
			type->SelectableByRectangle = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "Teleporter")) {
			type->Teleporter = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "Sounds")) {
			if (!lua_istable(l, -1)) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;

				if (!strcmp(value, "selected")) {
					if (redefine) {
						free(type->Sound.Selected.Name);
					}
					lua_rawgeti(l, -1, k + 1);
					type->Sound.Selected.Name = strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				} else if (!strcmp(value, "acknowledge")) {
					if (redefine) {
						free(type->Sound.Acknowledgement.Name);
					}
					lua_rawgeti(l, -1, k + 1);
					type->Sound.Acknowledgement.Name = strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				} else if (!strcmp(value, "ready")) {
					if (redefine) {
						free(type->Sound.Ready.Name);
					}
					lua_rawgeti(l, -1, k + 1);
					type->Sound.Ready.Name = strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				} else if (!strcmp(value, "repair")) {
					if (redefine) {
						free(type->Sound.Repair.Name);
					}
					lua_rawgeti(l, -1, k + 1);
					type->Sound.Repair.Name = strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				} else if (!strcmp(value, "harvest")) {
					int res;
					const char* name;

					lua_rawgeti(l, -1, k + 1);
					name = LuaToString(l, -1 );
					lua_pop(l, 1);
					++k;
					for (res = 0; res < MaxCosts; ++res) {
						if (!strcmp(name, DefaultResourceNames[res])) {
							break;
						}
					}
					if (res == MaxCosts) {
						lua_pushfstring(l, "Resource not found: %s", value);
						lua_error(l);
					}
					if (redefine) {
						free(type->Sound.Harvest[res].Name);
					}
					lua_rawgeti(l, -1, k + 1);
					type->Sound.Harvest[res].Name = strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				} else if (!strcmp(value, "help")) {
					if (redefine) {
						free(type->Sound.Help.Name);
					}
					lua_rawgeti(l, -1, k + 1);
					type->Sound.Help.Name = strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				} else if (!strcmp(value, "dead")) {
					if (redefine) {
						free(type->Sound.Dead.Name);
					}
					lua_rawgeti(l, -1, k + 1);
					type->Sound.Dead.Name = strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				} else if (!strcmp(value, "attack")) {
					if (redefine) {
						free(type->Weapon.Attack.Name);
					}
					lua_rawgeti(l, -1, k + 1);
					type->Weapon.Attack.Name = strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				} else {
					lua_pushfstring(l, "Unsupported sound tag: %s", value);
					lua_error(l);
				}
			}
		} else {
			for (i = 0; i < NumberBoolFlag; ++i) { // User defined bool flags
				if (!strcmp(value, BoolFlagName[i])) {
					type->BoolFlag[i] = LuaToBoolean(l, -1);
					break;
				}
			}
			if (i == NumberBoolFlag) {
				printf("\n%s\n",type->Name);
				lua_pushfstring(l, "Unsupported tag: %s", value);
				lua_error(l);
				DebugCheck(1);
			}
		}
		lua_pop(l, 1);
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
**		Parse unit-stats.
**
**		@param list		List describing the unit-stats.
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
	//		Parse the list:		(still everything could be changed!)
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
				   // This leaves half initialized stats
				   errl("Unsupported tag", value);
				}
				sublist = gh_cdr(sublist);
			}
		} else {
		   // This leaves a half initialized unit
		   errl("Unsupported tag", value);
		}
	}

	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDefineUnitStats(lua_State* l)
{
	const char* value;
	UnitType* type;
	UnitStats* stats;
	int i;
	int args;
	int j;

	args = lua_gettop(l);
	j = 0;

	type = UnitTypeByIdent(LuaToString(l, j + 1));
	DebugCheck(!type);
	++j;

	i = LuaToNumber(l, j + 1);
	DebugCheck(i >= PlayerMax);
	++j;

	stats = &type->Stats[i];

	//
	//		Parse the list:		(still everything could be changed!)
	//
	for (; j < args; ++j) {

		value = LuaToString(l, j + 1);
		++j;

		if (!strcmp(value, "level")) {
			stats->Level = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "speed")) {
			stats->Speed = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "attack-range")) {
			stats->AttackRange = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "sight-range")) {
			stats->SightRange = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "armor")) {
			stats->Armor = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "basic-damage")) {
			stats->BasicDamage = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "piercing-damage")) {
			stats->PiercingDamage = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "hit-points")) {
			stats->HitPoints = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "regeneration-rate")) {
			stats->RegenerationRate = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "costs")) {
			int subargs;
			int k;

			if (!lua_istable(l, j + 1)) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
			}
			subargs = luaL_getn(l, j + 1);
			for (k = 0; k < subargs; ++k) {

				lua_rawgeti(l, j + 1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;

				for (i = 0; i < MaxCosts; ++i) {
					if (!strcmp(value, DefaultResourceNames[i])) {
						lua_rawgeti(l, j + 1, k + 1);
						stats->Costs[i] = LuaToNumber(l, -1);
						lua_pop(l, 1);
						break;
					}
				}
				if (i == MaxCosts) {
				   // This leaves half initialized stats
				   lua_pushfstring(l, "Unsupported tag: %s", value);
				   lua_error(l);
				}
			}
		} else {
		   // This leaves a half initialized unit
		   lua_pushfstring(l, "Unsupported tag: %s", value);
		   lua_error(l);
		}
	}

	return 0;
}
#endif

// ----------------------------------------------------------------------------

/**
**		Access unit-type object
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
		errl("CclGetUnitType: not a unit-type", ptr);
		return 0;
	}
}
#elif defined(USE_LUA)
global UnitType* CclGetUnitType(lua_State* l)
{
	const char* str;

	// Be kind allow also strings or symbols
	if (lua_isstring(l, -1)) {
		str = LuaToString(l, -1);
		DebugLevel3("CclGetUnitType: %s\n"_C_ str);
		return UnitTypeByIdent(str);
	} else if (lua_isuserdata(l, -1)) {
		LuaUserData* data;
		data = lua_touserdata(l, -1);
		if (data->Type == LuaUnitType) {
			return data->Data;
		}
	}
	lua_pushfstring(l, "CclGetUnitType: not a unit-type");
	lua_error(l);
	return NULL;
}
#endif

/**
**		Print the unit-type object
**
**		@param ptr		Scheme object.
**		@param f		Output structure.
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
#endif

/**
**		Get unit-type structure.
**
**		@param ident		Identifier for unit-type.
**
**		@return				Unit-type structure.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclUnitType(SCM ident)
{
	char* str;
	UnitType* type;

	str = CclConvertToString(ident);
	if (str) {
		type = UnitTypeByIdent(str);
		DebugLevel3Fn("CclUnitType: '%s' -> '%ld'\n" _C_ str _C_ (long)type);
		free(str);
		return CclMakeSmobObj(SiodUnitTypeTag, type);
	} else {
		errl("CclUnitType: no unittype by ident: ", ident);
		return SCM_BOOL_F;
	}
}
#elif defined(USE_LUA)
local int CclUnitType(lua_State* l)
{
	const char* str;
	UnitType* type;
	LuaUserData* data;

	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	str = LuaToString(l, 1);
	type = UnitTypeByIdent(str);
	DebugLevel3Fn("CclUnitType: '%s' -> '%ld'\n" _C_ str _C_ (long)type);
	data = lua_newuserdata(l, sizeof(LuaUserData));
	data->Type = LuaUnitType;
	data->Data = type;
	return 1;
}
#endif

/**
**		Get all unit-type structures.
**
**		@return				An array of all unit-type structures.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclUnitTypeArray(void)
{
	SCM array;
	SCM value;
	int i;

	array = cons_array(gh_int2scm(NumUnitTypes), NIL);

	for (i = 0; i < NumUnitTypes; ++i) {
		value = CclMakeSmobObj(SiodUnitTypeTag, &UnitTypes[i]);
		gh_vector_set_x(array, gh_int2scm(i), value);
	}
	return array;
}
#elif defined(USE_LUA)
local int CclUnitTypeArray(lua_State* l)
{
	int i;
	LuaUserData* data;

	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	lua_newtable(l);

	for (i = 0; i < NumUnitTypes; ++i) {
		data = lua_newuserdata(l, sizeof(LuaUserData));
		data->Type = LuaUnitType;
		data->Data = UnitTypes[i];
		lua_rawseti(l, 1, i + 1);
	}
	return 1;
}
#endif

/**
**		Get the ident of the unit-type structure.
**
**		@param ptr		Unit-type object.
**
**		@return				The identifier of the unit-type.
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
local int CclGetUnitTypeIdent(lua_State* l)
{
	const UnitType* type;

	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	type = CclGetUnitType(l);
	lua_pushstring(l, type->Ident);
	return 1;
}
#endif

/**
**		Get the name of the unit-type structure.
**
**		@param ptr		Unit-type object.
**
**		@return				The name of the unit-type.
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
local int CclGetUnitTypeName(lua_State* l)
{
	const UnitType* type;

	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	type = CclGetUnitType(l);
	lua_pushstring(l, type->Name);
	return 1;
}
#endif

/**
**		Set the name of the unit-type structure.
**
**		@param ptr		Unit-type object.
**		@param name		The name to set.
**
**		@return				The name of the unit-type.
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
local int CclSetUnitTypeName(lua_State* l)
{
	UnitType* type;

	if (lua_gettop(l) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	lua_pushvalue(l, 1);
	type = CclGetUnitType(l);
	lua_pop(l, 1);
	free(type->Name);
	type->Name = strdup(LuaToString(l, 2));

	lua_pushvalue(l, 2);
	return 1;
}
#endif

/**
**		Define tileset mapping from original number to internal symbol
**
**		@param list		List of all names.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineUnitTypeWcNames(SCM list)
{
	int i;
	char** cp;

	if ((cp = UnitTypeWcNames)) {				// Free all old names
		while (*cp) {
			free(*cp++);
		}
		free(UnitTypeWcNames);
	}

	//
	//		Get new table.
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

	if ((cp = UnitTypeWcNames)) {		// Free all old names
		while (*cp) {
			free(*cp++);
		}
		free(UnitTypeWcNames);
	}

	//
	//		Get new table.
	//
	i = lua_gettop(l);
	UnitTypeWcNames = cp = malloc((i + 1) * sizeof(char*));
	if (!cp) {
		fprintf(stderr, "out of memory.\n");
		ExitFatal(-1);
	}

	for (j = 0; j < i; ++j) {
		*cp++ = strdup(LuaToString(l, j + 1));
	}
	*cp = NULL;

	return 0;
}
#endif

// ----------------------------------------------------------------------------

/**
**		Define an unit-type animations set.
**
**		@param list		Animations list.
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
		t[-1].Flags |= 0x80;				// Marks end of list

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
	str = LuaToString(l, j + 1);
	++j;
	anims = calloc(1, sizeof(Animations));

	for (; j < args; ++j) {
		id = LuaToString(l, j + 1);
		++j;
		if (!strcmp(id, "harvest")) {
			resource = LuaToString(l, j + 1);
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
			t->Flags = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			t->Pixel = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 3);
			t->Sleep = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 4);
			i = LuaToNumber(l, -1);
			lua_pop(l, 1);
			t->Frame = i - frame;
			frame = i;
			if (t->Flags & AnimationRestart) {
				frame = 0;
			}
			++t;
			lua_pop(l, 1);
		}
		t[-1].Flags |= 0x80;				// Marks end of list

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
**		Define boolean flag.
**
**		@param list : list of flags' name.
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
		str = strdup(LuaToString(l, j + 1));
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
**		Register CCL features for unit-type.
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

	gh_new_procedureN("define-unittype-wc-names", CclDefineUnitTypeWcNames);

	gh_new_procedureN("define-animations", CclDefineAnimations);
#elif defined(USE_LUA)
	lua_register(Lua, "DefineUnitType", CclDefineUnitType);
	lua_register(Lua, "DefineUnitStats", CclDefineUnitStats);
	lua_register(Lua, "DefineBoolFlags", CclDefineBoolFlags);

	lua_register(Lua, "UnitType", CclUnitType);
	lua_register(Lua, "UnitTypeArray", CclUnitTypeArray);
	// unit type structure access
	lua_register(Lua, "GetUnitTypeIdent", CclGetUnitTypeIdent);
	lua_register(Lua, "GetUnitTypeName", CclGetUnitTypeName);
	lua_register(Lua, "SetUnitTypeName", CclSetUnitTypeName);

	lua_register(Lua, "DefineUnitTypeWcNames", CclDefineUnitTypeWcNames);

	lua_register(Lua, "DefineAnimations", CclDefineAnimations);

#endif
}

//@}
