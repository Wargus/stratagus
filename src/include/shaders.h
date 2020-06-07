#ifndef __SHADERS_H__
#define __SHADERS_H__

#include <SDL.h>

#ifndef __APPLE__
extern bool LoadShaderExtensions();
extern void RenderWithShader(SDL_Renderer *renderer, SDL_Window* win, SDL_Texture* backBuffer);
extern const char* NextShader();
#else
#include "stratagus.h"

inline bool LoadShaderExtensions() {
    return false;
}

inline void RenderWithShader(SDL_Renderer*, SDL_Window*, SDL_Texture*) {
    fprintf(stderr, "shaders not supported on macOS\n");
    ExitFatal(-1);
}

inline const char* NextShader() {
    return "shaders not supported on macOS";
}
#endif

#endif
