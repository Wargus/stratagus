#ifndef __SHADERS_H__
#define __SHADERS_H__

#include <SDL.h>

#ifndef __APPLE__
extern bool LoadShaderExtensions();
extern bool RenderWithShader(SDL_Renderer *renderer, SDL_Window* win, SDL_Texture* backBuffer, SDL_Rect *srcrect, SDL_Rect *dstrect);
#else
#include "stratagus.h"

inline bool LoadShaderExtensions() {
    return false;
}

inline bool RenderWithShader(SDL_Renderer*, SDL_Window*, SDL_Texture*, SDL_Rect*, SDL_Rect*) {
    return false;
}
#endif

#endif
