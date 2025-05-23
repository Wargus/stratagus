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
/**@name parameters.h - General parameters. */
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

#ifndef PARAMETERS_H
#define PARAMETERS_H

#include "filesystem.h"

#include <string>

class Parameters
{
public:
	void SetDefaultValues();
	void SetLocalPlayerNameFromEnv();

	void SetUserDirectory(const fs::path& path) {
		if (path.empty()) {
			SetDefaultUserDirectory(true);
		} else {
			userDirectory = path;
		}
	}
	const fs::path& GetUserDirectory() const { return userDirectory; }

private:
	void SetDefaultUserDirectory(bool noPortable = false);

public:
	std::string applicationName;
	fs::path luaStartFilename;
	fs::path luaEditorStartFilename;
	std::string luaScriptArguments;
	std::string LocalPlayerName;        /// Name of local player
	bool benchmark = false;             /// If true, run as fast as possible and report fps at the end of a game
private:
	fs::path userDirectory;          /// Directory containing user settings and data
public:
	static Parameters Instance;
};

#endif /* PARAMETERS_H */
