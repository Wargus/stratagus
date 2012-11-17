# - Try to find the tolua++
# Once done this will define
#
#  TOLUA++_FOUND - system has tolua++
#  TOLUA++_APP - the tolua++ program
#  TOLUA++_INLUDE_DIR - the tolua++ include directory
#  TOLUA++_LIBRARY - the tolua++ library

# Copyright (c) 2011, Pali Rohár <pali.rohar@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(TOLUA++_INCLUDE_DIR AND TOLUA++_LIBRARY AND TOLUA++_APP)
	set(TOLUA++_FOUND true)
else()
	find_path(TOLUA++_INCLUDE_DIR tolua++.h)
	find_library(TOLUA++_LIBRARY NAMES tolua++ tolua++5.1 toluapp)
	find_program(TOLUA++_APP NAMES tolua++ tolua++5.1 toluapp)

	if(TOLUA++_INCLUDE_DIR AND TOLUA++_LIBRARY AND TOLUA++_APP)
		set(TOLUA++_FOUND true)
		message(STATUS "Found program tolua++: ${TOLUA++_APP}")
		message(STATUS "Found library tolua++: ${TOLUA++_LIBRARY}")
	else()
		set(TOLUA++_FOUND false)
		message(FATAL_ERROR "Could not find library or program tolua++")
	endif()

	mark_as_advanced(TOLUA++_INCLUDE_DIR AND TOLUA++_LIBRARY AND TOLUA++_APP)
endif()
