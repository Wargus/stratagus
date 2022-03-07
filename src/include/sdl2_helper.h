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
/**@name sdl2_helper.h - SDL2 helpers headerfile. */
//
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

#pragma once

#include <memory>
#include "SDL.h"

//@{

namespace sdl2 {
    struct SDL_Deleter
    {
        void operator()(SDL_Surface*  ptr) { if (ptr) SDL_FreeSurface(ptr);     } 
        void operator()(SDL_Texture*  ptr) { if (ptr) SDL_DestroyTexture(ptr);  }
        void operator()(SDL_Renderer* ptr) { if (ptr) SDL_DestroyRenderer(ptr); }
        void operator()(SDL_Window*   ptr) { if (ptr) SDL_DestroyWindow(ptr);   }
    };

    template<typename T>
    struct _shared_ptr : public std::shared_ptr<T>
    {
        explicit _shared_ptr(T* t = nullptr) : std::shared_ptr<T>(t, SDL_Deleter()) {}
        
        void reset(T* t = nullptr) { std::shared_ptr<T>::reset(t, SDL_Deleter()); }
    };


    using SurfacePtr    = std::unique_ptr<SDL_Surface,  SDL_Deleter>;
    using TexturePtr    = std::unique_ptr<SDL_Texture,  SDL_Deleter>;
    using RendererPtr   = std::unique_ptr<SDL_Renderer, SDL_Deleter>;
    using WindowPtr     = std::unique_ptr<SDL_Window,   SDL_Deleter>;
    using SurfaceShPtr  = _shared_ptr<SDL_Surface>;
    using TextureShPtr  = _shared_ptr<SDL_Texture>;
    using RendererShPtr = _shared_ptr<SDL_Renderer>;
    using WindowShPtr   = _shared_ptr<SDL_Window>;
}

//@}