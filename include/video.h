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
//
//	(c) Copyright 1999,2000 by Lutz Sammer
//
//	$Id$
//

#ifndef __VIDEO_H__
#define __VIDEO_H__

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

typedef unsigned char VMemType8;	///  8 bpp modes pointer
typedef unsigned short VMemType16;	/// 16 bpp modes pointer
typedef struct { char a,b,c;} VMemType24;/// 24 bpp modes pointer
typedef unsigned long VMemType32;	/// 32 bpp modes pointer

/**
**	General video mode pointer.
**
**	@see VMemType8 @see VMemType16 @see VMemType24 @see VMemType32
*/
typedef union __vmem_type__ {
    VMemType8	D8;			///  8 bpp access
    VMemType16	D16;			/// 16 bpp access
    VMemType24	D24;			/// 24 bpp access
    VMemType32	D32;			/// 32 bpp access
} VMemType;

    /// MACRO defines speed of colorcycling FIXME: should be made configurable
#define COLOR_CYCLE_SPEED	(FRAMES_PER_SECOND/4)

// FIXME: not quite correct for new multiple palette version
    /// System-Wide used colors.
enum _sys_colors_ {
    ColorBlack = 0,			/// use for black
    ColorDarkGreen = 149,
    ColorBlue = 206,
    ColorWhite = 246,
    ColorNPC = 247,
    ColorGray = 248,
    ColorRed = 249,
    ColorGreen = 250,
    ColorYellow = 251,
    ColorBlinkRed = 252,
    ColorViolett = 253,

// FIXME: this should some where made configurable
    ColorWaterCycleStart = 38,		/// color # start for color cycling
    ColorWaterCycleEnd = 47,		/// color # end   for color cycling
    ColorIconCycleStart = 240,		/// color # start for color cycling
    ColorIconCycleEnd = 244		/// color # end   for color cycling
};

typedef enum _sys_colors_ SysColors;	/// System-Wide used colors.

typedef struct _palette_ Palette;	/// palette typedef

/// Palette structure.
struct _palette_ {
    unsigned char r;			/// red component
    unsigned char g;			/// green component
    unsigned char b;			/// blue component
};

typedef unsigned char GraphicData;	/// generic graphic data type

/**
**	General graphic object typedef. (forward)
*/
typedef struct _graphic_ Graphic;

/**
**	General graphic object type.
*/
typedef struct _graphic_type_ {
	/**
	**	Draw the object unclipped.
	**
	**	@param o	pointer to object
	**	@param f	number of frame (object index)
	**	@param x	x coordinate on the screen
	**	@param y	y coordinate on the screen
	*/
    void (*Draw)	(const Graphic* o,unsigned f,int x,int y);
	/**
	**	Draw the object unclipped and flipped in X direction.
	**
	**	@param o	pointer to object
	**	@param f	number of frame (object index)
	**	@param x	x coordinate on the screen
	**	@param y	y coordinate on the screen
	*/
    void (*DrawX)	(const Graphic* o,unsigned f,int x,int y);
	/**
	**	Draw the object clipped to the current clipping.
	**
	**	@param o	pointer to object
	**	@param f	number of frame (object index)
	**	@param x	x coordinate on the screen
	**	@param y	y coordinate on the screen
	*/
    void (*DrawClip)	(const Graphic* o,unsigned f,int x,int y);
	/**
	**	Draw the object clipped and flipped in X direction.
	**
	**	@param o	pointer to object
	**	@param f	number of frame (object index)
	**	@param x	x coordinate on the screen
	**	@param y	y coordinate on the screen
	*/
    void (*DrawClipX)	(const Graphic* o,unsigned f,int x,int y);
	/**
	**	Draw part of the object unclipped.
	**
	**	@param o	pointer to object
	**	@param gx	X offset into object
	**	@param gy	Y offset into object
	**	@param w	width to display
	**	@param h	height to display
	**	@param x	x coordinate on the screen
	**	@param y	y coordinate on the screen
	*/
    void (*DrawSub)	(const Graphic* o,int gx,int gy
			,unsigned w,unsigned h,int x,int y);
	/**
	**	Draw part of the object unclipped and flipped in X direction.
	**
	**	@param o	pointer to object
	**	@param gx	X offset into object
	**	@param gy	Y offset into object
	**	@param w	width to display
	**	@param h	height to display
	**	@param x	x coordinate on the screen
	**	@param y	y coordinate on the screen
	*/
    void (*DrawSubX)	(const Graphic* o,int gx,int gy
			,unsigned w,unsigned h,int x,int y);
	/**
	**	Draw part of the object clipped to the current clipping.
	**
	**	@param o	pointer to object
	**	@param gx	X offset into object
	**	@param gy	Y offset into object
	**	@param w	width to display
	**	@param h	height to display
	**	@param x	x coordinate on the screen
	**	@param y	y coordinate on the screen
	*/
    void (*DrawSubClip)	(const Graphic* o,int gx,int gy
			,unsigned w,unsigned h,int x,int y);
	/**
	**	Draw part of the object clipped and flipped in X direction.
	**
	**	@param o	pointer to object
	**	@param gx	X offset into object
	**	@param gy	Y offset into object
	**	@param w	width to display
	**	@param h	height to display
	**	@param x	x coordinate on the screen
	**	@param y	y coordinate on the screen
	*/
    void (*DrawSubClipX)(const Graphic* o,int gx,int gy
			,unsigned w,unsigned h,int x,int y);

	/**
	**	Draw the object unclipped and zoomed.
	**
	**	@param o	pointer to object
	**	@param f	number of frame (object index)
	**	@param x	x coordinate on the screen
	**	@param y	y coordinate on the screen
	**	@param z	Zoom factor X 10 (10 = 1:1).
	*/
    void (*DrawZoom)	(const Graphic* o,unsigned f,int x,int y,int z);

    // FIXME: add zooming functions.

	/*
	**	Free the object.
	**
	**	@param o	pointer to object
	*/
    void (*Free)	(Graphic* o);
} GraphicType;

/**
**	General graphic object
*/
struct _graphic_ {
	// cache line 0
    GraphicType*	Type;		/// Object type dependend
    void*		Frames;		/// Frames of the object
    void*		Pixels;		/// Pointer to local or global palette
    unsigned		Width;		/// Width of the object
	// cache line 1
    unsigned		Height;		/// Height of the object
    unsigned		NumFrames;	/// Number of frames
    unsigned		Size;		/// Size of frames
    Palette*		Palette;        /// Loaded Palette
	// cache line 2
    //void*		Offsets;	/// Offsets into frames
};

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

    /**
    **	Wanted videomode, fullscreen or windowed.
    */
extern char VideoFullScreen;

    /**
    **	Architecture-dependant video depth. Set by InitVideoXXX, if 0.
    **	(8,15,16,24,32)
    **	@see InitVideo @see InitVideoX11 @see InitVideoSVGA @see InitVideoSdl
    **	@see InitVideoWin32 @see main
    */
extern int VideoDepth;

    /**
    **	Architecture-dependant videomemory. Set by InitVideoXXX.
    **	FIXME: need a new function to set it, see #ifdef SDL code
    **	@see InitVideo @see InitVideoX11 @see InitVideoSVGA @see InitVideoSdl
    **	@see InitVideoWin32 @see VMemType
    */
extern VMemType* VideoMemory;

#define VideoMemory8	(&VideoMemory->D8)	/// video memory  8bpp
#define VideoMemory16	(&VideoMemory->D16)	/// video memory 16bpp
#define VideoMemory24	(&VideoMemory->D24)	/// video memory 24bpp
#define VideoMemory32	(&VideoMemory->D32)	/// video memory 32bpp

    /**
    **	Architecture-dependant system palette. Applies as conversion between
    **	GlobalPalette colors and their representation in videomemory.
    **	Set by VideoCreatePalette or VideoSetPalette.
    **	@see VideoCreatePalette @VideoSetPalette
    */
extern VMemType* Pixels;

#define Pixels8		(&Pixels->D8)		/// global pixels  8bpp
#define Pixels16	(&Pixels->D16)		/// global pixels 16bpp
#define Pixels24	(&Pixels->D24)		/// global pixels 24bpp
#define Pixels32	(&Pixels->D32)		/// global pixels 32bpp

    ///	Loaded system palette. 256-entries long, active system palette.
extern Palette GlobalPalette[256];

    /**
    **	Video synchronization speed. Synchronization time in prozent.
    **	If =0, video framerate is not synchronized. 100 is exact
    **	FRAMES_PER_SECOND (30). Game will try to redraw screen within
    **	intervals of VideoSyncSpeed, not more, not less.
    **	@see FRAMES_PER_SECOND @see VideoInterrupts
    */
extern int VideoSyncSpeed;

    /**
    **	Counter. Counts how many video interrupts occured, while proceed event
    **	queue. If <1 simply do nothing, =1 means that we should redraw screen.
    **	>1 means that framerate is too slow.
    **	@see CheckVideoInterrupts @VideoSyncSpeed
    */
extern volatile int VideoInterrupts;

    /**
    **	Draw pixel unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    */
extern void (*VideoDrawPixel)(SysColors color,int x,int y);

    /**
    **	Draw 25% translucent pixel (Alpha=64) unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    */
extern void (*VideoDraw25TransPixel)(SysColors color,int x,int y);

    /**
    **	Draw 50% translucent pixel (Alpha=128) unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    */
extern void (*VideoDraw50TransPixel)(SysColors color,int x,int y);

    /**
    **	Draw 75% translucent pixel (Alpha=192) unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    */
extern void (*VideoDraw75TransPixel)(SysColors color,int x,int y);

    /**
    **	Draw translucent pixel unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param alpha	alpha value of pixel.
    */
extern void (*VideoDrawTransPixel)(SysColors color,int x,int y,int alpha);

    /**
    **	Draw pixel clipped to current clip setting.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    */
extern void (*VideoDrawPixelClip)(SysColors color,int x,int y);

    /**
    **	Draw 25% translucent pixel clipped to current clip setting.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    */
extern void (*VideoDraw25TransPixelClip)(SysColors color,int x,int y);

    /**
    **	Draw 50% translucent pixel clipped to current clip setting.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    */
extern void (*VideoDraw50TransPixelClip)(SysColors color,int x,int y);

    /**
    **	Draw 75% translucent pixel clipped to current clip setting.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    */
extern void (*VideoDraw75TransPixelClip)(SysColors color,int x,int y);

    /**
    **	Draw translucent pixel clipped to current clip setting.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param alpha	alpha value of pixel.
    */
extern void (*VideoDrawTransPixelClip)(SysColors color,int x,int y,int alpha);

    /**
    **	Draw vertical line unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param height	height of line.
    */
extern void (*VideoDrawVLine)(SysColors color,int x,int y
	,unsigned height);

    /**
    **	Draw 25% translucent vertical line unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param height	height of line.
    */
extern void (*VideoDraw25TransVLine)(SysColors color,int x,int y
	,unsigned height);

    /**
    **	Draw 50% translucent vertical line unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param height	height of line.
    */
extern void (*VideoDraw50TransVLine)(SysColors color,int x,int y
	,unsigned height);

    /**
    **	Draw 75% translucent vertical line unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param height	height of line.
    */
extern void (*VideoDraw75TransVLine)(SysColors color,int x,int y
	,unsigned height);

    /**
    **	Draw translucent vertical line unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param height	height of line.
    **	@param alpha	alpha value of pixel.
    */
extern void (*VideoDrawTransVLine)(SysColors color,int x,int y
	,unsigned height,int alpha);

    /**
    **	Draw vertical line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param height	height of line.
    */
extern void (*VideoDrawVLineClip)(SysColors color,int x,int y
	,unsigned height);

    /**
    **	Draw 25% translucent vertical line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param height	height of line.
    */
extern void (*VideoDraw25TransVLineClip)(SysColors color,int x,int y
	,unsigned height);

    /**
    **	Draw 50% translucent vertical line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param height	height of line.
    */
extern void (*VideoDraw50TransVLineClip)(SysColors color,int x,int y
	,unsigned height);

    /**
    **	Draw 75% translucent vertical line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param height	height of line.
    */
extern void (*VideoDraw75TransVLineClip)(SysColors color,int x,int y
	,unsigned height);

    /**
    **	Draw translucent vertical line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param height	height of line.
    **	@param alpha	alpha value of pixel.
    */
extern void (*VideoDrawTransVLineClip)(SysColors color,int x,int y
	,unsigned height,int alpha);

    /**
    **	Draw horizontal line unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param width	width of line.
    */
extern void (*VideoDrawHLine)(SysColors color,int x,int y
	,unsigned width);

    /**
    **	Draw 25% translucent horizontal line unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param width	width of line.
    */
extern void (*VideoDraw25TransHLine)(SysColors color,int x,int y
	,unsigned width);

    /**
    **	Draw 50% translucent horizontal line unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param width	width of line.
    */
extern void (*VideoDraw50TransHLine)(SysColors color,int x,int y
	,unsigned width);

    /**
    **	Draw 75% translucent horizontal line unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param width	width of line.
    */
extern void (*VideoDraw75TransHLine)(SysColors color,int x,int y
	,unsigned width);

    /**
    **	Draw translucent horizontal line unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param width	width of line.
    **	@param alpha	alpha value of pixel.
    */
extern void (*VideoDrawTransHLine)(SysColors color,int x,int y
	,unsigned width,int alpha);

    /**
    **	Draw horizontal line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param width	width of line.
    */
extern void (*VideoDrawHLineClip)(SysColors color,int x,int y
	,unsigned width);

    /**
    **	Draw 25% translucent horizontal line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param width	width of line.
    */
extern void (*VideoDraw25TransHLineClip)(SysColors color,int x,int y
	,unsigned width);

    /**
    **	Draw 50% translucent horizontal line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param width	width of line.
    */
extern void (*VideoDraw50TransHLineClip)(SysColors color,int x,int y
	,unsigned width);

    /**
    **	Draw 75% translucent horizontal line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param width	width of line.
    */
extern void (*VideoDraw75TransHLineClip)(SysColors color,int x,int y
	,unsigned width);

    /**
    **	Draw translucent horizontal line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param width	width of line.
    **	@param alpha	alpha value of pixel.
    */
extern void (*VideoDrawTransHLineClip)(SysColors color,int x,int y
	,unsigned width,int alpha);

    /**
    **	Draw line unclipped.
    **
    **	@param color	Color index.
    **	@param sx	Source x coordinate on the screen
    **	@param sy	Source y coordinate on the screen
    **	@param dx	Destination x coordinate on the screen
    **	@param dy	Destination y coordinate on the screen
    */
extern void (*VideoDrawLine)(SysColors color,int sx,int sy,int dx,int dy);

    /**
    **	Draw 25% translucent line unclipped.
    **
    **	@param color	Color index.
    **	@param sx	Source x coordinate on the screen
    **	@param sy	Source y coordinate on the screen
    **	@param dx	Destination x coordinate on the screen
    **	@param dy	Destination y coordinate on the screen
    */
extern void (*VideoDraw25TransLine)(SysColors color,int sx,int sy
	,int dx,int dy);

    /**
    **	Draw 50% translucent line unclipped.
    **
    **	@param color	Color index.
    **	@param sx	Source x coordinate on the screen
    **	@param sy	Source y coordinate on the screen
    **	@param dx	Destination x coordinate on the screen
    **	@param dy	Destination y coordinate on the screen
    */
extern void (*VideoDraw50TransLine)(SysColors color,int sx,int sy
	,int dx,int dy);

    /**
    **	Draw 75% translucent line unclipped.
    **
    **	@param color	Color index.
    **	@param sx	Source x coordinate on the screen
    **	@param sy	Source y coordinate on the screen
    **	@param dx	Destination x coordinate on the screen
    **	@param dy	Destination y coordinate on the screen
    */
extern void (*VideoDraw75TransLine)(SysColors color,int sx,int sy
	,int dx,int dy);

    /**
    **	Draw translucent line unclipped.
    **
    **	@param color	Color index.
    **	@param sx	Source x coordinate on the screen
    **	@param sy	Source y coordinate on the screen
    **	@param dx	Destination x coordinate on the screen
    **	@param dy	Destination y coordinate on the screen
    **	@param alpha	alpha value of pixel.
    */
extern void (*VideoDrawTransLine)(SysColors color,int sx,int sy,int dx,int dy
	,int alpha);

    /**
    **	Draw line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param sx	Source x coordinate on the screen
    **	@param sy	Source y coordinate on the screen
    **	@param dx	Destination x coordinate on the screen
    **	@param dy	Destination y coordinate on the screen
    */
extern void (*VideoDrawLineClip)(SysColors color,int sx,int sy,int dx,int dy);

    /**
    **	Draw 25% translucent line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param sx	Source x coordinate on the screen
    **	@param sy	Source y coordinate on the screen
    **	@param dx	Destination x coordinate on the screen
    **	@param dy	Destination y coordinate on the screen
    */
extern void (*VideoDraw25TransLineClip)(SysColors color,int sx,int sy
	,int dx,int dy);

    /**
    **	Draw 50% translucent line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param sx	Source x coordinate on the screen
    **	@param sy	Source y coordinate on the screen
    **	@param dx	Destination x coordinate on the screen
    **	@param dy	Destination y coordinate on the screen
    */
extern void (*VideoDraw50TransLineClip)(SysColors color,int sx,int sy,
	int dx,int dy);

    /**
    **	Draw 75% translucent line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param sx	Source x coordinate on the screen
    **	@param sy	Source y coordinate on the screen
    **	@param dx	Destination x coordinate on the screen
    **	@param dy	Destination y coordinate on the screen
    */
extern void (*VideoDraw75TransLineClip)(SysColors color,int sx,int sy
	,int dx,int dy);

    /**
    **	Draw translucent line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param sx	Source x coordinate on the screen
    **	@param sy	Source y coordinate on the screen
    **	@param dx	Destination x coordinate on the screen
    **	@param dy	Destination y coordinate on the screen
    **	@param alpha	alpha value of pixel.
    */
extern void (*VideoDrawTransLineClip)(SysColors color,int sx,int sy
	,int dx,int dy,int alpha);

    /**
    **	Draw rectangle.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param h	height of rectangle.
    **	@param w	width of rectangle.
    */
extern void (*VideoDrawRectangle)(SysColors color,int x,int y
	,unsigned w,unsigned h);

    /**
    **	Draw 25% translucent rectangle.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param h	height of rectangle.
    **	@param w	width of rectangle.
    */
extern void (*VideoDraw25TransRectangle)(SysColors color,int x,int y
	,unsigned w,unsigned h);

    /**
    **	Draw 50% translucent rectangle.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param h	height of rectangle.
    **	@param w	width of rectangle.
    */
extern void (*VideoDraw50TransRectangle)(SysColors color,int x,int y
	,unsigned w,unsigned h);

    /**
    **	Draw 75% translucent rectangle.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param h	height of rectangle.
    **	@param w	width of rectangle.
    */
extern void (*VideoDraw75TransRectangle)(SysColors color,int x,int y
	,unsigned w,unsigned h);

    /**
    **	Draw translucent rectangle.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param h	height of rectangle.
    **	@param w	width of rectangle.
    **	@param alpha	alpha value of pixel.
    */
extern void (*VideoDrawTransRectangle)(SysColors color,int x,int y
	,unsigned w,unsigned h,int alpha);

    /**
    **	Draw rectangle clipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param h	height of rectangle.
    **	@param w	width of rectangle.
    */
extern void (*VideoDrawRectangleClip)(SysColors color,int x,int y
	,unsigned w,unsigned h);

    /**
    **	Draw 25% translucent rectangle clipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param h	height of rectangle.
    **	@param w	width of rectangle.
    */
extern void (*VideoDraw25TransRectangleClip)(SysColors color,int x,int y
	,unsigned w,unsigned h);

    /**
    **	Draw 50% translucent rectangle clipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param h	height of rectangle.
    **	@param w	width of rectangle.
    */
extern void (*VideoDraw50TransRectangleClip)(SysColors color,int x,int y
	,unsigned w,unsigned h);

    /**
    **	Draw 75% translucent rectangle clipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param h	height of rectangle.
    **	@param w	width of rectangle.
    */
extern void (*VideoDraw75TransRectangleClip)(SysColors color,int x,int y
	,unsigned w,unsigned h);

    /**
    **	Draw translucent rectangle clipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param h	height of rectangle.
    **	@param w	width of rectangle.
    **	@param alpha	alpha value of pixel.
    */
extern void (*VideoDrawTransRectangleClip)(SysColors color,int x,int y
	,unsigned w,unsigned h,int alpha);

    /// Does ColorCycling..
extern void (*ColorCycle)(void);

/*----------------------------------------------------------------------------
--	Macros
----------------------------------------------------------------------------*/

    /// Get the width of a single frame of a graphic object
#define VideoGraphicWidth(o)	((o)->Width)
    /// Get the height of a single frame of a graphic object
#define VideoGraphicHeight(o)	((o)->Height)
    /// Get the number of frames of a graphic object
#define VideoGraphicFrames(o)	((o)->NumFrames)

    ///	Draw a graphic object unclipped.
#define VideoDraw(o,f,x,y)	((o)->Type->Draw)((o),(f),(x),(y))
    ///	Draw a graphic object unclipped and flipped in X direction.
#define VideoDrawX(o,f,x,y)	((o)->Type->DrawX)((o),(f),(x),(y))
    ///	Draw a graphic object clipped to the current clipping.
#define VideoDrawClip(o,f,x,y)	((o)->Type->DrawClip)((o),(f),(x),(y))
    ///	Draw a graphic object clipped and flipped in X direction.
#define VideoDrawClipX(o,f,x,y)	((o)->Type->DrawClipX)((o),(f),(x),(y))

    ///	Draw a part of graphic object unclipped.
#define VideoDrawSub(o,ix,iy,w,h,x,y) \
	((o)->Type->DrawSub)((o),(ix),(iy),(w),(h),(x),(y))
    ///	Draw a part of graphic object unclipped and flipped in X direction.
#define VideoDrawSubX(o,ix,iy,w,h,x,y) \
	((o)->Type->DrawSubX)((o),(ix),(iy),(w),(h),(x),(y))
    ///	Draw a part of graphic object clipped to the current clipping.
#define VideoDrawSubClip(o,ix,iy,w,h,x,y) \
	((o)->Type->DrawSubClip)((o),(ix),(iy),(w),(h),(x),(y))
    ///	Draw a part of graphic object clipped and flipped in X direction.
#define VideoDrawSubClipX(o,ix,iy,w,h,x,y) \
	((o)->Type->DrawSubClipX)((o),(ix),(iy),(w),(h),(x),(y))

#if 0
// FIXME: not written
    ///	Draw a graphic object zoomed unclipped.
#define VideoDrawZoom(o,f,x,y,z) \
	((o)->Type->DrawZoom)((o),(f),(x),(y),(z))
    ///	Draw a graphic object zoomed unclipped flipped in X direction.
#define VideoDrawZoomX(o,f,x,y,z) \
	((o)->Type->DrawZoomX)((o),(f),(x),(y),(z))
    ///	Draw a graphic object zoomed clipped to the current clipping.
#define VideoDrawZoomClip(o,f,x,y,z) \
	((o)->Type->DrawZoomClip)((o),(f),(x),(y),(z))
    ///	Draw a graphic object zoomed clipped and flipped in X direction.
#define VideoDrawZoomClipX(o,f,x,y,z) \
	((o)->Type->DrawZoomClipX)((o),(f),(x),(y),(z))

    ///	Draw a part of graphic object zoomed unclipped.
#define VideoDrawZoomSub(o,ix,iy,w,h,x,y,z) \
	((o)->Type->DrawZoomSub)((o),(ix),(iy),(w),(h),(x),(y),(z))
    ///	Draw a part of graphic object zoomed unclipped flipped in X direction.
#define VideoDrawZoomSubX(o,ix,iy,w,h,x,y,z) \
	((o)->Type->DrawZoomSubX)((o),(ix),(iy),(w),(h),(x),(y),(z))
    ///	Draw a part of graphic object zoomed clipped to the current clipping.
#define VideoDrawZoomSubClip(o,ix,iy,w,h,x,y,z) \
	((o)->Type->DrawZoomSubClip)((o),(ix),(iy),(w),(h),(x),(y),(z))
    ///	Draw a part of graphic object zoomed clipped flipped in X direction.
#define VideoDrawZoomSubClipX(o,ix,iy,w,h,x,y,z) \
	((o)->Type->DrawZoomSubClipX)((o),(ix),(iy),(w),(h),(x),(y),(z))

#endif

    ///	Free a graphic object.
#define VideoFree(o)		((o)->Type->Free)((o))
    ///	Save (NULL) free a graphic object.
#define VideoSaveFree(o) \
	do { if( (o) ) ((o)->Type->Free)((o)); } while( 0 )

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// initialize the video part
extern void InitVideo(void);

    /**
    **	Invalidates selected area on window or screen. Use for accurate
    **	redrawing. in so
    **	@param x x screen coordinate
    **	@param y y screen coordinate
    **	@param w width in pixel
    **	@param h height in pixel
    */
extern void InvalidateArea(int x,int y,int w,int h);

    /// Simply invalidates whole window or screen.
extern void Invalidate(void);

    /**
    **	Realize videomemory. X11 implemenataion just does XFlush.
    **	SVGALIB without linear addressing should use this.
    */
extern void RealizeVideoMemory(void);

    /**
    **	Process all system events. This function also keeps synchronization
    **	of game.
    */
extern void WaitEventsAndKeepSync(void);

    ///	Load graphic from PNG file.
extern Graphic* LoadGraphicPNG(const char* name);

    /// New graphic
extern Graphic* NewGraphic(unsigned d,unsigned w,unsigned h);

    /// Make graphic
extern Graphic* MakeGraphic(unsigned,unsigned,unsigned,void*,unsigned);

    /// Load graphic
extern Graphic* LoadGraphic(const char* file);

    /// Load sprite
extern Graphic* LoadSprite(const char* file,unsigned w,unsigned h);

    /// Init graphic
extern void InitGraphic(void);

    /// Init sprite
extern void InitSprite(void);

    /// Init line draw
extern void InitLineDraw(void);

    ///	Draw circle.
extern void VideoDrawCircle(SysColors color,int x,int y,unsigned r);

    ///	Draw 25% translucent circle.
extern void VideoDraw25TransCircle(SysColors color,int x,int y,unsigned r);

    ///	Draw 50% translucent circle.
extern void VideoDraw50TransCircle(SysColors color,int x,int y,unsigned r);

    ///	Draw 75% translucent circle.
extern void VideoDraw75TransCircle(SysColors color,int x,int y,unsigned r);

    ///	Draw translucent circle.
extern void VideoDrawTransCircle(SysColors color,int x,int y,unsigned r
	,int alpha);

    ///	Draw circle clipped.
extern void VideoDrawCircleClip(SysColors color,int x,int y,unsigned r);

    ///	Draw 25% translucent circle clipped.
extern void VideoDraw25TransCircleClip(SysColors color,int x,int y,unsigned r);

    ///	Draw 50% translucent circle clipped.
extern void VideoDraw50TransCircleClip(SysColors color,int x,int y,unsigned r);

    ///	Draw 75% translucent circle clipped.
extern void VideoDraw75TransCircleClip(SysColors color,int x,int y,unsigned r);

    ///	Draw translucent circle clipped.
extern void VideoDrawTransCircleClip(SysColors color,int x,int y,unsigned r
	,int alpha);

    ///	Fill rectangle.
extern void VideoFillRectangle(SysColors color,int x,int y
	,unsigned w,unsigned h);

    ///	Fill 25% translucent rectangle.
extern void VideoFill25TransRectangle(SysColors color,int x,int y
	,unsigned w,unsigned h);

    ///	Fill 50% translucent rectangle.
extern void VideoFill50TransRectangle(SysColors color,int x,int y
	,unsigned w,unsigned h);

    ///	Fill 75% translucent rectangle.
extern void VideoFill75TransRectangle(SysColors color,int x,int y
	,unsigned w,unsigned h);

    ///	Fill translucent rectangle.
extern void VideoFillTransRectangle(SysColors color,int x,int y
	,unsigned w,unsigned h,int alpha);

    ///	Fill rectangle clipped.
extern void VideoFillRectangleClip(SysColors color,int x,int y
	,unsigned w,unsigned h);

    ///	Fill 25% translucent rectangle clipped.
extern void VideoFill25TransRectangleClip(SysColors color,int x,int y
	,unsigned w,unsigned h);

    ///	Fill 50% translucent rectangle clipped.
extern void VideoFill50TransRectangleClip(SysColors color,int x,int y
	,unsigned w,unsigned h);

    ///	Fill 75% translucent rectangle clipped.
extern void VideoFill75TransRectangleClip(SysColors color,int x,int y
	,unsigned w,unsigned h);

    ///	Fill translucent rectangle clipped.
extern void VideoFillTransRectangleClip(SysColors color,int x,int y
	,unsigned w,unsigned h,int alpha);

    ///	Fill circle.
extern void VideoFillCircle(SysColors color,int x,int y,unsigned r);

    ///	Fill 25% translucent circle.
extern void VideoFill25TransCircle(SysColors color,int x,int y,unsigned r);

    ///	Fill 50% translucent circle.
extern void VideoFill50TransCircle(SysColors color,int x,int y,unsigned r);

    ///	Fill 75% translucent circle.
extern void VideoFill75TransCircle(SysColors color,int x,int y,unsigned r);

    ///	Fill translucent circle.
extern void VideoFillTransCircle(SysColors color,int x,int y,unsigned r
	,int alpha);

    ///	Fill circle clipped.
extern void VideoFillCircleClip(SysColors color,int x,int y,unsigned r);

    ///	Fill 25% translucent circle clipped.
extern void VideoFill25TransCircleClip(SysColors color,int x,int y,unsigned r);

    ///	Fill 50% translucent circle clipped.
extern void VideoFill50TransCircleClip(SysColors color,int x,int y,unsigned r);

    ///	Fill 75% translucent circle clipped.
extern void VideoFill75TransCircleClip(SysColors color,int x,int y,unsigned r);

    ///	Fill translucent circle clipped.
extern void VideoFillTransCircleClip(SysColors color,int x,int y,unsigned r
	,int alpha);

    /**
    **	Set clipping for nearly all vector primitives. Functions which support
    **	clipping will be marked Clip. Set the system-wide clipping rectangle.
    **
    **	@param left	Left x coordinate
    **	@param top	Top y coordinate
    **	@param right	Right x coordinate
    **	@param bottom	Bottom y coordinate
    */
extern void SetClipping(int left,int top,int right,int bottom);

    /**
    **	Push current clipping.
    */
extern void PushClipping(void);

    /**
    **	Pop current clipping.
    */
extern void PopClipping(void);

    /**
    **	Load a picture and display it on the screen (full screen),
    **	changing the colormap and so on..
    **
    **	@param name name of the picture (file) to display
    */
extern void DisplayPicture(const char *name);

    /**
    **	Load palette from resource. Just loads palette, to set it use
    **	VideoCreatePalette, which sets system palette.
    **
    **	@param pal buffer to store palette (256-entries long)
    **	@param name resource file name
    **
    **	@see VideoCreatePalette
    */
extern void LoadRGB(Palette* pal,const char* name);

    /**
    **	Creates a hardware palette from an independend Palette struct.
    **
    **	@param palette	System independ palette structure.
    **
    **	@returns	A palette in hardware  dependend format.
    */
extern VMemType* VideoCreateNewPalette(const Palette* palette);

    /**
    **	Initialize Pixels[] for all players.
    **	(bring Players[] in sync with Pixels[])
    **
    **	@see VideoSetPalette
    */
extern void SetPlayersPalette(void);

    /**
    **	Initializes system palette. Also calls SetPlayersPalette to set
    **	palette for all players.
    **
    **	@param palette VMemType structure, as created by VideoCreateNewPalette
    **	@see SetPlayersPalette
    */
extern void VideoSetPalette(const VMemType* palette);

    /**
    **	Set the system hardware palette from an independend Palette struct.
    **
    **	@param palette	System independ palette structure.
    */
extern void VideoCreatePalette(const Palette* palette);

    ///	Initializes video synchronization.
extern void SetVideoSync(void);

    /// Prints warning if video is too slow..
extern void CheckVideoInterrupts(void);

    /// Toggle mouse grab mode
extern void ToggleGrabMouse(void);

//@}

#endif	// !__VIDEO_H__
