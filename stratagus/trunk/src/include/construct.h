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
/**@name construct.h	-	The constructions headerfile. */
/*
**	(c) Copyright 1998-2000 by Lutz Sammer
**
**	$Id$
*/

#ifndef __CONSTRUCT_H__
#define __CONSTRUCT_H__

//@{

/*----------------------------------------------------------------------------
--	CONSTRUCTION
----------------------------------------------------------------------------*/

typedef struct _construction_ {
    char*	File[TilesetMax];	// sprite file

    int		Width;			// " width
    int		Height;			// " height

// --- FILLED UP ---

#ifdef NEW_VIDEO
    Graphic*	Sprite;			/// construction sprite image
#else
    RleSprite*	RleSprite;		/// construction sprite image
#endif
} Construction;

#define ConstructionWall	15

extern void LoadConstructions(void);
extern void DrawConstruction(int type,int image,int x,int y);

//@}

#endif	// !__CONSTRUCT_H__
