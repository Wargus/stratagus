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
/**@name ccl_sound.c	-	The sound ccl functions. */
//
//	(c) Copyright 1999-2003 by Lutz Sammer, Fabrice Rossi, and Jimmy Salmon
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
//	$Id$

//@{

/*----------------------------------------------------------------------------
--		Includes
----------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>

#include "stratagus.h"

#ifdef WITH_SOUND		// {

#include <stdlib.h>

#include "script.h"
#include "sound_id.h"
#include "sound.h"
#include "sound_server.h"
#include "cdaudio.h"

/*----------------------------------------------------------------------------
--		Variables
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

/**
**		Glue between c and scheme. Ask the sound system to associate a
**		sound id to a sound name.
**
**		@param name		name
*/
local int CclSoundForName(lua_State* l)
{
	SoundId id;
	const char* sound_name;
	LuaUserData* data;

	sound_name = LuaToString(l, -1);
	id = SoundIdForName(sound_name);

	data = lua_newuserdata(l, sizeof(LuaUserData));
	data->Type = LuaSoundType;
	data->Data = id;
	return 1;
}

/**
**		Get a Game Sound Id from either a siod sound id or a sound name
**
**		@param sound		Lisp cell, SoundID or string or symbol.
**		@return				The C sound id.
*/
local SoundId CclGetSoundId(lua_State* l)
{
	LuaUserData* data;

	if (lua_isstring(l, -1)) {
		CclSoundForName(l);
	}
	if (lua_isuserdata(l, -1)) {
		data = lua_touserdata(l, -1);
		if (data->Type == LuaSoundType) {
			return data->Data;
		}
	}
	LuaError(l, "CclGetSoundId: not a sound");
	return NULL;
}

/**
**		Create a sound.
**
**		Glue between c and scheme. This function asks the sound system to
**		register a sound under a given name, wiht an associated list of files
**		(the list can be replaced by only one file).
**
**		@param name		the name of the sound
**		@param file		a list of sound file names (or a file name)
**
**		@return				the sound id of the created sound
*/
local int CclMakeSound(lua_State* l)
{
	SoundId id;
	const char* c_name;
	const char* c_file;
	char** c_files;
	int args;
	int j;
	LuaUserData* data;

	if (lua_gettop(l) != 2) {
		LuaError(l, "incorrect argument");
	}

	c_name = LuaToString(l, 1);
	if (lua_isstring(l, 2)) {
		// only one file
		c_file = LuaToString(l, 2);
		id = MakeSound(c_name, &c_file, 1);
		DebugLevel3("Making sound `%s' from `%s' with id %p\n" _C_ c_name _C_
			c_file _C_ id);
	} else if (lua_istable(l, 2)) {
		// several files
		DebugLevel3("Making sound `%s'\n" _C_ c_name);
		args = luaL_getn(l, 2);
		c_files = malloc(args * sizeof(char*));
		for (j = 0; j < args; ++j) {
			lua_rawgeti(l, 2, j + 1);
			c_files[j] = strdup(LuaToString(l, -1));
			lua_pop(l, 1);
			DebugLevel3("\tComponent %d: `%s'\n" _C_ i _C_ c_files[j]);
		}
		// FIXME: check size before casting
		id = MakeSound(c_name, (const char**)c_files, (unsigned char)args);
		for (j = 0; j < args; ++j) {
			free(c_files[j]);
		}
		free(c_files);
	} else {
		LuaError(l, "string or table expected");
		return 0;
	}
	data = lua_newuserdata(l, sizeof(LuaUserData));
	data->Type = LuaSoundType;
	data->Data = id;
	return 1;
}

/**
**		Glue between c and scheme. This function asks the sound system to
**		build a special sound group.
**
**		@param name		the name of the sound
**		@param first		first group played (sound-id or string)
**		@param second		second group played (sound-id or string)
**
**		@return				The sound id of the created sound
*/
local int CclMakeSoundGroup(lua_State* l)
{
	SoundId id;
	const char* c_name;
	SoundId first;
	SoundId second;
	LuaUserData* data;

	if (lua_gettop(l) != 3) {
		LuaError(l, "incorrect argument");
	}

	c_name = LuaToString(l, 1);

	lua_pushvalue(l, 2);
	first = CclGetSoundId(l);
	lua_pop(l, 1);
	second = CclGetSoundId(l);
	id = MakeSoundGroup(c_name, first, second);
	data = lua_newuserdata(l, sizeof(LuaUserData));
	data->Type = LuaSoundType;
	data->Data = id;
	return 1;
}

/**
**		Glue between c and scheme. Ask to the sound system to remap a sound id
**		to a given name.
**
**		@param name		the new name for the sound
**		@param sound		the sound object
**
**		@return				the sound object
*/
local int CclMapSound(lua_State* l)
{
	const char* sound_name;

	if (lua_gettop(l) != 2) {
		LuaError(l, "incorrect argument");
	}
	sound_name = LuaToString(l, 1);
	MapSound(sound_name, CclGetSoundId(l));
	lua_pushvalue(l, 2);
	return 1;
}

/**
**		Ask the sound system to play the specified sound.
**
**		@param sound		either the sound name or the sound id
**
**		@return				SCM_UNSPECIFIED
*/
local int CclPlaySound(lua_State* l)
{
	SoundId id;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}

	id = CclGetSoundId(l);
	PlayGameSound(id, MaxSampleVolume);
	return 0;
}

/**
**		Glue between c and scheme. Ask the sound system to dump on the
**		standard output the mapping between sound names and sound id.
*/
local int CclDisplaySounds(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
	}

	DisplaySoundHashTable();
	return 0;
}

/**
**		Glue between c and scheme. Allows to specify some global game sounds
**		in a ccl file.
*/
local int CclDefineGameSounds(lua_State* l)
{
	//FIXME: should allow to define ALL the game sounds
	const char* value;
	int i;
	int args;
	int j;
	LuaUserData* data;

	args = lua_gettop(l);
	data = NULL;
	for (j = 0; j < args; ++j) {
		value = LuaToString(l, j + 1);
		++j;

		// let's handle now the different cases
		if (!strcmp(value, "click")) {
			if (!lua_isuserdata(l, j + 1) ||
					(data = lua_touserdata(l, j + 1))->Type != LuaSoundType) {
				LuaError(l, "Sound id expected");
			}
			GameSounds.Click.Sound = data->Data;
		} else if (!strcmp(value, "placement-error")) {
			if (!lua_isuserdata(l, j + 1) ||
					(data = lua_touserdata(l, j + 1))->Type != LuaSoundType) {
				LuaError(l, "Sound id expected");
			}
			GameSounds.PlacementError.Sound = data->Data;
		} else if (!strcmp(value, "placement-success")) {
			if (!lua_isuserdata(l, j + 1) ||
					(data = lua_touserdata(l, j + 1))->Type != LuaSoundType) {
				LuaError(l, "Sound id expected");
			}
			GameSounds.PlacementSuccess.Sound = data->Data;
		} else if (!strcmp(value, "work-complete")) {
			if (!lua_istable(l, j + 1) || luaL_getn(l, j + 1) != 2) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, j + 1, 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			for (i = 0; i < PlayerRaces.Count; ++i) {
				if (!strcmp(PlayerRaces.Name[i], value)) {
					break;
				}
			}
			if (i == PlayerRaces.Count) {
				LuaError(l, "Unknown race: %s" _C_ value);
			}
			lua_rawgeti(l, j + 1, 2);
			if (!lua_isuserdata(l, -1) ||
					(data = lua_touserdata(l, -1))->Type != LuaSoundType) {
				LuaError(l, "Sound id expected");
			}
			lua_pop(l, 1);
			GameSounds.WorkComplete[i].Sound = data->Data;
		} else if (!strcmp(value, "rescue")) {
			if (!lua_istable(l, j + 1) || luaL_getn(l, j + 1) != 2) {
				LuaError(l, "incorrect argument");
			}
			lua_rawgeti(l, j + 1, 1);
			value = LuaToString(l, -1);
			lua_pop(l, 1);
			for (i = 0; i < PlayerRaces.Count; ++i) {
				if (!strcmp(PlayerRaces.Name[i], value)) {
					break;
				}
			}
			if (i == PlayerRaces.Count) {
				LuaError(l, "Unknown race: %s" _C_ value);
			}
			lua_rawgeti(l, j + 1, 2);
			if (!lua_isuserdata(l, -1) ||
					(data = lua_touserdata(l, -1))->Type != LuaSoundType) {
				LuaError(l, "Sound id expected");
			}
			lua_pop(l, 1);
			GameSounds.Rescue[i].Sound = data->Data;
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}
	return 0;
}

/**
**		Global volume support
**
**		@param volume		new global sound volume
*/
local int CclSetSoundVolume(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}

	SetGlobalVolume(LuaToNumber(l, 1));
	lua_pushvalue(l, 1);
	return 1;
}

/**
**		Music volume support
**
**		@param volume		new global music volume
*/
local int CclSetMusicVolume(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}

	SetMusicVolume(LuaToNumber(l, 1));
	lua_pushvalue(l, 1);
	return 1;
}

/**
**		Set cd mode
**
**		@param mode		cd mode
*/
local int CclSetCdMode(lua_State* l)
{
#ifdef USE_CDAUDIO
	CDModes cdmode;
	const char* mode;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	mode = LuaToString(l, 1);

	if (!strcmp(mode, "all")) {
		cdmode = CDModeAll;
	} else if (!strcmp(mode, "random")) {
		cdmode = CDModeRandom;
	} else if (!strcmp(mode, "defined")) {
		cdmode = CDModeDefined;
	} else if (!strcmp(mode, "off")) {
		cdmode = CDModeOff;
	} else {
		cdmode = CDModeOff;
		LuaError(l, "Unsupported tag: %s" _C_ mode);
	}

	PlayCDRom(cdmode);
#endif
	lua_pushvalue(l, 1);
	return 1;
}

/**
**		Define play sections
*/
local int CclDefinePlaySections(lua_State* l)
{
	const char* value;
	PlaySection* p;
	int args;
	int j;
	int subargs;
	int k;

	++NumPlaySections;
	PlaySections = realloc(PlaySections, NumPlaySections * sizeof(PlaySection));
	p = PlaySections + NumPlaySections - 1;
	memset(p, 0, sizeof(PlaySection));

	args = lua_gettop(l);
	for (j = 0; j < args; ++j) {
		value = LuaToString(l, j + 1);
		++j;
		if (!strcmp(value, "race")) {
			p->Race = strdup(LuaToString(l, j + 1));
		} else if (!strcmp(value, "type")) {
			value = LuaToString(l, j + 1);
			if (!strcmp(value, "game")) {
				p->Type = PlaySectionGame;
			} else if (!strcmp(value, "briefing")) {
				p->Type = PlaySectionBriefing;
			} else if (!strcmp(value, "stats-victory")) {
				p->Type = PlaySectionStatsVictory;
			} else if (!strcmp(value, "stats-defeat")) {
				p->Type = PlaySectionStatsDefeat;
			} else if (!strcmp(value, "main-menu")) {
				p->Type = PlaySectionMainMenu;
			} else {
				LuaError(l, "Unsupported tag: %s" _C_ value);
			}
		} else if (!strcmp(value, "cd")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = luaL_getn(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, j + 1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;
				if (!strcmp(value, "order")) {
					lua_rawgeti(l, j + 1, k + 1);
					value = LuaToString(l, -1);
					lua_pop(l, 1);
					if (!strcmp(value, "all")) {
						p->CDOrder = PlaySectionOrderAll;
					} else if (!strcmp(value, "random")) {
						p->CDOrder = PlaySectionOrderRandom;
					} else {
						LuaError(l, "Unsupported tag: %s" _C_ value);
					}
				} else if (!strcmp(value, "tracks")) {
					int subsubargs;
					int subk;

					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}
					subsubargs = luaL_getn(l, -1);
					for (subk = 0; subk < subsubargs; ++subk) {
						lua_rawgeti(l, -1, subk + 1);
						p->CDTracks |= (1 << (int)LuaToNumber(l, -1));
						lua_pop(l, 1);
					}
					lua_pop(l, 1);
				} else {
					LuaError(l, "Unsupported tag: %s" _C_ value);
				}
			}
		} else if (!strcmp(value, "no-cd")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			subargs = luaL_getn(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, j + 1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;
				if (!strcmp(value, "order")) {
					lua_rawgeti(l, j + 1, k + 1);
					value = LuaToString(l, -1);
					lua_pop(l, 1);
					if (!strcmp(value, "all")) {
						p->FileOrder = PlaySectionOrderAll;
					} else if (!strcmp(value, "random")) {
						p->FileOrder = PlaySectionOrderRandom;
					} else {
						LuaError(l, "Unsupported tag: %s" _C_ value);
					}
				} else if (!strcmp(value, "files")) {
					int subsubargs;
					int subk;

					lua_rawgeti(l, j + 1, k + 1);
					if (!lua_istable(l, -1)) {
						LuaError(l, "incorrect argument");
					}
					subsubargs = luaL_getn(l, -1);
					p->Files = malloc((subsubargs + 1) * sizeof(char*));
					p->Files[subsubargs] = NULL;
					for (subk = 0; subk < subsubargs; ++subk) {
						lua_rawgeti(l, -1, subk + 1);
						p->Files[subk] = strdup(LuaToString(l, -1));
						lua_pop(l, 1);
					}
					lua_pop(l, 1);
				} else {
					LuaError(l, "Unsupported tag: %s" _C_ value);
				}
			}
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}
	return 0;
}

/**
**		Turn Off Sound (client side)
*/
local int CclSoundOff(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
	}

	SoundOff = 1;
	return 0;
}

/**
**		Turn On Sound (client side)
**
**		@return true if and only if the sound is REALLY turned on
**				(uses SoundFildes)
*/
local int CclSoundOn(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
	}

	if (SoundFildes != -1) {
		lua_pushboolean(l, 1);
		return 1;
	}
	SoundOff = 0;
	lua_pushboolean(l, 0);
	return 1;
}

/**
**		Turn Off Music (client side)
*/
local int CclMusicOff(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
	}

	StopMusic();
	MusicOff = 1;
	return 0;
}

/**
**		Turn On Music (client side)
**
**		@return true if and only if the sound is REALLY turned on
**				(uses SoundFildes)
*/
local int CclMusicOn(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
	}

	MusicOff = 0;
	return 0;
}

/**
**		Set the cut off distance.
**
**		@param distance new cut off distance for sounds
*/
local int CclSetGlobalSoundRange(lua_State* l)
{
	int d;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}

	// FIXME: check for errors
	d = LuaToNumber(l, 1);
	if (d > 0) {
		DistanceSilent = d;
	}
	lua_pushvalue(l, 1);
	return 1;
}

/**
**		Ask clone to use a sound thread
*/
local int CclSoundThread(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
	}

#ifdef USE_THREAD
	WithSoundThread = 1;
#endif
	return 0;
}

/**
**		Set the range of a given sound.
**
**		@param sound		the sound id or name of the sound
**		@param range		the new range for this sound
*/
local int CclSetSoundRange(lua_State* l) {
	unsigned char theRange;
	int tmp;
	SoundId id;

	if (lua_gettop(l) != 2) {
		LuaError(l, "incorrect argument");
	}

	tmp = LuaToNumber(l, 2);
	if (tmp < 0) {
		theRange = 0;
	} else if (tmp > 255) {
		theRange = 255;
	} else {
		theRange = (unsigned char)tmp;
	}
	DebugLevel3("Range: %u (%d)\n" _C_ TheRange _C_ tmp);
	lua_pushvalue(l, 1);
	id = CclGetSoundId(l);
	SetSoundRange(id, theRange);
	return 1;
}

/**
**		Play a music file.
**
**		@param name		Name of the music file to play.
*/
local int CclPlayMusic(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}

	PlayMusic(LuaToString(l, 1));

	return 0;
}

/**
**		Play a sound file.
**
**		@param name		Name of the sound file to play.
*/
local int CclPlayFile(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}

	PlaySoundFile(LuaToString(l, 1));

	return 0;
}

/**
**		Stop playing music.
*/
local int CclStopMusic(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		LuaError(l, "incorrect argument");
	}

	StopMusic();

	return 0;
}

/**
**		Register CCL features for sound.
*/
global void SoundCclRegister(void)
{
	lua_register(Lua, "SetSoundVolume", CclSetSoundVolume);
	lua_register(Lua, "SetMusicVolume", CclSetMusicVolume);
	lua_register(Lua, "SetCdMode", CclSetCdMode);

	lua_register(Lua, "DefinePlaySections", CclDefinePlaySections);

	lua_register(Lua, "SoundOff", CclSoundOff);
	lua_register(Lua, "SoundOn", CclSoundOn);
	lua_register(Lua, "MusicOff", CclMusicOff);
	lua_register(Lua, "MusicOn", CclMusicOn);
	lua_register(Lua, "SoundThread", CclSoundThread);
	lua_register(Lua, "SetGlobalSoundRange", CclSetGlobalSoundRange);
	lua_register(Lua, "DefineGameSounds", CclDefineGameSounds);
	lua_register(Lua, "DisplaySounds", CclDisplaySounds);
	lua_register(Lua, "MapSound", CclMapSound);
	lua_register(Lua, "SoundForName", CclSoundForName);
	lua_register(Lua, "SetSoundRange", CclSetSoundRange);
	lua_register(Lua, "MakeSound", CclMakeSound);
	lua_register(Lua, "MakeSoundGroup", CclMakeSoundGroup);
	lua_register(Lua, "PlaySound", CclPlaySound);

	lua_register(Lua, "PlayMusic", CclPlayMusic);
	lua_register(Lua, "PlayFile", CclPlayFile);
	lua_register(Lua, "StopMusic", CclStopMusic);
}

#else		// }{ WITH_SOUND

#include "script.h"

/**
**		Global volume support
**
**		@param volume		new global sound volume
*/
local int CclSetSoundVolume(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	lua_pushvalue(l, 1);
	return 1;
}

/**
**		Music volume support
**
**		@param volume		new global music volume
*/
local int CclSetMusicVolume(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	lua_pushvalue(l, 1);
	return 1;
}

/**
**		Set cd mode
**
**		@param mode		cd mode
*/
local int CclSetCdMode(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	lua_pushvalue(l, 1);
	return 1;
}

/**
**		Turn Off Sound (client side)
*/
local int CclSoundOff(lua_State* l)
{
	return 0;
}

/**
**		Turn On Sound (client side)
**
**		@return true if and only if the sound is REALLY turned on
**				(uses SoundFildes)
*/
local int CclSoundOn(lua_State* l)
{
	lua_pushboolean(l, 1);
	return 1;
}

/**
**		Turn Off Music (client side)
*/
local int CclMusicOff(lua_State* l)
{
	return 0;
}

/**
**		Turn On Music (client side)
**
**		@return true if and only if the sound is REALLY turned on
**				(uses SoundFildes)
*/
local int CclMusicOn(lua_State* l)
{
	return 0;
}

/**
**		Set the cut off distance.
**
**		@param distance new cut off distance for sounds
*/
local int CclSetGlobalSoundRange(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}
	lua_pushvalue(l, 1);
	return 1;
}

/**
**		Set the range of a given sound.
**
**		@param sound		the sound id or name of the sound
**		@param range		the new range for this sound
*/
local int CclSetSoundRange(lua_State* l)
{
	if (lua_gettop(l) != 2) {
		LuaError(l, "incorrect argument");
	}
	lua_pushvalue(l, 1);
	return 1;
}

/**
**		Ask clone to use a sound thread
*/
local int CclSoundThread(lua_State* l)
{
	return 0;
}

/**
**		Glue between c and scheme. Ask the sound system to dump on the
**		standard output the mapping between sound names and sound id.
*/
local int CclDisplaySounds(lua_State* l)
{
	return 0;
}

/**
**		Glue between c and scheme. Ask the sound system to associate a sound
**		id to a sound name.
*/
local int CclSoundForName(lua_State* l)
{
	return 0;
}

/**
**		Glue between c and scheme. Allows to specify some global game sounds
**		in a ccl file.
*/
local int CclDefineGameSounds(lua_State* l)
{
	return 0;
}

/**
**		Glue between c and scheme. Ask to the sound system to remap a sound id
**		to a given name.
**
**		@param name		the new name for the sound
**		@param sound		the sound object
**
**		@return				the sound object
*/
local int CclMapSound(lua_State* l)
{
	if (lua_gettop(l) != 2) {
		LuaError(l, "incorrect argument");
	}
	lua_pushvalue(l, 2);
	return 1;
}

/**
**		Play a music file.
**
**		@param name		Name of the music file to play.
*/
local int CclPlayMusic(lua_State* l)
{
	return 0;
}

/**
**		Play a sound file.
**
**		@param name		Name of the sound file to play.
*/
local int CclPlayFile(lua_State* l)
{
	return 0;
}

/**
**		Register CCL features for sound. Dummy version.
*/
global void SoundCclRegister(void)
{
	lua_register(Lua, "SetSoundVolume!", CclSetSoundVolume);
	lua_register(Lua, "SetMusicVolume!", CclSetMusicVolume);
	lua_register(Lua, "SetCdMode!", CclSetCdMode);
	lua_register(Lua, "SoundOff", CclSoundOff);
	lua_register(Lua, "SoundOn", CclSoundOn);
	lua_register(Lua, "MusicOff", CclMusicOff);
	lua_register(Lua, "MusicOn", CclMusicOn);
	lua_register(Lua, "SoundThread", CclSoundThread);
	lua_register(Lua, "SetGlobalSoundRange!", CclSetGlobalSoundRange);
	lua_register(Lua, "DefineGameSounds", CclDefineGameSounds);
	lua_register(Lua, "DisplaySounds", CclDisplaySounds);
	lua_register(Lua, "MapSound", CclMapSound);
	lua_register(Lua, "SoundForName", CclSoundForName);
	lua_register(Lua, "SetSoundRange!", CclSetSoundRange);

	lua_register(Lua, "PlayMusic", CclPlayMusic);
	lua_register(Lua, "PlayFile", CclPlayFile);
}

#endif		// } !WITH_SOUND

//@}
