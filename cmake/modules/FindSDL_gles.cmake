# - Try to find the SDL_gles library
# Once done this will define
#
#  SDLGLES_FOUND - system has SDL_gles
#  SDLGLES_INCLUDE_DIR - the SDL_gles include directory
#  SDLGLES_LIBRARY - The SDL_gles library

# Copyright (c) 2011, Pali Rohár <pali.rohar@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(SDLGLES_INCLUDE_DIR AND SDLGLES_LIBRARY)
	set(SDLGLES_FOUND true)
else()
	find_path(SDLGLES_INCLUDE_DIR SDL_gles.h PATH_SUFFIXES include/SDL)
	find_library(SDLGLES_LIBRARY NAMES SDL_gles)

	if(SDLGLES_INCLUDE_DIR AND SDLGLES_LIBRARY)
		set(SDLGLES_FOUND true)
		message(STATUS "Found SDL_gles: ${SDLGLES_LIBRARY}")
	else()
		set(SDLGLES_FOUND false)
		message(STATUS "Could not find SDL_gles")
	endif()

	mark_as_advanced(SDLGLES_INCLUDE_DIR SDLGLES_LIBRARY)
endif()
