#include "stratagus.h"
#include "parameters.h"
#include "video.h"
#include "game.h"
#include "iolib.h"
#include <iostream>
#include <fstream>

#ifdef USE_OPENGL

#ifndef __APPLE__
PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLISPROGRAMPROC glIsProgram;
PFNGLDELETEPROGRAMPROC glDeleteProgram;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLACTIVETEXTUREPROC glActiveTextureProc;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffers;
PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebuffer;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture;
PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffers;
PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbuffer;
PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorage;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbuffer;
PFNGLDRAWBUFFERSPROC glDrawBuffers;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatus;
#else
#define glGenFramebuffers glGenFramebuffersEXT
#define glBindFramebuffer glBindFramebufferEXT
#define glCheckFramebufferStatus glCheckFramebufferStatusEXT
#define glActiveTextureProc glActiveTexture
#define glFramebufferTexture glFramebufferTexture2DEXT
#endif

GLuint fullscreenShader;
GLuint fullscreenFramebuffer = 0;
GLuint fullscreenTexture;

void printShaderInfoLog(GLuint obj, const char* prefix)
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		fprintf(stdout, "%s: %s\n", prefix, infoLog);
		free(infoLog);
	}
}
void printProgramInfoLog(GLuint obj, const char* prefix)
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog;

	glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		fprintf(stdout, "%s: %s\n", prefix, infoLog);
		free(infoLog);
	}
}

/* This does not have to be very efficient, it is only called when the shader
   is changed by the user.
 */
extern bool LoadShaders(int direction, char* shadernameOut) {
	Video.ShaderIndex += direction;
	if (direction == 0 && Video.ShaderIndex == -1) {
		Video.ShaderIndex = 0;
	}

	GLuint vs, fs;
	GLint params;
	fs = glCreateShader(GL_FRAGMENT_SHADER);
	if (fs == 0) {
	    return false;
	}

	std::vector<FileList> flp;
	std::string shaderPath(StratagusLibPath);
	char *cShaderPath;
#ifdef _WIN32
	shaderPath.append("\\shaders\\");
	int fullpathsize = ExpandEnvironmentStrings(shaderPath.c_str(), NULL, 0);
	cShaderPath = (char*)calloc(fullpathsize + 1, sizeof(char));
	ExpandEnvironmentStrings(shaderPath.c_str(), cShaderPath, fullpathsize);
#else
	shaderPath.append("/shaders/");
	cShaderPath = shaderPath.c_str();
#endif
	int n = ReadDataDirectory(cShaderPath, flp);
	int numShaderFiles = 0;
	int shaderFileToIdx[1024];
	for (int i = 0; i < n; ++i) {
		int pos = flp[i].name.find(".glsl");
		if (pos > 0) {
			shaderFileToIdx[numShaderFiles] = i;
			numShaderFiles++;
		}
	}
	if (numShaderFiles <= 0) return false;
	while (Video.ShaderIndex < 0) {
		Video.ShaderIndex = numShaderFiles + Video.ShaderIndex;
	}
	Video.ShaderIndex = Video.ShaderIndex % numShaderFiles;

	if (shadernameOut) {
		strncpy(shadernameOut, flp[shaderFileToIdx[Video.ShaderIndex]].name.c_str(), 1023);
	}
	shaderPath.append(flp[shaderFileToIdx[Video.ShaderIndex]].name);
	std::ifstream myfile(shaderPath.c_str());
	std::string contents((std::istreambuf_iterator<char>(myfile)),
						  std::istreambuf_iterator<char>());
	myfile.close();

	const char *fragmentSrc[2] = { "#define FRAGMENT\n", contents.c_str() };
	const char *vertexSrc[2] = { "#define VERTEX\n", contents.c_str() };

	glShaderSource(fs, 2, fragmentSrc, NULL);
	glCompileShader(fs);
	glGetShaderiv(fs, GL_COMPILE_STATUS, &params);
	if (params == GL_FALSE) {
		printShaderInfoLog(fs, "Fragment Shader");
		glDeleteShader(fs);
		return false;
	}
	vs = glCreateShader(GL_VERTEX_SHADER);
	if (fs == 0) {
		glDeleteShader(fs);
		return false;
	}
	glShaderSource(vs, 2, vertexSrc, NULL);
	glCompileShader(vs);
	glGetShaderiv(fs, GL_COMPILE_STATUS, &params);
	if (params == GL_FALSE) {
		printShaderInfoLog(vs, "Vertex Shader");
		glDeleteShader(fs);
		glDeleteShader(vs);
		return false;
	}
	if (glIsProgram(fullscreenShader)) {
		glDeleteProgram(fullscreenShader);
	}
	fullscreenShader = glCreateProgram();
	if (fullscreenShader == 0) {
		glDeleteShader(fs);
		glDeleteShader(vs);
		return false;
	}
	glAttachShader(fullscreenShader, vs);
	glAttachShader(fullscreenShader, fs);
	glLinkProgram(fullscreenShader);
	glGetProgramiv(fullscreenShader, GL_LINK_STATUS, &params);
	if (params == GL_FALSE) {
		printProgramInfoLog(fullscreenShader, "Shader Program");
		glDeleteShader(fs);
		glDeleteShader(vs);
		glDeleteProgram(fullscreenShader);
		return false;
	}
	glDeleteShader(fs);
	glDeleteShader(vs);
	return true;
}

extern bool LoadShaderExtensions() {
#ifndef __APPLE__
	glCreateShader = (PFNGLCREATESHADERPROC)(uintptr_t)SDL_GL_GetProcAddress("glCreateShader");
	glShaderSource = (PFNGLSHADERSOURCEPROC)(uintptr_t)SDL_GL_GetProcAddress("glShaderSource");
	glCompileShader = (PFNGLCOMPILESHADERPROC)(uintptr_t)SDL_GL_GetProcAddress("glCompileShader");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)(uintptr_t)SDL_GL_GetProcAddress("glCreateProgram");
	glAttachShader = (PFNGLATTACHSHADERPROC)(uintptr_t)SDL_GL_GetProcAddress("glAttachShader");
	glLinkProgram = (PFNGLLINKPROGRAMPROC)(uintptr_t)SDL_GL_GetProcAddress("glLinkProgram");
	glUseProgram = (PFNGLUSEPROGRAMPROC)(uintptr_t)SDL_GL_GetProcAddress("glUseProgram");
	glGetShaderiv = (PFNGLGETSHADERIVPROC)(uintptr_t)SDL_GL_GetProcAddress("glGetShaderiv");
	glGetProgramiv = (PFNGLGETPROGRAMIVPROC)(uintptr_t)SDL_GL_GetProcAddress("glGetProgramiv");
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)(uintptr_t)SDL_GL_GetProcAddress("glGetShaderInfoLog");
	glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)(uintptr_t)SDL_GL_GetProcAddress("glGetProgramInfoLog");
	glIsProgram = (PFNGLISPROGRAMPROC)(uintptr_t)SDL_GL_GetProcAddress("glIsProgram");
	glDeleteProgram = (PFNGLDELETEPROGRAMPROC)(uintptr_t)SDL_GL_GetProcAddress("glDeleteProgram");
	glDeleteShader = (PFNGLDELETESHADERPROC)(uintptr_t)SDL_GL_GetProcAddress("glDeleteShader");

	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)(uintptr_t)SDL_GL_GetProcAddress("glGetUniformLocation");
	glActiveTextureProc = (PFNGLACTIVETEXTUREPROC)(uintptr_t)SDL_GL_GetProcAddress("glActiveTexture");
	glUniform1f = (PFNGLUNIFORM1FPROC)(uintptr_t)SDL_GL_GetProcAddress("glUniform1f");
	glUniform2f = (PFNGLUNIFORM2FPROC)(uintptr_t)SDL_GL_GetProcAddress("glUniform2f");
	glUniform1i = (PFNGLUNIFORM1IPROC)(uintptr_t)SDL_GL_GetProcAddress("glUniform1i");
	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)(uintptr_t)SDL_GL_GetProcAddress("glUniformMatrix4fv");

	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSEXTPROC)(uintptr_t)SDL_GL_GetProcAddress("glGenFramebuffers");
	glBindFramebuffer = (PFNGLBINDFRAMEBUFFEREXTPROC)(uintptr_t)SDL_GL_GetProcAddress("glBindFramebuffer");
	glFramebufferTexture = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)(uintptr_t)SDL_GL_GetProcAddress("glFramebufferTexture2D");
	glGenRenderbuffers = (PFNGLGENRENDERBUFFERSEXTPROC)(uintptr_t)SDL_GL_GetProcAddress("glGenRenderbuffers");
	glBindRenderbuffer = (PFNGLBINDRENDERBUFFEREXTPROC)(uintptr_t)SDL_GL_GetProcAddress("glBindRenderbuffer");
	glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEEXTPROC)(uintptr_t)SDL_GL_GetProcAddress("glRenderbufferStorage");
	glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)(uintptr_t)SDL_GL_GetProcAddress("glFramebufferRenderbuffer");
	glDrawBuffers = (PFNGLDRAWBUFFERSPROC)(uintptr_t)SDL_GL_GetProcAddress("glDrawBuffers");
	glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)(uintptr_t)SDL_GL_GetProcAddress("glCheckFramebufferStatus");
	if (glCreateShader && glGenFramebuffers && glGetUniformLocation && glActiveTextureProc) {
		return LoadShaders(0, NULL);
	} else {
		return false;
	}
#else
	return false; // FIXME: Does not currently work on OSX
#endif
}

extern void SetupFramebuffer() {
	glGenTextures(1, &fullscreenTexture); // generate a texture to render to off-screen
	glBindTexture(GL_TEXTURE_2D, fullscreenTexture); // bind it, so all texture functions go to it
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Video.ViewportWidth, Video.ViewportHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0); // give an empty image to opengl
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // make sure we use nearest filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glGenFramebuffers(1, &fullscreenFramebuffer); // generate a framebuffer to render to
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, fullscreenFramebuffer); // bind it
	glFramebufferTexture(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, fullscreenTexture, 0); // set our texture as the "screen" of the framebuffer
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0_EXT };
	glDrawBuffers(1, DrawBuffers);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT) {
		fprintf(stderr, "FATAL: Error Creating Framebuffer! Try running without OpenGL.");
		exit(-1);
	}
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, fullscreenFramebuffer);
}

extern void RenderFramebufferToScreen() {
	// switch the rendering target back to the real display
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
	// setup our shader program
	glUseProgram(fullscreenShader);

	// These are the default uniforms for glsl converted libretro shaders
	GLint Texture = glGetUniformLocation(fullscreenShader, "Texture");
	GLint MVPMatrix = glGetUniformLocation(fullscreenShader, "MVPMatrix");
	GLint FrameDirection = glGetUniformLocation(fullscreenShader, "FrameDirection");
	GLint FrameCount = glGetUniformLocation(fullscreenShader, "FrameCount");
	GLint OutputSize = glGetUniformLocation(fullscreenShader, "OutputSize");
	GLint TextureSize = glGetUniformLocation(fullscreenShader, "TextureSize");
	GLint InputSize = glGetUniformLocation(fullscreenShader, "InputSize");

	glUniform1i(Texture, 0);
	GLfloat matrix[4 * 4];
	glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	glUniformMatrix4fv(MVPMatrix, 1, GL_FALSE, matrix);
	glUniform1f(FrameDirection, 1);
	glUniform1f(FrameCount, 1);
	glUniform2f(OutputSize, (float)Video.ViewportWidth, (float)Video.ViewportHeight);
	glUniform2f(TextureSize, (float)Video.ViewportWidth, (float)Video.ViewportHeight);
	glUniform2f(InputSize, (float)Video.Width, (float)Video.Height);

	float widthRel = (float)Video.Width / Video.ViewportWidth;
	float heightRel = (float)Video.Height / Video.ViewportHeight;

	glActiveTextureProc(GL_TEXTURE0);
	// render the framebuffer texture to a fullscreen quad on the real display
	glBindTexture(GL_TEXTURE_2D, fullscreenTexture);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 1);
	glVertex2i(0, 0);
	glTexCoord2f(widthRel, 1);
	glVertex2i(Video.ViewportWidth, 0);
	glTexCoord2f(widthRel, 1 - heightRel);
	glVertex2i(Video.ViewportWidth, Video.ViewportHeight);
	glTexCoord2f(0, 1 - heightRel);
	glVertex2i(0, Video.ViewportHeight);
	glEnd();
	SDL_GL_SwapBuffers();
	glUseProgram(0); // Disable shaders again, and render to framebuffer again
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, fullscreenFramebuffer);
}
#endif
