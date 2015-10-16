#include "stratagus.h"
#include "video.h"
#include "SDL.h"
#include "SDL_syswm.h"
#include "SDL_opengl.h"

const char* vertex_shader = "#version 130\n\
\n\
uniform sampler2D u_texture;\n\
\n\
void main()\n\
{\n\
    gl_TexCoord[0] = gl_MultiTexCoord0;\n\
	gl_Position = ftransform();\n\
}";
#define MAX_SHADER 4
const char* fragment_shaders[MAX_SHADER] = {
	// Nearest-neighbour
	"#version 130\n\
\n\
uniform sampler2D u_texture;\n\
uniform float u_width;\n\
uniform float u_height;\n\
uniform float u_widthrel;\n\
uniform float u_heightrel;\n\
void main()\n\
{\n\
	vec4 myColor = texture2D(u_texture, gl_TexCoord[0].xy * vec2(u_widthrel, -u_heightrel));\n\
	gl_FragColor = myColor;\n\
}",
// HQX
"#version 130\n\
\n\
uniform sampler2D u_texture;\n\
uniform float u_width;\n\
uniform float u_height;\n\
uniform float u_widthrel;\n\
uniform float u_heightrel;\n\
\n\
const float mx = 0.325;      // start smoothing wt.\n\
const float k = -0.250;      // wt. decrease factor\n\
const float max_w = 0.25;    // max filter weigth\n\
const float min_w =-0.05;    // min filter weigth\n\
const float lum_add = 0.25;  // effects smoothing \n\
\n\
void main()\n\
{\n\
   vec2 v_texCoord = gl_TexCoord[0].xy * vec2(u_widthrel, -u_heightrel);\n\
\n\
   // hq2x\n\
   float x = 0.5 * (1.0 / u_width);\n\
   float y = 0.5 * (1.0 / u_height);\n\
   vec2 dg1 = vec2( x, y);\n\
   vec2 dg2 = vec2(-x, y);\n\
   vec2 dx = vec2(x, 0.0);\n\
   vec2 dy = vec2(0.0, y);\n\
\n\
   vec4 TexCoord[5];\n\
   TexCoord[0] = vec4(v_texCoord, 0.0, 0.0);\n\
   TexCoord[1].xy = TexCoord[0].xy - dg1;\n\
   TexCoord[1].zw = TexCoord[0].xy - dy;\n\
   TexCoord[2].xy = TexCoord[0].xy - dg2;\n\
   TexCoord[2].zw = TexCoord[0].xy + dx;\n\
   TexCoord[3].xy = TexCoord[0].xy + dg1;\n\
   TexCoord[3].zw = TexCoord[0].xy + dy;\n\
   TexCoord[4].xy = TexCoord[0].xy + dg2;\n\
   TexCoord[4].zw = TexCoord[0].xy - dx;\n\
\n\
   vec3 c00 = texture2D(u_texture, TexCoord[1].xy).xyz; \n\
   vec3 c10 = texture2D(u_texture, TexCoord[1].zw).xyz; \n\
   vec3 c20 = texture2D(u_texture, TexCoord[2].xy).xyz; \n\
   vec3 c01 = texture2D(u_texture, TexCoord[4].zw).xyz; \n\
   vec3 c11 = texture2D(u_texture, TexCoord[0].xy).xyz; \n\
   vec3 c21 = texture2D(u_texture, TexCoord[2].zw).xyz; \n\
   vec3 c02 = texture2D(u_texture, TexCoord[4].xy).xyz; \n\
   vec3 c12 = texture2D(u_texture, TexCoord[3].zw).xyz; \n\
   vec3 c22 = texture2D(u_texture, TexCoord[3].xy).xyz; \n\
   vec3 dt = vec3(1.0, 1.0, 1.0);\n\
\n\
   float md1 = dot(abs(c00 - c22), dt);\n\
   float md2 = dot(abs(c02 - c20), dt);\n\
\n\
   float w1 = dot(abs(c22 - c11), dt) * md2;\n\
   float w2 = dot(abs(c02 - c11), dt) * md1;\n\
   float w3 = dot(abs(c00 - c11), dt) * md2;\n\
   float w4 = dot(abs(c20 - c11), dt) * md1;\n\
\n\
   float t1 = w1 + w3;\n\
   float t2 = w2 + w4;\n\
   float ww = max(t1, t2) + 0.0001;\n\
\n\
   c11 = (w1 * c00 + w2 * c20 + w3 * c22 + w4 * c02 + ww * c11) / (t1 + t2 + ww);\n\
\n\
   float lc1 = k / (0.12 * dot(c10 + c12 + c11, dt) + lum_add);\n\
   float lc2 = k / (0.12 * dot(c01 + c21 + c11, dt) + lum_add);\n\
\n\
   w1 = clamp(lc1 * dot(abs(c11 - c10), dt) + mx, min_w, max_w);\n\
   w2 = clamp(lc2 * dot(abs(c11 - c21), dt) + mx, min_w, max_w);\n\
   w3 = clamp(lc1 * dot(abs(c11 - c12), dt) + mx, min_w, max_w);\n\
   w4 = clamp(lc2 * dot(abs(c11 - c01), dt) + mx, min_w, max_w);\n\
   \n\
   gl_FragColor = vec4(w1 * c10 + w2 * c21 + w3 * c12 + w4 * c01 + (1.0 - w1 - w2 - w3 - w4) * c11, 1);\n\
}",
// 2xSAL
"#version 130\n\
\n\
uniform sampler2D u_texture;\n\
uniform float u_width;\n\
uniform float u_height;\n\
uniform float u_widthrel;\n\
uniform float u_heightrel;\n\
\n\
void main()\n\
{\n\
   vec2 texCoord = gl_TexCoord[0].xy * vec2(u_widthrel, -u_heightrel);\n\
   vec2 UL, UR, DL, DR;\n\
   float dx = pow(u_width, -1.0) * 0.25;\n\
   float dy = pow(u_height, -1.0) * 0.25;\n\
   vec3 dt = vec3(1.0, 1.0, 1.0);\n\
   UL = texCoord + vec2(-dx, -dy);\n\
   UR = texCoord + vec2(dx, -dy);\n\
   DL = texCoord + vec2(-dx, dy);\n\
   DR = texCoord + vec2(dx, dy);\n\
   vec3 c00 = texture2D(u_texture, UL).xyz;\n\
   vec3 c20 = texture2D(u_texture, UR).xyz;\n\
   vec3 c02 = texture2D(u_texture, DL).xyz;\n\
   vec3 c22 = texture2D(u_texture, DR).xyz;\n\
   float m1=dot(abs(c00-c22),dt)+0.001;\n\
   float m2=dot(abs(c02-c20),dt)+0.001;\n\
   gl_FragColor = vec4((m1*(c02+c20)+m2*(c22+c00))/(2.0*(m1+m2)),1.0); \n\
}",
// SuperEagle
"#version 130\n\
\n\
uniform sampler2D u_texture;\n\
uniform float u_width;\n\
uniform float u_height;\n\
uniform float u_widthrel;\n\
uniform float u_heightrel;\n\
\n\
int GET_RESULT(float A, float B, float C, float D)\n\
{\n\
	int x = 0; int y = 0; int r = 0;\n\
	if (A == C) x+=1; else if (B == C) y+=1;\n\
	if (A == D) x+=1; else if (B == D) y+=1;\n\
	if (x <= 1) r+=1; \n\
	if (y <= 1) r-=1;\n\
	return r;\n\
} \n\
\n\
const vec3 dtt = vec3(65536.0,255.0,1.0);\n\
\n\
float reduce(vec3 color)\n\
{ \n\
	return dot(color, dtt);\n\
}\n\
\n\
void main()\n\
{\n\
   // get texel size   	\n\
	vec2 ps = vec2(0.999/u_width, 0.999/u_height);\n\
\n\
	vec2 v_texCoord = gl_TexCoord[0].xy * vec2(u_widthrel, -u_heightrel);\n\
\n\
	// calculating offsets, coordinates\n\
	vec2 dx = vec2( ps.x, 0.0); \n\
	vec2 dy = vec2( 0.0, ps.y);\n\
	vec2 g1 = vec2( ps.x,ps.y);\n\
	vec2 g2 = vec2(-ps.x,ps.y);	\n\
	\n\
	vec2 pixcoord  = v_texCoord/ps;	//VAR.CT\n\
	vec2 fp        = fract(pixcoord);\n\
	vec2 pC4       = v_texCoord-fp*ps;\n\
	vec2 pC8       = pC4+g1;		//VAR.CT\n\
\n\
	// Reading the texels\n\
	vec3 C0 = texture2D(u_texture,pC4-g1).xyz; \n\
	vec3 C1 = texture2D(u_texture,pC4-dy).xyz;\n\
	vec3 C2 = texture2D(u_texture,pC4-g2).xyz;\n\
	vec3 D3 = texture2D(u_texture,pC4-g2+dx).xyz;\n\
	vec3 C3 = texture2D(u_texture,pC4-dx).xyz;\n\
	vec3 C4 = texture2D(u_texture,pC4   ).xyz;\n\
	vec3 C5 = texture2D(u_texture,pC4+dx).xyz;\n\
	vec3 D4 = texture2D(u_texture,pC8-g2).xyz;\n\
	vec3 C6 = texture2D(u_texture,pC4+g2).xyz;\n\
	vec3 C7 = texture2D(u_texture,pC4+dy).xyz;\n\
	vec3 C8 = texture2D(u_texture,pC4+g1).xyz;\n\
	vec3 D5 = texture2D(u_texture,pC8+dx).xyz;\n\
	vec3 D0 = texture2D(u_texture,pC4+g2+dy).xyz;\n\
	vec3 D1 = texture2D(u_texture,pC8+g2).xyz;\n\
	vec3 D2 = texture2D(u_texture,pC8+dy).xyz;\n\
	vec3 D6 = texture2D(u_texture,pC8+g1).xyz;\n\
\n\
	vec3 p00,p10,p01,p11;\n\
\n\
	// reducing vec3 to float	\n\
	float c0 = reduce(C0);float c1 = reduce(C1);\n\
	float c2 = reduce(C2);float c3 = reduce(C3);\n\
	float c4 = reduce(C4);float c5 = reduce(C5);\n\
	float c6 = reduce(C6);float c7 = reduce(C7);\n\
	float c8 = reduce(C8);float d0 = reduce(D0);\n\
	float d1 = reduce(D1);float d2 = reduce(D2);\n\
	float d3 = reduce(D3);float d4 = reduce(D4);\n\
	float d5 = reduce(D5);float d6 = reduce(D6);\n\
\n\
	/*              SuperEagle code               */\n\
	/*  Copied from the Dosbox source code        */\n\
	/*  Copyright (C) 2002-2007  The DOSBox Team  */\n\
	/*  License: GNU-GPL                          */\n\
	/*  Adapted by guest(r) on 16.4.2007          */       \n\
	if (c4 != c8) {\n\
		if (c7 == c5) {\n\
			p01 = p10 = C7;\n\
			if ((c6 == c7) || (c5 == c2)) {\n\
					p00 = 0.25*(3.0*C7+C4);\n\
			} else {\n\
					p00 = 0.5*(C4+C5);\n\
			}\n\
\n\
			if ((c5 == d4) || (c7 == d1)) {\n\
					p11 = 0.25*(3.0*C7+C8);\n\
			} else {\n\
					p11 = 0.5*(C7+C8);\n\
			}\n\
		} else {\n\
			p11 = 0.125*(6.0*C8+C7+C5);\n\
			p00 = 0.125*(6.0*C4+C7+C5);\n\
\n\
			p10 = 0.125*(6.0*C7+C4+C8);\n\
			p01 = 0.125*(6.0*C5+C4+C8);\n\
		}\n\
	} else {\n\
		if (c7 != c5) {\n\
			p11 = p00 = C4;\n\
\n\
			if ((c1 == c4) || (c8 == d5)) {\n\
					p01 = 0.25*(3.0*C4+C5);\n\
			} else {\n\
					p01 = 0.5*(C4+C5);\n\
			}\n\
\n\
			if ((c8 == d2) || (c3 == c4)) {\n\
					p10 = 0.25*(3.0*C4+C7);\n\
			} else {\n\
					p10 = 0.5*(C7+C8);\n\
			}\n\
		} else {\n\
			int r = 0;\n\
			r += GET_RESULT(c5,c4,c6,d1);\n\
			r += GET_RESULT(c5,c4,c3,c1);\n\
			r += GET_RESULT(c5,c4,d2,d5);\n\
			r += GET_RESULT(c5,c4,c2,d4);\n\
\n\
			if (r > 0) {\n\
					p01 = p10 = C7;\n\
					p00 = p11 = 0.5*(C4+C5);\n\
			} else if (r < 0) {\n\
					p11 = p00 = C4;\n\
					p01 = p10 = 0.5*(C4+C5);\n\
			} else {\n\
					p11 = p00 = C4;\n\
					p01 = p10 = C7;\n\
			}\n\
		}\n\
	}\n\
\n\
	// Distributing the four products	\n\
	if (fp.x < 0.50)\n\
		{ if (fp.y < 0.50) p10 = p00;}\n\
	else\n\
		{ if (fp.y < 0.50) p10 = p01; else p10 = p11;}\n\
\n\
	gl_FragColor = vec4(p10, 1);\n\
}"
};


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
PFNGLACTIVETEXTUREPROC glActiveTexture;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM1IPROC glUniform1i;

GLuint fullscreenShader;
GLuint fullscreenFramebuffer = 0;
GLuint fullscreenTexture;
PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffers;
PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebuffer;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture;
PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffers;
PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbuffer;
PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorage;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbuffer;
PFNGLDRAWBUFFERSPROC glDrawBuffers;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatus;

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

unsigned shader_index = 0;

extern void LoadShaders() {
	GLuint vs, fs;
	fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, (const char**)&(fragment_shaders[shader_index]), NULL);
	glCompileShader(fs);
	printShaderInfoLog(fs, "Fragment Shader");
	shader_index = (shader_index + 1) % MAX_SHADER;
	vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, (const char**)&vertex_shader, NULL);
	glCompileShader(vs);
	printShaderInfoLog(vs, "Vertex Shader");
	if (glIsProgram(fullscreenShader)) {
		glDeleteProgram(fullscreenShader);
	}
	fullscreenShader = glCreateProgram();
	glAttachShader(fullscreenShader, vs);
	glAttachShader(fullscreenShader, fs);
	glLinkProgram(fullscreenShader);
	glDeleteShader(fs);
	glDeleteShader(vs);
	printProgramInfoLog(fullscreenShader, "Shader Program");
}

extern bool LoadShaderExtensions() {
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
	glActiveTexture = (PFNGLACTIVETEXTUREPROC)(uintptr_t)SDL_GL_GetProcAddress("glActiveTexture");
	glUniform1f = (PFNGLUNIFORM1FPROC)(uintptr_t)SDL_GL_GetProcAddress("glUniform1f");
	glUniform1i = (PFNGLUNIFORM1IPROC)(uintptr_t)SDL_GL_GetProcAddress("glUniform1i");

	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSEXTPROC)(uintptr_t)SDL_GL_GetProcAddress("glGenFramebuffers");
	glBindFramebuffer = (PFNGLBINDFRAMEBUFFEREXTPROC)(uintptr_t)SDL_GL_GetProcAddress("glBindFramebuffer");
	glFramebufferTexture = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)(uintptr_t)SDL_GL_GetProcAddress("glFramebufferTexture2D");
	glGenRenderbuffers = (PFNGLGENRENDERBUFFERSEXTPROC)(uintptr_t)SDL_GL_GetProcAddress("glGenRenderbuffers");
	glBindRenderbuffer = (PFNGLBINDRENDERBUFFEREXTPROC)(uintptr_t)SDL_GL_GetProcAddress("glBindRenderbuffer");
	glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEEXTPROC)(uintptr_t)SDL_GL_GetProcAddress("glRenderbufferStorage");
	glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)(uintptr_t)SDL_GL_GetProcAddress("glFramebufferRenderbuffer");
	glDrawBuffers = (PFNGLDRAWBUFFERSPROC)(uintptr_t)SDL_GL_GetProcAddress("glDrawBuffers");
	glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)(uintptr_t)SDL_GL_GetProcAddress("glCheckFramebufferStatus");

	if (glCreateShader && glGenFramebuffers && glGetUniformLocation && glActiveTexture) {
		LoadShaders();
		return true;
	} else {
		return false;
	}
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
	GLint textureloc = glGetUniformLocation(fullscreenShader, "u_texture");
	GLint widthloc = glGetUniformLocation(fullscreenShader, "u_width");
	GLint heightloc = glGetUniformLocation(fullscreenShader, "u_height");
	GLint widthrelloc = glGetUniformLocation(fullscreenShader, "u_widthrel");
	GLint heightrelloc = glGetUniformLocation(fullscreenShader, "u_heightrel");
	glUniform1f(widthloc, Video.ViewportWidth);
	glUniform1f(heightloc, Video.ViewportHeight);
	glUniform1f(widthrelloc, (float)Video.Width / (float)Video.ViewportWidth);
	glUniform1f(heightrelloc, (float)Video.Height / (float)Video.ViewportHeight);
	glUniform1i(textureloc, 0);
	glActiveTexture(GL_TEXTURE0);
	// render the framebuffer texture to a fullscreen quad on the real display
	glBindTexture(GL_TEXTURE_2D, fullscreenTexture);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2i(0, 0);
	glTexCoord2f(1, 0);
	glVertex2i(Video.ViewportWidth, 0);
	glTexCoord2f(1, 1);
	glVertex2i(Video.ViewportWidth, Video.ViewportHeight);
	glTexCoord2f(0, 1);
	glVertex2i(0, Video.ViewportHeight);
	glEnd();
	SDL_GL_SwapBuffers();
	glUseProgram(0); // Disable shaders again, and render to framebuffer again
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, fullscreenFramebuffer);
}
