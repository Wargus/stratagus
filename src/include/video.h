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
/**@name video.h	-	The video headerfile. */
/*
**	(c) Copyright 1998-2000 by Lutz Sammer
**
**	$Id$
*/

#ifndef __VIDEO_H__
#define __VIDEO_H__

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "new_video.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

typedef unsigned long VMemType32;	/// 32 bpp modes pointer
typedef unsigned short VMemType16;	/// 16 bpp modes pointer
typedef unsigned char VMemType8;	///  8 bpp modes pointer

    /// MACRO defines speed of colorcycling
#define COLOR_CYCLE_SPEED	(FRAMES_PER_SECOND/4)

#ifndef NEW_VIDEO	// { Should be removed with new video final version

/**@name RleSprite */
//@{

/** RleSprite structure. Transparent, single frame, not packed image.
  Fast enough. Primarily used for fonts. color 255 is transparent.
  @see LoadRleSprite
  @see DrawRleSprite
  @see DrawRleSpriteClipped
  @see DrawRleSpriteClippedX
  @see DrawRleSpriteX
  @see FreeRleSprite
 */
struct RleSprite {
    unsigned		Width;		/// width of a frame
    unsigned		Height;		/// height of a frame
    GraphicData*	Pixels;		/// pointer to local or global palette 
    int			NumFrames;	/// number of frames
    IfDebug( int ByteSize; )
    /**
    **	pointer to the frames. frames are bitmaps converted via global
    **	Pixels[] structure or local object Pixels (if local Pixels != 0).
    */
    unsigned char*	Frames[0];
};

typedef struct RleSprite RleSprite;

/** Load RLE Sprite. new memory block is allocated.
  @param name resource file name
  @param w width of a frame
  @param h height of a frame
 */
extern RleSprite* LoadRleSprite(const char* name,unsigned w,unsigned h);

/** Draw RLE Sprite..
  @param sprite pointer to RLE Sprite OBJECT
  @param frame number of frame
  @param x x coordinate on the screen
  @param y y coordinate on the screen
 */
extern void DrawRleSprite(RleSprite* sprite,unsigned frame,int x,int y);

/** Draw RLE Sprite FLIPPED X..
  @param sprite pointer to RLE Sprite OBJECT
  @param frame number of frame
  @param x x coordinate on the screen
  @param y y coordinate on the screen
 */
extern void DrawRleSpriteX(RleSprite* sprite,unsigned frame,int x,int y);

/** Draw RLE Sprite CLIPPED..
  @param sprite pointer to RLE Sprite OBJECT
  @param frame number of frame
  @param x x coordinate on the screen
  @param y y coordinate on the screen
 */
extern void DrawRleSpriteClipped(RleSprite* sprite,unsigned frame,int x,int y);

/** Draw RLE Sprite CLIPPED FLIPPED X..
  @param sprite pointer to RLE Sprite OBJECT
  @param frame number of frame
  @param x x coordinate on the screen
  @param y y coordinate on the screen
 */
extern void DrawRleSpriteClippedX(RleSprite* sprite,unsigned frame,int x,int y);

/** Free pointer returned by LoadRleSprite. Dereferencing pointer is no longer
  correct.
  @param sprite pointer returned by LoadRleSprite
 */
extern void FreeRleSprite(RleSprite *sprite);
//@}

#endif	// } !NEW_VIDEO

/** Set clipping for nearly all vector primitives. Functions which support
  clipping will be marked CLIPPED. Set system-wide clipping rectangle.
  @param left left x coordinate
  @param top  top y coordinate
  @param right right x coordinate
  @param bottom bottom y coordinate
 */
extern void SetClipping(int left,int top,int right,int bottom);

/** Architecture-dependant videomemory. Set by InitVideoXXX.
  @see InitVideo
  @see InitVideoX11
  @see InitVideoSVGA
  @see InitVideoSdl
  @see InitVideoWin32
 */
extern void* VideoMemory;

/** Architecture-dependant video depth. Set by InitVideoXXX.
  @see InitVideo
  @see InitVideoX11
  @see InitVideoSVGA
  @see InitVideoSdl
  @see InitVideoWin32
*/
extern int VideoDepth;
    /// Sub depth 555, 565
//extern int VideoSubDepth;

    /// 32 bpp modes video memory address
#define VideoMemory32	((VMemType32*)VideoMemory)
    /// 16 bpp modes video memory address
#define VideoMemory16	((VMemType16*)VideoMemory)
    ///8 bpp modes video memory address
#define VideoMemory8	((VMemType8*)VideoMemory)

/** Architecture-dependant system palette. Applies as conversion between
  GlobalPalette colors and their representation in videomemory. Set by
  VideoCreatePalette.
  @see VideoCreatePalette
 */
extern VMemType8 * Pixels8;	///  8 bpp
extern VMemType16 * Pixels16;	/// 16 bpp
extern VMemType32 * Pixels32;	/// 32 bpp

/** Set videomode. Tries to set choosen videomode. Only 640x480, 800x600
  and 1024x768 are available. If videoadapter is not good enough module will
  return failure. Default mode is 640x480.
  @return 1 if videomode set correctly; 0 otherwise. */
extern int SetVideoMode(int width);

/** Loaded system palette. 256-entries long, active system palette. */
extern Palette GlobalPalette[256];

/** Load palette from resource. Just loads palette, to set it use
  VideoCreatePalette, which sets system palette.
  @param pal buffer to store palette (256-entries long)
  @param name resource file name
 */
extern void LoadRGB(Palette *pal, const char *name);

/// Initialize Pixels[] for all players (bring Players[] in sync with Pixels[])
extern void SetPlayersPalette(void);

/**@name Architecture-dependant video functions */
//@{

/// Initializes video synchronization..
extern void InitVideoSync(void);

/// Prints warning if video is too slow..
extern void CheckVideoInterrupts(void);

/// Does ColorCycling..
extern void ColorCycle(void);

/** Creates a palette from a Palette struct */
extern GraphicData * VideoCreateNewPalette(const Palette *palette);

/** Initializes system palette. Also calls SetPlayersPalette to set palette for
  all players.
  @param palette GraphicData structure, as created by VideoCreateNewPalette
  @see SetPlayersPalette
 */
extern void VideoSetPalette(const GraphicData *palette);

/// OBSOLETE: Calls VideoCreateNewPalette with the palette argument, and then
/// feeds the result to VideoSetPalette to initialize system palette with it.
extern void VideoCreatePalette(const Palette *palette);

/// Process all system events. This function also keeps synchronization of game.
extern void WaitEventsAndKeepSync(void);

/** Realize videomemory. X11 implemenataion just does XFlush. SVGALIB without
 linear addressing should use this. */
extern void RealizeVideoMemory(void);

/// Initialize video hardware..
extern void GameInitDisplay(void);

/** Invalidates selected area on window or screen. Use for accurate redrawing.
  in so
  @param x x coordinate
  @param y y coordinate
  @param w width
  @param h height
 */
extern void InvalidateArea(int x,int y,int w,int h);

/// Simply invalidates whole window or screen.. 
extern void Invalidate(void);

/// Toggle mouse grab mode
extern void ToggleGrabMouse(void);

//@}

/** Counter. Counts how many video interrupts occured, while proceed event
  queue. If <1 simply do nothing, =1 means that we should redraw screen. >1
  means that framerate is too slow.
  @see CheckVideoInterrupts
 */
extern volatile int VideoInterrupts;

/**
** Video synchronization speed. Synchronization time in milliseconds.
** If =0, video framerate is not synchronized. Game will try to redraw screen
** within intervals of VideoSyncSpeed, not more, not less
*/
extern int VideoSyncSpeed;

/**
**	Wanted videomode, fullscreen or windowed.
*/
extern int VideoFullScreen;

//@}

#endif	// !__VIDEO_H__
