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
/**@name map_draw.c - The map drawing. */
//
//      (c) Copyright 1999-2004 by Lutz Sammer and Jimmy Salmon
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "tileset.h"
#include "video.h"
#include "map.h"
#include "../video/intern_video.h"
#include "player.h"
#include "pathfinder.h"
#include "ui.h"
#include "deco.h"

#include "etlib/dllist.h"
#if defined(DEBUG) && defined(TIMEIT)
#include "rdtsc.h"
#endif

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

#ifdef DEBUG
#define noTIMEIT  /// defined time function
#endif

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Draw tile
----------------------------------------------------------------------------*/

/**
**  Draw tile.
**
**  @param tile  tile number
**  @param x     X position into video memory
**  @param y     Y position into video memory
*/
#ifndef USE_OPENGL
/**
**  Draw a tile clipped
**
**  @param tile  pointer to tile graphic data
**  @param x     X position into video memory
**  @param y     Y position into video memory
**
*/
global void VideoDrawTile(const int tile, int x, int y)
{
	int tilepitch;
	int oldx;
	int oldy;
	SDL_Rect srect;
	SDL_Rect drect;

	tilepitch = TheMap.TileGraphic->Width / TileSizeX;

	srect.x = TileSizeX * (tile % tilepitch);
	srect.y = TileSizeY * (tile / tilepitch);
	srect.w = TileSizeX;
	srect.h = TileSizeY;

	oldx = x;
	oldy = y;
	CLIP_RECTANGLE(x, y, srect.w, srect.h);
	srect.x += x - oldx;
	srect.y += y - oldy;


	drect.x = x;
	drect.y = y;

	SDL_BlitSurface(TheMap.TileGraphic->Surface, &srect,
		TheScreen, &drect);
}
#else
global void VideoDrawTile(const int tile, int x, int y)
{
	GLint sx;
	GLint ex;
	GLint sy;
	GLint ey;
	GLfloat stx;
	GLfloat etx;
	GLfloat sty;
	GLfloat ety;
	Graphic *g;
	int t;

	g = TheMap.TileGraphic;
	sx = x;
	ex = sx + TileSizeX;
	sy = y;
	ey = sy + TileSizeY;

	t = tile % (g->Width / TileSizeX);
	stx = (GLfloat)t * TileSizeX / g->Width * g->TextureWidth;
	etx = (GLfloat)(t * TileSizeX + TileSizeX) / g->Width * g->TextureWidth;
	t = tile / (g->Width / TileSizeX);
	sty = (GLfloat)t * TileSizeY / g->Height * g->TextureHeight;
	ety = (GLfloat)(t * TileSizeY + TileSizeY) / g->Height * g->TextureHeight;

	glBindTexture(GL_TEXTURE_2D, g->TextureNames[0]);
	glBegin(GL_QUADS);
	glTexCoord2f(stx, sty);
	glVertex2i(sx, sy);
	glTexCoord2f(stx, ety);
	glVertex2i(sx, ey);
	glTexCoord2f(etx, ety);
	glVertex2i(ex, ey);
	glTexCoord2f(etx, sty);
	glVertex2i(ex, sy);
	glEnd();
}
#endif

/**
**  Draw tile.
**
**  @param tile  Tile number to draw.
**  @param x     X position into video memory
**  @param y     Y position into video memory
*/
global void MapDrawTile(int tile, int x, int y)
{
	VideoDrawTile(tile, x, y);
}

/*----------------------------------------------------------------------------
--  Global functions
----------------------------------------------------------------------------*/

/**
**  Mark position inside viewport be drawn for next display update.
**
**  @param x  X map tile position of point in Map to be marked.
**  @param y  Y map tile position of point in Map to be marked.
**
**  @return   True if inside and marked, false otherwise.
**
**  @note latimerius: MarkDrawPosMap() in split screen environment
**        schedules RedrawMap if (x,y) is visible inside *any* of the existing
**        viewports.  Is this OK, johns?  Do you think it would pay having
**        RedrawViewport0, RedrawViewport1 etc. variables and redraw just
**        vp's that actually need redrawing?  We should evaluate this.
**        JOHNS: A complete viewport redraw is still too much work. The final
**        version should only redraw the needed tiles.
*/
global int MarkDrawPosMap(int x, int y)
{
	Viewport* vp;

	if ((vp = MapTileGetViewport(x, y))) {
		MustRedraw |= RedrawMap;
		return 1;
	}
	return 0;
}

/**
**  Denote wether area in map is overlapping with the viewport.
**
**  @param vp  Viewport pointer.
**  @param sx  X map tile position of area in map to be checked.
**  @param sy  Y map tile position of area in map to be checked.
**  @param ex  X map tile position of area in map to be checked.
**  @param ey  Y map tile position of area in map to be checked.
**
**  @return    True if overlapping, false otherwise.
*/
global int MapAreaVisibleInViewport(const Viewport* vp, int sx, int sy,
	int ex, int ey)
{
	return sx >= vp->MapX && sy >= vp->MapY &&
		ex < vp->MapX + vp->MapWidth && ey < vp->MapY + vp->MapHeight;
}

/**
**  Check if a point is visible (inside) a viewport.
**
**  @param vp  Viewport pointer.
**  @param x   X map tile position of point in map to be checked.
**  @param y   Y map tile position of point in map to be checked.
**
**  @return    True if point is in the visible map, false otherwise
*/
local inline int PointInViewport(const Viewport* vp, int x, int y)
{
	return vp->MapX <= x && x < vp->MapX + vp->MapWidth &&
		vp->MapY <= y && y < vp->MapY + vp->MapHeight;
}

/**
**  Check if any part of an area is visible in a viewport.
**
**  @param vp  Viewport pointer.
**  @param sx  X map tile position of area in map to be checked.
**  @param sy  Y map tile position of area in map to be checked.
**  @param ex  X map tile position of area in map to be checked.
**  @param ey  Y map tile position of area in map to be checked.
**
**  @return    True if any part of area is visible, false otherwise
**
**  @todo Doesn't work if all points lay outside and the area covers
**        the complete viewport.
*/
global int AnyMapAreaVisibleInViewport(const Viewport* vp, int sx, int sy,
	int ex, int ey)
{
	// FIXME: Can be written faster
	return PointInViewport(vp, sx, sy) || PointInViewport(vp, sx, ey) ||
		PointInViewport(vp, ex, sy) || PointInViewport(vp, ex, ey);
}

/**
**  Mark overlapping area with viewport be drawn for next display update.
**
**  @param sx  X map tile position of area in Map to be marked.
**  @param sy  Y map tile position of area in Map to be marked.
**  @param ex  X map tile position of area in Map to be marked.
**  @param ey  Y map tile position of area in Map to be marked.
**
**  @return    True if overlapping and marked, false otherwise.
**
**  @see MustRedrawRow @see MustRedrawTile.
*/
global int MarkDrawAreaMap(int sx, int sy, int ex, int ey)
{
	if (MapTileGetViewport(sx, sy) || MapTileGetViewport(ex, ey) ||
			MapTileGetViewport(sx, ey) || MapTileGetViewport(ex, sy)) {
		MustRedraw |= RedrawMap;
		return 1;
	}
	return 0;
}

/**
**  Enable entire map be drawn for next display update.
*/
global void MarkDrawEntireMap(void)
{
	DebugLevel3Fn("\n");
	MustRedraw |= RedrawMap;
}

/**
**  Draw the map backgrounds.
**
**  @param vp  Viewport pointer.
**  @param x   Map viewpoint x position.
**  @param y   Map viewpoint y position.
**
** StephanR: variables explained below for screen:<PRE>
** *---------------------------------------*
** |										   |
** |			  *-----------------------*		   |<-TheUi.MapY,dy (in pixels)
** |			  |		  |   |		  |   |		  |   |		   |  |
** |			  |		  |   |		  |   |		  |   |		   |  |
** |			  |---+---+---+---+---+---|		   |  |
** |			  |		  |   |		  |   |		  |   |		   |  |MapHeight (in tiles)
** |			  |		  |   |		  |   |		  |   |		   |  |
** |			  |---+---+---+---+---+---|		   |  |
** |			  |		  |   |		  |   |		  |   |		   |  |
** |			  |		  |   |		  |   |		  |   |		   |  |
** |			  *-----------------------*		   |<-ey,TheUI.MapEndY (in pixels)
** |										   |
** |										   |
** *---------------------------------------*
**			  ^							  ^
**			dx|-----------------------|ex,TheUI.MapEndX (in pixels)
** TheUI.MapX		 MapWidth (in tiles)
** (in pixels)
** </PRE>
*/
global void DrawMapBackgroundInViewport(const Viewport* vp, int x, int y)
{
	int sx;
	int sy;
	int dx;
	int ex;
	int dy;
	int ey;
#ifdef TIMEIT
	u_int64_t sv = rdtsc();
	u_int64_t ev;
	static long mv = 9999999;
#endif

	ex = vp->EndX;
	sy = y * TheMap.Width;
	dy = vp->Y - vp->OffsetY;
	ey = vp->EndY;

	while (dy <= ey) {
		sx = x + sy;
		dx = vp->X - vp->OffsetX;
		while (dx <= ex) {
			//
			//  draw only tiles which must be drawn
			//
			// FIXME: unexplored fields could be drawn faster
			if (ReplayRevealMap) {
				MapDrawTile(TheMap.Fields[sx].Tile, dx, dy);
			} else {
				MapDrawTile(TheMap.Fields[sx].SeenTile, dx, dy);
			}

			++sx;
			dx += TileSizeX;
		}
		sy += TheMap.Width;
		dy += TileSizeY;
	}

#ifdef TIMEIT
	ev = rdtsc();
	sx = (ev - sv);
	if (sx < mv) {
		mv = sx;
	}

	DebugLevel1("%ld %ld %3ld\n" _C_ (long)sx _C_ mv _C_ (sx * 100) / mv);
#endif
}

/**
**  Initialize the fog of war.
**  Build tables, setup functions.
*/
global void InitMap(void)
{
}

//@}
