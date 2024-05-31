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
/**@name menus.h - The menu headerfile. */
//
//      (c) Copyright 1999-2006 by Andreas Arens and Jimmy Salmon
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

#ifndef __MENUS_H__
#define __MENUS_H__

//@{

#include <string_view>

/*----------------------------------------------------------------------------
--  Defines/Declarations
----------------------------------------------------------------------------*/

#define MI_FLAGS_ACTIVE     1  /// cursor on item
#define MI_FLAGS_CLICKED    2  /// mouse button pressed down on item

class ButtonStyle;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/// Draw menu button
extern void DrawUIButton(ButtonStyle *style, unsigned flags,
						 int x, int y, std::string_view text, int player = -1);

/// Pre menu setup
extern void PreMenuSetup();

//@}

#endif // !__MENUS_H__
