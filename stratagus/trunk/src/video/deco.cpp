//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________			     ______________________
//			  T H E	  W A R	  B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name deco.c	-	Mechanism for all 2D decorations on screen. */
//
//	(c) Copyright 2002 by Lutz Sammer and Stephan Rasenberg
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


/**
**	page Decorations Decorations - a mechanism for all what is on screen
**	
**	This pages describes the intend to use a special mechanism that
**	functions as an intermediate between the stratagus engine and the
**	video routines.
**	
**	@note It won't take away the possibility to directly draw something on
**	the screen without using this mechanism, so you have to be careful when
**	you decide to do that, as you should take care that no other object is
**	overlapping yours or you should be able to handle those overlapping
**	objects yourself.
**	
**	
**	@section DecoWhy Why we want 'a decoration mechanism'
**
**	This mechanism was though off, as we got some basic problems when
**	dealing with drawing everything separately from different parts of the
**	source code, details follow:
**
**	@subsection DecoP1 Problem 1: higher screen resolution support
**	
**	Screen resolutions beyond 640x480 requires much memory, which also
**	costs speed performance to draw it. Not only the drawing needs to
**	write to this memory. Some what is drawn, needs to read first to
**	combine certain colors (like Fog of War).
**
**	When drawing you also need to invalidate the area you have drawn.
**	Meaning (for some video platforms, like X11) that what you draw is
**	drawn in a background buffer (a videopage) and the invalidate action
**	puts it on the screen.
**
**	Which is nice, as you can do multiple draws and invaldiate the total
**	area covered by those in one go, delivering a smoother update (as it
**	does not flicker).  But to invalidate the total buffer needs a very
**	large amount of memory to be transferred for higher screen resolutions.
**
**	So we want to minimize the access to this memory, to only update those
**	parts that have been changed, increasing the performance.
**
**	Example: before using this mechanism, we simply handled the entire main
**	map, when an unit moves from one tile to another. While we only
**	wanted the few tiles it crosses to be updated (for that unit).
**
**	@subsection DecoP2 Problem 2: unit decorations and lines
**
**	To redraw only what is needed, we need to split these areas into
**	separate rectangles. This is much faster and simpler then any other
**	variant known to me. And is also nicer, as in the end we also need to
**	invalidate rectangles areas (so we don't need to determine these twice).
**
**	Units a drawn as a sprite and so if we want to redraw the unit, we
**	need to know the rectangle region where the sprite it located and
**	redraw everythin in that.
**
**	But we also draw extra decorations near the unit (like a health bar,
**	mana and such..), these really look nice in the game. But makes it
**	harder to denote the area that needs to be redrawn, as this is no
**	longer the rectangle as is determined by the sprite. And it can
**	even change, as some decorations will only appear when the unit is
**	selected.
**
**	Currently we also support lines showing the destination/path the units
**	will follow, we want to have it divided in a lot of small rectangles,
**	as a single rectangle bounding box will cover too much of the screen
**	(so a redraw of all what is inside) and again makes the invalidate
**	slower.
**
**	@subsection DecoP3 Problem 3: overlapping units/graphics
**
**	For an update to draw some graphic, we need to know which graphics
**	are overlapping it as they also need to be redrawn. There is no link
**	available of what is located near eachother. There is a link denotihg
**	the location of the unit on the map, but what we see doesn't have to
**	be at the same location on the map.
**
**      Example: think of a flying unit, which is not shown on the same tile
**	as a ground unit underneath, but slightly shifted.. possible one/two
**	tiles further.
**
**	So we need to denote some linkage in way that is fast and accurate in
**	determining what is overlapping and needs to be redrawn.
**
**	@subsection DecoP4 Problem 4: depth level (z-axis)
**
**	Before this mechanism we first draw the ground, then the units, then
**	the missiles and as last the fog-of-war. We also want more depth-levels
**	to be supported and let so for overlapping units/etc.. we need to
**	automaticly determine which needs to be drawn first (on given z-level).
**
**	@subsection DecoP5 Problem 5: future decorations
**
**	As we want to support all kinds of new decorations to make the
**	interface look nicer, we want a type-independent mechanism. Not as done
**	before: units/missiles/gui-buttons/.. but all of one type 'Decoration'.
**	Making the mechanism unware of what the Decoration needs to represent,
**	se we cann attach different kinds of decoratons, without having to
**	update this file and/or add new functionaility..
**	
**	
**	@section DecoIdeas Decoration mechanism project ideas
**
**	Ok, now to handle all those problems and most of all, be efficient
**	and fast, I have a proposal which I will describe according to the
**	following storyline delivering us the implementation issues, so you
**	can understand the reasoning behind it.
**
**	@note if you gonna change this implementation, please update this
**	storyline part and denote why things have been changed.
**
**	@subsection DecoI1 Idea 1: screentiles
**	We first need some structure of denoting what is where on the screen.
**
**	We could do that with quad-trees or some kind of mathematical way of
**	using line-equations to denote the location of each decoration, but I
**	considered these to be too slow. Also looking at the invalidate part,
**	we need to split a line up in rectangles and as each graphic is a
**	rectangle I came to the conclusion to split the screen up in multiple
**	tiles and let its decoration denote the tile(s) it is in.
**
**	@subsection DecoI2 Idea 2: 4x4 matrix in 16bit
**
**	Now we don't really need to have a direct linkage from tile to
**	decoration, as we only need to know which tiles are 'dirty', meaning
**	some graphic is updated, it marks the tiles it is covering and the
**	other Decoratons can look at their tiles to see if they need to be
**	redrawn.
**
**	As we only have 'clean' and 'dirty' tiles, I made the conclusion we can
**	use a single bit for that and save lots of memory. To enable us to
**	cover as much tiles in one integer, I use a 16bit word to denote 4x4=16
**	tiles. So to check a 4x4 tile area, only one 16bit integer is read from
**	memory.
**
**	Example: a unit covering 2x2 tiles, with the left-top tile at (5,6)
**	tiles away from the top-left of the screen. This will be at entry
**	[5/4,6/4]  in an array of 16bit integers denoting all tiles on the
**	screen. And at bit-index [5%4,6%4] in such a 16bit integer:
**	with bit-indexes:  3  2  1  0  delivers: 0000  or as hecadecimal:
**			   7  6  5  4            0000		0x6600
**			  11 10  9  8            0110
**			  15 14 13 12            0110
**
**	@subsection DecoI3 Idea 3: Minimize decorations to max 4x4 tiles
**
**	Now we still need to determine the tiles-size, but no matter what we
**	choose later on. There will always be decorations covering multiple
**	tiles. One 16bit integer denoted 4x4 tiles, but a decoration of 4x4
**	tiles will it only in a single 16bit integer when its top-left tile is
**	exactly at a 4-tile boundary. As wordt-case scenario, where the
**	top-left ia at the bottom-right of the 4x4 bitmap, it can only be 1x1
**	tile big. So we need to support decorations covering multiple 16bit
**	integers.
**
**	However I want to restrict each decoration to 2x2 16bits, as these
**	can be simply marked and checked with 4 16bit bitmasks. As the support
**	of dynamic decoration sizes requires extra while-loops and
**	if-statements to determine which 16bit entries in the array are we
**	to mark/check. I think by not using any if- or while-loop will speed
**	up performance a bit. So each decoration has 4 16bit bitmasks, the
**	left-top, right-top, left-bottom and right-bottom 4x4 bitmask.
**
**	Now in the worst-case the decoration with the lef-top tile at the
**	right-bottom bit of the left-top bitmask, will still be able to able
**	have a 5x5 tile size. And in the best-case, with the left-top tile at
**	the left-top bit of the left-top bitmaks, can handle 8x8 tiles.
**
**	But I'm not using this total freedom of beyond 4x4 tiles, as it gets
**	very inefficient to determine bits covering more then 4x4 tiles over
**	different 16bit bitmask. And altough a 4x4 tile area can also cover
**	all 4 16bit integers, I can conver this to an area within a 32bit
**	integer. The trick for this is very simple:
**
**	In example: a 3x2 tile area, with only some bits marked 'dirty', is
**	determined by the following bits in 4 16bit bitmasks.
**
**	tile-area: 00000000  delivering bitmasks: A 0000 0000 B
**		   00000000			    0000 0000
**		   00000000			    0000 0000
**		   00101000	A = top right	    0001 0100
**		   00011000	B = left top
**		   00000000	C = right bottom  C 0001 1000 D
**		   00000000	D = left bottom	    0000 0000
**		   00000000			    0000 0000
**						    0000 0000
**
**	Now if we combine A and C in a 32bit integer and right bit-shift with
**	the y-bit-offset of the decoration in the left-top 16bit. We can
**	put A and C in one 16bit:
**
**	(A and C) 0000 -right-shift->	0001
**		  0000			0001
**		  0000			0000
**		  0001			0000
**		  0001
**		  0000
**		  0000
**		  0000
**
**	We can do the same for B and D and combine the two resulting 16bit in
**	one 32bit, representing the 8x4 tile area in which the 'dirty' bits are:
**
**	(A and C) 0001  (B and D) 0100  as one 32bit: 00010100
**		  0001		  1000  	      00011000
**		  0000		  0000  	      00000000
**		  0000		  0000  	      00000000
**
**	Now is reasonable fast to find the bits marked 'dirty'.
**
**	@note The mechanism still supports larger Decorations though, from
**	caller point-of-view. But these are simply split up into multiple
**	ones, linking them with eachother, so the 'caller' does not have to
**	know about them.
**
**	@subsection DecoI4 Idea 4: DIRTYSCREEN_BITSIZE=16 (tile=16x16 pixels)
**
**	To denote tiles as pixels will costs too much memory and too much
**	time to maintain them. So we use tiles with a larger size to prevent
**	that, but care must be taken that we don't make them too large or we
**	let each tile overlap too many decorations and that costs unneeded
**	redraw. 
**
**	Looking at this, the largest decoration as unit is currently
**	128x128 (dark portal). So if we want it to be represented as a
**	single decoration and with a maximum of 2x2 tiles, we get
**	64x64 pixels for each tile.  This is too big though, as a normal
**	ground graphic of 32x32, will already be split up in 2x2 decorations.
**
**	But if we would use a tile of 32x32, a unit crossing from one tile to
**	another, would have an overlap with four tiles at once. And as it
**	happens a lot that units go that near to eachother on a ground graphic
**	of 32x32, this would lead to a redraw of the entire ground graphic and
**	the other units overlapping this. Which would cause a chain-reaction of
**	unneeded redraws.
**
**	This overlap happens a lot, also when an unit stand stationary, its
**	decorations (healthbar,manabar,..) is drawn below it.. on a lower
**	ground tile graphic.
**
**	So I suggest to do another step back and let one tile represent 16x16
**	pixels, this will make it possible to update only on of four corners of
**	a ground graphic, so overlapping decorations on the other three corners
**	don't have to be updated.
**
**	For screen resolution 1600x1280, this will cost 100x80 tiles in
**	25x20 16bits = 1000 bytes. Which will be accessed a lot, but as its
**	that small it would be accesable from cache and not influence speed
**	that much.
**
**	@note for the assumption that a ground graphic of 32x32 pixels can be
**	put in 2x2 tiles, the main screenmap should start exactly at a tile
**	boundary.
**/

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"

#ifdef NEW_DECODRAW

#include "video.h"
#include "sweepline.h"
#include "deco.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/**
**  The accuracy of decoration boundary box in screentiles:
**	The entire screen is divided into screentiles of the same size.
**	With n==1 we have highest accurate representation and prevent unneeded
**	redraw of decorations which aren't overlapping, but cost much memory and
**	CPU speed.
**	With high n we have less accurate representation and may redraw
**	decorations which aren't overlapping as pixels, but are as tiles.
**	With practice an ideal accuracy should be used, which is defined by
**	DIRTYSCREEN_BITDETAIL as a bit-shift (>=0) of getting from pixel to tile
**/
#define DIRTYSCREEN_BITDETAIL 4 // 4bit-->16x16 pixels for each screen-tile
#define DIRTYSCREEN_DETAILSIZE (1 << DIRTYSCREEN_BITDETAIL)

/**
**  The restricted number of tiles as both width and height for
**  DecorationSingle which only can handle a 2x2 of 16bit matrix.
**  So we could support a DECOSINGLE_TILES==8 (2 16bit matrixes with each a
**  width of 4 tiles) when the decoration start at the upper-left corner of the
**  tile-area represented. But in the worst-case, the decoration start at the
**  lower-right corner of the upper-left tile, making only a maximum of 5 tiles
**  possible in that case.. Even more so, the current code of CheckRedraw will
**  restrict this further concerning the height, as it will translate the
**  'dirty' bits of all 4 16bit matrixes into a single 32bit for 8x4 tiles,
**  making it much eassier to check anbd find the 'dirty' bits..
**  FIXME: investigate if the use of a selection to get the best possible
**         representation is worth the trouble and so doesn't result in
**         a performance loss.
**/
#define DECOSINGLE_TILES 4
#define DECOSINGLE_PIXELS (DECOSINGLE_TILES*DIRTYSCREEN_DETAILSIZE)


/*----------------------------------------------------------------------------
--	Externals
----------------------------------------------------------------------------*/

extern void DecorationInit(void);

extern Deco *DecorationAdd( void *data,
			    void (*drawclip)(void *data),
			    DecorationLevel l, 
			    unsigned x, unsigned y,
			    unsigned w, unsigned h );
extern void DecorationRemove( Deco *d );
extern void DecorationRemoveLevels( DecorationLevel min, DecorationLevel max );


extern void DecorationMark( Deco *d );

extern void DecorationRefreshDisplay(void);
extern void DecorationUpdateDisplay(void);


/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

/**
**	The array dirtysceen is the representation of the screen in screentiles.
**	Each tile is normally 'clean' and can only be marked 'dirty', so we only
**	need 1 bit for each tile. By using a 4x4 tile area, we can store 16
**	tiles in 2 bytes and still be able to get the proper bit with
**	bit-operations fast enough:   <--------x
**	                              3  2  1  0 y
**	                              7  6  5  4 |
**	                             11 10  9  8 |
**	                             15 14 13 12 v
**
**      As this should be platform independent, we can't use type 'int' (which
**	is either 16, 32 bit or more), but use 2 of type 'char' which size is
**	always 8 bit.
**
**      dirtyscreen		= the array itself
**      dirtyscreen_xtiles	= no. 2-byte tiles horizontally (screen width)
**      dirtyscreen_ytiles	= no. 2-byte tiles vertically (screen height)
**      dirtyscreen_size	= the total size in bytes of the array
**/
static unsigned char *dirtyscreen = NULL;
static unsigned dirtyscreen_xtiles, dirtyscreen_ytiles, dirtyscreen_size,
                dirtyscreen_xbitmaps, dirtyscreen_ybitmaps;

/**
**	To denote an entire row/colum of tiles in one 4x4 segment, these
**	bitmasks help. They are based upon an bit-index 0..3, for which 0 is
**	exactly at a 4x4 matrix boundary (for which we don't need a mask).
**
**	xbitmaskhead	= mask for a bit-index denoting a starting x-pos 
**	ybitmaskhead	= mask for a bit-index denoting an ending x-pos 
**	xbitmasktail	= mask for a bit-index denoting a starting y-pos 
**	ybitmasktail	= mask for a bit-index denoting an ending y-pos 
**
**	@note: use an and-operation upon these bitmasks to get a combination
**	for any row/column inside a single 4x4 matrix.
**/
static unsigned xbitmaskhead[4] = {
  0xFFFF /*1111*/, 0xEEEE /*1110*/, 0xCCCC /*1100*/, 0x8888 /*1000*/,
         /*1111*/         /*1110*/         /*1100*/         /*1000*/
         /*1111*/         /*1110*/         /*1100*/         /*1000*/
         /*1111*/         /*1110*/         /*1100*/         /*1000*/
};
static unsigned xbitmasktail[4] = {
  0xFFFF /*1111*/, 0x1111 /*0001*/, 0x3333 /*0011*/, 0x7777 /*0111*/,
         /*1111*/         /*0001*/         /*0011*/         /*0111*/
         /*1111*/         /*0001*/         /*0011*/         /*0111*/
         /*1111*/         /*0001*/         /*0011*/         /*0111*/
};
static unsigned ybitmaskhead[4] = {
  0xFFFF /*1111*/, 0xFFF0 /*0000*/, 0xFF00 /*0000*/, 0xF000 /*0000*/,
         /*1111*/         /*1111*/         /*0000*/         /*0000*/
         /*1111*/         /*1111*/         /*1111*/         /*0000*/
         /*1111*/         /*1111*/         /*1111*/         /*1111*/
};
static unsigned ybitmasktail[4] = {
  0xFFFF /*1111*/, 0x000F /*1111*/, 0x00FF /*1111*/, 0x0FFF /*1111*/,
         /*1111*/         /*0000*/         /*1111*/         /*1111*/
         /*1111*/         /*0000*/         /*0000*/         /*1111*/
         /*1111*/         /*0000*/         /*0000*/         /*0000*/
};

/**
**	All decoration structs ordered by their depth level are stored in
**	this array as a link-list for every depth-level.
**/
static Deco *dhead[LevCount] = { NULL };

/**
**	We have a separate link-list to store decorations which are deleted,
**	doing so we can re-use such a deocration again without a new
**	allocation.
**	FIXME: this will result into a memory-leak when this mechanism is no
**	longer used. But I expect this to be needed througout the program and
**	so no memory is given back.
**/
static Deco *dgarbage = NULL;
static DecorationSingle *dsgarbage = NULL;


/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Given decoration-single-tile is put in the garbage-list, so it can be
**	re-used another time.
**
**	@note it might not be referenced as the old given type, as it should
**	be considered de-allocated (access can result in undefined behavior).
**/
static void DecorationSingleDelete( DecorationSingle *t )
{
  t->nxt    = dsgarbage;
  dsgarbage = t;
}

/**
**	Allocate (or re-use from garage list) a DecorationSingle struct
**/
static DecorationSingle *DecorationSingleAllocate(void)
{
  DecorationSingle *t;

  // Get memory for garbage link-list
  if ( !dsgarbage )
  {
    int size = 1024 / sizeof( DecorationSingle ); // about 1KB at a time
    dsgarbage = t = malloc( size * sizeof( DecorationSingle ) );
    if ( !t )
    {
      printf( "Out of memory (DecorationTypeAllocate,video/deco.c)\n" );
      exit( 1 );
    }
    while ( --size )
    {
      t->nxt = t + 1;
      t++;
    }
    t->nxt = NULL;
  }

  // Get new Decoration from garbage link-list
  t = dsgarbage;
  dsgarbage = (DecorationSingle *) t->nxt;

  return t;
}

/**
**	Given decorationis put in the garbage-list, so it can be
**	re-used another time.
**
**	@note it might not be referenced as the old given type, as it should
**	be considered de-allocated (access can result in undefined behavior).
**/
static void DecorationDelete( Deco *d )
{
  d->nxt   = dgarbage;
  dgarbage = d;
}

/**
**	Allocate (or re-use from garage list) a Decoration struct
**/
static Deco *DecorationAllocate(void)
{
  Deco *d;

  // Get memory for garbage link-list
  if ( !dgarbage )
  {
    int size = 1024 / sizeof( Deco ); // about 1KB at a time
    dgarbage = d = malloc( size * sizeof( Deco ) );
    if ( !d )
    {
      printf( "Out of memory (DecorationTypeAllocate,video/deco.c)\n" );
      exit( 1 );
    }
    while ( --size )
    {
      d->nxt = d + 1;
      d++;
    }
    d->nxt = NULL;
  }

  // Get new Decoration from garbage link-list
  d = dgarbage;
  dgarbage = d->nxt;

  return d;
}

/**
**	Given decoration is removed from this mechanism and will no longer
**	affect other ones.
**
**	@param d	= decoration as been created with DecorationAdd
**
**	@note: it should be prevented that Decorations are added and directly
**	removed, as they still have parts of the screen marked 'dirty' and
**	will influence other decorations to be re-drawn unneededly.
**/
void DecorationRemove( Deco *d )
{
  DecorationSingle *t, *tmp;

// Mark is needed.. as we possibly need to redraw what was overlapping it
  DecorationMark( d );

// remove from linklists and let its memory be re-used again
  if ( d->prv )
    d->prv->nxt = d->nxt;
  else dhead[ d->l ] = NULL;

  if ( d->nxt )
    d->nxt->prv = d->prv;

  t = d->singles;
  DecorationDelete( d );
  do
  {
    tmp = t;
    t = t->nxt;
    DecorationSingleDelete( tmp );
  }
  while ( t );
}

/**
**	Remove all decorations belonging to a range of levels at once..
**
**	@param min	= first level to remove
**	@param max	= last level to remove (higher or equal to min)
**/
void DecorationRemoveLevels( DecorationLevel min, DecorationLevel max )
{
  DebugCheck( min > max || max == LevCount );

  do
  {
    Deco *d;

    while ( (d=dhead[ min ]) )
    {
      DecorationSingle *t, *tmp;

      DecorationMark( d );
      t = d->singles;
      DecorationDelete( d );
      do
      {
        tmp = t;
        t = t->nxt;
        DecorationSingleDelete( tmp );
      }
      while ( t );

      dhead[ min ] = d->nxt;
    }
  }
  while ( ++min < max );
}

/**
**	Clears the dirtyscreen array, making all tiles 'unmarked'
**	FIXME: is there some way to get this fast?
**	       (skipping parts that are already '0' ?)
*/
static void ClearDirtyscreen(void)
{
  memset( dirtyscreen, 0, dirtyscreen_size );
}

/**
**	Initialize and allocate memory for this mechanism for a fixed screen
**	resolution (may not alter, unless DecorationInit is called again).
**	pre : VideoWidth and VideoHeight need to have the first
**	      DIRTYSCREEN_BITDETAIL bits on zero.
**/
void DecorationInit(void)
{
  // Some platform dependent assumpions of the entire mechanism.
  // If failure, please fix..
  DebugCheck( sizeof(unsigned long) < 4 ); // atleast 32bit 
  DebugCheck( sizeof(unsigned int) < 2 );  // atleast 16bit 

  // The number of screen-tiles we support, a lower DIRTYSCREEN_BITDETAIL
  // means detailed smaller tiles, but also costs more memory.
  dirtyscreen_xtiles = (VideoWidth >> DIRTYSCREEN_BITDETAIL);
  dirtyscreen_ytiles = (VideoHeight >> DIRTYSCREEN_BITDETAIL);
  DebugCheck( (dirtyscreen_xtiles<<DIRTYSCREEN_BITDETAIL)!=VideoWidth ||
              (dirtyscreen_ytiles<<DIRTYSCREEN_BITDETAIL)!=VideoHeight );

  // Get memory for array dirtyscreen as 4x4bit matrixes each denoting 16
  // tiles. So (dirtyscreen_xtiles*dirtyscreen_ytiles)/16 matrixes of 2 bytes
  dirtyscreen_xbitmaps = 1 + (dirtyscreen_xtiles-1) / 4;
  dirtyscreen_ybitmaps = 1 + (dirtyscreen_ytiles-1) / 4;
  dirtyscreen_size     = dirtyscreen_xbitmaps * dirtyscreen_ybitmaps * 2;
  if ( dirtyscreen )
    dirtyscreen = realloc( dirtyscreen, dirtyscreen_size );
  else dirtyscreen = malloc( dirtyscreen_size );

  if ( !dirtyscreen )
  {
    printf( "Out of memory (InitDirtyscreen,video/deco.c)\n" );
    exit( 1 );
  }

  ClearDirtyscreen();

  { int i;
    for ( i = 0; i < LevCount; i++ )
      while ( dhead[i] != NULL )
      {
        Deco *d = dhead[i];
        dhead[i] = d->nxt;
        DecorationDelete( d );
      }
  }
}

/*
**      dirtyscreen		= the array itself
**      dirtyscreen_xtiles	= no. 2-byte tiles horizontally (screen width)
**      dirtyscreen_ytiles	= no. 2-byte tiles vertically (screen height)
*/
global void debugdirtyscreen(void)
{
  char *p;
  unsigned int x, y, xbit, ybit, mask;

  printf( "   " ); 
  for ( x=y=0; x<dirtyscreen_xtiles; y++ )
    if ( y % 5 == 0 )
      printf( " " );
    else printf("%d", x++ % 10 );
  printf( "\n" );

  for ( y=0; y<dirtyscreen_ybitmaps; y++ )
  {
    for ( ybit=0; ybit<4; ybit++ )
    {
      printf( "%3d", y*4+ybit );
      p = dirtyscreen + y * dirtyscreen_xbitmaps * 2;
      for ( x=0; x<dirtyscreen_xbitmaps; x++, p+=2 )
      {
        mask = ((p[1] << 8) | p[0]) >> (ybit*4);
        printf(" ");
        for ( xbit=1; xbit<=8; xbit <<= 1 )
          printf( "%c", (mask & xbit) ? '1' : '0' );
      }
      printf("\n");
    }
  }
}

/**
**	Marks given position on screen as 'dirty',  which can later be checked
**	to determine if something is overlapping it and to denote what needs to
**	be invalidated.
**	@param x	= x-position in pixels on screen
**	@param y	= y-position in pixels on screen
**	pre : given (x,y) should be inside screen resolution as given through
**	      DecorationInit
**/
static void MarkPos( unsigned x, unsigned y )
{
  char *p;
  unsigned bits;

  // Scale (x,y) down to the tile-index as it is stored in array dirtyscreen
  x >>= DIRTYSCREEN_BITDETAIL;
  y >>= DIRTYSCREEN_BITDETAIL;

  // As the array dirtyscreen denotes each tile as an individual bit (to reduce
  // memory), we use an easy to use 4x4 bit format that fits exactly into a
  // 16bit element:    <--------x
  //                   3  2  1  0 y
  //                   7  6  5  4 |
  //                  11 10  9  8 |
  //                  15 14 13 12 v
  // As the type of the dirtyscreen elements is char, we are also sure we
  // don't waste memory as it's element size is exactly twice (16bit) the size
  // of char (8bit), where as sizeof(unsigned) can be bigger.
  // But to perform bit-operation on the 16bit element, we need to get the
  // 4x4bit matrix as type unsigned at tile-index y*dirtyscreen_xtiles+x,
  // which is translated into 16bit bitmap-index as:
  // p = dirtyscreen + ((y/4)*dirtyscreen_xbitmaps+(x/4))*2
  p    = dirtyscreen + (((y>>2)*dirtyscreen_xbitmaps+(x>>2))<<1);

  // Mark the one bit refering to the tile (x,y) in least sig. 4x4 bits
  bits = (p[1] << 8) | p[0];
  bits |= ( 1 << ((x & 0x000F) + ((y & 0x000F)<<2)) );
  p[0] = bits & 0x00FF;
  p[1] = (bits >> 8);
}

/**
**      Marks given area on screen as 'dirty',  which can later be checked
**      to determine if something is overlapping it and to denote what needs to
**      be invalidated.
**      @param x        = x-position in pixels on screen
**      @param y        = y-position in pixels on screen
**      @param width    = width in pixels on screen (> 0)
**      @param height   = height in pixels on screen (> 0)
**      pre : given (x,y;x+width-1,y+height-1) should be inside screen
**            resolution as determined by DecorationInit 
**/
void MarkArea( int x, int y, int w, int h )
{
  char *tiles;
  unsigned int xmaskhead, xmasktail, ymaskhead, ymasktail, bits;
  int w2, nextline, bitindex;

  DebugCheck( x <= 0 || y <= 0 || w <= 0 || h <= 0 ||
              (x+w) >= VideoWidth || (y+h) >= VideoHeight );

  // First scale (x,y) down to the tile-index as it is stored in array
  // dirtyscreen and let w,h denote the width/height in tiles iso pixels
  x >>= DIRTYSCREEN_BITDETAIL;
  y >>= DIRTYSCREEN_BITDETAIL;
  w = ((w - 1) >> DIRTYSCREEN_BITDETAIL) + 1;
  h = ((h - 1) >> DIRTYSCREEN_BITDETAIL) + 1;
  DebugCheck( w > dirtyscreen_xtiles || h > dirtyscreen_ytiles );

  // Reference to top-left 4x4bit matrix containing tile (x,y) in dirtyscreen
  tiles = dirtyscreen + (((y>>2)*dirtyscreen_xbitmaps+(x>>2))<<1);

  // Now scale (w,h) down to the number of 16bit elements (in a 4x4 bit matrix)
  // to check and denote when we need to check the four sides with a bitmask
  // or just check wether the 4x4 matrix(es) should be entirely zero.
  bitindex = (x & 0x3);
  xmaskhead = xbitmaskhead[ bitindex ];
  xmasktail = xbitmasktail[ (x+w) & 0x3 ];
  if ( w < 4 && w <= 4 - bitindex )
  { // xmaskhead and xmasktail in same 4x4 matrix column  --> combine to one
    if ( x >= dirtyscreen_xtiles - 4 ) // at rightmost side of screen
    { // move one 4x4 matrix to the left to prevent acces outside 2D dimension
      tiles  -= 4 * 2;
      xmasktail &= xmaskhead;
      xmaskhead = 0;
    }
    else
    {
      xmaskhead &= xmasktail;
      xmasktail = 0;
    }
  }
  bitindex  = (y & 0x3);
  ymaskhead = ybitmaskhead[ bitindex ];
  ymasktail = ybitmasktail[ (y+h) & 0x3 ];
  if ( h < 4 && h <= 4 - bitindex )
  { // ymaskhead and ymasktail in same 4x4 matrix row  --> combine to one
    if ( y >= dirtyscreen_ytiles - 4 ) // at bottom side of screen
    { // move one 4x4 matrix upwards to prevent acces outside 2D dimension
      tiles  -= 2 * dirtyscreen_xbitmaps;
      ymasktail &= ymaskhead;
      ymaskhead = 0;
    }
    else
    {
      ymaskhead &= ymasktail;
      ymasktail  = 0;
    }
  }

  //
  // Mark the tiles with above bitmasks..
  //
  nextline=(dirtyscreen_xbitmaps-(w>>2))*2;
  w-=2;
  h-=2;

  // upper-left 4x4 matrixes
  bits = (tiles[1] << 8) | tiles[0];
  bits |= (ymaskhead&xmaskhead);
  tiles[0] = bits & 0x00FF;
  tiles[1] = bits & 0xFF00;

  // upper-middle 4x4 matrixes
  w2=2;
  while (  tiles+=2, w2-- > 0 )
  {
    bits = (tiles[1] << 8) | tiles[0];
    bits |= ymaskhead;
    tiles[0] = bits & 0x00FF;
    tiles[1] = bits & 0xFF00;
  }

  // upper-right 4x4 matrix
  bits = (tiles[1] << 8) | tiles[0];
  bits |= (ymaskhead&xmasktail);
  tiles[0] = bits & 0x00FF;
  tiles[1] = bits & 0xFF00;

  h--;

  // middle 4x4 matrixes
  while (  tiles+=nextline, h-- > 0 )
  {
    // left-middle 4x4 matrix
    bits = (tiles[1] << 8) | tiles[0];
    bits |= xmaskhead;
    tiles[0] = bits & 0x00FF;
    tiles[1] = bits & 0xFF00;

    // middle 4x4 matrixes
    w2 = w;
    while (  tiles+=2, w2-- > 0 )
    {
      tiles[0] |= 0xFF;
      tiles[1] |= 0xFF;
    }

    // right-middle 4x4 matrix
    bits = (tiles[1] << 8) | tiles[0];
    bits |= xmasktail;
    tiles[0] = bits & 0x00FF;
    tiles[1] = bits & 0xFF00;
  }

  // lower-left 4x4 matrix
  bits = (tiles[1] << 8) | tiles[0];
  bits |= (ymasktail&xmaskhead);
  tiles[0] = bits & 0x00FF;
  tiles[1] = bits & 0xFF00;

  // lower-middle 4x4 matrixes
  w2 = w;
  while (  tiles+=2, w2-- > 0 )
  {
    bits = (tiles[1] << 8) | tiles[0];
    bits |= ymasktail;
    tiles[0] = bits & 0x00FF;
    tiles[1] = bits & 0xFF00;
  }

  // lower-right 4x4 matrix
  bits = (tiles[1] << 8) | tiles[0];
  bits |= (ymasktail&xmasktail);
  tiles[0] = bits & 0x00FF;
  tiles[1] = bits & 0xFF00;
}

/**
**	Draws within a given area by setting up a proper clip rectangle and
**	calling the user-defined drawclip routine with its data.
**
**	@param x	= x pixel position on screen of left-top
**	@param y	= y pixel position on screen of left-top
**	@param data	= any type of object that needs to be drawn, this
**			  file will not need to know what it is.
**	@param drawclip	= function that can draw above data using draw rountines
**			  that can restrict drawing to a clip rectangle.
**/
static void DrawArea( int x, int y, int w, int h,
                      void *data, void (*drawclip)(void *data) )
{
  SetClipping( x, y, x+w-1, y+h-1 );
  drawclip( data );
}

/**
**      Checks if any pixel in area on screen overlapped by this Decoration is
**	marked 'dirty' and if so.. will redraw only those tiles again.
**
**      @param d   = valid decoration
**      @param t   = one of the single-tile decorations of above
**/
static void CheckRedraw( Deco *d, DecorationSingle *t )
{
  char *top, *bottom;
  unsigned long topbits, bottombits, leftbits, rightbits, bits;

  top    = t->tiles;
  bottom = top + dirtyscreen_xbitmaps * 2;

  // Get left-top and -bottom 16bit 4x4 matrixes, masked to get the 'dirty'
  // area overlapped by this decoration in a single 32bit back to 16bit
  topbits     = (top[1] << 8) | top[0];
  topbits    &= t->lefttopmask;
  bottombits  = (bottom[1] << 8) | bottom[0];
  bottombits &= t->leftbottommask;
  leftbits    = (bottombits << 16) | topbits;
  leftbits  >>= t->bity4;
  leftbits = ((leftbits & 0xF000) << 12)
           | ((leftbits & 0x0F00) <<  8)
           | ((leftbits & 0x00F0) <<  4)
           |  (leftbits & 0x000F);

  // Get right-top and -bottom 16bit 4x4 matrixes, masked to get the 'dirty'
  // area overlapped by this decoration in a single 32bit back to 16bit
  topbits     = (top[3] << 8) | top[2];
  topbits    &= t->righttopmask;
  bottombits  = (bottom[3] << 8) | bottom[2];
  bottombits &= t->rightbottommask;
  rightbits   = (bottombits << 16) | topbits;
  rightbits >>= t->bity4;
  rightbits = ((rightbits & 0xF000) << 16)
            | ((rightbits & 0x0F00) << 12)
            | ((rightbits & 0x00F0) <<  8)
            | ((rightbits & 0x000F) <<  4);

  // Now combine both left+right 16bit into one 32bit
  bits = rightbits | leftbits;

  // Check this 32bit as a 8x4 tile area
  // FIXME: try merging neighbouring 'dirty' bits, to minimize DrawIt calls
  if ( bits )
  {
    int x, y;
    x = t->topleftx;
    y = t->toplefty;
    bits >>= t->bitx;
    do
    {
      if ( bits & 0x1 ) // bit 0 is 'dirty'
        DrawArea( x, y, DIRTYSCREEN_DETAILSIZE, DIRTYSCREEN_DETAILSIZE,
                  d->data, d->drawclip );

      if ( bits & 0x2 ) // bit 1 is 'dirty'
        DrawArea( x+DIRTYSCREEN_DETAILSIZE, y,
                  DIRTYSCREEN_DETAILSIZE, DIRTYSCREEN_DETAILSIZE,
                  d->data, d->drawclip );

      if ( bits & 0x4 ) // bit 2 is 'dirty'
        DrawArea( x+2*DIRTYSCREEN_DETAILSIZE, y,
                  DIRTYSCREEN_DETAILSIZE, DIRTYSCREEN_DETAILSIZE,
                  d->data, d->drawclip );

      if ( bits & 0x8 ) // bit 3 is 'dirty'
        DrawArea( x+3*DIRTYSCREEN_DETAILSIZE, y,
                  DIRTYSCREEN_DETAILSIZE, DIRTYSCREEN_DETAILSIZE,
                  d->data, d->drawclip );

      y += DIRTYSCREEN_DETAILSIZE;
      bits >>= 8; // next line of 8 bits
    } while ( bits );
  }
}

/*
**	FOR DEBUG PURPOSE ONLY
**	Will print the given 16bit as 4x4 tiles.
*/
global debugdecobits( unsigned int bits )
{
  int y;
  printf( "16bits as 4x4: 3210\n" );
  for ( y=0; y<=3; y++ )
  {
    printf( "             %d %c%c%c%c\n", y,
            bits&8?'1':'0', bits&4?'1':'0',
            bits&2?'1':'0', bits&1?'1':'0' );
    bits >>= 4;
  }
}

/*
**	Convert 8x8 tiles from 4 16bit masks into 8x4 tiles in one 32bit,
**	moving the set bits up with given bitshift in y-direction.
**	For this to work only a 4x4 area within above 8x8 tiles might be set.
**
**	@param ybitshift       = a (full) bitshift to move bits to upper tile
**	@param topleftmask     = 16bit bitmask for top-left     4x4 tiles
**	@param toprightmask    = 16bit bitmask for top-right    4x4 tiles
**	@param bottomleftmas k = 16bit bitmask for bottom-left  4x4 tiles
**	@param bottomrightmask = 16bit bitmask for bottom-right 4x4 tiles
*/
local unsigned long convert8x8to8x4(
  int ybitshift,
  unsigned int topleftmask,    unsigned int toprightmask,
  unsigned int bottomleftmask, unsigned int bottomrightmask )
{
  unsigned long leftmask, rightmask;

  leftmask   = (unsigned long)bottomleftmask << 16;
  leftmask  |= topleftmask;
  leftmask >>= ybitshift;

  leftmask = ((leftmask & 0xF000) << 12)
           | ((leftmask & 0x0F00) <<  8)
           | ((leftmask & 0x00F0) <<  4)
           |  (leftmask & 0x000F);

  rightmask   = (unsigned long)bottomrightmask << 16;
  rightmask  |= toprightmask;
  rightmask >>= ybitshift;

  rightmask = ((rightmask & 0xF000) << 16)
            | ((rightmask & 0x0F00) << 12)
            | ((rightmask & 0x00F0) <<  8)
            | ((rightmask & 0x000F) <<  4);

  return leftmask | rightmask;
}

/*
**	FOR DEBUG PURPOSE ONLY
**	Will print the given t as separate bitvalues.
*/
global debugsinglebits( DecorationSingle *t )
{
  char *p;
  unsigned long topleftscreen, toprightscreen, bottomleftscreen, 
                bottomrightscreen, leftbits, rightbits, bits32, mark32;
  int y, x;

// 2x2 16bit matrixes for 8x8 tiles translated to 8x4 tiles in one 32bit
  bits32 = convert8x8to8x4( t->bity4, t->lefttopmask,    t->righttopmask,
                                      t->leftbottommask, t->rightbottommask );

// now the same for marked bits
  p = t->tiles;
  topleftscreen  = (p[1] << 8) | p[0];
  toprightscreen = (p[3] << 8) | p[2];
  p = t->tiles + dirtyscreen_xbitmaps * 2;
  bottomleftscreen  = (p[1] << 8) | p[0];
  bottomrightscreen = (p[3] << 8) | p[2];
  mark32 = convert8x8to8x4( t->bity4, topleftscreen     & t->lefttopmask,
                                      toprightscreen    & t->righttopmask,
                                      bottomleftscreen  & t->leftbottommask,
                                      bottomrightscreen & t->rightbottommask );

// print 8x8 and 9x4 tiles representations
  printf( "DecorationSingle as 8x8 tiles\n" );
  printf( "     76543210 76543210 76543210  76543210\n     ");
  for ( x=7; t->bitx<x; x-- )
    printf( " " );
  printf( "^\n" );
  leftbits  = t->lefttopmask;
  rightbits = t->righttopmask;
  for ( y=0; y<=7; y++ )
  {
    printf( "%s%d ", (y * 4 == t->bity4) ? "-->" : "   ", y );

    for ( x=8; x; x>>=1 )
      printf( "%c", toprightscreen & x ? '1' : '0' );
    for ( x=8; x; x>>=1 )
      printf( "%c", topleftscreen & x ? '1' : '0' );

    printf( " " );
    for ( x=8; x; x>>=1 )
      printf( "%c", rightbits & x ? '1' : '0' );
    for ( x=8; x; x>>=1 )
      printf( "%c", leftbits & x ? '1' : '0' );

    printf( " " );
    for ( x=8; x; x>>=1 )
      printf( "%c", rightbits & toprightscreen & x ? '1' : '0' );
    for ( x=8; x; x>>=1 )
      printf( "%c", leftbits & topleftscreen & x ? '1' : '0' );

    topleftscreen  >>= 4;
    toprightscreen >>= 4;
    leftbits       >>= 4;
    rightbits      >>= 4;

    if ( y < 4 )
    {
      printf( "  " );
      for ( x=128; x; x>>=1 )
        printf( "%c", bits32 & x ? '1' : '0' );
      bits32  >>= 8;
      printf( "  " );
      for ( x=128; x; x>>=1 )
        printf( "%c", mark32 & x ? '1' : '0' );
      mark32  >>= 8;
    }

    printf( "\n" );
    if ( y == 3 )
    {
      leftbits  = t->leftbottommask;
      rightbits = t->rightbottommask;
      topleftscreen  = bottomleftscreen;
      toprightscreen = bottomrightscreen;
    }
  }
  printf( "     (screen)&( deco )=(redraw)  ( deco )  (redraw)\n");
}

/**
**	Will mark a DecorationSingle (restricted to 4x4 tiles)
**
**	@param t = one DecorationSingle (will not follow its filed nxt!)
**/
static void DecorationSingleMark( DecorationSingle *t )
{
  char *p;
  unsigned long bits;

  // Mark left-top 16bit 4x4 matrix with area overlapped by this decoration
  p = t->tiles;
  bits  = (p[1] << 8) | p[0];
  bits |= t->lefttopmask;
  p[0] = bits & 0x00FF;
  p[1] = (bits >> 8);

  // Mark right-top 16bit 4x4 matrix with area overlapped by this decoration
  bits  = (p[3] << 8) | p[2];
  bits |= t->righttopmask;
  p[2] = bits & 0x00FF;
  p[3] = (bits >> 8);

  // Mark left-bottom 16bit 4x4 matrix with area overlapped by this decoration
  p += dirtyscreen_xbitmaps * 2;
  bits  = (p[1] << 8) | p[0];
  bits |= t->leftbottommask;
  p[0] = bits & 0x00FF;
  p[1] = (bits >> 8);

  // Mark right-bottom 16bit 4x4 matrix with area overlapped by this decoration
  bits  = (p[3] << 8) | p[2];
  bits |= t->rightbottommask;
  p[2] = bits & 0x00FF;
  p[3] = (bits >> 8);
}

/**
**	Will mark a complete Decoration (maybe multiple DecorationSingle)
**
**	@param d = Deco to completely mark
**/
void DecorationMark( Deco *d )
{
  DecorationSingle *t;
  for ( t = d->singles; t; t = t->nxt )
    DecorationSingleMark( t );
}

/**
**	Add a single decoration restricted by size so it covers max 4x4 tiles.
**
**	@param x	= x pixel position on screen of left-top
**	@param y	= y pixel position on screen of left-top
**	@param w	= width in pixels of area to be drawn from (x,y)
**	@param h	= height in pixels of area to be drawn from (x,y)
**/
static DecorationSingle *DecorationSingleNew( unsigned x, unsigned y,
                                              unsigned w, unsigned h )
{
  DecorationSingle *t;
  unsigned bitindex, xmaskhead, xmasktail, ymaskhead, ymasktail;

  DebugCheck( x < 0 || y < 0 || w <= 0 || h <= 0 ||
              (x+w) >= VideoWidth || (y+h) >= VideoHeight );

  // Fill in this new Decoration so it can be used
  t = DecorationSingleAllocate();
  t->topleftx = x;
  t->toplefty = y;

  // Instead of storing given (x,y,w,h), we use prepared pointer and bitmasks
  // to the dirtyscreen array, as these are fast and can be used again..
  //
  // First scale (x,y) down to the tile-index as it is stored in array
  // dirtyscreen and let w,h denote the width/height in tiles iso pixels
  x >>= DIRTYSCREEN_BITDETAIL;
  y >>= DIRTYSCREEN_BITDETAIL;
  w = ((w - 1) >> DIRTYSCREEN_BITDETAIL) + 1;
  h = ((h - 1) >> DIRTYSCREEN_BITDETAIL) + 1;
  DebugCheck( w > DECOSINGLE_TILES || h > DECOSINGLE_TILES );

  // Reference to top-left 4x4bit matrix containing tile (x,y) in dirtyscreen
  t->tiles = dirtyscreen + (((y>>2)*dirtyscreen_xbitmaps+(x>>2))<<1);

  // Now scale (w,h) down to the number of 16bit elements (in a 4x4 bit matrix)
  // to check and denote when we need to check the four sides with a bitmask
  // or just check wether the 4x4 matrix(es) should be entirely zero.
  t->bitx  = bitindex = (x & 0x3);
  xmaskhead = xbitmaskhead[ bitindex ];
  xmasktail = xbitmasktail[ (x+w) & 0x3 ];
  if ( w < 4 && w <= 4 - bitindex )
  { // xmaskhead and xmasktail in same 4x4 matrix column  --> combine to one
    if ( x >= dirtyscreen_xtiles - 4 ) // at rightmost side of screen
    { // move one 4x4 matrix to the left to prevent acces outside 2D dimension
      t->tiles  -= 4 * 2;
      xmasktail &= xmaskhead;
      xmaskhead = 0;
    }
    else
    {
      xmaskhead &= xmasktail;
      xmasktail = 0;
    }
  }
  bitindex  = (y & 0x3);
  t->bity4  = (bitindex * 4);
  ymaskhead = ybitmaskhead[ bitindex ];
  ymasktail = ybitmasktail[ (y+h) & 0x3 ];
  if ( h < 4 && h <= 4 - bitindex )
  { // ymaskhead and ymasktail in same 4x4 matrix row  --> combine to one
    if ( y >= dirtyscreen_ytiles - 4 ) // at bottom side of screen
    { // move one 4x4 matrix upwards to prevent acces outside 2D dimension
      t->tiles  -= 2 * dirtyscreen_xbitmaps;
      ymasktail &= ymaskhead;
      ymaskhead = 0;
    }
    else
    {
      ymaskhead &= ymasktail;
      ymasktail  = 0;
    }
  }

  // Check is this 'single' really is restricted to a 2x2 16bit area
  DebugCheck( (((w-1)>>2) + 1) > 2 || (((h-1)>>2) + 1) > 2 );

  // now using above head+tail masks, combine them to get 2x2 16bit masks
  t->lefttopmask     = xmaskhead & ymaskhead;
  t->righttopmask    = xmasktail & ymaskhead;
  t->leftbottommask  = xmaskhead & ymasktail;
  t->rightbottommask = xmasktail & ymasktail;

  return t;
}

/**
**	Add a new decoration for something that needs to be drawn on the
**	screen. You only need to supply the given information, after which the
**	mechanism itself determines (on the moment of a video refresh) what is
**	to be drawn.
**	When the object that the decoration represents changes in time, you
**	need to call the proepr mark directly function to denote this.
**	When the decoration is no longer needed, simply call DecorationDelete..
**
**	@param data	= any type of object that needs to be drawn, this
**			  mechanism will not need to know what it is.
**	@param drawclip	= function that can draw above data using draw rountines
**			  that can restrict drawing to a clip rectangle.
**	@param l	= depth-level; order in which de decoration is drawn
**	@param x	= x pixel position on screen of left-top
**	@param y	= y pixel position on screen of left-top
**	@param w	= width in pixels of area to be drawn from (x,y)
**	@param h	= height in pixels of area to be drawn from (x,y)
**/
Deco *DecorationAdd( void *data,
                     void (*drawclip)(void *data),
                     DecorationLevel l, 
                     unsigned x, unsigned y,
                     unsigned w, unsigned h )
{
  DecorationSingle **prevt;
  Deco *list, *d, *prv, **pprv;

  // Allocate and fill in this new DecorationType so it can be used
  d = DecorationAllocate();
  d->drawclip  = drawclip;
  d->data      = data;
  d->l         = l;
  d->x         = x;
  d->y         = y;
  d->w         = w;
  d->h         = h;

  // Restrict to screen (keeping original total location in d for check later)
  if( x<0 ) {
    int ofs=-x;
    if( w<=ofs ) {
      DecorationDelete( d );
      return NULL;
    }
    x=0;
    w-=ofs;
  }
  if( (x+w)>VideoWidth ) {
    if( x>=VideoWidth ) {
      DecorationDelete( d );
      return NULL;
    }
    w=VideoWidth-x;
  }
  if( y<0 ) {
    int ofs=-y;
    if( h<=ofs ) {
      DecorationDelete( d );
      return NULL;
    }
    y=0;
    h-=ofs;
  }
  if( (y+h)>VideoHeight ) {
    if( y>=VideoHeight ) {
      DecorationDelete( d );
      return NULL;
    }
    h=VideoHeight-y;
  }
  DebugCheck( x < 0 || y < 0 || w <= 0 || h <= 0 ||
              (x+w) > VideoWidth || (y+h) > VideoHeight );
  

  // Find entry for this decoration ordered on z(l):y:x and add it
  // @note we only need z-level really, but also do y:x to be able to draw
  // decorations of same z-levle on top of eachother as compatible with original
  // FIXME: use a smarter more faster method
  prv  = NULL;
  pprv = dhead + l;
  while ( (list = *pprv) && (list->y < y || (list->y == y && list->x < x) ) )
  {
    prv  = list;
    pprv = &list->nxt;
  }
  *pprv = d;
  d->prv = prv;
  d->nxt = list;
  if ( list )
    list->prv = d;

  // Split given area up into multiple Decorations of DIRTYSCREEN_DETAILSIZE
  // FIXME: can be done faster, or maybe we should do without?
  prevt = &d->singles;
  while ( h > DECOSINGLE_PIXELS )
  {
    int x2 = x;
    int w2 = w;
    while ( w2 > DECOSINGLE_PIXELS )
    {
      *prevt = DecorationSingleNew( x2, y,
                                    DECOSINGLE_PIXELS, DECOSINGLE_PIXELS );
      prevt = &(*prevt)->nxt;
      x2 += DECOSINGLE_PIXELS;
      w2 -= DECOSINGLE_PIXELS;
    }
    *prevt = DecorationSingleNew( x2, y, w2, DECOSINGLE_PIXELS );
    prevt = &(*prevt)->nxt;
    y += DECOSINGLE_PIXELS;
    h -= DECOSINGLE_PIXELS;
  }
  while ( w > DECOSINGLE_PIXELS )
  {
    *prevt = DecorationSingleNew( x, y, DECOSINGLE_PIXELS, h );
    prevt = &(*prevt)->nxt;
    x += DECOSINGLE_PIXELS;
    w -= DECOSINGLE_PIXELS;
  }
  *prevt = DecorationSingleNew( x, y, w, h );
  (*prevt)->nxt = NULL;

  // As this is a new Decoration to be put onto the screen, we mark its area
  DecorationMark(d);

  return d;
}

/**
**	Invalidate atleast that part of the screen that is marked dirty.
**	(note that not the minimum area, but a somehwat bigger area can be
**	 invalidated for efficiency reasons).
**/
static void InvalidateDirtyscreen(void)
{
  char *p;
  unsigned bits, pixelx, pixely, pixelstep, dirtylinesize, xcount, ycount;

  dirtylinesize = dirtyscreen_xbitmaps*2;
  ycount        = dirtyscreen_ybitmaps*2;
  pixelstep     = 4 << DIRTYSCREEN_BITDETAIL;
  p             = dirtyscreen;
  pixely        = 0;
  do
  {
    pixelx = 0;
    xcount = dirtylinesize;

  // First add all horizontal segments found on current dirtyline
    do
    {
      bits    = (p[1] << 8) | p[0];
      p      += 2;
      xcount--;

      if ( bits ) // one or more bits in 4x4 matrix set
      {
        int leftx = pixelx;
      // skip non-zero bits
        while ( xcount > 0 && *((unsigned *)p) )
        {
          p      += 2;
          pixelx += pixelstep;
          xcount--;
        }
        SweeplineAdd( leftx, pixelx+pixelstep-1, pixely );
      }
      pixelx += pixelstep;
    }
    while ( xcount > 0 );

    pixely += pixelstep;

  // Now check if some segments exist too long and needs invalidate
    SweeplineInvalidate( pixely );
  }
  while ( --ycount > 0 );

// Invalidate remaining rectangles, which have their shadow beyond VideoHeight
  SweeplineInvalidateAll();
}

/**
**	Draw all decorations, independently if they cover dirty pixels or not.
**	To be called only to redraw entire screen (like in the beginning),
**	or when the DecorationUpdateDisplay operates to slow.
**	FIXME: (which might
**	automaticly detected by checking the function-duration ?)
**
**/
void DecorationRefreshDisplay(void)
{
  Deco *d;
  int i;

// save clip rectangle
  PushClipping();

// Handle each decoration (not the singles)
  for ( i = 0; i < LevCount; i++ )
    for ( d = dhead[i]; d; d = d->nxt )
      DrawArea( d->x, d->y, d->w, d->h, d->data, d->drawclip );

  Invalidate();

// reset dirty array, to remember new updates for next refresh
  ClearDirtyscreen();

// restore clip rectangle
  PopClipping();
}

/**
**	Draw only those decorations which cover dirty pixels.
**	Cleaning those pixels again for a next time.
**	FIXME: when this operates slower then a normal DecorationRefreshDisplay,
**	       the DecorationRefreshDisplay needs to be called. This can be
**	       by measuring both function durations..
**/
void DecorationUpdateDisplay(void)
{
  DecorationSingle *t;
  Deco *d;
  int i;

// save clip rectangle
  PushClipping();

// Handle each decoration-single separately
  for ( i = 0; i < LevCount; i++ )
    for ( d = dhead[i]; d; d = d->nxt )
      for ( t = d->singles; t; t = t->nxt )
        CheckRedraw( d, t );

// FIXME: use followin function instead for speed.. never tried out though
//  InvalidateDirtyscreen();
  Invalidate();

// reset dirty array, to remember new updates for next refresh
  ClearDirtyscreen();

// restore clip rectangle
  PopClipping();
}

#endif

//@}
