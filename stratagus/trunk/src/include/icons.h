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
//	(c) Copyright 1998-2001 by Lutz Sammer
//
//	$Id$

#ifndef __ICONS_H__
#define __ICONS_H__

//@{

/*----------------------------------------------------------------------------
--	Documentation
----------------------------------------------------------------------------*/

/**
**	@struct _icon_ icons.h
**
**	\#include "icons.h"
**
**	typedef struct _icon_ Icon;
**
**	typedef Icon* IconId;
**
**	This structure contains all informations about an icon.
**	Currently only rectangular static icons of 46x38 pixels are supported.
**	In the future it is planned to support animated and not rectangular
**	icons and icons of different sizes.
**
**	The icon structure members:
**
**	Icon::Ident
**
**		Unique identifier of the icon, used to reference it in config
**		files and during startup.  Don't use this in game, use instead
**		the pointer to this structure.
**
**	Icon::Tileset
**
**		Unique identifier of the tileset, used to allow different
**		graphics for the same icons depending on the tileset. Resolved
**		during startup in InitIcons().
**		@see Tileset::Ident
**
**	Icon::File
**
**		Pointer to icon file (file containing the graphics), each icon
**		could have an own icon file or some up to all icons could share
**		the same icon file.
**
**	Icon::Index
**
**		Index into the icon file. You know one up to all icons could
**		be in the same file. This index distinguishes them.
**
**	Icon::X
**
**		X pixel index into the graphic image.
**		(Icon::Index%5)*ICON_WIDTH.
**
**	Icon::Y
**
**		Y pixel index into the graphic image.
**		(Icon::Index/5)*ICON_HEIGHT.
**
**	Icon::Width
**
**		Icon width in pixels, defaults to ICON_WIDTH.
**
**	Icon::Height
**
**		Icon height in pixels, defaults to ICON_WIDTH.
**
**	Icon::Graphic
**
**		Graphic image containing the loaded graphics. Loaded by
**		LoadIcons(). All icons belonging to the same icon file shares
**		this structure.
**
**	@todo
**		IconId can be removed, use Icon* for it.
*/

/**
**	@struct _icon_config_ icons.h
**
**	\#include "icons.h"
**
**	typedef struct _icon_config_ IconConfig;
**
**	This structure contains all configuration informations about an icon.
**
**	IconConfig::Name
**
**		Unique identifier of the icon, used to reference icons in config
**		files and during startup.  The name is resolved during game
**		start and the pointer placed in the next field.
**		@see Icon::Ident
**		
**	IconConfig::Icon
**
**		Pointer to an icon. This pointer is resolved during game start.
**
**	Example how this can be used in C initializers:
**
**	@code
**		{ "icon-peasant" },
**	@endcode
*/

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "video.h"

#ifndef __STRUCT_PLAYER__
#define __STRUCT_PLAYER__
typedef struct _player_ Player;
#endif

/*----------------------------------------------------------------------------
--	Defines
----------------------------------------------------------------------------*/

#define IconActive	1		/// cursor on icon
#define IconClicked	2		/// mouse button down on icon
#define IconSelected	4		/// this the selected icon

#define ICON_WIDTH	46		/// default icon width in panels
#define ICON_HEIGHT	38		/// default icon height in panels

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/**
**	A structure describing an icon file, which could contain one or more
**	icons. @internal use only.
**
**	@todo
**		IconFile::Icons member isn't setup and used.
*/
typedef struct _icon_file_ {
    char*	FileName;		/// Icon file name/path

    unsigned	Width;			/// Icon width
    unsigned	Height;			/// Icon height

	/** FIXME: unsed */
    unsigned	Icons;			/// Number of icons in this file

// --- FILLED UP ---
    Graphic*	Graphic;		/// Graphic data loaded
} IconFile;

    ///	Icon: rectangle image used in menus.
typedef struct _icon_ {
    char*	Ident;			/// Icon identifier
    char*	Tileset;		/// Tileset identifier

    IconFile*	File;			/// File containing the data
    unsigned	Index;			/// Index into file

    unsigned	X;			/// X index into graphic
    unsigned	Y;			/// Y index into graphic

    unsigned	Width;			/// Icon width
    unsigned	Height;			/// Icon height

// --- FILLED UP ---
    Graphic*	Graphic;		/// Graphic data loaded
} Icon;

typedef Icon* IconId;			/// Icon referencing

#define NoIcon	NULL			/// used for errors == no valid icon

    ///	Icon reference (used in config tables)
typedef struct _icon_config_ {
    char*	Name;			/// config icon name
    IconId	Icon;			/// icon pointer to use to run time
} IconConfig;

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern char** IconWcNames;			/// pud original -> internal

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

extern void InitIcons(void);			/// init icons
extern void LoadIcons(void);			/// load icons
extern void CleanIcons(void);			/// cleanup icons

extern IconId IconByIdent(const char* ident);	/// name -> icon
extern const char* IdentOfIcon(IconId icon);	/// icon -> name

    /// draw icon of an unit
extern void DrawUnitIcon(const Player*,IconId,unsigned,unsigned,unsigned);

extern void SaveIcons(FILE*);			/// Save icons
extern void IconCclRegister(void);		/// register CCL features

//@}

#endif	// !__ICONS_H__
