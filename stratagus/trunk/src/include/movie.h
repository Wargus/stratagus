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
/**@name movie.h - The movie header file. */
//
//      (c) Copyright 2002-2004 by Lutz Sammer
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

#ifndef __MOVIE_H__
#define __MOVIE_H__

//@{

/*----------------------------------------------------------------------------
--  Documentation
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "iolib.h"
#include "ogg/ogg.h"
#include "vorbis/codec.h"
#include "theora/theora.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

/**
**  Ogg data structure to handle vorbis/theora streaming.
*/
typedef struct _ogg_data_ {
	CLFile* File;      ///< Ogg file handle
	ogg_sync_state sync;
	ogg_page page;

	ogg_stream_state astream;
	vorbis_info vinfo;
	vorbis_comment vcomment;
	vorbis_block vblock;
	vorbis_dsp_state vdsp;

#ifdef USE_THEORA
	ogg_stream_state vstream;
	theora_info tinfo;
	theora_comment tcomment;
	theora_state tstate;
#endif

	int audio : 1;
#ifdef USE_THEORA
	int video : 1;
#endif
} OggData;

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern int OggInit(CLFile *f, OggData *data);
extern int OggGetNextPage(ogg_page *page, ogg_sync_state *sync, CLFile *f);

extern int VorbisProcessData(OggData *data, char *buffer);
extern int TheoraProcessData(OggData *data);

	/// Play a movie file
extern int PlayMovie(const char* name);

//@}

#endif // !__MOVIE_H__
