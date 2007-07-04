//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name translate.h - The translate headerfile. */
//
//      (c) Copyright 2005-2007 by Jimmy Salmon
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
extern void AddTranslation(const std::string &str1, const std::string &str2);
	/// Load a .po file
extern void LoadPO(const std::string &file);
	/// Set the stratagus and game translations
extern void SetTranslationsFiles(const std::string &stratagusfile, const std::string &gamefile);


//@}

#endif // !__TRANSLATE_H__

