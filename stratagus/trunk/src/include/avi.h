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
/**@name avi.h			-	avi support */
//
//	(c) Copyright 2002 by Lutz Sammer.
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
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

#include "iolib.h"

/*----------------------------------------------------------------------------
--	Declaration
----------------------------------------------------------------------------*/

/**
**	Avi file handle structure
*/
typedef struct _avi_file_ {
    CLFile*	FileHandle;		/// File handle
    // Video streams
    char	VideoCodec[8];		/// Video codec
    int		Width;			/// Video frame width
    int		Height;			/// Video frame height
    int		FPS100;			/// Frames per second * 100
    long	NumFrames;		/// Number of video frames
    int		VideoStream;		/// Video stream number
    unsigned long VideoTag;		/// Video stream tag
    // Audio streams
    int		AudioStream;		/// Audio stream number
} AviFile;


/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// Open an avi file
extern AviFile* AviOpen(const char* name);

    /// Close an avi file
extern void AviClose(AviFile* avi);

    /// Read next video frame
extern int AviReadNextFrame(AviFile* avi,unsigned char** frame);

//@}
