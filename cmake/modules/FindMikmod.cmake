# - Try to find the Mikmod library
# Once done this will define
#
#  MIKMOD_FOUND - system has Mikmod
#  MIKMOD_INCLUDE_DIR - the Mikmod include directory
#  MIKMOD_LIBRARY - The Mikmod library

# Copyright (c) 2011, Pali Rohár <pali.rohar@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(MIKMOD_INCLUDE_DIR AND MIKMOD_LIBRARY)
	set(MIKMOD_FOUND true)
else()
	find_path(MIKMOD_INCLUDE_DIR mikmod.h)
	find_library(MIKMOD_LIBRARY NAMES mikmod)

	if(MIKMOD_INCLUDE_DIR AND MIKMOD_LIBRARY)
		set(MIKMOD_FOUND true)
		message(STATUS "Found Mikmod: ${MIKMOD_LIBRARY}")
	else()
		set(MIKMOD_FOUND false)
		message(STATUS "Could not find Mikmod")
	endif()

	mark_as_advanced(MIKMOD_INCLUDE_DIR MIKMOD_LIBRARY)
endif()
