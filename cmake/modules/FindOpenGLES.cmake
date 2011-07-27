#-------------------------------------------------------------------
# This file is part of the CMake build system for OGRE
#     (Object-oriented Graphics Rendering Engine)
# For the latest info, see http://www.ogre3d.org/
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# Copyright (c) 2011, Pali Rohár <pali.rohar@gmail.com>
#  Modified:
#   * Search for EGL library
#   * Show staus message
#   * Cleaned code

# - Try to find OpenGLES
# Once done this will define
#  
#  OPENGLES_FOUND        - system has OpenGLES
#  OPENGLES_INCLUDE_DIR  - the GL include directory
#  OPENGLES_LIBRARIES    - Link these to use OpenGLES

if(OPENGLES_INCLUDE_DIR AND OPENGLES_LIBRARIES)
	set(OPENGLES_FOUND true)
else()

	if(WIN32)

		if(CYGWIN)
			find_path(OPENGLES_INCLUDE_DIR GLES/gl.h)
			find_library(OPENGLES_GL_LIBRARY libgles_cm)
			find_library(OPENGLES_EGL_LIBRARY EGL)
		elseif(BORLAND)
			set(OPENGLES_GL_LIBRARY import32 CACHE STRING "OpenGL ES 1.x library for win32")
			# TODO: EGL
		else()
			# MS compiler - todo - fix the following line:
			set(OPENGLES_GL_LIBRARY libgles_cm.lib CACHE STRING "OpenGL ES 1.x library for win32")
			# TODO: EGL
		endif()

	elseif(APPLE)

		
		set(OPENGLES_GL_LIBRARY "-framework OpenGLES")
		# TODO: EGL

	elseif(SYMBIAN)

		set(ORIGINAL_CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH})
		set(CMAKE_PREFIX_PATH ${CMAKE_SYSYEM_OUT_DIR})
		find_library(OPENGLES_gl_LIBRARY libgles_cm)
		set(CMAKE_PREFIX_PATH ${ORIGINAL_CMAKE_PREFIX_PATH})
		# TODO: EGL

	else()

		find_path(OPENGLES_INCLUDE_DIR GLES/gl.h
			/usr/openwin/share/include
			/opt/graphics/OpenGL/include
			/usr/X11R6/include
			/usr/include
		)

		find_library(OPENGLES_GL_LIBRARY NAMES GLES_CM PATHS
			/usr/openwin/lib
			/opt/graphics/OpenGL/lib
			/usr/shlib
			/usr/X11R6/lib
			/usr/lib
		)

		find_library(OPENGLES_EGL_LIBRARY NAMES EGL PATHS
			/usr/openwin/lib
			/opt/graphics/OpenGL/lib
			/usr/shlib
			/usr/X11R6/lib
			/usr/lib
		)

		# On Unix OpenGL most certainly always requires X11.
		# Feel free to tighten up these conditions if you don't 
		# think this is always true.

		find_package(X11)

	      	if(X11_FOUND)
			set(OPENGLES_LIBRARIES ${OPENGLES_LIBRARIES} ${X11_LIBRARIES})
		endif()

	endif()

	if(OPENGLES_EGL_LIBRARY AND OPENGLES_GL_LIBRARY)
		set(OPENGLES_LIBRARIES ${OPENGLES_LIBRARIES} ${OPENGLES_EGL_LIBRARY} ${OPENGLES_GL_LIBRARY})
		set(OPENGLES_FOUND true)
		message(STATUS "Found OpenGL ES 1.1 libraries: ${OPENGLES_LIBRARIES}")
	else()
		set(OPENGLES_FOUND false)
		message(STATUS "Could not find OpenGL ES 1.1 libraries")
	endif()

	mark_as_advanced(OPENGLES_INCLUDE_DIR OPENGLES_LIBRARIES)

endif()
