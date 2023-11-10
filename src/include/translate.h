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

//@{

#include "filesystem.h"

#include <string>

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/// Translate a string
extern const std::string &Translate(const std::string &str);

/**
** Translate a string handling plural (according to PO rules)
** str might have %n which would be replaced by count.
*/
extern const std::string &Plural(const std::string &str, std::size_t count);

/// Load a .po file
extern void LoadPO(const fs::path &file);
/// Set the stratagus and game translations
extern void SetTranslationsFiles(const fs::path &stratagusfile, const fs::path &gamefile);

#define _(str) Translate(str).c_str()
#define N_(str) str

//@}

#endif // !__TRANSLATE_H__

