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
/**@name player.cpp - The players. */
//
//      (c) Copyright 1998-2005 by Lutz Sammer, Jimmy Salmon, Nehal Mistry
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
-- Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "stratagus.h"
#include "video.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "map.h"
#include "ai.h"
#include "network.h"
#include "netconnect.h"
#include "interface.h"
#include "iolib.h"
#include "ui.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

int NumPlayers;                  /// How many player slots used
CPlayer Players[PlayerMax];       /// All players in play
CPlayer *ThisPlayer;              /// Player on this computer
PlayerRace PlayerRaces;          /// Player races

int NoRescueCheck;               /// Disable rescue check

/**
**  Colors used for minimap.
*/
SDL_Color *PlayerColorsRGB[PlayerMax];
Uint32 *PlayerColors[PlayerMax];

char *PlayerColorNames[PlayerMax];

/**
**  Which indexes to replace with player color
*/
int PlayerColorIndexStart;
int PlayerColorIndexCount;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Init players.
*/
void InitPlayers(void)
{
	int p;
	int x;

	for (p = 0; p < PlayerMax; ++p) {
		Players[p].Index = p;
		if (!Players[p].Type) {
			Players[p].Type = PlayerNobody;
		}
		for (x = 0; x < PlayerColorIndexCount; ++x) {
			PlayerColors[p][x] = Video.MapRGB(TheScreen->format, PlayerColorsRGB[p][x].r,
				PlayerColorsRGB[p][x].g, PlayerColorsRGB[p][x].b);
		}
	}
}

/**
**  Clean up players.
*/
void CleanPlayers(void)
{
	int p;

	for (p = 0; p < PlayerMax; ++p) {
		if (Players[p].Name) {
			free(Players[p].Name);
		}
		if (Players[p].Units) {
			free(Players[p].Units);
		}
	}
	ThisPlayer = NULL;
	memset(Players, 0, sizeof(Players));
	NumPlayers = 0;

	for (p = 0; p < PlayerRaces.Count; ++p) {
		free(PlayerRaces.Name[p]);
		free(PlayerRaces.Display[p]);
	}
	PlayerRaces.Count = 0;

	NoRescueCheck = 0;
}

/**
**  Save state of players to file.
**
**  @param file  Output file.
**
**  @note FIXME: Not completely saved.
*/
void SavePlayers(CFile* file)
{
	int i;
	int j;
	unsigned char r;
	unsigned char g;
	unsigned char b;

	file->printf("\n--------------------------------------------\n");
	file->printf("--- MODULE: players $Id$\n\n");

	//
	//  Dump all players
	//
	for (i = 0; i < NumPlayers; ++i) {
		file->printf("Player(%d,\n", i);
		file->printf("  \"name\", \"%s\",\n", Players[i].Name);
		file->printf("  \"type\", ");
		switch (Players[i].Type) {
			case PlayerNeutral:       file->printf("\"neutral\",");         break;
			case PlayerNobody:        file->printf("\"nobody\",");          break;
			case PlayerComputer:      file->printf("\"computer\",");        break;
			case PlayerPerson:        file->printf("\"person\",");          break;
			case PlayerRescuePassive: file->printf("\"rescue-passive\",");break;
			case PlayerRescueActive:  file->printf("\"rescue-active\","); break;
			default:                  file->printf("%d,",Players[i].Type);break;
		}
		file->printf(" \"race\", \"%s\",", PlayerRaces.Name[Players[i].Race]);
		file->printf(" \"ai-name\", \"%s\",\n", Players[i].AiName);
		file->printf("  \"team\", %d,", Players[i].Team);

		file->printf(" \"enemy\", \"");
		for (j = 0; j < PlayerMax; ++j) {
			file->printf("%c",(Players[i].Enemy & (1 << j)) ? 'X' : '_');
		}
		file->printf("\", \"allied\", \"");
		for (j = 0; j < PlayerMax; ++j) {
			file->printf("%c", (Players[i].Allied & (1 << j)) ? 'X' : '_');
		}
		file->printf("\", \"shared-vision\", \"");
		for (j = 0; j < PlayerMax; ++j) {
			file->printf("%c", (Players[i].SharedVision & (1 << j)) ? 'X' : '_');
		}
		file->printf("\",\n  \"start\", {%d, %d},\n", Players[i].StartX,
			Players[i].StartY);

		// Resources
		file->printf("  \"resources\", {");
		for (j = 0; j < MaxCosts; ++j) {
			if (j) {
				if (j == MaxCosts / 2) {
					file->printf("\n ");
				} else {
					file->printf(" ");
				}
			}
			file->printf("\"%s\", %d,", DefaultResourceNames[j],
				Players[i].Resources[j]);
		}
		// Last Resources
		file->printf("},\n  \"last-resources\", {");
		for (j = 0; j < MaxCosts; ++j) {
			if (j) {
				if (j == MaxCosts / 2) {
					file->printf("\n ");
				} else {
					file->printf(" ");
				}
			}
			file->printf("\"%s\", %d,", DefaultResourceNames[j],
				Players[i].LastResources[j]);
		}
		// Incomes
		file->printf("},\n  \"incomes\", {");
		for (j = 0; j < MaxCosts; ++j) {
			if (j) {
				if (j == MaxCosts / 2) {
					file->printf("\n ");
				} else {
					file->printf(" ");
				}
			}
			file->printf("\"%s\", %d,", DefaultResourceNames[j],
				Players[i].Incomes[j]);
		}
		// Revenue
		file->printf("},\n  \"revenue\", {");
		for (j = 0; j < MaxCosts; ++j) {
			if (j) {
				if (j == MaxCosts / 2) {
					file->printf("\n ");
				} else {
					file->printf(" ");
				}
			}
			file->printf("\"%s\", %d,", DefaultResourceNames[j],
				Players[i].Revenue[j]);
		}

		// UnitTypesCount done by load units.

		file->printf("},\n  \"%s\",\n", Players[i].AiEnabled ?
			"ai-enabled" : "ai-disabled");

		// Ai done by load ais.
		// Units done by load units.
		// TotalNumUnits done by load units.
		// NumBuildings done by load units.

		file->printf(" \"supply\", %d,", Players[i].Supply);
		file->printf(" \"unit-limit\", %d,", Players[i].UnitLimit);
		file->printf(" \"building-limit\", %d,", Players[i].BuildingLimit);
		file->printf(" \"total-unit-limit\", %d,", Players[i].TotalUnitLimit);

		file->printf("\n  \"score\", %d,", Players[i].Score);
		file->printf("\n  \"total-units\", %d,", Players[i].TotalUnits);
		file->printf("\n  \"total-buildings\", %d,", Players[i].TotalBuildings);
		file->printf("\n  \"total-resources\", {");
		for (j = 0; j < MaxCosts; ++j) {
			if (j) {
				file->printf(" ");
			}
			file->printf("%d,", Players[i].TotalResources[j]);
		}
		file->printf("},");
		file->printf("\n  \"total-razings\", %d,", Players[i].TotalRazings);
		file->printf("\n  \"total-kills\", %d,", Players[i].TotalKills);

		SDL_GetRGB(Players[i].Color, TheScreen->format, &r, &g, &b);
		file->printf("\n  \"color\", { %d, %d, %d },", r, g, b);

		// UnitColors done by init code.

		// Allow saved by allow.

		file->printf("\n  \"timers\", {");
		for (j = 0; j < UpgradeMax; ++j) {
			if (j) {
				file->printf(" ");
			}
			file->printf("%d,", Players[i].UpgradeTimers.Upgrades[j]);
		}
		file->printf("})\n\n");
	}

	DebugPrint("FIXME: must save unit-stats?\n");

	//
	//  Dump local variables
	//
	file->printf("SetThisPlayer(%d)\n\n", ThisPlayer->Index);
}

/**
**  Create a new player.
**
**  @param type  Player type (Computer,Human,...).
*/
void CreatePlayer(int type)
{
	int team;
	int i;
	CPlayer *player;

	if (NumPlayers == PlayerMax) { // already done for bigmaps!
		return;
	}
	player = &Players[NumPlayers];
	player->Index = NumPlayers;

	//  Allocate memory for the "list" of this player's units.
	//  FIXME: brutal way, as we won't need UnitMax for this player...
	//  FIXME: ARI: is this needed for 'PlayerNobody' ??
	//  FIXME: A: Johns: currently we need no init for the nobody player.
	if (!(player->Units = (CUnit **)calloc(UnitMax, sizeof(CUnit *)))) {
		DebugPrint("Not enough memory to create player %d.\n" _C_ NumPlayers);
		return;
	}

	//
	//  Take first slot for person on this computer,
	//  fill other with computer players.
	//
	if (type == PlayerPerson && !NetPlayers) {
		if (!ThisPlayer) {
			ThisPlayer = player;
		} else {
			type = PlayerComputer;
		}
	}
	if (NetPlayers && NumPlayers == NetLocalPlayerNumber) {
		ThisPlayer = &Players[NetLocalPlayerNumber];
	}

	if (NumPlayers == PlayerMax) {
		static int already_warned;

		if (!already_warned) {
			DebugPrint("Too many players\n");
			already_warned = 1;
		}
		return;
	}

	//
	//  Make simple teams:
	//  All person players are enemies.
	//
	switch (type) {
		case PlayerNeutral:
		case PlayerNobody:
		default:
			team = 0;
			player->SetName("Neutral");
			break;
		case PlayerComputer:
			team = 1;
			player->SetName("Computer");
			break;
		case PlayerPerson:
			team = 2 + NumPlayers;
			player->SetName("Person");
			break;
		case PlayerRescuePassive:
		case PlayerRescueActive:
			// FIXME: correct for multiplayer games?
			player->SetName("Computer");
			team = 2 + NumPlayers;
			break;
	}
	printf("CreatePlayer name %s\n", player->Name);

	player->Type = type;
	player->Race = 0;
	player->Team = team;
	player->Enemy = 0;
	player->Allied = 0;
	strcpy(player->AiName, "ai-passive");

	//
	//  Calculate enemy/allied mask.
	//
	for (i = 0; i < NumPlayers; ++i) {
		switch (type) {
			case PlayerNeutral:
			case PlayerNobody:
			default:
				break;
			case PlayerComputer:
				// Computer allied with computer and enemy of all persons.
				if (Players[i].Type == PlayerComputer) {
					player->Allied |= (1 << i);
					Players[i].Allied |= (1 << NumPlayers);
				} else if (Players[i].Type == PlayerPerson ||
						Players[i].Type == PlayerRescueActive) {
					player->Enemy |= (1 << i);
					Players[i].Enemy |= (1 << NumPlayers);
				}
				break;
			case PlayerPerson:
				// Humans are enemy of all?
				if (Players[i].Type == PlayerComputer ||
						Players[i].Type == PlayerPerson) {
					player->Enemy |= (1 << i);
					Players[i].Enemy |= (1 << NumPlayers);
				} else if (Players[i].Type == PlayerRescueActive ||
						Players[i].Type == PlayerRescuePassive) {
					player->Allied |= (1 << i);
					Players[i].Allied |= (1 << NumPlayers);
				}
				break;
			case PlayerRescuePassive:
				// Rescue passive are allied with persons
				if (Players[i].Type == PlayerPerson) {
					player->Allied |= (1 << i);
					Players[i].Allied |= (1 << NumPlayers);
				}
				break;
			case PlayerRescueActive:
				// Rescue active are allied with persons and enemies of computer
				if (Players[i].Type == PlayerComputer) {
					player->Enemy |= (1 << i);
					Players[i].Enemy |= (1 << NumPlayers);
				} else if (Players[i].Type == PlayerPerson) {
					player->Allied |= (1 << i);
					Players[i].Allied |= (1 << NumPlayers);
				}
				break;
		}
	}

	//
	//  Initial default incomes.
	//
	for (i = 0; i < MaxCosts; ++i) {
		player->Incomes[i] = DefaultIncomes[i];
	}

	memset(player->UnitTypesCount, 0, sizeof(player->UnitTypesCount));

	player->Supply = 0;
	player->Demand = 0;
	player->NumBuildings = 0;
	player->TotalNumUnits = 0;
	player->Score = 0;

	player->Color = PlayerColors[NumPlayers][0];

	if (Players[NumPlayers].Type == PlayerComputer ||
			Players[NumPlayers].Type == PlayerRescueActive) {
		player->AiEnabled = 1;
	} else {
		player->AiEnabled = 0;
	}

	++NumPlayers;
}

/**
**  Change player side.
**
**  @param side    New side (Race).
*/
void CPlayer::SetSide(int side)
{
	Assert(side >= 0 && side < PlayerRaces.Count);
	Assert(PlayerRaces.Name[side]);

	Race = side;
}

/**
**  Change player name.
**
**  @param name    New name.
*/
void CPlayer::SetName(const char *name)
{
	if (Name) {
		free(Name);
	}
	Name = strdup(name);
}

/*----------------------------------------------------------------------------
--  Resource management
----------------------------------------------------------------------------*/

/**
**  Change the player resource.
**
**  @param resource  Resource to change.
**  @param value     How many of this resource.
*/
void CPlayer::SetResource(int resource, int value)
{
	Resources[resource] = value;
}

/**
**  Check if the unit-type didn't break any unit limits.
**
**  @param type    Type of unit.
**
**  @return        True if enough, negative on problem.
**
**  @note The return values of the PlayerCheck functions are inconsistent.
*/
int CPlayer::CheckLimits(const CUnitType *type) const
{
	//
	//  Check game limits.
	//
	if (NumUnits < UnitMax) {
		if (type->Building && NumBuildings >= BuildingLimit) {
			Notify(NotifyYellow, -1, -1, "Building Limit Reached");
			return -1;
		}
		if (!type->Building && (TotalNumUnits - NumBuildings) >= UnitLimit) {
			Notify(NotifyYellow, -1, -1, "Unit Limit Reached");
			return -2;
		}
		if (Demand + type->Demand > Supply && type->Demand) {
			Notify(NotifyYellow, -1, -1, "Insufficient Supply, increase Supply.");
			return -3;
		}
		if (TotalNumUnits >= TotalUnitLimit) {
			Notify(NotifyYellow, -1, -1, "Total Unit Limit Reached");
			return -4;
		}
		if (UnitTypesCount[type->Slot] >=  Allow.Units[type->Slot]) {
			Notify(NotifyYellow, -1, -1, "Limit of %d Reached for this unit type",
				Allow.Units[type->Slot]);
			return -6;
		}
		return 1;
	} else {
		Notify(NotifyYellow, -1, -1, "Cannot create more units.");
		if (AiEnabled) {
			// AiNoMoreUnits(player, type);
		}
		return -5;
	}
}

/**
**  Check if enough resources for are available.
**
**  @param costs   How many costs.
**
**  @return        False if all enough, otherwise a bit mask.
**
**  @note The return values of the PlayerCheck functions are inconsistent.
*/
int CPlayer::CheckCosts(const int *costs) const
{
	int i;
	int err;

	err = 0;
	for (i = 1; i < MaxCosts; ++i) {
		if (Resources[i] < costs[i]) {
			Notify(NotifyYellow, -1, -1, "Not enough %s...%s more %s.",
				DefaultResourceNames[i], DefaultActions[i], DefaultResourceNames[i]);

			err |= 1 << i;
		}
	}

	return err;
}

/**
**  Check if enough resources for new unit is available.
**
**  @param type    Type of unit.
**
**  @return        False if all enough, otherwise a bit mask.
*/
int CPlayer::CheckUnitType(const CUnitType *type) const
{
	return CheckCosts(type->Stats[Index].Costs);
}

/**
**  Add costs to the resources
**
**  @param costs   How many costs.
*/
void CPlayer::AddCosts(const int *costs)
{
	for (int i = 1; i < MaxCosts; ++i) {
		Resources[i] += costs[i];
	}
}

/**
**  Add the costs of an unit type to resources
**
**  @param type    Type of unit.
*/
void CPlayer::AddUnitType(const CUnitType *type)
{
	// FIXME: a player could make money by upgrading and than cancel
	AddCosts(type->Stats[Index].Costs);
}

/**
**  Add a factor of costs to the resources
**
**  @param costs   How many costs.
**  @param factor  Factor of the costs to apply.
*/
void CPlayer::AddCostsFactor(const int *costs, int factor)
{
	for (int i = 1; i < MaxCosts; ++i) {
		Resources[i] += costs[i] * factor / 100;
	}
}

/**
**  Subtract costs from the resources
**
**  @param costs   How many costs.
*/
void CPlayer::SubCosts(const int *costs)
{
	for (int i = 1; i < MaxCosts; ++i) {
		Resources[i] -= costs[i];
	}
}

/**
**  Substract the costs of new unit from resources
**
**  @param type    Type of unit.
*/
void CPlayer::SubUnitType(const CUnitType *type)
{
	SubCosts(type->Stats[Index].Costs);
}

/**
**  Substract a factor of costs from the resources
**
**  @param costs   How many costs.
**  @param factor  Factor of the costs to apply.
*/
void CPlayer::SubCostsFactor(const int *costs, int factor)
{
	for (int i = 1; i < MaxCosts; ++i) {
		Resources[i] -= costs[i] * 100 / factor;
	}
}

/**
**  Have unit of type.
**
**  @param type    Type of unit.
**
**  @return        How many exists, false otherwise.
*/
int CPlayer::HaveUnitTypeByType(const CUnitType *type) const
{
	return UnitTypesCount[type->Slot];
}

/**
**  Have unit of type.
**
**  @param ident   Identifier of unit-type that should be lookuped.
**
**  @return        How many exists, false otherwise.
**
**  @note This function should not be used during run time.
*/
int CPlayer::HaveUnitTypeByIdent(const char *ident) const
{
	return UnitTypesCount[UnitTypeByIdent(ident)->Slot];
}

/**
**  Initialize the Ai for all players.
*/
void PlayersInitAi(void)
{
	for (int player = 0; player < NumPlayers; ++player) {
		if (Players[player].AiEnabled) {
			AiInit(&Players[player]);
		}
	}
}

/**
**  Handle AI of all players each game cycle.
*/
void PlayersEachCycle(void)
{
	for (int player = 0; player < NumPlayers; ++player) {
		if (Players[player].AiEnabled) {
			AiEachCycle(&Players[player]);
		}
	}
}

/**
**  Handle AI of a player each second.
**
**  @param player  the player to update AI
*/
void PlayersEachSecond(int player)
{
	int res;

	if ((GameCycle / CYCLES_PER_SECOND) % 10 == 0) {
		for (res = 0; res < MaxCosts; ++res) {
			Players[player].Revenue[res] =
				Players[player].Resources[res] -
				Players[player].LastResources[res];
			Players[player].Revenue[res] *= 6;  // estimate per minute
			Players[player].LastResources[res] =
				Players[player].Resources[res];
		}
	}
	if (Players[player].AiEnabled) {
		AiEachSecond(&Players[player]);
	}
}

#ifndef USE_OPENGL
/**
**  Change current color set to new player.
**
**  FIXME: use function pointer here.
**
**  @param player  Pointer to player.
**  @param sprite  The sprite in which the colors should be changed.
*/
void GraphicPlayerPixels(CPlayer *player, const Graphic *sprite)
{
	Assert(PlayerColorIndexCount);

	SDL_LockSurface(sprite->Surface);
	SDL_SetColors(sprite->Surface, player->UnitColors.Colors,
		PlayerColorIndexStart, PlayerColorIndexCount);
	if (sprite->SurfaceFlip) {
		SDL_SetColors(sprite->SurfaceFlip,
			player->UnitColors.Colors, PlayerColorIndexStart, PlayerColorIndexCount);
	}
	SDL_UnlockSurface(sprite->Surface);
}
#endif

/**
**  Setup the player colors for the current palette.
**
**  @todo  FIXME: could be called before PixelsXX is setup.
*/
void SetPlayersPalette(void)
{
	for (int i = 0; i < PlayerMax; ++i) {
		free(Players[i].UnitColors.Colors);
		Players[i].UnitColors.Colors = (SDL_Color*)malloc(PlayerColorIndexCount * sizeof(SDL_Color));
		memcpy(Players[i].UnitColors.Colors, PlayerColorsRGB[i],
			sizeof(SDL_Color) * PlayerColorIndexCount);
	}
}

/**
**  Output debug informations for players.
*/
void DebugPlayers(void)
{
#ifdef DEBUG
	int i;
	const char *playertype;

	DebugPrint("Nr   Color   I Name     Type         Race    Ai\n");
	DebugPrint("--  -------- - -------- ------------ ------- -----\n");
	for (i = 0; i < PlayerMax; ++i) {
		if (Players[i].Type == PlayerNobody) {
			continue;
		}
		switch (Players[i].Type) {
			case 0: playertype = "Don't know 0"; break;
			case 1: playertype = "Don't know 1"; break;
			case 2: playertype = "neutral     "; break;
			case 3: playertype = "nobody      "; break;
			case 4: playertype = "computer    "; break;
			case 5: playertype = "person      "; break;
			case 6: playertype = "rescue pas. "; break;
			case 7: playertype = "rescue akt. "; break;
			default : playertype = "?unknown?   "; break;
		}
		DebugPrint("%2d: %8.8s %c %-8.8s %s %7s %s\n" _C_ i _C_ PlayerColorNames[i] _C_
			ThisPlayer == &Players[i] ? '*' :
				Players[i].AiEnabled ? '+' : ' ' _C_
			Players[i].Name _C_ playertype _C_
			PlayerRaces.Name[Players[i].Race] _C_
			Players[i].AiName);
	}
#endif
}

/**
**  Notify player about a problem.
**
**  @param type    Problem type
**  @param x       Map X tile position
**  @param y       Map Y tile position
**  @param fmt     Message format
**  @param ...     Message varargs
**
**  @note The parameter type, isn't yet used.
**  @todo FIXME: We must also notfiy allied players.
*/
void CPlayer::Notify(int type, int x, int y, const char *fmt, ...) const
{
	char temp[128];
	va_list va;

	// Notify me, and my TEAM members
	if (this != ThisPlayer && !PlayersTeamed(ThisPlayer->Index, Index)) {
		return;
	}

	va_start(va, fmt);
	temp[sizeof(temp) - 1] = '\0';
	vsnprintf(temp, sizeof(temp) - 1, fmt, va);
	va_end(va);

	if (x != -1) {
		UI.Minimap.AddEvent(x, y);
	}
	if (this == ThisPlayer) {
		SetMessageEvent(x, y, "%s", temp);
	} else {
		SetMessageEvent(x, y, "(%s): %s", Name, temp);
	}

}

/**
**  Check if the player is an enemy
*/
bool CPlayer::IsEnemy(const CPlayer *x) const
{
	return (Enemy & (1 << x->Index)) != 0;
}

/**
**  Check if the unit is an enemy
*/
bool CPlayer::IsEnemy(const CUnit *x) const
{
	return IsEnemy(x->Player);
}

/**
**  Check if the player is an ally
*/
bool CPlayer::IsAllied(const CPlayer *x) const
{
	return (Allied & (1 << x->Index)) != 0;
}

/**
**  Check if the unit is an ally
*/
bool CPlayer::IsAllied(const CUnit *x) const
{
	return IsAllied(x->Player);
}

/**
**  Check if the player shares vision
*/
bool CPlayer::IsSharedVision(const CPlayer *x) const
{
	return (SharedVision & (1 << x->Index)) != 0 &&
		(x->SharedVision & (1 << Index)) != 0;
}

/**
**  Check if the unit shares vision
*/
bool CPlayer::IsSharedVision(const CUnit *x) const
{
	return IsSharedVision(x->Player);
}


//@}
