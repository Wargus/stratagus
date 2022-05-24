cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

set(TOLUA_BIN_SRC
tolua.c
toluabind.c
)

set(TOLUA_LIB_SRC
tolua_event.c
tolua_is.c
tolua_map.c
tolua_push.c
tolua_to.c
)


add_library(toluapp51 STATIC "${TOLUA_LIB_SRC}")
add_library(toluapp51_shared SHARED "${TOLUA_LIB_SRC}")
set_target_properties(toluapp51_shared PROPERTIES OUTPUT_NAME "toluapp51")
add_executable(toluapp "${TOLUA_BIN_SRC}" "${TOLUA_LIB_SRC}")

if(MSVC)
    target_link_libraries(toluapp51 PUBLIC "${PROJECT_BINARY_DIR}/$<CONFIGURATION>/lua51.lib" ${CMAKE_DL_LIBS})
    target_link_libraries(toluapp51_shared PUBLIC "${PROJECT_BINARY_DIR}/$<CONFIGURATION>/lua51.lib" ${CMAKE_DL_LIBS})
    target_link_libraries(toluapp PUBLIC "${PROJECT_BINARY_DIR}/$<CONFIGURATION>/lua51.lib" ${CMAKE_DL_LIBS})
else()
    target_link_libraries(toluapp51 PUBLIC "${PROJECT_BINARY_DIR}/liblua51.a" ${CMAKE_DL_LIBS})
    target_link_libraries(toluapp51_shared PUBLIC "${PROJECT_BINARY_DIR}/liblua51.a" ${CMAKE_DL_LIBS})
    target_link_libraries(toluapp PUBLIC "${PROJECT_BINARY_DIR}/liblua51.a" ${CMAKE_DL_LIBS})
endif(MSVC)

add_custom_command(TARGET toluapp51 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_SOURCE_DIR}/toluapp-simple/tolua++.h" ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

set_target_properties(toluapp PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
    )

set_target_properties(toluapp51 PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(toluapp51_shared PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(toluapp PROPERTIES LINKER_LANGUAGE CXX)

include_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
