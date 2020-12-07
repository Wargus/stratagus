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

#ifndef __APPLE__
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>

#include <stdlib.h>

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
static int currentShaderIdx = 0;

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
	GLuint vtxShaderId, fragShaderId;

	vtxShaderId = compileShader((std::string("#define VERTEX\n") + source).c_str(), GL_VERTEX_SHADER);
	fragShaderId = compileShader((std::string("#define FRAGMENT\n") + source).c_str(), GL_FRAGMENT_SHADER);

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
	int shaderFileToIdx[1024];
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

void RenderWithShader(SDL_Renderer *renderer, SDL_Window* win, SDL_Texture* backBuffer) {
	GLint oldProgramId;
	// Detach the texture
	SDL_SetRenderTarget(renderer, NULL);
	SDL_RenderClear(renderer);

	SDL_GL_BindTexture(backBuffer, NULL, NULL);
	GLuint shaderProgram = shaderPrograms[currentShaderIdx];
	if (shaderProgram != 0) {
		lazyGlGetIntegerv(GL_CURRENT_PROGRAM, &oldProgramId);
		glUseProgram(shaderProgram);
	}

	// These are the default uniforms and attrs for glsl converted libretro shaders
	GLint Texture = glGetUniformLocation(shaderProgram, "Texture");
	GLint MVPMatrix = glGetUniformLocation(shaderProgram, "MVPMatrix");
	GLint FrameDirection = glGetUniformLocation(shaderProgram, "FrameDirection");
	GLint FrameCount = glGetUniformLocation(shaderProgram, "FrameCount");
	GLint OutputSize = glGetUniformLocation(shaderProgram, "OutputSize");
	GLint TextureSize = glGetUniformLocation(shaderProgram, "TextureSize");
	GLint InputSize = glGetUniformLocation(shaderProgram, "InputSize");
	// (timfel): If I manually set the VertexCoord, it's wrong? But I have to set TexCoord? no idea...
	// GLint VertexCoord = glGetAttribLocation(shaderProgram, "VertexCoord");
	GLint TexCoord = glGetAttribLocation(shaderProgram, "TexCoord");

	// Window coordinates
	int w, h, xBorder = 0, yBorder = 0;
	SDL_GL_GetDrawableSize(win, &w, &h);

	// letterboxing
	double xScale = (double)w / Video.Width;
	double yScale = (double)h  / (Video.Height * Video.VerticalPixelSize);
	if (xScale > yScale) {
		xScale = yScale;
		xBorder = std::floor((w - (Video.Width * yScale)) / 2.0);
		w = Video.Width * yScale;
	} else {
		yScale = xScale;
		xScale = xScale * Video.VerticalPixelSize;
		yBorder = std::floor((h - (Video.Height * xScale)) / 2.0);
		h = Video.Height * xScale;
	}

	glUniform1i(Texture, 0);
	GLfloat modelview[4 * 4];
	GLfloat projection[4 * 4];
	lazyGlGetFloatv(GL_MODELVIEW_MATRIX, modelview);
	lazyGlGetFloatv(GL_PROJECTION_MATRIX, projection);
	GLfloat matrix[4 * 4] = {0.0f};
	for (int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				matrix[i * 4 + j] += modelview[i * 4 + k] * projection[k * 4 + j];
			}
		}
	}
	glUniformMatrix4fv(MVPMatrix, 1, GL_FALSE, matrix);
	glUniform1f(FrameDirection, 1);
	glUniform1f(FrameCount, 1);
	glUniform2f(OutputSize, (float)w, (float)h);
	glUniform2f(TextureSize, (float)Video.Width, (float)Video.Height);
	glUniform2f(InputSize, (float)Video.Width, (float)Video.Height);

	GLfloat minx, miny, maxx, maxy;
	GLfloat minu, maxu, minv, maxv;

	minx = 0.0f;
	miny = 0.0f;
	maxx = w;
	maxy = h;

	minu = 0.0f;
	maxu = 1.0f;
	minv = 0.0f;
	maxv = 1.0f;


	lazyGlMatrixMode(GL_PROJECTION);
	lazyGlLoadIdentity();
	lazyGlOrtho(0.0f, w, h, 0.0f, 0.0f, 1.0f);
	lazyGlViewport(xBorder, yBorder, w, h);

	lazyGlBegin(GL_TRIANGLE_STRIP);
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
	lazyGlEnd();
	SDL_GL_SwapWindow(win);

	if (shaderProgram != 0) {
		glUseProgram(oldProgramId);
	}
}

const char* NextShader() {
	if (shaderPrograms[++currentShaderIdx] == 0) {
		currentShaderIdx = 0;
	}
	std::cout << "NextShader: " << shaderNames[currentShaderIdx] << std::endl;
	return shaderNames[currentShaderIdx];
}

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
	lua_pushboolean(l, 0);
	return 1;
}

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
	if (shadersLoaded != -1) {
		return shadersLoaded == 1;
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
		shadersLoaded = loadShaders() > 0 ? 1 : 0;
	} else {
		shadersLoaded = 0;
	}

	lua_register(Lua, "GetShaderNames", CclGetShaderNames);
	lua_register(Lua, "GetShader", CclGetShader);
	lua_register(Lua, "SetShader", CclSetShader);

	return shadersLoaded == 1;
}

#endif
