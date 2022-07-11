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
/**@name mng.cpp - The mng graphic file loader. */
//
//      (c) Copyright 2004-2005 by Jimmy Salmon
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

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#ifdef USE_MNG

#include "video.h"
#include "iolib.h"
#include "iocompat.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/
uint32_t Mng::MaxFPS = 15;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

static mng_ptr MNG_DECL my_alloc(mng_size_t len)
{
	char *ptr = new char[len];
	memset(ptr, 0, len);
	return (mng_ptr)ptr;
}

static void MNG_DECL my_free(mng_ptr ptr, mng_size_t)
{
	delete[] static_cast<char *>(ptr);
}

static mng_bool MNG_DECL my_openstream(mng_handle handle)
{
	Mng *mng;

	mng = (Mng *)mng_get_userdata(handle);
	mng->fd = fopen(mng->name.c_str(), "rb");
	if (!mng->fd) {
		return MNG_FALSE;
	}
	return MNG_TRUE;
}

static mng_bool MNG_DECL my_closestream(mng_handle handle)
{
	Mng *mng;

	mng = (Mng *)mng_get_userdata(handle);
	if (mng->fd) {
		fclose(mng->fd);
	}
	return MNG_TRUE;
}

static mng_bool MNG_DECL my_readdata(mng_handle handle, mng_ptr buf, mng_uint32 buflen,
									 mng_uint32p read)
{
	Mng *mng;

	mng = (Mng *)mng_get_userdata(handle);
	*read = fread(buf, 1, buflen, mng->fd);
	return MNG_TRUE;
}

static mng_bool MNG_DECL my_processheader(mng_handle handle, mng_uint32 width,
										  mng_uint32 height)
{
	mng_imgtype type = mng_get_sigtype(handle);
	if (type != mng_it_mng) {
		return TRUE;
	}

	Mng *mng = (Mng *)mng_get_userdata(handle);

	// Allocate the SDL surface to hold the image
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	const Uint32 Rmask = 0x000000FF;
	const Uint32 Gmask = 0x0000FF00;
	const Uint32 Bmask = 0x00FF0000;
#else
	const Uint32 Rmask = 0xFF000000 >> 8;
	const Uint32 Gmask = 0x00FF0000 >> 8;
	const Uint32 Bmask = 0x0000FF00 >> 8;
#endif

	mng->buffer = new unsigned char[width * height * 3];
	memset(mng->buffer, width * height * 3, sizeof(unsigned char));

	mng->surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height,
										8 * 3, Rmask, Gmask, Bmask, 0);
	SDL_SetColorKey(mng->surface, 1, 0);
	if (mng->surface == NULL) {
		fprintf(stderr, "Out of memory");
		exit(1);
	}

	return MNG_TRUE;
}

static mng_ptr MNG_DECL my_getcanvasline(mng_handle handle, mng_uint32 linenr)
{
	Mng *mng;

	mng = (Mng *)mng_get_userdata(handle);
	return mng->buffer + linenr * mng->surface->w * 3;
}

static mng_bool MNG_DECL my_refresh(mng_handle handle, mng_uint32, mng_uint32,
									mng_uint32, mng_uint32)
{
	Mng *mng = (Mng *)mng_get_userdata(handle);
	SDL_LockSurface(mng->surface);
	for (int i = 0; i < mng->surface->h; ++i) {
		memcpy((char *)mng->surface->pixels + i * mng->surface->pitch,
			   mng->buffer + i * mng->surface->w * 3, mng->surface->w * 3);
	}
	SDL_UnlockSurface(mng->surface);

	return MNG_TRUE;
}

static mng_uint32 MNG_DECL my_gettickcount(mng_handle)
{
	return GetTicks();
}

static mng_bool MNG_DECL my_settimer(mng_handle handle, mng_uint32 msecs)
{
	Mng *mng = (Mng *)mng_get_userdata(handle);
	unsigned long ticks = GetTicks();
	uint32_t offset = std::max(static_cast<uint32_t>(msecs), static_cast<uint32_t>(1000 / mng->MaxFPS));
	mng->ticks = std::max(ticks + offset, mng->ticks + offset);

	return MNG_TRUE;
}

static mng_bool MNG_DECL my_processmend(mng_handle handle, mng_uint32 iterationsdone,
										mng_uint32)
{
	Mng *mng = (Mng *)mng_get_userdata(handle);
	mng->iteration = iterationsdone;

	return MNG_TRUE;
}

static mng_bool MNG_DECL my_errorproc(mng_handle handle, mng_int32,
									  mng_int8, mng_chunkid, mng_uint32, mng_int32, mng_int32, mng_pchar errortext)
{
	Mng *mng = (Mng *)mng_get_userdata(handle);
	mng->iteration = 0x7fffffff;
	if (errortext) {
		DebugPrint("MNG error: %s\n" _C_ errortext);
	}
	return TRUE;
}


Mng::Mng() :
	name(""), fd(NULL), handle(NULL), surface(NULL), buffer(NULL),
	ticks(0), iteration(0), is_dirty(false)
{
}


Mng::~Mng()
{
	if (handle) {
		mng_cleanup(&handle);
	}
	if (surface) {
		SDL_FreeSurface(surface);
	}
	delete[] buffer;
}

/**
**  Display a MNG
**
**  @param x  X coordinate
**  @param y  Y coordinate
*/
void Mng::Draw(int x, int y)
{
	if (ticks <= GetTicks()) {
		mng_display_resume(handle);
	}

	SDL_Rect rect = {(short int)x, (short int)y, (short unsigned int)(surface->w), (short unsigned int)(surface->h)};
	SDL_BlitSurface(surface, NULL, TheScreen, &rect);
}

static std::map<std::string, Mng *> MngCache;

Mng *Mng::New(const std::string &name)
{
	const std::string file = LibraryFileName(name.c_str());
	Mng *mng = MngCache[file];
	if (mng == NULL) {
		mng = new Mng();
		mng->name = LibraryFileName(name.c_str());
		Assert(mng);
	}
	mng->refcnt++;
	return mng;
}

void Mng::Free(Mng *mng)
{
	// XXX: Weird free bug that I don't understand, just skip it if already NULL
	if ((intptr_t)mng < 40) {
		return;
	}
	mng->refcnt--;
	if (mng->refcnt == 0) {
		MngCache.erase(mng->name);
		delete mng;
	}
}

/**
**  Load a MNG
**
**  @param name  Name of the MNG file
*/
bool Mng::Load()
{
	if (handle) {
		return handle != MNG_NULL && surface && iteration != 0x7fffffff;
	}
	handle = mng_initialize(this, my_alloc, my_free, MNG_NULL);
	if (handle == MNG_NULL) {
		return false;
	}
	mng_setcb_openstream(handle, my_openstream);
	mng_setcb_closestream(handle, my_closestream);
	mng_setcb_readdata(handle, my_readdata);
	mng_setcb_processheader(handle, my_processheader);
	mng_setcb_processmend(handle, my_processmend);
	mng_setcb_getcanvasline(handle, my_getcanvasline);
	mng_setcb_refresh(handle, my_refresh);
	mng_setcb_gettickcount(handle, my_gettickcount);
	mng_setcb_settimer(handle, my_settimer);
	mng_setcb_errorproc(handle, my_errorproc);

	mng_read(handle);
	if (surface && iteration != 0x7fffffff) {
		mng_display(handle);
	}

	if (!surface || iteration == 0x7fffffff) {
		return false;
	}
	return true;
}

/**
**  Reset a MNG
*/
void Mng::Reset()
{
	if (!handle) {
		return;
	}
	mng_display_reset(handle);
	iteration = 0;
	mng_display(handle);
}

void* Mng::_getData() const
{
	if (ticks <= GetTicks()) {
		is_dirty = true;
		mng_display_resume(handle);
	} else {
		is_dirty = false;
	}
	return surface;
}

#endif // USE_MNG

//@}
