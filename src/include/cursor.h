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
--	Includes
----------------------------------------------------------------------------*/

#include "player.h"
#include "video.h"

/*----------------------------------------------------------------------------
--	Definitions
----------------------------------------------------------------------------*/

/**
**	Cursor type typedef
*/
typedef struct _cursor_type_ CursorType;

/**
**	Private type which specifies current cursor type
*/
struct _cursor_type_ {
    const void*	Type;			/// Object type (future extensions)

    char*	Ident;			/// Identifier to reference it.
    char*	Race;			/// Race name.

    char*	File;			/// graphic file of the cursor.

    int		HotX;			/// hot point x
    int		HotY;			/// hot point y
    int		Width;			/// width of cursor
    int		Height;			/// height of cursor

// --- FILLED UP ---

    Graphic*	Sprite;			/// cursor sprite image
};

/**
**	Cursor definition
*/
typedef struct _cursor_config_ {
    char*	Name;			/// config cursor-type name
    CursorType*	Cursor;			/// cursor-type 
} CursorConfig;

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern const char CursorTypeType[];	/// unit type type
extern CursorType* Cursors;		/// cursor types description

extern enum CursorState_e CursorState;	/// cursor state
extern int CursorAction;		/// action for selection
extern int CursorValue;			/// value for CursorAction (spell type f.e.)
extern UnitType* CursorBuilding;	/// building cursor

extern CursorType* GameCursor;		/// cursor type
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

    /// initialize all cursor
extern void LoadCursors(unsigned int race);

    /// cursor-type by identifier
extern CursorType* CursorTypeByIdent(const char* ident);

    /** Draw cursor on screen in position x,y..
    **	@param type	cursor type pointer
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param frame	sprite animation frame
    */
extern void DrawCursor(const CursorType* type,int x,int y,int frame);

    /// destroy the cursor background (for menu use!)
extern void DestroyCursorBackground(void);

    /// hide the cursor
extern void HideCursor(void);

    /// draw any cursor
extern void DrawAnyCursor(void);

    /// hide any cursor
extern int HideAnyCursor(void);

    /// initialize the cursor module
extern void InitCursor(void);

//@}

#endif	// !__CURSOR_H__
