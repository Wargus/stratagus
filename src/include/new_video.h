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

#ifndef __NEW_VIDEO_H__
#define __NEW_VIDEO_H__

//@{

// Little NOTE: this should become the new video headerfile

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

#if 0
Note:	This new graphic object should generalize all the different objects
	currently used in ALE Clone, Graphic Image, Sprite, RLESprite.
	Also should generalize the handling of different hardwares.
	(8bit,16bit,...)
#endif

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
    unsigned char r;			/// RED COMPONENT
    unsigned char g;			/// GREEN COMPONENT
    unsigned char b;			/// BLUE COMPONENT
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
    void (*Draw)	(Graphic* o,unsigned f,int x,int y);
	/**
	**	Draw the object unclipped and flipped in X direction.
	**
	**	@param o	pointer to object
	**	@param f	number of frame (object index)
	**	@param x	x coordinate on the screen
	**	@param y	y coordinate on the screen
	*/
    void (*DrawX)	(Graphic* o,unsigned f,int x,int y);
	/**
	**	Draw the object clipped to the current clipping.
	**
	**	@param o	pointer to object
	**	@param f	number of frame (object index)
	**	@param x	x coordinate on the screen
	**	@param y	y coordinate on the screen
	*/
    void (*DrawClip)	(Graphic* o,unsigned f,int x,int y);
	/**
	**	Draw the object clipped and flipped in X direction.
	**
	**	@param o	pointer to object
	**	@param f	number of frame (object index)
	**	@param x	x coordinate on the screen
	**	@param y	y coordinate on the screen
	*/
    void (*DrawClipX)	(Graphic* o,unsigned f,int x,int y);
	/**
	**	Draw part of the object.
	**
	**	@param o	pointer to object
	**	@param gx	X offset into object
	**	@param gy	Y offset into object
	**	@param w	width to display
	**	@param h	height to display
	**	@param x	x coordinate on the screen
	**	@param y	y coordinate on the screen
	*/
    void (*DrawSub)	(Graphic* o,int gx,int gy
			,unsigned w,unsigned h,int x,int y);
    void (*DrawSubX)	(Graphic* o,int gx,int gy
			,unsigned w,unsigned h,int x,int y);
    void (*DrawSubClip)	(Graphic* o,int gx,int gy
			,unsigned w,unsigned h,int x,int y);
    void (*DrawSubClipX)(Graphic* o,int gx,int gy
			,unsigned w,unsigned h,int x,int y);
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
  GraphicType*	        Type;		/// Object type dependend
  Palette*              Palette;        /// Loaded Palette
  GraphicData*          Pixels;		/// Pointer to local or global palette
  unsigned		Width;		/// Width of the object
  unsigned		Height;		/// Height of the object
  unsigned	        NumFrames;	/// Number of frames
  void*		        Frames;		/// Frames of the object
};

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

    /**
    **	Wanted videomode, fullscreen or windowed.
    */
extern int VideoFullScreen;

    /**
    **	Draw pixel unclipped.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    */
extern void (*VideoDrawPixel)(SysColors color,int x,int y);

    /**
    **	Draw pixel clipped to current clip setting.
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    */
extern void (*VideoDrawPixelClip)(SysColors color,int x,int y);

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
    **	Draw horizontal line clipped to current clip setting
    **
    **	@param color	Color index.
    **	@param x	x coordinate on the screen
    **	@param y	y coordinate on the screen
    **	@param width	width of line.
    */
extern void (*VideoDrawHLineClip)(SysColors color,int x,int y
	,unsigned width);

/*----------------------------------------------------------------------------
--	Macros
----------------------------------------------------------------------------*/

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

    /// 	Free a graphic object.
#define VideoFree(o)		((o)->Type->Free)((o))
    /// 	Save (NULL) free a graphic object.
#define VideoSaveFree(o) \
	do { if( (o) ) ((o)->Type->Free)((o)); } while( 0 )

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

extern void InitVideo(void);		/// initialize the video part

    ///	Load graphic from PNG file.
extern Graphic* LoadGraphicPNG(const char* name);

    /// New graphic
extern Graphic* NewGraphic(unsigned d,unsigned w,unsigned h);

    /// Make graphic
extern Graphic* MakeGraphic(unsigned d,unsigned w,unsigned h,void* p);

    /// Load graphic
extern Graphic* LoadGraphic(const char* file);

    /// Init graphic
extern void InitGraphic(void);

    /// Init line draw
extern void InitLineDraw(void);

    ///	Draw rectangle.
extern void VideoDrawRectangle(SysColors color,int x,int y
	,unsigned w,unsigned h);

    ///	Fill rectangle.
extern void VideoFillRectangle(SysColors color,int x,int y
	,unsigned w,unsigned h);

    /**
    **	Load a picture and display it on the screen (full screen),
    **	changing the colormap and so on..
    **
    **	@param name name of the picture (file) to display
    */
extern void DisplayPicture(const char *name);

//@}

#endif	// !__NEW_VIDEO_H__
