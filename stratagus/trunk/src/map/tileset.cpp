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
/**@name tileset.c	-	The tileset. */
//
//	(c) Copyright 1998-2001 by Lutz Sammer
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

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freecraft.h"
#include "tileset.h"
#include "map.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern int WoodTable[16];		/// Table for wood removable.
extern int RockTable[20];		/// Table for rock removable.

/**
**	Mapping of wc numbers to our internal tileset symbols.
**	The numbers are used in puds.
**	0=summer, 1=winter, 2=wasteland, 3=swamp.
*/
global char** TilesetWcNames;

/**
**	Number of available Tilesets.
*/
global int NumTilesets;

/**
**	Tileset information.
**
**	@see TilesetMax, @see NumTilesets
*/
global Tileset** Tilesets;

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Load tileset and setup TheMap for this tileset.
**
**	@see TheMap, @see Tilesets
*/
global void LoadTileset(void)
{
    int i;
    int n;
    int tile;
    int gap;
    int tiles_per_row;
    unsigned char* data;
    char* buf;
    const unsigned short* table;

    //
    //	Find the tileset.
    //
    for( i=0; i<NumTilesets; ++i ) {
	if( !strcmp(TheMap.TerrainName,Tilesets[i]->Ident) ) {
	    break;
	}
    }
    if( i==NumTilesets ) {
	fprintf(stderr,"Tileset `%s' not available\n",TheMap.TerrainName);
	exit(-1);
    }
    DebugCheck( i!=TheMap.Terrain );
    TheMap.Tileset=Tilesets[i];

    //
    //	Load and prepare the tileset
    //
    buf=alloca(strlen(Tilesets[i]->File)+9+1);
    strcat(strcpy(buf,"graphics/"),Tilesets[i]->File);
    ShowLoadProgress("\tTileset `%s'\n",Tilesets[i]->File);
    TheMap.TileData=LoadGraphic(buf);

    //
    //	Calculate number of tiles in graphic tile
    //
    if( TheMap.TileData->Width==626 ) {
	// FIXME: allow 1 pixel gap between the tiles!!
	gap=1;
	tiles_per_row=(TheMap.TileData->Width+1)/(TileSizeX+1);
	TheMap.TileCount=n=tiles_per_row*((TheMap.TileData->Height+1)
		/(TileSizeY+1));
    } else if( TheMap.TileData->Width==527 ) {
	// FIXME: allow 1 pixel gap between the tiles!!
	gap=1;
	tiles_per_row=(TheMap.TileData->Width+1)/(TileSizeX+1);
	TheMap.TileCount=n=tiles_per_row*((TheMap.TileData->Height+1)
		/(TileSizeY+1));
    } else {
	gap=0;
	tiles_per_row=TheMap.TileData->Width/TileSizeX;
	TheMap.TileCount=n=tiles_per_row*(TheMap.TileData->Height
		/TileSizeY);
    }

    DebugLevel2Fn(" %d Tiles in file %s, %d per row\n"
	    ,TheMap.TileCount,TheMap.Tileset->File,tiles_per_row);

    if( n>MaxTilesInTileset ) {
	fprintf(stderr,"Too many tiles in tileset. Increase MaxTilesInTileset and recompile.\n");
	exit(-1);
    }


    //
    //	Precalculate the graphic starts of the tiles
    //
    data=malloc(n*TileSizeX*TileSizeY);
    TheMap.Tiles=malloc(n*sizeof(*TheMap.Tiles));
    for( i=0; i<n; ++i ) {
	TheMap.Tiles[i]=data+i*TileSizeX*TileSizeY;
    }

    //
    //	Convert the graphic data into faster format
    //
    for( tile=0; tile<n; ++tile ) {
	unsigned char* s;
	unsigned char* d;

	s=((char*)TheMap.TileData->Frames)
		+((tile%tiles_per_row)*(TileSizeX+gap))
		+((tile/tiles_per_row)*(TileSizeY+gap))*TheMap.TileData->Width;
	d=TheMap.Tiles[tile];
	if( d!=data+tile*TileSizeX*TileSizeY ) {
	    abort();
	}
	for( i=0; i<TileSizeY; ++i ) {
	    memcpy(d,s,TileSizeX*sizeof(unsigned char));
	    d+=TileSizeX;
	    s+=TheMap.TileData->Width;
	}
    }

    free(TheMap.TileData->Frames);	// release old memory
    TheMap.TileData->Frames=data;
    TheMap.TileData->Width=TileSizeX;
    TheMap.TileData->Height=TileSizeY*n;

    //
    //	Build the TileTypeTable
    //
    //	FIXME: types are currently hardcoded, use the supplied flags.
    //

    table=TheMap.Tileset->Table;
    TheMap.Tileset->TileTypeTable
	    =calloc(n,sizeof(*TheMap.Tileset->TileTypeTable));

    //
    //	Solid tiles
    //
    for( i=0; i<0x10; ++i ) {
	if( (tile=table[0x010+i]) ) {		// solid light water
	    TheMap.Tileset->TileTypeTable[tile]=TileTypeWater;
	}
	if( (tile=table[0x020+i]) ) {		// solid dark water
	    TheMap.Tileset->TileTypeTable[tile]=TileTypeWater;
	}
	if( (tile=table[0x030+i]) ) {		// solid light coast
	    TheMap.Tileset->TileTypeTable[tile]=TileTypeCoast;
	}
	if( (tile=table[0x040+i]) ) {		// solid dark coast
	    TheMap.Tileset->TileTypeTable[tile]=TileTypeCoast;
	}
	if( (tile=table[0x050+i]) ) {		// solid light ground
	    TheMap.Tileset->TileTypeTable[tile]=TileTypeGrass;
	}
	if( (tile=table[0x060+i]) ) {		// solid dark ground
	    TheMap.Tileset->TileTypeTable[tile]=TileTypeGrass;
	}
	if( (tile=table[0x070+i]) ) {		// solid forest
	    TheMap.Tileset->TileTypeTable[tile]=TileTypeWood;
	}
	if( (tile=table[0x080+i]) ) {		// solid rocks
	    TheMap.Tileset->TileTypeTable[tile]=TileTypeRock;
	}

	if( i<3 && (tile=table[0x090+i]) ) {	// solid human walls
	    TheMap.Tileset->TileTypeTable[tile]=TileTypeHumanWall;
	}
	if( i<3 && (tile=table[0x0A0+i]) ) {	// solid orc walls
	    TheMap.Tileset->TileTypeTable[tile]=TileTypeOrcWall;
	}
	if( i<3 && (tile=table[0x0B0+i]) ) {	// solid human walls
	    TheMap.Tileset->TileTypeTable[tile]=TileTypeHumanWall;
	}
	if( i<3 && (tile=table[0x0C0+i]) ) {	// solid orc walls
	    TheMap.Tileset->TileTypeTable[tile]=TileTypeOrcWall;
	}

	// 00,D0,E0,F0 Unused
    }

    //
    //	Mixed tiles
    //
    for( i=0; i<0xE0; ++i ) {
	if( (tile=table[0x100+i]) ) {		// mixed water
	    TheMap.Tileset->TileTypeTable[tile]=TileTypeWater;
	}
	if( (tile=table[0x200+i]) ) {		// mixed water/coast
	    TheMap.Tileset->TileTypeTable[tile]=TileTypeWater;
	}
	if( (tile=table[0x300+i]) ) {		// mixed coast
	    TheMap.Tileset->TileTypeTable[tile]=TileTypeCoast;
	}
	if( (tile=table[0x400+i]) ) {		// mixed rocks/coast
	    TheMap.Tileset->TileTypeTable[tile]=TileTypeRock;
	}
	if( (tile=table[0x500+i]) ) {		// mixed ground/coast
	    TheMap.Tileset->TileTypeTable[tile]=TileTypeCoast;
	}
	if( (tile=table[0x600+i]) ) {		// mixed ground
	    TheMap.Tileset->TileTypeTable[tile]=TileTypeGrass;
	}
	if( (tile=table[0x700+i]) ) {		// mixed forest/ground
	    TheMap.Tileset->TileTypeTable[tile]=TileTypeWood;
	}
	if( (((i&0xF0)==0x40 || (i&0xF0)==0x90) && (i&0xF)<5)
		|| (i&0xF)<3 ) {
	    if( (tile=table[0x800+i]) ) {	// mixed human wall
		TheMap.Tileset->TileTypeTable[tile]=TileTypeHumanWall;
	    }
	    if( (tile=table[0x900+i]) ) {	// mixed orc wall
		TheMap.Tileset->TileTypeTable[tile]=TileTypeOrcWall;
	    }
	}
    }

    //
    //	mark the special tiles
    //
    for( i=0; i<6; ++i ) {
	if( (tile=TheMap.Tileset->ExtraTrees[i]) ) {
	    TheMap.Tileset->TileTypeTable[tile]=TileTypeWood;
	}
	if( (tile=TheMap.Tileset->ExtraRocks[i]) ) {
	    TheMap.Tileset->TileTypeTable[tile]=TileTypeRock;
	}
    }
    if( (tile=TheMap.Tileset->TopOneTree) ) {
	TheMap.Tileset->TileTypeTable[tile]=TileTypeWood;
    }
    if( (tile=TheMap.Tileset->MidOneTree) ) {
	TheMap.Tileset->TileTypeTable[tile]=TileTypeWood;
    }
    if( (tile=TheMap.Tileset->BotOneTree) ) {
	TheMap.Tileset->TileTypeTable[tile]=TileTypeWood;
    }
    if( (tile=TheMap.Tileset->TopOneRock) ) {
	TheMap.Tileset->TileTypeTable[tile]=TileTypeRock;
    }
    if( (tile=TheMap.Tileset->MidOneRock) ) {
	TheMap.Tileset->TileTypeTable[tile]=TileTypeRock;
    }
    if( (tile=TheMap.Tileset->BotOneRock) ) {
	TheMap.Tileset->TileTypeTable[tile]=TileTypeRock;
    }

    //
    //	Build wood removement table. FIXME: use tileset type configuration
    //
    WoodTable[ 0]= -1;
    WoodTable[ 1]= TheMap.Tileset->BotOneTree;
    WoodTable[ 2]= -1;
    WoodTable[ 3]= table[0x710];
    WoodTable[ 4]= TheMap.Tileset->TopOneTree;
    WoodTable[ 5]= TheMap.Tileset->MidOneTree;
    WoodTable[ 6]= table[0x770];
    WoodTable[ 7]= table[0x790];
    WoodTable[ 8]= -1;
    WoodTable[ 9]= table[0x700];
    WoodTable[10]= -1;
    WoodTable[11]= table[0x720];
    WoodTable[12]= table[0x730];
    WoodTable[13]= table[0x740];
    WoodTable[14]= table[0x7B0];
    WoodTable[15]= table[0x7D0];

    //
    //	Build rock removement table. FIXME: use tileset type configuration
    //
    RockTable[ 0]= -1;
    RockTable[ 1]= TheMap.Tileset->BotOneRock;
    RockTable[ 2]= -1;
    RockTable[ 3]= table[0x410];
    RockTable[ 4]= TheMap.Tileset->TopOneRock;
    RockTable[ 5]= TheMap.Tileset->MidOneRock;
    RockTable[ 6]= table[0x470];
    RockTable[ 7]= table[0x490];
    RockTable[ 8]= -1;
    RockTable[ 9]= table[0x400];
    RockTable[10]= -1;
    RockTable[11]= table[0x420];
    RockTable[12]= table[0x430];
    RockTable[13]= table[0x440];
    RockTable[14]= table[0x4B0];
    RockTable[15]= table[0x080];

    RockTable[16]= table[0x4C0];
    RockTable[17]= table[0x460];
    RockTable[18]= table[0x4A0];
    RockTable[19]= table[0x4D0];
}

/**
**	Save solid part of tileset.
*/
local void SaveTilesetSolid(FILE* file,const unsigned short* table
	,const char* name,const char* flag,int start)
{
    int i;
    int j;
    int n;

    fprintf(file,"  'solid (list \"%s\"",name);
    if( flag && *flag ) {
	fprintf(file," %s",flag);
    }
    for( n=15; n>=0 && !table[start+n] ; n-- ) {
    }
    i=fprintf(file,"\n    #(");
    for( j=0; j<=n; ++j ) {
	i+=fprintf(file," %3d",table[start+j]);
    }
    i+=fprintf(file,"))");

    while( (i+=8)<80 ) {
	fprintf(file,"\t");
    }
    fprintf(file,"; %03X\n",start);
}

/**
**	Save mixed part of tileset.
*/
local void SaveTilesetMixed(FILE* file,const unsigned short* table
	,const char* name1,const char* name2,const char* flag,int start)
{
    int x;
    int i;
    int j;
    int n;

    if( start>=0x9E0 ) {
	return;
    }
    fprintf(file,"  'mixed (list \"%s\" \"%s\" %s\n",name1,name2,flag);
    for( x=0; x<0x100; x+=0x10 ) {
	if( start+x>=0x9E0 ) {
	    break;
	}
	fprintf(file,"    #(");
	for( n=15; n>=0 && !table[start+x+n] ; n-- ) {
	}
	i=6;
	for( j=0; j<=n; ++j ) {
	    i+=fprintf(file," %3d",table[start+x+j]);
	}
	if( x==0xF0 || (start==0x900 && x==0xD0)) {
	    i+=fprintf(file,"))");
	} else {
	    i+=fprintf(file,")");
	}

	while( (i+=8)<80 ) {
	    fprintf(file,"\t");
	}
	fprintf(file,"; %03X\n",start+x);
    }
}

/**
**	Save the tileset.
**
**	@param file	Output file.
*/
local void SaveTileset(FILE* file,const Tileset* tileset)
{
    const unsigned short* table;

    fprintf(file,"\n(define-tileset\n  '%s 'class '%s",
	    tileset->Ident,tileset->Class);
    fprintf(file,"\n  'name \"%s\"",tileset->Name);
    fprintf(file,"\n  'image \"%s\"",tileset->File);
    fprintf(file,"\n  'palette \"%s\"",tileset->PaletteFile);
    fprintf(file,"\n  ;; Slots descriptions");
    fprintf(file,"\n  'slots (list\n  'special (list\t\t;; Can't be in pud\n");
    fprintf(file,"    'extra-trees #( %d %d %d %d %d %d )\n"
	,tileset->ExtraTrees[0] ,tileset->ExtraTrees[1]
	,tileset->ExtraTrees[2] ,tileset->ExtraTrees[3]
	,tileset->ExtraTrees[4] ,tileset->ExtraTrees[5]);
    fprintf(file,"    'top-one-tree %d 'mid-one-tree %d 'bot-one-tree %d\n"
	,tileset->TopOneTree ,tileset->MidOneTree ,tileset->BotOneTree);
    fprintf(file,"    'removed-tree %d\n",tileset->RemovedTree);
    fprintf(file,"    'growing-tree #( %d %d )\n"
	,tileset->GrowingTree[0] ,tileset->GrowingTree[1]);
    fprintf(file,"    'extra-rocks #( %d %d %d %d %d %d )\n"
	,tileset->ExtraRocks[0] ,tileset->ExtraRocks[1]
	,tileset->ExtraRocks[2] ,tileset->ExtraRocks[3]
	,tileset->ExtraRocks[4] ,tileset->ExtraRocks[5]);
    fprintf(file,"    'top-one-rock %d 'mid-one-rock %d 'bot-one-rock %d\n"
	,tileset->TopOneRock ,tileset->MidOneRock ,tileset->BotOneRock);
    fprintf(file,"    'removed-rock %d )\n",tileset->RemovedRock);

    table=tileset->Table;
    //
    //	Solids
    //
    SaveTilesetSolid(file,table,"unused",		"",		0x00);
    SaveTilesetSolid(file,table,"light-water",		"'water",	0x10);
    SaveTilesetSolid(file,table,"dark-water",		"'water",	0x20);
    SaveTilesetSolid(file,table,"light-coast",		"'no-building",	0x30);
    SaveTilesetSolid(file,table,"dark-coast",		"'no-building",	0x40);
    SaveTilesetSolid(file,table,"light-grass",		"",		0x50);
    SaveTilesetSolid(file,table,"dark-grass",		"",		0x60);
    SaveTilesetSolid(file,table,"forest",		"'forest",	0x70);
    SaveTilesetSolid(file,table,"rocks",		"'rock",	0x80);
    SaveTilesetSolid(file,table,"human-closed-wall",	"'wall",	0x90);
    SaveTilesetSolid(file,table,"orc-closed-wall",	"'wall",	0xA0);
    SaveTilesetSolid(file,table,"human-open-wall",	"'wall",	0xB0);
    SaveTilesetSolid(file,table,"orc-open-wall",	"'wall",	0xC0);
    SaveTilesetSolid(file,table,"unused",		"",		0xD0);
    SaveTilesetSolid(file,table,"unused",		"",		0xE0);
    SaveTilesetSolid(file,table,"unused",		"",		0xF0);

    //
    //	Mixeds
    //
    SaveTilesetMixed(file,table,"dark-water","light-water","'water",	0x100);
    SaveTilesetMixed(file,table,"light-water","light-coast","'coast",	0x200);
    SaveTilesetMixed(file,table,"dark-coast","light-coast","'no-building",0x300);
    SaveTilesetMixed(file,table,"rocks","light-coast","'rock",		0x400);
    SaveTilesetMixed(file,table,"light-coast","light-ground","'no-building",0x500);
    SaveTilesetMixed(file,table,"dark-ground","light-ground","",	0x600);
    SaveTilesetMixed(file,table,"forest","light-ground","'forest",	0x700);
    SaveTilesetMixed(file,table,"human-wall","dark-ground","'wall",	0x800);
    SaveTilesetMixed(file,table,"orc-wall","dark-ground","'wall",	0x900);
    fprintf(file,"  )\n");
    fprintf(file,"  ;; Animated tiles\n");
    fprintf(file,"  'animations (list #( ) )\n");
    fprintf(file,"  'objects (list #( ) ))\n");
}

/**
**	Save the current tileset.
**
**	@param file	Output file.
*/
global void SaveTilesets(FILE* file)
{
    int i;
    char** sp;

    fprintf(file,"\n;;; -----------------------------------------\n");
    fprintf(file,";;; MODULE: tileset $Id$\n\n");

    //	Original number to internal unit-type name.

    i=fprintf(file,"(define-tileset-wc-names");
    for( sp=TilesetWcNames; *sp; ++sp ) {
	if( i+strlen(*sp)>79 ) {
	    i=fprintf(file,"\n ");
	}
	i+=fprintf(file," '%s",*sp);
    }
    fprintf(file,")\n");

    for( i=0; i<NumTilesets; ++i ) {
	SaveTileset(file,Tilesets[i]);
    }
}

/**
**	Cleanup the tileset module.
**
**	@note	this didn't frees the configuration memory.
*/
global void CleanTilesets(void)
{
    int i;
    char** ptr;

    //
    //	Free the tilesets
    //
    for( i=0; i<NumTilesets; ++i ) {
	free(Tilesets[i]->Ident);
	free(Tilesets[i]->Class);
	free(Tilesets[i]->Name);
	free(Tilesets[i]->File);
	free(Tilesets[i]->PaletteFile);
	free(Tilesets[i]->Table);
	free(Tilesets[i]->TileTypeTable);
	free(Tilesets[i]->AnimationTable);
	free(Tilesets[i]);
    }
    free(Tilesets);
    Tilesets=NULL;
    NumTilesets=0;

    //
    //	Should this be done by the map?
    //
    VideoFree(TheMap.TileData);
    IfDebug( TheMap.TileData=NULL; );
    free(TheMap.Tiles);
    IfDebug( TheMap.Tiles=NULL; );

    //
    //	Mapping the original tileset numbers in puds to our internal strings
    //
    if( (ptr=TilesetWcNames) ) {	// Free all old names
	while( *ptr ) {
	    free(*ptr++);
	}
	free(TilesetWcNames);

	TilesetWcNames=NULL;
    }
}

//@}
