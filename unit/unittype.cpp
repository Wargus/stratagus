//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name unittype.c - The unit types. */
//
//      (c) Copyright 1998-2004 by Lutz Sammer and Jimmy Salmon
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
//      $Id$

//@{

/*----------------------------------------------------------------------------
--  Includes
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
#include "script.h"
#include "spells.h"

#include "etlib/hash.h"

#include "myendian.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/**
**		Unit-type type definition
*/
global const char UnitTypeType[] = "unit-type";

#ifdef DEBUG
global int NoWarningUnitType;						/// quiet ident lookup
#endif

global UnitType* UnitTypes[UnitTypeMax];		/// unit-types definition
global int NumUnitTypes;						/// number of unit-types made

/*
**		Next unit type are used hardcoded in the source.
**
**		FIXME: find a way to make it configurable!
*/
global UnitType* UnitTypeHumanWall;				/// Human wall
global UnitType* UnitTypeOrcWall;				/// Orc wall

/**
**		Mapping of W*rCr*ft number to our internal unit-type symbol.
**		The numbers are used in puds.
*/
global char** UnitTypeWcNames;

#ifdef DOXYGEN						  // no real code, only for document

/**
**		Lookup table for unit-type names
*/
local UnitType* UnitTypeHash[UnitTypeMax];

#else

/**
**		Lookup table for unit-type names
*/
local hashtable(UnitType*, UnitTypeMax) UnitTypeHash;

#endif

/**
**		Default resources for a new player.
*/
global int DefaultResources[MaxCosts];

/**
**		Default resources for a new player with low resources.
*/
global int DefaultResourcesLow[MaxCosts];

/**
**		Default resources for a new player with mid resources.
*/
global int DefaultResourcesMedium[MaxCosts];

/**
**		Default resources for a new player with high resources.
*/
global int DefaultResourcesHigh[MaxCosts];

/**
**		Default incomes for a new player.
*/
global int DefaultIncomes[MaxCosts];

/**
**		Default action for the resources.
*/
global char* DefaultActions[MaxCosts];

/**
**		Default names for the resources.
*/
global char* DefaultResourceNames[MaxCosts];

/**
**		Default amounts for the resources.
*/
global int DefaultResourceAmounts[MaxCosts];

/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

/**
**		Update the player stats for changed unit types.
**	  @param reset indicates wether default value should be set to each stat (level, upgrades)
*/
global void UpdateStats(int reset)
{
	UnitType* type;
	UnitStats* stats;
	int player;
	unsigned i;
	int j;

	//
	//  Update players stats
	//
	for (j = 0; j<NumUnitTypes; ++j) {
		type = UnitTypes[j];
		if (reset) {
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
					stats->Level = 0;		// Disables level display
				} else {
					stats->Level = 1;
				}
			}
		}

		//
		//	  As side effect we calculate the movement flags/mask here.
		//
		switch (type->UnitType) {
			case UnitTypeLand:				// on land
				type->MovementMask =
					MapFieldLandUnit |
					MapFieldSeaUnit |
					MapFieldBuilding |		// already occuppied
					MapFieldCoastAllowed |
					MapFieldWaterAllowed |		// can't move on this
					MapFieldUnpassable;
				break;
			case UnitTypeFly:				// in air
				type->MovementMask =
					MapFieldAirUnit;		// already occuppied
				break;
			case UnitTypeNaval:				// on water
				if (type->Transporter) {
					type->MovementMask =
						MapFieldLandUnit |
						MapFieldSeaUnit |
						MapFieldBuilding |		// already occuppied
						MapFieldLandAllowed;		// can't move on this
					// Johns: MapFieldUnpassable only for land units?
				} else {
					type->MovementMask =
						MapFieldLandUnit |
						MapFieldSeaUnit |
						MapFieldBuilding |		// already occuppied
						MapFieldCoastAllowed |
						MapFieldLandAllowed |		// can't move on this
						MapFieldUnpassable;
				}
				break;
			default:
				DebugLevel1Fn("Where moves this unit?\n");
				type->MovementMask = 0;
				break;
		}
		if (type->Building || type->ShoreBuilding) {
			// Shore building is something special.
			if (type->ShoreBuilding) {
				type->MovementMask =
					MapFieldLandUnit |
					MapFieldSeaUnit |
					MapFieldBuilding |		// already occuppied
					MapFieldLandAllowed;		// can't build on this
			}
			type->MovementMask |= MapFieldNoBuilding;
			//
			//		A little chaos, buildings without HP can be entered.
			//		The oil-patch is a very special case.
			//
			if (type->_HitPoints) {
				type->FieldFlags = MapFieldBuilding;
			} else {
				type->FieldFlags = MapFieldNoBuilding;
			}
		} else switch (type->UnitType) {
			case UnitTypeLand:				// on land
				type->FieldFlags = MapFieldLandUnit;
				break;
			case UnitTypeFly:				// in air
				type->FieldFlags = MapFieldAirUnit;
				break;
			case UnitTypeNaval:				// on water
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
**		Parse UDTA area from puds.
**
**		@param udta		Pointer to udta area.
**		@param length		length of udta area.
*/
global void ParsePudUDTA(const char* udta, int length __attribute__((unused)))
{
	int i;
	int v;
	const char* start;
	UnitType* unittype;

	// FIXME: not the fastest, remove UnitTypeByWcNum from loops!
#ifdef DEBUG
	if (length != 5694 && length != 5948) {
		DebugLevel0("\n***\n");
		DebugLevel0Fn("%d\n" _C_ length);
		DebugLevel0("***\n\n");
	}
#endif
	start = udta;

	for (i = 0; i < 110; ++i) {				// overlap frames
		unittype = UnitTypeByWcNum(i);
		v = FetchLE16(udta);
		unittype->Construction = ConstructionByWcNum(v);
	}
	for (i = 0; i < 508; ++i) {				// skip obsolete data
		v = FetchLE16(udta);
	}
	for (i = 0; i < 110; ++i) {				// sight range
		unittype = UnitTypeByWcNum(i);
		v = FetchLE32(udta);
		unittype->_SightRange = v;
	}
	for (i = 0; i < 110; ++i) {				// hit points
		unittype = UnitTypeByWcNum(i);
		v = FetchLE16(udta);
		unittype->_HitPoints = v;
	}
	for (i = 0; i < 110; ++i) {				// Flag if unit is magic
//		unittype = UnitTypeByWcNum(i);
//		v = Fetch8(udta);
//		unittype->Magic = v;
		Fetch8(udta);
	}
	for (i = 0; i < 110; ++i) {				// Build time * 6 = one second FRAMES
		unittype = UnitTypeByWcNum(i);
		v = Fetch8(udta);
		unittype->_Costs[TimeCost] = v;
	}
	for (i = 0; i < 110; ++i) {				// Gold cost / 10
		unittype = UnitTypeByWcNum(i);
		v = Fetch8(udta);
		unittype->_Costs[GoldCost] = v * 10;
	}
	for (i = 0; i < 110; ++i) {				// Lumber cost / 10
		unittype = UnitTypeByWcNum(i);
		v = Fetch8(udta);
		unittype->_Costs[WoodCost] = v * 10;
	}
	for (i = 0; i < 110; ++i) {				// Oil cost / 10
		unittype = UnitTypeByWcNum(i);
		v = Fetch8(udta);
		unittype->_Costs[OilCost] = v * 10;
	}
	for (i = 0; i < 110; ++i) {				// Unit size in tiles
		unittype = UnitTypeByWcNum(i);
		v = FetchLE16(udta);
		unittype->TileWidth = v;
		v = FetchLE16(udta);
		unittype->TileHeight = v;
	}
	for (i = 0; i < 110; ++i) {				// Box size in pixel
		unittype = UnitTypeByWcNum(i);
		v = FetchLE16(udta);
		unittype->BoxWidth = v;
		v = FetchLE16(udta);
		unittype->BoxHeight = v;
	}

	for (i = 0; i < 110; ++i) {				// Attack range
		unittype = UnitTypeByWcNum(i);
		v = Fetch8(udta);
		unittype->_AttackRange = v;
	}
	for (i = 0; i < 110; ++i) {				// React range
		unittype = UnitTypeByWcNum(i);
		v = Fetch8(udta);
		unittype->ReactRangeComputer = v;
	}
	for (i = 0; i < 110; ++i) {				// React range
		unittype = UnitTypeByWcNum(i);
		v = Fetch8(udta);
		unittype->ReactRangePerson = v;
	}
	for (i = 0; i < 110; ++i) {				// Armor
		unittype = UnitTypeByWcNum(i);
		v = Fetch8(udta);
		unittype->_Armor = v;
	}
	for (i = 0; i < 110; ++i) {				// Selectable via rectangle
		unittype = UnitTypeByWcNum(i);
		v = Fetch8(udta);
		unittype->SelectableByRectangle = v != 0;
	}
	for (i = 0; i < 110; ++i) {				// Priority
		unittype = UnitTypeByWcNum(i);
		v = Fetch8(udta);
		unittype->Priority = v;
	}
	for (i = 0; i < 110; ++i) {				// Basic damage
		unittype = UnitTypeByWcNum(i);
		v = Fetch8(udta);
		unittype->_BasicDamage = v;
	}
	for (i = 0; i < 110; ++i) {				// Piercing damage
		unittype = UnitTypeByWcNum(i);
		v = Fetch8(udta);
		unittype->_PiercingDamage = v;
	}
	//
	// This is not used in stratagus. so it was simply removed.
	// We use our own upgrade methods that are a lot more flexible.
	// Maybe we could use this one day, not sure.
	//
#if 0
	for (i = 0; i < 110; ++i) {				// Weapons upgradable
		unittype = UnitTypeByWcNum(i);
		v = Fetch8(udta);
		unittype->WeaponsUpgradable = v;
	}
	for (i = 0; i < 110; ++i) {				// Armor upgradable
		unittype = UnitTypeByWcNum(i);
		v = Fetch8(udta);
		unittype->ArmorUpgradable = v;
	}
#else
	for (i = 0; i < 110; ++i) {				// Skip weapons upgradable
		Fetch8(udta);
	}
	for (i = 0; i < 110; ++i) {				// Skip armor upgradable
		Fetch8(udta);
	}
#endif
	for (i = 0; i < 110; ++i) {				// Missile Weapon
		unittype = UnitTypeByWcNum(i);
		v = Fetch8(udta);
		unittype->Missile.Name = strdup(MissileTypeWcNames[v]);
		DebugCheck(unittype->Missile.Missile);
	}
	for (i = 0; i < 110; ++i) {				// Unit type
		unittype = UnitTypeByWcNum(i);
		v = Fetch8(udta);
		unittype->UnitType = v;
	}
	for (i = 0; i < 110; ++i) {				// Decay rate * 6 = secs
		unittype = UnitTypeByWcNum(i);
		v = Fetch8(udta);
		unittype->DecayRate = v;
	}
	for (i = 0; i < 110; ++i) {				// Annoy computer factor
		unittype = UnitTypeByWcNum(i);
		v = Fetch8(udta);
		unittype->AnnoyComputerFactor = v;
	}
	for (i = 0; i < 58; ++i) {				// 2nd mouse button action
		unittype = UnitTypeByWcNum(i);
		v = Fetch8(udta);
		unittype->MouseAction = v;
	}
	for (; i < 110; ++i) {				// 2nd mouse button action
		unittype = UnitTypeByWcNum(i);
		unittype->MouseAction = 0;
	}
	for (i = 0; i < 110; ++i) {				// Point value for killing unit
		unittype = UnitTypeByWcNum(i);
		v = FetchLE16(udta);
		unittype->Points = v;
	}
	for (i = 0; i < 110; ++i) {				// Can target (1 land, 2 sea, 4 air)
		unittype = UnitTypeByWcNum(i);
		v = Fetch8(udta);
		unittype->CanTarget = v;
	}

	for (i = 0; i < 110; ++i) {				// Flags
		unittype = UnitTypeByWcNum(i);
		v = FetchLE32(udta);
	/// Nice looking bit macro
#define BIT(b,v) (((v >> b)) & 1)
		unittype->LandUnit = BIT(0, v);
		unittype->AirUnit = BIT(1, v);
		unittype->ExplodeWhenKilled = BIT(2, v);
		unittype->SeaUnit = BIT(3, v);
		//  BIT(4,v) This makes the unit a critter, true for demons, skeletons and sheep.
		//  There were some uses for this in code, like removing a health bar and the like,
		//  but I (n0body) don't think they were usefull. Thus BIT(4,v) is from now on ignored.
		unittype->Building = BIT(5, v);
		unittype->PermanentCloak = BIT(6, v);
		unittype->DetectCloak = BIT(7, v);
		// Cowards
		unittype->Coward = BIT(8, v) | BIT(26, v);
		if (BIT(9, v)) {
			unittype->ResInfo[OilCost] = (ResourceInfo*)malloc(sizeof(ResourceInfo));
			memset(unittype->ResInfo[OilCost], 0, sizeof(ResourceInfo));
			unittype->ResInfo[OilCost]->ResourceId = OilCost;
			unittype->ResInfo[OilCost]->FinalResource = OilCost;
			unittype->ResInfo[OilCost]->WaitAtResource = 150;
			unittype->ResInfo[OilCost]->WaitAtDepot = 150;
			unittype->ResInfo[OilCost]->ResourceCapacity = 100;
		}
		unittype->Transporter = BIT(10, v);
		unittype->CanStore[GoldCost] = BIT(12, v);
		unittype->Vanishes = BIT(13, v);
		unittype->GroundAttack = BIT(14, v);
//		unittype->IsUndead = BIT(15, v);
		unittype->ShoreBuilding = BIT(16, v);
//		unittype->CanCastSpell = BIT(17,v);
//		unittype->CanCastSpell = (char*)malloc(/*nb_spell*/);
		unittype->CanCastSpell = NULL;//
		unittype->CanStore[WoodCost] = BIT(18, v);
		unittype->CanAttack = BIT(19, v);
//		unittype->Hero = BIT(23, v);
		unittype->CanStore[OilCost] = BIT(24, v);
//		unittype->Volatile = BIT(25, v);
//		unittype->Organic = BIT(27, v);

		if (BIT(11, v) || BIT(21, v)) {
			unittype->GivesResource = OilCost;
		}
		if (BIT(22, v)) {
			unittype->GivesResource = GoldCost;
		}

#ifdef DEBUG
		if (BIT(28, v))		DebugLevel0("Unused bit 28 used in %d\n" _C_ i);
		if (BIT(29, v))		DebugLevel0("Unused bit 29 used in %d\n" _C_ i);
		if (BIT(30, v))		DebugLevel0("Unused bit 30 used in %d\n" _C_ i);
		if (BIT(31, v))		DebugLevel0("Unused bit 31 used in %d\n" _C_ i);
#endif
#undef BIT
	}

	// FIXME: peon applies also to peon-with-gold and peon-with-wood
	// FIXME: oil-tanker applies also to oil-tanker-full

	DebugLevel0("\tUDTA used %d bytes\n" _C_ udta-start);

	UpdateStats(1);
}

/**
**		Get the animations structure by ident.
**
**		@param ident		Identifier for the animation.
**		@return				Pointer to the animation structure.
**
**		@todo		Remove the use of scheme symbols to store, use own hash.
*/
global Animations* AnimationsByIdent(const char* ident)
{
	Animations** tmp;

	tmp = (Animations**)hash_find(AnimationsHash, ident);
	if (tmp) {
		return *tmp;
	}
	DebugLevel0Fn("Warning animation `%s' not found\n" _C_ ident);
	return NULL;
}

/**
**		Save state of an animition set to file.
**
**		@param name		Animation name.
**		@param resname		Resource name.
**		@param anim		Save this animation.
**		@param file		Output file.
*/
local void SaveAnimation(const char* name, const char* resname,
	const Animation* anim, CLFile* file)
{
	int i;
	int p;
	int frame;
	const Animation* temp;

	if (anim) {
		//
		//		Calculate the wait sum and pixels count.
		//
		i = p = 0;
		for (temp = anim; ; ++temp) {
			i += temp->Sleep & 0xFF;
			p += temp->Pixel;
			if (temp->Flags & AnimationEnd) {
				break;
			}
			if (temp->Flags & AnimationRestart) {
				i = p = 0;
			}
		}

		CLprintf(file, "\n  '%s ", name);
		if (resname) {
			CLprintf(file, "'%s ", resname);
		}
		CLprintf(file, "'(\t; #%d", i);
		if (p) {
			CLprintf(file, " P%d", p);
		}

		temp = anim;
		frame = 0;
		for (;;) {
			CLprintf(file, "\n	");
			for (i = 0; i < 4; ++i) {
				if (i) {
					CLprintf(file, " ");
				}
				frame += temp->Frame;
				CLprintf(file, "#(%2d %d %3d %3d)",
					temp->Flags & 0x7F, temp->Pixel, temp->Sleep & 0xFF, frame);
				if (temp->Flags & AnimationRestart) {
					frame = 0;
				}
				if (temp->Flags & AnimationEnd) {
					CLprintf(file, ")");
					return;
				}
				++temp;
			}
		}
	}
}

/**
**		Save state of the animitions set to file.
**
**		We save only the first occurance of an animation.
**
**		@param type		Save animations of this unit-type.
**		@param file		Output file.
*/
local void SaveAnimations(const UnitType* type, CLFile* file)
{
	const Animations* anims;
	int i;
	int q;

	if (!(anims = type->Animations)) {
		return;
	}

	//
	//		Look if this is the first use of it.
	//
	for (i = 0; i < NumUnitTypes && UnitTypes[i] != type ; ++i) {
		if (UnitTypes[i]->Animations == anims) {
			return;						// allready handled.
		}
	}

	CLprintf(file, "\n;;------\n;;\t");
	//
	//		Print all units that use this animation.
	//
	q = 0;
	for (i = 0 ; i < NumUnitTypes ; ++i) {
		if (UnitTypes[i]->Animations == anims) {
			if (q) {
				CLprintf(file, ", ");
			}
			CLprintf(file, "%s", UnitTypes[i]->Name);
			q = 1;
		}
	}
	CLprintf(file, "\n(define-animations 'animations-%s", type->Ident + 5);

	SaveAnimation("still", NULL, anims->Still, file);
	SaveAnimation("move", NULL, anims->Move, file);
	SaveAnimation("attack", NULL, anims->Attack, file);
	SaveAnimation("repair", NULL, anims->Repair, file);
	for (i = 0; i < MaxCosts; ++i) {
		SaveAnimation("harvest", DefaultResourceNames[i],
			anims->Harvest[i], file);
	}
	SaveAnimation("die", NULL, anims->Die, file);

	CLprintf(file, ")\n");
}

/**
**		Save state of an unit-type to file.
**
**		@param file		Output file.
**		@param type		Unit-type to save.
**		@param all		Flag save all values.
**
**		@todo		Arrange the variables more logical
*/
local void SaveUnitType(CLFile* file, const UnitType* type, int all)
{
	int i;
	int flag;
	ResourceInfo* res;

	CLprintf(file, "(define-unit-type '%s", type->Ident);
	CLprintf(file, " 'name \"%s\"\n  ", type->Name);
	// Graphic files
	if (type->SameSprite) {
		CLprintf(file, "'use '%s", type->SameSprite);
	} else {
		CLprintf(file, "'files '(");
		for (flag = i = 0; i < TilesetMax; ++i) {
			if (type->File[i]) {
				if (flag) {
					CLprintf(file, "\n	");
				}
				CLprintf(file, "%s \"%s\"", Tilesets[i]->Ident, type->File[i]);
				flag = 1;
			}
		}
		CLprintf(file, ")");
	}
	CLprintf(file, "\n  'size '(%d %d)\n", type->Width, type->Height);
	if (type->ShadowFile) {
		CLprintf(file, "  'shadow '(file \"%s\" size (%d %d) offset (%d %d))\n",
			type->ShadowFile, type->ShadowWidth, type->ShadowHeight,
			type->ShadowOffsetX, type->ShadowOffsetY);
	}

	//
	//		Animations are shared, find first use of the unit-type animation.
	//
	for (i = 0; i < NumUnitTypes && UnitTypes[i] != type; ++i) {
		if (UnitTypes[i]->Animations == type->Animations) {
			break;
		}
	}
	CLprintf(file, "  'animations 'animations-%s", UnitTypes[i]->Ident + 5);
	CLprintf(file, "  'icon '%s\n", IdentOfIcon(type->Icon.Icon));
	for (i = flag = 0; i < MaxCosts; ++i) {
		if (all || type->_Costs[i]) {
			if (!flag) {
				CLprintf(file, "  'costs '(");
				flag = 1;
			} else {
				CLprintf(file, " ");
			}
			CLprintf(file, "%s %d", DefaultResourceNames[i], type->_Costs[i]);
		}
	}
	if (flag) {
		CLprintf(file, ")\n");
	}

	for (i = flag = 0; i < MaxCosts; ++i) {
		if (type->ImproveIncomes[i] > DefaultIncomes[i]) {
			if (!flag) {
				CLprintf(file, "   'improve-production '(");
				flag = 1;
			} else {
				CLprintf(file, " ");
			}
			CLprintf(file, "%s %d ", DefaultResourceNames[i],
				type->ImproveIncomes[i] - DefaultIncomes[i]);
		}
	}
	if (flag) {
		CLprintf(file, ")\n");
	}

	if (type->RepairHP) {
		for (i = flag = 0; i < MaxCosts; ++i) {
			if (all || type->_Costs[i]) {
				if (!flag) {
					CLprintf(file, "  'repair-costs '(");
					flag = 1;
				} else {
					CLprintf(file, " ");
				}
				CLprintf(file, "%s %d", DefaultResourceNames[i],
					type->RepairCosts[i]);
			}
		}
	}
	if (flag) {
		CLprintf(file, ")\n");
	}

	if (type->RepairHP) {
		CLprintf(file, "  'repair-hp %d\n", type->RepairHP);
	}

	if (type->Construction) {
		CLprintf(file, "  'construction '%s\n", type->Construction->Ident);
	}
	CLprintf(file, "  'speed %d\n", type->_Speed);
	CLprintf(file, "  'hit-points %d\n", type->_HitPoints);
	CLprintf(file, "  'regeneration-rate %d\n", type->_RegenerationRate);
	if (type->BurnPercent) {
		CLprintf(file, "  'burn-percent %d 'burn-damage-rate %d\n",
				type->BurnPercent, type->BurnDamageRate);
	}
	if (all || type->_MaxMana) {
		CLprintf(file, "  'max-mana %d\n", type->_MaxMana);
	}
	CLprintf(file, "  'tile-size '(%d %d)", type->TileWidth, type->TileHeight);
	CLprintf(file, "  'box-size '(%d %d)\n", type->BoxWidth, type->BoxHeight);

	if(!type->Selectable) {
		CLprintf(file, "  'not-selectable\n");
	}

	CLprintf(file, "  'neutral-minimap-color '(%d %d %d)\n",
		type->NeutralMinimapColorRGB.r,type->NeutralMinimapColorRGB.g,
		type->NeutralMinimapColorRGB.b);

	CLprintf(file, "  'sight-range %d", type->_SightRange);
	if (all || type->ReactRangeComputer) {
		CLprintf(file, "  'computer-reaction-range %d", type->ReactRangeComputer);
	}
	if (all || type->ReactRangePerson) {
		CLprintf(file, "  'person-reaction-range %d", type->ReactRangePerson);
	}
	CLprintf(file, "\n");

	if (all || type->_Armor) {
		CLprintf(file, "  'armor %d", type->_Armor);
	} else {
		CLprintf(file, " ");
	}
	CLprintf(file, "  'basic-damage %d", type->_BasicDamage);
	CLprintf(file, "  'piercing-damage %d", type->_PiercingDamage);
	CLprintf(file, "  'missile '%s\n", type->Missile.Name);
	CLprintf(file, "  'draw-level %d", type->DrawLevel);
	if (all || type->MinAttackRange) {
		CLprintf(file, "  'min-attack-range %d", type->MinAttackRange);
		CLprintf(file, "  'max-attack-range %d\n", type->_AttackRange);
	} else if (type->_AttackRange) {
		CLprintf(file, "  'max-attack-range %d\n", type->_AttackRange);
	}
	CLprintf(file, "  'priority %d", type->Priority);
	if (all || type->AnnoyComputerFactor) {
		CLprintf(file, "  'annoy-computer-factor %d", type->AnnoyComputerFactor);
	}
	CLprintf(file, "\n");
	if (all || type->DecayRate) {
		CLprintf(file, "  'decay-rate %d\n", type->DecayRate);
	}
	if (all || type->Points) {
		CLprintf(file, "  'points %d\n", type->Points);
	}
	if (all || type->Demand) {
		CLprintf(file, "  'demand %d\n", type->Demand);
	}
	if (all || type->Supply) {
		CLprintf(file, "  'supply %d\n", type->Supply);
	}

	if (type->CorpseName) {
		CLprintf(file, "  'corpse '(%s %d)\n",
			type->CorpseName, type->CorpseScript);
	}
	if (type->ExplodeWhenKilled) {
		CLprintf(file, "  'explode-when-killed '%s\n", type->Explosion.Name);
	}

	CLprintf(file, "  ");
	switch (type->UnitType) {
		case UnitTypeLand:
			CLprintf(file, "'type-land");
			break;
		case UnitTypeFly:
			CLprintf(file, "'type-fly");
			break;
		case UnitTypeNaval:
			CLprintf(file, "'type-naval");
			break;
		default:
			CLprintf(file, "'type-unknown");
			break;
	}
	CLprintf(file, "\n");

	CLprintf(file, "  ");
	switch (type->MouseAction) {
		case MouseActionNone:
			if (all) {
				CLprintf(file, "'right-none");
			}
			break;
		case MouseActionAttack:
			CLprintf(file, "'right-attack");
			break;
		case MouseActionMove:
			CLprintf(file, "'right-move");
			break;
		case MouseActionHarvest:
			CLprintf(file, "'right-harvest");
			break;
		case MouseActionSpellCast:
			CLprintf(file, "'right-spell-cast");
			break;
		case MouseActionSail:
			CLprintf(file, "'right-sail");
			break;
		default:
			CLprintf(file, "'right-unknown");
			break;
	}
	CLprintf(file, "\n");

	if (type->GroundAttack) {
		CLprintf(file, "  'can-ground-attack\n");
	}
	if (type->CanAttack) {
		CLprintf(file, "  'can-attack\n");
	}
	if (type->RepairRange) {
		CLprintf(file, "  'repair-range %d\n", type->RepairRange);
	}
	if (type->CanTarget) {
		CLprintf(file, "  ");
		if (type->CanTarget & CanTargetLand) {
			CLprintf(file, "'can-target-land ");
		}
		if (type->CanTarget & CanTargetSea) {
			CLprintf(file, "'can-target-sea ");
		}
		if (type->CanTarget & CanTargetAir) {
			CLprintf(file, "'can-target-air ");
		}
		if (type->CanTarget & ~7) {
			CLprintf(file, "'can-target-other ");
		}
		CLprintf(file, "\n");
	}
	CLprintf(file, "'can-target-flag '(");
	for (i = 0; i < NumberBoolFlag; i++) { // User defined flags
		if (type->CanTargetFlag[i] != CONDITION_TRUE) {
			CLprintf(file, "%s %s ", BoolFlagName[i],
				type->CanTargetFlag[i] == CONDITION_ONLY ? "only" : "false");
		}
	}
	CLprintf(file, ") ");

	if (type->Building) {
		CLprintf(file, "  'building");
	}
	if (type->VisibleUnderFog) {
		CLprintf(file, "  'visible-under-fog");
	}
	if (type->BuilderOutside) {
		CLprintf(file, "  'builder-outside");
	}
	if (type->BuilderLost) {
		CLprintf(file, "  'builder-lost");
	}
	if (type->AutoBuildRate) {
		CLprintf(file, "  'auto-build-rate");
	}
	if (type->ShoreBuilding) {
		CLprintf(file, "  'shore-building\n");
	}
	if (type->LandUnit) {
		CLprintf(file, "  'land-unit\n");
	}
	if (type->AirUnit) {
		CLprintf(file, "  'air-unit\n");
	}
	if (type->SeaUnit) {
		CLprintf(file, "  'sea-unit\n");
	}

	if (type->RandomMovementProbability) {
		CLprintf(file, "  'random-movement-probability %d\n",
			type->RandomMovementProbability);
	}
	if (type->ClicksToExplode) {
		CLprintf(file, "  'clicks-to-explode %d\n", type->ClicksToExplode);
	}
	if (type->Revealer) {
		CLprintf(file, "  'revealer\n");
	}
	if (type->PermanentCloak) {
		CLprintf(file, "  'permanent-cloak\n");
	}
	if (type->DetectCloak) {
		CLprintf(file, "  'detect-cloak\n");
	}
	if (type->Transporter) {
		CLprintf(file, "  'transporter\n");
	}
	if (type->Transporter) {
		CLprintf(file, "  'max-on-board %d\n", type->MaxOnBoard);
	}

	if (type->Coward) {
		CLprintf(file, "  'coward\n");
	}

	if (type->Harvester) {
		for (i = 0; i < MaxCosts; ++i) {
			if (type->ResInfo[i]) {
				res = type->ResInfo[i];
				CLprintf(file, "  'can-gather-resource '(\n");
				CLprintf(file, "		resource-id %s\n",
					DefaultResourceNames[res->ResourceId]);
				CLprintf(file, "		final-resource %s\n",
					DefaultResourceNames[res->FinalResource]);
				if (res->HarvestFromOutside) {
					CLprintf(file, "		harvest-from-outside\n");
				}
				if (res->WaitAtResource) {
					CLprintf(file, "		wait-at-resource %d\n",
						res->WaitAtResource);
				}
				if (res->WaitAtDepot) {
					CLprintf(file, "		wait-at-depot %d\n",
						res->WaitAtDepot);
				}
				if (res->ResourceCapacity) {
					CLprintf(file, "		resource-capacity %d\n",
						res->ResourceCapacity);
				}
				if (res->ResourceStep) {
					CLprintf(file, "		resource-step %d\n",
						res->ResourceStep);
				}
				if (res->TerrainHarvester) {
					CLprintf(file, "		terrain-harvester\n");
				}
				if (res->LoseResources) {
					CLprintf(file, "		lose-resources\n");
				}
				if (res->FileWhenEmpty) {
					CLprintf(file, "		file-when-empty %s\n",
						res->FileWhenEmpty);
				}
				if (res->FileWhenLoaded) {
					CLprintf(file, "		file-when-loaded %s\n",
						res->FileWhenLoaded);
				}
				CLprintf(file, "		)\n");
			}
		}
	}

	if (type->GivesResource) {
		CLprintf(file, "  'gives-resource '%s\n",
			DefaultResourceNames[type->GivesResource]);
	}

	// Save store info.
	for (flag = i = 0; i < MaxCosts; ++i)
		if (type->CanStore[i]) {
			if (!flag) {
				flag = 1;
				CLprintf(file, "  'can-store '(%s", DefaultResourceNames[i]);
			} else {
					CLprintf(file, " %s", DefaultResourceNames[i]);
			}
		}
	if (flag)
		CLprintf(file, ")");

	if (type->CanCastSpell) {
		CLprintf(file, "  'can-cast-spell '( ");
		for (i = 0; i < SpellTypeCount; ++i) {
			if (type->CanCastSpell[i]) {
				CLprintf(file, "%s ", SpellTypeTable[i]->Ident);
			}
		}
		CLprintf(file, ")\n");
	}
	if (type->MustBuildOnTop) {
		CLprintf(file, "  'must-build-on-top '%s\n", type->MustBuildOnTop->Ident);
	}
	if (type->CanHarvest) {
		CLprintf(file, "  'can-harvest\n");
	}

	if (type->Vanishes) {
		CLprintf(file, "  'vanishes\n");
	}
	if (type->SelectableByRectangle) {
		CLprintf(file, "  'selectable-by-rectangle\n");
	}
	if (type->Teleporter) {
		CLprintf(file, "  'teleporter\n");
	}
	for (i = 0; i < NumberBoolFlag; i++) { // User defined flags
		if (type->BoolFlag[i]) {
			CLprintf(file, "  '%s\n", BoolFlagName[i]);
		}
	}


	CLprintf(file, "  'sounds '(");
	if (type->Sound.Selected.Name) {
		CLprintf(file, "\n	selected \"%s\"", type->Sound.Selected.Name);
	}
	if (type->Sound.Acknowledgement.Name) {
		CLprintf(file, "\n	acknowledge \"%s\"",
			type->Sound.Acknowledgement.Name);
	}
	if (type->Sound.Ready.Name) {
		CLprintf(file, "\n	ready \"%s\"", type->Sound.Ready.Name);
	}
	if (type->Sound.Repair.Name) {
		CLprintf(file, "\n	repair \"%s\"", type->Sound.Repair.Name);
	}
	for (i = 0; i < MaxCosts; ++i) {
		if (type->Sound.Harvest[i].Name) {
			CLprintf(file, "\n	harvest %s \"%s\"", DefaultResourceNames[i],
				type->Sound.Harvest[i].Name);
		}
	}
	if (type->Sound.Help.Name) {
		CLprintf(file, "\n	help \"%s\"", type->Sound.Help.Name);
	}
	if (type->Sound.Dead.Name) {
		CLprintf(file, "\n	dead \"%s\"", type->Sound.Dead.Name);
	}

	// FIXME: Attack should be removed!
	if (type->Weapon.Attack.Name) {
		CLprintf(file, "\n	attack \"%s\"", type->Weapon.Attack.Name);
	}
	CLprintf(file, ")");
	CLprintf(file, ")\n\n");
}

/**
**  Save state of an unit-stats to file.
**
**  @param stats  Unit-stats to save.
**  @param ident  Unit-type ident.
**  @param plynr  Player number.
**  @param file   Output file.
*/
local void SaveUnitStats(const UnitStats* stats, const char* ident, int plynr,
	CLFile* file)
{
	int j;

	DebugCheck(plynr >= PlayerMax);
	CLprintf(file, "DefineUnitStats(\"%s\", %d,\n  ", ident, plynr);
	CLprintf(file, "\"level\", %d, ", stats->Level);
	CLprintf(file, "\"speed\", %d, ", stats->Speed);
	CLprintf(file, "\"attack-range\", %d, ", stats->AttackRange);
	CLprintf(file, "\"sight-range\", %d,\n  ", stats->SightRange);
	CLprintf(file, "\"armor\", %d, ", stats->Armor);
	CLprintf(file, "\"basic-damage\", %d, ", stats->BasicDamage);
	CLprintf(file, "\"piercing-damage\", %d, ", stats->PiercingDamage);
	CLprintf(file, "\"hit-points\", %d,\n  ", stats->HitPoints);
	CLprintf(file, "\"regeneration-rate\", %d,\n  ", stats->RegenerationRate);
	CLprintf(file, "\"costs\", {");
	for (j = 0; j < MaxCosts; ++j) {
		if (j) {
			CLprintf(file, " ");
		}
		CLprintf(file, "\"%s\", %d,", DefaultResourceNames[j], stats->Costs[j]);
	}
	CLprintf(file, "})\n");
}

/**
**  Save declaration of user defined flags.
**
**  @param file  Output file.
*/
global void SaveFlags(CLFile* file)
{
#if 0
	int i;

	if (NumberBoolFlag != 0) {
		CLprintf(file, "(define-bool-flags");
		for (i = 0; i < NumberBoolFlag; i++) {
			CLprintf(file, " '%s", BoolFlagName[i]);
		}
		CLprintf(file, ")\n");
	}
#endif
}

/**
**  Save the names of all unit types, before actually defining anything about them.
**
**  @param file  Output file.
*/
global void SaveUnitTypeDefs(CLFile* file)
{
#if 0
	int i;

	CLprintf(file, "\n;;; Declare all unit types in advance.\n");
	//  Define all types in advance to avoid undefined unit problems.
	for (i = 0; i < NumUnitTypes; ++i) {
		CLprintf(file, "(define-unit-type '%s)\n", UnitTypes[i]->Ident);
	}
	CLprintf(file, "\n");
#endif
}

/**
**  Save state of the unit-type table to file.
**
**  @param file  Output file.
*/
global void SaveUnitTypes(CLFile* file)
{
	int i;
	int j;
//	char** sp;

	CLprintf(file, "\n--- -----------------------------------------\n");
	CLprintf(file, "--- MODULE: unittypes $Id$\n\n");
#if 0
	// Original number to internal unit-type name.

	i = CLprintf(file, "(define-unittype-wc-names");
	for (sp = UnitTypeWcNames; *sp; ++sp) {
		if (i + strlen(*sp) > 79) {
			i = CLprintf(file, "\n ");
		}
		i += CLprintf(file, " '%s", *sp);
	}
	CLprintf(file, ")\n");

	// Save all animations.

	for (i = 0; i < NumUnitTypes; ++i) {
		SaveAnimations(UnitTypes[i], file);
	}

	// Save all types

	for (i = 0; i < NumUnitTypes; ++i) {
		CLprintf(file, "\n");
		SaveUnitType(file, UnitTypes[i], 0);
	}
#endif
	// Save all stats

	for (i = 0; i < NumUnitTypes; ++i) {
		CLprintf(file, "\n");
		for (j = 0; j < PlayerMax; ++j) {
			if (Players[j].Type != PlayerNobody) {
				SaveUnitStats(&UnitTypes[i]->Stats[j], UnitTypes[i]->Ident, j, file);
			}
		}
	}
}

/**
**		Find unit-type by identifier.
**
**		@param ident		The unit-type identifier.
**		@return				Unit-type pointer.
*/
global UnitType* UnitTypeByIdent(const char* ident)
{
	UnitType* const* type;

	type = (UnitType* const*)hash_find(UnitTypeHash, ident);
	return type ? *type : 0;
}

/**
**		Find unit-type by wc number.
**
**		@param num		The unit-type number used in f.e. puds.
**		@return				Unit-type pointer.
*/
global UnitType* UnitTypeByWcNum(unsigned num)
{
	return UnitTypeByIdent(UnitTypeWcNames[num]);
}

/**
**		Allocate an empty unit-type slot.
**
**		@param ident		Identifier to identify the slot (malloced by caller!).
**
**		@return				New allocated (zeroed) unit-type pointer.
*/
global UnitType* NewUnitTypeSlot(char* ident)
{
	UnitType* type;

	type = malloc(sizeof(UnitType));
	if (!type) {
		fprintf(stderr, "Out of memory\n");
		ExitFatal(-1);
	}
	memset(type, 0, sizeof(UnitType));
	type->Slot = NumUnitTypes;
	DebugLevel3Fn("Making a new unit, called %s, branded %d\n"
			_C_ ident _C_ type->Type);
	type->Ident = ident;
	UnitTypes[NumUnitTypes++] = type;
	//
	//		Rehash.
	//
	*(UnitType**)hash_add(UnitTypeHash, type->Ident) = type;
	return type;
}

/**
**		Draw unit-type on map.
**
**  @param type    Unit-type pointer.
**  @param frame   Animation frame of unit-type.
**  @param sprite  Sprite to use for drawing
**  @param x       Screen X pixel postion to draw unit-type.
**  @param y       Screen Y pixel postion to draw unit-type.
**
**  @todo  Do screen position caculation in high level.
**         Better way to handle in x mirrored sprites.
*/
global void DrawUnitType(const UnitType* type, Graphic* sprite, int frame, int x, int y)
{
	// FIXME: move this calculation to high level.
	x -= (type->Width - type->TileWidth * TileSizeX) / 2;
	y -= (type->Height - type->TileHeight * TileSizeY) / 2;

	if (type->Flip) {
		if (frame < 0) {
			VideoDrawClipX(sprite, -frame - 1, x, y);
		} else {
			VideoDrawClip(sprite, frame, x, y);
		}
	} else {
		int row;

		row = type->NumDirections / 2 + 1;
		if (frame < 0) {
			frame = ((-frame - 1) / row) * type->NumDirections + type->NumDirections - (-frame - 1) % row;
		} else {
			frame = (frame / row) * type->NumDirections + frame % row;
		}
		VideoDrawClip(sprite, frame, x, y);
	}
}

/**
**		Init unit types.
*/
global void InitUnitTypes(int reset_player_stats)
{
	int type;

	for (type = 0; type < NumUnitTypes; ++type) {
		//
		//		Initialize:
		//
		DebugCheck(UnitTypes[type]->Slot != type);
		//
		//		Add idents to hash.
		//
		*(UnitType**)hash_add(UnitTypeHash, UnitTypes[type]->Ident) =
			UnitTypes[type];
	}

	// LUDO : called after game is loaded -> don't reset stats !
	UpdateStats(reset_player_stats);		// Calculate the stats

	//
	//		Setup hardcoded unit types. FIXME: should be moved to some configs.
	//
	UnitTypeHumanWall = UnitTypeByIdent("unit-human-wall");
	UnitTypeOrcWall = UnitTypeByIdent("unit-orc-wall");
}

/**
**  Loads the Sprite for a unit type
**
**  @param unittype  type of unit to load
*/
global void LoadUnitTypeSprite(UnitType* unittype)
{
	const char* file;
	char buf[4096];
	ResourceInfo* resinfo;
	int res;
	UnitType* type;

	if (unittype->SameSprite) {
		type = UnitTypeByIdent(unittype->SameSprite);
		if (!type) {
			PrintFunction();
			fprintf(stdout, "Unit-type %s not found\n", type->SameSprite);
			ExitFatal(-1);
		}
	} else {
		type = unittype;
	}

	if ((file = type->ShadowFile)) {
		file = strcat(strcpy(buf, "graphics/"), file);
		type->ShadowSprite = LoadSprite(file, type->ShadowWidth,
			type->ShadowHeight);
		FlipGraphic(type->ShadowSprite);
		MakeShadowSprite(type->ShadowSprite);
	}

	if (type->Harvester) {
		for (res = 0; res < MaxCosts; ++res) {
			if ((resinfo = type->ResInfo[res])) {
				if ((file = resinfo->FileWhenLoaded)) {
					file = strcat(strcpy(buf, "graphics/"), file);
					resinfo->SpriteWhenLoaded = LoadSprite(file, type->Width,
						type->Height);
					FlipGraphic(resinfo->SpriteWhenLoaded);
				}
				if ((file = resinfo->FileWhenEmpty)) {
					file = strcat(strcpy(buf, "graphics/"), file);
					resinfo->SpriteWhenEmpty = LoadSprite(file, type->Width,
						type->Height);
					FlipGraphic(resinfo->SpriteWhenEmpty);
				}
			}
		}
	}

	file = type->File[TheMap.Terrain];
	if (!file) {						// default one
		file = type->File[0];
	}
	if (file) {
		strcpy(buf, "graphics/");
		strcat(buf, file);
		type->Sprite = LoadSprite(buf, type->Width, type->Height);
		FlipGraphic(type->Sprite);
	}

	if (unittype->SameSprite) {
		unittype->Sprite = type->Sprite;
	}
}
				
/**
**		Load the graphics for the unit-types.
*/
global void LoadUnitTypes(void)
{
	UnitType* type;
	int i;

	for (i = 0; i < NumUnitTypes; ++i) {
		type = UnitTypes[i];

		//
		//		Lookup icons.
		//
		type->Icon.Icon = IconByIdent(type->Icon.Name);
		if (!type->Icon.Icon) {
			printf("Can't find icon %s\n", type->Icon.Name);
			ExitFatal(-1);
		}
		//
		//		Lookup missiles.
		//
		type->Missile.Missile = MissileTypeByIdent(type->Missile.Name);
		if (type->Explosion.Name) {
			type->Explosion.Missile = MissileTypeByIdent(type->Explosion.Name);
		}
		//
		//		Lookup corpse.
		//
		if (type->CorpseName) {
			type->CorpseType = UnitTypeByIdent(type->CorpseName);
		}
		
		//
		// Load Sprite
		//
#ifndef DYNAMIC_LOAD
		if (!type->Sprite) {
			ShowLoadProgress("Unit \"%s\"", type->Name);
			LoadUnitTypeSprite(type);
		}
#endif
		// FIXME: should i copy the animations of same graphics?
	}
}

/**
**		Cleanup the unit-type module.
*/
global void CleanUnitTypes(void)
{
	UnitType* type;
	void** ptr;
	int i;
	int j;
	int res;
	Animations* anims;

	DebugLevel0Fn("FIXME: icon, sounds not freed.\n");

	//
	//		Mapping the original unit-type numbers in puds to our internal strings
	//
	if ((ptr = (void**)UnitTypeWcNames)) {		// Free all old names
		while (*ptr) {
			free(*ptr++);
		}
		free(UnitTypeWcNames);

		UnitTypeWcNames = NULL;
	}

	//		FIXME: scheme contains references on this structure.
	//		Clean all animations.

	for (i = 0; i < NumUnitTypes; ++i) {
		type = UnitTypes[i];

		if (!(anims = type->Animations)) {		// Must be handled?
			continue;
		}
		for (j = i; j < NumUnitTypes; ++j) {		// Remove all uses.
			if (anims == UnitTypes[j]->Animations) {
				UnitTypes[j]->Animations = NULL;
			}
		}
		type->Animations = NULL;
		if (anims->Still) {
			free(anims->Still);
		}
		if (anims->Move) {
			free(anims->Move);
		}
		if (anims->Attack) {
			free(anims->Attack);
		}
		if (anims->Die) {
			free(anims->Die);
		}
		if (anims->Repair) {
			free(anims->Repair);
		}
		for (i = 0; i < MaxCosts; ++i) {
			if (anims->Harvest[i]) {
				free(anims->Harvest[i]);
			}
		}
		free(anims);
	}

	//		Clean all unit-types

	for (i = 0; i < NumUnitTypes; ++i) {
		type = UnitTypes[i];
		hash_del(UnitTypeHash, type->Ident);

		DebugCheck(!type->Ident);
		free(type->Ident);
		DebugCheck(!type->Name);
		free(type->Name);

		free(type->BoolFlag);
		free(type->CanTargetFlag);

		if (type->SameSprite) {
			free(type->SameSprite);
		}
		if (type->File[0]) {
			free(type->File[0]);
		}
		if (type->File[1]) {
			free(type->File[1]);
		}
		if (type->File[2]) {
			free(type->File[2]);
		}
		if (type->File[3]) {
			free(type->File[3]);
		}
		if (type->Icon.Name) {
			free(type->Icon.Name);
		}
		if (type->Missile.Name) {
			free(type->Missile.Name);
		}
		if (type->Explosion.Name) {
			free(type->Explosion.Name);
		}
		if (type->CorpseName) {
			free(type->CorpseName);
		}
		if (type->CanCastSpell) {
			free(type->CanCastSpell);
		}

		for (res = 0; res < MaxCosts; ++res) {
			if (type->ResInfo[res]) {
				if (type->ResInfo[res]->SpriteWhenLoaded) {
					VideoSafeFree(type->ResInfo[res]->SpriteWhenLoaded);
				}
				if (type->ResInfo[res]->SpriteWhenEmpty) {
					VideoSafeFree(type->ResInfo[res]->SpriteWhenEmpty);
				}
				if (type->ResInfo[res]->FileWhenEmpty) {
					free(type->ResInfo[res]->FileWhenEmpty);
				}
				if (type->ResInfo[res]->FileWhenLoaded) {
					free(type->ResInfo[res]->FileWhenLoaded);
				}
				free(type->ResInfo[res]);
			}
		}

		//
		//		FIXME: Sounds can't be freed, they still stuck in sound hash.
		//
		if (type->Sound.Selected.Name) {
			free(type->Sound.Selected.Name);
		}
		if (type->Sound.Acknowledgement.Name) {
			free(type->Sound.Acknowledgement.Name);
		}
		if (type->Sound.Ready.Name) {
			free(type->Sound.Ready.Name);
		}
		if (type->Sound.Repair.Name) {
			free(type->Sound.Repair.Name);
		}
		for (j = 0; j < MaxCosts; ++j) {
			if (type->Sound.Harvest[j].Name) {
				free(type->Sound.Harvest[j].Name);
			}
		}
		if (type->Sound.Help.Name) {
			free(type->Sound.Help.Name);
		}
		if (type->Sound.Dead.Name) {
			free(type->Sound.Dead.Name);
		}
		if (type->Weapon.Attack.Name) {
			free(type->Weapon.Attack.Name);
		}

		if (!type->SameSprite) {		// our own graphics
			VideoSafeFree(type->Sprite);
		}
#ifdef USE_OPENGL
		for (j = 0; j < PlayerMax; ++j) {
			VideoSafeFree(type->PlayerColorSprite[j]);
		}
#endif
		free(UnitTypes[i]);
		UnitTypes[i] = 0;
	}
	NumUnitTypes = 0;

	for (i = 0; i < NumberBoolFlag; i++) { // User defined flags
		free(BoolFlagName[i]);
	}
	free(BoolFlagName);
	BoolFlagName = NULL;
	NumberBoolFlag = 0;

	//
	//		Clean hardcoded unit types.
	//
	UnitTypeHumanWall = NULL;
	UnitTypeOrcWall = NULL;
}

//@}
