# - Try to find the StackTrace library
# Once done this will define
#
#  STACKTRACE_FOUND - system has StackTrace
#  STACKTRACE_PROJECT_DIR - the StackTrace project directory
#  STACKTRACE_LIBRARY - The StackTrace library

# Copyright (c) 2015, cybermind <cybermindid@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(STACKTRACE_PROJECT_DIR AND STACKTRACE_LIBRARY)
	set(STACKTRACE_FOUND true)
else()
	find_path(STACKTRACE_PROJECT_DIR stacktrace/call_stack.hpp stacktrace/stack_exception.hpp)
	find_library(STACKTRACE_LIBRARY NAMES StackTrace)

	if(STACKTRACE_PROJECT_DIR AND STACKTRACE_LIBRARY)
		set(STACKTRACE_FOUND true)
		message(STATUS "Found StackTrace: ${STACKTRACE_LIBRARY}")
	else()
		set(STACKTRACE_FOUND false)
		message(STATUS "Could not find StackTrace")
	endif()

	mark_as_advanced(STACKTRACE_PROJECT_DIR STACKTRACE_LIBRARY)
endif()
