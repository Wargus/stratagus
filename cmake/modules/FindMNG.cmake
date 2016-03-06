# - Try to find the MNG library
# Once done this will define
#
#  MNG_FOUND - system has Mikmod
#  MNG_INCLUDE_DIR - the Mikmod include directory
#  MNG_LIBRARY - The Mikmod library

# Copyright (c) 2011, Pali Rohár <pali.rohar@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(MNG_INCLUDE_DIR AND MNG_LIBRARY AND JPEG_INCLUDE_DIR AND JPEG_LIBRARY)
	set(MNG_FOUND true)
else()
	find_path(MNG_INCLUDE_DIR libmng.h)
	find_path(JPEG_INCLUDE_DIR jpeglib.h)
	find_library(MNG_LIBRARY NAMES mng)
	find_library(MNG_LIBRARY NAMES jpeg)

	if(MNG_INCLUDE_DIR AND MNG_LIBRARY AND JPEG_INCLUDE_DIR AND JPEG_LIBRARY)
		set(MNG_FOUND true)
		message(STATUS "Found MNG: ${MNG_LIBRARY} and JPEG: ${JPEG_LIBRARY}")
	else()
		set(MNG_FOUND false)
		message(STATUS "Could not find MNG and JPEG")
	endif()

	mark_as_advanced(MNG_INCLUDE_DIR MNG_LIBRARY JPEG_INCLUDE_DIR JPEG_LIBRARY)
endif()
