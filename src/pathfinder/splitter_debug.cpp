//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//   T H E   W A R   B E G I N S
//    Stratagus - A free fantasy real time strategy game engine
//
/**@name splitter_debug.c - Map splitter into regions - debugging.  */
//
// (c) Copyright 1999-2003 by Ludovic Pollet
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
// $Id$
//@{

#ifdef MAP_REGIONS

/*----------------------------------------------------------------------------
-- Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stratagus.h"
#include "player.h"
#include "unit.h"
#include "map.h"

#include "pathfinder.h"

#include "splitter_local.h"

/**
** Start an xpm file for storing map representation (debug only)
**
** @param f        Output file
** @param sx       Image size (X)
** @param sy       Image size (Y)
** @param nbcols   Max number of colors
*/
static void StartXpm(FILE * f, int sx,int sy, int nbcols)
{
	int i;
	static unsigned int colors[4] = {
		0x000000,
		0x0000FF, 0x00FF00, 0xFF0000
	};
	static unsigned int xors[8] = {
			0x000000,
			0x808080,
			0x404040,
			0xc0c0c0,
			0x202020,
		0x606060,
			0xa0a0a0,
			0xe0e0e0
	};
	unsigned int color;

	if (nbcols < 3) {
		nbcols = 3;
	}

	fprintf(f, "/* XPM */\n");
	fprintf(f, "static char * _debugregions_xpm[] = {\n");
	fprintf(f, "  /* width height ncols cpp */\n");
	fprintf(f, "  \"%d %d %d %d\",\n", sx, sy, nbcols, 4);
	fprintf(f, "  /* colors */\n");

	for (i = 0 ;i < nbcols; i++) {
		color = colors[i & 3] ^ xors[(i >> 2) & 7];
			fprintf(f, "  \"%04x c #%06x\",\n",i,color);
	}
}

/**
** Assign each region a color in such a way that
** two adjacents regions will never have the same color
*/
static void RegionAssignColor(void)
{
	int i, j, c, good;
	for (i = 0; i < RegionMax; i++) {
		Regions[i].Color = -1;
	}

	for (i = 0; i < RegionMax; i++) {
		c = -1;
		do {
			c++;
			good = 1;
			for (j = 0; j < Regions[i].ConnectionsNumber; j++) {
				if (c == Regions[Regions[i].Connections[j]].Color) {
					good = 0;
					break;
				}
			}
		} while (!good);
		Regions[i].Color = c;
	}
}

/**
** Full Debug
** Create a "debugregions.xpm" containing the splitted map
** Also call some debug functions
*/
void MapSplitterDebug(void)
{
	FILE * f;
	unsigned int color;
	int x, y;
	int total, i, j;
	RegionId reg;
	RegionSegment * seg;


	f = fopen("debugregions.xpm","w");
	if (!f) {
		return;
	}

	/* Output the map. */
	StartXpm(f, TheMap.Info.MapWidth, 2 * TheMap.Info.MapHeight, (RegionMax > 4 ? 1 + RegionMax : 5));

	for (y = 0; y < TheMap.Info.MapHeight; y++) {
			fprintf(f, "  \"");
		for (x = 0; x < TheMap.Info.MapWidth; x++) {
			if (!TileMappable(x, y)) {
				color = 0;
			} else {
					color = (TileIsWater(x, y) ? 1 : 2);
				if (!TilePassable(x, y)) {
					color += 2;
				}
			}
			fprintf(f,"%04x",color);
		}

		fprintf(f, "\"%c\n", (y + 1 == TheMap.Info.MapHeight ? ' ':','));
	}

	RegionAssignColor();

	/* Output the map regions */
	for (y = 0; y < TheMap.Info.MapHeight; y++) {
			fprintf(f, "  \"");
		for (x = 0; x < TheMap.Info.MapWidth; x++) {
			if (!TileMappable(x, y)) {
				color = 0;
			} else {
				reg = RegionMapping(x,y);
					if (reg == NoRegion){
					color = 0;
					} else {
					color = Regions[reg].Color + 1;
				}
			}
			fprintf(f,"%04x",color);
		}

		fprintf(f, "\"%c\n", (y + 1 == TheMap.Info.MapHeight ? ' ':','));
	}

	fprintf(f,"}\n");
	fclose(f);

	total = 0;
	for (i = 0; i < RegionMax; i++) {
		printf("Region %3d connected to :", i);
		for (j = 0; j < Regions[i].ConnectionsNumber; j++) {
			printf(" %3d", Regions[i].Connections[j]);
		}
		printf("\n");
		total += Regions[i].TileCount;
	}

	for (y = 0; y < TheMap.Info.MapHeight; y++) {
		for (x = 0; x < TheMap.Info.MapWidth; x++) {
			if (RegionMapping(x,y) == NoRegion) {
				total++;
			}
		}
	}

	total = 0;
	for (i = 0; i < RegionMax; i++) {
		seg = Regions[i].FirstSegment;
		while (seg) {
			total += seg->MaxX - seg->MinX + 1;
			seg = seg->Next;
		}
	}

	for (y = 0; y < TheMap.Info.MapHeight; y++) {
		for (x = 0; x < TheMap.Info.MapWidth; x++) {
			if (RegionMapping(x,y) == NoRegion) {
				total++;
			}
		}
	}
}

/**
** Check that not mappable tile are not mapped,
** check that water is mapped in watter region,
** ...
*/
void RegionDebugWater(void)
{
	int x,y;
	RegionId reg;
	for (y = 0; y < TheMap.Info.MapWidth; y++) {
			for (x = 0; x < TheMap.Info.MapWidth; x++) {
			reg = RegionMapping(x,y);
			if (!TileMappable(x,y)) {
				Assert(reg == NoRegion);
			} else {
				Assert(reg != NoRegion);
				Assert(TileIsWater(x,y) == Regions[reg].IsWater);
			}
		}
	}
}

/**
** Check existing connections for a given region
** and compare with existing values
**
** @param reg the region
*/
static void RegionDebugConnexion(RegionId reg)
{
	RegionId opp;
	RegionSegment * seg;
	int Connections[MaxRegionNumber];
	int ConnectionsCount[MaxRegionNumber];
	int ConnectionNb;
// RegionDefinition * adef;

	int tx,ty;
	int x,y;
	int i,j;
// int found;

	seg = Regions[reg].FirstSegment;
	ConnectionNb = 0;
	while (seg) {
		y = seg->Y;

		for (x = seg->MinX; x <= seg->MaxX; x++) {
			for (i = 0; i < 8; i++) {
				tx = x + adjacents[i][0];
				ty = y + adjacents[i][1];

				if (!InMap(tx, ty)) {
					continue;
				}
				opp = RegionMapping(tx, ty);
				if (opp == NoRegion || opp == reg) {
					continue;
				}

				for (j = 0; j < ConnectionNb; j++) {
					if (Connections[j] == opp) {
						ConnectionsCount[j]++;
						opp = NoRegion;
						break;
					}
				}

				if (opp != NoRegion) {
					ConnectionsCount[ConnectionNb] = 1;
					Connections[ConnectionNb] = opp;
					ConnectionNb++;
				}
			}
		}

		seg = seg->Next;
	}
/*
	adef = Regions + reg;
	// OK, compare to existing stats
	for (i = 0; i < ConnectionNb; i++) {
		found = 0;
		for (j = 0; j < adef->ConnectionsNumber; j++) {
			if (Connections[i] == adef->Connections[j]) {
				found = 1;
			}
		}
		if (!found) {
			DebugLevel2Fn("Region %d : Connection to %d is missing\n" _C_ reg _C_ Connections[i]);
		}
	}

	for (i = 0; i < adef->ConnectionsNumber; i++) {
		found = 0;
		for (j = 0; j < ConnectionNb; j++) {
			if (adef->Connections[i] == Connections[j]) {
				found = 1;
			}
		}
		if (!found) {
			DebugLevel2Fn("Region %d : Connection to %d does not exists\n" _C_ reg _C_ Connections[i]);
		}
	}
*/
}

/**
** Find all existing connections between regions & compare to dynamic structure
**
*/
void RegionDebugAllConnexions(void)
{
	int i;
	for (i = 0; i < RegionMax; i++) {
		RegionDebugConnexion(i);
	}
}

#endif // MAP_REGIONS
//@}
