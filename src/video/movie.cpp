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
/**@name movie.c - Movie playback functions. */
//
// (c) Copyright 2005 by Nehal Mistry
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

#ifdef USE_THEORA

/*----------------------------------------------------------------------------
-- Includes
----------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "stratagus.h"
#include "video.h"
#include "sound.h"
#include "sound_server.h"
#include "movie.h"
#include "network.h"
#include "iocompat.h"
#include "iolib.h"

#include "SDL.h"

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern SDL_Surface *TheScreen;
static int MovieStop;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Callbacks for movie input.
*/

static void MovieCallbackButtonPressed(unsigned dummy)
{
	MovieStop = 1;
}

static void MovieCallbackButtonReleased(unsigned dummy)
{
}

static void MovieCallbackKeyPressed(unsigned dummya, unsigned dummyb)
{
	MovieStop = 1;
}


static void MovieCallbackKeyReleased(unsigned dummya, unsigned dummyb)
{
}

static void MovieCallbackKeyRepeated(unsigned dummya, unsigned dummyb)
{
}

static void MovieCallbackMouseMove(int dummya, int dummyb)
{
}

static void MovieCallbackMouseExit(void)
{
}

int OutputTheora(OggData *data, SDL_Overlay *yuv_overlay, SDL_Rect *rect)
{
	int i;
	yuv_buffer yuv;
	int crop_offset;

	theora_decode_YUVout(&data->tstate, &yuv);

	if (SDL_MUSTLOCK(TheScreen)) {
		if (SDL_LockSurface(TheScreen) < 0) {
			return - 1;
		}
	}

	if (SDL_LockYUVOverlay(yuv_overlay) < 0) {
		return -1;
	}

	crop_offset = data->tinfo.offset_x + yuv.y_stride * data->tinfo.offset_y;
	for(i=0; i < yuv_overlay->h; ++i) {
		memcpy(yuv_overlay->pixels[0] + yuv_overlay->pitches[0] * i,
		  yuv.y + crop_offset + yuv.y_stride * i, yuv_overlay->w);
	}

	crop_offset = (data->tinfo.offset_x / 2) + (yuv.uv_stride) *
	  (data->tinfo.offset_y / 2);
	for (i=0; i < yuv_overlay->h / 2; ++i) {
		memcpy(yuv_overlay->pixels[1] + yuv_overlay->pitches[1] * i,
		  yuv.v + yuv.uv_stride * i, yuv_overlay->w / 2);
		memcpy(yuv_overlay->pixels[2] + yuv_overlay->pitches[2] * i,
		  yuv.u + crop_offset + yuv.uv_stride * i, yuv_overlay->w / 2);
	}

	if (SDL_MUSTLOCK(TheScreen)) {
		SDL_UnlockSurface(TheScreen);
	}
	SDL_UnlockYUVOverlay(yuv_overlay);

	SDL_DisplayYUVOverlay(yuv_overlay, rect);

	return 0;
}

int TheoraProcessData(OggData *data)
{
	ogg_packet packet;

	while (1) {
		if (ogg_stream_packetout(&data->vstream, &packet) != 1) {
			if (OggGetNextPage(&data->page, &data->sync, data->File)) {
				// EOF
				return -1;
			}

			ogg_stream_pagein(&data->vstream, &data->page);
		} else {
			theora_decode_packetin(&data->tstate, &packet);
			data->tstate.internal_encode = NULL;	// needed, maybe a bug in libtheora?
			return 0;
		}
	}
}


/**
**  Play a video file.
**
**  @param name   Filename of movie file.
**
**  @return       Non-zero if file isn't a supported movie.
**
*/
int PlayMovie(const char* name)
{
	OggData *data;
	CLFile *f;
	SDL_Rect rect;
	SDL_Overlay *yuv_overlay;
	Sample *sample;
	EventCallback callbacks;
	unsigned int start_ticks;
	int need_data;
	int diff;
	char buffer[PATH_MAX];

	name = LibraryFileName(name, buffer);

	if (!(f = CLopen(name, CL_OPEN_READ))) {
		fprintf(stderr, "Can't open file `%s'\n", name);
		return -1;
	}

	data = malloc(sizeof(OggData));

	if (OggInit(f, data) || !data->video) {
		free(data);
		CLclose(f);
		return -1;
	}

	data->File = f;

	rect.x = 0;
	rect.y = 0;

	rect.w = data->tinfo.frame_width;
	rect.h = data->tinfo.frame_height;
	rect.w = VideoWidth;
	rect.h = VideoHeight;

	yuv_overlay = SDL_CreateYUVOverlay(data->tinfo.frame_width,
	  data->tinfo.frame_height, SDL_YV12_OVERLAY, TheScreen);

	SDL_DisplayYUVOverlay(yuv_overlay, &rect);

	if (yuv_overlay == NULL) {
		exit(-1);
		return -1;
	}

	StopMusic();
	if ((sample = LoadVorbis(name, PlayAudioStream))) {
		if ((sample->Channels != 1 && sample->Channels != 2) ||
				sample->SampleSize != 16) {
			DebugPrint("Not supported music format\n");
			SoundFree(sample);
			return 0;
		}
		MusicSample = sample;
		PlayingMusic = 1;
	}

	callbacks.ButtonPressed = MovieCallbackButtonPressed;
	callbacks.ButtonReleased = MovieCallbackButtonReleased;
	callbacks.MouseMoved = MovieCallbackMouseMove;
	callbacks.MouseExit = MovieCallbackMouseExit;
	callbacks.KeyPressed = MovieCallbackKeyPressed;
	callbacks.KeyReleased = MovieCallbackKeyReleased;
	callbacks.KeyRepeated = MovieCallbackKeyRepeated;
	callbacks.NetworkEvent = NetworkEvent;

	MovieStop = 0;
	start_ticks = SDL_GetTicks();
	need_data = 1;
	while (!MovieStop) {
		if (need_data) {
			if (TheoraProcessData(data)) {
				break;
			}
			need_data = 0;
		}

		diff = SDL_GetTicks() - start_ticks - theora_granule_time(&data->tstate,
		  data->tstate.granulepos) * 1000;

		if (diff > 100) {
			// too far behind, skip some frames
			need_data = 1;
			continue;
		}
		if (diff > 0) {
			OutputTheora(data, yuv_overlay, &rect);
			need_data = 1;
		}
		
		WaitEventsOneFrame(&callbacks);
	}

	StopMusic();
	SDL_FreeYUVOverlay(yuv_overlay);

	if (data->audio) {
		ogg_stream_clear(&data->astream);
		vorbis_block_clear(&data->vblock);
		vorbis_dsp_clear(&data->vdsp);
		vorbis_comment_clear(&data->vcomment);
		vorbis_info_clear(&data->vinfo);
	}

	if (data->video) {
		ogg_stream_clear(&data->vstream);
		theora_comment_clear(&data->tcomment);
		theora_info_clear(&data->tinfo);
	}

	return 0;
}

#else

int PlayMovie(const char* name)
{
	return -1;
}

#endif

//@}
