//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name deco.h 	-	Mechanism for all 2D decorations on screen */
//
//	(c) Copyright 2002 by Lutz Sammer and Stephan Rasenberg
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the License,
//	or (at your option) any later version.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

#ifndef __DECO_H__
#define __DECO_H__

//@{

/*----------------------------------------------------------------------------
--      Declarations
----------------------------------------------------------------------------*/

/**
** Each 2D decoration on screen has a depth-level (z-position) which determines
** the order in which is needs to be drawn together with the other listed
** decorations. For this to work properly, everything that needs to be drawn
** should be added as decoration to this mechanism.
** FIXME: proepr levels should be defined here..
*/
typedef enum {
  LevUnderground,
  LevMole,
  LevGround,
  LevWaterLow,
  LevCarLow,
  LevPersonLow,
  LevPersonHigh,
  LevCarHigh,
  LevWater,
  LevBuilding,
  LevMountain,
  LevSkylow,
  LevProjectile,
  LevSkyMid,
  LevArplaneLow,
  LevSkyHigh,
  LevAirplaneHigh,
  LevSkyHighest,
  LevCount 
} DecorationLevel;

/**
**	
**/
typedef struct DecorationSingle {
// next single-tile decoration belonging to the same decoration
  struct DecorationSingle *nxt;
// exact 2x2 tile area
  char *tiles;
// 16bit bitmask which denote the area in above tiles overlapped by this deco
  unsigned int lefttopmask, righttopmask, leftbottommask, rightbottommask;
// the bit index (bity+bitx) of the left-top in the first tile
// @note  bity is a multiple of 4 tiles (so no multiple needed)
  int bitx, bity;
// left-top pixel position
  int topleftx, toplefty;
} DecorationSingle;

/**
**	A 2D decoration as is supported by this mechanism, add any to be draw
**	element (sprite/line/button/etc..) as a 2D decoration following this
**	structu, so the mechanism can use it and automaticly update any other
**	decoration overlapping it.
**
**	mark	= a function that marks the part of the screen covered by
**		  this decoration using DecorationMark.. functions below.
**	check	= a function that check the part of the screen covered by
**		  this decoration using DecorationCheck.. functions below.
**
**	@note: the mark and check functions are made in control of external
**	functions, as not every decoration is marked as a single box, but need
**	to mark/check multiple box-areas (ie. a line).
**
**	draw	= a function that draws the given decoration, just as it
**	normaly would, directly using the video draw functions.
**
**	@note: the mark,check and draw functions need to be consistent with
**	eachother (both mark as check need to cover a total area exactly/bigger
**	than the draw routine will affect), else the screen would looked
**	screwed up. The mark function might mark a slightly larger area when
**	marking accurately is expensive in CPU usage (ie marking a line), but
**	this will cause unneeded drawings as other check operations will
**	trigger upon this area.
**
**	nxt	= next decoration based on depth-level (internal use only)
**/
typedef struct Decoration {
  void *data;
  void (*drawclip)(void *data);
  struct DecorationSingle *singles;
  struct Decoration *nxt;
  int x, y, w, h;
} Decoration;

/**
**
**
**/

/*----------------------------------------------------------------------------
--      Functions
----------------------------------------------------------------------------*/


extern void DecorationInit(void);

extern Decoration *DecorationAdd( void *data,
				   void (*drawclip)(void *data),
				   DecorationLevel l, 
				   unsigned x, unsigned y,
				   unsigned w, unsigned h );
extern void DecorationRemove( Decoration *d );

extern void DecorationMark( Decoration *d );

extern void DecorationRefreshDisplay(void);
extern void DecorationUpdateDisplay(void);

//@}

#endif	// !__DECO_H__
