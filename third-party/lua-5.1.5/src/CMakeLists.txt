cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

set(LUA_LIB_SRC
 "lapi.c"
 "lcode.c"
 "ldebug.c"
 "ldo.c"
 "ldump.c"
 "lfunc.c"
 "lgc.c"
 "llex.c"
 "lmem.c"
 "lobject.c"
 "lopcodes.c"
 "lparser.c"
 "lstate.c"
 "lstring.c"
 "ltable.c"
 "ltm.c"
 "lundump.c"
 "lvm.c"
 "lzio.c"
 "lauxlib.c"
 "lbaselib.c"
 "ldblib.c"
 "liolib.c"
 "lmathlib.c"
 "loslib.c"
 "ltablib.c"
 "lstrlib.c"
 "loadlib.c"
 "linit.c"
)

set(LUA_SRC "lua.c")

add_library(lua51_static STATIC "${LUA_LIB_SRC}")
set_target_properties(lua51_static PROPERTIES OUTPUT_NAME "lua51")

add_library(lua51_shared SHARED "${LUA_LIB_SRC}")
set_target_properties(lua51_shared PROPERTIES OUTPUT_NAME "lua51")

add_executable(lua51 "${LUA_SRC}" "${LUA_LIB_SRC}")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

if(UNIX)
target_compile_definitions(lua51_static PUBLIC "LUA_USE_DLOPEN" "LUA_USE_MKSTEMP")
target_compile_definitions(lua51_shared PUBLIC "LUA_USE_DLOPEN" "LUA_USE_MKSTEMP")
target_compile_definitions(lua51 PUBLIC "LUA_USE_DLOPEN" "LUA_USE_MKSTEMP")
target_link_libraries(lua51 PUBLIC ${CMAKE_DL_LIBS})
target_link_libraries(lua51_static PUBLIC ${CMAKE_DL_LIBS})
target_link_libraries(lua51_shared PUBLIC ${CMAKE_DL_LIBS})
endif()

set_target_properties(lua51_static PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(lua51_shared PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(lua51 PROPERTIES LINKER_LANGUAGE CXX)

set_target_properties(lua51 PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
    )

add_custom_command(TARGET lua51_static POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_SOURCE_DIR}/src/lua.h" "${PROJECT_SOURCE_DIR}/src/luaconf.h" "${PROJECT_SOURCE_DIR}/src/lualib.h" "${PROJECT_SOURCE_DIR}/src/lauxlib.h" ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
