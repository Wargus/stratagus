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
//      (c) Copyright 1998-2005 by Lutz Sammer and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
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

#include "util.h"

#include "myendian.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

#ifdef DEBUG
int NoWarningUnitType;  ///< quiet ident lookup
#endif

UnitType* UnitTypes[UnitTypeMax];   ///< unit-types definition
int NumUnitTypes;                   ///< number of unit-types made

/**
**  Next unit type are used hardcoded in the source.
**
**  @todo find a way to make it configurable!
*/
UnitType* UnitTypeHumanWall;       ///< Human wall
UnitType* UnitTypeOrcWall;         ///< Orc wall

/**
**  Mapping of W*rCr*ft number to our internal unit-type symbol.
**  The numbers are used in puds.
*/
char** UnitTypeWcNames;

#ifdef DOXYGEN // no real code, only for document

/**
**  Lookup table for unit-type names
*/
static UnitType* UnitTypeHash[UnitTypeMax];

#else

/**
**  Lookup table for unit-type names
*/
static hashtable(UnitType*, UnitTypeMax) UnitTypeHash;

#endif

/**
**  Default resources for a new player.
*/
int DefaultResources[MaxCosts];

/**
**  Default resources for a new player with low resources.
*/
int DefaultResourcesLow[MaxCosts];

/**
**  Default resources for a new player with mid resources.
*/
int DefaultResourcesMedium[MaxCosts];

/**
**  Default resources for a new player with high resources.
*/
int DefaultResourcesHigh[MaxCosts];

/**
**  Default incomes for a new player.
*/
int DefaultIncomes[MaxCosts];

/**
**  Default action for the resources.
*/
char* DefaultActions[MaxCosts];

/**
**  Default names for the resources.
*/
char* DefaultResourceNames[MaxCosts];

/**
**  Default amounts for the resources.
*/
int DefaultResourceAmounts[MaxCosts];

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Update the player stats for changed unit types.
**  @param reset indicates wether default value should be set to each stat (level, upgrades)
*/
void UpdateStats(int reset)
{
	UnitType* type;
	UnitStats* stats;
	int player;
	unsigned i;
	int j;

	//
	//  Update players stats
	//
	for (j = 0; j < NumUnitTypes; ++j) {
		type = UnitTypes[j];
		if (reset) {
			// LUDO : FIXME : reset loading of player stats !
			for (player = 0; player < PlayerMax; ++player) {
				stats = &type->Stats[player];
				for (i = 0; i < MaxCosts; ++i) {
					stats->Costs[i] = type->_Costs[i];
				}
				if (!stats->Variables) {
					stats->Variables = calloc(UnitTypeVar.NumberVariable, sizeof (*stats->Variables));
				}
				for (i = 0; (int) i < UnitTypeVar.NumberVariable; i++) {
					stats->Variables[i] = type->Variable[i];
				}
				if (type->Building) {
					stats->Variables[LEVEL_INDEX].Value = 0; // Disables level display
				} else {
					stats->Variables[LEVEL_INDEX].Value = 1;
					stats->Variables[LEVEL_INDEX].Max = 1;
				}
				stats->AttackRange = stats->Variables[ATTACKRANGE_INDEX].Max;
				stats->SightRange = stats->Variables[SIGHTRANGE_INDEX].Value;
				stats->HitPoints = type->_HitPoints;
				stats->RegenerationRate = stats->Variables[HP_INDEX].Increase;
				stats->Mana = stats->Variables[MANA_INDEX].Max;

				stats->Variables[SIGHTRANGE_INDEX].Max = stats->SightRange;
				stats->Variables[HP_INDEX].Max = stats->HitPoints;
			}
		}

		//
		//  As side effect we calculate the movement flags/mask here.
		//
		switch (type->UnitType) {
			case UnitTypeLand:                              // on land
				type->MovementMask =
					MapFieldLandUnit |
					MapFieldSeaUnit |
					MapFieldBuilding | // already occuppied
					MapFieldCoastAllowed |
					MapFieldWaterAllowed | // can't move on this
					MapFieldUnpassable;
				break;
			case UnitTypeFly:                               // in air
				type->MovementMask =
					MapFieldAirUnit; // already occuppied
				break;
			case UnitTypeNaval:                             // on water
				if (type->CanTransport) {
					type->MovementMask =
						MapFieldLandUnit |
						MapFieldSeaUnit |
						MapFieldBuilding | // already occuppied
						MapFieldLandAllowed; // can't move on this
					// Johns: MapFieldUnpassable only for land units?
				} else {
					type->MovementMask =
						MapFieldLandUnit |
						MapFieldSeaUnit |
						MapFieldBuilding | // already occuppied
						MapFieldCoastAllowed |
						MapFieldLandAllowed | // can't move on this
						MapFieldUnpassable;
				}
				break;
			default:
				DebugPrint("Where moves this unit?\n");
				type->MovementMask = 0;
				break;
		}
		if (type->Building || type->ShoreBuilding) {
			// Shore building is something special.
			if (type->ShoreBuilding) {
				type->MovementMask =
					MapFieldLandUnit |
					MapFieldSeaUnit |
					MapFieldBuilding | // already occuppied
					MapFieldLandAllowed; // can't build on this
			}
			type->MovementMask |= MapFieldNoBuilding;
			//
			// A little chaos, buildings without HP can be entered.
			// The oil-patch is a very special case.
			//
			if (type->_HitPoints) {
				type->FieldFlags = MapFieldBuilding;
			} else {
				type->FieldFlags = MapFieldNoBuilding;
			}
		} else switch (type->UnitType) {
			case UnitTypeLand: // on land
				type->FieldFlags = MapFieldLandUnit;
				break;
			case UnitTypeFly: // in air
				type->FieldFlags = MapFieldAirUnit;
				break;
			case UnitTypeNaval: // on water
				type->FieldFlags = MapFieldSeaUnit;
				break;
			default:
				DebugPrint("Where moves this unit?\n");
				type->FieldFlags = 0;
				break;
		}
	}
}

/**
**  Get the animations structure by ident.
**
**  @param ident  Identifier for the animation.
**  @return  Pointer to the animation structure.
**
**  @todo Remove the use of scheme symbols to store, use own hash.
*/
Animations* AnimationsByIdent(const char* ident)
{
	Animations** tmp;

	tmp = (Animations**)hash_find(AnimationsHash, ident);
	if (tmp) {
		return *tmp;
	}
	DebugPrint("Warning animation `%s' not found\n" _C_ ident);
	return NULL;
}

/**
**  Get the animations structure by ident.
**
**  @param ident  Identifier for the animation.
**  @return  Pointer to the animation structure.
**
**  @todo Remove the use of scheme symbols to store, use own hash.
*/
NewAnimations* NewAnimationsByIdent(const char* ident)
{
	NewAnimations** tmp;

	tmp = (NewAnimations**)hash_find(NewAnimationsHash, ident);
	if (tmp) {
		return *tmp;
	}
	DebugPrint("Warning animation `%s' not found\n" _C_ ident);
	return NULL;
}

/**
**  Save state of an unit-stats to file.
**
**  @param stats  Unit-stats to save.
**  @param ident  Unit-type ident.
**  @param plynr  Player number.
**  @param file   Output file.
*/
static void SaveUnitStats(const UnitStats* stats, const char* ident, int plynr,
	CLFile* file)
{
	int j;

	Assert(plynr < PlayerMax);
	CLprintf(file, "DefineUnitStats(\"%s\", %d,\n  ", ident, plynr);
	for (j = 0; j < UnitTypeVar.NumberVariable; ++j) {
		CLprintf(file, "\"%s\", {Value = %d, Max = %d, Increase = %d},\n  ",
			UnitTypeVar.VariableName[j], stats->Variables[j].Value,
			stats->Variables[j].Max, stats->Variables[j].Increase);
	}
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
**  Save state of the unit-type table to file.
**
**  @param file  Output file.
*/
void SaveUnitTypes(CLFile* file)
{
	int i;
	int j;
// char** sp;

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
**  Find unit-type by identifier.
**
**  @param ident  The unit-type identifier.
**
**  @return       Unit-type pointer.
*/
UnitType* UnitTypeByIdent(const char* ident)
{
	UnitType* const* type;

	type = (UnitType* const*)hash_find(UnitTypeHash, ident);
	return type ? *type : 0;
}

/**
**  Find unit-type by wc number.
**
**  @param num  The unit-type number used in f.e. puds.
**
**  @return     Unit-type pointer.
*/
UnitType* UnitTypeByWcNum(unsigned num)
{
	return UnitTypeByIdent(UnitTypeWcNames[num]);
}

/**
**  Allocate an empty unit-type slot.
**
**  @param ident  Identifier to identify the slot (malloced by caller!).
**
**  @return       New allocated (zeroed) unit-type pointer.
*/
UnitType* NewUnitTypeSlot(char* ident)
{
	UnitType* type;

	type = calloc(1, sizeof(UnitType));
	if (!type) {
		fprintf(stderr, "Out of memory\n");
		ExitFatal(-1);
	}
	type->Slot = NumUnitTypes;
	type->Ident = ident;
	UnitTypes[NumUnitTypes++] = type;
	*(UnitType**)hash_add(UnitTypeHash, type->Ident) = type;
	return type;
}

/**
**  Draw unit-type on map.
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
void DrawUnitType(const UnitType* type, Graphic* sprite, int player, int frame,
	int x, int y)
{
#ifdef USE_OPENGL
	if (!sprite->PlayerColorTextures[player]) {
		MakePlayerColorTexture(sprite, player);
	}
#else
	SDL_SetColors(sprite->Surface, Players[player].UnitColors.Colors, 208, 4);
	if (sprite->SurfaceFlip) {
		SDL_SetColors(sprite->SurfaceFlip, Players[player].UnitColors.Colors, 208, 4);
	}
#endif

	// FIXME: move this calculation to high level.
	x -= (type->Width - type->TileWidth * TileSizeX) / 2;
	y -= (type->Height - type->TileHeight * TileSizeY) / 2;
	x += type->OffsetX;
	y += type->OffsetY;

	if (type->Flip) {
		if (frame < 0) {
			VideoDrawPlayerColorClipX(sprite, player, -frame - 1, x, y);
		} else {
			VideoDrawPlayerColorClip(sprite, player, frame, x, y);
		}
	} else {
		int row;

		row = type->NumDirections / 2 + 1;
		if (frame < 0) {
			frame = ((-frame - 1) / row) * type->NumDirections + type->NumDirections - (-frame - 1) % row;
		} else {
			frame = (frame / row) * type->NumDirections + frame % row;
		}
		VideoDrawPlayerColorClip(sprite, player, frame, x, y);
	}
}

/**
**  Init unit types.
*/
void InitUnitTypes(int reset_player_stats)
{
	int type;

	for (type = 0; type < NumUnitTypes; ++type) {
		//
		//  Initialize:
		//
		Assert(UnitTypes[type]->Slot == type);
		//
		//  Add idents to hash.
		//
		*(UnitType**)hash_add(UnitTypeHash, UnitTypes[type]->Ident) =
			UnitTypes[type];
	}

	// LUDO : called after game is loaded -> don't reset stats !
	UpdateStats(reset_player_stats); // Calculate the stats

	//
	// Setup hardcoded unit types. FIXME: should be moved to some configs.
	//
	UnitTypeHumanWall = UnitTypeByIdent("unit-human-wall");
	UnitTypeOrcWall = UnitTypeByIdent("unit-orc-wall");
}

/**
**  Loads the Sprite for a unit type
**
**  @param type  type of unit to load
*/
void LoadUnitTypeSprite(UnitType* type)
{
	const char* file;
	ResourceInfo* resinfo;
	int res;

	if (type->ShadowFile) {
		type->ShadowSprite = NewGraphic(type->ShadowFile, type->ShadowWidth,
			type->ShadowHeight);
		LoadGraphic(type->ShadowSprite);
		FlipGraphic(type->ShadowSprite);
		MakeShadowSprite(type->ShadowSprite);
	}

	if (type->Harvester) {
		for (res = 0; res < MaxCosts; ++res) {
			if ((resinfo = type->ResInfo[res])) {
				if (resinfo->FileWhenLoaded) {
					resinfo->SpriteWhenLoaded = NewGraphic(resinfo->FileWhenLoaded,
						type->Width, type->Height);
					LoadGraphic(resinfo->SpriteWhenLoaded);
					FlipGraphic(resinfo->SpriteWhenLoaded);
				}
				if (resinfo->FileWhenEmpty) {
					resinfo->SpriteWhenEmpty = NewGraphic(resinfo->FileWhenEmpty,
						type->Width, type->Height);
					LoadGraphic(resinfo->SpriteWhenEmpty);
					FlipGraphic(resinfo->SpriteWhenEmpty);
				}
			}
		}
	}

	file = type->File[TheMap.Terrain];
	if (!file) { // default one
		file = type->File[0];
	}
	if (file) {
		type->Sprite = NewGraphic(file, type->Width, type->Height);
		LoadGraphic(type->Sprite);
		FlipGraphic(type->Sprite);
	}
}

/**
** Load the graphics for the unit-types.
*/
void LoadUnitTypes(void)
{
	UnitType* type;
	int i;

	for (i = 0; i < NumUnitTypes; ++i) {
		type = UnitTypes[i];

		//
		// Lookup icons.
		//
		type->Icon.Icon = IconByIdent(type->Icon.Name);
		if (!type->Icon.Icon) {
			printf("Can't find icon %s\n", type->Icon.Name);
			ExitFatal(-1);
		}
		//
		// Lookup missiles.
		//
		type->Missile.Missile = MissileTypeByIdent(type->Missile.Name);
		if (type->Explosion.Name) {
			type->Explosion.Missile = MissileTypeByIdent(type->Explosion.Name);
		}
		//
		// Lookup corpse.
		//
		if (type->CorpseName) {
			type->CorpseType = UnitTypeByIdent(type->CorpseName);
		}

		//
		// Lookup BuildingTypes
		if (type->BuildingRules) {
			int x;
			BuildRestriction* b;

			x = 0;
			while (type->BuildingRules[x] != NULL) {
				b = type->BuildingRules[x];
				while (b != NULL) {
					if (b->RestrictType == RestrictAddOn) {
						b->Data.AddOn.Parent = UnitTypeByIdent(b->Data.AddOn.ParentName);
					} else if (b->RestrictType == RestrictOnTop) {
						b->Data.OnTop.Parent = UnitTypeByIdent(b->Data.OnTop.ParentName);
					} else if (b->RestrictType == RestrictDistance) {
						b->Data.Distance.RestrictType = UnitTypeByIdent(b->Data.Distance.RestrictTypeName);
					}
					b = b->Next;
				}
				++x;
			}
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
**  Cleanup the unit-type module.
*/
void CleanUnitTypes(void)
{
	UnitType* type;
	char** ptr;
	int i;
	int j;
	int res;
	Animations* anims;

	DebugPrint("FIXME: icon, sounds not freed.\n");

	//
	//  Mapping the original unit-type numbers in puds to our internal strings
	//
	if ((ptr = UnitTypeWcNames)) { // Free all old names
		while (*ptr) {
			free(*ptr++);
		}
		free(UnitTypeWcNames);

		UnitTypeWcNames = NULL;
	}

	// FIXME: scheme contains references on this structure.
	// Clean all animations.

	for (i = 0; i < NumUnitTypes; ++i) {
		type = UnitTypes[i];

		if (!(anims = type->Animations)) { // Must be handled?
			continue;
		}
		for (j = i; j < NumUnitTypes; ++j) { // Remove all uses.
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
		for (j = 0; j < MaxCosts; ++j) {
			if (anims->Harvest[j]) {
				free(anims->Harvest[j]);
			}
		}
		free(anims);
	}

	// Clean all unit-types

	for (i = 0; i < NumUnitTypes; ++i) {
		type = UnitTypes[i];
		hash_del(UnitTypeHash, type->Ident);

		Assert(type->Ident);
		free(type->Ident);
		Assert(type->Name);
		free(type->Name);

		free(type->Variable);
		free(type->BoolFlag);
		free(type->CanTargetFlag);
		free(type->CanTransport);

		for (j = 0; j < PlayerMax; j++) {
			free(type->Stats[j].Variables);
		}

		// Free Building Restrictions if there are any
		if (type->BuildingRules) {
			int x;
			BuildRestriction* b;
			BuildRestriction* f;

			x = 0;
			while (type->BuildingRules[x] != NULL) {
				b = type->BuildingRules[x];
				while (b != NULL) {
					f = b;
					b = b->Next;
					if (f->RestrictType == RestrictAddOn) {
						free(f->Data.AddOn.ParentName);
					} else if (f->RestrictType == RestrictOnTop) {
						free(f->Data.OnTop.ParentName);
					} else if (f->RestrictType == RestrictDistance) {
						free(f->Data.Distance.RestrictTypeName);
					}
					free(f);
				}
				++x;
			}
			free(type->BuildingRules);
		}
		for (j = 0; j < TilesetMax; ++j) {
			if (type->File[j]) {
				free(type->File[j]);
			}
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
		free(type->AutoCastActive);

		for (res = 0; res < MaxCosts; ++res) {
			if (type->ResInfo[res]) {
				if (type->ResInfo[res]->SpriteWhenLoaded) {
					FreeGraphic(type->ResInfo[res]->SpriteWhenLoaded);
				}
				if (type->ResInfo[res]->SpriteWhenEmpty) {
					FreeGraphic(type->ResInfo[res]->SpriteWhenEmpty);
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
		// FIXME: Sounds can't be freed, they still stuck in sound hash.
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

		FreeGraphic(type->Sprite);

		free(UnitTypes[i]);
		UnitTypes[i] = 0;
	}
	NumUnitTypes = 0;

	for (i = 0; i < UnitTypeVar.NumberBoolFlag; i++) { // User defined flags
		free(UnitTypeVar.BoolFlagName[i]);
	}
	for (i = 0; i < UnitTypeVar.NumberVariable; i++) { // User defined variables
		free(UnitTypeVar.VariableName[i]);
	}
	free(UnitTypeVar.BoolFlagName);
	free(UnitTypeVar.VariableName);
	free(UnitTypeVar.Variable);
	free(UnitTypeVar.DecoVar);
	memset(&UnitTypeVar, 0, sizeof (UnitTypeVar));

	//
	// Clean hardcoded unit types.
	//
	UnitTypeHumanWall = NULL;
	UnitTypeOrcWall = NULL;
}

//@}
