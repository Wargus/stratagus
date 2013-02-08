# - Try to detect some SDL gles support
# Once done this will define
#
#  SDLGLES_FOUND - system has gles support in SDL
#  SDLGLES_TYPE - Native, Maemo or EGL
#  SDLGLES_INCLUDE_DIR - include directory for SDL gles (can be empty)
#  SDLGLES_LIBRARY - library for SDL gles (can be empty)

# Types:
#  Native - support is in SDL, program only needs to add SDL_OPENGLES flag to SDL_SetVideoMode
#  Maemo - support using SDL_gles library which was created for Maemo but should work with other systems too
#          program needs to call SDL_GLES_Init, SDL_GLES_CreateContext and SDL_GLES_MakeCurrent
#          More info on https://garage.maemo.org/projects/sdlhildon and https://wiki.maemo.org/User:Javispedro/SDL-GLES
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
		include(CMakePushCheckState)
		include(CheckTypeSize)

		cmake_push_check_state()
		set(CMAKE_REQUIRED_INCLUDES SDL include/SDL)
		set(CMAKE_EXTRA_INCLUDE_FILES SDL_video.h)
		check_type_size(SDL_OPENGLES, SDLGLES_NATIVE)
		cmake_pop_check_state()

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
		# Check for Maemo support
		find_path(SDLGLES_INCLUDE_DIR SDL_gles.h PATH_SUFFIXES SDL include/SDL)
		find_library(SDLGLES_LIBRARY NAMES SDL_gles)

		if(SDLGLES_INCLUDE_DIR AND SDLGLES_LIBRARY)
			set(SDLGLES_FOUND true)
			set(SDLGLES_TYPE "Maemo")
			message(STATUS "Found Maemo SDL gles: ${SDLGLES_LIBRARY}")
		else()
			unset(SDLGLES_INCLUDE_DIR)
			unset(SDLGLES_LIBRARY)
			message(STATUS "Could not find Maemo SDL gles")
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
