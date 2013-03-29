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
//
//      (c) Copyright 2012 by Joris Dauphin
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

#ifndef GAME_H
#define GAME_H

#include <string>

class CFile;

extern void LoadGame(const std::string &filename); /// Load saved game
extern int SaveGame(const std::string &filename); /// Save game
extern void DeleteSaveGame(const std::string &filename); /// Delete save game
extern bool SaveGameLoading;                 /// Save game is in progress of loading

extern void InitModules();              /// Initialize all modules
extern void LuaRegisterModules();       /// Register lua script of each modules
extern void LoadModules();              /// Load all modules
extern void CleanModules();             /// Cleanup all modules

extern void FreeAllContainers();

extern void SaveGameSettings(CFile &file);             /// Save game settings

extern std::string GameName;                /// Name of the game
extern std::string FullGameName;            /// Full Name of the game

extern bool UseHPForXp;                     /// true if gain XP by dealing damage, false if by killing.

#endif // GAME_H

