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
/**@name cursor.h	-	The cursors header file. */
//
//	(c) Copyright 1998-2001 by Lutz Sammer
//
//	$Id$

#ifndef __CURSOR_H__
#define __CURSOR_H__

//@{

/*----------------------------------------------------------------------------
--	Documentation
----------------------------------------------------------------------------*/

/**
**	@struct _cursor_type_ cursor.h
**
**	\#include "cursor.h"
**
**	typedef struct _cursor_type_ CursorType;
**
**	This structure contains all informations about a cursor.
**	The cursor changes depending of the current user input state.
**	A cursor can have transparent areas and color cycle animated.
**
**	In the future it is planned to support animated cursors.
**
**	The cursor-type structure members:
**
**	CursorType::OType
**
**		Object type (future extensions).
**
**	CursorType::Ident
**
**		Unique identifier of the cursor, used to reference it in config
**		files and during startup. Don't use this in game, use instead
**		the pointer to this structure.
**
**	CursorType::Race
**
**		Owning Race of this cursor ("human", "orc", "alliance",
**		"mythical", ...). If NULL, this cursor could be used by any
**		race.
**
**	CursorType::File
**
**		File containing the image graphics of the cursor.
**
**	CursorType::HotX CursorType::HotY
**
**		Hot spot of the cursor in pixels. Relative to the sprite origin
**		(0,0). The hot spot of a cursor is the point to which FreeCraft
**		refers in tracking the cursor's position.
**
**	CursorType::Width CursorType::Height
**
**		Size of the cursor in pixels.
**
**	CursorType::Graphic
**
**		Contains the sprite of the cursor, loaded from CursorType::File.
**		Multicolor image with alpha or transparency.
*/

/**
**	@struct _cursor_config_ cursor.h
**
**	\#include "cursor.h"
**
**	typedef struct _cursor_config_ CursorConfig;
**
**	This structure contains all informations to reference/use a cursor.
**	It is normally used in other config structures.
**
**	CursorConfig::Name
**
**		Name to reference this cursor-type. Used while initialization.
**		(See CursorType::Ident)
**
**	CursorConfig::Cursor
**
**		Pointer to this cursor-type. Used while runtime.
*/

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "player.h"
#include "video.h"

/*----------------------------------------------------------------------------
--	Definitions
----------------------------------------------------------------------------*/

    ///	Cursor-type typedef
typedef struct _cursor_type_ CursorType;

    ///	Private type which specifies the cursor-type
struct _cursor_type_ {
    const void*	OType;			/// object type (future extensions)

    char*	Ident;			/// identifier to reference it
    char*	Race;			/// race name

    char*	File;			/// graphic file of the cursor

    int		HotX;			/// hot point x
    int		HotY;			/// hot point y
    int		Width;			/// width of cursor
    int		Height;			/// height of cursor

// --- FILLED UP ---

    Graphic*	Sprite;			/// cursor sprite image
};

    /// Cursor config reference
typedef struct _cursor_config_ {
    char*	Name;			/// config cursor-type name
    CursorType*	Cursor;			/// cursor-type pointer
} CursorConfig;

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern const char CursorTypeType[];	/// cursor-type type
extern CursorType* Cursors;		/// cursor-types description

extern enum CursorState_e CursorState;	/// cursor state
extern int CursorAction;		/// action for selection
extern int CursorValue;			/// value for action (spell type f.e.)
extern UnitType* CursorBuilding;	/// building cursor

extern CursorType* GameCursor;		/// cursor-type
extern int CursorX;			/// cursor position on screen X
extern int CursorY;			/// cursor position on screen Y
extern int CursorStartX;		/// rectangle started on screen X
extern int CursorStartY;		/// rectangle started on screen Y

extern int OldCursorX;			/// saved cursor position on screen X
extern int OldCursorY;			/// saved cursor position on screen Y
extern int OldCursorW;			/// saved cursor width in pixel
extern int OldCursorH;			/// saved cursor height in pixel
extern int OldCursorSize;		/// size of saved cursor image
extern void* OldCursorImage;		/// background saved behind cursor

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// Load all cursors
extern void LoadCursors(unsigned int race);

    /// Cursor-type by identifier
extern CursorType* CursorTypeByIdent(const char* ident);

    /// Draw cursor on screen in position x,y
extern void DrawCursor(const CursorType* type,int x,int y,int frame);

    /// Destroy the cursor background (for menu use!)
extern void DestroyCursorBackground(void);

    /// Hide the cursor
extern void HideCursor(void);

    /// Draw any cursor
extern void DrawAnyCursor(void);

    /// Hide any cursor
extern int HideAnyCursor(void);

    /// Initialize the cursor module
extern void InitCursor(void);

//@}

#endif	// !__CURSOR_H__
