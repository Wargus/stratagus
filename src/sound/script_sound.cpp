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
/**@name script_sound.cpp - The sound ccl functions. */
//
//      (c) Copyright 1999-2007 by Lutz Sammer, Fabrice Rossi, and Jimmy Salmon
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
-- Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "sound.h"

#include "player.h"
#include "script.h"
#include "sound_server.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Glue between c and scheme. Ask the sound system to associate a
**  sound id to a sound name.
**
**  @param l  Lua state.
*/
static int CclSoundForName(lua_State *l)
{
	const std::string sound_name = std::string{LuaToString(l, -1)};
	CSound *id = SoundForName(sound_name);

	LuaUserData *data = (LuaUserData *)lua_newuserdata(l, sizeof(LuaUserData));
	data->Type = LuaSoundType;
	data->Data = id;
	return 1;
}

/**
**  Get a Game Sound Id from either a lua sound id or a sound name
**
**  @param l  Lua state.
**
**  @return   The C sound id.
*/
static CSound *CclGetSound(lua_State *l)
{
	bool pop = false;
	if (lua_isstring(l, -1)) {
		CclSoundForName(l);
		pop = true;
	}
	if (lua_isuserdata(l, -1)) {
		LuaUserData *data = (LuaUserData *)lua_touserdata(l, -1);
		if (data->Type == LuaSoundType) {
			if (pop) {
				lua_pop(l, 1);
			}
			return (CSound *)data->Data;
		}
	}
	LuaError(l, "CclGetSound: not a sound");
	return nullptr;
}

/**
**  Create a sound.
**
**  Glue between c and scheme. This function asks the sound system to
**  register a sound under a given name, with an associated list of files
**  (the list can be replaced by only one file).
**
**  @param l  Lua state.
**
**  @return   the sound id of the created sound
*/
static int CclMakeSound(lua_State *l)
{
	LuaCheckArgs(l, 2);

	std::string c_name = std::string{LuaToString(l, 1)};
	std::vector<std::string> files;
	CSound *id = nullptr;
	if (lua_isstring(l, 2)) {
		// only one file
		files.push_back(std::string{LuaToString(l, 2)});
		id = MakeSound(c_name, files);
	} else if (lua_istable(l, 2)) {
		// several files
		const int args = lua_rawlen(l, 2);
		files.reserve(args);
		for (int j = 0; j < args; ++j) {
			files.push_back(std::string{LuaToString(l, 2, j + 1)});
		}
		id = MakeSound(c_name, files);
	} else {
		LuaError(l, "string or table expected");
		return 0;
	}
	LuaUserData *data = (LuaUserData *)lua_newuserdata(l, sizeof(LuaUserData));
	data->Type = LuaSoundType;
	data->Data = id;
	return 1;
}

/**
**  Glue between c and scheme. This function asks the sound system to
**  build a special sound group.
**
**  @param l  Lua state.
**
**  @return   The sound id of the created sound
*/
static int CclMakeSoundGroup(lua_State *l)
{
	LuaCheckArgs(l, 3);

	std::string c_name{LuaToString(l, 1)};

	lua_pushvalue(l, 2);
	CSound *first = CclGetSound(l);
	lua_pop(l, 1);
	CSound *second = CclGetSound(l);
	CSound *id = MakeSoundGroup(c_name, first, second);
	LuaUserData *data = (LuaUserData *)lua_newuserdata(l, sizeof(LuaUserData));
	data->Type = LuaSoundType;
	data->Data = id;
	return 1;
}

/**
**  Glue between c and scheme. Ask to the sound system to remap a sound id
**  to a given name.
**
**  @param l  Lua state.
**
**  @return   the sound object
*/
static int CclMapSound(lua_State *l)
{
	LuaCheckArgs(l, 2);
	std::string sound_name = std::string{LuaToString(l, 1)};
	MapSound(sound_name, CclGetSound(l));
	lua_pushvalue(l, 2);
	return 1;
}

/**
** <b>Description</b>
**
**  Ask the sound system to play the specified sound.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code><strong>PlaySound</strong>("rescue (orc)")</code></div>
*/
static int CclPlaySound(lua_State *l)
{
	const int args = lua_gettop(l);
	if (args < 1 || args > 2) {
		LuaError(l, "incorrect argument");
	}

	lua_pushvalue(l, 1);
	CSound *id = CclGetSound(l);
	lua_pop(l, 1);
	const bool always = args == 2 && LuaToBoolean(l, 2);
	PlayGameSound(id, MaxSampleVolume, always);
	return 0;
}

static void SetSoundConfigRace(lua_State *l, int j, SoundConfig soundConfigs[])
{
	if (!lua_istable(l, j + 1) || lua_rawlen(l, j + 1) != 2) {
		LuaError(l, "incorrect argument");
	}
	const std::string_view raceName = LuaToString(l, j + 1, 1);
	const int raceIndex = PlayerRaces.GetRaceIndexByName(raceName);
	if (raceIndex == -1) {
		LuaError(l, "Unknown race: %s", raceName.data());
	}
	lua_rawgeti(l, j + 1, 2);
	LuaUserData *data = nullptr;
	if (!lua_isuserdata(l, -1)
		|| (data = (LuaUserData *)lua_touserdata(l, -1))->Type != LuaSoundType) {
		LuaError(l, "Sound id expected");
	}
	lua_pop(l, 1);
	soundConfigs[raceIndex].Sound = (CSound *)data->Data;
}

/**
**  Glue between c and scheme. Allows to specify some global game sounds
**  in a ccl file.
**
**  @param l  Lua state.
*/
static int CclDefineGameSounds(lua_State *l)
{
	//FIXME: should allow to define ALL the game sounds

	const int args = lua_gettop(l);
	for (int j = 0; j < args; ++j) {
		const std::string_view value = LuaToString(l, j + 1);
		++j;

		LuaUserData *data = nullptr;

		// let's handle now the different cases
		if (value == "click") {
			if (!lua_isuserdata(l, j + 1)
				|| (data = (LuaUserData *)lua_touserdata(l, j + 1))->Type != LuaSoundType) {
				LuaError(l, "Sound id expected");
			}
			GameSounds.Click.Sound = (CSound *)data->Data;
		} else if (value == "transport-docking") {
			if (!lua_isuserdata(l, j + 1)
				|| (data = (LuaUserData *)lua_touserdata(l, j + 1))->Type != LuaSoundType) {
				LuaError(l, "Sound id expected");
			}
			GameSounds.Docking.Sound = (CSound *)data->Data;
		} else if (value == "placement-error") {
			SetSoundConfigRace(l, j, GameSounds.PlacementError);
		} else if (value == "placement-success") {
			SetSoundConfigRace(l, j, GameSounds.PlacementSuccess);
		} else if (value == "work-complete") {
			SetSoundConfigRace(l, j, GameSounds.WorkComplete);
		} else if (value == "research-complete") {
			SetSoundConfigRace(l, j, GameSounds.ResearchComplete);
		} else if (value == "not-enough-res") {
			if (!lua_istable(l, j + 1) || lua_rawlen(l, j + 1) != 3) {
				LuaError(l, "incorrect argument");
			}
			const std::string_view resName = LuaToString(l, j + 1, 1);
			const int resId = GetResourceIdByName(l, resName);
			const std::string_view raceName = LuaToString(l, j + 1, 2);
			const int raceIndex = PlayerRaces.GetRaceIndexByName(raceName);
			if (raceIndex == -1) {
				LuaError(l, "Unknown race: %s", raceName.data());
			}
			lua_rawgeti(l, j + 1, 3);
			if (!lua_isuserdata(l, -1)
				|| (data = (LuaUserData *)lua_touserdata(l, -1))->Type != LuaSoundType) {
				LuaError(l, "Sound id expected");
			}
			lua_pop(l, 1);
			GameSounds.NotEnoughRes[raceIndex][resId].Sound = (CSound *)data->Data;
		} else if (value == "not-enough-food") {
			SetSoundConfigRace(l, j, GameSounds.NotEnoughFood);
		} else if (value == "rescue") {
			SetSoundConfigRace(l, j, GameSounds.Rescue);
		} else if (value == "building-construction") {
			SetSoundConfigRace(l, j, GameSounds.BuildingConstruction);
		} else if (value == "chat-message") {
			if (!lua_isuserdata(l, j + 1)
				|| (data = (LuaUserData *)lua_touserdata(l, j + 1))->Type != LuaSoundType) {
				LuaError(l, "Sound id expected");
			}
			GameSounds.ChatMessage.Sound = (CSound *)data->Data;
		} else {
			LuaError(l, "Unsupported tag: %s", value.data());
		}
	}
	return 0;
}

/**
** <b>Description</b>
**
**  Set the cut off distance.
**
**  @param l  Lua state.
**
** Example:
**
** <div class="example"><code><strong>SetGlobalSoundRange</strong>(200)</code></div>
*/
static int CclSetGlobalSoundRange(lua_State *l)
{
	LuaCheckArgs(l, 1);

	// FIXME: check for errors
	int d = LuaToNumber(l, 1);
	if (d > 0) {
		DistanceSilent = d;
	}
	return 0;
}

/**
**  Set the range of a given sound.
**
**  @param l  Lua state.
*/
static int CclSetSoundRange(lua_State *l)
{
	LuaCheckArgs(l, 2);

	int range = LuaToNumber(l, 2);
	clamp(&range, 0, 255);
	const unsigned char theRange = static_cast<unsigned char>(range);

	lua_pushvalue(l, 1);
	if (CSound* id = CclGetSound(l)) {
		id->Range = theRange;
	}
	return 0;
}

/**
**  Register CCL features for sound.
*/
void SoundCclRegister()
{
	lua_register(Lua, "SetGlobalSoundRange", CclSetGlobalSoundRange);
	lua_register(Lua, "DefineGameSounds", CclDefineGameSounds);
	lua_register(Lua, "MapSound", CclMapSound);
	lua_register(Lua, "SoundForName", CclSoundForName);
	lua_register(Lua, "SetSoundRange", CclSetSoundRange);
	lua_register(Lua, "MakeSound", CclMakeSound);
	lua_register(Lua, "MakeSoundGroup", CclMakeSoundGroup);
	lua_register(Lua, "PlaySound", CclPlaySound);
}

//@}
