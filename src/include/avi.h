//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name avi.h - avi support */
//
//      (c) Copyright 2002-2004 by Lutz Sammer.
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
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
//      $Id$

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "iolib.h"

/*----------------------------------------------------------------------------
--  Declaration
----------------------------------------------------------------------------*/

/**
**  Avi frame buffer typedef
*/
typedef struct _avi_frame_buffer_ AviFrameBuffer;

/**
**  Avi frame buffer structure
**
**  Used to stored read and used frames.
*/
struct _avi_frame_buffer_ {
	AviFrameBuffer* Next;     /// Next buffer
	int             Length;   /// Buffer length
	unsigned char   Data[1];  /// Buffer data
};

/**
**  Avi file handle structure
*/
typedef struct _avi_file_ {
	CLFile*          FileHandle;       /// File handle
	// Video streams
	char             VideoCodec[8];    /// Video codec
	int              Width;            /// Video frame width
	int              Height;           /// Video frame height
	int              FPS100;           /// Frames per second * 100
	long             NumFrames;        /// Number of video frames
	int              VideoStream;      /// Video stream number
	unsigned long    VideoTag;         /// Video stream tag
	AviFrameBuffer*  VideoFrames;      /// Video frames
	AviFrameBuffer** VideoFramesTail;  /// Video frames tail pointer
	AviFrameBuffer*  VideoBuffer;      /// Current video frame buffer
	// Audio streams
	int              AudioStream;      /// Audio stream number
	unsigned long    AudioTag;         /// Audio stream tag
	AviFrameBuffer*  AudioFrames;      /// Audio frames
	AviFrameBuffer** AudioFramesTail;  /// Audio frames tail pointer
	AviFrameBuffer*  AudioBuffer;      /// Current audio frame buffer
	int              AudioRemain;      /// Remaining bytes in buffer
} AviFile;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

	/// Open an avi file
extern AviFile* AviOpen(const char* name);
	/// Close an avi file
extern void AviClose(AviFile* avi);

	/// Read next video frame
extern int AviReadNextVideoFrame(AviFile* avi, unsigned char** frame);
	/// Read next audio frame
extern int AviReadNextAudioFrame(AviFile* avi, unsigned char** frame);

	/// Play the sound of an avi movie
extern void PlayAviOgg(AviFile* avi);

//@}
