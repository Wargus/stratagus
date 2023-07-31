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
/**@name action_train.cpp - The building train action. */
//
//      (c) Copyright 1998-2013 by Joris Dauphin
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

#include "stratagus.h"

#include "parameters.h"

#include <stdlib.h>

#ifdef USE_WIN32
#include <Shlobj.h>
#include <filesystem>
#endif

/* static */ Parameters Parameters::Instance;


void Parameters::SetDefaultValues()
{
	applicationName = "stratagus";
	luaStartFilename = "scripts/stratagus.lua";
	luaEditorStartFilename = "scripts/editor.lua";
	SetDefaultUserDirectory();
}

void Parameters::SetDefaultUserDirectory(bool noPortable)
{
#ifdef USE_GAME_DIR
	userDirectory = StratagusLibPath;
#elif USE_WIN32
	if (!noPortable) {
		// if launcher is in the same directory as the data, we are in a portable install
		std::string executable_path = GetExecutablePath();
		if (std::filesystem::equivalent(std::filesystem::path(StratagusLibPath), std::filesystem::path(executable_path).parent_path())) {
			userDirectory = StratagusLibPath;
			return;
		}
	}
	char data_path[4096] = {'\0'};
	SHGetFolderPathA(nullptr, CSIDL_PERSONAL|CSIDL_FLAG_CREATE, nullptr, 0, data_path);
	if (data_path[0]) {
		userDirectory = std::string(data_path) + "/Stratagus";
	} else if (getenv("APPDATA")) {
		userDirectory = std::string(getenv("APPDATA")) + "/Stratagus";
	}
#else // USE_GAME_DIR
	char *configDir = getenv("XDG_CONFIG_HOME");
	if (configDir) {
		userDirectory = std::string(configDir) + "/stratagus";
	} else {
		configDir = getenv("HOME");
		if (configDir) {
#ifdef USE_MAC
			userDirectory = std::string(configDir) + "/Library/Application\ Support/Stratagus";
#else
			userDirectory = std::string(configDir) + "/.config/stratagus";
#endif // USE_MAC
		}
	}
#endif // USE_GAME_DIR
}

static std::string GetLocalPlayerNameFromEnv()
{
	const char *userName = nullptr;

#ifdef USE_WIN32
	userName = getenv("USERNAME");
#else
	userName = getenv("USER");
#endif

	if (userName && userName[0]) {
		return userName;
	} else {
		return "Anonymous";
	}
}

void Parameters::SetLocalPlayerNameFromEnv()
{
	LocalPlayerName = GetLocalPlayerNameFromEnv();
}


//@}
