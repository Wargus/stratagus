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
/**@name animation.cpp - The animated image loader (WebP/GIF). */
//
//      (c) Copyright 2004-2005 by Jimmy Salmon (MNG version)
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
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "iolib.h"
#include "video.h"

#include <SDL_image.h>
#include <map>

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/
uint32_t Animation::MaxFPS = 15;

static std::map<std::string, std::weak_ptr<Animation>> AnimationCache;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

Animation::~Animation()
{
	if (anim != nullptr) {
		IMG_FreeAnimation(anim);
		anim = nullptr;
	}
	// the frame surface is owned by the gcn::SDLImage base class
}

/**
**  Show the frame which is due, if the current one has been displayed long enough
**
**  Frames are composited onto a persistent surface, so that animations made of
**  partial frames (GIFs usually are) add up to the whole picture. Jumping
**  ahead to catch up after the engine stalled would leave that picture
**  unfinished, so one call shows at most one frame.
*/
void Animation::advance() const
{
	if (anim == nullptr || ticks > GetTicks()) {
		return;
	}
	if (frame >= anim->count) {		// wrapped around: one loop finished
		frame = 0;
		++iteration;
	}
	if (frame == 0) {				// start of a loop, throw away the previous run
		SDL_FillRect(mSurface, nullptr, 0);
	}
	SDL_BlitSurface(anim->frames[frame], nullptr, mSurface, nullptr);

	int delay = anim->delays[frame];
	if (delay < 2) {				// same default as SDL2_image
		delay = 100;
	}
	// don't let animations run faster than the engine can draw them
	delay = std::max(delay, int(1000 / std::max<uint32_t>(MaxFPS, 1)));

	ticks = GetTicks() + delay;
	++frame;
}

/**
**  Display an animation
**
**  @param x  X coordinate in screen pixels
**  @param y  Y coordinate in screen pixels
*/
void Animation::Draw(int x, int y)
{
	if (mSurface == nullptr) {
		return;
	}
	advance();

	SDL_Rect rect = {static_cast<short int>(x), static_cast<short int>(y),
					 static_cast<short unsigned int>(mSurface->w), static_cast<short unsigned int>(mSurface->h)
					};
	SDL_BlitSurface(mSurface, nullptr, TheScreen, &rect);
}

/**
**  Get an animation, loading it on first use
**
**  The file decides which format is used, so animated WebP and animated GIF
**  (and whatever else SDL2_image grows an animation decoder for) can be used
**  interchangeably.
**
**  @param name  Name of the animation file
*/
std::shared_ptr<Animation> Animation::New(const std::string &name)
{
	const std::string file = LibraryFileName(name);
	auto &cache = AnimationCache[file];
	auto animation = cache.lock();

	if (animation == nullptr) {
		animation = std::make_shared<Animation>();
		animation->name = file;
		cache = animation;
	}
	return animation;
}

/**
**  Load an animation
**
**  @return  True when the animation could be decoded
*/
bool Animation::Load()
{
	if (anim != nullptr) {
		return mSurface != nullptr;
	}
	anim = IMG_LoadAnimation(name.c_str());
	if (anim == nullptr) {
		ErrorPrint("Can't load the animation '%s': %s\n", name.c_str(), SDL_GetError());
		return false;
	}
	// The surface keeps the format of the screen, so it can be blitted as it is.
	// Fully transparent pixels stay at zero, which is the colour key.
	mSurface = SDL_CreateRGBSurfaceWithFormat(0, anim->w, anim->h,
											TheScreen->format->BitsPerPixel, TheScreen->format->format);
	if (mSurface == nullptr) {
		ErrorPrint("Out of memory");
		exit(1);
	}
	SDL_SetColorKey(mSurface, SDL_TRUE, 0);
	SDL_FillRect(mSurface, nullptr, 0);
	ticks = 0;						// show the first frame at once
	return true;
}

/**
**  Reset an animation back to its first frame
*/
void Animation::Reset()
{
	frame = 0;
	iteration = 0;
	ticks = 0;
}

SDL_Surface *Animation::getSurface() const
{
	advance();
	return mSurface;
}

//@}
