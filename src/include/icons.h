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
/**@name icons.h	-	The icons headerfile. */
//
//	(c) Copyright 1998-2000 by Lutz Sammer
//
//	$Id$
//

#ifndef __ICONS_H__
#define __ICONS_H__

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Defines
----------------------------------------------------------------------------*/

#define IconActive	1		/// cursor on icon
#define IconClicked	2		/// mouse button down on icon
#define IconSelected	4		/// this the selected icon

#define ICON_WIDTH	46		/// icon width in panels
#define ICON_HEIGHT	38		/// icon height in panels

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

typedef unsigned IconId;		/// Icon referencing

#define NoIcon	-1			/// used for errors = no valid icon

/**
**      Icon definition (used in config tables)
*/
typedef struct _icon_config_ {
    char*	Name;			/// config icon name
    IconId	Icon;			/// identifier to use to run time
} IconConfig;

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

extern void InitIcons(void);				/// init icons
extern void LoadIcons(void);				/// load icons
extern void CleanIcons(void);				/// cleanup

extern IconId IconByIdent(const char* ident);		/// name -> icon
extern const char* IdentOfIcon(IconId icon);		/// icon -> name

#ifdef NEW_VIDEO
    /// draw icons of an unit
extern void DrawUnitIcon(const void*,IconId,unsigned,unsigned,unsigned);
#else
    /// draw icons of an unit
extern void DrawUnitIcon(IconId num,unsigned flags,unsigned x,unsigned y);
#endif

//@}

#endif	// !__ICONS_H__
