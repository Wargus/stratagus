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
/**@name unittype.cpp - The unit types. */
//
//      (c) Copyright 1998-2006 by Lutz Sammer and Jimmy Salmon
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "stratagus.h"

#include <string>
#include <map>

#include "unittype.h"

#include "animation.h"
#include "animation/animation_exactframe.h"
#include "animation/animation_frame.h"

#include "video.h"
#include "tileset.h"
#include "map.h"
#include "sound.h"
#include "unitsound.h"
#include "construct.h"
#include "player.h"
#include "missile.h"
#include "script.h"
#include "spells.h"
#include "iolib.h"
#include "luacallback.h"
#include "util.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

std::vector<CUnitType *> UnitTypes;   /// unit-types definition
std::map<std::string, CUnitType *> UnitTypeMap;

/**
**  Next unit type are used hardcoded in the source.
**
**  @todo find a way to make it configurable!
*/
CUnitType *UnitTypeHumanWall;       /// Human wall
CUnitType *UnitTypeOrcWall;         /// Orc wall

/**
**  Default incomes for a new player.
*/
int DefaultIncomes[MaxCosts];

/**
**  Default action for the resources.
*/
std::string DefaultActions[MaxCosts];

/**
**  Default names for the resources.
*/
std::string DefaultResourceNames[MaxCosts];

/**
**  Default amounts for the resources.
*/
int DefaultResourceAmounts[MaxCosts];

/**
**  Default max amounts for the resources.
*/
int DefaultResourceMaxAmounts[MaxCosts];

/**
**  Default names for the resources.
*/
std::string ExtraDeathTypes[ANIMATIONS_DEATHTYPES];

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

int GetResourceIdByName(const char *resourceName)
{
	for (unsigned int res = 0; res < MaxCosts; ++res) {
		if (!strcmp(resourceName, DefaultResourceNames[res].c_str())) {
			return res;
		}
	}
	return -1;
}

int GetResourceIdByName(lua_State *l, const char *resourceName)
{
	const int res = GetResourceIdByName(resourceName);
	if (res == -1) {
		LuaError(l, "Resource not found: %s" _C_ resourceName);
	}
	return res;
}

/// Parse integer in animation frame.
extern int ParseAnimInt(CUnit *unit, const char *parseint);

CUnitType::CUnitType() :
	Slot(0), Width(0), Height(0), OffsetX(0), OffsetY(0), DrawLevel(0),
	ShadowWidth(0), ShadowHeight(0), ShadowOffsetX(0), ShadowOffsetY(0),
	Animations(NULL), StillFrame(0),
	DeathExplosion(NULL), OnHit(NULL), CorpseType(NULL),
	Construction(NULL), RepairHP(0), TileWidth(0), TileHeight(0),
	BoxWidth(0), BoxHeight(0), NumDirections(0), MinAttackRange(0),
	ReactRangeComputer(0), ReactRangePerson(0), Priority(0),
	BurnPercent(0), BurnDamageRate(0), RepairRange(0),
	CanCastSpell(NULL), AutoCastActive(NULL),
	AutoBuildRate(0), RandomMovementProbability(0), ClicksToExplode(0),
	MaxOnBoard(0), StartingResources(0),
	UnitType(UnitTypeLand), DecayRate(0), AnnoyComputerFactor(0),
	MouseAction(0), CanTarget(0),
	Flip(0), Revealer(0), LandUnit(0), AirUnit(0), SeaUnit(0),
	ExplodeWhenKilled(0), Building(0), VisibleUnderFog(0),
	PermanentCloak(0), DetectCloak(0),
	Coward(0), AttackFromTransporter(0),
	Vanishes(0), GroundAttack(0), ShoreBuilding(0), CanAttack(0),
	BuilderOutside(0), BuilderLost(0), CanHarvest(0), Harvester(0),
	Neutral(0), SelectableByRectangle(0), IsNotSelectable(0), Decoration(0),
	Indestructible(0), Teleporter(0), ShieldPiercing(0), SaveCargo(0),
	NonSolid(0), Wall(0),
	GivesResource(0), Supply(0), Demand(0), FieldFlags(0), MovementMask(0),
	Sprite(NULL), ShadowSprite(NULL)
{
#ifdef USE_MNG
	memset(&Portrait, 0, sizeof(Portrait));
#endif
	memset(RepairCosts, 0, sizeof(RepairCosts));
	memset(CanStore, 0, sizeof(CanStore));
	memset(ResInfo, 0, sizeof(ResInfo));
	memset(&NeutralMinimapColorRGB, 0, sizeof(NeutralMinimapColorRGB));
	memset(ImproveIncomes, 0, sizeof(ImproveIncomes));
}

CUnitType::~CUnitType()
{
	delete DeathExplosion;
	delete OnHit;

	BoolFlag.clear();

	// Free Building Restrictions if there are any
	for (std::vector<CBuildRestriction *>::iterator b = BuildingRules.begin();
		 b != BuildingRules.end(); ++b) {
		delete *b;
	}
	BuildingRules.clear();
	for (std::vector<CBuildRestriction *>::iterator b = AiBuildingRules.begin();
		 b != AiBuildingRules.end(); ++b) {
		delete *b;
	}
	AiBuildingRules.clear();

	delete[] CanCastSpell;
	delete[] AutoCastActive;

	for (int res = 0; res < MaxCosts; ++res) {
		if (this->ResInfo[res]) {
			if (this->ResInfo[res]->SpriteWhenLoaded) {
				CGraphic::Free(this->ResInfo[res]->SpriteWhenLoaded);
			}
			if (this->ResInfo[res]->SpriteWhenEmpty) {
				CGraphic::Free(this->ResInfo[res]->SpriteWhenEmpty);
			}
			delete this->ResInfo[res];
		}
	}

	CGraphic::Free(Sprite);
	CGraphic::Free(ShadowSprite);
#ifdef USE_MNG
	if (this->Portrait.Num) {
		int j;
		for (j = 0; j < this->Portrait.Num; ++j) {
			delete this->Portrait.Mngs[j];
			// delete[] this->Portrait.Files[j];
		}
		delete[] this->Portrait.Mngs;
		delete[] this->Portrait.Files;
	}
#endif

}


bool CUnitType::CheckUserBoolFlags(const char *BoolFlags) const
{
	for (unsigned int i = 0; i < UnitTypeVar.GetNumberBoolFlag(); ++i) { // User defined flags
		if (BoolFlags[i] != CONDITION_TRUE &&
			((BoolFlags[i] == CONDITION_ONLY) ^ (BoolFlag[i].value))) {
			return false;
		}
	}
	return true;
}

bool CUnitType::CanMove() const
{
	return Animations && Animations->Move;
}



/**
**  Update the player stats for changed unit types.
**  @param reset indicates wether default value should be set to each stat (level, upgrades)
*/
void UpdateStats(int reset)
{
	//
	//  Update players stats
	//
	for (std::vector<CUnitType *>::size_type j = 0; j < UnitTypes.size(); ++j) {
		CUnitType &type = *UnitTypes[j];
		if (reset) {
			for (int player = 0; player < PlayerMax; ++player) {
				type.Stats[player] = type.DefaultStat;
			}
		}

		// Non-solid units can always be entered and they don't block anything
		if (type.NonSolid) {
			if (type.Building) {
				type.MovementMask = MapFieldLandUnit |
									MapFieldSeaUnit |
									MapFieldBuilding |
									MapFieldCoastAllowed |
									MapFieldWaterAllowed |
									MapFieldNoBuilding |
									MapFieldUnpassable;
				type.FieldFlags = MapFieldNoBuilding;
			} else {
				type.MovementMask = 0;
				type.FieldFlags = 0;
			}
			continue;
		}

		//
		//  As side effect we calculate the movement flags/mask here.
		//
		switch (type.UnitType) {
			case UnitTypeLand:                              // on land
				type.MovementMask =
					MapFieldLandUnit |
					MapFieldSeaUnit |
					MapFieldBuilding | // already occuppied
					MapFieldCoastAllowed |
					MapFieldWaterAllowed | // can't move on this
					MapFieldUnpassable;
				break;
			case UnitTypeFly:                               // in air
				type.MovementMask = MapFieldAirUnit; // already occuppied
				break;
			case UnitTypeNaval:                             // on water
				if (type.CanTransport()) {
					type.MovementMask =
						MapFieldLandUnit |
						MapFieldSeaUnit |
						MapFieldBuilding | // already occuppied
						MapFieldLandAllowed; // can't move on this
					// Johns: MapFieldUnpassable only for land units?
				} else {
					type.MovementMask =
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
				type.MovementMask = 0;
				break;
		}
		if (type.Building || type.ShoreBuilding) {
			// Shore building is something special.
			if (type.ShoreBuilding) {
				type.MovementMask =
					MapFieldLandUnit |
					MapFieldSeaUnit |
					MapFieldBuilding | // already occuppied
					MapFieldLandAllowed; // can't build on this
			}
			type.MovementMask |= MapFieldNoBuilding;
			//
			// A little chaos, buildings without HP can be entered.
			// The oil-patch is a very special case.
			//
			if (type.DefaultStat.Variables[HP_INDEX].Max) {
				type.FieldFlags = MapFieldBuilding;
			} else {
				type.FieldFlags = MapFieldNoBuilding;
			}
		} else {
			switch (type.UnitType) {
				case UnitTypeLand: // on land
					type.FieldFlags = MapFieldLandUnit;
					break;
				case UnitTypeFly: // in air
					type.FieldFlags = MapFieldAirUnit;
					break;
				case UnitTypeNaval: // on water
					type.FieldFlags = MapFieldSeaUnit;
					break;
				default:
					DebugPrint("Where moves this unit?\n");
					type.FieldFlags = 0;
					break;
			}
		}
	}
}

/**
**  Save state of an unit-stats to file.
**
**  @param stats  Unit-stats to save.
**  @param ident  Unit-type ident.
**  @param plynr  Player number.
**  @param file   Output file.
*/
static bool SaveUnitStats(const CUnitStats &stats, const CUnitType &type, int plynr,
						  CFile &file)
{
	Assert(plynr < PlayerMax);

	if (stats == type.DefaultStat) {
		return false;
	}
	file.printf("DefineUnitStats(\"%s\", %d,\n  ", type.Ident.c_str(), plynr);
	for (unsigned int i = 0; i < UnitTypeVar.GetNumberVariable(); ++i) {
		file.printf("\"%s\", {Value = %d, Max = %d, Increase = %d%s},\n  ",
					UnitTypeVar.VariableNameLookup[i], stats.Variables[i].Value,
					stats.Variables[i].Max, stats.Variables[i].Increase,
					stats.Variables[i].Enable ? ", Enable = true" : "");
	}
	file.printf("\"costs\", {");
	for (unsigned int i = 0; i < MaxCosts; ++i) {
		if (i) {
			file.printf(" ");
		}
		file.printf("\"%s\", %d,", DefaultResourceNames[i].c_str(), stats.Costs[i]);
	}
	file.printf("\"storing\", {");
	for (unsigned int i = 0; i < MaxCosts; ++i) {
		if (i) {
			file.printf(" ");
		}
		file.printf("\"%s\", %d,", DefaultResourceNames[i].c_str(), stats.Storing[i]);
	}
	file.printf("})\n");
	return true;
}

/**
**  Save state of the unit-type table to file.
**
**  @param file  Output file.
*/
void SaveUnitTypes(CFile &file)
{
	file.printf("\n--- -----------------------------------------\n");
	file.printf("--- MODULE: unittypes\n\n");

	// Save all stats
	for (std::vector<CUnitType *>::size_type i = 0; i < UnitTypes.size(); ++i) {
		const CUnitType &type = *UnitTypes[i];
		bool somethingSaved = false;

		for (int j = 0; j < PlayerMax; ++j) {
			if (Players[j].Type != PlayerNobody) {
				somethingSaved |= SaveUnitStats(type.Stats[j], type, j, file);
			}
		}
		if (somethingSaved) {
			file.printf("\n");
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
CUnitType *UnitTypeByIdent(const std::string &ident)
{
	std::map<std::string, CUnitType *>::iterator ret = UnitTypeMap.find(ident);
	if (ret != UnitTypeMap.end()) {
		return (*ret).second;
	}
	return NULL;
}

/**
**  Allocate an empty unit-type slot.
**
**  @param ident  Identifier to identify the slot (malloced by caller!).
**
**  @return       New allocated (zeroed) unit-type pointer.
*/
CUnitType *NewUnitTypeSlot(const std::string &ident)
{
	CUnitType *type;
	size_t new_bool_size = UnitTypeVar.GetNumberBoolFlag();

	type = new CUnitType;
	if (!type) {
		fprintf(stderr, "Out of memory\n");
		ExitFatal(-1);
	}
	type->Slot = UnitTypes.size();
	type->Ident = ident;
	type->BoolFlag.resize(new_bool_size);

	type->DefaultStat.Variables = new CVariable[UnitTypeVar.GetNumberVariable()];
	for (unsigned int i = 0; i < UnitTypeVar.GetNumberVariable(); ++i) {
		type->DefaultStat.Variables[i] = UnitTypeVar.Variable[i];
	}
	UnitTypes.push_back(type);
	UnitTypeMap[type->Ident] = type;
	return type;
}

/**
**  Draw unit-type on map.
**
**  @param type    Unit-type pointer.
**  @param sprite  Sprite to use for drawing
**  @param player  Player number for color substitution.
**  @param frame   Animation frame of unit-type.
**  @param x       Screen X pixel postion to draw unit-type.
**  @param y       Screen Y pixel postion to draw unit-type.
**
**  @todo  Do screen position caculation in high level.
**         Better way to handle in x mirrored sprites.
*/
void DrawUnitType(const CUnitType &type, CPlayerColorGraphic *sprite, int player, int frame, int x, int y)
{
	// FIXME: move this calculation to high level.
	x -= (type.Width - type.TileWidth * PixelTileSize.x) / 2;
	y -= (type.Height - type.TileHeight * PixelTileSize.y) / 2;
	x += type.OffsetX;
	y += type.OffsetY;

	if (type.Flip) {
		if (frame < 0) {
			sprite->DrawPlayerColorFrameClipX(player, -frame - 1, x, y);
		} else {
			sprite->DrawPlayerColorFrameClip(player, frame, x, y);
		}
	} else {
		int row;

		row = type.NumDirections / 2 + 1;
		if (frame < 0) {
			frame = ((-frame - 1) / row) * type.NumDirections + type.NumDirections - (-frame - 1) % row;
		} else {
			frame = (frame / row) * type.NumDirections + frame % row;
		}
		sprite->DrawPlayerColorFrameClip(player, frame, x, y);
	}
}

/**
**  Get the still animation frame
*/
static int GetStillFrame(CUnitType *type)
{
	CAnimation *anim = type->Animations->Still;

	while (anim) {
		if (anim->Type == AnimationFrame) {
			CAnimation_Frame &a_frame = *static_cast<CAnimation_Frame *>(anim);
			// Use the frame facing down
			return a_frame.ParseAnimInt(NoUnitP) + type->NumDirections / 2;
		} else if (anim->Type == AnimationExactFrame) {
			CAnimation_ExactFrame &a_frame = *static_cast<CAnimation_ExactFrame *>(anim);

			return a_frame.ParseAnimInt(NoUnitP);
		}
		anim = anim->Next;
	}
	return type->NumDirections / 2;
}

/**
**  Init unit types.
*/
void InitUnitTypes(int reset_player_stats)
{
	CUnitType *type;

	for (size_t i = 0; i < UnitTypes.size(); ++i) {
		type = UnitTypes[i];
		Assert(type->Slot == (int)i);

		//  Add idents to hash.
		UnitTypeMap[type->Ident] = UnitTypes[i];

		// Determine still frame
		type->StillFrame = GetStillFrame(type);

		// Lookup BuildingTypes
		for (std::vector<CBuildRestriction *>::iterator b = type->BuildingRules.begin();
			 b < type->BuildingRules.end(); ++b) {
			(*b)->Init();
		}

		// Lookup AiBuildingTypes
		for (std::vector<CBuildRestriction *>::iterator b = type->AiBuildingRules.begin();
			 b < type->AiBuildingRules.end(); ++b) {
			(*b)->Init();
		}
	}

	// LUDO : called after game is loaded -> don't reset stats !
	UpdateStats(reset_player_stats); // Calculate the stats

	//
	// Setup hardcoded unit types. FIXME: should be moved to some configs.
	// Temporary fix: UnitTypeHumanWall and UnitTypeOrcWall are exported to lua
	//
	//UnitTypeHumanWall = UnitTypeByIdent("unit-human-wall");
	//UnitTypeOrcWall = UnitTypeByIdent("unit-orc-wall");
}

/**
**  Loads the Sprite for a unit type
**
**  @param type  type of unit to load
*/
void LoadUnitTypeSprite(CUnitType &type)
{
	if (!type.ShadowFile.empty()) {
		type.ShadowSprite = CGraphic::ForceNew(type.ShadowFile, type.ShadowWidth, type.ShadowHeight);
		type.ShadowSprite->Load();
		if (type.Flip) {
			type.ShadowSprite->Flip();
		}
		type.ShadowSprite->MakeShadow();
	}

	if (type.Harvester) {
		for (int i = 0; i < MaxCosts; ++i) {
			ResourceInfo *resinfo = type.ResInfo[i];
			if (!resinfo) {
				continue;
			}
			if (!resinfo->FileWhenLoaded.empty()) {
				resinfo->SpriteWhenLoaded = CPlayerColorGraphic::New(resinfo->FileWhenLoaded,
																	 type.Width, type.Height);
				resinfo->SpriteWhenLoaded->Load();
				if (type.Flip) {
					resinfo->SpriteWhenLoaded->Flip();
				}
			}
			if (!resinfo->FileWhenEmpty.empty()) {
				resinfo->SpriteWhenEmpty = CPlayerColorGraphic::New(resinfo->FileWhenEmpty,
																	type.Width, type.Height);
				resinfo->SpriteWhenEmpty->Load();
				if (type.Flip) {
					resinfo->SpriteWhenEmpty->Flip();
				}
			}
		}
	}

	if (!type.File.empty()) {
		type.Sprite = CPlayerColorGraphic::New(type.File, type.Width, type.Height);
		type.Sprite->Load();
		if (type.Flip) {
			type.Sprite->Flip();
		}
	}

#ifdef USE_MNG
	if (type.Portrait.Num) {
		for (int i = 0; i < type.Portrait.Num; ++i) {
			type.Portrait.Mngs[i] = new Mng;
			type.Portrait.Mngs[i]->Load(type.Portrait.Files[i]);
		}
		// FIXME: should be configurable
		type.Portrait.CurrMng = 0;
		type.Portrait.NumIterations = SyncRand() % 16 + 1;
	}
#endif
}

/**
** Load the graphics for the unit-types.
*/
void LoadUnitTypes()
{
	for (std::vector<CUnitType *>::size_type i = 0; i < UnitTypes.size(); ++i) {
		CUnitType &type = *UnitTypes[i];

		// Lookup icons.
		type.Icon.Load();
		// Lookup missiles.
		type.Missile.MapMissile();
		type.Explosion.MapMissile();

		// Lookup impacts
		for (int i = 0; i < ANIMATIONS_DEATHTYPES + 2; ++i) {
			type.Impact[i].MapMissile();
		}
		// Lookup corpse.
		if (!type.CorpseName.empty()) {
			type.CorpseType = UnitTypeByIdent(type.CorpseName);
		}
#ifndef DYNAMIC_LOAD
		// Load Sprite
		if (!type.Sprite) {
			ShowLoadProgress("Unit \"%s\"", type.Name.c_str());
			LoadUnitTypeSprite(type);
		}
#endif
		// FIXME: should i copy the animations of same graphics?
	}
}

void CUnitTypeVar::Init()
{
	// Variables.
	Variable.resize(GetNumberVariable());
	size_t new_size = UnitTypeVar.GetNumberBoolFlag();
	for (unsigned int i = 0; i < UnitTypes.size(); ++i) { // adjust array for unit already defined
		UnitTypes[i]->BoolFlag.resize(new_size);
	}
}

void CUnitTypeVar::Clear()
{
	Variable.clear();

	for (std::vector<CDecoVar *>::iterator it = DecoVar.begin();
		 it != DecoVar.end(); ++it) {
		delete(*it);
	}
	DecoVar.clear();
}

extern void FreeAnimations();

/**
**  Cleanup the unit-type module.
*/
void CleanUnitTypes()
{

	DebugPrint("FIXME: icon, sounds not freed.\n");
	FreeAnimations();

	// Clean all unit-types

	for (size_t i = 0; i < UnitTypes.size(); ++i) {
		delete UnitTypes[i];
	}
	UnitTypes.clear();
	UnitTypeMap.clear();
	UnitTypeVar.Clear();

	//
	// Clean hardcoded unit types.
	//
	UnitTypeHumanWall = NULL;
	UnitTypeOrcWall = NULL;
}

//@}
