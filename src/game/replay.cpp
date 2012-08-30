//       _________ __                 __
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
#include "game.h"
#include "interface.h"
#include "iocompat.h"
#include "iolib.h"
#include "map.h"
#include "netconnect.h"
#include "network.h"
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

extern void ExpandPath(std::string &newpath, const std::string &path);
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
	LogEntry() : GameCycle(0), Flush(0), PosX(0), PosY(0), DestUnitNumber(0),
		Num(0), SyncRandSeed(0), Next(NULL) {
		UnitNumber = 0;
	}

	unsigned long GameCycle;
	int UnitNumber;
	std::string UnitIdent;
	std::string Action;
	int Flush;
	int PosX;
	int PosY;
	int DestUnitNumber;
	std::string Value;
	int Num;
	unsigned SyncRandSeed;
	LogEntry *Next;
};

/**
**  Multiplayer Player definition
*/
class MPPlayer
{
public:
	MPPlayer() : Race(0), Team(0), Type(0) {}

	std::string Name;
	int Race;
	int Team;
	int Type;
};

/**
** Full replay structure (definition + logs)
*/
class FullReplay
{
public:
	FullReplay() :
		MapId(0), Type(0), Race(0), LocalPlayer(0),
		Resource(0), NumUnits(0), Difficulty(0), NoFow(false), RevealMap(0),
		MapRichness(0), GameType(0), Opponents(0), Commands(NULL) {
		memset(Engine, 0, sizeof(Engine));
		memset(Network, 0, sizeof(Network));
	}
	std::string Comment1;
	std::string Comment2;
	std::string Comment3;
	std::string Date;
	std::string Map;
	std::string MapPath;
	unsigned MapId;

	int Type;
	int Race;
	int LocalPlayer;
	MPPlayer Players[PlayerMax];

	int Resource;
	int NumUnits;
	int Difficulty;
	bool NoFow;
	int RevealMap;
	int MapRichness;
	int GameType;
	int Opponents;
	int Engine[3];
	int Network[3];
	LogEntry *Commands;
};

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// Variables
//----------------------------------------------------------------------------

bool CommandLogDisabled;           /// True if command log is off
ReplayType ReplayGameType;         /// Replay game type
static bool DisabledLog;           /// Disabled log for replay
static CFile *LogFile;             /// Replay log file
static unsigned long NextLogCycle; /// Next log cycle number
static int InitReplay;             /// Initialize replay
static FullReplay *CurrentReplay;
static LogEntry *ReplayStep;

//----------------------------------------------------------------------------
// Log commands
//----------------------------------------------------------------------------

/**
** Allocate & fill a new FullReplay structure, from GameSettings.
**
** @return A new FullReplay structure
*/
static FullReplay *StartReplay()
{
	FullReplay *replay = new FullReplay;

	time_t now;
	char dateStr[64];

	time(&now);
	const struct tm *timeinfo = localtime(&now);
	strftime(dateStr, sizeof(dateStr), "%c", timeinfo);

	replay->Comment1 = "Generated by Stratagus Version " VERSION "";
	replay->Comment2 = "Visit " HOMEPAGE " for more information";

	if (GameSettings.NetGameType == SettingsSinglePlayerGame) {
		replay->Type = ReplaySinglePlayer;
	} else {
		replay->Type = ReplayMultiPlayer;
	}

	for (int i = 0; i < PlayerMax; ++i) {
		replay->Players[i].Name = Players[i].Name;
		replay->Players[i].Race = GameSettings.Presets[i].Race;
		replay->Players[i].Team = GameSettings.Presets[i].Team;
		replay->Players[i].Type = GameSettings.Presets[i].Type;
	}

	replay->LocalPlayer = ThisPlayer->Index;

	replay->Date = dateStr;
	replay->Map = Map.Info.Description;
	replay->MapId = (signed int)Map.Info.MapUID;
	replay->MapPath = CurrentMapPath;
	replay->Resource = GameSettings.Resources;
	replay->NumUnits = GameSettings.NumUnits;
	replay->Difficulty = GameSettings.Difficulty;
	replay->NoFow = GameSettings.NoFogOfWar;
	replay->GameType = GameSettings.GameType;
	replay->RevealMap = GameSettings.RevealMap;
	replay->MapRichness = GameSettings.MapRichness;
	replay->Opponents = GameSettings.Opponents;

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
	if (CurrentReplay->Type == ReplayMultiPlayer) {
		ExitNetwork1();
		NetPlayers = 2;
		GameSettings.NetGameType = SettingsMultiPlayerGame;

		ReplayGameType = ReplayMultiPlayer;
		NetLocalPlayerNumber = CurrentReplay->LocalPlayer;
	} else {
		GameSettings.NetGameType = SettingsSinglePlayerGame;
		ReplayGameType = ReplaySinglePlayer;
	}

	for (int i = 0; i < PlayerMax; ++i) {
		GameSettings.Presets[i].Race = CurrentReplay->Players[i].Race;
		GameSettings.Presets[i].Team = CurrentReplay->Players[i].Team;
		GameSettings.Presets[i].Type = CurrentReplay->Players[i].Type;
	}

	if (strcpy_s(CurrentMapPath, sizeof(CurrentMapPath), CurrentReplay->MapPath.c_str()) != 0) {
		fprintf(stderr, "Replay map path is too long\n");
		// FIXME: need to handle errors better
		Exit(1);
	}
	GameSettings.Resources = CurrentReplay->Resource;
	GameSettings.NumUnits = CurrentReplay->NumUnits;
	GameSettings.Difficulty = CurrentReplay->Difficulty;
	Map.NoFogOfWar = GameSettings.NoFogOfWar = CurrentReplay->NoFow;
	GameSettings.GameType = CurrentReplay->GameType;
	FlagRevealMap = GameSettings.RevealMap = CurrentReplay->RevealMap;
	GameSettings.MapRichness = CurrentReplay->MapRichness;
	GameSettings.Opponents = CurrentReplay->Opponents;

	// FIXME : check engine version
	// FIXME : FIXME: check network version
	// FIXME : check mapid
}

/**
**  Free a replay from memory
**
**  @param replay  Pointer to the replay to be freed
*/
static void DeleteReplay(FullReplay *replay)
{
	LogEntry *log = replay->Commands;

	while (log) {
		LogEntry *next = log->Next;
		delete log;
		log = next;
	}
	delete replay;
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
	if (log.PosX != -1 || log.PosY != -1) {
		file.printf("PosX = %d, PosY = %d, ", log.PosX, log.PosY);
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
	file.printf("  Type = %d,\n", CurrentReplay->Type);
	file.printf("  Race = %d,\n", CurrentReplay->Race);
	file.printf("  LocalPlayer = %d,\n", CurrentReplay->LocalPlayer);
	file.printf("  Players = {\n");
	for (int i = 0; i < PlayerMax; ++i) {
		if (!CurrentReplay->Players[i].Name.empty()) {
			file.printf("\t{ Name = \"%s\",", CurrentReplay->Players[i].Name.c_str());
		} else {
			file.printf("\t{");
		}
		file.printf(" Race = %d,", CurrentReplay->Players[i].Race);
		file.printf(" Team = %d,", CurrentReplay->Players[i].Team);
		file.printf(" Type = %d }%s", CurrentReplay->Players[i].Type,
					i != PlayerMax - 1 ? ",\n" : "\n");
	}
	file.printf("  },\n");
	file.printf("  Resource = %d,\n", CurrentReplay->Resource);
	file.printf("  NumUnits = %d,\n", CurrentReplay->NumUnits);
	file.printf("  Difficulty = %d,\n", CurrentReplay->Difficulty);
	file.printf("  NoFow = %s,\n", CurrentReplay->NoFow ? "true" : "false");
	file.printf("  RevealMap = %d,\n", CurrentReplay->RevealMap);
	file.printf("  GameType = %d,\n", CurrentReplay->GameType);
	file.printf("  Opponents = %d,\n", CurrentReplay->Opponents);
	file.printf("  MapRichness = %d,\n", CurrentReplay->MapRichness);
	file.printf("  Engine = { %d, %d, %d },\n",
				CurrentReplay->Engine[0], CurrentReplay->Engine[1], CurrentReplay->Engine[2]);
	file.printf("  Network = { %d, %d, %d }\n",
				CurrentReplay->Network[0], CurrentReplay->Network[1], CurrentReplay->Network[2]);
	file.printf("} )\n");
	const LogEntry *log = CurrentReplay->Commands;
	while (log) {
		PrintLogCommand(*log, file);
		log = log->Next;
	}
}

/**
**  Append the LogEntry structure at the end of currentLog, and to LogFile
**
**  @param log   Pointer the replay log entry to be added
**  @param dest  The file to output to
*/
static void AppendLog(LogEntry *log, CFile &file)
{
	LogEntry **last;

	// Append to linked list
	last = &CurrentReplay->Commands;
	while (*last) {
		last = &(*last)->Next;
	}

	*last = log;
	log->Next = 0;

	PrintLogCommand(*log, file);
	file.flush();
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
void CommandLog(const char *action, const CUnit *unit, int flush,
				int x, int y, const CUnit *dest, const char *value, int num)
{
	if (CommandLogDisabled) { // No log wanted
		return;
	}
	//
	// Create and write header of log file. The player number is added
	// to the save file name, to test more than one player on one computer.
	//
	if (!LogFile) {
		struct stat tmp;
		char buf[16];
		std::string path(Parameters::Instance.GetUserDirectory());
		if (!GameName.empty()) {
			path += "/";
			path += GameName;
		}
		path += "/logs";

		if (stat(path.c_str(), &tmp) < 0) {
			makedir(path.c_str(), 0777);
		}

		snprintf(buf, sizeof(buf), "%d", ThisPlayer->Index);

		path += "/log_of_stratagus_";
		path += buf;
		path += ".log";

		LogFile = new CFile;
		if (LogFile->open(path.c_str(), CL_OPEN_WRITE) == -1) {
			// don't retry for each command
			CommandLogDisabled = false;
			delete LogFile;
			LogFile = NULL;
			return;
		}

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

	LogEntry *log = new LogEntry;

	//
	// Frame, unit, (type-ident only to be better readable).
	//
	log->GameCycle = GameCycle;

	log->UnitNumber = (unit ? UnitNumber(*unit) : -1);
	log->UnitIdent = (unit ? unit->Type->Ident.c_str() : "");

	log->Action = action;
	log->Flush = flush;

	//
	// Coordinates given.
	//
	log->PosX = x;
	log->PosY = y;

	//
	// Destination given.
	//
	log->DestUnitNumber = (dest ? UnitNumber(*dest) : -1);

	//
	// Value given.
	//
	log->Value = (value ? value : "");

	//
	// Number given.
	//
	log->Num = num;

	log->SyncRandSeed = SyncRandSeed;

	// Append it to ReplayLog list
	AppendLog(log, *LogFile);
}

/**
** Parse log
*/
static int CclLog(lua_State *l)
{
	LogEntry *log;
	LogEntry **last;
	const char *value;

	LuaCheckArgs(l, 1);
	if (!lua_istable(l, 1)) {
		LuaError(l, "incorrect argument");
	}

	Assert(CurrentReplay);

	log = new LogEntry;
	log->UnitNumber = -1;
	log->PosX = -1;
	log->PosY = -1;
	log->DestUnitNumber = -1;
	log->Num = -1;

	lua_pushnil(l);
	while (lua_next(l, 1)) {
		value = LuaToString(l, -2);
		if (!strcmp(value, "GameCycle")) {
			log->GameCycle = LuaToNumber(l, -1);
		} else if (!strcmp(value, "UnitNumber")) {
			log->UnitNumber = LuaToNumber(l, -1);
		} else if (!strcmp(value, "UnitIdent")) {
			log->UnitIdent = LuaToString(l, -1);
		} else if (!strcmp(value, "Action")) {
			log->Action = LuaToString(l, -1);
		} else if (!strcmp(value, "Flush")) {
			log->Flush = LuaToNumber(l, -1);
		} else if (!strcmp(value, "PosX")) {
			log->PosX = LuaToNumber(l, -1);
		} else if (!strcmp(value, "PosY")) {
			log->PosY = LuaToNumber(l, -1);
		} else if (!strcmp(value, "DestUnitNumber")) {
			log->DestUnitNumber = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Value")) {
			log->Value = LuaToString(l, -1);
		} else if (!strcmp(value, "Num")) {
			log->Num = LuaToNumber(l, -1);
		} else if (!strcmp(value, "SyncRandSeed")) {
			log->SyncRandSeed = (unsigned)LuaToNumber(l, -1);
		} else {
			LuaError(l, "Unsupported key: %s" _C_ value);
		}
		lua_pop(l, 1);
	}

	// Append to linked list
	last = &CurrentReplay->Commands;
	while (*last) {
		last = &(*last)->Next;
	}

	*last = log;

	return 0;
}

/**
** Parse replay-log
*/
static int CclReplayLog(lua_State *l)
{
	FullReplay *replay;
	const char *value;
	int j;

	LuaCheckArgs(l, 1);
	if (!lua_istable(l, 1)) {
		LuaError(l, "incorrect argument");
	}

	Assert(CurrentReplay == NULL);

	replay = new FullReplay;

	lua_pushnil(l);
	while (lua_next(l, 1) != 0) {
		value = LuaToString(l, -2);
		if (!strcmp(value, "Comment1")) {
			replay->Comment1 = LuaToString(l, -1);
		} else if (!strcmp(value, "Comment2")) {
			replay->Comment2 = LuaToString(l, -1);
		} else if (!strcmp(value, "Comment3")) {
			replay->Comment3 = LuaToString(l, -1);
		} else if (!strcmp(value, "Date")) {
			replay->Date = LuaToString(l, -1);
		} else if (!strcmp(value, "Map")) {
			replay->Map = LuaToString(l, -1);
		} else if (!strcmp(value, "MapPath")) {
			replay->MapPath = LuaToString(l, -1);
		} else if (!strcmp(value, "MapId")) {
			replay->MapId = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Type")) {
			replay->Type = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Race")) {
			replay->Race = LuaToNumber(l, -1);
		} else if (!strcmp(value, "LocalPlayer")) {
			replay->LocalPlayer = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Players")) {
			if (!lua_istable(l, -1) || lua_rawlen(l, -1) != PlayerMax) {
				LuaError(l, "incorrect argument");
			}
			for (j = 0; j < PlayerMax; ++j) {
				int top;

				lua_rawgeti(l, -1, j + 1);
				if (!lua_istable(l, -1)) {
					LuaError(l, "incorrect argument");
				}
				top = lua_gettop(l);
				lua_pushnil(l);
				while (lua_next(l, top) != 0) {
					value = LuaToString(l, -2);
					if (!strcmp(value, "Name")) {
						replay->Players[j].Name = LuaToString(l, -1);
					} else if (!strcmp(value, "Race")) {
						replay->Players[j].Race = LuaToNumber(l, -1);
					} else if (!strcmp(value, "Team")) {
						replay->Players[j].Team = LuaToNumber(l, -1);
					} else if (!strcmp(value, "Type")) {
						replay->Players[j].Type = LuaToNumber(l, -1);
					} else {
						LuaError(l, "Unsupported key: %s" _C_ value);
					}
					lua_pop(l, 1);
				}
				lua_pop(l, 1);
			}
		} else if (!strcmp(value, "Resource")) {
			replay->Resource = LuaToNumber(l, -1);
		} else if (!strcmp(value, "NumUnits")) {
			replay->NumUnits = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Difficulty")) {
			replay->Difficulty = LuaToNumber(l, -1);
		} else if (!strcmp(value, "NoFow")) {
			replay->NoFow = LuaToBoolean(l, -1);
		} else if (!strcmp(value, "RevealMap")) {
			replay->RevealMap = LuaToNumber(l, -1);
		} else if (!strcmp(value, "GameType")) {
			replay->GameType = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Opponents")) {
			replay->Opponents = LuaToNumber(l, -1);
		} else if (!strcmp(value, "MapRichness")) {
			replay->MapRichness = LuaToNumber(l, -1);
		} else if (!strcmp(value, "Engine")) {
			if (!lua_istable(l, -1) || lua_rawlen(l, -1) != 3) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, -1, 1);
			replay->Engine[0] = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			replay->Engine[1] = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 3);
			replay->Engine[2] = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "Network")) {
			if (!lua_istable(l, -1) || lua_rawlen(l, -1) != 3) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, -1, 1);
			replay->Network[0] = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 2);
			replay->Network[1] = LuaToNumber(l, -1);
			lua_pop(l, 1);
			lua_rawgeti(l, -1, 3);
			replay->Network[2] = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else {
			LuaError(l, "Unsupported key: %s" _C_ value);
		}
		lua_pop(l, 1);
	}

	CurrentReplay = replay;

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
	return ReplayGameType != ReplayNone;
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
int LoadReplay(const std::string &name)
{
	CleanReplayLog();
	ReplayGameType = ReplaySinglePlayer;

	LuaLoadFile(name);

	NextLogCycle = ~0UL;
	if (!CommandLogDisabled) {
		CommandLogDisabled = true;
		DisabledLog = true;
	}
	GameObserve = true;
	InitReplay = 1;

	return 0;
}

/**
**  End logging
*/
void EndReplayLog()
{
	if (LogFile) {
		LogFile->close();
		delete LogFile;
		LogFile = NULL;
	}
	if (CurrentReplay) {
		DeleteReplay(CurrentReplay);
		CurrentReplay = NULL;
	}
	ReplayStep = NULL;
}

/**
**  Clean replay log
*/
void CleanReplayLog()
{
	if (CurrentReplay) {
		DeleteReplay(CurrentReplay);
		CurrentReplay = 0;
	}
	ReplayStep = NULL;

	// if (DisabledLog) {
	CommandLogDisabled = false;
	DisabledLog = false;
	// }
	GameObserve = false;
	NetPlayers = 0;
	ReplayGameType = ReplayNone;
}

/**
**  Do next replay
*/
static void DoNextReplay()
{
	Assert(ReplayStep != 0);

	NextLogCycle = ReplayStep->GameCycle;

	if (NextLogCycle != GameCycle) {
		return;
	}

	const int unitSlot = ReplayStep->UnitNumber;
	const char *action = ReplayStep->Action.c_str();
	const int flags = ReplayStep->Flush;
	const Vec2i pos(ReplayStep->PosX, ReplayStep->PosY);
	const int arg1 = ReplayStep->PosX;
	const int arg2 = ReplayStep->PosY;
	CUnit *unit = unitSlot != -1 ? &UnitManager.GetSlotUnit(unitSlot) : NULL;
	CUnit *dunit = (ReplayStep->DestUnitNumber != -1 ? &UnitManager.GetSlotUnit(ReplayStep->DestUnitNumber) : NULL);
	const char *val = ReplayStep->Value.c_str();
	const int num = ReplayStep->Num;

	Assert(unitSlot == -1 || ReplayStep->UnitIdent == unit->Type->Ident);

	if (SyncRandSeed != ReplayStep->SyncRandSeed) {
#ifdef DEBUG
		if (!ReplayStep->SyncRandSeed) {
			// Replay without the 'sync info
			ThisPlayer->Notify("%s", _("No sync info for this replay !"));
		} else {
			ThisPlayer->Notify(_("Replay got out of sync (%lu) !"), GameCycle);
			DebugPrint("OUT OF SYNC %u != %u\n" _C_ SyncRandSeed _C_ ReplayStep->SyncRandSeed);
			DebugPrint("OUT OF SYNC GameCycle %lu \n" _C_ GameCycle);
			Assert(0);
			// ReplayStep = 0;
			// NextLogCycle = ~0UL;
			// return;
		}
#else
		ThisPlayer->Notify("%s", _("Replay got out of sync !"));
		ReplayStep = 0;
		NextLogCycle = ~0UL;
		return;
#endif
	}

	if (!strcmp(action, "stop")) {
		SendCommandStopUnit(*unit);
	} else if (!strcmp(action, "stand-ground")) {
		SendCommandStandGround(*unit, flags);
	} else if (!strcmp(action, "follow")) {
		SendCommandFollow(*unit, *dunit, flags);
	} else if (!strcmp(action, "move")) {
		SendCommandMove(*unit, pos, flags);
	} else if (!strcmp(action, "repair")) {
		SendCommandRepair(*unit, pos, dunit, flags);
	} else if (!strcmp(action, "auto-repair")) {
		SendCommandAutoRepair(*unit, arg1);
	} else if (!strcmp(action, "attack")) {
		SendCommandAttack(*unit, pos, dunit, flags);
	} else if (!strcmp(action, "attack-ground")) {
		SendCommandAttackGround(*unit, pos, flags);
	} else if (!strcmp(action, "patrol")) {
		SendCommandPatrol(*unit, pos, flags);
	} else if (!strcmp(action, "board")) {
		SendCommandBoard(*unit, *dunit, flags);
	} else if (!strcmp(action, "unload")) {
		SendCommandUnload(*unit, pos, dunit, flags);
	} else if (!strcmp(action, "build")) {
		SendCommandBuildBuilding(*unit, pos, *UnitTypeByIdent(val), flags);
	} else if (!strcmp(action, "dismiss")) {
		SendCommandDismiss(*unit);
	} else if (!strcmp(action, "resource-loc")) {
		SendCommandResourceLoc(*unit, pos, flags);
	} else if (!strcmp(action, "resource")) {
		SendCommandResource(*unit, *dunit, flags);
	} else if (!strcmp(action, "return")) {
		SendCommandReturnGoods(*unit, dunit, flags);
	} else if (!strcmp(action, "train")) {
		SendCommandTrainUnit(*unit, *UnitTypeByIdent(val), flags);
	} else if (!strcmp(action, "cancel-train")) {
		SendCommandCancelTraining(*unit, num, (val && *val) ? UnitTypeByIdent(val) : NULL);
	} else if (!strcmp(action, "upgrade-to")) {
		SendCommandUpgradeTo(*unit, *UnitTypeByIdent(val), flags);
	} else if (!strcmp(action, "cancel-upgrade-to")) {
		SendCommandCancelUpgradeTo(*unit);
	} else if (!strcmp(action, "research")) {
		SendCommandResearch(*unit, *CUpgrade::Get(val), flags);
	} else if (!strcmp(action, "cancel-research")) {
		SendCommandCancelResearch(*unit);
	} else if (!strcmp(action, "spell-cast")) {
		SendCommandSpellCast(*unit, pos, dunit, num, flags);
	} else if (!strcmp(action, "auto-spell-cast")) {
		SendCommandAutoSpellCast(*unit, num, arg1);
	} else if (!strcmp(action, "diplomacy")) {
		int state;
		if (!strcmp(val, "neutral")) {
			state = DiplomacyNeutral;
		} else if (!strcmp(val, "allied")) {
			state = DiplomacyAllied;
		} else if (!strcmp(val, "enemy")) {
			state = DiplomacyEnemy;
		} else if (!strcmp(val, "crazy")) {
			state = DiplomacyCrazy;
		} else {
			DebugPrint("Invalid diplomacy command: %s" _C_ val);
			state = -1;
		}
		SendCommandDiplomacy(arg1, state, arg2);
	} else if (!strcmp(action, "shared-vision")) {
		bool state;
		state = atoi(val) ? true : false;
		SendCommandSharedVision(arg1, state, arg2);
	} else if (!strcmp(action, "input")) {
		if (val[0] == '-') {
			CclCommand(val + 1, false);
		} else {
			HandleCheats(val);
		}
	} else if (!strcmp(action, "chat")) {
		SetMessage("%s", val);
		PlayGameSound(GameSounds.ChatMessage.Sound, MaxSampleVolume);
	} else if (!strcmp(action, "quit")) {
		CommandQuit(arg1);
	} else {
		DebugPrint("Invalid action: %s" _C_ action);
	}

	ReplayStep = ReplayStep->Next;
	NextLogCycle = ReplayStep ? (unsigned)ReplayStep->GameCycle : ~0UL;
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
			if (!CurrentReplay->Players[i].Name.empty()) {
				Players[i].SetName(CurrentReplay->Players[i].Name);
			}
		}
		ReplayStep = CurrentReplay->Commands;
		NextLogCycle = (ReplayStep ? (unsigned)ReplayStep->GameCycle : ~0UL);
		InitReplay = 0;
	}

	if (!ReplayStep) {
		SetMessage("%s", _("End of replay"));
		GameObserve = false;
		return;
	}

	if (NextLogCycle != ~0UL && NextLogCycle != GameCycle) {
		return;
	}

	do {
		DoNextReplay();
	} while (ReplayStep && (NextLogCycle == ~0UL || NextLogCycle == GameCycle));

	if (!ReplayStep) {
		SetMessage("%s", _("End of replay"));
		GameObserve = false;
	}
}

/**
**  Replay user commands from log each cycle, single player games
*/
void SinglePlayerReplayEachCycle()
{
	if (ReplayGameType == ReplaySinglePlayer) {
		ReplayEachCycle();
	}
}

/**
**  Replay user commands from log each cycle, multiplayer games
*/
void MultiPlayerReplayEachCycle()
{
	if (ReplayGameType == ReplayMultiPlayer) {
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
	FILE *fd;
	char *buf;
	std::ostringstream logfile;
	std::string destination;
	struct stat sb;
	size_t size;

	if (filename.find_first_of("\\/") != std::string::npos) {
		fprintf(stderr, "\\ or / not allowed in SaveReplay filename\n");
		return -1;
	}

	destination = Parameters::Instance.GetUserDirectory() + "/" + GameName + "/logs/" + filename;

	logfile << Parameters::Instance.GetUserDirectory() << "/" << GameName << "/logs/log_of_stratagus_" << ThisPlayer->Index << ".log";

	if (stat(logfile.str().c_str(), &sb)) {
		fprintf(stderr, "stat failed\n");
		return -1;
	}
	buf = new char[sb.st_size];
	if (!buf) {
		fprintf(stderr, "Out of memory\n");
		return -1;
	}
	fd = fopen(logfile.str().c_str(), "rb");
	if (!fd) {
		fprintf(stderr, "fopen failed\n");
		delete[] buf;
		return -1;
	}
	size = fread(buf, sb.st_size, 1, fd);
	fclose(fd);

	fd = fopen(destination.c_str(), "wb");
	if (!fd) {
		fprintf(stderr, "Can't save to `%s'\n", destination.c_str());
		delete[] buf;
		return -1;
	}
	fwrite(buf, sb.st_size, size, fd);
	fclose(fd);

	delete[] buf;

	return 0;
}

void StartReplay(const std::string &filename, bool reveal)
{
	std::string replay;

	CleanPlayers();
	ExpandPath(replay, filename);
	LoadReplay(replay);

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
