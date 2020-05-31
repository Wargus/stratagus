# - Locate SDL library

find_path(SDL2_INCLUDE_DIR SDL.h
  HINTS
    ENV SDLDIR
  PATH_SUFFIXES include/SDL2 include
)

find_library(SDL2_LIBRARY_TEMP
  NAMES SDL2
  HINTS
    ENV SDLDIR
  PATH_SUFFIXES lib
)

find_library(SDL2main_LIBRARY
  NAMES SDL2main
  HINTS
    ENV SDLDIR
  PATH_SUFFIXES lib
)

# SDL may require threads on your system.
# The Apple build may not need an explicit flag because one of the
# frameworks may already provide it.
# But for non-OSX systems, I will use the CMake Threads package.
if(NOT APPLE)
  find_package(Threads)
endif()

# MinGW needs an additional library, mwindows
# It's total link flags should look like -lmingw32 -lSDLmain -lSDL -lmwindows
# (Actually on second look, I think it only needs one of the m* libraries.)
if(MINGW)
  set(MINGW32_LIBRARY mingw32 CACHE STRING "mwindows for MinGW")
endif()

if(SDL2_LIBRARY_TEMP)
  # For OS X, SDL uses Cocoa as a backend so it must link to Cocoa.
  # CMake doesn't display the -framework Cocoa string in the UI even
  # though it actually is there if I modify a pre-used variable.
  # I think it has something to do with the CACHE STRING.
  # So I use a temporary variable until the end so I can set the
  # "real" variable in one-shot.
  if(APPLE)
    set(SDL2_LIBRARY_TEMP ${SDL2_LIBRARY_TEMP} "-framework Cocoa")
  endif()

  # For threads, as mentioned Apple doesn't need this.
  # In fact, there seems to be a problem if I used the Threads package
  # and try using this line, so I'm just skipping it entirely for OS X.
  if(NOT APPLE)
    set(SDL2_LIBRARY_TEMP ${SDL2_LIBRARY_TEMP} ${CMAKE_THREAD_LIBS_INIT})
  endif()

  # For MinGW library
  if(MINGW)
    set(SDL2_LIBRARY_TEMP ${MINGW32_LIBRARY} ${SDL2_LIBRARY_TEMP})
  endif()

  # Set the final string here so the GUI reflects the final state.
  set(SDL2_LIBRARY ${SDL2_LIBRARY_TEMP} CACHE STRING "Where the SDL Library can be found")
  # Set the temp variable to INTERNAL so it is not seen in the CMake GUI
  set(SDL2_LIBRARY_TEMP "${SDL2_LIBRARY_TEMP}" CACHE INTERNAL "")
endif()

if(SDL2_INCLUDE_DIR AND EXISTS "${SDL2_INCLUDE_DIR}/SDL_version.h")
  file(STRINGS "${SDL2_INCLUDE_DIR}/SDL_version.h" SDL_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL_MAJOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL2_INCLUDE_DIR}/SDL_version.h" SDL_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL_MINOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL2_INCLUDE_DIR}/SDL_version.h" SDL_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL_PATCHLEVEL[ \t]+[0-9]+$")
  string(REGEX REPLACE "^#define[ \t]+SDL_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL_VERSION_MAJOR "${SDL_VERSION_MAJOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL_VERSION_MINOR "${SDL_VERSION_MINOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDL_VERSION_PATCH "${SDL_VERSION_PATCH_LINE}")
  set(SDL_VERSION_STRING ${SDL_VERSION_MAJOR}.${SDL_VERSION_MINOR}.${SDL_VERSION_PATCH})
  unset(SDL_VERSION_MAJOR_LINE)
  unset(SDL_VERSION_MINOR_LINE)
  unset(SDL_VERSION_PATCH_LINE)
  unset(SDL_VERSION_MAJOR)
  unset(SDL_VERSION_MINOR)
  unset(SDL_VERSION_PATCH)
  set(SDL2_FOUND TRUE)
endif()

if (SDL2_FOUND)
  if (NOT SDL2_FIND_QUIETLY)
     MESSAGE( STATUS "sdl2 found: includes in ${SDL2_INCLUDE_DIR}, library in ${SDL2_LIBRARY}")
  endif (NOT SDL2_FIND_QUIETLY)
else (THEORA_FOUND)
  if (SDL2_FIND_REQUIRED)
     MESSAGE( FATAL_ERROR "sdl2 not found")
  endif (SDL2_FIND_REQUIRED)
endif (SDL2_FOUND)

MARK_AS_ADVANCED(SDL2_INCLUDE_DIR SDL2_LIBRARY SDL2main_LIBRARY)
