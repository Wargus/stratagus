#ifndef __SHADERS_H__
#define __SHADERS_H__
#ifdef USE_OPENGL
#define MAX_SHADERS 5
extern unsigned ShaderIndex;
#ifndef __APPLE__
extern void LoadShaders();
extern bool LoadShaderExtensions();
extern void SetupFramebuffer();
extern void RenderFramebufferToScreen();
#else
#define LoadShaders()
#define LoadShaderExtensions() false
#define SetupFramebuffer()
#define RenderFramebufferToScreen()
#endif
#endif
#endif
