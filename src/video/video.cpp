//   ___________		     _________		      _____  __
//   \_	  _____/______	 ____	____ \_	  ___ \____________ _/ ____\/  |_
//    |	   __) \_  __ \_/ __ \_/ __ \/	  \  \/\_  __ \__  \\	__\\   __\ 
//    |	    \	|  | \/\  ___/\	 ___/\	   \____|  | \// __ \|	|   |  |
//    \___  /	|__|	\___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________			     ______________________
//			  T H E	  W A R	  B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name video.c	-	The universal video functions. */
//
//	(c) Copyright 1999-2001 by Lutz Sammer
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the License,
//	or (at your option) any later version.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "freecraft.h"
#include "video.h"

#include "map.h"
#include "ui.h"
#include "cursor.h"
#include "iolib.h"

#ifdef USE_SDL
#include <SDL/SDL.h>
#endif

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

// JOHNS: This is needed, because later I want to support it all with the same
//	  executable, choosable at runtime.
#ifdef USE_X11
#define UseX11		1
#define UseSdl		0
#define UseSVGALib	0
#define UseWin32	0
#endif

#ifdef USE_SDL
#define UseX11		0
#define UseSdl		1
#define UseSVGALib	0
#define UseWin32	0
#endif

#ifdef USE_SVGALIB
#define UseX11		0
#define UseSdl		0
#define UseSVGALib	1
#define UseWin32	0
#endif

#ifdef noUSE_WIN32
#define UseX11		0
#define UseSdl		0
#define UseSVGALib	0
#define UseWin32	1
#endif

/**
**	Structure of pushed clippings.
*/
typedef struct _clip_ {
    struct _clip_*	Next;		/// next pushed clipping.
    int			X1;		/// pushed clipping top left
    int			Y1;		/// pushed clipping top left
    int			X2;		/// pushed clipping bottom right
    int			Y2;		/// pushed clipping bottom right
} Clip;

/*----------------------------------------------------------------------------
--	Externals
----------------------------------------------------------------------------*/

extern void InitVideoSdl(void);
extern void InitVideoX11(void);
extern void InitVideoSVGA(void);
extern void InitVideoWin32(void);

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

global char VideoFullScreen;		/// true fullscreen wanted

global int ColorCycleAll;		/// Flag Color Cycle with all palettes

global int ClipX1;			/// current clipping top left
global int ClipY1;			/// current clipping top left
global int ClipX2;			/// current clipping bottom right
global int ClipY2;			/// current clipping bottom right

local Clip* Clips;			/// stack of all clips
local Clip* ClipsGarbage;		/// garbage-list of available clips

#ifdef DEBUG
global unsigned AllocatedGraphicMemory; /// Allocated memory for objects
global unsigned CompressedGraphicMemory;/// memory for compressed objects
#endif

    /**
    **	Architecture-dependant video depth. Set by InitVideoXXX, if 0.
    **	(8,15,16,24,32)
    **	@see InitVideo @see InitVideoX11 @see InitVideoSVGA @see InitVideoSdl
    **	@see InitVideoWin32 @see main
    */
global int VideoDepth;

    /**
    **	Architecture-dependant video bpp (bits pro pixel).
    **	Set by InitVideoXXX. (8,16,24,32)
    **	@see InitVideo @see InitVideoX11 @see InitVideoSVGA @see InitVideoSdl
    **	@see InitVideoWin32 @see main
    */
global int VideoBpp;

    /**
    **	Architecture-dependant videomemory. Set by InitVideoXXX.
    **	FIXME: need a new function to set it, see #ifdef SDL code
    **	@see InitVideo @see InitVideoX11 @see InitVideoSVGA @see InitVideoSdl
    **	@see InitVideoWin32 @see VMemType
    */
global VMemType* VideoMemory;

    /**
    **	Architecture-dependant system palette. Applies as conversion between
    **	GlobalPalette colors and their representation in videomemory.
    **	Set by VideoCreatePalette or VideoSetPalette.
    **	@see VideoCreatePalette @VideoSetPalette
    */
global VMemType* Pixels;

global int VideoSyncSpeed=100;		/// 0 disable interrupts
global volatile int VideoInterrupts;	/// be happy, were are quicker

    /// Loaded system palette. 256-entries long, active system palette.
global Palette GlobalPalette[256];

    /**
    **  As a video 8bpp pixel color doesn't have RGB encoded, but denote some
    **  index (a value from contents Pixels) in a system pallete, the
    **  following precalculated arrays deliver a shortcut.
    **  NOTE: all array pointers are NULL in a non 8bpp mode
    **
    **  commonpalette:
    **  The single main color palette denoting all possible colors on which all
    **  other palettes (including above GlobalPalette) are based.
    **  Note:this means other palettes probably doesn't contains unique colors.
    **  
    **  commonpalette_defined:
    **  Denotes the defined entries (as bit index) in above palette.
    **  Needed as for X11 it is possible we can't get all 256 colors.
    **  
    **  colorcube8:
    **  Array of 32*32*32 system colors, to get from an unsigned int RGB
    **  (5x5x5 bit) to a system color.
    **  
    **  lookup25trans8:
    **  Array to get from two system colors as unsigned int (color1<<8)|color2
    **  to a new system color which is aproximately 75% color1 and 25% color2.
    **  lookup50trans8:
    **  The same for 50% color1 and 50% color2.
    **  
    **  VideoAllocPalette8:
    **  Funcytion to let hardware independent palette be converted (when set).
    */
global Palette   *commonpalette    = NULL;
global unsigned long commonpalette_defined[8];
global VMemType8 *colorcube8       = NULL;
global VMemType8 *lookup25trans8   = NULL;
global VMemType8 *lookup50trans8   = NULL;
global void (*VideoAllocPalette8)( Palette *palette,
                                   Palette *syspalette,
                                   unsigned long syspalette_defined[8] )=NULL;

    /// Does ColorCycling..
global void (*ColorCycle)(void);


/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Set clipping for graphic routines.
**
**	@param left	Left X screen coordinate.
**	@param top	Top Y screen coordinate.
**	@param right	Right X screen coordinate.
**	@param bottom	Bottom Y screen coordinate.
*/
global void SetClipping(int left,int top,int right,int bottom)
{
    IfDebug(
	if( left>right || top>bottom || left<0 || left>=VideoWidth
		|| top<0 || top>=VideoHeight || right<0
		|| right>=VideoWidth || bottom<0 || bottom>=VideoHeight ) {
	    DebugLevel0Fn("Wrong clipping, write cleaner code.\n");
	}
    );

    // Note this swaps the coordinates, if wrong ordered
    if( left>right ) { left^=right; right^=left; left^=right; }
    if( top>bottom ) { top^=bottom; bottom^=top; top^=bottom; }

    // Limit to video width, NOTE: this should check the caller
    if( left<0 )			left=0;
    else if( left>=VideoWidth )		left=VideoWidth-1;
    if( top<0 )				top=0;
    else if( top>=VideoHeight )		top=VideoHeight-1;
    if( right<0 )			right=0;
    else if( right>=VideoWidth )	right=VideoWidth-1;
    if( bottom<0 )			bottom=0;
    else if( bottom>=VideoHeight )	bottom=VideoHeight-1;

    ClipX1=left;
    ClipY1=top;
    ClipX2=right;
    ClipY2=bottom;
}

/**
**	Push current clipping.
*/
global void PushClipping(void)
{
    Clip *clip;

    if ((clip = ClipsGarbage)) {
	ClipsGarbage = ClipsGarbage->Next;
    } else {
	clip = malloc(sizeof(Clip));
    }

    clip->Next = Clips;
    clip->X1 = ClipX1;
    clip->Y1 = ClipY1;
    clip->X2 = ClipX2;
    clip->Y2 = ClipY2;
    Clips = clip;
}

/**
**	Pop current clipping.
*/
global void PopClipping(void)
{
    Clip *clip;

    clip = Clips;
    if (clip) {
	Clips = clip->Next;
	ClipX1 = clip->X1;
	ClipY1 = clip->Y1;
	ClipX2 = clip->X2;
	ClipY2 = clip->Y2;

	clip->Next = ClipsGarbage;
	ClipsGarbage = clip;
    } else {
	ClipX1 = 0;
	ClipY1 = 0;
	ClipX2 = VideoWidth;
	ClipY2 = VideoHeight;
    }
}

/**
**	Load a picture and display it on the screen (full screen),
**	changing the colormap and so on..
**
**	@param name	Name of the picture (file) to display.
*/
global void DisplayPicture(const char *name)
{
    Graphic* title;

    title=LoadGraphic(name);
    VideoSetPalette(title->Pixels);

    VideoLockScreen();

    // FIXME: bigger window ?
    VideoDrawSubClip(title,0,0
	,title->Width,title->Height
	,(VideoWidth-title->Width)/2,(VideoHeight-title->Height)/2);

    VideoUnlockScreen();

    VideoFree(title);
    // FIXME: (ARI:) New Palette got stuck in memory?
}

/**
**	Load palette from resource. Just loads palette, to set it use
**	VideoCreatePalette, which sets system palette.
**
**	@param pal buffer to store palette (256-entries long)
**	@param name resource file name
**
**	@see VideoCreatePalette
*/
global void LoadRGB(Palette *pal, const char *name)
{
    CLFile *fp;
    int i;
    unsigned char *p;
    unsigned char buffer[256*3];

    if( !(fp=CLopen(name)) || CLread(fp,buffer,256*3)!=256*3 ) {
	fprintf(stderr,"Can't load palette %s\n",name);
	exit(-1);
    }

    p=buffer;
    for( i=0;i<256;i++ ) {
	pal[i].r=(*p++)<<2;
	pal[i].g=(*p++)<<2;
	pal[i].b=(*p++)<<2;
    }

    CLclose(fp);
}

// FIXME: this isn't 100% correct
// Color cycling info - forest:
// 3	flash red/green (attacked building on minimap)
// 38-47	cycle		(water)
// 48-56	cycle		(water-coast boundary)
// 202	pulsates red	(Circle of Power)
// 240-244	cycle		(water around ships, Runestone, Dark Portal)
// Color cycling info - swamp:
// 3	flash red/green (attacked building on minimap)
// 4	pulsates red	(Circle of Power)
// 5-9	cycle		(Runestone, Dark Portal)
// 38-47	cycle		(water)
// 88-95	cycle		(waterholes in coast and ground)
// 240-244	cycle		(water around ships)
// Color cycling info - wasteland:
// 3	flash red/green (attacked building on minimap)
// 38-47	cycle		(water)
// 64-70	cycle		(coast)
// 202	pulsates red	(Circle of Power)
// 240-244	cycle		(water around ships, Runestone, Dark Portal)
// Color cycling info - winter:
// 3	flash red/green (attacked building on minimap)
// 40-47	cycle		(water)
// 48-54	cycle		(half-sunken ice-floe)
// 202	pulsates red	(Circle of Power)
// 205-207	cycle		(lights on christmas tree)
// 240-244	cycle		(water around ships, Runestone, Dark Portal)

/**
**	Color cycle for 8 bpp video mode.
**
**	FIXME: Also icons and some units use color cycling.
**	FIXME: must be configured by the tileset or global.
*/
local void ColorCycle8(void)
{
    int i;
    int x;
    VMemType8 *pixels;

    if (ColorCycleAll) {
	PaletteLink *current_link = PaletteList;

	while (current_link != NULL) {
	    x = ((VMemType8 *) (current_link->Palette))[38];
	    for (i = 38; i < 47; ++i) {	// tileset color cycle
		((VMemType8 *) (current_link->Palette))[i] =
		    ((VMemType8 *) (current_link->Palette))[i + 1];
	    }
	    ((VMemType8 *) (current_link->Palette))[47] = x;

	    x = ((VMemType8 *) (current_link->Palette))[240];
	    for (i = 240; i < 244; ++i) {	// units/icons color cycle
		((VMemType8 *) (current_link->Palette))[i] =
		    ((VMemType8 *) (current_link->Palette))[i + 1];
	    }
	    ((VMemType8 *) (current_link->Palette))[244] = x;
	    current_link = current_link->Next;
	}
    } else {

	//
	//        Color cycle tileset palette
	//
	pixels = TheMap.TileData->Pixels;
	x = pixels[38];
	for (i = 38; i < 47; ++i) {
	    pixels[i] = pixels[i + 1];
	}
	pixels[47] = x;

	x = Pixels8[38];
	for (i = 38; i < 47; ++i) {	// tileset color cycle
	    Pixels8[i] = Pixels8[i + 1];
	}
	Pixels8[47] = x;

	x = Pixels8[240];
	for (i = 240; i < 244; ++i) {	// units/icons color cycle
	    Pixels8[i] = Pixels8[i + 1];
	}
	Pixels8[244] = x;
    }

    MapColorCycle();		// FIXME: could be little more informativer
    MustRedraw |= RedrawMap | RedrawInfoPanel;
}

/**
**	Color cycle for 16 bpp video mode.
**
**	FIXME: Also icons and some units use color cycling.
**	FIXME: must be configured by the tileset or global.
*/
local void ColorCycle16(void)
{
    int i;
    int x;
    VMemType16 *pixels;

    if (ColorCycleAll) {
	PaletteLink *current_link = PaletteList;

	while (current_link != NULL) {
	    x = ((VMemType16 *) (current_link->Palette))[38];
	    for (i = 38; i < 47; ++i) {	// tileset color cycle
		((VMemType16 *) (current_link->Palette))[i] =
		    ((VMemType16 *) (current_link->Palette))[i + 1];
	    }
	    ((VMemType16 *) (current_link->Palette))[47] = x;

	    x = ((VMemType16 *) (current_link->Palette))[240];
	    for (i = 240; i < 244; ++i) {	// units/icons color cycle
		((VMemType16 *) (current_link->Palette))[i] =
		    ((VMemType16 *) (current_link->Palette))[i + 1];
	    }
	    ((VMemType16 *) (current_link->Palette))[244] = x;
	    current_link = current_link->Next;
	}
    } else {

	//
	//        Color cycle tileset palette
	//
	pixels = TheMap.TileData->Pixels;
	x = pixels[38];
	for (i = 38; i < 47; ++i) {
	    pixels[i] = pixels[i + 1];
	}
	pixels[47] = x;

	x = Pixels16[38];
	for (i = 38; i < 47; ++i) {	// tileset color cycle
	    Pixels16[i] = Pixels16[i + 1];
	}
	Pixels16[47] = x;

	x = Pixels16[240];
	for (i = 240; i < 244; ++i) {	// units/icons color cycle
	    Pixels16[i] = Pixels16[i + 1];
	}
	Pixels16[244] = x;
    }

    MapColorCycle();		// FIXME: could be little more informativer
    MustRedraw |= RedrawMap | RedrawInfoPanel;
}

/**
**	Color cycle for 24 bpp video mode.
**
**	FIXME: Also icons and some units use color cycling.
**	FIXME: must be configured by the tileset or global.
*/
local void ColorCycle24(void)
{
    int i;
    VMemType24 x;
    VMemType24 *pixels;

    if (ColorCycleAll) {
	PaletteLink *current_link = PaletteList;

	while (current_link != NULL) {
	    x = ((VMemType24 *) (current_link->Palette))[38];
	    for (i = 38; i < 47; ++i) {	// tileset color cycle
		((VMemType24 *) (current_link->Palette))[i] =
		    ((VMemType24 *) (current_link->Palette))[i + 1];
	    }
	    ((VMemType24 *) (current_link->Palette))[47] = x;

	    x = ((VMemType24 *) (current_link->Palette))[240];
	    for (i = 240; i < 244; ++i) {	// units/icons color cycle
		((VMemType24 *) (current_link->Palette))[i] =
		    ((VMemType24 *) (current_link->Palette))[i + 1];
	    }
	    ((VMemType24 *) (current_link->Palette))[244] = x;
	    current_link = current_link->Next;
	}
    } else {

	//
	//        Color cycle tileset palette
	//
	pixels = TheMap.TileData->Pixels;
	x = pixels[38];
	for (i = 38; i < 47; ++i) {
	    pixels[i] = pixels[i + 1];
	}
	pixels[47] = x;

	x = Pixels24[38];
	for (i = 38; i < 47; ++i) {	// tileset color cycle
	    Pixels24[i] = Pixels24[i + 1];
	}
	Pixels24[47] = x;

	x = Pixels24[240];
	for (i = 240; i < 244; ++i) {	// units/icons color cycle
	    Pixels24[i] = Pixels24[i + 1];
	}
	Pixels24[244] = x;
    }

    MapColorCycle();		// FIXME: could be little more informativer
    MustRedraw |= RedrawMap | RedrawInfoPanel;
}

/**
**	Color cycle for 32 bpp video mode.
**
**	FIXME: Also icons and some units use color cycling.
**	FIXME: must be configured by the tileset or global.
*/
local void ColorCycle32(void)
{
    int i;
    int x;
    VMemType32 *pixels;

    if (ColorCycleAll) {
	PaletteLink *current_link = PaletteList;

	while (current_link != NULL) {
	    x = ((VMemType32 *) (current_link->Palette))[38];
	    for (i = 38; i < 47; ++i) { // tileset color cycle
		((VMemType32 *) (current_link->Palette))[i] =
		    ((VMemType32 *) (current_link->Palette))[i + 1];
	    }
	    ((VMemType32 *) (current_link->Palette))[47] = x;

	    x = ((VMemType32 *) (current_link->Palette))[240];
	    for (i = 240; i < 244; ++i) {	// units/icons color cycle
		((VMemType32 *) (current_link->Palette))[i] =
		    ((VMemType32 *) (current_link->Palette))[i + 1];
	    }
	    ((VMemType32 *) (current_link->Palette))[244] = x;
	    current_link = current_link->Next;
	}
    } else {

	//
	//	  Color cycle tileset palette
	//
	pixels = TheMap.TileData->Pixels;
	x = pixels[38];
	for (i = 38; i < 47; ++i) {
	    pixels[i] = pixels[i + 1];
	}
	pixels[47] = x;

	x = Pixels32[38];
	for (i = 38; i < 47; ++i) {	// tileset color cycle
	    Pixels32[i] = Pixels32[i + 1];
	}
	Pixels32[47] = x;

	x = Pixels32[240];
	for (i = 240; i < 244; ++i) {	// units/icons color cycle
	    Pixels32[i] = Pixels32[i + 1];
	}
	Pixels32[244] = x;
    }

    MapColorCycle();		// FIXME: could be little more informativer
    MustRedraw |= RedrawMap | RedrawInfoPanel;
}

/*===========================================================================
Following functions support a single common palette for 8bpp
===========================================================================*/
/**
**      Fills a hardware independend palette with most common colors.
**      (Only really needed in 8bpp, to get a good representation of all color
**       possibilities.)
**      NOTE: define BPP8_WINSAFE or BPP8_IRGB for a different version.
**            X11 supports BPP8_NORMAL to prevent using a common palette
**            (this will deliver "color allocation error" though!)
**
**      @param palette  256 color palette, to be filled with most common RGB.
**
**      FIXME: Use TheUI settings (brightness, contrast and saturation) and
**      visual color range knowledge to reduce the ammount of colors needed.
*/
local void VideoFillCommonPalette8( Palette *palette )
{
#ifdef BPP8_WINSAFE /*---------------------------------------------------------
  This pallete generator is consider safe for Windows(tm), as static colors used
  by the system are kept and at the original locations.
  ---------------------------------------------------------------------------*/
  const unsigned char win_top[] = {
      0,   0,   0,
    128,   0,   0,
      0, 128,   0,
    128, 128,   0,
      0,   0, 128,
    128,   0, 128,
      0, 128, 128,
    192, 192, 192,
    192, 220, 192,
    166, 202, 240 };
  const unsigned char win_bottom[] = {
    255, 251, 240,
    160, 160, 164,
    128, 128, 128,
    255,   0,   0,
      0, 255,   0,
    255, 255,   0,
      0,   0, 255,
    255,   0, 255,
      0, 255, 255,
    255, 255, 255 };
  const unsigned char colorlevel[] = { 0, 87, 138, 181, 220, 255 };
  const unsigned char graylevel[]  = {
    47,  67,  82,  95, 106, 116, 125, 134, 142, 150, 157, 164, 171, 177,
   183, 189, 195, 201, 206, 212, 217, 222, 227, 232, 237, 241, 246, 251 };

  int i, r, g, b;

  /* Fill top of palette with static system colors */
  for ( i = 0; i <= 3*9; i += 3, palette++ )
  {
    palette->r = win_top[ i ];
    palette->g = win_top[ i + 1 ];
    palette->b = win_top[ i + 2 ];
  }

  /* Fill 6*6*6 colorcube (without values already present in static parts) */
  for ( r = 0; r <= 5; r++ )
    for ( g = 0; g <= 5; g++ )
      for ( b = 0; b <= 5; b++ )
        if ( (r && r != 5) || (g && g != 5) || (b && b != 5) )
        {
          palette->r = colorlevel[ r ];
          palette->g = colorlevel[ g ];
          palette->b = colorlevel[ b ];
          palette++;
        }

  /* Fill up remaining non-static part with grayshades */
  for ( i = 0; i <= 27; i++, palette++ )
    palette->r = palette->g = palette->b = graylevel[ i ];

  /* Fill bottom of palette with static system colors */
  for ( i = 0; i <= 3*9; i += 3, palette++ )
  {
    palette->r = win_bottom[ i ];
    palette->g = win_bottom[ i + 1 ];
    palette->b = win_bottom[ i + 2 ];
  }

#else
#ifdef BPP8_IRGB /*------------------------------------------------------------
  Palette generator using 8bit encoded as [IIRRGGBB], where I denotes the
  intensity of the RGB values along a grayshade axis. Which delivers a better
  spread out RGB range, but can not handle extreme values like 255:0:255
  ---------------------------------------------------------------------------*/
  int i, r, g, b;

  for ( i = 0; i <= 3*68; i+=68 )
    for ( r = 0; r <= 3*17; r+=17 )
      for ( g = 0; g <= 3*17; g+=17 )
        for ( b = 0; b <= 3*17; b+=17, palette++ )
        {
          palette->r = i+r;
          palette->g = i+g;
          palette->b = i+b;
        }

#else /* default ------------------------------------------------------------
  Experimental palette, defining a colorcube in a hshorter (most common) range
  and defining remaining as 40 grayshades over total range.
  This delivered best quality in 8bpp gameplay:
  - the large range for grayshades is valuable as many items need them
  - the shortened colorcube seems just to fit any colors needed.
  ---------------------------------------------------------------------------*/
  int i, r, g, b;

  /* Fill 6*6*6 colorcube (shortend and in lower RGB range) */
  for ( r = 0; r <= 5; r++ )
    for ( g = 0; g <= 5; g++ )
      for ( b = 0; b <= 5; b++, palette++ )
      {
        palette->r = 15+(127*r)/5;
        palette->g = 15+(127*g)/5;
        palette->b = 15+(127*b)/5;
      }

  /* Fill up remaining part with grayshades */
  for ( i = 0; i <= 39; i++, palette++ )
    palette->r = palette->g = palette->b = ((i-216)*255)/39;
#endif
#endif
}


/**
**      Fill a colorcube to get from a RGB (5x5x5 bit) to a system 8bpp color
**
**      @param palette  Array of 256 bytes with hardware dependent color as
**                      index, delivers RGB value (each in range 0..255).
**
**      @param pal_def  Denotes which entries (as bit index) in above palette
**                      are defined (and so may be used in colorcube).
**                      NOTE: atleast one defined entry should be available
**
**      @param cube     Array of 32768 (32*32*32) bytes with RGB value (each in
**                      range 0..31) as index, delivers color index.
*/
local void VideoFillColorcube8( const Palette *palette,
                                 const unsigned long pal_def[8],
                                       VMemType8 *cube )
{
  int r, g, b, i;

  for ( r = 0; r <= 255; r+=8 )
    for ( g = 0; g <= 255; g+=8 )
      for ( b = 0; b <= 255; b+=8 )
      {
        const Palette *pal;
        long int mindistance = 255*255*3+1;
        int colorfound = 0;

      // seek closest color in given palette
        for ( pal = palette, i = 0; i <= 255; pal++, i++ )
        {
          unsigned long bit;

          bit = 1 << (i&0x1F);
          if ( pal_def[ i>>5 ] & bit )
          {
            long int distance, xr, xg, xb;

            xr = (long int)pal->r - r;
            xb = (long int)pal->b - b;
            xg = (long int)pal->g - g;
            distance = xr*xr + xb*xb + xg*xg;
            if ( distance < mindistance )
            {
              mindistance=distance;
              colorfound=i;
            }
          }
        }

      // refer RGB to the system color (palette index) found
        *cube++ = colorfound;
      //  fprintf( stderr, "%d %d %d = %d %d %d\n",
      //           r, g, b, palette[colorfound].r, palette[colorfound].g,
      //           palette[colorfound].b );
      }
}

/**
**      Find a new hardware dependend palette, re-using the colors as set in
**      the colorcube.
**
**      @param cube     A 5x5x5 bit colorcube to get from RGB to system color.
**
**      @param palette  Hardware independend palette of 256 colors.
**
**      @return         A hardware dependend 8bpp pixel table.
**
*/
global VMemType8* VideoFindNewPalette8( const VMemType8 *cube,
                                        const Palette *palette )
{
  VMemType8 *newpixels, *p;
  int i;

  newpixels=p=malloc(256*sizeof(VMemType8));

  i=256;
  do
  {
    int r, g, b;

    //FIXME: find a faster way, with rounding..
    //r = palette->r>>3;
    //g = palette->g>>3;
    //b = palette->b>>3;
    r = (palette->r*31+15)/255;
    g = (palette->g*31+15)/255;
    b = (palette->b*31+15)/255;

    palette++;
    *p++ = cube[ (r<<10) | (g<<5) | b ];
  } while ( --i > 0 );

  return newpixels;
}

/**
**      Fill a lookup table to get from two system colors 8bpp as unsigned int
**      (color1<<8)|color2 to a new system color which is alpha% color2 and
**      (100-alpha)% color1.
**
**      @param palette	Already filled 256 color palette, to get from system
**                      color to RGB.
**
**      @param cube	Already filled 32*32*32 array of system colors, to get
**                      from RGB (as 5x5x5 bit) back to system color.
**
**      @param lookup	Allocated 256*256 array of system colors, to be filled
**                      as a lookup table for transparency alpha.
**
**      @param alpha	value in 0..255 denoting 0..100% transparency
*/
local void FillTransLookup8( const Palette *palette,
                              const VMemType8 *cube,
                                    VMemType8 *lookup,
                                    unsigned char alpha )

{
  const Palette *p1, *p2;
  unsigned int i, j;
  unsigned int alpha1,alpha2,r1, g1, b1, r2, g2, b2;

  alpha1=255-alpha;
  alpha2=alpha;
  for ( p1=palette, i = 256; i > 0; i-- )
  {
    r1 = alpha1*(unsigned int)p1->r;
    g1 = alpha1*(unsigned int)p1->g;
    b1 = alpha1*(unsigned int)p1->b;
    p1++;
    for ( p2=palette, j = 256; j > 0; j-- )
    {
      r2 = (r1 + alpha2*(unsigned int)p2->r + 255*4)/(255*8);
      g2 = (g1 + alpha2*(unsigned int)p2->g + 255*4)/(255*8);
      b2 = (b1 + alpha2*(unsigned int)p2->b + 255*4)/(255*8);
      p2++;

      *lookup++ = cube[ (r2<<10) | (g2<<5) | b2 ];
    }
  }
}

/**
**      Initialize globals based on a single common palette of 256 colors.
**      Only needed for 8bpp, which hasn't RGB encoded in its system color.
**      FIXME: should be called again when it gets dependent of TheUI settings
**             then call VideoFreePalette first to prevent "can not allocate"
*/
local void InitSingleCommonPalette8( void )
{
  Palette *tmp;
  int i;

  if ( (tmp=malloc(256*sizeof(Palette))) &&
       (!VideoAllocPalette8 || (commonpalette=malloc(256*sizeof(Palette)))) &&
       (colorcube8=malloc(32*32*32*sizeof(VMemType8))) &&
       (lookup25trans8=malloc(256*256*sizeof(VMemType8))) &&
       (lookup50trans8=malloc(256*256*sizeof(VMemType8))) )
  {
  // Create one allocated pallette of 256 colors to be used by all
  // palettes created later.
  // This prevents "can not allocate color" errors, but the downside is
  // that a fullscreen graphic can not use its own pallette to the fullest
  // (not all colors might be present in the system pallete).
  VideoFillCommonPalette8(tmp);
  //for ( i=0; i<=255; i++ )
  //  fprintf( stderr, "%d %d %d\n", tmp[i].r, tmp[i].g, tmp[i].b );

  if ( VideoAllocPalette8 )
  { // Palette needs to be converted to hardware dependent palette
    VideoAllocPalette8(tmp,commonpalette,commonpalette_defined);
    free( tmp );
  }
  else
  { // Use palette AS-IS FIXME: unused at the moment..
    commonpalette = tmp;
    for ( i=0; i<=7; i++ )
      commonpalette_defined[i]=0xFFFFFFFF;
  }

  // Create a colorcube to easily get from RGB back to system color
  VideoFillColorcube8(commonpalette,commonpalette_defined,colorcube8);

  // Create lookup tables to get from one system color to another.
  //FIXME: With max 256 unique colors in above colorcube, each RGB axis can
  //       contain 3root(256)=6.3496.. variations. Currently only 5
  //       supported (0,25,50,75,100% with/without use of lookup tables).
  //       So extra levels 12.5% and 37.5% needed for better representation.
  FillTransLookup8(commonpalette,colorcube8,lookup25trans8,(255+2)/4);
  FillTransLookup8(commonpalette,colorcube8,lookup50trans8,(255+1)/2);
  }
  else
  {
    fprintf( stderr, "Out of memory for special 8bpp display mode\n"
                     "Try another mode if you're low on memory\n" );
    exit( -1 );
  }
}

/*===========================================================================*/

/**
**	Initializes system palette. Also calls SetPlayersPalette to set
**	palette for all players.
**
**	@param palette VMemType structure, as created by VideoCreateNewPalette
**
**	@see SetPlayersPalette
*/
global void VideoSetPalette(const VMemType* palette)
{
#if 0	// ARI: FIXME: This ruins menu palettes, when loaded via default.cm (introduce refcnt?)
    if( Pixels ) {
	free(Pixels);//FIXME: free unsufficient, XFreeColors needed for X11
    }
#endif
    Pixels=(VMemType*)palette;
    SetPlayersPalette();
}

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Set the system hardware palette from an independend Palette struct.
**
**	@param palette	System independ palette structure.
*/
global void VideoCreatePalette(const Palette* palette)
{
    VMemType* temp;

    temp = VideoCreateNewPalette(palette);

    VideoSetPalette(temp);
}

/**
**	Lock the screen for write access.
*/
global void VideoLockScreen(void)
{
#ifdef USE_SDL
    // FIXME: move to system api part!
    extern SDL_Surface *Screen;			/// internal screen

    SDL_LockSurface(Screen);
    VideoMemory=Screen->pixels;
#endif
}

/**
**	Unlock the screen for write access.
*/
global void VideoUnlockScreen(void)
{
#ifdef USE_SDL
    // FIXME: move to system api part!
    extern SDL_Surface *Screen;			/// internal screen

    SDL_UnlockSurface(Screen);
    VideoMemory=Screen->pixels;
#endif
}

/**
**	Return ticks in ms since start.
*/
global unsigned long GetTicks(void)
{
#if UseSdl
    return SDL_GetTicks();
#endif
#if UseX11
    extern unsigned long X11GetTicks(void);

    return X11GetTicks();
#endif
#if UseSVGALib
    extern unsigned long SVGAGetTicks(void);

    return SVGAGetTicks();
#endif
}

/**
**	Video initialize.
*/
global void InitVideo(void)
{
#ifdef __OPTIMIZE__
    if( UseSdl ) {
	InitVideoSdl();
    } else if( UseX11 ) {
	InitVideoX11();
    } else if( UseSVGALib ) {
	InitVideoSVGA();
    } else if( UseWin32 ) {
	InitVideoWin32();
    } else {
	IfDebug( abort(); );
    }
#else
    #if UseSdl
	InitVideoSdl();
    #else
	#if UseX11
	    InitVideoX11();
	#else
	    #if UseSVGALib
		InitVideoSVGA();
	    #else
		#if UseWin32
		    InitVideoWin32();
		#else
		    abort();
		#endif
	    #endif
	#endif
    #endif
#endif

    //
    //	Use single common palette to be used for all palettes in 8bpp
    //
    #ifndef BPP8_NORMAL
    if ( UseX11 && VideoBpp == 8 ) // FIXME: to be extended for all video..
      InitSingleCommonPalette8();
    #endif


    //
    //	Init video sub modules
    //
    InitGraphic();
    InitLineDraw();
    InitSprite();
    InitCursors();
    switch( VideoBpp ) {
	case  8: ColorCycle=ColorCycle8 ; break;
	case 15:
	case 16: ColorCycle=ColorCycle16; break;
	case 24: ColorCycle=ColorCycle24; break;
	case 32: ColorCycle=ColorCycle32; break;
    }

    DebugLevel3Fn("%d %d\n",MapWidth,MapHeight);
}

//@}
