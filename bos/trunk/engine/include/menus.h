//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name menus.h - The menu headerfile. */
//
//      (c) Copyright 1999-2007 by Andreas Arens and Jimmy Salmon
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

#ifndef __MENUS_H__
#define __MENUS_H__

//@{

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
extern void DrawMenuButton(ButtonStyle *style, unsigned flags,
	int x, int y, const std::string &text);


	/// Pre menu setup
extern void PreMenuSetup(void);

//@}

#endif // !__MENUS_H__
