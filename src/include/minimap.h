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
/**@name minimap.h - The minimap headerfile. */
//
//      (c) Copyright 1998,2000-2003 by Lutz Sammer and Jimmy Salmon
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
//      $Id$

#ifndef __MINIMAP_H__
#define __MINIMAP_H__

//@{

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

#define MINIMAP_FAC (16 * 3)  ///< integer scale factor

	/// unit attacked are shown red for at least this amount of cycles
#define ATTACK_RED_DURATION (1 * CYCLES_PER_SECOND)
	/// unit attacked are shown blinking for this amount of cycles
#define ATTACK_BLINK_DURATION (7 * CYCLES_PER_SECOND)

	/// Update seen tile change in minimap
#define UpdateMinimapSeenXY(tx, ty)

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern int MinimapX;  ///< Minimap drawing position x offset
extern int MinimapY;  ///< Minimap drawing position y offset

extern int MinimapWithTerrain;   ///< display minimap with terrain
extern int MinimapFriendly;      ///< switch colors of friendly units
extern int MinimapShowSelected;  ///< highlight selected units

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

	/// Update tile change in minimap
extern void UpdateMinimapXY(int tx, int ty);
	/// Update minimap terrain
extern void UpdateMinimapTerrain(void);
	/// Update complete minimap
extern void UpdateMinimap(void);
	/// Create new minimap
extern void CreateMinimap(void);
	/// Destroy minimap
extern void DestroyMinimap(void);
	/// Draw minimap with viewpoint
extern void DrawMinimap(int vx, int vy);
	/// Draw minimap viewpoint cursor
extern void DrawMinimapCursor(int vx, int vy);

	/// Convert minimap cursor X position to tile map coordinate
extern int ScreenMinimap2MapX(int x);
	/// Convert minimap cursor Y position to tile map coordinate
extern int ScreenMinimap2MapY(int y);

//@}

#endif // !__MINIMAP_H__
