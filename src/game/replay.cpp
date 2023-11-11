﻿//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
/**@name replay.cpp - Replay game. */
//
//      (c) Copyright 2000-2008 by Lutz Sammer, Andreas Arens, and Jimmy Salmon.
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

//----------------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------------

#include "stratagus.h"

#include "replay.h"

#include "actions.h"
#include "commands.h"
#include "filesystem.h"
#include "game.h"
#include "interface.h"
#include "iolib.h"
#include "map.h"
#include "netconnect.h"
#include "network.h"
#include "parameters.h"
#include "player.h"
#include "script.h"
#include "settings.h"
#include "sound.h"
#include "translate.h"
#include "unit.h"
#include "unit_manager.h"
#include "unittype.h"
#include "version.h"

#include <sstream>
#include <time.h>

extern fs::path ExpandPath(const std::string &path);
extern void StartMap(const std::string &filename, bool clean);

//----------------------------------------------------------------------------
// Structures
//----------------------------------------------------------------------------

/**
**  LogEntry structure.
*/
class LogEntry
{
public:
	LogEntry() = default;

	unsigned long GameCycle = 0;
	int UnitNumber = 0;
	std::string UnitIdent;
	std::string Action;
	int Flush = 0;
	Vec2i Pos{0, 0};
	int DestUnitNumber = 0;
	std::string Value;
	int Num = 0;
	unsigned SyncRandSeed = 0;
};

/**
** Full replay structure (definition + logs)
*/
class FullReplay
{
public:
	FullReplay() { ReplaySettings.Init(); }
	std::string Comment1;
	std::string Comment2;
	std::string Comment3;
	std::string Date;
	std::string Map;
	std::string MapPath;
	unsigned MapId = 0;

	int LocalPlayer = 0;
	std::string PlayerNames[PlayerMax];

	Settings ReplaySettings;
	int Engine[3]{};
	int Network[3]{};
	std::vector<LogEntry> Commands;
};

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// Variables
//----------------------------------------------------------------------------

bool CommandLogDisabled;           /// True if command log is off
EReplayType ReplayGameType;        /// Replay game type
static bool DisabledLog;           /// Disabled log for replay
static std::unique_ptr<CFile> LogFile; /// Replay log file
static fs::path LastLogFileName;   /// Last log file name
static unsigned long NextLogCycle; /// Next log cycle number
static bool InitReplay;             /// Initialize replay
static std::unique_ptr<FullReplay> CurrentReplay;
static std::optional<std::size_t> ReplayIndex;

//----------------------------------------------------------------------------
// Log commands
//----------------------------------------------------------------------------

/**
** Allocate & fill a new FullReplay structure, from GameSettings.
**
** @return A new FullReplay structure
*/
static std::unique_ptr<FullReplay> StartReplay()
{
	auto replay = std::make_unique<FullReplay>();

	time_t now;
	char dateStr[64];

	time(&now);
	const struct tm *timeinfo = localtime(&now);
	strftime(dateStr, sizeof(dateStr), "%c", timeinfo);

	replay->Comment1 = "Generated by Stratagus Version " VERSION "";
	replay->Comment2 = "Visit " HOMEPAGE " for more information";

	replay->ReplaySettings = GameSettings;

	for (int i = 0; i < PlayerMax; ++i) {
		replay->PlayerNames[i] = Players[i].Name;
		replay->ReplaySettings.Presets[i].PlayerColor = GameSettings.Presets[i].PlayerColor;
		// Some settings are determined during CreateGame and then stored on the
		// Players array. For replay, we want to store these,
		// generally. However, this is not true for teams, since there is
		// automatic handling of alliance flags for rescueable players.
		// Additionally, teams are automatically created in CPlayer::Init.
		// The presets team is actually not used verbatim, but instead used
		// as the basis of further calculation for the actual player's team
		// (see game.cpp CreateGame right after the CreatePlayer call where
		// we check GameSettings.Presets[i].Team != SettingsPresetMapDefault)
		// So in order for the replay to work, we must not save the actual player
		// teams, but instead the preset teams.
		replay->ReplaySettings.Presets[i].AIScript = Players[i].AiName; // GameSettings.Presets[i].AIScript;
		replay->ReplaySettings.Presets[i].Race = Players[i].Race; // GameSettings.Presets[i].Race;
		replay->ReplaySettings.Presets[i].Team = GameSettings.Presets[i].Team; // Players[i].Team;
		replay->ReplaySettings.Presets[i].Type = Players[i].Type; // GameSettings.Presets[i].Type;
	}

	replay->LocalPlayer = ThisPlayer->Index;

	replay->Date = dateStr;
	replay->Map = Map.Info.Description;
	replay->MapId = (signed int)Map.Info.MapUID;
	replay->MapPath = CurrentMapPath;

	replay->Engine[0] = StratagusMajorVersion;
	replay->Engine[1] = StratagusMinorVersion;
	replay->Engine[2] = StratagusPatchLevel;

	replay->Network[0] = NetworkProtocolMajorVersion;
	replay->Network[1] = NetworkProtocolMinorVersion;
	replay->Network[2] = NetworkProtocolPatchLevel;
	return replay;
}

/**
**  Applies settings the game used at the start of the replay
*/
static void ApplyReplaySettings()
{
	if (CurrentReplay->ReplaySettings.NetGameType == NetGameTypes::SettingsMultiPlayerGame) {
		ExitNetwork1();
		NetPlayers = 2;
		NetLocalPlayerNumber = CurrentReplay->LocalPlayer;
		ReplayGameType = EReplayType::MultiPlayer;
	} else {
		ReplayGameType = EReplayType::SinglePlayer;
	}
	GameSettings = CurrentReplay->ReplaySettings;

	if (strcpy_s(CurrentMapPath, sizeof(CurrentMapPath), CurrentReplay->MapPath.c_str()) != 0) {
		ErrorPrint("Replay map path is too long\n");
		// FIXME: need to handle errors better
		Exit(1);
	}

	Map.NoFogOfWar = GameSettings.NoFogOfWar;
	FlagRevealMap = GameSettings.RevealMap;

	GameSettings.Save(+[](std::string s) { DebugPrint("%s\n", s.c_str()); });

	// FIXME : check engine version
	// FIXME : FIXME: check network version
	// FIXME : check mapid
}

static void PrintLogCommand(const LogEntry &log, CFile &file)
{
	file.printf("Log( { ");
	file.printf("GameCycle = %lu, ", log.GameCycle);
	if (log.UnitNumber != -1) {
		file.printf("UnitNumber = %d, ", log.UnitNumber);
	}
	if (!log.UnitIdent.empty()) {
		file.printf("UnitIdent = \"%s\", ", log.UnitIdent.c_str());
	}
	file.printf("Action = \"%s\", ", log.Action.c_str());
	file.printf("Flush = %d, ", log.Flush);
	if (log.Pos.x != -1 || log.Pos.y != -1) {
		file.printf("PosX = %d, PosY = %d, ", log.Pos.x, log.Pos.y);
	}
	if (log.DestUnitNumber != -1) {
		file.printf("DestUnitNumber = %d, ", log.DestUnitNumber);
	}
	if (!log.Value.empty()) {
		file.printf("Value = [[%s]], ", log.Value.c_str());
	}
	if (log.Num != -1) {
		file.printf("Num = %d, ", log.Num);
	}
	file.printf("SyncRandSeed = %d } )\n", (signed)log.SyncRandSeed);
}

/**
**  Output the FullReplay list to file
**
**  @param file  The file to output to
*/
static void SaveFullLog(CFile &file)
{
	file.printf("\n--- -----------------------------------------\n");
	file.printf("--- MODULE: replay list\n");

	file.printf("\n");
	file.printf("ReplayLog( {\n");
	file.printf("  Comment1 = \"%s\",\n", CurrentReplay->Comment1.c_str());
	file.printf("  Comment2 = \"%s\",\n", CurrentReplay->Comment2.c_str());
	file.printf("  Date = \"%s\",\n", CurrentReplay->Date.c_str());
	file.printf("  Map = \"%s\",\n", CurrentReplay->Map.c_str());
	file.printf("  MapPath = \"%s\",\n", CurrentReplay->MapPath.c_str());
	file.printf("  MapId = %u,\n", CurrentReplay->MapId);
	file.printf("  LocalPlayer = %d,\n", CurrentReplay->LocalPlayer);
	file.printf("  Players = {\n");
	for (int i = 0; i < PlayerMax; ++i) {
		file.printf("\t{ Name = \"%s\", ", CurrentReplay->PlayerNames[i].c_str());
		CurrentReplay->ReplaySettings.Presets[i].Save([&] (std::string field) {
			file.printf("%s, ", field.c_str());
		});
		file.printf("}%s", i != PlayerMax - 1 ? ",\n" : "\n");
	}
	file.printf("  },\n");
	CurrentReplay->ReplaySettings.Save([&] (std::string field) {
		file.printf("  %s,\n", field.c_str());
	}, false);
	file.printf("  Engine = { %d, %d, %d },\n",
				CurrentReplay->Engine[0], CurrentReplay->Engine[1], CurrentReplay->Engine[2]);
	file.printf("  Network = { %d, %d, %d }\n",
				CurrentReplay->Network[0], CurrentReplay->Network[1], CurrentReplay->Network[2]);
	file.printf("} )\n");
	for (const auto &command : CurrentReplay->Commands) {
		PrintLogCommand(command, file);
	}
}

/**
**  Append the LogEntry structure at the end of currentLog, and to LogFile
**
**  @param log   Pointer the replay log entry to be added
**  @param dest  The file to output to
*/
static void AppendLog(LogEntry&& log, CFile &file)
{
	PrintLogCommand(log, file);
	file.flush();

	CurrentReplay->Commands.push_back(std::move(log));
}

/**
**  Log commands into file.
**
**  This could later be used to recover, crashed games.
**
**  @param action  Command name (move,attack,...).
**  @param unit    Unit that receive the command.
**  @param flush   Append command or flush old commands.
**  @param x       optional X map position.
**  @param y       optional y map position.
**  @param dest    optional destination unit.
**  @param value   optional command argument (unit-type,...).
**  @param num     optional number argument
*/
void CommandLog(const char *action,
                const CUnit *unit,
                int flush,
                int x,
                int y,
                const CUnit *dest,
                const char *value,
                int num)
{
	if (CommandLogDisabled) { // No log wanted
		return;
	}
	//
	// Create and write header of log file. The player number is added
	// to the save file name, to test more than one player on one computer.
	//
	if (!LogFile) {
		time_t now;
		time(&now);

		fs::path path(Parameters::Instance.GetUserDirectory());
		if (!GameName.empty()) {
			path /= GameName;
		}
		path /= "logs";

		fs::create_directories(path);

		path /= "log_of_stratagus_" + std::to_string(ThisPlayer->Index) + "_"
		      + std::to_string((intmax_t) now) + ".log";

		LogFile = std::make_unique<CFile>();
		if (LogFile->open(path.string().c_str(), CL_OPEN_WRITE) == -1) {
			// don't retry for each command
			CommandLogDisabled = false;
			LogFile = nullptr;
			return;
		}
		LastLogFileName = path;
		if (CurrentReplay) {
			SaveFullLog(*LogFile);
		}
	}

	if (!CurrentReplay) {
		CurrentReplay = StartReplay();

		SaveFullLog(*LogFile);
	}

	if (!action) {
		return;
	}

	LogEntry log;

	//
	// Frame, unit, (type-ident only to be better readable).
	//
	log.GameCycle = GameCycle;

	log.UnitNumber = (unit ? UnitNumber(*unit) : -1);
	log.UnitIdent = (unit ? unit->Type->Ident : "");

	log.Action = action;
	log.Flush = flush;

	//
	// Coordinates given.
	//
	log.Pos.x = x;
	log.Pos.y = y;

	//
	// Destination given.
	//
	log.DestUnitNumber = (dest ? UnitNumber(*dest) : -1);

	//
	// Value given.
	//
	log.Value = (value ? value : "");

	//
	// Number given.
	//
	log.Num = num;

	log.SyncRandSeed = SyncRandSeed;

	// Append it to ReplayLog list
	AppendLog(std::move(log), *LogFile);
}

/**
** Parse log
*/
static int CclLog(lua_State *l)
{
	LuaCheckArgs(l, 1);
	if (!lua_istable(l, 1)) {
		LuaError(l, "incorrect argument");
	}

	Assert(CurrentReplay);

	LogEntry log;
	log.UnitNumber = -1;
	log.Pos = {-1, -1};
	log.DestUnitNumber = -1;
	log.Num = -1;

	lua_pushnil(l);
	while (lua_next(l, 1)) {
		const std::string_view value = LuaToString(l, -2);
		if (value == "GameCycle") {
			log.GameCycle = LuaToNumber(l, -1);
		} else if (value == "UnitNumber") {
			log.UnitNumber = LuaToNumber(l, -1);
		} else if (value == "UnitIdent") {
			log.UnitIdent = LuaToString(l, -1);
		} else if (value == "Action") {
			log.Action = LuaToString(l, -1);
		} else if (value == "Flush") {
			log.Flush = LuaToNumber(l, -1);
		} else if (value == "PosX") {
			log.Pos.x = LuaToNumber(l, -1);
		} else if (value == "PosY") {
			log.Pos.y = LuaToNumber(l, -1);
		} else if (value == "DestUnitNumber") {
			log.DestUnitNumber = LuaToNumber(l, -1);
		} else if (value == "Value") {
			log.Value = LuaToString(l, -1);
		} else if (value == "Num") {
			log.Num = LuaToNumber(l, -1);
		} else if (value == "SyncRandSeed") {
			log.SyncRandSeed = lua_tointeger(l, -1);
		} else {
			LuaError(l, "Unsupported key: %s", value.data());
		}
		lua_pop(l, 1);
	}
	CurrentReplay->Commands.push_back(std::move(log));
	return 0;
}

/**
** Parse replay-log
*/
static int CclReplayLog(lua_State *l)
{
	LuaCheckArgs(l, 1);
	if (!lua_istable(l, 1)) {
		LuaError(l, "incorrect argument");
	}

	Assert(CurrentReplay == nullptr);

	std::unique_ptr<FullReplay> replay = std::make_unique<FullReplay>();

	lua_pushnil(l);
	while (lua_next(l, 1) != 0) {
		std::string_view value = LuaToString(l, -2);
		if (value == "Comment1") {
			replay->Comment1 = LuaToString(l, -1);
		} else if (value == "Comment2") {
			replay->Comment2 = LuaToString(l, -1);
		} else if (value == "Comment3") {
			replay->Comment3 = LuaToString(l, -1);
		} else if (value == "Date") {
			replay->Date = LuaToString(l, -1);
		} else if (value == "Map") {
			replay->Map = LuaToString(l, -1);
		} else if (value == "MapPath") {
			replay->MapPath = LuaToString(l, -1);
		} else if (value == "MapId") {
			replay->MapId = LuaToNumber(l, -1);
		} else if (value == "LocalPlayer") {
			replay->LocalPlayer = LuaToNumber(l, -1);
		} else if (value == "Players") {
			if (!lua_istable(l, -1) || lua_rawlen(l, -1) != PlayerMax) {
				LuaError(l, "incorrect argument");
			}
			for (int j = 0; j < PlayerMax; ++j) {
				int top;

				lua_rawgeti(l, -1, j + 1);
				if (!lua_istable(l, -1)) {
					LuaError(l, "incorrect argument");
				}
				top = lua_gettop(l);
				lua_pushnil(l);
				while (lua_next(l, top) != 0) {
					value = LuaToString(l, -2);
					if (value == "Name") {
						replay->PlayerNames[j] = LuaToString(l, -1);
					} else if (value == "AIScript") {
						replay->ReplaySettings.Presets[j].AIScript = LuaToString(l, -1);
					} else if (value == "PlayerColor") {
						replay->ReplaySettings.Presets[j].PlayerColor = LuaToNumber(l, -1);
					} else if (value == "Race") {
						replay->ReplaySettings.Presets[j].Race = LuaToNumber(l, -1);
					} else if (value == "Team") {
						replay->ReplaySettings.Presets[j].Team = LuaToNumber(l, -1);
					} else if (value == "Type") {
						replay->ReplaySettings.Presets[j].Type = static_cast<PlayerTypes>(LuaToNumber(l, -1));
					} else {
						LuaError(l, "Unsupported key: %s", value.data());
					}
					lua_pop(l, 1);
				}
				lua_pop(l, 1);
			}
		} else if (value == "Engine") {
			if (!lua_istable(l, -1) || lua_rawlen(l, -1) != 3) {
				LuaError(l, "incorrect argument");
			}
			replay->Engine[0] = LuaToNumber(l, -1, 1);
			replay->Engine[1] = LuaToNumber(l, -1, 2);
			replay->Engine[2] = LuaToNumber(l, -1, 3);
		} else if (value == "Network") {
			if (!lua_istable(l, -1) || lua_rawlen(l, -1) != 3) {
				LuaError(l, "incorrect argument");
			}
			replay->Network[0] = LuaToNumber(l, -1, 1);
			replay->Network[1] = LuaToNumber(l, -1, 2);
			replay->Network[2] = LuaToNumber(l, -1, 3);
		} else {
			if (!replay->ReplaySettings.SetField({value.data(), value.size()}, LuaToNumber(l, -1))) {
				LuaError(l, "Unsupported key: %s", value.data());
			}
		}
		lua_pop(l, 1);
	}

	CurrentReplay = std::move(replay);

	// Apply CurrentReplay settings.
	if (!SaveGameLoading) {
		ApplyReplaySettings();
	} else {
		CommandLogDisabled = false;
	}

	return 0;
}

/**
**  Check if we're replaying a game
*/
bool IsReplayGame()
{
	return ReplayGameType != EReplayType::None;
}

/**
**  Save generated replay
**
**  @param file  file to save to.
*/
void SaveReplayList(CFile &file)
{
	SaveFullLog(file);
}

/**
**  Load a log file to replay a game
**
**  @param name  name of file to load.
*/
static void LoadReplay(const fs::path &name)
{
	CleanReplayLog();
	ReplayGameType = EReplayType::SinglePlayer;
	LuaLoadFile(name);

	NextLogCycle = ~0UL;
	if (!CommandLogDisabled) {
		CommandLogDisabled = true;
		DisabledLog = true;
	}
	GameObserve = true;
	InitReplay = true;
}

/**
**  End logging
*/
void EndReplayLog()
{
	if (LogFile) {
		LogFile->close();
		LogFile = nullptr;
	}
	CurrentReplay = nullptr;

	ReplayIndex = std::nullopt;
}

/**
**  Clean replay log
*/
void CleanReplayLog()
{
	CurrentReplay = nullptr;

	ReplayIndex = std::nullopt;

	// if (DisabledLog) {
	CommandLogDisabled = false;
	DisabledLog = false;
	// }
	GameObserve = false;
	NetPlayers = 0;
	ReplayGameType = EReplayType::None;
}

/**
**  Do next replay
*/
static void DoNextReplay()
{
	Assert(ReplayIndex);

	const auto &ReplayStep = CurrentReplay->Commands[*ReplayIndex];
	NextLogCycle = ReplayStep.GameCycle;

	if (NextLogCycle != GameCycle) {
		return;
	}

	const int unitSlot = ReplayStep.UnitNumber;
	const auto& action = ReplayStep.Action;
	const int flags = ReplayStep.Flush;
	const Vec2i pos(ReplayStep.Pos);
	const int arg1 = ReplayStep.Pos.x;
	const int arg2 = ReplayStep.Pos.y;
	CUnit *unit = unitSlot != -1 ? &UnitManager->GetSlotUnit(unitSlot) : nullptr;
	CUnit *dunit = (ReplayStep.DestUnitNumber != -1 ? &UnitManager->GetSlotUnit(ReplayStep.DestUnitNumber) : nullptr);
	const auto& val = ReplayStep.Value;
	const int num = ReplayStep.Num;

	Assert(unitSlot == -1 || ReplayStep.UnitIdent == unit->Type->Ident);

	if (SyncRandSeed != ReplayStep.SyncRandSeed) {
#ifdef DEBUG
		if (!ReplayStep.SyncRandSeed) {
			// Replay without the 'sync info
			ThisPlayer->Notify("%s", _("No sync info for this replay !"));
		} else {
			ThisPlayer->Notify(_("Replay got out of sync (%lu) !"), GameCycle);
			ErrorPrint("OUT OF SYNC %u != %u\n", SyncRandSeed, ReplayStep.SyncRandSeed);
			ErrorPrint("OUT OF SYNC GameCycle %lu \n", GameCycle);
			Assert(0);
			// ReplayStep = 0;
			// NextLogCycle = ~0UL;
			// return;
		}
#else
		ThisPlayer->Notify("%s", _("Replay got out of sync !"));
		ReplayIndex = std::nullopt;
		NextLogCycle = ~0UL;
		return;
#endif
	}

	if (action == "stop") {
		SendCommandStopUnit(*unit);
	} else if (action == "stand-ground") {
		SendCommandStandGround(*unit, flags);
	} else if (action == "defend") {
		SendCommandDefend(*unit, *dunit, flags);
	} else if (action == "follow") {
		SendCommandFollow(*unit, *dunit, flags);
	} else if (action == "move") {
		SendCommandMove(*unit, pos, flags);
	} else if (action == "repair") {
		SendCommandRepair(*unit, pos, dunit, flags);
	} else if (action == "auto-repair") {
		SendCommandAutoRepair(*unit, arg1);
	} else if (action == "attack") {
		SendCommandAttack(*unit, pos, dunit, flags);
	} else if (action == "attack-ground") {
		SendCommandAttackGround(*unit, pos, flags);
	} else if (action == "patrol") {
		SendCommandPatrol(*unit, pos, flags);
	} else if (action == "board") {
		SendCommandBoard(*unit, *dunit, flags);
	} else if (action == "unload") {
		SendCommandUnload(*unit, pos, dunit, flags);
	} else if (action == "build") {
		SendCommandBuildBuilding(*unit, pos, UnitTypeByIdent(val), flags);
	} else if (action == "explore") {
		SendCommandExplore(*unit, flags);
	} else if (action == "dismiss") {
		SendCommandDismiss(*unit);
	} else if (action == "resource-loc") {
		SendCommandResourceLoc(*unit, pos, flags);
	} else if (action == "resource") {
		SendCommandResource(*unit, *dunit, flags);
	} else if (action == "return") {
		SendCommandReturnGoods(*unit, dunit, flags);
	} else if (action == "train") {
		SendCommandTrainUnit(*unit, UnitTypeByIdent(val), flags);
	} else if (action == "cancel-train") {
		SendCommandCancelTraining(*unit, num, !val.empty() ? &UnitTypeByIdent(val) : nullptr);
	} else if (action == "upgrade-to") {
		SendCommandUpgradeTo(*unit, UnitTypeByIdent(val), flags);
	} else if (action == "cancel-upgrade-to") {
		SendCommandCancelUpgradeTo(*unit);
	} else if (action == "research") {
		SendCommandResearch(*unit, *CUpgrade::Get(val), flags);
	} else if (action == "cancel-research") {
		SendCommandCancelResearch(*unit);
	} else if (action == "spell-cast") {
		SendCommandSpellCast(*unit, pos, dunit, num, flags);
	} else if (action == "auto-spell-cast") {
		SendCommandAutoSpellCast(*unit, num, arg1);
	} else if (action == "diplomacy") {
		if (auto state = DiplomacyFromString(val)) {
			SendCommandDiplomacy(arg1, *state, arg2);
		} else {
			DebugPrint("Invalid diplomacy command: %s", val.data());
		}
	} else if (action == "shared-vision") {
		const bool state = to_number(val) != 0;
		SendCommandSharedVision(arg1, state, arg2);
	} else if (action == "input") {
		if (val[0] == '-') {
			CclCommand(val.substr(1).data(), false);
		} else {
			HandleCheats(val);
		}
	} else if (action == "chat") {
		SetMessage("%s", val.data());
		PlayGameSound(GameSounds.ChatMessage.Sound, MaxSampleVolume);
	} else if (action == "quit") {
		CommandQuit(arg1);
	} else {
		DebugPrint("Invalid action: %s", action.data());
	}

	++*ReplayIndex;
	if (*ReplayIndex == CurrentReplay->Commands.size()) {
		ReplayIndex = std::nullopt;
		NextLogCycle = ~0UL;
	} else {
		NextLogCycle = CurrentReplay->Commands[*ReplayIndex].GameCycle;
	}
}

/**
**  Replay user commands from log each cycle
*/
static void ReplayEachCycle()
{
	if (!CurrentReplay) {
		return;
	}
	if (InitReplay) {
		for (int i = 0; i < PlayerMax; ++i) {
			if (!CurrentReplay->PlayerNames[i].empty()) {
				Players[i].SetName(CurrentReplay->PlayerNames[i]);
			}
		}
		ReplayIndex =
			CurrentReplay->Commands.empty() ? std::nullopt : std::make_optional(std::size_t{0});
		NextLogCycle = (ReplayIndex ? CurrentReplay->Commands[*ReplayIndex].GameCycle : ~0UL);
		InitReplay = false;
	}

	if (!ReplayIndex) {
		SetMessage("%s", _("End of replay"));
		GameObserve = false;
		return;
	}

	if (NextLogCycle != ~0UL && NextLogCycle != GameCycle) {
		return;
	}

	do {
		DoNextReplay();
	} while (ReplayIndex && (NextLogCycle == ~0UL || NextLogCycle == GameCycle));

	if (!ReplayIndex) {
		SetMessage("%s", _("End of replay"));
		GameObserve = false;
	}
}

/**
**  Replay user commands from log each cycle, single player games
*/
void SinglePlayerReplayEachCycle()
{
	if (ReplayGameType == EReplayType::SinglePlayer) {
		ReplayEachCycle();
	}
}

/**
**  Replay user commands from log each cycle, multiplayer games
*/
void MultiPlayerReplayEachCycle()
{
	if (ReplayGameType == EReplayType::MultiPlayer) {
		ReplayEachCycle();
	}
}

/**
**  Save the replay
**
**  @param filename  Name of the file to save to
**
**  @return          0 for success, -1 for failure
*/
int SaveReplay(const std::string &filename)
{
	if (filename.find_first_of("\\/") != std::string::npos) {
		ErrorPrint("\\ or / not allowed in SaveReplay filename\n");
		return -1;
	}
	const auto destination = Parameters::Instance.GetUserDirectory() / GameName / "logs" / filename;

	if (!fs::copy_file(LastLogFileName, destination, fs::copy_options::overwrite_existing)) {
		ErrorPrint("Can't save to '%s'\n", destination.u8string().c_str());
		return -1;
	}

	return 0;
}

void StartReplay(const std::string &filename, bool reveal)
{
	CleanPlayers();
	LoadReplay(ExpandPath(filename));

	ReplayRevealMap = reveal;

	StartMap(CurrentMapPath, false);
}

/**
**  Register Ccl functions with lua
*/
void ReplayCclRegister()
{
	lua_register(Lua, "Log", CclLog);
	lua_register(Lua, "ReplayLog", CclReplayLog);
}

//@}
