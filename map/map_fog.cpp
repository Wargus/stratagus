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
/**@name map_fog.c - The map fog of war handling. */
//
//      (c) Copyright 1999-2004 by Lutz Sammer, Vladi Shabanski,
//                                 Russell Smith, and Jimmy Salmon
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
#include "player.h"
#include "unittype.h"
#include "unit.h"
#include "map.h"
#include "minimap.h"
#include "font.h"
#include "ui.h"
#include "../video/intern_video.h"

#if defined(DEBUG) && defined(TIMEIT)
#include "rdtsc.h"
#endif

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

#ifdef DEBUG

// Use this to see FOW visibility for every tile
#define noDEBUG_FOG_OF_WAR

#define noTIMEIT  /// defined time function
#endif

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

global int OriginalFogOfWar;                /// Use original style fog of war
global int FogOfWarOpacity;                 /// Fog of war Opacity.

/**
**  Mapping for fog of war tiles.
*/
local const int FogTable[16] = {
	 0,11,10, 2,  13, 6, 14, 3,  12, 15, 4, 1,  8, 9, 7, 0,
};

global unsigned char* VisionTable[3];
global int* VisionLookup;

local unsigned char* VisibleTable;

local SDL_Surface* OnlyFogSurface;

/**
**  Draw unexplored area function pointer. (display and video mode independ)
*/
local void (*VideoDrawUnexplored)(const int, int, int);

/**
**  Draw fog of war function pointer. (display and video mode independ)
*/
local void (*VideoDrawFog)(const int, int, int);

/**
**  Draw only fog of war function pointer. (display and video mode independ)
*/
local void (*VideoDrawOnlyFog)(int, int);

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Find the Number of Units that can see this square using a long
**  lookup. So when a 225 Viewed square can be calculated properly.
**
**  @param player  Player to mark sight.
**  @param tx      X center position.
**  @param ty      Y center position.
**
**  @return        Number of units that can see this square.
*/
local int LookupSight(const Player* player, int tx, int ty)
{
	int i;
	int visiblecount;
	int range;
	int mapdistance;
	Unit* unit;

	visiblecount=0;
	for (i = 0; i < player->TotalNumUnits; ++i) {
		unit = player->Units[i];
		range = unit->CurrentSightRange;
		mapdistance = MapDistanceToUnit(tx,ty,unit);
		if (mapdistance <= range) {
			++visiblecount;
		}
		if ((tx >= unit->X && tx < unit->X + unit->Type->TileWidth &&
				mapdistance == range + 1) || (ty >= unit->Y &&
				ty < unit->Y + unit->Type->TileHeight &&
				mapdistance == range + 1)) {
			--visiblecount;
		}
		if (visiblecount >= 255) {
			return 255;
		}
	}
	return visiblecount;
}

/**
**  Find out if a field is seen (By player, or by shared vision)
**  This function will return > 1 with no fog of war.
**
**  @param player  Player to check for.
**  @param x       X tile to check.
**  @param y       Y tile to check.
**
**  @return        0 unexplored, 1 explored, > 1 visible.
*/
global unsigned char IsTileVisible(const Player* player, int x, int y)
{
	int i;
	unsigned char visiontype;
	unsigned char* visible;

	visible = TheMap.Fields[y * TheMap.Width + x].Visible;
	visiontype = visible[player->Player];

	if (visiontype > 1) {
		return visiontype;
	}
	if (!player->SharedVision) {
		if (visiontype) {
			return visiontype + TheMap.NoFogOfWar;
		}
		return 0;
	}

	for (i = 0; i < PlayerMax ; ++i) {
		if (player->SharedVision & (1 << i) &&
				(Players[i].SharedVision & (1 << player->Player))) {
			if (visible[i] > 1) {
				return 2;
			}
			visiontype |= visible[i];
		}
	}
	if (visiontype) {
		return visiontype + TheMap.NoFogOfWar;
	}
	return 0;
}

/**
**  Find out what the tile flags are a tile is covered by fog
**
**  @param player  player who is doing operation
**  @param x       X map location
**  @param y       Y map location
**  @param mask    input mask to filter
**
**  @return        Filtered mask after taking fog into account
*/
global int MapFogFilterFlags(Player* player, int x, int y, int mask)
{
	int nunits;
	int unitcount;
	int fogmask;
	Unit* table[UnitMax];

	// Calculate Mask for tile with fog
    if (x < 0 || y < 0 || x >= TheMap.Width || y >= TheMap.Height) {
		return mask;
	}

	nunits = UnitCacheOnTile(x, y, table);
	fogmask = -1;
	unitcount = 0;
	while (unitcount < nunits) {
		if (!UnitVisibleAsGoal(table[unitcount], player)) {
			fogmask &= ~table[unitcount]->Type->FieldFlags;
		}
		++unitcount;
	}
	return mask & fogmask;
}

/**
**  Mark a tile's sight. (Explore and make visible.)
**
**  @param player  Player to mark sight.
**  @param x       X tile to mark.
**  @param y       Y tile to mark.
*/
global void MapMarkTileSight(const Player* player, int x, int y)
{
	unsigned char v;

	v = TheMap.Fields[x + y * TheMap.Width].Visible[player->Player];
	switch (v) {
		case 0:  // Unexplored
		case 1:  // Unseen
			// When there is NoFogOfWar only unexplored tiles are marked.
			if ((!TheMap.NoFogOfWar) || v == 0) {
				UnitsOnTileMarkSeen(player, x, y, 0);
			}
			v = 2;
			TheMap.Fields[x + y * TheMap.Width].Visible[player->Player] = v;
			if (IsTileVisible(ThisPlayer, x, y) > 1) {
				MapMarkSeenTile(x, y);
			}
			return;
		case 255:  // Overflow
			DebugLevel0Fn("Visible overflow (Player): %d\n" _C_ player->Player);
			break;
		default:  // seen -> seen
			++v;
			break;
	}
	TheMap.Fields[x + y * TheMap.Width].Visible[player->Player] = v;
}

/**
**  Unmark a tile's sight. (Explore and make visible.)
**
**  @param player  Player to mark sight.
**  @param x       X tile to mark.
**  @param y       Y tile to mark.
*/
global void MapUnmarkTileSight(const Player* player, int x, int y)
{
	unsigned char v;

	v = TheMap.Fields[x + y * TheMap.Width].Visible[player->Player];
	switch (v) {
		case 255:
			// FIXME: (mr-russ) Lookupsight is broken :(
			DebugCheck(1);
			v = LookupSight(player, x, y);
			DebugCheck(v < 254);
			break;
		case 0:  // Unexplored
		case 1:
			// We are at minimum, don't do anything shouldn't happen.
			DebugCheck(1);
			break;
		case 2:
			// When there is NoFogOfWar units never get unmarked.
			if (!TheMap.NoFogOfWar) {
				UnitsOnTileUnmarkSeen(player, x, y, 0);
			}
			// Check visible Tile, then deduct...
			if (IsTileVisible(ThisPlayer, x, y) > 1) {
				MapMarkSeenTile(x, y);
			}
		default:  // seen -> seen
			v--;
			break;
	}
	TheMap.Fields[x + y * TheMap.Width].Visible[player->Player] = v;
}

/**
**	Mark a tile for cloak detection.
**
**	@param player  Player to mark sight.
**	@param x       X tile to mark.
**	@param y       Y tile to mark.
*/
global void MapMarkTileDetectCloak(const Player* player, int x, int y)
{
	unsigned char v;

	v = TheMap.Fields[x + y * TheMap.Width].VisCloak[player->Player];
	if (v == 0) {
		UnitsOnTileMarkSeen(player, x, y, 1);
	}
	DebugCheck(v == 255);
	++v;
	TheMap.Fields[x + y * TheMap.Width].VisCloak[player->Player] = v;
}

/**
**	Unmark a tile for cloak detection.
**
**	@param player  Player to mark sight.
**	@param x       X tile to mark.
**	@param y       Y tile to mark.
*/
global void MapUnmarkTileDetectCloak(const Player* player, int x, int y)
{
	unsigned char v;

	v = TheMap.Fields[x + y * TheMap.Width].VisCloak[player->Player];
	DebugCheck(v == 0);
	if (v == 1) {
		UnitsOnTileUnmarkSeen(player, x, y, 1);
	}
	--v;
	TheMap.Fields[x + y * TheMap.Width].VisCloak[player->Player] = v;
}

/**
**  Mark the sight of unit. (Explore and make visible.)
**
**  @param player  player to mark the sight for (not unit owner)
**  @param x       x location to mark
**  @param y       y location to mark
**  @param w       width to mark, in square
**  @param h       height to mark, in square
**  @param range   Radius to mark.
**  @param marker  Function to mark or unmark sight
*/
global void MapSight(const Player* player, int x, int y, int w, int h, int range,
	void (*marker)(const Player*, int, int))
{
	int mx;
	int my;
	int cx[4];
	int cy[4];
	int steps;
	int cycle;
	int p;

	// Mark as seen
	if (!range) {
		DebugLevel0Fn("Zero sight range\n");
		return;
	}

	p = player->Player;

	// Mark Horizontal sight for unit
	for (mx = x - range; mx < x + range + w; ++mx) {
		for (my = y; my < y + h; ++my) {
			if (mx >= 0 && mx < TheMap.Width) {
				marker(player, mx, my);
			}
		}
	}

	// Mark vertical sight for unit (don't remark self) (above unit)
	for (my = y - range; my < y; ++my) {
		for (mx = x; mx < x + w; ++mx) {
			if (my >= 0 && my < TheMap.Width) {
				marker(player, mx, my);
			}
		}
	}

	// Mark vertical sight for unit (don't remark self) (below unit)
	for (my = y + h; my < y + range + h; ++my) {
		for (mx = x; mx < x + w; ++mx) {
			if (my >= 0 && my < TheMap.Width) {
				marker(player, mx, my);
			}
		}
	}

	// Now the cross has been marked, need to use loop to mark in circle
	steps = 0;
	while (VisionTable[0][steps] <= range) {
		// 0 - Top right Quadrant
		cx[0] = x + w - 1;
		cy[0] = y - VisionTable[0][steps];
		// 1 - Top left Quadrant
		cx[1] = x;
		cy[1] = y - VisionTable[0][steps];
		// 2 - Bottom Left Quadrant
		cx[2] = x;
		cy[2] = y + VisionTable[0][steps] + h - 1;
		// 3 - Bottom Right Quadrant
		cx[3] = x + w - 1;
		cy[3] = y + VisionTable[0][steps] + h - 1;
		// loop for steps
		++steps;  // Increment past info pointer
		while (VisionTable[1][steps] != 0 || VisionTable[2][steps] != 0) {
			// Loop through for repeat cycle
			cycle = 0;
			while (cycle++ < VisionTable[0][steps]) {
				cx[0] += VisionTable[1][steps];
				cy[0] += VisionTable[2][steps];
				cx[1] -= VisionTable[1][steps];
				cy[1] += VisionTable[2][steps];
				cx[2] -= VisionTable[1][steps];
				cy[2] -= VisionTable[2][steps];
				cx[3] += VisionTable[1][steps];
				cy[3] -= VisionTable[2][steps];
				if (cx[0] < TheMap.Width && cy[0] >= 0) {
					marker(player, cx[0], cy[0]);
				}
				if (cx[1] >= 0 && cy[1] >= 0) {
					marker(player, cx[1], cy[1]);
				}
				if (cx[2] >= 0 && cy[2] < TheMap.Height) {
					marker(player, cx[2], cy[2]);
				}
				if (cx[3] < TheMap.Width && cy[3] < TheMap.Height) {
					marker(player, cx[3], cy[3]);
				}
			}
			++steps;
		}
	}
}

/**
**  Update fog of war.
*/
global void UpdateFogOfWarChange(void)
{
	int x;
	int y;
	int w;

	DebugLevel0Fn("\n");
	//
	//  Mark all explored fields as visible again.
	//
	if (TheMap.NoFogOfWar) {
		w = TheMap.Width;
		for (y = 0; y < TheMap.Height; ++y) {
			for (x = 0; x < TheMap.Width; ++x) {
				if (IsMapFieldExplored(ThisPlayer, x, y)) {
					MapMarkSeenTile(x, y);
				}
			}
		}
	}
	//
	//  Global seen recount.
	//
	for (x = 0; x < NumUnits; ++x) {
		UnitCountSeen(Units[x]);
	}
	MarkDrawEntireMap();
}

/*----------------------------------------------------------------------------
--  Draw fog solid
----------------------------------------------------------------------------*/

#ifndef USE_OPENGL
/**
**  Draw fog of war
**
**  @param tile  tile number
**  @param x     X position into video memory
**  @param y     Y position into video memory
*/
global void VideoDrawFogSDL(const int tile, int x, int y)
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

	SDL_SetAlpha(TheMap.TileGraphic->Surface, SDL_SRCALPHA | SDL_RLEACCEL, FogOfWarOpacity);
	SDL_BlitSurface(TheMap.TileGraphic->Surface, &srect, TheScreen, &drect);
	SDL_SetAlpha(TheMap.TileGraphic->Surface, SDL_RLEACCEL, 0);
}

/**
**  Draw unexplored fog of war
**
**  @param tile  tile number
**  @param x     X position into video memory
**  @param y     Y position into video memory
*/
global void VideoDrawUnexploredSDL(const int tile, int x, int y)
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
	SDL_BlitSurface(TheMap.TileGraphic->Surface, &srect, TheScreen, &drect);
}

/**
**  Draw only fog of war
**
**  @param x     X position into video memory
**  @param y     Y position into video memory
*/
global void VideoDrawOnlyFogSDL(int x, int y)
{
	int oldx;
	int oldy;
	SDL_Rect srect;
	SDL_Rect drect;

	srect.x = 0;
	srect.y = 0;
	srect.w = OnlyFogSurface->w;
	srect.h = OnlyFogSurface->h;

	oldx = x;
	oldy = y;
	CLIP_RECTANGLE(x, y, srect.w, srect.h);
	srect.x += x - oldx;
	srect.y += y - oldy;

	drect.x = x;
	drect.y = y;

	SDL_BlitSurface(OnlyFogSurface, &srect, TheScreen, &drect);
}

#else

/**
**  Draw fog of war
**
**  @param tile  tile number
**  @param x     X position into video memory
**  @param y     Y position into video memory
*/
global void VideoDrawFogOpenGL(
	const int tile __attribute__((unused)),
	int x __attribute__((unused)), int y __attribute__((unused)))
{
	int tilepitch;
	Graphic* graphic;
	int gx;
	int gy;
	int sx;
	int ex;
	int sy;
	int ey;
	GLfloat stx;
	GLfloat etx;
	GLfloat sty;
	GLfloat ety;

	tilepitch = TheMap.TileGraphic->Width / TileSizeX;
	graphic = TheMap.TileGraphic;

	gx = TileSizeX * (tile % tilepitch);
	gy = TileSizeY * (tile / tilepitch);

	sx = x;
	ex = sx + TileSizeX;
	sy = y;
	ey = y + TileSizeY;

	stx = (GLfloat)gx / graphic->Width * graphic->TextureWidth;
	etx = (GLfloat)(gx + TileSizeX) / graphic->Width * graphic->TextureWidth;
	sty = (GLfloat)gy / graphic->Height * graphic->TextureHeight;
	ety = (GLfloat)(gy + TileSizeY) / graphic->Height * graphic->TextureHeight;

	// FIXME: slow
	glColor4ub(0, 0, 0, FogOfWarOpacity);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glBindTexture(GL_TEXTURE_2D, graphic->TextureNames[0]);
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

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

/**
**  Draw unexplored fog of war
**
**  @param tile  tile number
**  @param x     X position into video memory
**  @param y     Y position into video memory
*/
global void VideoDrawUnexploredOpenGL(const int tile, int x, int y)
{
	int tilepitch;
	Graphic* graphic;
	int gx;
	int gy;
	int sx;
	int ex;
	int sy;
	int ey;
	GLfloat stx;
	GLfloat etx;
	GLfloat sty;
	GLfloat ety;

	tilepitch = TheMap.TileGraphic->Width / TileSizeX;
	graphic = TheMap.TileGraphic;

	gx = TileSizeX * (tile % tilepitch);
	gy = TileSizeY * (tile / tilepitch);

	sx = x;
	ex = sx + TileSizeX;
	sy = y;
	ey = y + TileSizeY;

	stx = (GLfloat)gx / graphic->Width * graphic->TextureWidth;
	etx = (GLfloat)(gx + TileSizeX) / graphic->Width * graphic->TextureWidth;
	sty = (GLfloat)gy / graphic->Height * graphic->TextureHeight;
	ety = (GLfloat)(gy + TileSizeY) / graphic->Height * graphic->TextureHeight;

	glBindTexture(GL_TEXTURE_2D, graphic->TextureNames[0]);
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

/**
**  Draw only fog of war
**
**  @param x  X position into video memory
**  @param y  Y position into video memory
*/
global void VideoDrawOnlyFogOpenGL(int x, int y)
{
	GLint sx;
	GLint ex;
	GLint sy;
	GLint ey;

	sx = x;
	ex = sx + TileSizeX;
	sy = y;
	ey = sy + TileSizeY;

	glDisable(GL_TEXTURE_2D);
	glColor4ub(0, 0, 0, FogOfWarOpacity);
	glBegin(GL_QUADS);
	glVertex2i(sx, sy);
	glVertex2i(sx, ey);
	glVertex2i(ex, ey);
	glVertex2i(ex, sy);
	glEnd();
	glEnable(GL_TEXTURE_2D);
}
#endif

/*----------------------------------------------------------------------------
--  Old version correct working but not 100% original
----------------------------------------------------------------------------*/

/**
**  Draw fog of war tile.
**
**  @param sx  Offset into fields to current tile.
**  @param sy  Start of the current row.
**  @param dx  X position into video memory.
**  @param dy  Y position into video memory.
*/
local void DrawFogOfWarTile(int sx, int sy, int dx, int dy)
{
	int w;
	int tile;
	int tile2;
	int x;
	int y;

#define IsMapFieldExploredTable(x, y) \
	(VisibleTable[(y) * TheMap.Width + (x)])
#define IsMapFieldVisibleTable(x, y) \
	(VisibleTable[(y) * TheMap.Width + (x)] > 1)

	w = TheMap.Width;
	tile = tile2 = 0;
	x = sx - sy;
	y = sy / TheMap.Width;

	//
	//  Which Tile to draw for fog
	//
	// Investigate tiles around current tile
	// 1 2 3
	// 4 * 5
	// 6 7 8

	if (sy) {
		if (sx != sy) {
			if (!IsMapFieldExploredTable(x - 1, y - 1)) {
				tile2 |= 2;
				tile |= 2;
			} else if (!IsMapFieldVisibleTable(x - 1, y - 1)) {
				tile |= 2;
			}
		}
		if (!IsMapFieldExploredTable(x, y - 1)) {
			tile2 |= 3;
			tile |= 3;
		} else if (!IsMapFieldVisibleTable(x, y - 1)) {
			tile |= 3;
		}
		if (sx != sy + w - 1) {
			if (!IsMapFieldExploredTable(x + 1, y - 1)) {
				tile2 |= 1;
				tile |= 1;
			} else if (!IsMapFieldVisibleTable(x + 1, y - 1)) {
				tile |= 1;
			}
		}
	}

	if (sx != sy) {
		if (!IsMapFieldExploredTable(x - 1, y)) {
			tile2 |= 10;
			tile |= 10;
		} else if (!IsMapFieldVisibleTable(x - 1, y)) {
			tile |= 10;
		}
	}
	if (sx != sy + w - 1) {
		if (!IsMapFieldExploredTable(x + 1, y)) {
			tile2 |= 5;
			tile |= 5;
		} else if (!IsMapFieldVisibleTable(x + 1, y)) {
			tile |= 5;
		}
	}

	if (sy + w < TheMap.Height * w) {
		if (sx != sy) {
			if (!IsMapFieldExploredTable(x - 1, y + 1)) {
				tile2 |= 8;
				tile |= 8;
			} else if (!IsMapFieldVisibleTable(x - 1, y + 1)) {
				tile |= 8;
			}
		}
		if (!IsMapFieldExploredTable(x, y + 1)) {
			tile2 |= 12;
			tile |= 12;
		} else if (!IsMapFieldVisibleTable(x, y + 1)) {
			tile |= 12;
		}
		if (sx != sy + w - 1) {
			if (!IsMapFieldExploredTable(x + 1, y + 1)) {
				tile2 |= 4;
				tile |= 4;
			} else if (!IsMapFieldVisibleTable(x + 1, y + 1)) {
				tile |= 4;
			}
		}
	}

	tile = FogTable[tile];
	tile2 = FogTable[tile2];

	if (ReplayRevealMap) {
		tile2 = 0;
		tile = 0;
	}

	if (IsMapFieldVisibleTable(x, y) || ReplayRevealMap) {
		if (tile && tile != tile2) {
			VideoDrawFog(tile, dx, dy);
		}
	} else {
		VideoDrawOnlyFog(dx, dy);
	}
	if (tile2) {
		VideoDrawUnexplored(tile2, dx, dy);
	}

#undef IsMapFieldExploredTable
#undef IsMapFieldVisibleTable
}

/**
**  Draw the map fog of war.
**
**  @param vp  Viewport pointer.
**  @param x   Map viewpoint x position.
**  @param y   Map viewpoint y position.
*/
global void DrawMapFogOfWar(Viewport* vp, int x, int y)
{
	int sx;
	int sy;
	int dx;
	int ex;
	int dy;
	int ey;
	int p;
	int my;
	int mx;

#ifdef TIMEIT
	u_int64_t sv = rdtsc();
	u_int64_t ev;
	static long mv = 9999999;
#endif

	// flags must redraw or not
#ifdef NEW_MAPDRAW
	char* redraw_row;
	char* redraw_tile;

	redraw_row = vp->MustRedrawRow;
	redraw_tile = vp->MustRedrawTile;
#endif
	if (ReplayRevealMap) {
		return;
	}
	p = ThisPlayer->Player;

	sx = vp->MapX - 1;
	if (sx < 0) {
		sx = 0;
	}
	ex = vp->MapX + vp->MapWidth + 1;
	if (ex > TheMap.Width) {
		ex = TheMap.Width;
	}
	my = vp->MapY - 1;
	if (my < 0) {
		my = 0;
	}
	ey = vp->MapY + vp->MapHeight + 1;
	if (ey > TheMap.Height) {
		ey = TheMap.Height;
	}
	// Update for visibility all tile in viewport
	// and 1 tile around viewport (for fog-of-war connection display)
	for (my = vp->MapY; my < ey; ++my) {
		for (mx = sx; mx < ex; ++mx) {
			VisibleTable[my * TheMap.Width + mx] = IsTileVisible(ThisPlayer, mx, my);
		}
	}
	ex = vp->EndX;
	sy = y * TheMap.Width;
	dy = vp->Y - vp->OffsetY;
	ey = vp->EndY;

	while (dy <= ey) {
#ifdef NEW_MAPDRAW
		// row must be redrawn
		if (*redraw_row) {
#if NEW_MAPDRAW > 1
			(*redraw_row)--;
#else
			*redraw_row = 0;
#endif
#endif
			sx = x + sy;
			dx = vp->X - vp->OffsetX;
			while (dx <= ex) {
#ifdef NEW_MAPDRAW
				if (*redraw_tile) {
#if NEW_MAPDRAW > 1
					(*redraw_tile)--;
#else
					*redraw_tile = 0;
#endif
#endif
					mx = (dx - vp->X + vp->OffsetX) / TileSizeX + vp->MapX;
					my = (dy - vp->Y + vp->OffsetY) / TileSizeY + vp->MapY;
					if (VisibleTable[my * TheMap.Width + mx]) {
						DrawFogOfWarTile(sx, sy, dx, dy);
					} else {
						VideoFillRectangleClip(ColorBlack, dx, dy, TileSizeX, TileSizeY);
					}

// Used to debug NEW_FOW problems
#if defined(DEBUG_FOG_OF_WAR)
extern int VideoDrawText(int x, int y, unsigned font, const unsigned char* text);
#define GameFont 1
					{
						char seen[7];
						int x = (dx - vp->X) / TileSizeX + vp->MapX;
						int y = (dy - vp->Y) / TileSizeY + vp->MapY;

#if 1
						// Fog of War Vision
						// Really long and ugly, shared and own vision:
						// sprintf(seen,"%d(%d)",TheMap.Fields[y * TheMap.Width + x].Visible[ThisPlayer->Player],IsMapFieldVisible(ThisPlayer,x, y));
						// Shorter version, but no shared vision:
						sprintf(seen, "%d", TheMap.Fields[y * TheMap.Width + x].Visible[ThisPlayer->Player]);
//						if (TheMap.Fields[y * TheMap.Width + x].Visible[0]) {
							VideoDrawText(dx, dy, GameFont,seen);
//						}
#else
						// Unit Distance Checks
						if (Selected[1] && Selected[0]) {
							sprintf(seen, "%d", MapDistanceBetweenUnits(Selected[0], Selected[1]));
							VideoDrawText(dx, dy, GameFont, seen);
						} else if (Selected[0]) {
							sprintf(seen, "%d", MapDistanceToUnit(x, y,Selected[0]));
							VideoDrawText(dx, dy, GameFont, seen);
						}
#endif
					}
#endif
#ifdef NEW_MAPDRAW
				}
				++redraw_tile;
#endif
				++sx;
				dx += TileSizeX;
			}
#ifdef NEW_MAPDRAW
		} else {
			redraw_tile += vp->MapWidth;
		}
		++redraw_row;
#endif
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
**  Initialise the fog of war.
**  Build tables, setup functions.
*/
global void InitMapFogOfWar(void)
{
	VisibleTable = malloc(TheMap.Width * TheMap.Height * sizeof(*VisibleTable));

#ifdef USE_OPENGL
	VideoDrawUnexplored = VideoDrawUnexploredOpenGL;
	VideoDrawFog = VideoDrawFogOpenGL;
	VideoDrawOnlyFog = VideoDrawOnlyFogOpenGL;
#else
	//
	//	Generate Only Fog surface.
	//
	{
		unsigned char r;
		unsigned char g;
		unsigned char b;
		Uint32 color;
		SDL_Surface* s;

		s = SDL_CreateRGBSurface(SDL_SWSURFACE, TileSizeX, TileSizeY,
			32, RMASK, GMASK, BMASK, AMASK);

		// FIXME: Make the color configurable
		SDL_GetRGB(ColorBlack, TheScreen->format, &r, &g, &b);
		color = VideoMapRGB(s->format, r, g, b);

		SDL_FillRect(s, NULL, color);
		OnlyFogSurface = SDL_DisplayFormat(s);
		SDL_SetAlpha(OnlyFogSurface, SDL_SRCALPHA | SDL_RLEACCEL, FogOfWarOpacity);
		SDL_FreeSurface(s);
	}
	VideoDrawUnexplored = VideoDrawUnexploredSDL;
	VideoDrawFog = VideoDrawFogSDL;
	VideoDrawOnlyFog = VideoDrawOnlyFogSDL;
#endif
}

/**
**  Cleanup the fog of war.
*/
global void CleanMapFogOfWar(void)
{
	if (VisibleTable) {
		free(VisibleTable);
		VisibleTable = NULL;
	}
#ifndef OPENGL
	if (OnlyFogSurface) {
		SDL_FreeSurface(OnlyFogSurface);
		OnlyFogSurface = NULL;
	}
#endif
}

/**
**  Initialize Vision and Goal Tables.
*/
global void InitVisionTable(void)
{
	int* visionlist;
	int maxsize;
	int sizex;
	int sizey;
	int maxsearchsize;
	int i;
	int VisionTablePosition;
	int marker;
	int direction;
	int right;
	int up;
	int repeat;

	// Initialize Visiontable to large size, can't be more entries than tiles.
	VisionTable[0] = malloc(MaxMapWidth * MaxMapWidth * sizeof(int));
	VisionTable[1] = malloc(MaxMapWidth * MaxMapWidth * sizeof(int));
	VisionTable[2] = malloc(MaxMapWidth * MaxMapWidth * sizeof(int));

	VisionLookup = malloc((MaxMapWidth + 2) * sizeof(int));
#ifndef SQUAREVISION
	visionlist = malloc(MaxMapWidth * MaxMapWidth * sizeof(int));
	//*2 as diagonal distance is longer

	maxsize = MaxMapWidth;
	maxsearchsize = MaxMapWidth;
	// Fill in table of map size
	for (sizex = 0; sizex < maxsize; ++sizex) {
		for (sizey = 0; sizey < maxsize; ++sizey) {
			visionlist[sizey * maxsize + sizex] = isqrt(sizex * sizex + sizey * sizey);
		}
	}

	VisionLookup[0] = 0;
	i = 1;
	VisionTablePosition = 0;
	while (i < maxsearchsize) {
		// Set Lookup Table
		VisionLookup[i] = VisionTablePosition;
		// Put in Null Marker
		VisionTable[0][VisionTablePosition] = i;
		VisionTable[1][VisionTablePosition] = 0;
		VisionTable[2][VisionTablePosition] = 0;
		++VisionTablePosition;


		// find i in left column
		marker = maxsize * i;
		direction = 0;
		right = 0;
		up = 0;

		// If not on top row, continue
		do {
			repeat = 0;
			do {
				// search for repeating
				// Test Right
				if ((repeat == 0 || direction == 1) && visionlist[marker + 1] == i) {
					right = 1;
					up = 0;
					++repeat;
					direction = 1;
					++marker;
				} else if ((repeat == 0 || direction == 2) && visionlist[marker - maxsize] == i) {
					up = 1;
					right = 0;
					++repeat;
					direction = 2;
					marker = marker - maxsize;
				} else if ((repeat == 0 || direction == 3) && visionlist[marker + 1 - maxsize] == i &&
						visionlist[marker - maxsize] != i && visionlist[marker + 1] != i) {
					up = 1;
					right = 1;
					++repeat;
					direction = 3;
					marker = marker + 1 - maxsize;
				} else {
					direction = 0;
					break;
				}

			   // search rigth
			   // search up - store as down.
			   // search diagonal
			}  while (direction && marker > (maxsize * 2));
			if (right || up) {
				VisionTable[0][VisionTablePosition] = repeat;
				VisionTable[1][VisionTablePosition] = right;
				VisionTable[2][VisionTablePosition] = up;
				++VisionTablePosition;
			}
		} while (marker > (maxsize * 2));
		++i;
	}

	free(visionlist);
#else
	// Find maximum distance in corner of map.
	maxsize = MaxMapWidth;
	maxsearchsize = isqrt(MaxMapWidth / 2);
	// Mark 1, It's a special case
	// Only Horizontal is marked
	VisionTable[0][0] = 1;
	VisionTable[1][0] = 0;
	VisionTable[2][0] = 0;
	VisionTable[0][1] = 1;
	VisionTable[1][1] = 1;
	VisionTable[2][1] = 0;

	// Mark VisionLookup
	VisionLookup[0] = 0;
	VisionLookup[1] = 0;
	i = 2;
	VisionTablePosition = 1;
	while (i <= maxsearchsize) {
		++VisionTablePosition;
		VisionLookup[i] = VisionTablePosition;
		// Setup Vision Start
		VisionTable[0][VisionTablePosition] = i;
		VisionTable[1][VisionTablePosition] = 0;
		VisionTable[2][VisionTablePosition] = 0;
		// Do Horizontal
		++VisionTablePosition;
		VisionTable[0][VisionTablePosition] = i;
		VisionTable[1][VisionTablePosition] = 1;
		VisionTable[2][VisionTablePosition] = 0;
		// Do Vertical
		++VisionTablePosition;
		VisionTable[0][VisionTablePosition] = i - 1;
		VisionTable[1][VisionTablePosition] = 0;
		VisionTable[2][VisionTablePosition] = 1;
		i++;
	}

	// Update Size of VisionTable to what was used.
	realloc(VisionTable[0], (VisionTablePosition + 2) * sizeof(int));
	realloc(VisionTable[1], (VisionTablePosition + 2) * sizeof(int));
	realloc(VisionTable[2], (VisionTablePosition + 2) * sizeof(int));
#endif
}

/**
**  Clean Up Generated Vision and Goal Tables.
*/
global void FreeVisionTable(void)
{
	// Free Vision Data
	if (VisionTable[0]) {
		free(VisionTable[0]);
		VisionTable[0] = NULL;
	}
	if (VisionTable[1]) {
		free(VisionTable[1]);
		VisionTable[1] = NULL;
	}
	if (VisionTable[2]) {
		free(VisionTable[2]);
		VisionTable[2] = NULL;
	}
	if (VisionLookup) {
		free(VisionLookup);
		VisionLookup = NULL;
	}
}
//@}
