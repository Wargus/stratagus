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
/**@name icons.c	-	The icons. */
//
//	(c) Copyright 1998-2001 by Lutz Sammer
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
#include "video.h"
#include "icons.h"
#include "player.h"
#include "ccl.h"

#include "etlib/hash.h"

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

#ifndef USE_CCL

/**
**	Unit icons
*/
local struct _icons_files_ {
    char*	File[TilesetMax];	/// file name
    unsigned	Width;			/// icon width
    unsigned	Height;			/// icon height
} IconsFiles[] = {
#ifdef NEW_NAMES
    { {  "tilesets/summer/icons.png"
	,"tilesets/winter/icons.png"
	,"tilesets/wasteland/icons.png"
	,"tilesets/swamp/icons.png" }
		, ICON_WIDTH, ICON_HEIGHT },
#else
    { {  "icons (summer).png"
	,"icons (winter).png"
	,"icons (wasteland).png"
	,"icons (swamp).png" }
		, ICON_WIDTH, ICON_HEIGHT },
#endif
};

/**
**	Default without CCL support.
**	Table mapping the original icon numbers in puds to our internal string.
*/
local char* DefaultIconWcNames[] = {
    "icon-peasant",
    "icon-peon",
    "icon-footman",
    "icon-grunt",
    "icon-archer",
    "icon-axethrower",
    "icon-ranger",
    "icon-berserker",
    "icon-knight",
    "icon-ogre",
    "icon-paladin",
    "icon-ogre-mage",
    "icon-dwarves",
    "icon-goblin-sappers",
    "icon-mage",
    "icon-death-knight",
    "icon-ballista",
    "icon-catapult",
    "icon-human-oil-tanker",
    "icon-orc-oil-tanker",
    "icon-human-transport",
    "icon-orc-transport",
    "icon-elven-destroyer",
    "icon-troll-destroyer",
    "icon-battleship",
    "icon-ogre-juggernaught",
    "icon-gnomish-submarine",
    "icon-giant-turtle",
    "icon-gnomish-flying-machine",
    "icon-goblin-zeppelin",
    "icon-gryphon-rider",
    "icon-dragon",
    "icon-lothar",
    "icon-gul-dan",
    "icon-uther-lightbringer",
    "icon-zuljin",
    "icon-cho-gall",
    "icon-daemon",
    "icon-farm",
    "icon-pig-farm",
    "icon-town-hall",
    "icon-great-hall",
    "icon-human-barracks",
    "icon-orc-barracks",
    "icon-elven-lumber-mill",
    "icon-troll-lumber-mill",
    "icon-human-blacksmith",
    "icon-orc-blacksmith",
    "icon-human-shipyard",
    "icon-orc-shipyard",
    "icon-human-refinery",
    "icon-orc-refinery",
    "icon-human-foundry",
    "icon-orc-foundry",
    "icon-human-oil-platform",
    "icon-orc-oil-platform",
    "icon-stables",
    "icon-ogre-mound",
    "icon-gnomish-inventor",
    "icon-goblin-alchemist",
    "icon-human-watch-tower",
    "icon-orc-watch-tower",
    "icon-church",
    "icon-altar-of-storms",
    "icon-mage-tower",
    "icon-temple-of-the-damned",
    "icon-keep",
    "icon-stronghold",
    "icon-castle-upgrade",
    "icon-fortress-upgrade",
    "icon-castle",
    "icon-fortress",
    "icon-gryphon-aviary",
    "icon-dragon-roost",
    "icon-gold-mine",
    "icon-human-guard-tower",
    "icon-human-cannon-tower",
    "icon-orc-guard-tower",
    "icon-orc-cannon-tower",
    "icon-oil-patch",
    "icon-dark-portal",
    "icon-circle-of-power",
    "icon-runestone",
    "icon-move-peasant",
    "icon-move-peon",
    "icon-repair",
    "icon-harvest",
    "icon-build-basic",
    "icon-build-advanced",
    "icon-return-goods-peasant",
    "icon-return-goods-peon",
    "icon-cancel",
    "icon-human-wall",
    "icon-orc-wall",
    "icon-slow",
    "icon-invisibility",
    "icon-haste",
    "icon-runes",
    "icon-unholy-armor",
    "icon-lightning",
    "icon-flame-shield", // 100
    "icon-fireball",
    "icon-touch-of-darkness",
    "icon-death-coil",
    "icon-whirlwind",
    "icon-blizzard",
    "icon-holy-vision",
    "icon-heal",
    "icon-death-and-decay",
    "icon-109",
    "icon-exorcism",
    "icon-eye-of-kilrogg",
    "icon-bloodlust",
    "icon-unknown113",
    "icon-skeleton",		// RaiseDead and Skeleton are equal
    "icon-critter",		// Polymorph and Critter are equal
    "icon-sword1",
    "icon-sword2",
    "icon-sword3",
    "icon-battle-axe1",
    "icon-battle-axe2",
    "icon-battle-axe3",
    "icon-122",
    "icon-123",
    "icon-arrow1",
    "icon-arrow2",
    "icon-arrow3",
    "icon-throwing-axe1",
    "icon-throwing-axe2",
    "icon-throwing-axe3",
    "icon-horse1",
    "icon-horse2",
    "icon-longbow",
    "icon-ranger-scouting",
    "icon-ranger-marksmanship",
    "icon-light-axes",
    "icon-berserker-scouting",
    "icon-berserker-regeneration",
    "icon-catapult1",
    "icon-catapult2",
    "icon-ballista1",
    "icon-ballista2",
    "icon-human-demolish",
    "icon-orc-demolish",
    "icon-human-ship-cannon1",
    "icon-human-ship-cannon2",
    "icon-human-ship-cannon3",
    "icon-orc-ship-cannon1",
    "icon-orc-ship-cannon2",
    "icon-orc-ship-cannon3",
    "icon-orc-ship-armor1",
    "icon-orc-ship-armor2",
    "icon-orc-ship-armor3",
    "icon-human-ship-armor1",
    "icon-human-ship-armor2",
    "icon-human-ship-armor3",
    "icon-orc-ship-move",
    "icon-human-ship-move",
    "icon-orc-ship-return-oil",
    "icon-human-ship-return-oil",
    "icon-orc-ship-haul-oil",
    "icon-human-ship-haul-oil",
    "icon-human-unload",
    "icon-orc-unload",
    "icon-human-shield1",
    "icon-human-shield2",
    "icon-human-shield3",
    "icon-orc-shield1",
    "icon-orc-shield2",
    "icon-orc-shield3",
    "icon-170",
    "icon-171",
    "icon-172",
    "icon-173",
    "icon-174",
    "icon-175",
    "icon-176",
    "icon-177",
    "icon-human-patrol-land",
    "icon-orc-patrol-land",
    "icon-human-stand-ground",
    "icon-orc-stand-ground",
    "icon-human-attack-ground",
    "icon-orc-attack-ground",
    "icon-human-patrol-naval",
    "icon-orc-patrol-naval",
    "icon-korgath-bladefist",
    "icon-alleria",
    "icon-danath",
    "icon-teron-gorefiend",
    "icon-grom-hellscream",
    "icon-kurdan-and-sky-ree",
    "icon-deathwing",
    "icon-khadgar",
    "icon-dentarg",
    "icon-turalyon",

    NULL
};

/**
**	Table aliases for icons.
*/
local const char* DefaultIconAliases[][2] = {
    { "icon-raise-dead",	"icon-skeleton" },
    { "icon-polymorph",		"icon-critter" },
};

#endif

/**
**	Maps the original icon numbers in puds to our internal strings.
*/
global char** IconWcNames
#ifndef USE_CCL
    = DefaultIconWcNames
#endif
    ;

local Icon** Icons;			/// Table of all icons.
local int IconsCount;			/// Number of icons in Icons.

local char** IconAliases;		/// Table of all aliases for icons.
local int IconAliasesCount;		/// Number of icons aliases in Aliases.

#ifdef DOXYGEN

local IconFile* IconFileHash[31];	/// lookup table for icon file names

local Icon* IconHash[61];		/// lookup table for icon names

#else

local hashtable(IconFile*,31) IconFileHash;/// lookup table for icon file names

local hashtable(Icon*,61) IconHash;	/// lookup table for icon names

#endif

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	@brief Add an icon definition.
**
**	@bug Redefining an icon isn't supported.
**
**	@param ident	Icon identifier.
**	@param tileset	Tileset identifier.
**	@param index	Index into file.
**	@param file	Graphic file containing the icons.
*/
local void AddIcon(char* ident,char* tileset,int index,char* file)
{
    void** ptr;
    char* str;
    IconFile* iconfile;
    Icon* icon;

    ptr=hash_find(IconFileHash,file);
    if( ptr ) {
	iconfile=*ptr;
    } else {				// new file
	iconfile=malloc(sizeof(IconFile));
	iconfile->FileName=strdup(file);
	iconfile->Width=ICON_WIDTH;
	iconfile->Height=ICON_HEIGHT;

	iconfile->Icons=0;

	iconfile->Graphic=NULL;
	*(IconFile**)hash_add(IconFileHash,iconfile->FileName)=iconfile;
    }
    str=strdcat(ident,tileset);
    ptr=hash_find(IconHash,str);
    if( ptr ) {
	DebugLevel0Fn("FIXME: Icon already defined `%s'\n",str);
	free(str);
	return;
    } else {
	icon=malloc(sizeof(Icon));
	icon->Ident=strdup(ident);
	icon->Tileset=strdup(tileset);
	icon->File=iconfile;
	icon->Index=index;
	icon->X=(index%5)*iconfile->Width;
	icon->Y=(index/5)*iconfile->Height;
	icon->Width=iconfile->Width;
	icon->Height=iconfile->Height;

	icon->Graphic=NULL;
	*(Icon**)hash_add(IconHash,str)=icon;
    }
    Icons=realloc(Icons,sizeof(Icon*)*(IconsCount+1));
    Icons[IconsCount++]=icon;
}

/**
**	Init the icons.
**
**	Add the short name and icon aliases to hash table.
*/
global void InitIcons(void)
{
    unsigned i;
    static int done;

    DebugCheck( done );
    done=1;

    DebugLevel0Fn("Init icons %s\n",TheMap.Tileset->Ident);

#ifndef USE_CCL
    //
    //	Add icons name to hash table
    //
    for( i=0; IconWcNames[i]; ++i ) {
	unsigned j;

	for( j=0; j<4; ++j ) {
	    AddIcon(IconWcNames[i],Tilesets[j].Ident,i,IconsFiles[0].File[j]);
	}
    }
#endif

    //
    //	Add icons of the current tileset, with shortcut to hash.
    //
    for( i=0; i<IconsCount; ++i ) {
	if( !strcmp(Icons[i]->Tileset,TheMap.Tileset->Ident) ) {
	    *(Icon**)hash_add(IconHash,Icons[i]->Ident)=Icons[i];
	}
    }


#ifndef USE_CCL
    //
    //	Different names for the same thing
    //
    for( i=0; i<sizeof(DefaultIconAliases)/sizeof(*DefaultIconAliases); ++i ) {
	IconId id;

	id=IconByIdent(DefaultIconAliases[i][1]);
	DebugCheck( id==NoIcon );

	*(Icon**)hash_add(IconHash,(char*)DefaultIconAliases[i][0])=id;
    }
#else
    //
    //	Different names for the same thing
    //
    for( i=0; i<IconAliasesCount; ++i ) {
	IconId id;

	id=IconByIdent(IconAliases[i*2+1]);
	DebugCheck( id==NoIcon );

	*(Icon**)hash_add(IconHash,IconAliases[i*2+0])=id;
    }
#endif
}

/**
**	Load the graphics for the icons. Graphic data is only loaded once
**	and than shared.
*/
global void LoadIcons(void)
{
    unsigned i;

    InitIcons();

    //
    //	Load all icon files.
    //
    for( i=0; i<IconsCount; ++i ) {
	if( !strcmp(Icons[i]->Tileset,TheMap.Tileset->Ident) ) {
	    if( !Icons[i]->File->Graphic ) {
		char* buf;
		char* file;

		file=Icons[i]->File->FileName;
		buf=alloca(strlen(file)+9+1);
#ifdef NEW_NAMES
		file=strcat(strcpy(buf,"graphics/"),file);
#else
		file=strcat(strcpy(buf,"graphic/"),file);
#endif
		ShowLoadProgress("\tIcons %s\n",file);
		Icons[i]->File->Graphic=LoadGraphic(file);
	    }
	    Icons[i]->Graphic=Icons[i]->File->Graphic;
	}
    }
}

/**
**	Cleanup memory used by the icons.
**
**	@todo
**		Write this cleanup function. Needed if we want to play more
**		than one level without new start of the program.
*/
global void CleanIcons(void)
{
    DebugLevel0Fn("FIXME: cleanup not supported\n");
}

/**
**	Find the icon by identifier.
**
**	@param ident	The icon identifier.
**
**	@return		Icon pointer or NoIcon == NULL if not found.
*/
global IconId IconByIdent(const char* ident)
{
    const IconId* icon;

    icon=(const IconId*)hash_find(IconHash,(char*)ident);

    if( icon ) {
	return *icon;
    }

    DebugLevel0Fn("Icon %s not found\n",ident);
    return NoIcon;
}

/**
**	Get the identifier of an icon.
**
**	@param icon	Icon pointer
**
**	@return		The identifier for the icon
*/
global const char* IdentOfIcon(IconId icon)
{
    DebugCheck( !icon );

    return icon->Ident;
}

/**
**	Draw unit icon 'icon' with border on x,y
**
**	@param icon	Icon identifier
**	@param flags	State of icon (clicked, mouse over...)
**	@param x	X display position
**	@param y	Y display position
*/
global void DrawUnitIcon(const void* player,IconId icon,unsigned flags
	,unsigned x,unsigned y)
{
    int color;

    DebugCheck( !icon );

    color= (flags&IconActive) ? ColorGray : ColorBlack;

    VideoDrawRectangleClip(color,x,y,ICON_WIDTH+7,ICON_HEIGHT+7);
    VideoDrawRectangleClip(ColorBlack,x+1,y+1,ICON_WIDTH+5,ICON_HEIGHT+5);

    VideoDrawVLine(ColorGray,x+ICON_WIDTH+4,y+5,ICON_HEIGHT-1);	// _|
    VideoDrawVLine(ColorGray,x+ICON_WIDTH+5,y+5,ICON_HEIGHT-1);
    VideoDrawHLine(ColorGray,x+5,y+ICON_HEIGHT+4,ICON_WIDTH+1);
    VideoDrawHLine(ColorGray,x+5,y+ICON_HEIGHT+5,ICON_WIDTH+1);

    color= (flags&IconClicked) ? ColorGray : ColorWhite;	// |~
    VideoDrawHLine(color,x+5,y+3,ICON_WIDTH+1);
    VideoDrawHLine(color,x+5,y+4,ICON_WIDTH+1);
    VideoDrawVLine(color,x+3,y+3,ICON_HEIGHT+3);
    VideoDrawVLine(color,x+4,y+3,ICON_HEIGHT+3);

    if( (flags&IconClicked) ) {
	++x; ++y;
    }

    x+=4;
    y+=4;
    GraphicPlayerPixels(player,icon->Graphic);
    VideoDrawSub(icon->Graphic,icon->X,icon->Y,icon->Width,icon->Height,x,y);

    if( flags&IconSelected ) {
	VideoDrawRectangleClip(ColorGreen,x,y,ICON_WIDTH-1,ICON_HEIGHT-1);
    }
}

/**
**	Save state of the icons to file.
**
**	@param file	Output file.
*/
global void SaveIcons(FILE* file)
{
    char** cp;
    int i;

    fprintf(file,"\n;;; -----------------------------------------\n");
    fprintf(file,";;; MODULE: icons $Id$\n\n");

    if( (cp=IconWcNames) ) {
	fprintf(file,"(define-icon-wc-names");

	i=90;
	while( *cp ) {
	    if( i+strlen(*cp)>79 ) {
		i=fprintf(file,"\n ");
	    }
	    i+=fprintf(file," '%s",*cp++);
	}
	fprintf(file,")\n\n");
    }

    for( i=0; i<IconsCount; ++i ) {
	fprintf(file,"(define-icon '%s '%s\n",
		Icons[i]->Ident,Icons[i]->Tileset);
	fprintf(file,"  'normal %d \"%s\")\n",
		Icons[i]->Index,Icons[i]->File->FileName);
    }
}

#ifdef USE_CCL

/**
**	@brief Parse icon definition.
**
**	@param list	Icon definition list.
*/
local SCM CclDefineIcon(SCM list)
{
    SCM value;
    char* ident;
    char* tileset;
    char* str;
    int n;

    IfDebug( n=0; );

    //	Identifier

    ident=gh_scm2newstr(gh_car(list),NULL);
    list=gh_cdr(list);

    //	Tileset

    tileset=gh_scm2newstr(gh_car(list),NULL);
    list=gh_cdr(list);

    //	Type

    value=gh_car(list);
    list=gh_cdr(list);
    if( gh_eq_p(value,gh_symbol2scm("normal")) ) {
	//	Normal icon - index, file
	n=gh_scm2int(gh_car(list));
	list=gh_cdr(list);
	str=gh_scm2newstr(gh_car(list),NULL);
	list=gh_cdr(list);

    } else {
	str=gh_scm2newstr(value,NULL);
	fprintf(stderr,"%s: Wrong tag `%s'\n",ident,str);
    }

    if( !gh_null_p(list) ) {
	fprintf(stderr,"too much arguments\n");
    }

    DebugLevel3Fn("icon %s/%s %d of %s\n",ident,tileset,n,str);
    AddIcon(ident,tileset,n,str);
    free(ident);
    free(tileset);
    free(str);

    return SCM_UNSPECIFIED;
}

/**
**	@brief Parse icon alias definition.
**
**	@todo
**		Should check if alias is free and icon already defined.
**
**	@param alias	Icon alias name.
**	@param icon	Original icon.
*/
local SCM CclDefineIconAlias(SCM alias,SCM icon)
{
    IconAliases=realloc(IconAliases,sizeof(char*)*2*(IconAliasesCount+1));
    IconAliases[IconAliasesCount*2+0]=gh_scm2newstr(alias,NULL);
    IconAliases[IconAliasesCount*2+1]=gh_scm2newstr(icon,NULL);
    IconAliasesCount++;

    return SCM_UNSPECIFIED;
}

/**
**	@brief Define icon mapping from original number to internal symbol
**
**	@param list	List of all names.
*/
local SCM CclDefineIconWcNames(SCM list)
{
    int i;
    char** cp;

    if( (cp=IconWcNames) ) {		// Free all old names
	while( *cp ) {
	    free(*cp++);
	}
	free(IconWcNames);
    }

    //
    //	Get new table.
    //
    i=gh_length(list);
    IconWcNames=cp=malloc((i+1)*sizeof(char*));
    while( i-- ) {
	*cp++=gh_scm2newstr(gh_car(list),NULL);
	list=gh_cdr(list);
    }
    *cp=NULL;

    return SCM_UNSPECIFIED;
}

/**
**	Register CCL features for icons.
**
**	@todo
**		Add more functions for CCL. (draw-icon)
*/
global void IconCclRegister(void)
{
    gh_new_procedureN("define-icon",CclDefineIcon);
    gh_new_procedure2_0("define-icon-alias",CclDefineIconAlias);

    gh_new_procedureN("define-icon-wc-names",CclDefineIconWcNames);
}

#endif

//@}
