//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name tileset.c	-	The tileset. */
//
//	(c) Copyright 1998-2003 by Lutz Sammer and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; version 2 dated June, 1991.
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
//	$Id$

//@{

/*----------------------------------------------------------------------------
--		Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ccl.h"
#include "stratagus.h"
#include "tileset.h"
#include "map.h"
#include "iolib.h"

/*----------------------------------------------------------------------------
--		Variables
----------------------------------------------------------------------------*/

/**
**		Mapping of wc numbers to our internal tileset symbols.
**		The numbers are used in puds.
**		0=summer, 1=winter, 2=wasteland, 3=swamp.
*/
global char** TilesetWcNames;

/**
**		Number of available Tilesets.
*/
global int NumTilesets;

/**
**		Tileset information.
**
**		@see TilesetMax, @see NumTilesets
*/
global Tileset** Tilesets;

/**
**		Size of a tile in X
*/
global int TileSizeX = 32;

/**
**		Size of a tile in Y
*/
global int TileSizeY = 32;

/*----------------------------------------------------------------------------
--		Functions
----------------------------------------------------------------------------*/

/**
**		Load tileset and setup ::TheMap for this tileset.
**
**		@see TheMap @see Tilesets.
*/
global void LoadTileset(void)
{
	int i;
	int n;
	int tile;
	int tiles_per_row;
	int solid;
	int mixed;
	unsigned char* data;
	char* buf;
	const unsigned short* table;

	//
	//  Find the tileset.
	//
	for (i = 0; i < NumTilesets; ++i) {
		if (!strcmp(TheMap.TerrainName, Tilesets[i]->Ident)) {
			break;
		}
	}
	if (i == NumTilesets) {
		fprintf(stderr, "Tileset `%s' not available\n", TheMap.TerrainName);
		ExitFatal(-1);
	}
	DebugCheck(i != TheMap.Terrain);

	if (!Tilesets[i]->Table) {
		char buf[1024];
		LibraryFileName(Tilesets[i]->File, buf);
		LuaLoadFile(buf);
	}

	TheMap.Tileset = Tilesets[i];
	//
	//  Load and prepare the tileset
	//
	buf = alloca(strlen(Tilesets[i]->ImageFile) + 9 + 1);
	strcat(strcpy(buf, "graphics/"), Tilesets[i]->ImageFile);
	ShowLoadProgress("Tileset `%s'", Tilesets[i]->ImageFile);
#ifdef USE_SDL_SURFACE
	TheMap.TileGraphic = LoadGraphic(buf);
#else
	TheMap.TileData = LoadGraphic(buf);
#endif
#ifdef USE_OPENGL
	MakeTexture(TheMap.TileData, TheMap.TileData->Width, TheMap.TileData->Height);
#endif

	TileSizeX = Tilesets[i]->TileSizeX;
	TileSizeY = Tilesets[i]->TileSizeY;

	//
	//  Calculate number of tiles in graphic tile
	//
#ifdef USE_SDL_SURFACE
	tiles_per_row = TheMap.TileGraphic->Width / TileSizeX;

	TheMap.TileCount = n =
		tiles_per_row * (TheMap.TileGraphic->Height / TileSizeY);
#else
	tiles_per_row = TheMap.TileData->Width / TileSizeX;

	TheMap.TileCount = n =
		tiles_per_row * (TheMap.TileData->Height / TileSizeY);
#endif
	DebugLevel2Fn(" %d Tiles in file %s, %d per row\n" _C_ TheMap.
		TileCount _C_ TheMap.Tileset->ImageFile _C_ tiles_per_row);

	if (n > MaxTilesInTileset) {
		fprintf(stderr,
			"Too many tiles in tileset. Increase MaxTilesInTileset and recompile.\n");
		ExitFatal(-1);
	}

	//
	//  Precalculate the graphic starts of the tiles
	//
	data = malloc(n * TileSizeX * TileSizeY);
#ifndef USE_SDL_SURFACE
	TheMap.Tiles = malloc(n * sizeof(*TheMap.Tiles));

	for (i = 0; i < n; ++i) {
		TheMap.Tiles[i] = data + i * TileSizeX * TileSizeY;
	}

	//
	//  Convert the graphic data into faster format
	//
	for (tile = 0; tile < n; ++tile) {
		unsigned char* s;
		unsigned char* d;

		s = (char *)TheMap.TileData->Frames +
			(tile % tiles_per_row) * TileSizeX +
			(tile / tiles_per_row) * TileSizeY * TheMap.TileData->Width;
		d = TheMap.Tiles[tile];
		DebugCheck(d != data + tile * TileSizeX * TileSizeY);
		for (i = 0; i < TileSizeY; ++i) {
			memcpy(d, s, TileSizeX * sizeof(unsigned char));
			d += TileSizeX;
			s += TheMap.TileData->Width;
		}
	}
#endif

#ifndef USE_OPENGL
#ifdef USE_SDL_SURFACE
/*
	free(TheMap.TileGraphic->Frames);		// release old memory
	TheMap.TileGraphic->Frames = data;
	TheMap.TileGraphic->Width = TileSizeX;
	TheMap.TileGraphic->Height = TileSizeY * n;
*/
#else
	free(TheMap.TileData->Frames);		// release old memory
	TheMap.TileData->Frames = data;
	TheMap.TileData->Width = TileSizeX;
	TheMap.TileData->Height = TileSizeY * n;
#endif
#endif

	//
	//  Build the TileTypeTable
	//
	TheMap.Tileset->TileTypeTable =
		calloc(n, sizeof(*TheMap.Tileset->TileTypeTable));

	table = TheMap.Tileset->Table;
	n = TheMap.Tileset->NumTiles;
	for (i = 0; i < n; ++i) {
		if ((tile = table[i])) {
			unsigned flags;

			//Initialize all Lookup Items to zero
			TheMap.Tileset->MixedLookupTable[table[i]] = 0;

			flags = TheMap.Tileset->FlagsTable[i];
			if (flags & MapFieldWaterAllowed) {
				TheMap.Tileset->TileTypeTable[tile] = TileTypeWater;
			} else if (flags & MapFieldCoastAllowed) {
				TheMap.Tileset->TileTypeTable[tile] = TileTypeCoast;
			} else if (flags & MapFieldWall) {
				if (flags & MapFieldHuman) {
					TheMap.Tileset->TileTypeTable[tile] = TileTypeHumanWall;
				} else {
					TheMap.Tileset->TileTypeTable[tile] = TileTypeOrcWall;
				}
			} else if (flags & MapFieldRocks) {
				TheMap.Tileset->TileTypeTable[tile] = TileTypeRock;
			} else if (flags & MapFieldForest) {
				TheMap.Tileset->TileTypeTable[tile] = TileTypeWood;
			}
		}
	}

	//
	//  mark the special tiles
	//
	if ((tile = TheMap.Tileset->TopOneTree)) {
		TheMap.Tileset->TileTypeTable[tile] = TileTypeWood;
	}
	if ((tile = TheMap.Tileset->MidOneTree)) {
		TheMap.Tileset->TileTypeTable[tile] = TileTypeWood;
	}
	if ((tile = TheMap.Tileset->BotOneTree)) {
		TheMap.Tileset->TileTypeTable[tile] = TileTypeWood;
	}
	if ((tile = TheMap.Tileset->TopOneRock)) {
		TheMap.Tileset->TileTypeTable[tile] = TileTypeRock;
	}
	if ((tile = TheMap.Tileset->MidOneRock)) {
		TheMap.Tileset->TileTypeTable[tile] = TileTypeRock;
	}
	if ((tile = TheMap.Tileset->BotOneRock)) {
		TheMap.Tileset->TileTypeTable[tile] = TileTypeRock;
	}

	//
	//  Build wood removement table.
	//
	n = TheMap.Tileset->NumTiles;
	for (mixed = solid = i = 0; i < n;) {
		if (TheMap.Tileset->Tiles[i].BaseTerrain
			&& TheMap.Tileset->Tiles[i].MixTerrain) {
			if (TheMap.Tileset->FlagsTable[i] & MapFieldForest) {
				mixed = i;
			}
			i += 256;
		} else {
			if (TheMap.Tileset->Tiles[i].BaseTerrain != 0 &&
				TheMap.Tileset->Tiles[i].MixTerrain == 0)		{
					if (TheMap.Tileset->FlagsTable[i] & MapFieldForest) {
						solid = i;
				}
			}
			i += 16;
		}
	}
	TheMap.Tileset->WoodTable[ 0] = -1;
	TheMap.Tileset->WoodTable[ 1] = table[mixed + 0x30];
	TheMap.Tileset->WoodTable[ 2] = table[mixed + 0x70];
	TheMap.Tileset->WoodTable[ 3] = table[mixed + 0xB0];
	TheMap.Tileset->WoodTable[ 4] = table[mixed + 0x10];
	TheMap.Tileset->WoodTable[ 5] = table[mixed + 0x50];
	TheMap.Tileset->WoodTable[ 6] = table[mixed + 0x90];
	TheMap.Tileset->WoodTable[ 7] = table[mixed + 0xD0];
	TheMap.Tileset->WoodTable[ 8] = table[mixed + 0x00];
	TheMap.Tileset->WoodTable[ 9] = table[mixed + 0x40];
	TheMap.Tileset->WoodTable[10] = table[mixed + 0x80];
	TheMap.Tileset->WoodTable[11] = table[mixed + 0xC0];
	TheMap.Tileset->WoodTable[12] = table[mixed + 0x20];
	TheMap.Tileset->WoodTable[13] = table[mixed + 0x60];
	TheMap.Tileset->WoodTable[14] = table[mixed + 0xA0];
	TheMap.Tileset->WoodTable[15] = table[solid];
	TheMap.Tileset->WoodTable[16] = -1;
	TheMap.Tileset->WoodTable[17] = TheMap.Tileset->BotOneTree;
	TheMap.Tileset->WoodTable[18] = TheMap.Tileset->TopOneTree;
	TheMap.Tileset->WoodTable[19] = TheMap.Tileset->MidOneTree;

	//Mark which corners of each tile has tree in it.
	//All corners for solid tiles. (Same for rocks)
	//1 Bottom Left
	//2 Bottom Right
	//4 Top Right
	//8 Top Left
	//16 Bottom Tree Tile
	//32 Top Tree Tile
	for (i = solid; i < solid + 16; ++i) {
		TheMap.Tileset->MixedLookupTable[table[i]] = 15;
	}
	for (i = mixed; i < mixed + 256; ++i) {
		int check;

		check = (int)((i - mixed) / 16);
		switch (check) {
			case 0:
				TheMap.Tileset->MixedLookupTable[table[i]] = 8;
				break;
			case 1:
				TheMap.Tileset->MixedLookupTable[table[i]] = 4;
				break;
			case 2:
				TheMap.Tileset->MixedLookupTable[table[i]] = 8 + 4;
				break;
			case 3:
				TheMap.Tileset->MixedLookupTable[table[i]] = 1;
				break;
			case 4:
				TheMap.Tileset->MixedLookupTable[table[i]] = 8 + 1;
				break;
			case 5:
				TheMap.Tileset->MixedLookupTable[table[i]] = 4 + 1;
				break;
			case 6:
				TheMap.Tileset->MixedLookupTable[table[i]] = 8 + 4 + 1;
				break;
			case 7:
				TheMap.Tileset->MixedLookupTable[table[i]] = 2;
				break;
			case 8:
				TheMap.Tileset->MixedLookupTable[table[i]] = 8 + 2;
				break;
			case 9:
				TheMap.Tileset->MixedLookupTable[table[i]] = 4 + 2;
				break;
			case 10:
				TheMap.Tileset->MixedLookupTable[table[i]] = 8 + 4 + 2;
		 		break;
			case 11:
				TheMap.Tileset->MixedLookupTable[table[i]] = 2 + 1;
				break;
			case 12:
				TheMap.Tileset->MixedLookupTable[table[i]] = 8 + 2 + 1;
				break;
			case 13:
				TheMap.Tileset->MixedLookupTable[table[i]] = 4 + 2 + 1;
				break;
			default:
				TheMap.Tileset->MixedLookupTable[table[i]] = 0;
				break;
		}
	}
	//16 Bottom Tree Special
	//32 Top Tree Special
	//64 Mid tree special - differentiate with mixed tiles.
	TheMap.Tileset->MixedLookupTable[TheMap.Tileset->BotOneTree] = 12 + 16;
	TheMap.Tileset->MixedLookupTable[TheMap.Tileset->TopOneTree] = 3 + 32;
	TheMap.Tileset->MixedLookupTable[TheMap.Tileset->MidOneTree] = 15 + 48;

	//
	//  Build rock removement table.
	//
	for (mixed = solid = i = 0; i < n;) {
		if (TheMap.Tileset->Tiles[i].BaseTerrain
			&& TheMap.Tileset->Tiles[i].MixTerrain) {
			if (TheMap.Tileset->FlagsTable[i] & MapFieldRocks) {
				mixed = i;
			}
			i += 256;
		} else {
			if (TheMap.Tileset->Tiles[i].BaseTerrain != 0 &&
				TheMap.Tileset->Tiles[i].MixTerrain == 0) {
					  if (TheMap.Tileset->FlagsTable[i] & MapFieldRocks) {
					solid = i;
					  }
			}
			i += 16;
		}
	}

	//Mark which corners of each tile has rock in it.
	//All corners for solid tiles.
	//1 Bottom Left
	//2 Bottom Right
	//4 Top Right
	//8 Top Left
	for (i = solid; i < solid + 16; ++i) {
		TheMap.Tileset->MixedLookupTable[table[i]] = 15;
	}
	for (i = mixed; i < mixed + 256; ++i) {
		int check;

		check = (int)((i - mixed) / 16);
		switch (check) {
			case 0:
				TheMap.Tileset->MixedLookupTable[table[i]] = 8;
				break;
			case 1:
				TheMap.Tileset->MixedLookupTable[table[i]] = 4;
				break;
			case 2:
				TheMap.Tileset->MixedLookupTable[table[i]] = 8 + 4;
				break;
			case 3:
				TheMap.Tileset->MixedLookupTable[table[i]] = 1;
				break;
			case 4:
				TheMap.Tileset->MixedLookupTable[table[i]] = 8 + 1;
				break;
			case 5:
				TheMap.Tileset->MixedLookupTable[table[i]] = 4 + 1;
				break;
			case 6:
				TheMap.Tileset->MixedLookupTable[table[i]] = 8 + 4 + 1;
				break;
			case 7:
				TheMap.Tileset->MixedLookupTable[table[i]] = 2;
				break;
			case 8:
				TheMap.Tileset->MixedLookupTable[table[i]] = 8 + 2;
				break;
			case 9:
				TheMap.Tileset->MixedLookupTable[table[i]] = 4 + 2;
				break;
			case 10:
				TheMap.Tileset->MixedLookupTable[table[i]] = 8 + 4 + 2;
		 		break;
			case 11:
				TheMap.Tileset->MixedLookupTable[table[i]] = 2 + 1;
				break;
			case 12:
				TheMap.Tileset->MixedLookupTable[table[i]] = 8 + 2 + 1;
				break;
			case 13:
				TheMap.Tileset->MixedLookupTable[table[i]] = 4 + 2 + 1;
				break;
			default:
				TheMap.Tileset->MixedLookupTable[table[i]] = 0;
				break;
		}
	}

	TheMap.Tileset->MixedLookupTable[TheMap.Tileset->BotOneRock] = 12 + 16;
	TheMap.Tileset->MixedLookupTable[TheMap.Tileset->TopOneRock] = 3 + 32;
	TheMap.Tileset->MixedLookupTable[TheMap.Tileset->MidOneRock] = 15 + 48;

	TheMap.Tileset->RockTable[ 0] = -1;
	TheMap.Tileset->RockTable[ 1] = table[mixed + 0x30];
	TheMap.Tileset->RockTable[ 2] = table[mixed + 0x70];
	TheMap.Tileset->RockTable[ 3] = table[mixed + 0xB0];
	TheMap.Tileset->RockTable[ 4] = table[mixed + 0x10];
	TheMap.Tileset->RockTable[ 5] = table[mixed + 0x50];
	TheMap.Tileset->RockTable[ 6] = table[mixed + 0x90];
	TheMap.Tileset->RockTable[ 7] = table[mixed + 0xD0];
	TheMap.Tileset->RockTable[ 8] = table[mixed + 0x00];
	TheMap.Tileset->RockTable[ 9] = table[mixed + 0x40];
	TheMap.Tileset->RockTable[10] = table[mixed + 0x80];
	TheMap.Tileset->RockTable[11] = table[mixed + 0xC0];
	TheMap.Tileset->RockTable[12] = table[mixed + 0x20];
	TheMap.Tileset->RockTable[13] = table[mixed + 0x60];
	TheMap.Tileset->RockTable[14] = table[mixed + 0xA0];
	TheMap.Tileset->RockTable[15] = table[solid];
	TheMap.Tileset->RockTable[16] = -1;
	TheMap.Tileset->RockTable[17] = TheMap.Tileset->BotOneRock;
	TheMap.Tileset->RockTable[18] = TheMap.Tileset->TopOneRock;
	TheMap.Tileset->RockTable[19] = TheMap.Tileset->MidOneRock;
	//
	//		FIXME: Build wall replacement tables
	//
	TheMap.Tileset->HumanWallTable[ 0] = 0x090;
	TheMap.Tileset->HumanWallTable[ 1] = 0x830;
	TheMap.Tileset->HumanWallTable[ 2] = 0x810;
	TheMap.Tileset->HumanWallTable[ 3] = 0x850;
	TheMap.Tileset->HumanWallTable[ 4] = 0x800;
	TheMap.Tileset->HumanWallTable[ 5] = 0x840;
	TheMap.Tileset->HumanWallTable[ 6] = 0x820;
	TheMap.Tileset->HumanWallTable[ 7] = 0x860;
	TheMap.Tileset->HumanWallTable[ 8] = 0x870;
	TheMap.Tileset->HumanWallTable[ 9] = 0x8B0;
	TheMap.Tileset->HumanWallTable[10] = 0x890;
	TheMap.Tileset->HumanWallTable[11] = 0x8D0;
	TheMap.Tileset->HumanWallTable[12] = 0x880;
	TheMap.Tileset->HumanWallTable[13] = 0x8C0;
	TheMap.Tileset->HumanWallTable[14] = 0x8A0;
	TheMap.Tileset->HumanWallTable[15] = 0x0B0;

	TheMap.Tileset->OrcWallTable[ 0] = 0x0A0;
	TheMap.Tileset->OrcWallTable[ 1] = 0x930;
	TheMap.Tileset->OrcWallTable[ 2] = 0x910;
	TheMap.Tileset->OrcWallTable[ 3] = 0x950;
	TheMap.Tileset->OrcWallTable[ 4] = 0x900;
	TheMap.Tileset->OrcWallTable[ 5] = 0x940;
	TheMap.Tileset->OrcWallTable[ 6] = 0x920;
	TheMap.Tileset->OrcWallTable[ 7] = 0x960;
	TheMap.Tileset->OrcWallTable[ 8] = 0x970;
	TheMap.Tileset->OrcWallTable[ 9] = 0x9B0;
	TheMap.Tileset->OrcWallTable[10] = 0x990;
	TheMap.Tileset->OrcWallTable[11] = 0x9D0;
	TheMap.Tileset->OrcWallTable[12] = 0x980;
	TheMap.Tileset->OrcWallTable[13] = 0x9C0;
	TheMap.Tileset->OrcWallTable[14] = 0x9A0;
	TheMap.Tileset->OrcWallTable[15] = 0x0C0;

	// Set destroyed walls to TileTypeUnknown
	for (i = 0; i < 16; ++i) {
		n = 0;
		tile = TheMap.Tileset->HumanWallTable[i];
		while (TheMap.Tileset->Table[tile]) {		// Skip good tiles
			++tile;
			++n;
		}
		while (!TheMap.Tileset->Table[tile]) {		// Skip separator
			++tile;
			++n;
		}
		while (TheMap.Tileset->Table[tile]) {		// Skip good tiles
			++tile;
			++n;
		}
		while (!TheMap.Tileset->Table[tile]) {		// Skip separator
			++tile;
			++n;
		}
		while (n < 16 && TheMap.Tileset->Table[tile]) {
			TheMap.Tileset->TileTypeTable[
				TheMap.Tileset->Table[tile]] = TileTypeUnknown;
			++tile;
			++n;
		}
	}
};

/**
**		Save flag part of tileset.
**
**		@param file		File handle for the saved flags.
**		@param flags		Bit field of the flags.
*/
local void SaveTilesetFlags(CLFile* file, unsigned flags)
{
	if (flags & MapFieldWaterAllowed) {
		CLprintf(file, " 'water");
	}
	if (flags & MapFieldLandAllowed) {
		CLprintf(file, " 'land");
	}
	if (flags & MapFieldCoastAllowed) {
		CLprintf(file, " 'coast");
	}
	if (flags & MapFieldNoBuilding) {
		CLprintf(file, " 'no-building");
	}
	if (flags & MapFieldUnpassable) {
		CLprintf(file, " 'unpassable");
	}
	if (flags & MapFieldWall) {
		CLprintf(file, " 'wall");
	}
	if (flags & MapFieldRocks) {
		CLprintf(file, " 'rock");
	}
	if (flags & MapFieldForest) {
		CLprintf(file, " 'forest");
	}
	if (flags & MapFieldLandUnit) {
		CLprintf(file, " 'land-unit");
	}
	if (flags & MapFieldAirUnit) {
		CLprintf(file, " 'air-unit");
	}
	if (flags & MapFieldSeaUnit) {
		CLprintf(file, " 'sea-unit");
	}
	if (flags & MapFieldBuilding) {
		CLprintf(file, " 'building");
	}
	if (flags & MapFieldHuman) {
		CLprintf(file, " 'human");
	}
}

/**
**		Save solid part of tileset.
**
**		@param file		File handle to save the solid part.
**		@param table		Tile numbers.
**		@param name		Ascii name of solid tile
**		@param flags		Tile attributes.
**		@param start		Start index into table.
*/
local void SaveTilesetSolid(CLFile* file, const unsigned short* table,
	const char* name, unsigned flags, int start)
{
	int i;
	int j;
	int n;

	CLprintf(file, "  'solid (list \"%s\"", name);
	SaveTilesetFlags(file, flags);
	// Remove empty tiles at end of block
	for (n = 15; n >= 0 && !table[start + n]; --n) {
	}
	i = CLprintf(file, "\n	#(");
	for (j = 0; j <= n; ++j) {
		i += CLprintf(file, " %3d", table[start + j]);
	}
	i += CLprintf(file, "))");

	while ((i += 8) < 80) {
		CLprintf(file, "\t");
	}
	CLprintf(file, "; %03X\n", start);
}

/**
**		Save mixed part of tileset.
**
**		@param file		File handle to save the mixed part.
**		@param table		Tile numbers.
**		@param name1		First ascii name of mixed tiles.
**		@param name2		Second Ascii name of mixed tiles.
**		@param flags		Tile attributes.
**		@param start		Start index into table.
**		@param end		End of tiles.
*/
local void SaveTilesetMixed(CLFile* file, const unsigned short* table,
	const char* name1, const char* name2, unsigned flags, int start, int end)
{
	int x;
	int i;
	int j;
	int n;

	CLprintf(file, "  'mixed (list \"%s\" \"%s\"", name1, name2);
	SaveTilesetFlags(file, flags);
	CLprintf(file, "\n");
	for (x = 0; x < 0x100; x += 0x10) {
		if (start + x >= end) {				// Check end must be 0x10 aligned
			break;
		}
		CLprintf(file, "	#(");
		// Remove empty slots at end of table
		for (n = 15; n >= 0 && !table[start + x + n]; --n) {
		}
		i = 6;
		for (j = 0; j <= n; ++j) {
			i += CLprintf(file, " %3d", table[start + x + j]);
		}
		if (x == 0xF0) {
			i += CLprintf(file, "))");
		} else {
			i += CLprintf(file, ")");
		}

		while ((i += 8) < 80) {
			CLprintf(file, "\t");
		}
		CLprintf(file, "; %03X\n", start + x);
	}
}

/**
**		Save the tileset.
**
**		@param file		Output file.
**		@param tileset		Save the content of this tileset.
*/
local void SaveTileset(CLFile* file, const Tileset* tileset)
{
	const unsigned short* table;
	int i;
	int n;

	CLprintf(file, "\n(define-tileset\n  '%s 'class '%s", tileset->Ident,
		tileset->Class);
	CLprintf(file, "\n  'name \"%s\"", tileset->Name);
	CLprintf(file, "\n  'image \"%s\"", tileset->ImageFile);
	CLprintf(file, "\n  'palette \"%s\"", tileset->PaletteFile);
	CLprintf(file, "\n  ;; Slots descriptions");
	CLprintf(file, "\n  'slots (list\n  'special (list\t\t;; Can't be in pud\n");
	CLprintf(file, "	'top-one-tree %d 'mid-one-tree %d 'bot-one-tree %d\n",
		tileset->TopOneTree, tileset->MidOneTree, tileset->BotOneTree);
	CLprintf(file, "	'removed-tree %d\n", tileset->RemovedTree);
	CLprintf(file, "	'growing-tree #( %d %d )\n", tileset->GrowingTree[0],
		tileset->GrowingTree[1]);
	CLprintf(file, "	'top-one-rock %d 'mid-one-rock %d 'bot-one-rock %d\n",
		tileset->TopOneRock, tileset->MidOneRock, tileset->BotOneRock);
	CLprintf(file, "	'removed-rock %d )\n", tileset->RemovedRock);

	table = tileset->Table;
	n = tileset->NumTiles;

	for (i = 0; i < n;) {
		//
		//	  Mixeds
		//
		if (tileset->Tiles[i].BaseTerrain && tileset->Tiles[i].MixTerrain) {
			SaveTilesetMixed(file, table,
				tileset->SolidTerrainTypes[tileset->Tiles[i].BaseTerrain].TerrainName,
				tileset->SolidTerrainTypes[tileset->Tiles[i].MixTerrain].TerrainName,
				tileset->FlagsTable[i], i, n);
			i += 256;
			//
			//	  Solids
			//
		} else {
			SaveTilesetSolid(file, table,
				tileset->SolidTerrainTypes[tileset->Tiles[i].BaseTerrain].TerrainName,
				tileset->FlagsTable[i], i);
			i += 16;
		}
	}
	CLprintf(file, "  )\n");
	CLprintf(file, "  ;; Animated tiles\n");
	CLprintf(file, "  'animations (list #( ) )\n");
	CLprintf(file, "  'objects (list #( ) ))\n");
}

/**
**		Save the current tileset module.
**
**		@param file		Output file.
*/
global void SaveTilesets(CLFile* file)
{
	int i;
	char** sp;

	CLprintf(file, "\n;;; -----------------------------------------\n");
	CLprintf(file, ";;; MODULE: tileset $Id$\n\n");

	//  Original number to internal tileset name

	i = CLprintf(file, "(define-tileset-wc-names");
	for (sp = TilesetWcNames; *sp; ++sp) {
		if (i + strlen(*sp) > 79) {
			i = CLprintf(file, "\n ");
		}
		i += CLprintf(file, " '%s", *sp);
	}
	CLprintf(file, ")\n");

	// 		Save all loaded tilesets

	for (i = 0; i < NumTilesets; ++i) {
		SaveTileset(file, Tilesets[i]);
	}
}

/**
**		Cleanup the tileset module.
**
**		@note		this didn't frees the configuration memory.
*/
global void CleanTilesets(void)
{
	int i;
	int j;
	char** ptr;

	//
	//		Free the tilesets
	//
	for (i = 0; i < NumTilesets; ++i) {
		free(Tilesets[i]->Ident);
		free(Tilesets[i]->File);
		free(Tilesets[i]->Class);
		free(Tilesets[i]->Name);
		free(Tilesets[i]->ImageFile);
		free(Tilesets[i]->PaletteFile);
		free(Tilesets[i]->Table);
		free(Tilesets[i]->FlagsTable);
		free(Tilesets[i]->Tiles);
		free(Tilesets[i]->TileTypeTable);
		free(Tilesets[i]->AnimationTable);
		for (j = 0; j < Tilesets[i]->NumTerrainTypes; ++j) {
			free(Tilesets[i]->SolidTerrainTypes[j].TerrainName);
		}
		free(Tilesets[i]->SolidTerrainTypes);

		free(Tilesets[i]);
	}
	free(Tilesets);
	Tilesets = NULL;
	NumTilesets = 0;

	//
	//		Should this be done by the map?
	//
#ifdef USE_SDL_SURFACE
	VideoSaveFree(TheMap.TileGraphic);
	TheMap.TileGraphic = NULL;
#else
	VideoSaveFree(TheMap.TileData);
	TheMap.TileData = NULL;
	free(TheMap.Tiles);
	TheMap.Tiles = NULL;
#endif

	//
	//		Mapping the original tileset numbers in puds to our internal strings
	//
	if ((ptr = TilesetWcNames)) {		// Free all old names
		while (*ptr) {
			free(*ptr++);
		}
		free(TilesetWcNames);

		TilesetWcNames = NULL;
	}
}

//@}
