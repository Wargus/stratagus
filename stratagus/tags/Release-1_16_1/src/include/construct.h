/*
**	A clone of a famous game.
*/
/**@name construct.h	-	The constructions headerfile. */
/*
**	(c) Copyright 1998,1999 by Lutz Sammer
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

    RleSprite*	RleSprite;		// sprite image
} Construction;

#define ConstructionWall	15

extern void LoadConstructions(void);
extern void DrawConstruction(int type,int image,int x,int y);

//@}

#endif	// !__CONSTRUCT_H__
