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
/**@name pud.c		-	The pud. */
//
//	(c) Copyright 1998-2002 by Lutz Sammer
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
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
#include "map.h"
#include "player.h"
#include "settings.h"
#include "mpq.h"

#include "myendian.h"


/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

local char *scm_ptr;
local char *scm_endptr;


/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Convert scm's MTXM section into internal format.
**
**	@param mtxm	Section data
**	@param width	Section width
**	@param height	Section height
**	@param map	Map to store into
*/
local void ScmConvertMTXM(const unsigned short * mtxm,int width,int height,WorldMap* map)
{
    const unsigned short* ctab;
    int h;
    int w;

    DebugCheck( UnitTypeOrcWall->_HitPoints>=256
	    || UnitTypeHumanWall->_HitPoints>=256 );

    if( map->Terrain<TilesetMax ) {
	// FIXME: should use terrain name!!
	ctab=Tilesets[map->Terrain]->Table;
	DebugLevel0Fn("FIXME: %s <-> %s\n" _C_ Tilesets[map->Terrain]->Class _C_
		map->TerrainName);
    } else {
	DebugLevel1("Unknown terrain!\n");
	// FIXME: don't use TilesetSummer
	ctab=Tilesets[TilesetSummer]->Table;
    }

    for( h=0; h<height; ++h ) {
	for( w=0; w<width; ++w ) {
	    int v;

	    v=ConvertLE16(mtxm[h*width+w]);
	    map->Fields[w+h*TheMap.Width].Tile=ctab[v];
	    map->Fields[w+h*TheMap.Width].Value=0;
	    //
	    //	Walls are handled special (very ugly).
	    //
	    if( (v&0xFFF0)==0x00A0
		    || (v&0xFFF0)==0x00C0
		    || (v&0xFF00)==0x0900 ) {
		map->Fields[w+h*TheMap.Width].Value=
			UnitTypeOrcWall->_HitPoints;
	    } else if( (v&0x00F0)==0x0090
		    || (v&0xFFF0)==0x00B0
		    || (v&0xFF00)==0x0800 ) {
		map->Fields[w+h*TheMap.Width].Value=
			UnitTypeHumanWall->_HitPoints;
	    }
	}
    }
}

/**
**
*/
local int ScmReadHeader(char* header,long* length)
{
    long len;

    if( scm_ptr >= scm_endptr) {
	return 0;
    }
    memcpy(header, scm_ptr, 4);
    scm_ptr += 4;
    memcpy(&len, scm_ptr, 4);
    scm_ptr += 4;
    *length = ConvertLE32(len);
    return 1;
}

/**
**
*/
local int ScmReadWord(void)
{
    unsigned short temp_short;

    memcpy(&temp_short, scm_ptr, 2);
    scm_ptr += 2;
    return ConvertLE16(temp_short);
}

/**
**
*/
local int ScmReadByte(void)
{
    unsigned char temp_char;

    temp_char = *scm_ptr;
    scm_ptr += 1;
    return temp_char;
}

/**
**	Get the info for a scm level.
*/
global MapInfo* GetScmInfo(const char* scm)
{
    char *scmdata;
    long length;
    char header[5];
    char buf[1024];
    MapInfo* info;
    FILE *fpMpq;

    if( !(fpMpq=fopen(scm, "rb")) ) {
	fprintf(stderr,"Try ./path/name\n");
	sprintf(buf, "scm: fopen(%s)", scm);
	perror(buf);
	ExitFatal(-1);
    }

    if( MpqReadInfo(fpMpq) ) {
	fprintf(stderr,"MpqReadInfo failed\n");
	ExitFatal(-1);
    }

    // FIXME: not always the first entry
    scmdata = malloc(MpqBlockTable[0*4+2]+1);
    MpqExtractTo(scmdata, 0, fpMpq);

    fclose(fpMpq);

    info=calloc(1, sizeof(MapInfo));	// clears with 0

    scm_ptr = scmdata;
    scm_endptr = scm_ptr + MpqBlockTable[0*4+2];
    header[4] = '\0';

    while( ScmReadHeader(header,&length) ) {

	//
	//	SCM version
	//
	if( !memcmp(header, "VER ",4) ) {
	    if( length==2 ) {
		int v;
		v = ScmReadWord();
		// 57 - beta57
		// 59 - 1.00
		// 63 - 1.04
		// 205 - brood
		continue;
	    }
	    DebugLevel1("Wrong VER  length\n");
	}

	//
	//	SCM version additional information
	//
	if( !memcmp(header, "IVER",4) ) {
	    if( length==2 ) {
		int v;
		v = ScmReadWord();
		// 9 - obsolete, beta
		// 10 - current
		continue;
	    }
	    DebugLevel1("Wrong IVER length\n");
	}

	//
	//	Verification code
	//
	if( !memcmp(header, "VCOD",4) ) {
	    if( length==1040 ) {
		scm_ptr += 1040;
		continue;
	    }
	    DebugLevel1("Wrong VCOD length\n");
	}

	//
	//	Specifies the owner of the player
	//
	if( !memcmp(header, "IOWN",4) ) {
	    if( length==12 ) {
		scm_ptr += 12;
		continue;
	    }
	    DebugLevel1("Wrong IOWN length\n");
	}

	//
	//	Specifies the owner of the player, same as IOWN but with 0 added
	//
	if( !memcmp(header, "OWNR",4) ) {
	    if( length==12 ) {
		int i;
		int p;

		for( i=0; i<12; ++i ) {
		    p=ScmReadByte();
		    if( p==0 ) {
			info->PlayerType[i]=PlayerNobody;
		    }
		    else if( p==3 ) {
			// FIXME: should this be passive?
			info->PlayerType[i]=PlayerRescuePassive;
		    }
		    else if( p==5 ) {
			info->PlayerType[i]=PlayerComputer;
		    }
		    else if( p==6 ) {
			info->PlayerType[i]=PlayerPerson;
		    }
		    else if( p==7 ) {
			info->PlayerType[i]=PlayerNeutral;
		    }
		    else {
			DebugLevel1("Wrong OWNR type: %d\n" _C_ p);
			info->PlayerType[i]=PlayerNobody;
		    }
		}
		continue;
	    }
	    DebugLevel1("Wrong OWNR length\n");
	}

	//
	//	Terrain type
	//
	if( !memcmp(header, "ERA ",4) ) {
	    if( length==2 ) {
		int i;
		int t;
		t = ScmReadWord();
		//
		//	Look if we have this as tileset.
		//
		for( i=0; i<t && TilesetWcNames[i]; ++i ) {
		}
		if( !TilesetWcNames[i] ) {
		    t=0;
		}
		// 00 - Badlands
		// 01 - Space Platform
		// 02 - Installation
		// 03 - Ashworld
		// 04 - Jungle World
		// 05 - Desert World
		// 06 - Arctic World
		// 07 - Twilight World
		t=0;	// FIXME: remove when tilesets work
		info->MapTerrainName=strdup(TilesetWcNames[t]);
		info->MapTerrain=t;
		continue;
	    }
	    DebugLevel1("Wrong ERA  length\n");
	}

	//
	//	Dimension
	//
	if( !memcmp(header, "DIM ",4) ) {
	    if( length==4 ) {
		info->MapWidth=ScmReadWord();
		info->MapHeight=ScmReadWord();
		continue;
	    }
	    DebugLevel1("Wrong DIM  length\n");
	}

	//
	//	Identifies race of each player
	//
	if( !memcmp(header, "SIDE",4) ) {
	    if( length==12 ) {
		int i;
		int v;

		// 00 - Zerg
		// 01 - Terran
		// 02 - Protoss
		// 03 - Independent
		// 04 - Neutral
		// 05 - User Select
		// 07 - Inactive
		// 10 - Human

		for( i=0; i<12; ++i ) {
		    v=ScmReadByte();
		    info->PlayerSide[i]=v;
		}
		continue;
	    }
	    DebugLevel1("Wrong SIDE length\n");
	}

	//
	//	Graphical tile map
	//
	if( !memcmp(header, "MTXM",4) ) {
	    scm_ptr += length;
	    continue;
	}

	//
	//	Player unit restrictions
	//
	if( !memcmp(header, "PUNI",4) ) {
	    if( length==228*12 + 228 + 228*12 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong PUNI length\n");
	}

	//
	//	Player upgrade restrictions
	//
	if( !memcmp(header, "UPGR",4) ) {
	    if( length==46*12 + 46*12 + 46 + 46 + 46*12 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong UPGR length\n");
	}

	//
	//	Player technology restrictions
	//
	if( !memcmp(header, "PTEC",4) ) {
	    if( length==24*12 + 24*12 + 24 + 24 + 24*12 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong PTEC length\n");
	}

	//
	//	Units
	//
	if( !memcmp(header, "UNIT",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong UNIT length\n");
	}

	//
	//	Isometric tile mapping
	//
	if( !memcmp(header, "ISOM",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong ISOM length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "TILE",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong TILE length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "DD2 ",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong DD2  length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "THG2",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong THG2 length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "MASK",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong MASK length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "STR ",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		info->Description=strdup(scm_ptr+2051);
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong STR  length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "UPRP",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong UPRP length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "UPUS",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong UPUS length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "MRGN",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong MRGN length\n");
	}

	//
	//	Triggers
	//
	if( !memcmp(header, "TRIG",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong TRIG length\n");
	}

	//
	//	Mission briefing
	//
	if( !memcmp(header, "MBRF",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong MBRF length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "SPRP",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong SPRP length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "FORC",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong FORC length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "WAV ",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong WAV  length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "UNIS",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong UNIS length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "UPGS",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong UPGS length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "TECS",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong TECS length\n");
	}

	DebugLevel2("Unsupported Section: %4.4s\n" _C_ header);
	scm_ptr += length;
    }

    free(scmdata);

    return info;
}

/**
**
*/
global void LoadScm(const char* scm,WorldMap* map)
{
    char *scmdata;
    long length;
    char header[5];
    char buf[1024];
    int width;
    int height;
    int aiopps;
    FILE *fpMpq;

    if (!map->Info) {
	map->Info = GetScmInfo(scm);
    }

    if( !(fpMpq=fopen(scm, "rb")) ) {
	fprintf(stderr,"Try ./path/name\n");
	sprintf(buf, "scm: fopen(%s)", scm);
	perror(buf);
	ExitFatal(-1);
    }

    if( MpqReadInfo(fpMpq) ) {
	fprintf(stderr,"ReadMpqInfo failed\n");
	ExitFatal(-1);
    }

    // FIXME: not always the first entry
    scmdata = malloc(MpqBlockTable[0*4+2]+1);
    MpqExtractTo(scmdata, 0, fpMpq);

    fclose(fpMpq);

    scm_ptr = scmdata;
    scm_endptr = scm_ptr + MpqBlockTable[0*4+2];
    header[4] = '\0';
    aiopps=width=height=0;

    while( ScmReadHeader(header,&length) ) {

	//
	//	PUD version
	//
	if( !memcmp(header, "VER ",4) ) {
	    if( length==2 ) {
		int v;
		v = ScmReadWord();
		continue;
	    }
	    DebugLevel1("Wrong VER  length\n");
	}

	//
	//	PUD version additional information
	//
	if( !memcmp(header, "IVER",4) ) {
	    if( length==2 ) {
		int v;
		v = ScmReadWord();
		continue;
	    }
	    DebugLevel1("Wrong IVER length\n");
	}

	//
	//	Verification code
	//
	if( !memcmp(header, "VCOD",4) ) {
	    if( length==1040 ) {
		scm_ptr += 1040;
		continue;
	    }
	    DebugLevel1("Wrong VCOD length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "IOWN",4) ) {
	    if( length==12 ) {
		scm_ptr += 12;
		continue;
	    }
	    DebugLevel1("Wrong IOWN length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "OWNR",4) ) {
	    if( length==12 ) {
		int i;
		int p;

		for( i=0; i<12; ++i ) {
		    p=ScmReadByte();
		    if( p==0 ) {
			p=PlayerNobody;
		    }
		    else if( p==3 ) {
			p=PlayerRescuePassive;
		    }
		    else if( p==5 ) {
			p=PlayerComputer;
		    }
		    else if( p==6 ) {
			p=PlayerPerson;
		    }
		    else if( p==7 ) {
			p=PlayerNeutral;
		    }
		    else {
			DebugLevel1("Wrong OWNR type: %d\n" _C_ p);
			p=PlayerNobody;
		    }

		    // Single player games only:
		    // ARI: FIXME: convert to a preset array to share with network game code
		    if (GameSettings.Opponents != SettingsPresetMapDefault) {
			if (p == PlayerComputer) {
			    if (aiopps < GameSettings.Opponents) {
				aiopps++;
			    } else {
				p = PlayerNobody;
			    }
			}
		    }
		    // Network games only:
		    if (GameSettings.Presets[i].Type != SettingsPresetMapDefault) {
			p = GameSettings.Presets[i].Type;
		    }
		    CreatePlayer(p);
		    PlayerSetAiNum(&Players[i], 0);
		}
		for( i=12; i<PlayerMax; ++i ) {
		    p=PlayerNobody;
		    CreatePlayer(p);
		    PlayerSetAiNum(&Players[i], 0);
		}
		continue;
	    }
	    DebugLevel1("Wrong OWNR length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "ERA ",4) ) {
	    if( length==2 ) {
		int t;
		int i;

		t = ScmReadWord();
		if (GameSettings.Terrain != SettingsPresetMapDefault) {
		    t = GameSettings.Terrain;
		}
		if( map->TerrainName ) {
		    free(map->TerrainName);
		}
		//
		//	Look if we have this as tileset.
		//
		for( i=0; i<t && TilesetWcNames[i]; ++i ) {
		}
		if( !TilesetWcNames[i] ) {
		    t=0;
		}
		map->TerrainName=strdup(TilesetWcNames[t]);
		map->Terrain=t;
		continue;
	    }
	    DebugLevel1("Wrong ERA  length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "DIM ",4) ) {
	    if( length==4 ) {
		width=ScmReadWord();
		height=ScmReadWord();

		DebugLevel2("\tMap %d x %d\n" _C_ width _C_ height);

		if( !map->Fields ) {
		    map->Width=width;
		    map->Height=height;

		    map->Fields=calloc(width*height,sizeof(*map->Fields));
		    if( !map->Fields ) {
			perror("calloc()");
			ExitFatal(-1);
		    }
		    TheMap.Visible[0]=calloc(TheMap.Width*TheMap.Height/8,1);
		    if( !TheMap.Visible[0] ) {
			perror("calloc()");
			ExitFatal(-1);
		    }
		    InitUnitCache();
		    // FIXME: this should be CreateMap or InitMap?
		} else {			// FIXME: should do some checks here!
		    DebugLevel0Fn("Warning: Fields already allocated\n");
		}
		continue;
	    }
	    DebugLevel1("Wrong DIM  length\n");
	}

	//
	//	Identifies race of each player
	//
	if( !memcmp(header, "SIDE",4) ) {
	    if( length==12 ) {
		int i;
		int v;

		for( i=0; i<12; ++i ) {
		    v=ScmReadByte();
		    switch( v ) {
//			case PlayerRaceHuman:
//			case PlayerRaceOrc:
//			case PlayerRaceNeutral:
			case 0:
			case 1:
			    break;
			case 2:	// Protoss
			    v=PlayerRaceHuman;
			    break;
			case 5:
			    v=PlayerRaceHuman;
			    break;
			default:
			    DebugLevel1("Unknown race %d\n" _C_ v);
			    v=PlayerRaceNeutral;
			    break;
		    }
		    if (GameSettings.Presets[i].Race == SettingsPresetMapDefault) {
			PlayerSetSide(&Players[i],v);
		    } else {
			PlayerSetSide(&Players[i],GameSettings.Presets[i].Race);
		    }
		}
		continue;
	    }
	    DebugLevel1("Wrong SIDE length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "MTXM",4) ) {
	    if( length==width*height*2 ) {
		ScmConvertMTXM((unsigned short*)scm_ptr,width,height,map);
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong SIDE length\n");
	}

	//
	//	Player unit restrictions
	//
	if( !memcmp(header, "PUNI",4) ) {
	    if( length==228*12 + 228 + 228*12 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong PUNI length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "UPGR",4) ) {
	    // 1748
//	    if( length==45*12 + 45*12 + 45 + 45 + 45*12 ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong UPGR length\n");
	}

	//
	//	Player technology restrictions
	//
	if( !memcmp(header, "PTEC",4) ) {
	    if( length==24*12 + 24*12 + 24 + 24 + 24*12 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong PTEC length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "UNIT",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong UNIT length\n");
	}

	//
	//	Isometric tile mapping
	//
	if( !memcmp(header, "ISOM",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong ISOM length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "TILE",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong TILE length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "DD2 ",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong DD2  length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "THG2",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong THG2 length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "MASK",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong MASK length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "STR ",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		strcpy(map->Description, scm_ptr+2051);
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong STR  length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "UPRP",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong UPRP length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "UPUS",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong UPUS length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "MRGN",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong MRGN length\n");
	}

	//
	//	Triggers
	//
	if( !memcmp(header, "TRIG",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong TRIG length\n");
	}

	//
	//	Mission briefing
	//
	if( !memcmp(header, "MBRF",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong MBRF length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "SPRP",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong SPRP length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "FORC",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong FORC length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "WAV ",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong WAV  length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "UNIS",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong UNIS length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "UPGS",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong UPGS length\n");
	}

	//
	//	
	//
	if( !memcmp(header, "TECS",4) ) {
//	    if( length== ) {
	    if( 1 ) {
		scm_ptr += length;
		continue;
	    }
	    DebugLevel1("Wrong TECS length\n");
	}

	DebugLevel2("Unsupported Section: %4.4s\n" _C_ header);
	scm_ptr += length;
    }
}

