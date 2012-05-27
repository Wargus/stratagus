# - Try to find the MakeNSIS
# Once done this will define
#
#  MAKENSIS_FOUND - system has MakeNSIS
#  MAKENSIS - the MakeNSIS program
#  MAKENSIS_FLAGS - the MakeNSIS flags
#  MAKENSIS_SUFFIX - the MakeNSIS output file suffix

# Copyright (c) 2011, Pali Rohár <pali.rohar@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(MAKENSIS)
	set(MAKENSIS_FOUND true)
else()
	find_program(MAKENSIS NAMES makensis)
	find_package(SelfPackers)

	set(MAKENSIS_ADDITIONAL_FLAGS "" CACHE STRING "Additional flags for makensis")

	if(MAKENSIS)
		set(MAKENSIS_FOUND true)
		message(STATUS "Found MakeNSIS: ${MAKENSIS}")
	else()
		set(MAKENSIS_FOUND false)
#		message(FATAL_ERROR "Could not find program MakeNSIS") # TODO: This fail if REQUIRED is not used too!
		message(STATUS "Could not find program MakeNSIS")
	endif()

	if(NOT CMAKE_VERBOSE_MAKEFILE)
		set(MAKENSIS_FLAGS ${MAKENSIS_FLAGS} -V2 -DQUIET)
	endif()

	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		set(MAKENSIS_FLAGS ${MAKENSIS_FLAGS} -DDBG)
		set(MAKENSIS_SUFFIX ${MAKENSIS_SUFFIX}-debug)
	endif()

	if(CMAKE_SIZEOF_VOID_P STREQUAL 8)
		set(MAKENSIS_FLAGS ${MAKENSIS_FLAGS} -DX86_64)
		set(MAKENSIS_SUFFIX ${MAKENSIS_SUFFIX}-x86_64)
	endif()

	if(ENABLE_UPX AND SELF_PACKER_FOR_EXECUTABLE)
		set(MAKENSIS_FLAGS ${MAKENSIS_FLAGS} -DUPX=${SELF_PACKER_FOR_EXECUTABLE} -DUPX_FLAGS=${SELF_PACKER_FOR_EXECUTABLE_FLAGS})
	endif()

	set(MAKENSIS_FLAGS ${MAKENSIS_ADDITIONAL_FLAGS} ${MAKENSIS_FLAGS} -NOCD CACHE STRING "")
	set(MAKENSIS_SUFFIX ${MAKENSIS_SUFFIX}${CMAKE_EXECUTABLE_SUFFIX} CACHE STRING "")

	mark_as_advanced(MAKENSIS MAKENSIS_FLAGS MAKENSIS_SUFFIX)
endif()
