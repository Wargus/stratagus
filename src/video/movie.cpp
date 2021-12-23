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
/**@name movie.cpp - Movie playback functions. */
//
//      (c) Copyright 2005-2011 by Nehal Mistry, Jimmy Salmon and Pali Rohár
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

//@{

#if defined(USE_THEORA) && defined(USE_VORBIS)

/*----------------------------------------------------------------------------
-- Includes
----------------------------------------------------------------------------*/

#include <cstdlib>

#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#include <theora/theora.h>

#include "stratagus.h"

#include "movie.h"

#include "iocompat.h"
#include "iolib.h"
#include "network.h"
#include "sound.h"
#include "sound_server.h"
#include "video.h"

#include "SDL.h"
#include "SDL_endian.h"

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern SDL_Surface *TheScreen;
static bool MovieStop;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

int OggGetNextPage(ogg_page *page, ogg_sync_state *sync, CFile *f)
{
	char *buf;
	int bytes;

	while (ogg_sync_pageout(sync, page) != 1) {
		// need more bytes
		buf = ogg_sync_buffer(sync, 4096);
		bytes = f->read(buf, 4096);
		if (!bytes || ogg_sync_wrote(sync, bytes)) {
			return -1;
		}
	}

	return 0;
}

int OggInit(CFile *f, OggData *data)
{
	ogg_packet packet;
	int num_vorbis;
	int num_theora;
	int ret;

	unsigned magic;
	f->read(&magic, sizeof(magic));
	if (SDL_SwapLE32(magic) != 0x5367674F) { // "OggS" in ASCII
		return -1;
	}
	f->seek(0, SEEK_SET);

	ogg_sync_init(&data->sync);

	vorbis_info_init(&data->vinfo);
	vorbis_comment_init(&data->vcomment);

	theora_info_init(&data->tinfo);
	theora_comment_init(&data->tcomment);

	num_theora = 0;
	num_vorbis = 0;
	while (1) {
		ogg_stream_state test;

		if (OggGetNextPage(&data->page, &data->sync, f)) {
			return -1;
		}

		if (!ogg_page_bos(&data->page)) {
			if (num_vorbis) {
				ogg_stream_pagein(&data->astream, &data->page);
			}
			if (num_theora) {
				ogg_stream_pagein(&data->vstream, &data->page);
			}
			break;
		}

		ogg_stream_init(&test, ogg_page_serialno(&data->page));
		ogg_stream_pagein(&test, &data->page);

		// initial codec headers
		while (ogg_stream_packetout(&test, &packet) == 1) {
			if (theora_decode_header(&data->tinfo, &data->tcomment, &packet) >= 0) {
				memcpy(&data->vstream, &test, sizeof(test));
				++num_theora;
			} else
				if (!vorbis_synthesis_headerin(&data->vinfo, &data->vcomment, &packet)) {
					memcpy(&data->astream, &test, sizeof(test));
					++num_vorbis;
				} else {
					ogg_stream_clear(&test);
				}
		}
	}

	data->audio = num_vorbis;
	data->video = num_theora;

	// remainint codec headers
	while ((num_vorbis && num_vorbis < 3)
		   || (num_theora && num_theora < 3)) {
		// are we in the theora page ?
		while (num_theora && num_theora < 3 &&
			   (ret = ogg_stream_packetout(&data->vstream, &packet))) {
			if (ret < 0) {
				return -1;
			}
			if (theora_decode_header(&data->tinfo, &data->tcomment, &packet)) {
				return -1;
			}
			++num_theora;
		}

		// are we in the vorbis page ?
		while (num_vorbis && num_vorbis < 3 &&
			   (ret = ogg_stream_packetout(&data->astream, &packet))) {
			if (ret < 0) {
				return -1;
			}
			if (vorbis_synthesis_headerin(&data->vinfo, &data->vcomment, &packet)) {
				return -1;

			}
			++num_vorbis;
		}

		if (OggGetNextPage(&data->page, &data->sync, f)) {
			break;
		}

		if (num_vorbis) {
			ogg_stream_pagein(&data->astream, &data->page);
		}
		if (num_theora) {
			ogg_stream_pagein(&data->vstream, &data->page);
		}
	}

	if (num_vorbis) {
		vorbis_synthesis_init(&data->vdsp, &data->vinfo);
		vorbis_block_init(&data->vdsp, &data->vblock);
	} else {
		vorbis_info_clear(&data->vinfo);
		vorbis_comment_clear(&data->vcomment);
	}

	if (num_theora) {
		theora_decode_init(&data->tstate, &data->tinfo);
		data->tstate.internal_encode = NULL;  // needed for a bug in libtheora (fixed in next release)
	} else {
		theora_info_clear(&data->tinfo);
		theora_comment_clear(&data->tcomment);
	}

	return !(num_vorbis || num_theora);
}

void OggFree(OggData *data)
{
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
		theora_clear(&data->tstate);
	}
	ogg_sync_clear(&data->sync);
}

/**
**  Callbacks for movie input.
*/

static void MovieCallbackButtonPressed(unsigned)
{
	MovieStop = true;
}

static void MovieCallbackButtonReleased(unsigned)
{
}

static void MovieCallbackKeyPressed(unsigned, unsigned)
{
	MovieStop = true;
}


static void MovieCallbackKeyReleased(unsigned, unsigned)
{
}

static void MovieCallbackKeyRepeated(unsigned, unsigned)
{
}

static void MovieCallbackMouseMove(const PixelPos &)
{
}

static void MovieCallbackMouseExit()
{
}

/**
**  Draw Ogg data to the overlay
*/
static int OutputTheora(OggData *data, SDL_Texture *yuv_overlay, SDL_Rect *rect)
{
	yuv_buffer yuv;

	theora_decode_YUVout(&data->tstate, &yuv);

	SDL_UpdateYUVTexture(yuv_overlay, NULL, yuv.y, yuv.y_stride, yuv.u, yuv.uv_stride, yuv.v, yuv.uv_stride);
	SDL_RenderClear(TheRenderer);
	SDL_RenderCopy(TheRenderer, yuv_overlay, NULL, rect);
	SDL_RenderPresent(TheRenderer);

	return 0;
}

/**
**  Process Ogg data
*/
static int TheoraProcessData(OggData *data)
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
*/
int PlayMovie(const std::string &name)
{
	int videoWidth, videoHeight;
	videoWidth  = Video.Width;
	videoHeight = Video.Height;

	const std::string filename = LibraryFileName(name.c_str());

	CFile f;
	if (f.open(filename.c_str(), CL_OPEN_READ) == -1) {
		fprintf(stderr, "Can't open file '%s'\n", name.c_str());
		return 0;
	}

	OggData data;
	memset(&data, 0, sizeof(data));
	if (OggInit(&f, &data) || !data.video) {
		OggFree(&data);
		f.close();
		return -1;
	}

	data.File = &f;
	SDL_Rect rect;

	if (data.tinfo.frame_width * 300 / 4 > data.tinfo.frame_height * 100) {
		rect.w = videoWidth;
		rect.h = videoWidth * data.tinfo.frame_height / data.tinfo.frame_width;
		rect.x = 0;
		rect.y = (videoHeight - rect.h) / 2;
	} else {
		rect.w = videoHeight * data.tinfo.frame_width / data.tinfo.frame_height;
		rect.h = videoHeight;
		rect.x = (videoWidth - rect.w) / 2;
		rect.y = 0;
	}

	SDL_RenderClear(TheRenderer);
	Video.ClearScreen();
	SDL_Texture *yuv_overlay = SDL_CreateTexture(TheRenderer,
	                                             SDL_PIXELFORMAT_YV12,
	                                             SDL_TEXTUREACCESS_STREAMING,
	                                             data.tinfo.frame_width,
	                                             data.tinfo.frame_height);

	if (yuv_overlay == NULL) {
		fprintf(stderr, "SDL_CreateYUVOverlay: %s\n", SDL_GetError());
		fprintf(stderr, "SDL_CreateYUVOverlay: %dx%d\n", data.tinfo.frame_width, data.tinfo.frame_height);
		OggFree(&data);
		f.close();
		return 0;
	}

	StopMusic();
	Mix_Music *sample = LoadMusic(filename);
	if (sample) {
		PlayMusic(sample);
	}

	EventCallback callbacks;

	callbacks.ButtonPressed = MovieCallbackButtonPressed;
	callbacks.ButtonReleased = MovieCallbackButtonReleased;
	callbacks.MouseMoved = MovieCallbackMouseMove;
	callbacks.MouseExit = MovieCallbackMouseExit;
	callbacks.KeyPressed = MovieCallbackKeyPressed;
	callbacks.KeyReleased = MovieCallbackKeyReleased;
	callbacks.KeyRepeated = MovieCallbackKeyRepeated;
	callbacks.NetworkEvent = NetworkEvent;

	const EventCallback *old_callbacks = GetCallbacks();
	SetCallbacks(&callbacks);

	Invalidate();
	SDL_RenderClear(TheRenderer);
	SDL_SetRenderDrawColor(TheRenderer, 0, 0, 0, 255);
	SDL_RenderDrawRect(TheRenderer, NULL);
	// SDL_RenderPresent(TheRenderer);

	MovieStop = false;
	const unsigned int start_ticks = SDL_GetTicks();
	bool need_data = true;
	while (!MovieStop) {
		if (need_data) {
			if (TheoraProcessData(&data)) {
				break;
			}
			need_data = false;
		}

		const int diff = SDL_GetTicks() - start_ticks
						 - static_cast<int>(theora_granule_time(&data.tstate, data.tstate.granulepos) * 1000);

		if (diff > 100) {
			// too far behind, skip some frames
			need_data = true;
			continue;
		}
		if (diff > 0) {
			OutputTheora(&data, yuv_overlay, &rect);
			need_data = true;
		}

		WaitEventsOneFrame();
	}

	StopMusic();
	SDL_DestroyTexture(yuv_overlay);

	OggFree(&data);
	f.close();

	SetCallbacks(old_callbacks);

	return 0;
}

Movie::~Movie()
{
	if (rect != NULL) {
		free(rect);
	}
	if (surface != NULL) {
		SDL_FreeSurface(surface);
	}
	if (yuv_overlay != NULL) {
		SDL_DestroyTexture(yuv_overlay);
	}
	if (data != NULL) {
		OggFree(data);
	}
	if (f != NULL) {
		delete f;
	}
}

static void RenderToSurface(SDL_Surface *surface, SDL_Texture *yuv_overlay, SDL_Rect *rect, OggData *data) {
	yuv_buffer *yuv = (yuv_buffer*)calloc(sizeof(yuv_buffer), 1);
	theora_decode_YUVout(&data->tstate, yuv);
	SDL_UpdateYUVTexture(yuv_overlay, NULL, yuv->y, yuv->y_stride, yuv->u, yuv->uv_stride, yuv->v, yuv->uv_stride);
	SDL_RenderClear(TheRenderer);

	// since SDL will render us at logical size, and SDL_RenderReadPixels will read the at
	// output size, we need to adapt the rectangles to read and write from dynamically
	int rw, rh, ww, wh;
	SDL_GetWindowSize(TheWindow, &ww, &wh);
	SDL_RenderGetLogicalSize(TheRenderer, &rw, &rh);
	SDL_Rect render_rect;
	render_rect.x = 0;
	render_rect.y = 0;
	double scaleX = (double)ww / rw;
	double scaleY = (double)wh / rh;
	double scale = std::min(scaleX, scaleY);
	render_rect.w = rect->w / scale;
	render_rect.h = rect->h / scale;

	SDL_Rect read_rect;
	read_rect.w = rect->w;
	read_rect.h = rect->h;
	read_rect.x = (ww - (rw * scale)) / 2;
	read_rect.y = (wh - (rh * scale)) / 2;
	SDL_RenderCopy(TheRenderer, yuv_overlay, NULL, &render_rect);
	if (SDL_RenderReadPixels(TheRenderer, &read_rect, surface->format->format, surface->pixels, surface->pitch)) {
		fprintf(stderr, "Reading from renderer not supported\n");
		SDL_FillRect(surface, NULL, 0); // completely transparent
	}
	free(yuv);
}

bool Movie::Load(const std::string &name, int w, int h)
{
	Width = w;
	Height = h;
	const std::string filename = LibraryFileName(name.c_str());

	f = new CFile();
	if (f->open(filename.c_str(), CL_OPEN_READ) == -1) {
		fprintf(stderr, "Can't open file '%s'\n", name.c_str());
		return false;
	}

	rect = (SDL_Rect*)calloc(sizeof(SDL_Rect), 1);
	rect->x = 0;
	rect->y = 0;
	rect->w = w;
	rect->h = h;

	surface = SDL_CreateRGBSurface(0, w, h, TheScreen->format->BitsPerPixel,
								   0x00ff0000,
								   0x0000ff00,
								   0x000000ff,
								   0xff000000);

	if (surface == NULL) {
		fprintf(stderr, "SDL_CreateRGBSurface: %s\n", SDL_GetError());
		f->close();
		return false;
	}

	return true;
}

void *Movie::_getData() const
{
	if (data == NULL) {
		data = (OggData*)calloc(sizeof(OggData), 1);
		if (OggInit(f, data) || !data->video) {
			OggFree(data);
			f->close();
			fprintf(stderr, "Could not init OggData or not a video\n");
			return surface;
		}

		data->File = f;
		yuv_overlay = SDL_CreateTexture(TheRenderer,
										SDL_PIXELFORMAT_YV12,
										SDL_TEXTUREACCESS_STREAMING,
										data->tinfo.frame_width,
										data->tinfo.frame_height);

		if (yuv_overlay == NULL) {
			fprintf(stderr, "SDL_CreateYUVOverlay: %s\n", SDL_GetError());
			fprintf(stderr, "SDL_CreateYUVOverlay: %dx%d\n", data->tinfo.frame_width, data->tinfo.frame_height);
			OggFree(data);
			f->close();
			return surface;
		}

		start_time = SDL_GetTicks();
		need_data = true;
		TheoraProcessData(data);
		RenderToSurface(surface, yuv_overlay, rect, data);
	}
	if (need_data) {
		if (TheoraProcessData(data)) {
			is_dirty = false;
			return surface;
		}
		need_data = false;
	}

	const int diff = SDL_GetTicks() - start_time
		- static_cast<int>(theora_granule_time(&data->tstate, data->tstate.granulepos) * 1000);

	if (diff > 0) {
		RenderToSurface(surface, yuv_overlay, rect, data);
		need_data = true;
	}

	return surface;
}

#else

#include <string>
#include <string.h>
#include <stdio.h>

/**
**  Play a video file.
**
**  @param name   Filename of movie file.
**
**  @return       Non-zero if file isn't a supported movie.
*/
int PlayMovie(const std::string &name)
{
	if (strstr(name.c_str(), ".ogg") || strstr(name.c_str(), ".ogv")) {
		fprintf(stderr, "PlayMovie() '%s' is not supported, please recompile stratagus with theora support\n", name.c_str());
		return 0;
	}
	return -1;
}

#endif

//@}
