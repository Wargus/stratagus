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
/**@name construct.c	-	The constructions. */
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
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "video.h"
#include "tileset.h"
#include "map.h"
#include "construct.h"
#include "ccl.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/**
**	Construction type definition
*/
global const char ConstructionType[] = "construction";

/**
**	Constructions.
*/
local Construction** Constructions;

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
    const char* file;
    Construction** cop;

    if ((cop = Constructions)) {
	while (*cop) {
	    if (!(*cop)->Ident) {
		continue;
	    }
	    file = (*cop)->File[TheMap.Terrain].File;
	    if (file) {			// default one
		(*cop)->Width = (*cop)->File[TheMap.Terrain].Width;
		(*cop)->Height = (*cop)->File[TheMap.Terrain].Height;
	    } else {
		file = (*cop)->File[0].File;
		(*cop)->Width = (*cop)->File[0].Width;
		(*cop)->Height = (*cop)->File[0].Height;
	    }
	    if (file && *file) {
		char* buf;

		buf = alloca(strlen(file) + 9 + 1);
		file = strcat(strcpy(buf, "graphics/"), file);
		ShowLoadProgress("Construction %s", file);
		(*cop)->Sprite = LoadSprite(file,
		    (*cop)->Width, (*cop)->Height);
	    }
	    file = (*cop)->ShadowFile[TheMap.Terrain].File;
	    if (file) {
		(*cop)->ShadowWidth = (*cop)->ShadowFile[TheMap.Terrain].Width;
		(*cop)->ShadowHeight = (*cop)->ShadowFile[TheMap.Terrain].Height;
	    } else {
		file = (*cop)->ShadowFile[0].File;
		(*cop)->ShadowWidth = (*cop)->ShadowFile[0].Width;
		(*cop)->ShadowHeight = (*cop)->ShadowFile[0].Height;
	    }
	    if (file && *file) {
		char* buf;

		buf = alloca(strlen(file) + 9 + 1);
		file = strcat(strcpy(buf, "graphics/"), file);
		ShowLoadProgress("Construction %s", file);
		(*cop)->ShadowSprite = LoadSprite(file,
		    (*cop)->ShadowWidth, (*cop)->ShadowHeight);
	    }
	    ++cop;
	}
    }
}

/**
**	Save state of constructions to file.
**
**	@param file	Output file.
*/
global void SaveConstructions(CLFile* file)
{
    int j;
    int i;
    char** cp;
    Construction** cop;
    ConstructionFrame* cframe;

    CLprintf(file, "\n;;; -----------------------------------------\n");
    CLprintf(file, ";;; MODULE: constructions $Id$\n\n");

    // FIXME: needed?
    
    //
    //	Dump table wc2 construction numbers -> internal symbol.
    //
    if ((cp = ConstructionWcNames)) {
	CLprintf(file, "(define-construction-wc-names");

	i = 90;
	while (*cp) {
	    if (i + strlen(*cp) > 79) {
		i = CLprintf(file, "\n ");
	    }
	    i += CLprintf(file, " '%s", *cp++);
	}
	CLprintf(file, ")\n\n");
    }

    //
    //	Dump table of all constructions
    //
    if ((cop = Constructions)) {
	while (*cop) {
	    if (!(*cop)->Ident) {
		continue;
	    }
	    CLprintf(file, "(define-construction '%s\n", (*cop)->Ident);
	    for (j = 0; j < TilesetMax; ++j) {
		if (!(*cop)->File[j].File) {
		    continue;
		}
		CLprintf(file, "  'file '(\n");
		CLprintf(file, "    tileset %s\n", Tilesets[j]->Class);
		CLprintf(file, "    file  \"%s\"\n", (*cop)->File[j].File);
		CLprintf(file, "    size (%d %d))\n", (*cop)->File[j].Width,
		    (*cop)->File[j].Height);
	    }
	    for (j = 0; j < TilesetMax; ++j) {
		if (!(*cop)->ShadowFile[j].File) {
		    continue;
		}
		CLprintf(file, "  'shadow-file '(\n");
		CLprintf(file, "    tileset %s\n", Tilesets[j]->Class);
		CLprintf(file, "    file  \"%s\"\n", (*cop)->ShadowFile[j].File);
		CLprintf(file, "    size (%d %d))\n", (*cop)->ShadowFile[j].Width,
		    (*cop)->ShadowFile[j].Height);
	    }
	    cframe = (*cop)->Frames;
	    if (cframe) {
		CLprintf(file, "  'constructions (list");
		while (cframe) {
		    CLprintf(file, "\n    '(percent %d\n", cframe->Percent);
		    if (cframe->File == ConstructionFileConstruction) {
			CLprintf(file, "      file construction\n");
		    } else {
			CLprintf(file, "      file main\n");
		    }
		    CLprintf(file, "      frame %d)",cframe->Frame);
		    cframe = cframe->Next;
		}
		CLprintf(file, ")\n");
	    }
	    CLprintf(file, ")\n\n");
	    ++cop;
	}
    }
}

/**
**	Cleanup the constructions.
*/
global void CleanConstructions(void)
{
    char** cp;
    int j;
    Construction** cop;
    ConstructionFrame* cframe;
    ConstructionFrame* tmp;

    //
    //	Mapping original construction numbers in puds to our internal strings
    //
    if ((cp = ConstructionWcNames)) {	// Free all old names
	while (*cp) {
	    free(*cp++);
	}
	free(ConstructionWcNames);
	ConstructionWcNames = NULL;
    }

    //
    //	Free the construction table.
    //
    if ((cop = Constructions)) {
	while (*cop) {
	    if ((*cop)->Ident) {
		free((*cop)->Ident);
	    }
	    for (j = 0; j < TilesetMax; ++j) {
		if ((*cop)->File[j].File) {
		    free((*cop)->File[j].File);
		}
	    }
	    VideoSaveFree((*cop)->Sprite);
	    for (j = 0; j < TilesetMax; ++j) {
		if ((*cop)->ShadowFile[j].File) {
		    free((*cop)->ShadowFile[j].File);
		}
	    }
	    VideoSaveFree((*cop)->ShadowSprite);
	    cframe = (*cop)->Frames;
	    while (cframe) {
		tmp = cframe->Next;
		free(cframe);
		cframe = tmp;
	    }
	    free(*cop);
	    ++cop;
	}
	free(Constructions);
	Constructions = NULL;
    }
}

/**
**	Get construction by identifier.
**
**	@param ident	Identfier of the construction
**
**	@return		Construction structure pointer
*/
global Construction* ConstructionByIdent(const char* ident)
{
    Construction** cop;

    if ((cop = Constructions)) {
	while (*cop) {
	    if ((*cop)->Ident && !strcmp(ident, (*cop)->Ident)) {
		return *cop;
	    }
	    ++cop;
	}
    }
    DebugLevel0Fn("Construction `%s' not found.\n" _C_ ident);
    return NULL;
}

/**
**	Get construction by original wc number.
**
**	@param num	Original number used in puds.
*/
global Construction* ConstructionByWcNum(int num)
{
    return ConstructionByIdent(ConstructionWcNames[num]);
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

    if ((cp = ConstructionWcNames)) {		// Free all old names
	while (*cp) {
	    free(*cp++);
	}
	free(ConstructionWcNames);
    }

    //
    //	Get new table.
    //
    i = gh_length(list);
    ConstructionWcNames = cp = malloc((i + 1) * sizeof(char*));
    while (i--) {
	*cp++ = gh_scm2newstr(gh_car(list), NULL);
	list = gh_cdr(list);
    }
    *cp = NULL;

    return SCM_UNSPECIFIED;
}

/**
**	Parse the construction.
**
**	@param list	List describing the construction.
**
**	@note make this more flexible
*/
local SCM CclDefineConstruction(SCM list)
{
    SCM value;
    SCM sublist;
    char* str;
    Construction* construction;
    Construction** cop;
    int i;

    //	Slot identifier

    str = gh_scm2newstr(gh_car(list), NULL);
    list = gh_cdr(list);

    if ((cop = Constructions) == NULL) {
	Constructions = malloc(2 * sizeof(Construction*));
	Constructions[0] = calloc(1, sizeof(Construction));
	Constructions[1] = NULL;
	construction = Constructions[0];
    } else {
	for (i = 0; *cop; ++i, ++cop) {
	}
	Constructions = realloc(Constructions, (i + 2) * sizeof(Construction*));
	Constructions[i] = calloc(1, sizeof(Construction));
	Constructions[i+1] = NULL;
	construction = Constructions[i];
    }
    construction->OType = ConstructionType;
    construction->Ident = str;

    //
    //	Parse the arguments, in tagged format.
    //
    while (!gh_null_p(list)) {
	int files;

	value = gh_car(list);
	list = gh_cdr(list);

	if ((files = gh_eq_p(value, gh_symbol2scm("file"))) ||
		gh_eq_p(value, gh_symbol2scm("shadow-file"))) {
	    int tileset;
	    char* file;
	    int w;
	    int h;

	    tileset = 0;
	    file = NULL;
	    w = 0;
	    h = 0;

	    sublist = gh_car(list);
	    while (!gh_null_p(sublist)) {
		value = gh_car(sublist);
		sublist = gh_cdr(sublist);

		if (gh_eq_p(value, gh_symbol2scm("tileset"))) {
		    str = gh_scm2newstr(gh_car(sublist), NULL);
		    sublist = gh_cdr(sublist);

		    // FIXME: use a general get tileset function here!
		    i = 0;
		    if (strcmp(str, "default")) {
			for (; i < NumTilesets; ++i) {
			    if (!strcmp(str,Tilesets[i]->Ident)) {
				break;
			    }
			    if (!strcmp(str,Tilesets[i]->Class)) {
				break;
			    }
			}
			if (i == NumTilesets) {
			    fprintf(stderr, "Tileset `%s' not available\n", str);
			    errl("tileset not available", gh_car(sublist));
			}
		    }
		    tileset = i;
		    free(str);
		} else if (gh_eq_p(value, gh_symbol2scm("file"))) {
		    file = gh_scm2newstr(gh_car(sublist), NULL);
		    sublist = gh_cdr(sublist);
		} else if (gh_eq_p(value, gh_symbol2scm("size"))) {
		    value = gh_car(sublist);
		    sublist = gh_cdr(sublist);
		    w = gh_scm2int(gh_car(value));
		    value = gh_cdr(value);
		    h = gh_scm2int(gh_car(value));
		} else {
		    errl("Unsupported tag", value);
		}
	    }
	    if (files) {
		free(construction->File[tileset].File);
		construction->File[tileset].File = file;
		construction->File[tileset].Width = w;
		construction->File[tileset].Height = h;
	    } else {
		free(construction->ShadowFile[tileset].File);
		construction->ShadowFile[tileset].File = file;
		construction->ShadowFile[tileset].Width = w;
		construction->ShadowFile[tileset].Height = h;
	    }
	} else if (gh_eq_p(value, gh_symbol2scm("constructions"))) {
	    sublist = gh_car(list);
	    while (!gh_null_p(sublist)) {
		SCM slist;
		int percent;
		int file;
		int frame;
		ConstructionFrame** cframe;

		percent = 0;
		file = 0;
		frame = 0;

		slist = gh_car(sublist);
		sublist = gh_cdr(sublist);
		while (!gh_null_p(slist)) {
		    value = gh_car(slist);
		    slist = gh_cdr(slist);

		    if (gh_eq_p(value, gh_symbol2scm("percent"))) {
			percent = gh_scm2int(gh_car(slist));
			slist = gh_cdr(slist);
		    } else if (gh_eq_p(value, gh_symbol2scm("file"))) {
			value = gh_car(slist);
			if (gh_eq_p(value, gh_symbol2scm("construction"))) {
			    file = ConstructionFileConstruction;
			} else if (gh_eq_p(value, gh_symbol2scm("main"))) {
			    file = ConstructionFileMain;
			} else {
			    errl("Unsupported tag", value);
			}
			slist = gh_cdr(slist);
		    } else if (gh_eq_p(value, gh_symbol2scm("frame"))) {
			frame = gh_scm2int(gh_car(slist));
			slist = gh_cdr(slist);
		    } else {
			errl("Unsupported tag", value);
		    }
		}
		cframe = &construction->Frames;
		while (*cframe) {
		    cframe = &((*cframe)->Next);
		}
		(*cframe) = malloc(sizeof(ConstructionFrame));
		(*cframe)->Percent = percent;
		(*cframe)->File = file;
		(*cframe)->Frame = frame;
		(*cframe)->Next = NULL;
	    }
	} else {
	    // FIXME: this leaves a half initialized construction
	    errl("Unsupported tag", value);
	}
	list = gh_cdr(list);
    }

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
    gh_new_procedureN("define-construction", CclDefineConstruction);

}
//@}
