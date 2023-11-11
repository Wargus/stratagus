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
/**@name translate.cpp - Translate languages. */
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "translate.h"

#include "iolib.h"

#include <fstream>
#include <spiritless_po.h>

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

static spiritless_po::Catalog catalog;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Translate a string
*/
const std::string &Translate(const std::string &str)
{
	return catalog.gettext(str);
}

/**
** Translate a string handling plural (according to PO rules)
** str might have %n which would be replaced by count.
*/
const std::string& Plural(const std::string& str, std::size_t count)
{
	return catalog.ngettext(str, str, count);
}

/**
**  Load a .po file
*/
void LoadPO(const fs::path& filename)
{
	const fs::path fullfilename = LibraryFileName(filename.string());
	DebugPrint(
		"LoadPO(\"%s\") -> \"%s\"\n", filename.u8string().c_str(), fullfilename.u8string().c_str());
	
	std::ifstream poFile(fullfilename);
	catalog.Add(poFile);
}

/**
**  Set the stratagus and game translations
**/
void SetTranslationsFiles(const fs::path &stratagusfile, const fs::path &gamefile)
{
	DebugPrint("SetTranslationsFiles(\"%s\", \"%s\")\n",
	           stratagusfile.u8string().c_str(),
	           gamefile.u8string().c_str());
	catalog.Clear();

	LoadPO(stratagusfile);
	LoadPO(gamefile);
}

//@}
