/**
 * MIT License
 *
 * Copyright (c) 2020 Tim Felgentreff
 * Copyright (c) 2017 Augusto Ruiz
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "shaders.h"
#include <stdint.h>

#ifndef __APPLE__
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>

#include <stdlib.h>
#include <regex>
#include <iostream>
#include <fstream>

#include "stratagus.h"
#include "parameters.h"
#include "video.h"
#include "game.h"
#include "iolib.h"
#include "script.h"

// Avoiding the use of GLEW or some extensions handler
PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLVALIDATEPROGRAMPROC glValidateProgram;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLUSEPROGRAMPROC glUseProgram;

PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;

PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
PFNGLVERTEXATTRIB4FPROC glVertexAttrib4f;

#ifdef WIN32
#define CCONV __stdcall
#else
#define CCONV
#endif

void (CCONV *lazyGlBegin)(GLenum);
void (CCONV *lazyGlEnd)(void);
void (CCONV *lazyGlTexCoord2f)(GLfloat, GLfloat);
void (CCONV *lazyGlVertex2f)(GLfloat, GLfloat);
void (CCONV *lazyGlGetIntegerv)(GLenum, GLint*);
void (CCONV *lazyGlGetFloatv)(GLenum, GLfloat*);
void (CCONV *lazyGlViewport)(GLint, GLint, GLsizei, GLsizei);
void (CCONV *lazyGlMatrixMode)(GLenum);
void (CCONV *lazyGlLoadIdentity)(void);
void (CCONV *lazyGlOrtho)(GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);

static const int MAX_SHADERS = 128;
static GLuint shaderPrograms[MAX_SHADERS + 1] = { (GLuint) 0 };
static const char* shaderNames[MAX_SHADERS + 1] = { NULL };
static char shadersLoaded = -1;
#define canUseShaders (shadersLoaded == 1)
#define shadersAreInitialized (shadersLoaded != -1)
#define setCanUseShaders(x) (shadersLoaded = (x ? 1 : 0))
static int currentShaderIdx = 0;

static std::regex invalidQuoteReplaceRegex("\"([a-zA-Z0-9 -\\.]+)\"");

const char* none =
#include "./shaders/noshader.glsl"
;
const char* CRT =
#include "./shaders/crt.glsl"
;
const char* VHS =
#include "./shaders/vhs.glsl"
;
const char* xBRZ =
#include "./shaders/xbrz.glsl"
;

static GLuint compileShader(const char* source, GLuint shaderType) {
	// Create ID for shader
	GLuint result = glCreateShader(shaderType);
	// Define shader text
	glShaderSource(result, 1, &source, NULL);
	// Compile shader
	glCompileShader(result);

	// Check vertex shader for errors
	GLint shaderCompiled = GL_FALSE;
	glGetShaderiv( result, GL_COMPILE_STATUS, &shaderCompiled );
	if( shaderCompiled != GL_TRUE ) {
		std::cout << "Error during compilation: " << result << "!" << std::endl;
		GLint logLength;
		glGetShaderiv(result, GL_INFO_LOG_LENGTH, &logLength);
		if (logLength > 0) {
			GLchar *log = (GLchar*)malloc(logLength);
			glGetShaderInfoLog(result, logLength, &logLength, log);
			std::cout << "Shader compile log: " << log << std::endl;
			free(log);
		}
		glDeleteShader(result);
		result = 0;
	} else {
		std::cout << "Shader compiled correctly. Id = " << result << std::endl;
	}
	return result;
}

static GLuint compileProgramSource(std::string source) {
	GLuint programId = 0;
	GLuint vtxShaderId = 0;
	GLuint fragShaderId = 0;

	uint32_t offset = 0;
	std::smatch m;
	while (std::regex_search(source.cbegin() + offset, source.cend(), m, invalidQuoteReplaceRegex)) {
		uint32_t next_offset = offset + m.position() + m.length();
		for (std::string::iterator it = source.begin() + offset + m.position(); it < source.begin() + next_offset; it++) {
			if (*it == '"') {
				source.replace(it, it + 1, " ");
			} else if (*it == ' ') {
				source.replace(it, it + 1, "_");
			}
		}
		offset = next_offset;
	}

	vtxShaderId = compileShader((std::string("#define VERTEX\n") + source).c_str(), GL_VERTEX_SHADER);
	if (vtxShaderId) {
		fragShaderId = compileShader((std::string("#define FRAGMENT\n") + source).c_str(), GL_FRAGMENT_SHADER);
	}

	if(vtxShaderId && fragShaderId) {
		programId = glCreateProgram();
		// Associate shader with program
		glAttachShader(programId, vtxShaderId);
		glAttachShader(programId, fragShaderId);
		glLinkProgram(programId);
		glValidateProgram(programId);

		// Check the status of the compile/link
		GLint logLen;
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &logLen);
		if(logLen > 0) {
			char* log = (char*) malloc(logLen * sizeof(char));
			// Show any errors as appropriate
			glGetProgramInfoLog(programId, logLen, &logLen, log);
			std::cout << "Prog Info Log: " << std::endl << log << std::endl;
			free(log);
		}
	}
	if(vtxShaderId) {
		glDeleteShader(vtxShaderId);
	}
	if(fragShaderId) {
		glDeleteShader(fragShaderId);
	}
	return programId;
}

static GLuint compileProgram(std::string shaderFile) {
	std::ifstream f(shaderFile);
	std::string source((std::istreambuf_iterator<char>(f)),
						std::istreambuf_iterator<char>());
	std::cout << "[Shaders] Compiling shader: " << shaderFile << std::endl;
	return compileProgramSource(source);
}

static int loadShaders() {
	int numShdr = 0;

#define COMPILE_BUILTIN_SHADER(name)									\
	std::cout << "[Shaders] Compiling shader: " #name << std::endl;		\
	shaderPrograms[numShdr] = compileProgramSource(std::string(name));	\
	shaderNames[numShdr] = #name ;										\
	if (shaderPrograms[numShdr] != 0) {									\
		numShdr++;														\
	}
	COMPILE_BUILTIN_SHADER(none);
	COMPILE_BUILTIN_SHADER(xBRZ);
	COMPILE_BUILTIN_SHADER(CRT);
	COMPILE_BUILTIN_SHADER(VHS);
#undef COMPILE_BUILTIN_SHADER

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
	cShaderPath = (char*)shaderPath.c_str();
#endif
	int n = ReadDataDirectory(cShaderPath, flp);
	for (int i = 0; i < n; ++i) {
		int pos = flp[i].name.find(".glsl");
		if (pos > 0) {
			GLuint program = compileProgram(shaderPath + flp[i].name);
			if (program) {
				shaderPrograms[numShdr] = program;
				shaderNames[numShdr] = strdup(flp[i].name.c_str());
				numShdr += 1;
				if (numShdr >= MAX_SHADERS) {
					break;
				}
			}
		}
	}

	return numShdr;
}

// caches that don't change for a shader program
static int LastShaderIndex = -1;
static GLuint ShaderProgram;
static GLint Texture;
static GLint MVPMatrix;
static GLint FrameDirection;
static GLint FrameCount;
static GLint OutputSize;
static GLint TextureSize;
static GLint InputSize;
static GLint VertexCoord;
static GLint TexCoord;
// caches that don't change for a window size
static int RecacheCount = 0; // rechache multiple times - races!
extern uint8_t SizeChangeCounter; // from sdl.cpp
static uint16_t LastSizeVersion = -1;
static int LastVideoWidth = -1;
static int LastVideoHeight = -1;
static double LastVideoVerticalPixelSize = -1.0;
static int DrawableWidth;
static int DrawableHeight;
static int XBorder;
static int YBorder;
static GLfloat modelview[4 * 4];
static GLfloat projection[4 * 4];
static GLfloat matrix[4 * 4] = {0.0f};

static bool RenderWithShaderInternal(SDL_Renderer *renderer, SDL_Window* win, SDL_Texture* backBuffer);

// keep this function small, so the compiler can inline it
bool RenderWithShader(SDL_Renderer *renderer, SDL_Window* win, SDL_Texture* backBuffer) {
	if (!canUseShaders || currentShaderIdx == 0) {
		return false;
	}
	return RenderWithShaderInternal(renderer, win, backBuffer);
}

static bool RenderWithShaderInternal(SDL_Renderer *renderer, SDL_Window* win, SDL_Texture* backBuffer) {
	GLint oldProgramId;
	// Detach the texture
	SDL_SetRenderTarget(renderer, NULL);
	SDL_RenderClear(renderer);

	SDL_GL_BindTexture(backBuffer, NULL, NULL);
	if (LastShaderIndex != currentShaderIdx) {
		LastShaderIndex = currentShaderIdx;
		// force to recalculate everything based on size, too
		LastSizeVersion = -1;

		ShaderProgram = shaderPrograms[currentShaderIdx];
		if (ShaderProgram != 0) {
			// These are the default uniforms and attrs for glsl converted libretro shaders
			Texture = glGetUniformLocation(ShaderProgram, "Texture");
			MVPMatrix = glGetUniformLocation(ShaderProgram, "MVPMatrix");
			FrameDirection = glGetUniformLocation(ShaderProgram, "FrameDirection");
			FrameCount = glGetUniformLocation(ShaderProgram, "FrameCount");
			OutputSize = glGetUniformLocation(ShaderProgram, "OutputSize");
			TextureSize = glGetUniformLocation(ShaderProgram, "TextureSize");
			InputSize = glGetUniformLocation(ShaderProgram, "InputSize");
			// (timfel): If I manually set the VertexCoord, it's wrong? But I have to set TexCoord? no idea...
			// VertexCoord = glGetAttribLocation(ShaderProgram, "VertexCoord");
			TexCoord = glGetAttribLocation(ShaderProgram, "TexCoord");
		}

		// the texture is always 0, never changes
		glUniform1i(Texture, 0);
		glUniform1f(FrameDirection, 1);
		glUniform1f(FrameCount, 1);
	}

	if (ShaderProgram != 0) {
		lazyGlGetIntegerv(GL_CURRENT_PROGRAM, &oldProgramId);
		glUseProgram(ShaderProgram);
	}

	if (RecacheCount > 0 || LastSizeVersion != SizeChangeCounter || LastVideoWidth != Video.Width || LastVideoHeight != Video.Height || LastVideoVerticalPixelSize != Video.VerticalPixelSize) {
		if (RecacheCount == 0) {
			RecacheCount = 60;
		} else {
			RecacheCount--;
		}
		LastSizeVersion = SizeChangeCounter;
		LastVideoWidth = Video.Width;
		LastVideoHeight = Video.Height;
		LastVideoVerticalPixelSize = Video.VerticalPixelSize;

		// Window coordinates
		SDL_GL_GetDrawableSize(win, &DrawableWidth, &DrawableHeight);

		// letterboxing
		double xScale = (double)DrawableWidth / LastVideoWidth;
		double yScale = (double)DrawableHeight / (LastVideoHeight * LastVideoVerticalPixelSize);
		if (xScale > yScale) {
			xScale = yScale;
			XBorder = std::floor((DrawableWidth - (LastVideoWidth * yScale)) / 2.0);
			YBorder = 0;
			DrawableWidth = LastVideoWidth * yScale;
		} else {
			yScale = xScale;
			xScale = xScale * LastVideoVerticalPixelSize;
			XBorder = 0;
			YBorder = std::floor((DrawableHeight - (LastVideoHeight * xScale)) / 2.0);
			DrawableHeight = LastVideoHeight * xScale;
		}

		// these uniforms only change with the video size
		glUniform2f(OutputSize, (float)DrawableWidth, (float)DrawableHeight);
		glUniform2f(TextureSize, (float)LastVideoWidth, (float)LastVideoHeight);
		glUniform2f(InputSize, (float)LastVideoWidth, (float)LastVideoHeight);

		lazyGlGetFloatv(GL_MODELVIEW_MATRIX, modelview);
		lazyGlGetFloatv(GL_PROJECTION_MATRIX, projection);
		memset(matrix, 0, sizeof(matrix));
		for (int i = 0; i < 4; i++) {
			for(int j = 0; j < 4; j++) {
				for (int k = 0; k < 4; k++) {
					matrix[i * 4 + j] += modelview[i * 4 + k] * projection[k * 4 + j];
				}
			}
		}
		glUniformMatrix4fv(MVPMatrix, 1, GL_FALSE, matrix);
	}

	const GLfloat minx = 0.0f;
	const GLfloat miny = 0.0f;
	const GLfloat maxx = DrawableWidth;
	const GLfloat maxy = DrawableHeight;

	const GLfloat minu = 0.0f;
	const GLfloat maxu = 1.0f;
	const GLfloat minv = 0.0f;
	const GLfloat maxv = 1.0f;

	lazyGlMatrixMode(GL_PROJECTION);
	lazyGlLoadIdentity();
	lazyGlOrtho(0.0f, DrawableWidth, DrawableHeight, 0.0f, 0.0f, 1.0f);
	lazyGlViewport(XBorder, YBorder, DrawableWidth, DrawableHeight);

	lazyGlBegin(GL_TRIANGLE_STRIP); {
		glVertexAttrib4f(TexCoord, minu, minv, 0, 0);
		lazyGlTexCoord2f(minu, minv);
		lazyGlVertex2f(minx, miny);

		glVertexAttrib4f(TexCoord, maxu, minv, 0, 0);
		lazyGlTexCoord2f(maxu, minv);
		lazyGlVertex2f(maxx, miny);

		glVertexAttrib4f(TexCoord, minu, maxv, 0, 0);
		lazyGlTexCoord2f(minu, maxv);
		lazyGlVertex2f(minx, maxy);

		glVertexAttrib4f(TexCoord, maxu, maxv, 0, 0);
		lazyGlTexCoord2f(maxu, maxv);
		lazyGlVertex2f(maxx, maxy);
	} lazyGlEnd();
	// SDL_GL_SwapWindow(win);

	if (ShaderProgram != 0) {
		glUseProgram(oldProgramId);
	}

	return true;
}

/**
** <b>Description</b>
**
**  Get the active shader.
**
** Example:
**
** <div class="example"><code>shader_name = <strong>GetShader</strong>()
**	print(shader_name)</code></div>
*/
static int CclGetShader(lua_State *l) {
	LuaCheckArgs(l, 0);
	const char* shaderName = shaderNames[currentShaderIdx];
	if (shaderName) {
		lua_pushstring(l, shaderName);
	} else {
		lua_pushnil(l);
	}
	return 1;
}

/**
** <b>Description</b>
**
**  Apply a shader.
**
** Example:
**
** <div class="example"><code>-- Apply a VHS shader
**	<strong>SetShader</strong>("VHS")</code></div>
*/
static int CclSetShader(lua_State *l) {
	LuaCheckArgs(l, 1);
	const char* shaderName = LuaToString(l, 1);
	for (int i = 0; i < MAX_SHADERS; i++) {
		const char* n = shaderNames[i];
		if (n) {
			if (!strcmp(n, shaderName)) {
				currentShaderIdx = i;
				std::cout << "SetShader: " << shaderNames[currentShaderIdx] << std::endl;
				lua_pushboolean(l, 1);
				return 1;
			}
		} else {
			break;
		}
	}
	currentShaderIdx = 0;
	lua_pushboolean(l, 0);
	return 1;
}

/**
** <b>Description</b>
**
**  Get the list of shaders.
**
** Example:
**
** <div class="example"><code>shaders = <strong>GetShaderNames</strong>()
**	for i,name in ipairs(shaders) do
**		print(name)
**	end</code></div>
*/
static int CclGetShaderNames(lua_State *l) {
	LuaCheckArgs(l, 0);
	lua_newtable(l);
	for (int i = 0; shaderNames[i] != NULL; i++) {
		lua_pushstring(l, shaderNames[i]);
		lua_rawseti(l, -2, i + 1);
	}
	return 1;
}

bool LoadShaderExtensions() {
	if (shadersAreInitialized) {
		return canUseShaders;
	}

	*(void **) (&lazyGlBegin) = SDL_GL_GetProcAddress("glBegin");
	*(void **) (&lazyGlEnd) = SDL_GL_GetProcAddress("glEnd");
	*(void **) (&lazyGlTexCoord2f) = SDL_GL_GetProcAddress("glTexCoord2f");
	*(void **) (&lazyGlVertex2f) = SDL_GL_GetProcAddress("glVertex2f");
	*(void **) (&lazyGlGetIntegerv) = SDL_GL_GetProcAddress("glGetIntegerv");
	*(void **) (&lazyGlGetFloatv) = SDL_GL_GetProcAddress("glGetFloatv");
	*(void **) (&lazyGlViewport) = SDL_GL_GetProcAddress("glViewport");
	*(void **) (&lazyGlMatrixMode) = SDL_GL_GetProcAddress("glMatrixMode");
	*(void **) (&lazyGlOrtho) = SDL_GL_GetProcAddress("glOrtho");
	*(void **) (&lazyGlLoadIdentity) = SDL_GL_GetProcAddress("glLoadIdentity");

	glCreateShader = (PFNGLCREATESHADERPROC)SDL_GL_GetProcAddress("glCreateShader");
	glShaderSource = (PFNGLSHADERSOURCEPROC)SDL_GL_GetProcAddress("glShaderSource");
	glCompileShader = (PFNGLCOMPILESHADERPROC)SDL_GL_GetProcAddress("glCompileShader");
	glGetShaderiv = (PFNGLGETSHADERIVPROC)SDL_GL_GetProcAddress("glGetShaderiv");
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)SDL_GL_GetProcAddress("glGetShaderInfoLog");
	glDeleteShader = (PFNGLDELETESHADERPROC)SDL_GL_GetProcAddress("glDeleteShader");
	glAttachShader = (PFNGLATTACHSHADERPROC)SDL_GL_GetProcAddress("glAttachShader");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)SDL_GL_GetProcAddress("glCreateProgram");
	glLinkProgram = (PFNGLLINKPROGRAMPROC)SDL_GL_GetProcAddress("glLinkProgram");
	glValidateProgram = (PFNGLVALIDATEPROGRAMPROC)SDL_GL_GetProcAddress("glValidateProgram");
	glGetProgramiv = (PFNGLGETPROGRAMIVPROC)SDL_GL_GetProcAddress("glGetProgramiv");
	glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)SDL_GL_GetProcAddress("glGetProgramInfoLog");
	glUseProgram = (PFNGLUSEPROGRAMPROC)SDL_GL_GetProcAddress("glUseProgram");
	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)SDL_GL_GetProcAddress("glGetUniformLocation");
	glUniform1i = (PFNGLUNIFORM1IPROC)SDL_GL_GetProcAddress("glUniform1i");
	glUniform1f = (PFNGLUNIFORM1FPROC)SDL_GL_GetProcAddress("glUniform1f");
	glUniform2f = (PFNGLUNIFORM2FPROC)SDL_GL_GetProcAddress("glUniform2f");
	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)SDL_GL_GetProcAddress("glUniformMatrix4fv");
	glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)SDL_GL_GetProcAddress("glGetAttribLocation");
	glVertexAttrib4f = (PFNGLVERTEXATTRIB4FPROC)SDL_GL_GetProcAddress("glVertexAttrib4f");

	if (lazyGlBegin && lazyGlEnd && lazyGlTexCoord2f && lazyGlVertex2f && lazyGlGetIntegerv &&
		glCreateShader && glShaderSource && glCompileShader && glGetShaderiv &&
		glGetShaderInfoLog && glDeleteShader && glAttachShader && glCreateProgram &&
		glLinkProgram && glValidateProgram && glGetProgramiv && glGetProgramInfoLog &&
		glUseProgram && glGetUniformLocation && glUniform1i && glUniform1f && glUniform2f &&
		glUniformMatrix4fv && glGetAttribLocation && glVertexAttrib4f) {
		setCanUseShaders(loadShaders() > 0);
	} else {
		setCanUseShaders(false);
	}

	lua_register(Lua, "GetShaderNames", CclGetShaderNames);
	lua_register(Lua, "GetShader", CclGetShader);
	lua_register(Lua, "SetShader", CclSetShader);

	return canUseShaders;
}

#endif
