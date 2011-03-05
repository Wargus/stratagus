# - Try to find the program tolua++
# Once done this will define
#
#  TOLUA++_FOUND - system has tolua++
#  TOLUA++ - program tolua++

# Copyright (c) 2011, Pali Rohár <pali.rohar@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(TOLUA++)
	set(TOLUA++_FOUND true)
else()
	find_program(TOLUA++ NAMES tolua++ tolua++5.1)
	if(TOLUA++)
		set(TOLUA++_FOUND true)
		message(STATUS "Found program tolua++: ${TOLUA++}")
	else()
		set(TOLUA++_FOUND false)
		message(FATAL_ERROR "Could not find program tolua++")
	endif()
	mark_as_advanced(TOLUA++)
endif()
