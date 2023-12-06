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

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/
uint32_t Mng::MaxFPS = 15;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

struct MngWrapper
{
	static mng_ptr alloc(mng_size_t len)
	{
		char *ptr = new char[len]{};
		return (mng_ptr) ptr;
	}

	static void free(mng_ptr ptr, mng_size_t) { delete[] static_cast<char *>(ptr); }

	static mng_bool openstream(mng_handle handle)
	{
		Mng *mng = (Mng *) mng_get_userdata(handle);

		mng->fd = fopen(mng->name.c_str(), "rb");
		if (!mng->fd) {
			return MNG_FALSE;
		}
		return MNG_TRUE;
	}

	static mng_bool closestream(mng_handle handle)
	{
		Mng *mng = (Mng *) mng_get_userdata(handle);

		if (mng->fd) {
			fclose(mng->fd);
		}
		return MNG_TRUE;
	}

	static mng_bool readdata(mng_handle handle, mng_ptr buf, mng_uint32 buflen, mng_uint32p read)
	{
		Mng *mng = (Mng *) mng_get_userdata(handle);

		*read = fread(buf, 1, buflen, mng->fd);
		return MNG_TRUE;
	}

	static mng_bool processheader(mng_handle handle, mng_uint32 width, mng_uint32 height)
	{
		mng_imgtype type = mng_get_sigtype(handle);
		if (type != mng_it_mng) {
			return MNG_TRUE;
		}

		Mng *mng = (Mng *) mng_get_userdata(handle);

		// Allocate the SDL surface to hold the image
# if SDL_BYTEORDER == SDL_LIL_ENDIAN
		const Uint32 Rmask = 0x000000FF;
		const Uint32 Gmask = 0x0000FF00;
		const Uint32 Bmask = 0x00FF0000;
# else
		const Uint32 Rmask = 0x00FF0000;
		const Uint32 Gmask = 0x0000FF00;
		const Uint32 Bmask = 0x000000FF;
# endif

		mng->buffer.resize(width * height * 3);
		memset(mng->buffer.data(), width * height * 3, sizeof(unsigned char));

		mng->mSurface.reset(
			SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 8 * 3, Rmask, Gmask, Bmask, 0));
		if (mng->mSurface == nullptr) {
			ErrorPrint("Out of memory");
			exit(1);
		}
		SDL_SetColorKey(mng->mSurface.get(), 1, 0);

		return MNG_TRUE;
	}

	static mng_ptr getcanvasline(mng_handle handle, mng_uint32 linenr)
	{
		Mng *mng = (Mng *) mng_get_userdata(handle);
		return &mng->buffer[linenr * mng->mSurface->w * 3];
	}

	static mng_bool refresh(mng_handle handle, mng_uint32, mng_uint32, mng_uint32, mng_uint32)
	{
		Mng *mng = (Mng *) mng_get_userdata(handle);
		SDL_LockSurface(mng->mSurface.get());
		for (int i = 0; i < mng->mSurface->h; ++i) {
			memcpy((char *) mng->mSurface->pixels + i * mng->mSurface->pitch,
			       &mng->buffer[i * mng->mSurface->w * 3],
			       mng->mSurface->w * 3);
		}
		SDL_UnlockSurface(mng->mSurface.get());

		return MNG_TRUE;
	}

	static mng_uint32 gettickcount(mng_handle) { return GetTicks(); }

	static mng_bool settimer(mng_handle handle, mng_uint32 msecs)
	{
		Mng *mng = (Mng *) mng_get_userdata(handle);
		unsigned long ticks = GetTicks();
		uint32_t offset =
			std::max(static_cast<uint32_t>(msecs), static_cast<uint32_t>(1000 / mng->MaxFPS));
		mng->ticks = std::max(ticks + offset, mng->ticks + offset);

		return MNG_TRUE;
	}

	static mng_bool processmend(mng_handle handle, mng_uint32 iterationsdone, mng_uint32)
	{
		Mng *mng = (Mng *) mng_get_userdata(handle);
		mng->iteration = iterationsdone;

		return MNG_TRUE;
	}

	static mng_bool errorproc(mng_handle handle,
	                          mng_int32,
	                          mng_int8,
	                          mng_chunkid,
	                          mng_uint32,
	                          mng_int32,
	                          mng_int32,
	                          mng_pchar errortext)
	{
		Mng *mng = (Mng *) mng_get_userdata(handle);
		mng->iteration = 0x7fffffff;
		if (errortext) {
			DebugPrint("MNG error: %s\n", errortext);
		}
		return MNG_TRUE;
	}
};

Mng::~Mng()
{
	if (handle) {
		mng_cleanup(&handle);
	}
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

	SDL_Rect rect = {(short int)x, (short int)y, (short unsigned int)(mSurface->w), (short unsigned int)(mSurface->h)};
	SDL_BlitSurface(mSurface.get(), nullptr, TheScreen, &rect);
}

static std::map<std::string, Mng *> MngCache;

Mng *Mng::New(const std::string &name)
{
	const std::string file = LibraryFileName(name);
	Mng *&mng = MngCache[file];
	if (mng == nullptr) {
		mng = new Mng();
		mng->name = LibraryFileName(name);
	}
	mng->refcnt++;
	return mng;
}

void Mng::Free(Mng *mng)
{
	// XXX: Weird free bug that I don't understand, just skip it if already nullptr
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
		return mSurface && iteration != 0x7fffffff;
	}
	handle = mng_initialize(this, &MngWrapper::alloc, &MngWrapper::free, MNG_NULL);
	if (handle == MNG_NULL) {
		return false;
	}
	mng_setcb_openstream(handle, &MngWrapper::openstream);
	mng_setcb_closestream(handle, &MngWrapper::closestream);
	mng_setcb_readdata(handle, &MngWrapper::readdata);
	mng_setcb_processheader(handle, &MngWrapper::processheader);
	mng_setcb_processmend(handle, &MngWrapper::processmend);
	mng_setcb_getcanvasline(handle, &MngWrapper::getcanvasline);
	mng_setcb_refresh(handle, &MngWrapper::refresh);
	mng_setcb_gettickcount(handle, &MngWrapper::gettickcount);
	mng_setcb_settimer(handle, &MngWrapper::settimer);
	mng_setcb_errorproc(handle, &MngWrapper::errorproc);

	mng_read(handle);
	if (mSurface && iteration != 0x7fffffff) {
		mng_display(handle);
	}

	if (!mSurface || iteration == 0x7fffffff) {
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
	return mSurface.get();
}

#endif // USE_MNG

//@}
