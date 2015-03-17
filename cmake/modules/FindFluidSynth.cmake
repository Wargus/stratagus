# - Try to find the FluidSynth library
# Once done this will define
#
#  FLUIDSYNTH_FOUND - system has FluidSynth
#  FLUIDSYNTH_INCLUDE_DIR - the FLUIDSYNTH include directory
#  FLUIDSYNTH_LIBRARY - The FLUIDSYNTH library

# Copyright (c) 2014, cybermind <cybermindid@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(FLUIDSYNTH_INCLUDE_DIR AND FLUIDSYNTH_LIBRARY)
	set(FLUIDSYNTH_FOUND true)
else()
	find_path(FLUIDSYNTH_INCLUDE_DIR fluidsynth.h)
	find_library(FLUIDSYNTH_LIBRARY fluidsynth)

	if(FLUIDSYNTH_INCLUDE_DIR AND FLUIDSYNTH_LIBRARY)
		set(FLUIDSYNTH_FOUND true)
		message(STATUS "Found FluidSynth: ${FLUIDSYNTH_LIBRARY}")
	else()
		set(FLUIDSYNTH_FOUND false)
		message(STATUS "Could not find FluidSynth")
	endif()

	mark_as_advanced(FLUIDSYNTH_INCLUDE_DIR FLUIDSYNTH_LIBRARY)
endif()
