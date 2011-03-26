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
/**@name translate.h - The translate headerfile. */
//
//      (c) Copyright 2005 by Jimmy Salmon
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

#ifndef __TRANSLATE_H__
#define __TRANSLATE_H__

#include <string>

//@{

/// The PO containing the translations for the stratagus engine
extern std::string StratagusTranslation;

/// The PO containing the translations for the game itself
extern std::string GameTranslation;
/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

	/// Translate a string
extern const char *Translate(const char *str);
	/// Add a translation
extern void AddTranslation(const char *str1, const char *str2);
	/// Load a .po file
extern void LoadPO(const char *file);
	/// Set the stratagus and game translations
extern void SetTranslationsFiles(const char *stratagusfile, const char *gamefile);


//@}

#endif // !__TRANSLATE_H__

