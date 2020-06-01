#ifndef __SHADERS_H__
#define __SHADERS_H__

#include <SDL.h>

#ifndef __APPLE__
extern bool LoadShaderExtensions();
extern void RenderWithShader(SDL_Renderer *renderer, SDL_Window* win, SDL_Texture* backBuffer);
extern const char* NextShader();
#else
bool LoadShaderExtensions() {
}
void RenderWithShader(SDL_Renderer*, SDL_Window*, SDL_Texture*) {
}
int NextShader() {
    return 0;
}
#endif

#endif
