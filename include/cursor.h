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
/*
**	(c) Copyright 1998-2000 by Lutz Sammer
**
**	$Id$
*/

#ifndef __CURSOR_H__
#define __CURSOR_H__

//@{

/*----------------------------------------------------------------------------
--	Definitions
----------------------------------------------------------------------------*/

    /// cursor type typedef
typedef struct _cursor_type_ CursorType;

    /// private type which specifies current cursor type
struct _cursor_type_ {
	/// resource filename one for each race
    const char*	File[PlayerMaxRaces];

// FIXME: this must be extra for each file (different sizes for the races)
// FIXME: or must define that each image has the same size
	/// hot point x
    int		HotX;
	/// hot point y
    int		HotY;
	/// width of cursor
    int		Width;
	/// height of cursor
    int		Height;

	/// sprite image of cursor : FILLED
    RleSprite*	RleSprite;
};

    /// cursor type (enumerated)
enum CursorType_e {
    CursorTypePoint = 0,
    CursorTypeGlass,
    CursorTypeCross,
    CursorTypeYellowHair,
    CursorTypeGreenHair,
    CursorTypeRedHair,
    CursorTypeMove,
    CursorTypeArrowE,
    CursorTypeArrowN,
    CursorTypeArrowNE,
    CursorTypeArrowNW,
    CursorTypeArrowS,
    CursorTypeArrowSE,
    CursorTypeArrowSW,
    CursorTypeArrowW,
    CursorMax
};

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

    /// cursor types description
extern CursorType Cursors[CursorMax];

extern int OldCursorX;			// last cursor data
extern int OldCursorY;
extern int OldCursorW;
extern int OldCursorH;

extern int CursorAction;		// action for selection
extern UnitType* CursorBuilding;	// building cursor

    /// current cursor type (shape)
extern CursorType* GameCursor;
extern int CursorX;			// cursor position
extern int CursorY;
extern int CursorStartX;		// rectangle started
extern int CursorStartY;

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// initialize all cursor
extern void LoadCursors(unsigned int race);

    /** Draw cursor on screen in position x,y..
    **	@param type	cursor type pointer
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param frame	sprite animation frame
    */
extern void DrawCursor(CursorType* type,int x,int y,int frame);

    /// hide the cursor
extern void HideCursor(void);

    /// draw any cursor 
extern void DrawAnyCursor(void);

    /// hide any cursor 
extern int HideAnyCursor(void);

//@}

#endif	// !__CURSOR_H__
