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

if(MNG_INCLUDE_DIR AND MNG_LIBRARY AND ((NOT MSVC) OR (JPEG_INCLUDE_DIR AND JPEG_LIBRARY)))
	set(MNG_FOUND true)
else()
	find_path(MNG_INCLUDE_DIR libmng.h)
	find_library(MNG_LIBRARY NAMES mng)
	if(MSVC)
		find_path(JPEG_INCLUDE_DIR jpeglib.h)
		find_library(JPEG_LIBRARY NAMES jpeg)
	endif()

	if(MNG_INCLUDE_DIR AND MNG_LIBRARY AND ((NOT MSVC) OR (JPEG_INCLUDE_DIR AND JPEG_LIBRARY)))
		set(MNG_FOUND true)
		message(STATUS "Found MNG: ${MNG_LIBRARY}")
		if (MSVC)
			message(STATUS  "Found JPEG: ${JPEG_LIBRARY}")
		endif()
	else()
		set(MNG_FOUND false)
		if(MSVC AND (NOT (JPEG_INCLUDE_DIR AND JPEG_LIBRARY)))
			message(STATUS "Could not find JPEG")
		endif()
		message(STATUS "Could not find MNG")
	endif()

	if(MSVC)
		mark_as_advanced(MNG_INCLUDE_DIR MNG_LIBRARY JPEG_INCLUDE_DIR JPEG_LIBRARY)
	else()
		mark_as_advanced(MNG_INCLUDE_DIR MNG_LIBRARY)
	endif()
endif()
