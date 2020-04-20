project(hqx)

cmake_minimum_required(VERSION 2.8)

include_directories(
	"include")

file(GLOB HQX_SOURCES
	"source/HQx.cc"
	"source/HQ2x.cc"
	"source/HQ3x.cc")

add_library(hqx ${HQX_SOURCES})
set_target_properties(hqx PROPERTIES
    OUTPUT_NAME "hqx"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    PREFIX "lib" )

add_executable(test "source/main.cc")
target_link_libraries(test hqx)
set_target_properties(test PROPERTIES
    OUTPUT_NAME "test"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}" )
