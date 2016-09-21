#ifndef __SHADERS_H__
#define __SHADERS_H__
#ifdef USE_OPENGL
#define MAX_SHADERS 5
// #define SHADERDEBUG // Uncomment for loading shaders from file
#ifndef __APPLE__
extern PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebuffer;
#else
#define glBindFramebuffer glBindFramebufferEXT
#endif
extern GLuint fullscreenFramebuffer;
extern unsigned ShaderIndex;
extern bool LoadShaders(int direction, char* shadernameOut);
extern bool LoadShaderExtensions();
extern void SetupFramebuffer();
extern void RenderFramebufferToScreen();
#endif
#endif
