# - Try to detect some SDL gles support
# Once done this will define
#
#  SDLGLES_FOUND - system has gles support in SDL
#  SDLGLES_TYPE - Native or EGL
#  SDLGLES_INCLUDE_DIR - include directory for SDL gles (can be empty)
#  SDLGLES_LIBRARY - library for SDL gles (can be empty)

# Types:
#  Native - support is in SDL, program only needs to add SDL_OPENGLES flag to SDL_SetVideoMode
#  EGL - support using directly EGL library, see http://pandorawiki.org/Combining_OpenGL_ES_1.1_and_SDL_to_create_a_window_on_the_Pandora
#        program needs to initialize EGL and GLES manually

# Copyright (c) 2011-2013, Pali Rohár <pali.rohar@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(SDLGLES_FOUND false)

if(SDLGLES_TYPE)
	set(SDLGLES_FOUND true)
else()
	if (NOT SDLGLES_FOUND)
		# Check for Native support
		include(CheckTypeSize)

		set(CMAKE_REQUIRED_INCLUDES SDL include/SDL)
		set(CMAKE_EXTRA_INCLUDE_FILES SDL_video.h)
		check_type_size(SDL_OPENGLES, SDLGLES_NATIVE)

		if(HAVE_SDLGLES_NATIVE)
			set(SDLGLES_FOUND true)
			set(SDLGLES_TYPE "Native")
			set(SDLGLES_INCLUDE_DIR "")
			set(SDLGLES_LIBRARY "")
			message(STATUS "Found Native SDL gles")
		else()
			message(STATUS "Could not find Native SDL gles")
		endif()
	endif()

	if (NOT SDLGLES_FOUND)
		# Check for EGL support
		find_package(OpenGLES)
		if(OPENGLES_FOUND)
			set(SDLGLES_FOUND true)
			set(SDLGLES_TYPE "EGL")
			set(SDLGLES_INCLUDE_DIR "")
			set(SDLGLES_LIBRARY "")
			message(STATUS "Found EGL SDL gles")
		else()
			message(STATUS "Could not find EGL SDL gles")
		endif()
	endif()

	mark_as_advanced(SDLGLES_TYPE SDLGLES_INCLUDE_DIR SDLGLES_LIBRARY)
endif()
