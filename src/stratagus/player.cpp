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
/**@name player.c - The players. */
//
//      (c) Copyright 1998-2004 by Lutz Sammer, Jimmy Salmon, Nehal Mistry
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

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

int NumPlayers;                  ///< How many player slots used
Player Players[PlayerMax];       ///< All players in play
Player* ThisPlayer;              ///< Player on this computer
PlayerRace PlayerRaces;          ///< Player races

int NoRescueCheck;               ///< Disable rescue check

/**
**  Colors used for minimap.
** @todo FIXME: make this configurable
*/
static SDL_Color PlayerColorsRGB[PlayerMax][4];
Uint32 PlayerColors[PlayerMax][4];

char* PlayerColorNames[PlayerMax] = {
	"red",
	"blue",
	"green",
	"violet",
	"orange",
	"black",
	"white",
	"yellow",
	"red",
	"blue",
	"green",
	"violet",
	"orange",
	"black",
	"white",
	"yellow",
};

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Get race array index by race type
**
**  @param race  Race
**
**  @return      Index to race in PlayerRaces
*/
int PlayerRacesIndex(int race)
{
	int i;

	for (i = 0; i < PlayerRaces.Count; ++i) {
		if (PlayerRaces.Race[i] == race) {
			return i;
		}
	}
	fprintf(stderr, "Invalid race: %d\n", race);
	Assert(0);
	return PlayerRaceNeutral;
}

/**
**  Init players.
*/
void InitPlayers(void)
{
	int p;
	int x;
	// FIXME: make this configurable
	struct {
		int R; int G; int B;
	} PColors[PlayerMax][4] = {
		{{ 164, 0, 0 }, { 124, 0, 0 }, { 92, 4, 0 }, { 68, 4, 0 }},
		{{ 12, 72, 204 }, { 4, 40, 160 }, { 0, 20, 116 }, { 0, 4, 76 }},
		{{ 44, 180, 148 }, { 20, 132, 92 }, { 4, 84, 44 }, { 0, 40, 12 }},
		{{ 152, 72, 176 }, { 116, 44, 132 }, { 80, 24, 88 }, { 44, 8, 44 }},
		{{ 248, 140, 20 }, { 200, 96, 16 }, { 152, 60, 16 }, { 108, 32, 12 }},
		{{ 40, 40, 60 }, { 28, 28, 44 }, { 20, 20, 32}, { 12, 12, 20 }},
		{{ 224, 224, 224 }, { 152, 152, 180 }, { 84, 84, 128 }, { 36, 40, 76 }},
		{{ 252, 252, 72 }, { 228, 204, 40 }, { 204, 160, 16 }, { 180, 116, 0 }},
		{{ 164, 0, 0 }, { 124, 0, 0 }, { 92, 4, 0 }, { 68, 4, 0 }},
		{{ 12, 72, 204 }, { 4, 40, 160 }, { 0, 20, 116 }, { 0, 4, 76 }},
		{{ 44, 180, 148 }, { 20, 132, 92 }, { 4, 84, 44 }, { 0, 40, 12 }},
		{{ 152, 72, 176 }, { 116, 44, 132 }, { 80, 24, 88 }, { 44, 8, 44 }},
		{{ 248, 140, 20 }, { 200, 96, 16 }, { 152, 60, 16 }, { 108, 32, 12 }},
		{{ 40, 40, 60 }, { 28, 28, 44 }, { 20, 20, 32}, { 12, 12, 20 }},
		{{ 224, 224, 224 }, { 152, 152, 180 }, { 84, 84, 128 }, { 36, 40, 76 }},
		{{ 252, 252, 72 }, { 228, 204, 40 }, { 204, 160, 16 }, { 180, 116, 0 }},
	};

	// FIXME: remove this
	for (p = 0; p < PlayerMax; ++p) {
		for (x = 0; x < 4; ++x) {
			PlayerColorsRGB[p][x].r = PColors[p][x].R;
			PlayerColorsRGB[p][x].g = PColors[p][x].G;
			PlayerColorsRGB[p][x].b = PColors[p][x].B;
		}
	}

	for (p = 0; p < PlayerMax; ++p) {
		Players[p].Player = p;
		if (!Players[p].Type) {
			Players[p].Type = PlayerNobody;
		}
		for (x = 0; x < 4; ++x) {
			PlayerColors[p][x] = VideoMapRGB(TheScreen->format, PlayerColorsRGB[p][x].r,
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

	//
	// Mapping the original race numbers in puds to our internal strings
	//
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
void SavePlayers(CLFile* file)
{
	int i;
	int j;
	unsigned char r;
	unsigned char g;
	unsigned char b;

	CLprintf(file, "\n--------------------------------------------\n");
	CLprintf(file, "--- MODULE: players $Id$\n\n");

	//
	//  Dump all players
	//
	for (i = 0; i < NumPlayers; ++i) {
		CLprintf(file, "Player(%d,\n", i);
		CLprintf(file, "  \"name\", \"%s\",\n", Players[i].Name);
		CLprintf(file, "  \"type\", ");
		switch (Players[i].Type) {
			case PlayerNeutral:       CLprintf(file, "\"neutral\",");         break;
			case PlayerNobody:        CLprintf(file, "\"nobody\",");          break;
			case PlayerComputer:      CLprintf(file, "\"computer\",");        break;
			case PlayerPerson:        CLprintf(file, "\"person\",");          break;
			case PlayerRescuePassive: CLprintf(file, "\"rescue-passive\",");break;
			case PlayerRescueActive:  CLprintf(file, "\"rescue-active\","); break;
			default:                  CLprintf(file, "%d,",Players[i].Type);break;
		}
		CLprintf(file, " \"race\", \"%s\",", Players[i].RaceName);
		CLprintf(file, " \"ai\", %d,\n", Players[i].AiNum);
		CLprintf(file, "  \"team\", %d,", Players[i].Team);

		CLprintf(file, " \"enemy\", \"");
		for (j = 0; j < PlayerMax; ++j) {
			CLprintf(file, "%c",(Players[i].Enemy & (1 << j)) ? 'X' : '_');
		}
		CLprintf(file, "\", \"allied\", \"");
		for (j = 0; j < PlayerMax; ++j) {
			CLprintf(file, "%c", (Players[i].Allied & (1 << j)) ? 'X' : '_');
		}
		CLprintf(file, "\", \"shared-vision\", \"");
		for (j = 0; j < PlayerMax; ++j) {
			CLprintf(file, "%c", (Players[i].SharedVision & (1 << j)) ? 'X' : '_');
		}
		CLprintf(file, "\",\n  \"start\", {%d, %d},\n", Players[i].StartX,
			Players[i].StartY);

		// Resources
		CLprintf(file, "  \"resources\", {");
		for (j = 0; j < MaxCosts; ++j) {
			if (j) {
				if (j == MaxCosts / 2) {
					CLprintf(file, "\n ");
				} else {
					CLprintf(file, " ");
				}
			}
			CLprintf(file, "\"%s\", %d,", DefaultResourceNames[j],
				Players[i].Resources[j]);
		}
		// Last Resources
		CLprintf(file, "},\n  \"last-resources\", {");
		for (j = 0; j < MaxCosts; ++j) {
			if (j) {
				if (j == MaxCosts / 2) {
					CLprintf(file, "\n ");
				} else {
					CLprintf(file, " ");
				}
			}
			CLprintf(file, "\"%s\", %d,", DefaultResourceNames[j],
				Players[i].LastResources[j]);
		}
		// Incomes
		CLprintf(file, "},\n  \"incomes\", {");
		for (j = 0; j < MaxCosts; ++j) {
			if (j) {
				if (j == MaxCosts / 2) {
					CLprintf(file, "\n ");
				} else {
					CLprintf(file, " ");
				}
			}
			CLprintf(file, "\"%s\", %d,", DefaultResourceNames[j],
				Players[i].Incomes[j]);
		}
		// Revenue
		CLprintf(file, "},\n  \"revenue\", {");
		for (j = 0; j < MaxCosts; ++j) {
			if (j) {
				if (j == MaxCosts / 2) {
					CLprintf(file, "\n ");
				} else {
					CLprintf(file, " ");
				}
			}
			CLprintf(file, "\"%s\", %d,", DefaultResourceNames[j],
				Players[i].Revenue[j]);
		}

		// UnitTypesCount done by load units.

		CLprintf(file, "},\n  \"%s\",\n", Players[i].AiEnabled ?
			"ai-enabled" : "ai-disabled");

		// Ai done by load ais.
		// Units done by load units.
		// TotalNumUnits done by load units.
		// NumBuildings done by load units.

		CLprintf(file, " \"supply\", %d,", Players[i].Supply);
		CLprintf(file, " \"unit-limit\", %d,", Players[i].UnitLimit);
		CLprintf(file, " \"building-limit\", %d,", Players[i].BuildingLimit);
		CLprintf(file, " \"total-unit-limit\", %d,", Players[i].TotalUnitLimit);

		CLprintf(file, "\n  \"score\", %d,", Players[i].Score);
		CLprintf(file, "\n  \"total-units\", %d,", Players[i].TotalUnits);
		CLprintf(file, "\n  \"total-buildings\", %d,", Players[i].TotalBuildings);
		CLprintf(file, "\n  \"total-resources\", {");
		for (j = 0; j < MaxCosts; ++j) {
			if (j) {
				CLprintf(file, " ");
			}
			CLprintf(file, "%d,", Players[i].TotalResources[j]);
		}
		CLprintf(file, "},");
		CLprintf(file, "\n  \"total-razings\", %d,", Players[i].TotalRazings);
		CLprintf(file, "\n  \"total-kills\", %d,", Players[i].TotalKills);

		SDL_GetRGB(Players[i].Color, TheScreen->format, &r, &g, &b);
		CLprintf(file, "\n  \"color\", { %d, %d, %d },", r, g, b);

		// UnitColors done by init code.

		// Allow saved by allow.

		CLprintf(file, "\n  \"timers\", {");
		for (j = 0; j < UpgradeMax; ++j) {
			if (j) {
				CLprintf(file, " ");
			}
			CLprintf(file, "%d,", Players[i].UpgradeTimers.Upgrades[j]);
		}
		CLprintf(file, "})\n\n");
	}

	DebugPrint("FIXME: must save unit-stats?\n");

	//
	//  Dump local variables
	//
	CLprintf(file, "SetThisPlayer(%d)\n\n", ThisPlayer->Player);
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
	Player* player;

	if (NumPlayers == PlayerMax) { // already done for bigmaps!
		return;
	}
	player = &Players[NumPlayers];
	player->Player = NumPlayers;

	//  Allocate memory for the "list" of this player's units.
	//  FIXME: brutal way, as we won't need UnitMax for this player...
	//  FIXME: ARI: is this needed for 'PlayerNobody' ??
	//  FIXME: A: Johns: currently we need no init for the nobody player.
	if (!(player->Units = (Unit**)calloc(UnitMax, sizeof(Unit*)))) {
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
			PlayerSetName(player, "Neutral");
			break;
		case PlayerComputer:
			team = 1;
			PlayerSetName(player, "Computer");
			break;
		case PlayerPerson:
			team = 2 + NumPlayers;
			PlayerSetName(player, "Person");
			break;
		case PlayerRescuePassive:
		case PlayerRescueActive:
			// FIXME: correct for multiplayer games?
			PlayerSetName(player, "Computer");
			team = 2 + NumPlayers;
			break;
	}

	player->Type = type;
	player->Race = PlayerRaces.Race[0];
	player->RaceName = PlayerRaces.Name[0];
	player->Team = team;
	player->Enemy = 0;
	player->Allied = 0;
	player->AiNum = PlayerAiUniversal;

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
	//  Initial default resources.
	//
	for (i = 0; i < MaxCosts; ++i) {
		player->Resources[i] = DefaultResources[i];
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
**  @param player  Pointer to player.
**  @param side    New side (Race).
*/
void PlayerSetSide(Player* player, int side)
{
	Assert(side >= 0 && side < PlayerRaces.Count);
	Assert(PlayerRaces.Name[side]);

	player->Race = side;
	player->RaceName = PlayerRaces.Name[side];
}

/**
**  Change player name.
**
**  @param player  Pointer to player.
**  @param name    New name.
*/
void PlayerSetName(Player* player, const char* name)
{
	if (player->Name) {
		free(player->Name);
	}
	player->Name = strdup(name);
}

/**
**  Change player ai.
**
**  @param player  Pointer to player.
**  @param ai      AI type.
*/
void PlayerSetAiNum(Player* player, int ai)
{
	player->AiNum = ai;
}

/*----------------------------------------------------------------------------
--  Resource management
----------------------------------------------------------------------------*/

/**
**  Change the player resource.
**
**  @param player    Pointer to player.
**  @param resource  Resource to change.
**  @param value     How many of this resource.
*/
void PlayerSetResource(Player* player, int resource, int value)
{
	player->Resources[resource] = value;
}

/**
**  Check if the unit-type didn't break any unit limits.
**
**  @param player  Pointer to player.
**  @param type    Type of unit.
**
**  @return        True if enough, negative on problem.
**
**  @note The return values of the PlayerCheck functions are inconsistent.
*/
int PlayerCheckLimits(const Player* player, const UnitType* type)
{
	//
	//  Check game limits.
	//
	if (NumUnits < UnitMax) {
		if (type->Building && player->NumBuildings >= player->BuildingLimit) {
			NotifyPlayer(player, NotifyYellow, 0, 0, "Building Limit Reached");
			return -1;
		}
		if (!type->Building && (player->TotalNumUnits - player->NumBuildings) >= player->UnitLimit) {
			NotifyPlayer(player, NotifyYellow, 0, 0, "Unit Limit Reached");
			return -2;
		}
		if (player->Demand + type->Demand > player->Supply && type->Demand) {
			NotifyPlayer(player, NotifyYellow, 0, 0, "Insufficient Supply, increase Supply.");
			return -3;
		}
		if (player->TotalNumUnits >= player->TotalUnitLimit) {
			NotifyPlayer(player, NotifyYellow, 0, 0, "Total Unit Limit Reached");
			return -4;
		}
		if (player->UnitTypesCount[type->Slot] >=  player->Allow.Units[type->Slot]) {
			NotifyPlayer(player, NotifyYellow, 0, 0, "Limit of %d Reached for this unit type",
					player->Allow.Units[type->Slot]);
			return -6;
		}
		return 1;
	} else {
		NotifyPlayer(player, NotifyYellow, 0, 0, "Cannot create more units.");
		if (player->AiEnabled) {
			// AiNoMoreUnits(player, type);
		}
		return -5;
	}
}

/**
**  Check if enough resources for are available.
**
**  @param player  Pointer to player.
**  @param costs   How many costs.
**
**  @return        False if all enought, otherwise a bit mask.
**
**  @note The return values of the PlayerCheck functions are inconsistent.
*/
int PlayerCheckCosts(const Player* player, const int* costs)
{
	int i;
	int err;

	err = 0;
	for (i = 1; i < MaxCosts; ++i) {
		if (player->Resources[i] < costs[i]) {
			NotifyPlayer(player, NotifyYellow, 0, 0, "Not enough %s...%s more %s.",
				DefaultResourceNames[i], DefaultActions[i], DefaultResourceNames[i]);

			err |= 1 << i;
		}
	}

	return err;
}

/**
**  Check if enough resources for new unit is available.
**
**  @param player  Pointer to player, which resources are checked.
**  @param type    Type of unit.
**
**  @return        False if all enought, otherwise a bit mask.
*/
int PlayerCheckUnitType(const Player* player, const UnitType* type)
{
	return PlayerCheckCosts(player, type->Stats[player->Player].Costs);
}

/**
**  Add costs to the resources
**
**  @param player  Pointer to player.
**  @param costs   How many costs.
*/
void PlayerAddCosts(Player* player, const int* costs)
{
	int i;

	for (i = 1; i < MaxCosts; ++i) {
		player->Resources[i] += costs[i];
	}
}

/**
**  Add the costs of an unit type to resources
**
**  @param player  Pointer of player, to which the resources are added.
**  @param type    Type of unit.
*/
void PlayerAddUnitType(Player* player, const UnitType* type)
{
	// FIXME: a player could make money by upgrading and than cancel
	PlayerAddCosts(player, type->Stats[player->Player].Costs);
}

/**
**  Add a factor of costs to the resources
**
**  @param player  Pointer to player.
**  @param costs   How many costs.
**  @param factor  Factor of the costs to apply.
*/
void PlayerAddCostsFactor(Player* player, const int* costs, int factor)
{
	int i;

	for (i = 1; i < MaxCosts; ++i) {
		player->Resources[i] += costs[i] * factor / 100;
	}
}

/**
**  Substract costs from the resources
**
**  @param player  Pointer to player.
**  @param costs   How many costs.
*/
void PlayerSubCosts(Player* player, const int* costs)
{
	int i;

	for (i = 1; i < MaxCosts; ++i) {
		player->Resources[i] -= costs[i];
	}
}

/**
**  Substract the costs of new unit from resources
**
**  @param player  Pointer of player, from which the resources are removed.
**  @param type    Type of unit.
*/
void PlayerSubUnitType(Player* player, const UnitType* type)
{
	PlayerSubCosts(player, type->Stats[player->Player].Costs);
}

/**
**  Substract a factor of costs from the resources
**
**  @param player  Pointer to player.
**  @param costs   How many costs.
**  @param factor  Factor of the costs to apply.
*/
void PlayerSubCostsFactor(Player* player, const int* costs, int factor)
{
	int i;

	for (i = 1; i < MaxCosts; ++i) {
		player->Resources[i] -= costs[i] * 100 / factor;
	}
}

/**
**  Have unit of type.
**
**  @param player  Pointer to player.
**  @param type    Type of unit.
**
**  @return        How many exists, false otherwise.
*/
int HaveUnitTypeByType(const Player* player, const UnitType* type)
{
	return player->UnitTypesCount[type->Slot];
}

/**
**  Have unit of type.
**
**  @param player  Pointer to owning player.
**  @param ident   Identifier of unit-type that should be lookuped.
**
**  @return        How many exists, false otherwise.
**
**  @note This function should not be used during run time.
*/
int HaveUnitTypeByIdent(const Player* player, const char* ident)
{
	return player->UnitTypesCount[UnitTypeByIdent(ident)->Slot];
}

/**
**  Initialize the Ai for all players.
*/
void PlayersInitAi(void)
{
	int player;

	for (player = 0; player < NumPlayers; ++player) {
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
	int player;

	for (player = 0; player < NumPlayers; ++player) {
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

/**
**  Change current color set to new player.
**
**  FIXME: use function pointer here.
**
**  @param player  Pointer to player.
**  @param sprite  The sprite in which the colors should be changed.
*/
void GraphicPlayerPixels(const Player* player, const Graphic* sprite)
{
#ifndef USE_OPENGL
	SDL_LockSurface(sprite->Surface);
	SDL_SetColors(sprite->Surface, ((Player*)player)->UnitColors.Colors, 208, 4);
	if (sprite->SurfaceFlip) {
		SDL_SetColors(sprite->SurfaceFlip,
			((Player*)player)->UnitColors.Colors, 208, 4);
	}
	SDL_UnlockSurface(sprite->Surface);
#endif
}

/**
**  Setup the player colors for the current palette.
**
**  @todo
**    FIXME: need better colors for the player 8-16.
**    FIXME: could be called before PixelsXX is setup.
*/
void SetPlayersPalette(void)
{
	int i;
	int o;

	//o = rand() & 0x7; // FIXME: random colors didn't work
	o = 0;
	for (i = 0; i < PlayerMax; ++i) {
		memcpy(Players[o].UnitColors.Colors, PlayerColorsRGB[i],
			sizeof(SDL_Color) * 4);
		o = (o + 1) % PlayerMax;
	}
}

/**
**  Output debug informations for players.
*/
void DebugPlayers(void)
{
#ifdef DEBUG
	int i;
	const char* colors[16] = {
		"red", "blue", "green", "violet", "orange", "black", "white", "yellow",
		"yellow", "yellow", "yellow", "yellow", "yellow", "yellow", "yellow",
		"yellow"
	};
	const char* playertype;
	const char* playerainum;

	DebugPrint("Nr   Color   I Name     Type         Race    Ai\n");
	DebugPrint("--  -------- - -------- ------------ ------- -- ---\n");
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
		switch (Players[i].AiNum) {
			case PlayerAiLand: playerainum = "(land)"; break;
			case PlayerAiPassive: playerainum = "(passive)"; break;
			case PlayerAiAir: playerainum = "(air)"; break;
			case PlayerAiSea: playerainum = "(sea)"; break;
			default: playerainum = "?unknown?"; break;
		}
		DebugPrint("%2d: %8.8s %c %-8.8s %s %7s %2d %s\n" _C_ i _C_ colors[i] _C_
			ThisPlayer == &Players[i] ? '*' :
				Players[i].AiEnabled ? '+' : ' ' _C_
			Players[i].Name _C_ playertype _C_
			PlayerRaces.Name[PlayerRacesIndex(Players[i].Race)] _C_
			Players[i].AiNum _C_ playerainum);
	}
#endif
}

/**
**  Notify player about a problem.
**
**  @param player  Player with it
**  @param type    Problem type
**  @param x       Map X tile position
**  @param y       Map Y tile position
**  @param fmt     Message format
**  @param ...     Message varargs
**
**  @note The parameter type, isn't yet used.
**  @todo FIXME: We must also notfiy allied players.
*/
void NotifyPlayer(const Player* player,
	int type __attribute__((unused)), int x, int y, const char* fmt, ...)
{
	char temp[128];
	va_list va;

	// Notify me, and my TEAM members
	if (player != ThisPlayer && !PlayersTeamed(ThisPlayer->Player, player->Player)) {
		return;
	}

	va_start(va, fmt);
	vsprintf(temp, fmt, va);
	va_end(va);

	//
	//  FIXME: show minimap animation for the event.
	//
	if (player == ThisPlayer) {
		SetMessageEvent(x, y, "%s", temp);
	} else {
		SetMessageEvent(x, y, "(%s): %s", player->Name, temp);
	}

}

//@}
