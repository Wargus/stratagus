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
/**@name ccl.c		-	The craft configuration language. */
//
//	(c) Copyright 1998-2000 by Lutz Sammer
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "freecraft.h"
#include "iolib.h"

#ifdef USE_CCL

#include "video.h"
#include "tileset.h"
#include "sound_id.h"
#include "icons.h"
#include "button.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "upgrade.h"
#include "depend.h"
#include "unit.h"
#include "map.h"
#include "minimap.h"
#include "ccl.h"
#include "pud.h"
#include "missile.h"
#include "ccl_sound.h"
#include "ccl.h"
#include "font.h"
#include "pathfinder.h"

#include <guile/gh.h>			// I use guile for a quick hack

#ifdef USE_SVGALIB
#undef GUILE_GTK
#endif

#ifdef GUILE_GTK			// experimental guile-gtk support

#include <guile-gtk.h>

extern void sgtk_init_gtk_gtk_glue();
extern void sgtk_init_gtk_gdk_glue();

#endif	// GUILE_GTK

#endif	// USE_CCL

#ifdef USE_CCL2

#include <string.h>

#include "ccl.h"
#include "missile.h"
#include "depend.h"
#include "upgrade.h"
#include "unit.h"
#include "map.h"
#include "pud.h"
#include "ccl_sound.h"
#include "ui.h"
#include "font.h"
#include "pathfinder.h"
#include "ai.h"

#endif // USE_CCL2

#if defined(USE_CCL) || defined(USE_CCL2)

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

global char* CclStartFile;		/// CCL start file
global int CclInConfigFile;		/// True while config file parsing

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Free pointer, if it lays in the heap.
**
**	@param ptr	Pointer into heap.
*/
global void CclFree(void* ptr)
{
#if 0
#ifndef __MINGW32__
    extern unsigned long end;

    // FIXME: is the save on all architectures?
    if( ptr<(void*)&end ) {
	return;
    }
    free(ptr);
#endif
#endif
}

// FIXME: memory loose, if you define the same thing again.
// FIXME: use CclFree.

/*............................................................................
..	Config
............................................................................*/

/**
**	Return the freecraft library path.
**
**	@return		Current libray path.
*/
local SCM CclFreeCraftLibraryPath(void)
{
    return gh_str02scm(FreeCraftLibPath);
}

/**
**	Default title-screen.
**
**	@param title	SCM title. (nil reports only)
**
**	@return		Current title screen.
*/
local SCM CclTitleScreen(SCM title)
{
    if( !gh_null_p(title) ) {
	if( TitleScreen ) {
	    CclFree(TitleScreen);
	    TitleScreen=NULL;
	}

	TitleScreen=gh_scm2newstr(title,NULL);
    } else {
	title=gh_str02scm(TitleScreen);
    }
    return title;
}

local SCM CclColorCycleAll(void){
  ColorCycleAll = 1;

  return SCM_UNSPECIFIED;
}

local SCM CclNoColorCycleAll(void){
  ColorCycleAll = 0;

  return SCM_UNSPECIFIED;
}

// FIXME: remove this
extern SCM CclManaSprite(SCM file,SCM x,SCM y,SCM w,SCM h);
extern SCM CclHealthSprite(SCM file,SCM x,SCM y,SCM w,SCM h);

/**
**	Enable display health as health-bar.
*/
local SCM CclShowHealthBar(void)
{
    ShowHealthBar=1;
    ShowHealthDot=0;

    return SCM_UNSPECIFIED;
}

/**
**	Enable display health as health-dot.
*/
local SCM CclShowHealthDot(void)
{
    ShowHealthBar=0;
    ShowHealthDot=1;

    return SCM_UNSPECIFIED;
}

/**
**	Enable display health as horizontal bar.
*/
local SCM CclShowHealthHorizontal(void)
{
    ShowHealthBar=1;
    ShowHealthDot=0;
    ShowHealthHorizontal=1;

    return SCM_UNSPECIFIED;
}

/**
**	Enable display health as vertical bar.
*/
local SCM CclShowHealthVertical(void)
{
    ShowHealthBar=1;
    ShowHealthDot=0;
    ShowHealthHorizontal=0;

    return SCM_UNSPECIFIED;
}

/**
**	Enable display mana as mana-bar.
*/
local SCM CclShowManaBar(void)
{
    ShowManaBar=1;
    ShowManaDot=0;

    return SCM_UNSPECIFIED;
}

/**
**	Enable display mana as mana-dot.
*/
local SCM CclShowManaDot(void)
{
    ShowManaBar=0;
    ShowManaDot=1;

    return SCM_UNSPECIFIED;
}

/**
**	Enable energy bars and dots only for selected units
*/
local SCM CclShowEnergySelected(void)
{
    ShowEnergySelectedOnly=1;

    return SCM_UNSPECIFIED;
}


/**
**	Enable display of full bars/dots.
*/
local SCM CclShowFull(void)
{
    ShowNoFull=0;

    return SCM_UNSPECIFIED;
}

/**
**	Enable display mana as horizontal bar.
*/
local SCM CclShowManaHorizontal(void)
{
    ShowManaBar=1;
    ShowManaDot=0;
    ShowManaHorizontal=1;

    return SCM_UNSPECIFIED;
}

/**
**	Enable display mana as vertical bar.
*/
local SCM CclShowManaVertical(void)
{
    ShowManaBar=1;
    ShowManaDot=0;
    ShowManaHorizontal=0;

    return SCM_UNSPECIFIED;
}

/**
**	Disable display of full bars/dots.
*/
local SCM CclShowNoFull(void)
{
    ShowNoFull=1;

    return SCM_UNSPECIFIED;
}

/**
**	Draw decorations always on top.
*/
local SCM CclDecorationOnTop(void)
{
    DecorationOnTop=1;

    return SCM_UNSPECIFIED;
}

/**
**	Enable display of sight range.
*/
local SCM CclShowSightRange(void)
{
    ShowSightRange=1;

    return SCM_UNSPECIFIED;
}

/**
**	Enable display of react range.
*/
local SCM CclShowReactRange(void)
{
    ShowReactRange=1;

    return SCM_UNSPECIFIED;
}

/**
**	Enable display of attack range.
*/
local SCM CclShowAttackRange(void)
{
    ShowAttackRange=1;

    return SCM_UNSPECIFIED;
}

/**
**	Enable display of orders.
*/
local SCM CclShowOrders(void)
{
    ShowOrders=1;

    return SCM_UNSPECIFIED;
}

/*
**	For debug increase mining speed.
*/
local SCM CclSpeedMine(SCM speed)
{
    SpeedMine=gh_scm2int(speed);

    return speed;
}

/**
**	For debug increase gold delivery speed.
**
**	@param speed	Speed factor of gold mining.
*/
local SCM CclSpeedGold(SCM speed)
{
    SpeedGold=gh_scm2int(speed);

    return speed;
}

/**
**	For debug increase wood chopping speed.
*/
local SCM CclSpeedChop(SCM speed)
{
    SpeedChop=gh_scm2int(speed);

    return speed;
}

/**
**	For debug increase wood delivery speed.
*/
local SCM CclSpeedWood(SCM speed)
{
    SpeedWood=gh_scm2int(speed);

    return speed;
}

/**
**	For debug increase haul speed.
*/
local SCM CclSpeedHaul(SCM speed)
{
    SpeedHaul=gh_scm2int(speed);

    return speed;
}

/**
**	For debug increase oil delivery speed.
*/
local SCM CclSpeedOil(SCM speed)
{
    SpeedOil=gh_scm2int(speed);

    return speed;
}

/**
**	For debug increase building speed.
*/
local SCM CclSpeedBuild(SCM speed)
{
    SpeedBuild=gh_scm2int(speed);

    return speed;
}

/**
**	For debug increase training speed.
*/
local SCM CclSpeedTrain(SCM speed)
{
    SpeedTrain=gh_scm2int(speed);

    return speed;
}

/**
**	For debug increase upgrading speed.
*/
local SCM CclSpeedUpgrade(SCM speed)
{
    SpeedUpgrade=gh_scm2int(speed);

    return speed;
}

/**
**	For debug increase researching speed.
*/
local SCM CclSpeedResearch(SCM speed)
{
    SpeedResearch=gh_scm2int(speed);

    return speed;
}

/**
**	For debug increase all speeds.
*/
local SCM CclSpeeds(SCM speed)
{
    SpeedMine=SpeedGold=
	SpeedChop=SpeedWood=
	SpeedHaul=SpeedOil=
	SpeedBuild=
	SpeedTrain=
	SpeedUpgrade=
	SpeedResearch=gh_scm2int(speed);

    return speed;
}

/**
**	Disable mouse scroll.
*/
local SCM CclMouseScrollOff(void)
{
    TheUI.MouseScroll=0;

    return SCM_UNSPECIFIED;
}

/**
**	Debug unit slots.
*/
local SCM CclUnits(void)
{
    Unit** slot;
    int freeslots;
    int destroyed;
    int nullrefs;
    int i;
    static char buf[80];

    i=0;
    slot=UnitSlotFree;
    while( slot ) {			// count the free slots
	++i;
	slot=(void*)*slot;
    }
    freeslots=i;

    //
    //	Look how many slots are used
    //
    destroyed=nullrefs=0;
    for( slot=UnitSlots; slot<UnitSlots+MAX_UNIT_SLOTS; ++slot ) {
	if( *slot
		&& (*slot<(Unit*)UnitSlots
			|| *slot>(Unit*)(UnitSlots+MAX_UNIT_SLOTS)) ) {
	    if( (*slot)->Destroyed ) {
		++destroyed;
	    } else if( !(*slot)->Refs ) {
		++nullrefs;
	    }
	}
    }

    sprintf(buf,"%d free, %d(%d) used, %d, destroyed, %d null"
	    ,freeslots,MAX_UNIT_SLOTS-1-freeslots,NumUnits,destroyed,nullrefs);
    SetStatusLine(buf);
    fprintf(stderr,"%d free, %d(%d) used, %d destroyed, %d null\n"
	    ,freeslots,MAX_UNIT_SLOTS-1-freeslots,NumUnits,destroyed,nullrefs);

    return gh_int2scm(destroyed);
}

/**
**	Compiled with sound.
*/
local SCM CclWithSound(void)
{
#ifdef WITH_SOUND
    return SCM_BOOL_T;
#else
    return SCM_BOOL_F;
#endif
}

/**
**	Get FreeCraft home path.
*/
local SCM CclGetFreeCraftHomePath(void)
{
    char buf[PATH_MAX];

    strcpy(buf,getenv("HOME"));
    strcat(buf,"/");
    strcat(buf,FREECRAFT_HOME_PATH);

    return gh_str02scm(buf);
}

/**
**	Get FreeCraft library path.
*/
local SCM CclGetFreeCraftLibraryPath(void)
{
    return gh_str02scm(FREECRAFT_LIB_PATH);
}

/*............................................................................
..	Tables
............................................................................*/

/**
**	Parse missile-type.
**
**	@param list	List describing missile.
*/
local SCM CclMissileType(SCM list)
{
    SCM value;
    int type;

    //	Slot
    value=gh_car(list);
    type=gh_scm2int(value);
    if( type>=MissileTypeMax ) {
	fprintf(stderr,"Wrong type %d\n",type);
	return list;
    }
    list=gh_cdr(list);
    DebugLevel3("MissileType: %d\n",type);

    //	Name
    value=gh_car(list);
    MissileTypes[type].Ident=gh_scm2newstr(value,NULL);
    list=gh_cdr(list);

    //	File
    value=gh_car(list);
    MissileTypes[type].File=gh_scm2newstr(value,NULL);
    list=gh_cdr(list);

    // Width,Height
    value=gh_car(list);
    MissileTypes[type].Width=gh_scm2int(value);
    list=gh_cdr(list);
    value=gh_car(list);
    MissileTypes[type].Height=gh_scm2int(value);
    list=gh_cdr(list);

    // Sound impact
    value=gh_car(list);
#ifdef WITH_SOUND
    if (ccl_sound_p(value)) {
	MissileTypes[type].ImpactSound.Sound=ccl_sound_id(value);
    } else
#endif
    if (!gh_boolean_p(value) || gh_scm2bool(value) ) {
	fprintf(stderr,"Wrong argument in MissileType\n");
    }
    list=gh_cdr(list);

    // FIXME: class, speed not written!!!

    return list;
}

/**
**	Load a pud. (Try in library path first)
**
**	@param file	filename of pud.
**
**	@return		FIXME: Nothing.
*/
local SCM CclLoadPud(SCM file)
{
    char* name;
    char buffer[1024];

    name=gh_scm2newstr(file,NULL);
    LoadPud(LibraryFileName(name,buffer),&TheMap);
    free(name);

    // FIXME: LoadPud should return an error
    return SCM_UNSPECIFIED;
}

/**
**	Define a map.
**
**	@param width	Map width.
**	@param height	Map height.
*/
local SCM CclDefineMap(SCM width,SCM height)
{
    TheMap.Width=gh_scm2int(width);
    TheMap.Height=gh_scm2int(height);

    TheMap.Fields=calloc(TheMap.Width*TheMap.Height,sizeof(*TheMap.Fields));
    InitUnitCache();

    MapX=MapY=0;

    return SCM_UNSPECIFIED;
}

/**
**	Parse a freecraft map.
**
**	@param list	list of tuples keyword data
*/
local SCM CclFreeCraftMap(SCM list)
{
    SCM value;
    SCM name;
    SCM data;

    //
    //	Parse the list:	(still everything could be changed!)
    //
    while( !gh_null_p(list) ) {

	value=gh_car(list);
	//gh_display(value);
	//gh_newline();
	if( gh_list_p(value) ) {
	    name=gh_car(value);
	    data=gh_cdr(value);
	    if( !gh_symbol_p(name) ) {
		fprintf(stderr,"symbol expected\n");
		return list;
	    }
	    if( gh_eq_p(name,gh_symbol2scm("version")) ) {
		DebugLevel1("VERSION:\n");
		gh_display(data);
		gh_newline();
		// FIXME:
	    } else if( gh_eq_p(name,gh_symbol2scm("description")) ) {
		DebugLevel1("DESCRIPTION:\n");
		gh_display(data);
		gh_newline();
		// FIXME:
	    } else if( gh_eq_p(name,gh_symbol2scm("terrain")) ) {
		int terrain;

		DebugLevel1("TERRAIN:\n");
		gh_display(data);
		gh_newline();
		value=gh_car(data);
		data=gh_cdr(data);
		terrain=gh_scm2int(value);
		TheMap.Terrain=terrain;
		// FIXME:
	    } else if( gh_eq_p(name,gh_symbol2scm("dimension")) ) {
		int width;
		int height;

		DebugLevel1("DIMENSION:\n");
		gh_display(data);
		gh_newline();
		value=gh_car(data);
		width=gh_scm2int(value);
		data=gh_cdr(data);
		value=gh_car(data);
		height=gh_scm2int(value);
		TheMap.Width=width;
		TheMap.Height=height;

		TheMap.Fields=calloc(width*height,sizeof(*TheMap.Fields));
		InitUnitCache();

	    } else if( gh_eq_p(name,gh_symbol2scm("tiles")) ) {
		int i;
		int l;

		DebugLevel1("TILES:\n");
		value=gh_car(data);
		if( !gh_vector_p(value) ) {
		    fprintf(stderr,"vector expected\n");
		    return SCM_UNSPECIFIED;
		}
		l=gh_vector_length(value);
		if( l!=TheMap.Width*TheMap.Height ) {
		    fprintf(stderr,"Wrong tile table length %d\n",l);
		}
		for( i=0; i<l; ++i ) {
		    TheMap.Fields[i].Tile=
			    Tilesets[0].Table[
				gh_scm2int(gh_vector_ref(value,gh_int2scm(i)))
			    ];
		}
	    } else {
		;
	    }
	} else {
	    fprintf(stderr,"list expected\n");
	    return list;
	}

	list=gh_cdr(list);
    }

    return list;
}

/*............................................................................
..	Commands
............................................................................*/

#ifdef USE_CCL

/**
**	Send command to ccl.
*/
global void CclCommand(char* command)
{
    gh_display(gh_eval_str_with_standard_handler(command));
    gh_newline();
}

#endif

#ifdef USE_CCL2

/**
**	Send command to ccl.
**
**	@param command	Zero terminated command string.
*/
global void CclCommand(char* command)
{
    char msg[80];
    int retval;

    strncpy(msg,command,sizeof(msg));

    // FIXME: cheat protection
    retval=repl_c_string(msg,0,0,sizeof(msg));
    DebugLevel3("\n%d=%s\n",retval,msg);

    SetMessageDup(msg);
}

#endif

#endif

#ifdef USE_CCL

/**
**	Callback to c-main1.
*/
local SCM CclMain1(void)
{
    main1(0,NULL);

    DebugLevel0("Shouldn't be reached\n");

    return SCM_UNSPECIFIED;
}


/*............................................................................
..	Setup
............................................................................*/

/**
**	Define a vararg scheme procedure.
*/
global SCM gh_new_procedureN(char* proc_name, SCM (*fn) ())
{
    return gh_new_procedure(proc_name,fn,0,0,1);
}

/**
**	Called from scheme.
*/
local void gh_main_prog(int argc,char* argv[])
{
    gh_new_procedure0_0("library-path",CclFreeCraftLibraryPath);
    gh_new_procedure1_0("title-screen",CclTitleScreen);
    gh_new_procedure0_0("color-cycle-all",CclColorCycleAll);
    gh_new_procedure0_0("no-color-cycle-all",CclNoColorCycleAll);

    gh_new_procedure5_0("mana-sprite",CclManaSprite);
    gh_new_procedure5_0("health-sprite",CclHealthSprite);
    gh_new_procedure0_0("show-health-bar",CclShowHealthBar);
    gh_new_procedure0_0("show-health-dot",CclShowHealthDot);
    gh_new_procedure0_0("show-mana-bar",CclShowManaBar);
    gh_new_procedure0_0("show-mana-dot",CclShowManaDot);
    gh_new_procedure0_0("show-full",CclShowFull);
    gh_new_procedure0_0("show-no-full",CclShowNoFull);
    gh_new_procedure0_0("decoration-on-top",CclDecorationOnTop);
    gh_new_procedure0_0("show-sight-range",CclShowSightRange);
    gh_new_procedure0_0("show-react-range",CclShowReactRange);
    gh_new_procedure0_0("show-attack-range",CclShowAttackRange);
    gh_new_procedure0_0("show-orders",CclShowOrders);

    gh_new_procedure1_0("speed-mine",CclSpeedMine);
    gh_new_procedure1_0("speed-gold",CclSpeedGold);
    gh_new_procedure1_0("speed-chop",CclSpeedChop);
    gh_new_procedure1_0("speed-wood",CclSpeedWood);
    gh_new_procedure1_0("speed-haul",CclSpeedHaul);
    gh_new_procedure1_0("speed-oil",CclSpeedOil);
    gh_new_procedure1_0("speed-build",CclSpeedBuild);
    gh_new_procedure1_0("speed-train",CclSpeedTrain);
    gh_new_procedure1_0("speed-upgrade",CclSpeedUpgrade);
    gh_new_procedure1_0("speed-research",CclSpeedResearch);
    gh_new_procedure1_0("speeds",CclSpeeds);

    gh_new_procedureN("missile-type",CclMissileType);

    TilesetCclRegister();
    MapCclRegister();
    PathfinderCclRegister();
    UnitButtonCclRegister();
    UnitTypeCclRegister();
    SoundCclRegister();
    UserInterfaceCclRegister();

    gh_new_procedure1_0("load-pud",CclLoadPud);
    gh_new_procedure2_0("define-map",CclDefineMap);

    gh_new_procedureN("freecraft-map",CclFreeCraftMap);

    gh_new_procedure1_0("contrast",Contrast);
    gh_new_procedure1_0("brightness",Brightness);
    gh_new_procedure1_0("saturation",Saturation);

    gh_new_procedure0_0("c-main1",CclMain1);

#ifdef GUILE_GTK
    SGTK_REGISTER_GLUE(sgtk_init_gtk_gtk_glue);
    SGTK_REGISTER_GLUE(sgtk_init_gtk_gdk_glue);
    // ALL Stupid
    {
    int argc;
    char** argv;
    static char* args[2] = { "FreeCraft", NULL };

    argc=1;
    argv=args;
    sgtk_init_with_args(&argc,&argv);
    }
    //sgtk_init_with_args(NULL,NULL);
#endif

    //
    //	Load and evaluate configuration file
    //
    CclInConfigFile=1;
    gh_eval_file(CclStartFile);
    CclInConfigFile=0;

    // FIXME: guile didn't cleanup, all memory is lost!

    main1(argc,argv);			// continue with setup
}

/**
**	Initialize ccl.
*/
global void CclInit(void)
{
    gh_enter(0,0,gh_main_prog);		// guile didn't return!
}

#endif

#ifdef USE_CCL2
/**
**	Initialize ccl.
*/
global void CclInit(void)
{
    char* sargv[5];
    char buf[1024];
    char* file;

    sargv[0] = "FreeCraft";
    sargv[1] = "-v1";
    sargv[2] = "-g0";
    sargv[3] = "-h100000:10";
#ifdef __MINGW32__
    sprintf(buf,"-l%s\\",FreeCraftLibPath);
#else
    sprintf(buf,"-l%s",FreeCraftLibPath);
#endif
    sargv[4] = strdup(buf);
    siod_init(5,sargv);

    init_subr_0("library-path",CclFreeCraftLibraryPath);
    init_subr_1("title-screen",CclTitleScreen);

    init_subr_5("mana-sprite",CclManaSprite);
    init_subr_5("health-sprite",CclHealthSprite);
    init_subr_0("color-cycle-all",CclColorCycleAll);
    init_subr_0("no-color-cycle-all",CclNoColorCycleAll);

    init_subr_0("show-health-bar",CclShowHealthBar);
    init_subr_0("show-health-dot",CclShowHealthDot);
// adicionado por protoman
    init_subr_0("show-health-vertical",CclShowHealthVertical);
    init_subr_0("show-health-horizontal",CclShowHealthHorizontal);
    init_subr_0("show-mana-vertical",CclShowManaVertical);
    init_subr_0("show-mana-horizontal",CclShowManaHorizontal);
// fim

    init_subr_0("show-mana-bar",CclShowManaBar);
    init_subr_0("show-mana-dot",CclShowManaDot);
    init_subr_0("show-energy-selected-only",CclShowEnergySelected);
    init_subr_0("show-full",CclShowFull);
    init_subr_0("show-no-full",CclShowNoFull);
    init_subr_0("decoration-on-top",CclDecorationOnTop);
    init_subr_0("show-sight-range",CclShowSightRange);
    init_subr_0("show-react-range",CclShowReactRange);
    init_subr_0("show-attack-range",CclShowAttackRange);
    init_subr_0("show-orders",CclShowOrders);

    init_subr_1("speed-mine",CclSpeedMine);
    init_subr_1("speed-gold",CclSpeedGold);
    init_subr_1("speed-chop",CclSpeedChop);
    init_subr_1("speed-wood",CclSpeedWood);
    init_subr_1("speed-haul",CclSpeedHaul);
    init_subr_1("speed-oil",CclSpeedOil);
    init_subr_1("speed-build",CclSpeedBuild);
    init_subr_1("speed-train",CclSpeedTrain);
    init_subr_1("speed-upgrade",CclSpeedUpgrade);
    init_subr_1("speed-research",CclSpeedResearch);
    init_subr_1("speeds",CclSpeeds);

    init_lsubr("missile-type",CclMissileType);

    TilesetCclRegister();
    MapCclRegister();
    PathfinderCclRegister();
    UnitButtonCclRegister();
    UnitTypeCclRegister();
    UpgradesCclRegister();
    DependenciesCclRegister();
    SoundCclRegister();
    FontsCclRegister();
    UserInterfaceCclRegister();
    AiCclRegister();

    init_subr_1("load-pud",CclLoadPud);
    init_subr_2("define-map",CclDefineMap);

    init_lsubr("freecraft-map",CclFreeCraftMap);

    gh_new_procedure0_0("mouse-scroll-off",CclMouseScrollOff);
    gh_new_procedure0_0("units",CclUnits);

    gh_new_procedure0_0("with-sound",CclWithSound);
    gh_new_procedure0_0("get-freecraft-home-path",CclGetFreeCraftHomePath);
    gh_new_procedure0_0("get-freecraft-library-path"
	    ,CclGetFreeCraftLibraryPath);

    print_welcome();

    //
    //	Load and evaluate configuration file
    //
    CclInConfigFile=1;
    //file=LibraryFileName("freecraft.ccl",buf);
    file=LibraryFileName("ccl/freecraft.ccl",buf);
    vload(file,0,1);
    CclInConfigFile=0;
}

#endif	// defined(USE_CCL) || defined(USE_CCL2)

//@}
