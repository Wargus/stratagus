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

#include "stratagus.h"

#ifdef WITH_SOUND		// {

#include <stdlib.h>

#include "ccl.h"
#include "sound_id.h"
#include "sound.h"
#include "sound_server.h"
#include "cdaudio.h"

/*----------------------------------------------------------------------------
--		Variables
----------------------------------------------------------------------------*/

/**
** C representation for the siod sound type
** ALPHA VERSION!!!!!!!!!
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local ccl_smob_type_t SiodSoundTag;

#define CCL_SOUNDP(x)		(CclGetSmobType(x) == SiodSoundTag)
#define CCL_SOUND_ID(x) ((SoundId)CclGetSmobData(x))
#elif defined(USE_LUA)
#endif

/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

/**
**		Cast a Stratagus sound id to its scheme version
**
**		@param id		the sound id
**
**		@return				its siod version
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM sound_id_ccl(SoundId id)
{
	SCM sound_id;

	sound_id = CclMakeSmobObj(SiodSoundTag, id);
	return sound_id;
}
#endif

/**
**		Glue between c and scheme. Ask the sound system to associate a
**		sound id to a sound name.
**
**		@param name		name
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSoundForName(SCM name)
{
	SoundId id;
	char* sound_name;

	sound_name = gh_scm2newstr(name, NULL);
	id = SoundIdForName(sound_name);
	free(sound_name);

	return sound_id_ccl(id);
}
#elif defined(USE_LUA)
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
#endif


/**
**		Get a Game Sound Id from either a siod sound id or a sound name
**
**		@param sound		Lisp cell, SoundID or string or symbol.
**		@return				The C sound id.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SoundId CclGetSoundId(SCM sound)
{
	if (CCL_SOUNDP(sound)) {		// if we've got the sound id
		return CCL_SOUND_ID(sound);
	} else {
		return CCL_SOUND_ID(CclSoundForName(sound));
	}
}
#elif defined(USE_LUA)
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
	lua_pushfstring(l, "CclGetSoundId: not a sound");
	lua_error(l);
	return NULL;
}
#endif

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
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclMakeSound(SCM name, SCM file)
{
	SoundId id;
	char* c_name;
	char* c_file;
	char** c_files;
	int nb;
	int i;
	SCM a_file;

	if (!gh_string_p(name)) {
		fprintf(stderr, "string expected\n");
		return SCM_UNSPECIFIED;
	}
	if (gh_string_p(file)) {
		// only one file
		c_name = gh_scm2newstr(name, NULL);
		c_file = gh_scm2newstr(file, NULL);
		id = MakeSound(c_name, (const char**)&c_file, 1);
		DebugLevel3("Making sound `%s' from `%s' with id %p\n" _C_ c_name _C_
			c_file _C_ id);
		// the sound name (c_name) must be kept but the file name can be freed
		// JOHNS: wrong!
		free(c_file);
		free(c_name);
	} else if (gh_list_p(file)) {
		// several files
		c_name = gh_scm2newstr(name, NULL);
		DebugLevel3("Making sound `%s'\n" _C_ c_name);
		nb = gh_length(file);
		c_files = (char**)malloc(sizeof(char*) * nb);
		for (i = 0; i < nb; ++i) {
			a_file = gh_car(file);
			if (!gh_string_p(name)) {
				fprintf(stderr, "string expected\n");
				// FIXME: memory leak!
				return SCM_UNSPECIFIED;
			}
			c_files[i] = gh_scm2newstr(a_file, NULL);
			DebugLevel3("\tComponent %d: `%s'\n" _C_ i _C_ c_files[i]);
			file = gh_cdr(file);
		}
		//FIXME: check size before casting
		id = MakeSound(c_name, (const char**)c_files, (unsigned char)nb);
		for (i = 0; i < nb; ++i) {
			free(c_files[i]);
		}
		free(c_name);
		free(c_files);
	} else {
		fprintf(stderr, "string or list expected\n");
		return SCM_UNSPECIFIED;
	}
	return sound_id_ccl(id);
}
#elif defined(USE_LUA)
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
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
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
		lua_pushfstring(l, "string or table expected");
		lua_error(l);
		return 0;
	}
	data = lua_newuserdata(l, sizeof(LuaUserData));
	data->Type = LuaSoundType;
	data->Data = id;
	return 1;
}
#endif

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
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclMakeSoundGroup(SCM name, SCM first, SCM second)
{
	SoundId id;
	char* c_name;

	if (!gh_string_p(name) && !gh_symbol_p(name)) {
		fprintf(stderr, "string or symbol expected\n");
		return SCM_UNSPECIFIED;
	}
	c_name = gh_scm2newstr(name, NULL);

	id = MakeSoundGroup(c_name, CclGetSoundId(first), CclGetSoundId(second));
	// JOHNS: not anymore: c_name consumed by MakeSoundGroup!
	free(c_name);
	return sound_id_ccl(id);
}
#elif defined(USE_LUA)
local int CclMakeSoundGroup(lua_State* l)
{
	SoundId id;
	const char* c_name;
	SoundId first;
	SoundId second;
	LuaUserData* data;

	if (lua_gettop(l) != 3) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
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
#endif

/**
**		Glue between c and scheme. Ask to the sound system to remap a sound id
**		to a given name.
**
**		@param name		the new name for the sound
**		@param sound		the sound object
**
**		@return				the sound object
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclMapSound(SCM name, SCM sound)
{
	char* sound_name;

	sound_name = gh_scm2newstr(name, NULL);
	MapSound(sound_name, CclGetSoundId(sound));
	free(sound_name);
	return sound;
}
#elif defined(USE_LUA)
local int CclMapSound(lua_State* l)
{
	const char* sound_name;

	if (lua_gettop(l) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	sound_name = LuaToString(l, 1);
	MapSound(sound_name, CclGetSoundId(l));
	lua_pushvalue(l, 2);
	return 1;
}
#endif

/**
**		Ask the sound system to play the specified sound.
**
**		@param sound		either the sound name or the sound id
**
**		@return				SCM_UNSPECIFIED
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclPlaySound(SCM sound)
{
	SoundId id;

	id = CclGetSoundId(sound);
	PlayGameSound(id, MaxSampleVolume);
	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclPlaySound(lua_State* l)
{
	SoundId id;

	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	id = CclGetSoundId(l);
	PlayGameSound(id, MaxSampleVolume);
	return 0;
}
#endif

/**
**		Test whether a scheme object is a clone sound id
**
**		@param sound		the scheme object
**
**		@return				true is sound is a clone sound id
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
global int ccl_sound_p(SCM sound)
{
	return CCL_SOUNDP(sound);
}
#endif

/**
**		Cast a scheme object to a clone sound id
**
**		@param sound		the scheme object
**
**		@return				the clone sound id
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
global SoundId ccl_sound_id(SCM sound)
{
	return CCL_SOUND_ID(sound);
}
#endif

/**
**		Glue between c and scheme. Ask the sound system to dump on the
**		standard output the mapping between sound names and sound id.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDisplaySounds(void)
{
	DisplaySoundHashTable();
	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDisplaySounds(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	DisplaySoundHashTable();
	return 0;
}
#endif

/**
**		Glue between c and scheme. Allows to specify some global game sounds
**		in a ccl file.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineGameSounds(SCM list)
{
	//FIXME: should allow to define ALL the game sounds
	SCM value;
	SCM data;
	SCM sublist;
	char* str;
	int i;

	while (!gh_null_p(list)) {

		value = gh_car(list);
		list = gh_cdr(list);
		if (!gh_symbol_p(value)) {
			PrintFunction();
			fprintf(stdout, "Symbol expected\n");
			return list;
		}
		// prepare for next iteration

		// let's handle now the different cases
		if (gh_eq_p(value, gh_symbol2scm("click"))) {
			data = gh_car(list);
			list = gh_cdr(list);
			if (!CCL_SOUNDP(data)) {
				fprintf(stderr, "Sound id expected\n");
				return list;
			}
			GameSounds.Click.Sound = CCL_SOUND_ID(data);
		} else if (gh_eq_p(value, gh_symbol2scm("placement-error"))) {
			data = gh_car(list);
			list = gh_cdr(list);
			if (!CCL_SOUNDP(data)) {
				fprintf(stderr, "Sound id expected\n");
				return list;
			}
			GameSounds.PlacementError.Sound = CCL_SOUND_ID(data);
		} else if (gh_eq_p(value, gh_symbol2scm("placement-success"))) {
			data = gh_car(list);
			list = gh_cdr(list);
			if (!CCL_SOUNDP(data)) {
				fprintf(stderr, "Sound id expected\n");
				return list;
			}
			GameSounds.PlacementSuccess.Sound = CCL_SOUND_ID(data);
		} else if (gh_eq_p(value,gh_symbol2scm("work-complete"))) {
			sublist = gh_car(list);
			list = gh_cdr(list);
			str = gh_scm2newstr(gh_car(sublist), NULL);
			for (i = 0; i < PlayerRaces.Count; ++i) {
				if (!strcmp(PlayerRaces.Name[i], str)) {
					break;
				}
			}
			if (i == PlayerRaces.Count) {
				fprintf(stderr, "Unknown race: %s\n", str);
				ExitFatal(1);
			}
			free(str);
			sublist = gh_cdr(sublist);
			data = gh_car(sublist);
			if (!CCL_SOUNDP(data)) {
				fprintf(stderr, "Sound id expected\n");
				ExitFatal(1);
			}
			GameSounds.WorkComplete[i].Sound = CCL_SOUND_ID(data);
		} else if (gh_eq_p(value,gh_symbol2scm("rescue"))) {
			sublist = gh_car(list);
			list = gh_cdr(list);
			str = gh_scm2newstr(gh_car(sublist), NULL);
			for (i = 0; i < PlayerRaces.Count; ++i) {
				if (!strcmp(PlayerRaces.Name[i], str)) {
					break;
				}
			}
			if (i == PlayerRaces.Count) {
				fprintf(stderr, "Unknown race: %s\n", str);
				ExitFatal(1);
			}
			free(str);
			sublist = gh_cdr(sublist);
			data = gh_car(sublist);
			if (!CCL_SOUNDP(data)) {
				fprintf(stderr, "Sound id expected\n");
				ExitFatal(1);
			}
			GameSounds.Rescue[i].Sound = CCL_SOUND_ID(data);
		} else {
			errl("Unsupported tag", value);
			return list;
		}
	}
	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
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
				lua_pushfstring(l, "Sound id expected");
				lua_error(l);
			}
			GameSounds.Click.Sound = data->Data;
		} else if (!strcmp(value, "placement-error")) {
			if (!lua_isuserdata(l, j + 1) ||
					(data = lua_touserdata(l, j + 1))->Type != LuaSoundType) {
				lua_pushfstring(l, "Sound id expected");
				lua_error(l);
			}
			GameSounds.PlacementError.Sound = data->Data;
		} else if (!strcmp(value, "placement-success")) {
			if (!lua_isuserdata(l, j + 1) ||
					(data = lua_touserdata(l, j + 1))->Type != LuaSoundType) {
				lua_pushfstring(l, "Sound id expected");
				lua_error(l);
			}
			GameSounds.PlacementSuccess.Sound = data->Data;
		} else if (!strcmp(value, "work-complete")) {
			if (!lua_istable(l, j + 1) || luaL_getn(l, j + 1) != 2) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
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
				lua_pushfstring(l, "Unknown race: %s", value);
				lua_error(l);
			}
			lua_rawgeti(l, j + 1, 2);
			if (!lua_isuserdata(l, -1) ||
					(data = lua_touserdata(l, -1))->Type != LuaSoundType) {
				lua_pushfstring(l, "Sound id expected");
				lua_error(l);
			}
			lua_pop(l, 1);
			GameSounds.WorkComplete[i].Sound = data->Data;
		} else if (!strcmp(value, "rescue")) {
			if (!lua_istable(l, j + 1) || luaL_getn(l, j + 1) != 2) {
				lua_pushstring(l, "incorrect argument");
				lua_error(l);
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
				lua_pushfstring(l, "Unknown race: %s", value);
				lua_error(l);
			}
			lua_rawgeti(l, j + 1, 2);
			if (!lua_isuserdata(l, -1) ||
					(data = lua_touserdata(l, -1))->Type != LuaSoundType) {
				lua_pushfstring(l, "Sound id expected");
				lua_error(l);
			}
			lua_pop(l, 1);
			GameSounds.Rescue[i].Sound = data->Data;
		} else {
			lua_pushfstring(l, "Unsupported tag: %s", value);
			lua_error(l);
		}
	}
	return 0;
}
#endif

/**
**		Global volume support
**
**		@param volume		new global sound volume
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetSoundVolume(SCM volume)
{
	SetGlobalVolume(gh_scm2int(volume));
	return volume;
}
#elif defined(USE_LUA)
local int CclSetSoundVolume(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	SetGlobalVolume(LuaToNumber(l, 1));
	lua_pushvalue(l, 1);
	return 1;
}
#endif

/**
**		Music volume support
**
**		@param volume		new global music volume
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetMusicVolume(SCM volume)
{
	SetMusicVolume(gh_scm2int(volume));
	return volume;
}
#elif defined(USE_LUA)
local int CclSetMusicVolume(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	SetMusicVolume(LuaToNumber(l, 1));
	lua_pushvalue(l, 1);
	return 1;
}
#endif

/**
**		Set cd mode
**
**		@param mode		cd mode
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetCdMode(SCM mode)
{
#ifdef USE_CDAUDIO
	CDModes cdmode;

	if (gh_eq_p(mode, gh_symbol2scm("all"))) {
		cdmode = CDModeAll;
	} else if (gh_eq_p(mode, gh_symbol2scm("random"))) {
		cdmode = CDModeRandom;
	} else if (gh_eq_p(mode, gh_symbol2scm("defined"))) {
		cdmode = CDModeDefined;
	} else if (gh_eq_p(mode, gh_symbol2scm("off"))) {
		cdmode = CDModeOff;
	} else {
		cdmode = CDModeOff;
		errl("Unsupported tag", mode);
	}

	PlayCDRom(cdmode);
#endif
	return mode;
}
#elif defined(USE_LUA)
local int CclSetCdMode(lua_State* l)
{
#ifdef USE_CDAUDIO
	CDModes cdmode;
	const char* mode;

	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
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
		lua_pushfstring(l, "Unsupported tag: %s", mode);
		lua_error(l);
	}

	PlayCDRom(cdmode);
#endif
	lua_pushvalue(l, 1);
	return 1;
}
#endif

/**
**		Define play sections
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefinePlaySections(SCM list)
{
	SCM value;
	SCM sublist;
	PlaySection* p;
	int i;

	++NumPlaySections;
	PlaySections = realloc(PlaySections, NumPlaySections * sizeof(PlaySection));
	p = PlaySections + NumPlaySections - 1;
	memset(p, 0, sizeof(PlaySection));

	while (!gh_null_p(list)) {
		value = gh_car(list);
		list = gh_cdr(list);
		if (gh_eq_p(value, gh_symbol2scm("race"))) {
			value = gh_car(list);
			list = gh_cdr(list);
			p->Race = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("type"))) {
			value = gh_car(list);
			list = gh_cdr(list);
			if (gh_eq_p(value, gh_symbol2scm("game"))) {
				p->Type = PlaySectionGame;
			} else if (gh_eq_p(value, gh_symbol2scm("briefing"))) {
				p->Type = PlaySectionBriefing;
			} else if (gh_eq_p(value, gh_symbol2scm("stats-victory"))) {
				p->Type = PlaySectionStatsVictory;
			} else if (gh_eq_p(value, gh_symbol2scm("stats-defeat"))) {
				p->Type = PlaySectionStatsDefeat;
			} else if (gh_eq_p(value, gh_symbol2scm("main-menu"))) {
				p->Type = PlaySectionMainMenu;
			} else {
				errl("Unsupported tag", value);
			}
		} else if (gh_eq_p(value, gh_symbol2scm("cd"))) {
			sublist = gh_car(list);
			list = gh_cdr(list);
			while (!gh_null_p(sublist)) {
				value = gh_car(sublist);
				sublist = gh_cdr(sublist);
				if (gh_eq_p(value, gh_symbol2scm("order"))) {
					value = gh_car(sublist);
					sublist = gh_cdr(sublist);
					if (gh_eq_p(value, gh_symbol2scm("all"))) {
						p->CDOrder = PlaySectionOrderAll;
					} else if (gh_eq_p(value, gh_symbol2scm("random"))) {
						p->CDOrder = PlaySectionOrderRandom;
					} else {
						errl("Unsupported tag", value);
					}
				} else if (gh_eq_p(value, gh_symbol2scm("tracks"))) {
					SCM temp;

					value = gh_car(sublist);
					sublist = gh_cdr(sublist);
					for (i = 0; i < (signed)gh_vector_length(value); ++i) {
						temp=gh_vector_ref(value, gh_int2scm(i));
						p->CDTracks |= (1 << gh_scm2int(temp));
					}
				} else {
					errl("Unsupported tag", value);
				}
			}
		} else if (gh_eq_p(value, gh_symbol2scm("no-cd"))) {
			sublist = gh_car(list);
			list = gh_cdr(list);
			while (!gh_null_p(sublist)) {
				value = gh_car(sublist);
				sublist = gh_cdr(sublist);
				if (gh_eq_p(value, gh_symbol2scm("order"))) {
					value = gh_car(sublist);
					sublist = gh_cdr(sublist);
					if (gh_eq_p(value, gh_symbol2scm("all"))) {
						p->FileOrder = PlaySectionOrderAll;
					} else if (gh_eq_p(value, gh_symbol2scm("random"))) {
						p->FileOrder = PlaySectionOrderRandom;
					} else {
						errl("Unsupported tag", value);
					}
				} else if (gh_eq_p(value, gh_symbol2scm("files"))) {
					SCM sublist2;

					sublist2 = gh_car(sublist);
					sublist = gh_cdr(sublist);
					i = 0;
					while (!gh_null_p(sublist2)) {
						value = gh_car(sublist2);
						sublist2 = gh_cdr(sublist2);
						++i;
						p->Files = realloc(p->Files, (i + 1) * sizeof(char*));
						p->Files[i - 1] = gh_scm2newstr(value, NULL);
						p->Files[i] = NULL;
					}
				} else {
					errl("Unsupported tag", value);
				}
			}
		} else {
			errl("Unsupported tag", value);
		}
	}

	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDefinePlaySections(lua_State* l)
{
#if 0
	SCM value;
	SCM sublist;
	PlaySection* p;
	int i;

	++NumPlaySections;
	PlaySections = realloc(PlaySections, NumPlaySections * sizeof(PlaySection));
	p = PlaySections + NumPlaySections - 1;
	memset(p, 0, sizeof(PlaySection));

	while (!gh_null_p(list)) {
		value = gh_car(list);
		list = gh_cdr(list);
		if (gh_eq_p(value, gh_symbol2scm("race"))) {
			value = gh_car(list);
			list = gh_cdr(list);
			p->Race = gh_scm2newstr(value, NULL);
		} else if (gh_eq_p(value, gh_symbol2scm("type"))) {
			value = gh_car(list);
			list = gh_cdr(list);
			if (gh_eq_p(value, gh_symbol2scm("game"))) {
				p->Type = PlaySectionGame;
			} else if (gh_eq_p(value, gh_symbol2scm("briefing"))) {
				p->Type = PlaySectionBriefing;
			} else if (gh_eq_p(value, gh_symbol2scm("stats-victory"))) {
				p->Type = PlaySectionStatsVictory;
			} else if (gh_eq_p(value, gh_symbol2scm("stats-defeat"))) {
				p->Type = PlaySectionStatsDefeat;
			} else if (gh_eq_p(value, gh_symbol2scm("main-menu"))) {
				p->Type = PlaySectionMainMenu;
			} else {
				errl("Unsupported tag", value);
			}
		} else if (gh_eq_p(value, gh_symbol2scm("cd"))) {
			sublist = gh_car(list);
			list = gh_cdr(list);
			while (!gh_null_p(sublist)) {
				value = gh_car(sublist);
				sublist = gh_cdr(sublist);
				if (gh_eq_p(value, gh_symbol2scm("order"))) {
					value = gh_car(sublist);
					sublist = gh_cdr(sublist);
					if (gh_eq_p(value, gh_symbol2scm("all"))) {
						p->CDOrder = PlaySectionOrderAll;
					} else if (gh_eq_p(value, gh_symbol2scm("random"))) {
						p->CDOrder = PlaySectionOrderRandom;
					} else {
						errl("Unsupported tag", value);
					}
				} else if (gh_eq_p(value, gh_symbol2scm("tracks"))) {
					SCM temp;

					value = gh_car(sublist);
					sublist = gh_cdr(sublist);
					for (i = 0; i < (signed)gh_vector_length(value); ++i) {
						temp=gh_vector_ref(value, gh_int2scm(i));
						p->CDTracks |= (1 << gh_scm2int(temp));
					}
				} else {
					errl("Unsupported tag", value);
				}
			}
		} else if (gh_eq_p(value, gh_symbol2scm("no-cd"))) {
			sublist = gh_car(list);
			list = gh_cdr(list);
			while (!gh_null_p(sublist)) {
				value = gh_car(sublist);
				sublist = gh_cdr(sublist);
				if (gh_eq_p(value, gh_symbol2scm("order"))) {
					value = gh_car(sublist);
					sublist = gh_cdr(sublist);
					if (gh_eq_p(value, gh_symbol2scm("all"))) {
						p->FileOrder = PlaySectionOrderAll;
					} else if (gh_eq_p(value, gh_symbol2scm("random"))) {
						p->FileOrder = PlaySectionOrderRandom;
					} else {
						errl("Unsupported tag", value);
					}
				} else if (gh_eq_p(value, gh_symbol2scm("files"))) {
					SCM sublist2;

					sublist2 = gh_car(sublist);
					sublist = gh_cdr(sublist);
					i = 0;
					while (!gh_null_p(sublist2)) {
						value = gh_car(sublist2);
						sublist2 = gh_cdr(sublist2);
						++i;
						p->Files = realloc(p->Files, (i + 1) * sizeof(char*));
						p->Files[i - 1] = gh_scm2newstr(value, NULL);
						p->Files[i] = NULL;
					}
				} else {
					errl("Unsupported tag", value);
				}
			}
		} else {
			errl("Unsupported tag", value);
		}
	}
#endif
	return 0;
}
#endif

/**
**		Turn Off Sound (client side)
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSoundOff(void)
{
	SoundOff = 1;
	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclSoundOff(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	SoundOff = 1;
	return 0;
}
#endif

/**
**		Turn On Sound (client side)
**
**		@return true if and only if the sound is REALLY turned on
**				(uses SoundFildes)
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSoundOn(void)
{
	if (SoundFildes != -1) {
		return SCM_BOOL_T;
	}
	SoundOff = 0;
	return SCM_BOOL_F;
}
#elif defined(USE_LUA)
local int CclSoundOn(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	if (SoundFildes != -1) {
		lua_pushboolean(l, 1);
		return 1;
	}
	SoundOff = 0;
	lua_pushboolean(l, 0);
	return 1;
}
#endif

/**
**		Turn Off Music (client side)
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclMusicOff(void)
{
	StopMusic();
	MusicOff = 1;
	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclMusicOff(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	StopMusic();
	MusicOff = 1;
	return 0;
}
#endif

/**
**		Turn On Music (client side)
**
**		@return true if and only if the sound is REALLY turned on
**				(uses SoundFildes)
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclMusicOn(void)
{
	MusicOff = 0;
	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclMusicOn(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	MusicOff = 0;
	return 0;
}
#endif

/**
**		Set the cut off distance.
**
**		@param distance new cut off distance for sounds
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetGlobalSoundRange(SCM distance)
{
	int d;

	// FIXME: check for errors
	d = gh_scm2int(distance);
	if (d > 0) {
		DistanceSilent = d;
	}
	return distance;
}
#elif defined(USE_LUA)
local int CclSetGlobalSoundRange(lua_State* l)
{
	int d;

	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	// FIXME: check for errors
	d = LuaToNumber(l, 1);
	if (d > 0) {
		DistanceSilent = d;
	}
	lua_pushvalue(l, 1);
	return 1;
}
#endif

/**
**		Ask clone to use a sound thread
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSoundThread(void)
{
#ifdef USE_THREAD
	WithSoundThread = 1;
#endif
	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclSoundThread(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

#ifdef USE_THREAD
	WithSoundThread = 1;
#endif
	return 0;
}
#endif

/**
**		Set the range of a given sound.
**
**		@param sound		the sound id or name of the sound
**		@param range		the new range for this sound
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetSoundRange(SCM sound, SCM range) {
	//FIXME check for errors
	unsigned char theRange;
	int tmp;
	SoundId id;

	tmp = gh_scm2int(range);
	if (tmp < 0) {
		theRange = 0;
	} else if (tmp > 255) {
		theRange = 255;
	} else {
		theRange = (unsigned char)tmp;
	}
	DebugLevel3("Range: %u (%d)\n" _C_ TheRange _C_ tmp);
	id = CclGetSoundId(sound);
	SetSoundRange(id, theRange);
	return sound;
}
#elif defined(USE_LUA)
local int CclSetSoundRange(lua_State* l) {
	unsigned char theRange;
	int tmp;
//	SoundId id;

	if (lua_gettop(l) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
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
//	id = CclGetSoundId(sound);
//	SetSoundRange(id, theRange);
//	return sound;
	return 0;
}
#endif

/**
**		Play a music file.
**
**		@param name		Name of the music file to play.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclPlayMusic(SCM name)
{
	char* music_name;

	music_name = gh_scm2newstr(name, NULL);
	PlayMusic(music_name);
	free(music_name);

	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclPlayMusic(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	PlayMusic(LuaToString(l, 1));

	return 0;
}
#endif

/**
**		Play a sound file.
**
**		@param name		Name of the sound file to play.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclPlayFile(SCM name)
{
	char* filename;

	filename = gh_scm2newstr(name, NULL);
	PlayFile(filename);
	free(filename);

	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclPlayFile(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	PlayFile(LuaToString(l, 1));

	return 0;
}
#endif

/**
**		Stop playing music.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclStopMusic(void)
{
	StopMusic();

	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclStopMusic(lua_State* l)
{
	if (lua_gettop(l) != 0) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}

	StopMusic();

	return 0;
}
#endif

/**
**		Register CCL features for sound.
*/
global void SoundCclRegister(void)
{
#if defined(USE_GUILE) || defined(USE_SIOD)
	SiodSoundTag = CclMakeSmobType("Sound");

	gh_new_procedure1_0("set-sound-volume!", CclSetSoundVolume);
	gh_new_procedure1_0("set-music-volume!", CclSetMusicVolume);
	gh_new_procedure1_0("set-cd-mode!", CclSetCdMode);

	gh_new_procedureN("define-play-sections", CclDefinePlaySections);

	gh_new_procedure0_0("sound-off", CclSoundOff);
	gh_new_procedure0_0("sound-on", CclSoundOn);
	gh_new_procedure0_0("music-off", CclMusicOff);
	gh_new_procedure0_0("music-on", CclMusicOn);
	gh_new_procedure0_0("sound-thread", CclSoundThread);
	gh_new_procedure1_0("set-global-sound-range!", CclSetGlobalSoundRange);
	gh_new_procedureN("define-game-sounds", CclDefineGameSounds);
	gh_new_procedure0_0("display-sounds", CclDisplaySounds);
	gh_new_procedure2_0("map-sound", CclMapSound);
	gh_new_procedure1_0("sound-for-name", CclSoundForName);
	gh_new_procedure2_0("set-sound-range!", CclSetSoundRange);
	gh_new_procedure2_0("make-sound", CclMakeSound);
	gh_new_procedure3_0("make-sound-group", CclMakeSoundGroup);
	gh_new_procedure1_0("play-sound", CclPlaySound);

	gh_new_procedure1_0("play-music", CclPlayMusic);
	gh_new_procedure1_0("play-file", CclPlayFile);
	gh_new_procedure0_0("stop-music", CclStopMusic);
#elif defined(USE_LUA)
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
#endif
}

#else		// }{ WITH_SOUND

#include "ccl.h"

/**
**		Global volume support
**
**		@param volume		new global sound volume
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetSoundVolume(SCM volume)
{
	return volume;
}
#elif defined(USE_LUA)
local int CclSetSoundVolume(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	lua_pushvalue(l, 1);
	return 1;
}
#endif

/**
**		Music volume support
**
**		@param volume		new global music volume
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetMusicVolume(SCM volume)
{
	return volume;
}
#elif defined(USE_LUA)
local int CclSetMusicVolume(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	lua_pushvalue(l, 1);
	return 1;
}
#endif

/**
**		Set cd mode
**
**		@param mode		cd mode
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetCdMode(SCM mode)
{
	return mode;
}
#elif defined(USE_LUA)
local int CclSetCdMode(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	lua_pushvalue(l, 1);
	return 1;
}
#endif

/**
**		Turn Off Sound (client side)
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSoundOff(void)
{
	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclSoundOff(lua_State* l)
{
	return 0;
}
#endif

/**
**		Turn On Sound (client side)
**
**		@return true if and only if the sound is REALLY turned on
**				(uses SoundFildes)
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSoundOn(void)
{
	return SCM_BOOL_T;
}
#elif defined(USE_LUA)
local int CclSoundOn(lua_State* l)
{
	lua_pushboolean(l, 1);
	return 1;
}
#endif

/**
**		Turn Off Music (client side)
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclMusicOff(void)
{
	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclMusicOff(lua_State* l)
{
	return 0;
}
#endif

/**
**		Turn On Music (client side)
**
**		@return true if and only if the sound is REALLY turned on
**				(uses SoundFildes)
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclMusicOn(void)
{
	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclMusicOn(lua_State* l)
{
	return 0;
}
#endif

/**
**		Set the cut off distance.
**
**		@param distance new cut off distance for sounds
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetGlobalSoundRange(SCM distance)
{
	return distance;
}
#elif defined(USE_LUA)
local int CclSetGlobalSoundRange(lua_State* l)
{
	if (lua_gettop(l) != 1) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	lua_pushvalue(l, 1);
	return 1;
}
#endif

/**
**		Set the range of a given sound.
**
**		@param sound		the sound id or name of the sound
**		@param range		the new range for this sound
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSetSoundRange(SCM sound, SCM range __attribute__((unused)))
{
	return sound;
}
#elif defined(USE_LUA)
local int CclSetSoundRange(lua_State* l)
{
	if (lua_gettop(l) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	lua_pushvalue(l, 1);
	return 1;
}
#endif

/**
**		Ask clone to use a sound thread
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSoundThread(void)
{
	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclSoundThread(lua_State* l)
{
	return 0;
}
#endif

/**
**		Glue between c and scheme. Ask the sound system to dump on the
**		standard output the mapping between sound names and sound id.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDisplaySounds(void)
{
	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclDisplaySounds(lua_State* l)
{
	return 0;
}
#endif

/**
**		Glue between c and scheme. Ask the sound system to associate a sound
**		id to a sound name.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclSoundForName(SCM name __attribute__((unused)))
{
	return NIL;
}
#elif defined(USE_LUA)
local int CclSoundForName(lua_State* l)
{
	return 0;
}
#endif

/**
**		Glue between c and scheme. Allows to specify some global game sounds
**		in a ccl file.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclDefineGameSounds(SCM list __attribute__((unused)))
{
	return NIL;
}
#elif defined(USE_LUA)
local SCM CclDefineGameSounds(lua_State* l)
{
	return 0;
}
#endif

/**
**		Glue between c and scheme. Ask to the sound system to remap a sound id
**		to a given name.
**
**		@param name		the new name for the sound
**		@param sound		the sound object
**
**		@return				the sound object
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclMapSound(SCM name __attribute__((unused)), SCM sound)
{
	return sound;
}
#elif defined(USE_LUA)
local int CclMapSound(lua_State* l)
{
	if (lua_gettop(l) != 2) {
		lua_pushstring(l, "incorrect argument");
		lua_error(l);
	}
	lua_pushvalue(l, 2);
	return 1;
}
#endif

/**
**		Play a music file.
**
**		@param name		Name of the music file to play.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclPlayMusic(SCM name __attribute__((unused)))
{
	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclPlayMusic(lua_State* l)
{
	return 0;
}
#endif

/**
**		Play a sound file.
**
**		@param name		Name of the sound file to play.
*/
#if defined(USE_GUILE) || defined(USE_SIOD)
local SCM CclPlayFile(SCM name __attribute__((unused)))
{
	return SCM_UNSPECIFIED;
}
#elif defined(USE_LUA)
local int CclPlayFile(lua_State* l)
{
	return 0;
}
#endif

/**
**		Register CCL features for sound. Dummy version.
*/
global void SoundCclRegister(void)
{
#if defined(USE_GUILE) || defined(USE_SIOD)
	gh_new_procedure1_0("set-sound-volume!", CclSetSoundVolume);
	gh_new_procedure1_0("set-music-volume!", CclSetMusicVolume);
	gh_new_procedure1_0("set-cd-mode!", CclSetCdMode);
	gh_new_procedure0_0("sound-off", CclSoundOff);
	gh_new_procedure0_0("sound-on", CclSoundOn);
	gh_new_procedure0_0("music-off", CclMusicOff);
	gh_new_procedure0_0("music-on", CclMusicOn);
	gh_new_procedure0_0("sound-thread", CclSoundThread);
	gh_new_procedure1_0("set-global-sound-range!", CclSetGlobalSoundRange);
	gh_new_procedureN("define-game-sounds", CclDefineGameSounds);
	gh_new_procedure0_0("display-sounds", CclDisplaySounds);
	gh_new_procedure2_0("map-sound", CclMapSound);
	gh_new_procedure1_0("sound-for-name", CclSoundForName);
	gh_new_procedure2_0("set-sound-range!", CclSetSoundRange);

	gh_new_procedure1_0("play-music", CclPlayMusic);
	gh_new_procedure1_0("play-file", CclPlayFile);
#elif defined(USE_LUA)
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
#endif
}

#endif		// } !WITH_SOUND

//@}
