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
/**@name deco.h - Mechanism for all 2D decorations on screen */
//
//      (c) Copyright 2002-2004 by Lutz Sammer and Stephan Rasenberg
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

#ifndef __DECO_H__
#define __DECO_H__

//@{

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

/**
** @fixme docu
**/
typedef struct DecorationSingle {
  struct DecorationSingle *nxt; ///< next single-tile decoration belonging to the same decoration
  unsigned short *tiles;        ///< exact 2x2 tile area
  unsigned short bitmask;       ///< 16bit bitmask to and in order to *mark* the small deco
  int cornerx, cornery;         ///< The coordinates of the upper-left corner of the 64*64 square
#ifdef DEBUG
  int x, y, w, h;  ///< coordinates of the small deco on screen. mostly debug purposes.
#endif
} DecorationSingle;

/**
**  A 2D decoration as is supported by this mechanism, add any to be draw
**  element (sprite/line/button/etc..) as a 2D decoration following this
**  structu, so the mechanism can use it and automaticly update any other
**  decoration overlapping it.
**
**  @note x, y, w, h now needed outside, but might be removed in the future
**/
typedef struct Deco {
  void* data; ///< an user given data-type given to above function, to be able
			  ///< to provide a generic draw-function independent of its data.
  void (*drawclip)(void* data); ///< an user given function that draws the decoration using
								///< some vidoe functions based on the clip rectangle ClipX1,..
  struct DecorationSingle* singles; ///< The sub-decoration type, as this decoration might be split
									///< into multiple small/fixed-sized data-type (internal use only)
  struct Deco* prv; ///< prev decoration based on depth-level (internal use only)
  struct Deco* nxt; ///< next decoration based on depth-level (internal use only)
  int x; ///< dimension as given to DecorationAdd..
  int y; ///< dimension as given to DecorationAdd..
  int w; ///< dimension as given to DecorationAdd..
  int h; ///< dimension as given to DecorationAdd..
  int level; ///< decoration level as given to DecorationAdd (internal use only)
} Deco;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/


extern void DecorationInit(void);

extern Deco* DecorationAdd(void *data,void (*drawclip)(void *data),
	int level, int x, int y, int w, int h );
extern Deco* DecorationMove(Deco *d, int x, int y, int w, int h);
extern void DecorationRemove(Deco *d);
extern void DecorationRemoveLevels(int min, int max);

extern void DecorationMark(Deco *d);

extern void DecorationRefreshDisplay(void);
extern void DecorationUpdateDisplay(void);

//@}

#endif // !__DECO_H__
