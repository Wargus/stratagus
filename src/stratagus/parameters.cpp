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

/* static */ Parameters Parameters::Instance;


void Parameters::SetDefaultValues()
{
	applicationName = "stratagus";
	luaStartFilename = "scripts/stratagus.lua";
	luaEditorStartFilename = "scripts/editor.lua";
	SetUserDirectory();
}

void Parameters::SetUserDirectory()
{
#ifdef USE_WIN32
	UserDirectory = getenv("APPDATA");
#else
	UserDirectory = getenv("HOME");
#endif

	if (!UserDirectory.empty()) {
		UserDirectory += "/";
	}

#ifdef USE_WIN32
	UserDirectory += "Stratagus";
#elif defined(USE_MAC)
	UserDirectory += "Library/Stratagus";
#else
	UserDirectory += ".stratagus";
#endif
}

static std::string GetLocalPlayerNameFromEnv()
{
	const char *userName = NULL;

#ifdef USE_WIN32
	userName = getenv("USERNAME");
#elif !defined(USE_MAEMO)
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
