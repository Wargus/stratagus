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
/**@name construct.c	-	The constructions. */
//
//	(c) Copyright 1998-2000 by Lutz Sammer
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
#include "video.h"
#include "tileset.h"
#include "map.h"
#include "construct.h"
#include "ccl.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

#ifndef laterUSE_CCL
// FIXME: Make this configurable with CCL and than use only this.

#define DEFAULT	NULL			/// use the first slot as default

/**
**	Constructions.
*/
local Construction Constructions[] = {
{ "construction-0",			// added for bad puds they have 0
#ifdef NEW_NAMES
  { "neutral/buildings/land construction site.png",
  "tilesets/winter/neutral/buildings/land construction site.png",
  DEFAULT,
  DEFAULT },
#else
  { "land construction site (summer,wasteland).png",
  "land construction site (winter).png",
  DEFAULT,
  DEFAULT },
#endif
	64,64 },
{ "construction-1",
  { "",
  DEFAULT,
  DEFAULT,
  DEFAULT },
	0,0 },
{ "construction-2",
  { "",
  DEFAULT,
  DEFAULT,
  DEFAULT },
	0,0 },
{ "construction-3",
  { "",
  DEFAULT,
  DEFAULT,
  DEFAULT },
	0,0 },
{ "construction-4",
  { "",
  DEFAULT,
  DEFAULT,
  DEFAULT },
	0,0 },
{ "construction-5",
  { "",
  DEFAULT,
  DEFAULT,
  DEFAULT },
	0,0 },
{ "construction-land",
#ifdef NEW_NAMES
  { "neutral/buildings/land construction site.png",
  "tilesets/winter/neutral/buildings/land construction site.png",
  DEFAULT,
  DEFAULT },
#else
  { "land construction site (summer,wasteland).png",
  "land construction site (winter).png",
  DEFAULT,
  DEFAULT },
#endif
	64,64 },
{ "construction-human-shipyard",
#ifdef NEW_NAMES
  { "human/buildings/shipyard construction site.png",
  "tilesets/winter/human/buildings/shipyard construction site.png",
  DEFAULT,
  "tilesets/swamp/human/buildings/shipyard construction site.png" },
#else
  { "human shipyard construction site (summer,wasteland).png",
  "human shipyard construction site (winter).png",
  DEFAULT,
  "human shipyard construction site (swamp).png" },
#endif
	96,96 },
{ "construction-orc-shipyard",
#ifdef NEW_NAMES
  { "orc/buildings/shipyard construction site.png",
  "tilesets/winter/orc/buildings/shipyard construction site.png",
  DEFAULT,
  "tilesets/swamp/orc/buildings/shipyard construction site.png" },
#else
  { "orc shipyard construction site (summer,wasteland).png",
  "orc shipyard construction site (winter).png",
  DEFAULT,
  "orc shipyard construction site (swamp).png" },
#endif
	96,96 },
{ "construction-human-oil-well",
#ifdef NEW_NAMES
  { "tilesets/summer/human/buildings/oil well construction site.png",
  "tilesets/winter/human/buildings/oil well construction site.png",
  "tilesets/wasteland/human/buildings/oil well construction site.png",
  "tilesets/swamp/human/buildings/oil well construction site.png" },
#else
  { "human oil well construction site (summer).png",
  "human oil well construction site (winter).png",
  "human oil well construction site (wasteland).png",
  "human oil well construction site (swamp).png" },
#endif
	96,96 },
{ "construction-orc-oil-well",
#ifdef NEW_NAMES
  { "tilesets/summer/orc/buildings/oil well construction site.png",
  "tilesets/winter/orc/buildings/oil well construction site.png",
  "tilesets/wasteland/orc/buildings/oil well construction site.png",
  "tilesets/swamp/orc/buildings/oil well construction site.png" },
#else
  { "orc oil well construction site (summer).png",
  "orc oil well construction site (winter).png",
  "orc oil well construction site (wasteland).png",
  "orc oil well construction site (swamp).png" },
#endif
	96,96 },
{ "construction-human-refinery",
#ifdef NEW_NAMES
  { "human/buildings/refinery construction site.png",
  "tilesets/winter/human/buildings/refinery construction site.png",
  DEFAULT,
  "tilesets/swamp/human/buildings/refinery construction site.png" },
#else
  { "human refinery construction site (summer,wasteland).png",
  "human refinery construction site (winter).png",
  DEFAULT,
  "human refinery construction site (swamp).png" },
#endif
	96,96 },
{ "construction-orc-refinery",
#ifdef NEW_NAMES
  { "orc/buildings/refinery construction site.png",
  "tilesets/winter/orc/buildings/refinery construction site.png",
  DEFAULT,
  "tilesets/swamp/orc/buildings/refinery construction site.png" },
#else
  { "orc refinery construction site (summer,wasteland).png",
  "orc refinery construction site (winter).png",
  DEFAULT,
  "orc refinery construction site (swamp).png" },
#endif
	96,96 },
{ "construction-human-foundry",
#ifdef NEW_NAMES
  { "human/buildings/foundry construction site.png",
  "tilesets/winter/human/buildings/foundry construction site.png",
  DEFAULT,
  "tilesets/swamp/human/buildings/foundry construction site.png" },
#else
  { "human foundry construction site (summer,wasteland).png",
  "human foundry construction site (winter).png",
  DEFAULT,
  "human foundry construction site (swamp).png" },
#endif
	96,96 },
{ "construction-orc-foundry",
#ifdef NEW_NAMES
  { "orc/buildings/foundry construction site.png",
  "tilesets/winter/orc/buildings/foundry construction site.png",
  DEFAULT,
  "tilesets/swamp/orc/buildings/foundry construction site.png" },
#else
  { "orc foundry construction site (summer,wasteland).png",
  "orc foundry construction site (winter).png",
  DEFAULT,
  "orc foundry construction site (swamp).png" },
#endif
	96,96 },
{ "construction-wall",
#ifdef NEW_NAMES
  { "tilesets/summer/neutral/buildings/wall construction site.png",
  "tilesets/winter/neutral/buildings/wall construction site.png",
  "tilesets/wasteland/neutral/buildings/wall construction site.png",
  DEFAULT },
#else
  { "wall construction site (summer).png",
  "wall construction site (winter).png",
  "wall construction site (wasteland).png",
  DEFAULT },
#endif
	32,32 },

{ }
};

#endif

/**
**	Table mapping the original construction numbers in puds to
**	our internal string.
*/
global char** ConstructionWcNames;

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Initialize  the constructions.
*/
global void InitConstructions(void)
{
}

/**
**	Load the graphics for the constructions.
**
**	HELPME:	who make this better terrain depended and extendable
**	HELPME: filename constuction.
*/
global void LoadConstructions(void)
{
    int i;
    const char* file;

    for( i=0; i<sizeof(Constructions)/sizeof(*Constructions)-1; ++i ) {
	file=Constructions[i].File[TheMap.Terrain];
	if( !file ) {			// default one
	    file=Constructions[i].File[0];
	}
	if( *file ) {
	    char* buf;

	    buf=alloca(strlen(file)+9+1);
#ifdef NEW_NAMES
	    file=strcat(strcpy(buf,"graphics/"),file);
#else
	    file=strcat(strcpy(buf,"graphic/"),file);
#endif
	    ShowLoadProgress("\tConstruction %s\n",file);
	    Constructions[i].Sprite=LoadSprite(file
		    ,Constructions[i].Width,Constructions[i].Height);
	}
    }
}

/**
**	Save state of constructions to file.
**
**	@param file	Output file.
*/
global void SaveConstructions(FILE* file)
{
    int i;
    int j;
    char** cp;

    fprintf(file,"\n;;; -----------------------------------------\n");
    fprintf(file,";;; MODULE: constructions $Id$\n\n");

    //
    //	Dump table wc2 construction numbers -> internal symbol.
    //
    if( (cp=ConstructionWcNames) ) {
	fprintf(file,"(define-construction-wc-names");

	i=90;
	while( *cp ) {
	    if( i+strlen(*cp)>79 ) {
		i=fprintf(file,"\n ");
	    }
	    i+=fprintf(file," '%s",*cp++);
	}
	fprintf(file,")\n\n");
    }

    //
    //	Dump table of all constructions
    //
    for( i=0; i<sizeof(Constructions)/sizeof(*Constructions)-1; ++i ) {
	fprintf(file,"(define-construction '%s\n",Constructions[i].Ident);
	fprintf(file,"  'files '(");
	for( j=0; j<TilesetMax; ++j ) {
	    if( Constructions[i].File[j] ) {
		if( j ) {
		    fputs("\n    ",file);
		}
		fprintf(file,"%s \"%s\"",Tilesets[j].Name,
			Constructions[i].File[j]);
	    }
	}
	fprintf(file,")\n");
	fprintf(file,"  'size '(%d %d))\n\n", Constructions[i].Width,
		Constructions[i].Height);
    }
}

/**
**	Cleanup the constructions.
*/
global void CleanConstructions(void)
{
    char** cp;

    DebugLevel0Fn("FIXME: not written\n");

    //
    //	Mapping original construction numbers in puds to our internal strings
    //
    if( (cp=ConstructionWcNames) ) {	// Free all old names
	while( *cp ) {
	    free(*cp++);
	}
	free(ConstructionWcNames);
	ConstructionWcNames=NULL;
    }
}

/**
**	Draw construction.
**
**	@param type	Type number of construction.
**	@param frame	Frame number to draw.
**	@param x	X position.
**	@param y	Y position.
*/
global void DrawConstruction(int type,int frame,int x,int y)
{
    // FIXME: This should be moved to caller/init...
    x-=Constructions[type].Width/2;
    y-=Constructions[type].Height/2;

    VideoDrawClip(Constructions[type].Sprite,frame,x,y);
}

// ----------------------------------------------------------------------------

/**
**	Define construction mapping from original number to internal symbol
**
**	@param list	List of all names.
*/
local SCM CclDefineConstructionWcNames(SCM list)
{
    int i;
    char** cp;

    if( (cp=ConstructionWcNames) ) {		// Free all old names
	while( *cp ) {
	    free(*cp++);
	}
	free(ConstructionWcNames);
    }

    //
    //	Get new table.
    //
    i=gh_length(list);
    ConstructionWcNames=cp=malloc((i+1)*sizeof(char*));
    while( i-- ) {
	*cp++=gh_scm2newstr(gh_car(list),NULL);
	list=gh_cdr(list);
    }
    *cp=NULL;

    return SCM_UNSPECIFIED;
}

/**
**	Parse the construction.
**
**	@param list	List describing the construction.
*/
local SCM CclDefineConstruction(SCM list)
{
#if 0
    SCM value;
    char* str;
    MissileType* mtype;
    unsigned i;

    //	Slot identifier

    str=gh_scm2newstr(gh_car(list),NULL);
    list=gh_cdr(list);
    IfDebug( i=NoWarningMissileType; NoWarningMissileType=1; );
    mtype=MissileTypeByIdent(str);
    IfDebug( NoWarningMissileType=i; );
    if( mtype ) {
	DebugLevel0Fn("Redefining missile-type `%s'\n",str);
	CclFree(str);
    } else {
	mtype=NewMissileTypeSlot(str);	// str consumed!
    }

    //
    //	Parse the arguments, already the new tagged format.
    //
    while( !gh_null_p(list) ) {
	value=gh_car(list);
	list=gh_cdr(list);
	if( gh_eq_p(value,gh_symbol2scm("file")) ) {
	    CclFree(mtype->File);
	    mtype->File=gh_scm2newstr(gh_car(list),NULL);
	} else if( gh_eq_p(value,gh_symbol2scm("size")) ) {
	    value=gh_car(list);
	    mtype->Width=gh_scm2int(gh_car(value));
	    value=gh_cdr(value);
	    mtype->Height=gh_scm2int(gh_car(value));
	} else if( gh_eq_p(value,gh_symbol2scm("frames")) ) {
	    mtype->Frames=gh_scm2int(gh_car(list));
	} else if( gh_eq_p(value,gh_symbol2scm("fired-sound")) ) {
	    CclFree(mtype->FiredSound.Name);
	    mtype->FiredSound.Name=gh_scm2newstr(gh_car(list),NULL);
	} else if( gh_eq_p(value,gh_symbol2scm("impact-sound")) ) {
	    CclFree(mtype->ImpactSound.Name);
	    mtype->ImpactSound.Name=gh_scm2newstr(gh_car(list),NULL);
	} else if( gh_eq_p(value,gh_symbol2scm("class")) ) {
	    value=gh_car(list);
	    for( i=0; MissileClassNames[i]; ++i ) {
		if( gh_eq_p(value,
			    gh_symbol2scm((char*)MissileClassNames[i])) ) {
		    mtype->Class=i;
		    break;
		}
	    }
	    if( !MissileClassNames[i] ) {
		// FIXME: this leaves a half initialized missile-type
		errl("Unsupported class",value);
	    }
	} else if( gh_eq_p(value,gh_symbol2scm("delay")) ) {
	    mtype->Delay=gh_scm2int(gh_car(list));
	} else if( gh_eq_p(value,gh_symbol2scm("sleep")) ) {
	    mtype->Sleep=gh_scm2int(gh_car(list));
	} else if( gh_eq_p(value,gh_symbol2scm("speed")) ) {
	    mtype->Speed=gh_scm2int(gh_car(list));
	} else if( gh_eq_p(value,gh_symbol2scm("range")) ) {
	    mtype->Range=gh_scm2int(gh_car(list));
	} else if( gh_eq_p(value,gh_symbol2scm("impact-missile")) ) {
	    CclFree(mtype->ImpactName);
	    mtype->ImpactName=gh_scm2newstr(gh_car(list),NULL);
	} else {
	    // FIXME: this leaves a half initialized missile-type
	    errl("Unsupported tag",value);
	}
	list=gh_cdr(list);
    }
#endif

    return SCM_UNSPECIFIED;
}

// ----------------------------------------------------------------------------

/**
**	Register CCL features for construction.
*/
global void ConstructionCclRegister(void)
{
    gh_new_procedureN("define-construction-wc-names",
	    CclDefineConstructionWcNames);
    gh_new_procedureN("define-construction",CclDefineConstruction);

}
//@}
