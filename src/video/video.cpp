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
/**@name video.c	-	The universal video functions. */
//
//	(c) Copyright 1999-2001 by Lutz Sammer
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

global int ColorCycleAll;               /// Color Cycle with all palettes.

global int ClipX1;			/// current clipping top left
global int ClipY1;			/// current clipping top left
global int ClipX2;			/// current clipping bottom right
global int ClipY2;			/// current clipping bottom right

local Clip* Clips;			/// stack of all clips.

extern PaletteLink *palette_list;

#ifdef DEBUG
global unsigned AllocatedGraphicMemory;	/// Allocated memory for objects
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

    ///	Loaded system palette. 256-entries long, active system palette.
global Palette GlobalPalette[256];

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
    if( left>right ) { left^=right; right^=left; left^=right; }
    if( top>bottom ) { top^=bottom; bottom^=top; top^=bottom; }
    
    if( left<0 )    left=0;
    else if( left>=VideoWidth )		left=VideoWidth-1;
    if( top<0 )	    top=0;
    else if( top>=VideoHeight )		top=VideoHeight-1;
    if( right<0 )   right=0;
    else if( right>=VideoWidth )	right=VideoWidth-1;
    if( bottom<0 )  bottom=0;
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
    Clip* clip;

    clip=malloc(sizeof(Clip));
    clip->Next=Clips;
    clip->X1=ClipX1;
    clip->Y1=ClipY1;
    clip->X2=ClipX2;
    clip->Y2=ClipY2;
    Clips=clip;
}

/**
**	Pop current clipping.
*/
global void PopClipping(void)
{
    Clip* clip;

    clip=Clips;
    if( clip ) {
	Clips=clip->Next;
	ClipX1=clip->X1;
	ClipY1=clip->Y1;
	ClipX2=clip->X2;
	ClipY2=clip->Y2;
	free(clip);
    } else {
	ClipX1=0;
	ClipY1=0;
	ClipX2=VideoWidth;
	ClipY2=VideoHeight;
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
    // FIXME: remove the casts :{
    VideoSetPalette((VMemType*)title->Pixels);

#ifdef USE_SDL
    // FIXME: should be moved to system/hardware dependend part
    { extern SDL_Surface *Screen;		/// internal screen
    SDL_LockSurface(Screen);

    VideoMemory=Screen->pixels;
#endif

    // FIXME: bigger window ?
    VideoDrawSubClip(title,0,0
	,title->Width,title->Height
	,(VideoWidth-title->Width)/2,(VideoHeight-title->Height)/2);

#ifdef USE_SDL
    // FIXME: should be moved to system/hardware dependend part
    SDL_UnlockSurface(Screen); }
#endif
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
    FILE *fp;
    int i;
    
    if((fp=fopen(name,"rb")) == NULL) {
	fprintf(stderr,"Can't load palette %s\n",name);
	exit(-1);
    }

    for(i=0;i<256;i++){
	pal[i].r=fgetc(fp)<<2;
	pal[i].g=fgetc(fp)<<2;
	pal[i].b=fgetc(fp)<<2;
    }
    
    fclose(fp);
}

// FIXME: this isn't 100% correct
// Color cycling info - forest:
// 3	flash red/green	(attacked building on minimap)
// 38-47	cycle		(water)
// 48-56	cycle		(water-coast boundary)
// 202	pulsates red	(Circle of Power)
// 240-244	cycle		(water around ships, Runestone, Dark Portal)
// Color cycling info - swamp:
// 3	flash red/green	(attacked building on minimap)
// 4	pulsates red	(Circle of Power)
// 5-9	cycle		(Runestone, Dark Portal)
// 38-47	cycle		(water)
// 88-95	cycle		(waterholes in coast and ground)
// 240-244	cycle		(water around ships)
// Color cycling info - wasteland:
// 3	flash red/green	(attacked building on minimap)
// 38-47	cycle		(water)
// 64-70	cycle		(coast)
// 202	pulsates red	(Circle of Power)
// 240-244	cycle		(water around ships, Runestone, Dark Portal)
// Color cycling info - winter:
// 3	flash red/green	(attacked building on minimap)
// 40-47	cycle		(water)
// 48-54	cycle		(half-sunken ice-floe)
// 202	pulsates red	(Circle of Power)
// 205-207	cycle		(lights on christmas tree)
// 240-244	cycle		(water around ships, Runestone, Dark Portal)

/**
**	Color cycle for 8 bpp video mode.
**
**	FIXME: not correct cycles only palette of tileset.
**	FIXME: Also icons and some units use color cycling.
**	FIXME: must be configured by the tileset or global.
*/
global void ColorCycle8(void)
{
    int i;
    int x;
    VMemType8* pixels;

    if(ColorCycleAll){
      PaletteLink * current_link = palette_list;
      while(current_link != NULL){
	x=((VMemType8*)(current_link->Palette))[38];
	for( i=38; i<47; ++i ) {	// tileset color cycle
	  ((VMemType8*)(current_link->Palette))[i]=
	    ((VMemType8*)(current_link->Palette))[i+1];
	}
	((VMemType8*)(current_link->Palette))[47]=x;
	
	x=((VMemType8*)(current_link->Palette))[240];
	for( i=240; i<244; ++i ) {	// units/icons color cycle
	  ((VMemType8*)(current_link->Palette))[i]=
	    ((VMemType8*)(current_link->Palette))[i+1];
	}
	((VMemType8*)(current_link->Palette))[244]=x;
	current_link = current_link->Next;
      }
    } else { 

      //
      //	Color cycle tileset palette
      //
      pixels=TheMap.TileData->Pixels;
      x = pixels[38];
      for(i = 38; i < 47; ++i){
	pixels[i] = pixels[i+1];
      }
      pixels[47] = x;
      
      x=Pixels8[38];
      for( i=38; i<47; ++i ) {	// tileset color cycle
	Pixels8[i]=Pixels8[i+1];
      }
      Pixels8[47]=x;
      
      x=Pixels8[240];
      for( i=240; i<244; ++i ) {	// units/icons color cycle
	Pixels8[i]=Pixels8[i+1];
      }
      Pixels8[244]=x;
    }
      
    MapColorCycle();		// FIXME: could be little more informativer
    MustRedraw|=RedrawMap|RedrawInfoPanel;
}

/**
**	Color cycle for 16 bpp video mode.
**
**	FIXME: not correct cycles only palette of tileset.
**	FIXME: Also icons and some units use color cycling.
**	FIXME: must be configured by the tileset or global.
*/
global void ColorCycle16(void)
{
    int i;
    int x;
    VMemType16* pixels;

    if(ColorCycleAll){
      PaletteLink * current_link = palette_list;
      while(current_link != NULL){
	x=((VMemType16*)(current_link->Palette))[38];
	for( i=38; i<47; ++i ) {	// tileset color cycle
	  ((VMemType16*)(current_link->Palette))[i]=
	    ((VMemType16*)(current_link->Palette))[i+1];
	}
	((VMemType16*)(current_link->Palette))[47]=x;
	
	x=((VMemType16*)(current_link->Palette))[240];
	for( i=240; i<244; ++i ) {	// units/icons color cycle
	  ((VMemType16*)(current_link->Palette))[i]=
	    ((VMemType16*)(current_link->Palette))[i+1];
	}
	((VMemType16*)(current_link->Palette))[244]=x;
	current_link = current_link->Next;
      }
    } else { 

      //
      //	Color cycle tileset palette
      //
      pixels=TheMap.TileData->Pixels;
      x = pixels[38];
      for(i = 38; i < 47; ++i){
	pixels[i] = pixels[i+1];
      }
      pixels[47] = x;
      
      x=Pixels16[38];
      for( i=38; i<47; ++i ) {	// tileset color cycle
	Pixels16[i]=Pixels16[i+1];
      }
      Pixels16[47]=x;

      x=Pixels16[240];
      for( i=240; i<244; ++i ) {	// units/icons color cycle
	Pixels16[i]=Pixels16[i+1];
      }
      Pixels16[244]=x;
    }

    MapColorCycle();		// FIXME: could be little more informativer
    MustRedraw|=RedrawMap|RedrawInfoPanel;
}

/**
**	Color cycle for 24 bpp video mode.
**
**	FIXME: not correct cycles only palette of tileset.
**	FIXME: Also icons and some units use color cycling.
**	FIXME: must be configured by the tileset or global.
*/
global void ColorCycle24(void)
{
    int i;
    VMemType24 x;
    VMemType24* pixels;

    if(ColorCycleAll){
      PaletteLink * current_link = palette_list;
      while(current_link != NULL){
	x=((VMemType24*)(current_link->Palette))[38];
	for( i=38; i<47; ++i ) {	// tileset color cycle
	  ((VMemType24*)(current_link->Palette))[i]=
	    ((VMemType24*)(current_link->Palette))[i+1];
	}
	((VMemType24*)(current_link->Palette))[47]=x;
	
	x=((VMemType24*)(current_link->Palette))[240];
	for( i=240; i<244; ++i ) {	// units/icons color cycle
	  ((VMemType24*)(current_link->Palette))[i]=
	    ((VMemType24*)(current_link->Palette))[i+1];
	}
	((VMemType24*)(current_link->Palette))[244]=x;
	current_link = current_link->Next;
      }
    } else { 

      //
      //	Color cycle tileset palette
      //
      pixels=TheMap.TileData->Pixels;
      x = pixels[38];
      for(i = 38; i < 47; ++i){
	pixels[i] = pixels[i+1];
      }
      pixels[47] = x;
      
      x=Pixels24[38];
      for( i=38; i<47; ++i ) {	// tileset color cycle
	Pixels24[i]=Pixels24[i+1];
      }
      Pixels24[47]=x;
      
      x=Pixels24[240];
      for( i=240; i<244; ++i ) {	// units/icons color cycle
	Pixels24[i]=Pixels24[i+1];
      }
      Pixels24[244]=x;
    }

    MapColorCycle();		// FIXME: could be little more informativer
    MustRedraw|=RedrawMap|RedrawInfoPanel;
}

/**
**	Color cycle for 32 bpp video mode.
**
**	FIXME: not correct cycles only palette of tileset.
**	FIXME: Also icons and some units use color cycling.
**	FIXME: must be configured by the tileset or global.
*/
global void ColorCycle32(void)
{
    int i;
    int x;
    VMemType32* pixels;

    if(ColorCycleAll){
      PaletteLink * current_link = palette_list;
      while(current_link != NULL){
	x=((VMemType32*)(current_link->Palette))[38];
	for( i=38; i<47; ++i ) {	// tileset color cycle
	  ((VMemType32*)(current_link->Palette))[i]=
	    ((VMemType32*)(current_link->Palette))[i+1];
	}
	((VMemType32*)(current_link->Palette))[47]=x;
	
	x=((VMemType32*)(current_link->Palette))[240];
	for( i=240; i<244; ++i ) {	// units/icons color cycle
	  ((VMemType32*)(current_link->Palette))[i]=
	    ((VMemType32*)(current_link->Palette))[i+1];
	}
	((VMemType32*)(current_link->Palette))[244]=x;
	current_link = current_link->Next;
      }
    } else { 

      //
      //	Color cycle tileset palette
      //
      pixels=TheMap.TileData->Pixels;
      x = pixels[38];
      for(i = 38; i < 47; ++i){
	pixels[i] = pixels[i+1];
      }
      pixels[47] = x;
      
      x=Pixels32[38];
      for( i=38; i<47; ++i ) {	// tileset color cycle
	Pixels32[i]=Pixels32[i+1];
      }
      Pixels32[47]=x;
      
      x=Pixels32[240];
      for( i=240; i<244; ++i ) {	// units/icons color cycle
	Pixels32[i]=Pixels32[i+1];
      }
      Pixels32[244]=x;
    }

    MapColorCycle();		// FIXME: could be little more informativer
    MustRedraw|=RedrawMap|RedrawInfoPanel;
}

/**
**	Initializes system palette. Also calls SetPlayersPalette to set
**	palette for all players.
**
**	@param palette VMemType structure, as created by VideoCreateNewPalette
**	@see SetPlayersPalette
*/
global void VideoSetPalette(const VMemType* palette)
{
#if 0	// ARI: FIXME: This ruins menu palettes, when loaded via default.cm (introduce refcnt?)
    if( Pixels ) {
	free(Pixels);
    }
#endif
    Pixels=(VMemType*)palette;
    SetPlayersPalette();
}

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
    //	Init video sub modules
    //
    InitGraphic();
    InitLineDraw();
    InitSprite();
    InitCursor();
    switch( VideoDepth ) {
	case  8: ColorCycle=ColorCycle8 ; break;
	case 15:
	case 16: ColorCycle=ColorCycle16; break;
	case 24: ColorCycle=ColorCycle24; break;
	case 32: ColorCycle=ColorCycle32; break;
    }

    DebugLevel3(__FUNCTION__": %d %d\n",MapWidth,MapHeight);
}

//@}
