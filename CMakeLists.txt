project(stratagus)
cmake_minimum_required(VERSION 2.6)

# Stratagus sources

include_directories(
	src/include
	src/guichan/include
	src/guichan/include/guichan
)

set(action_SRCS
	src/action/action_attack.cpp
	src/action/action_board.cpp
	src/action/action_build.cpp
	src/action/action_die.cpp
	src/action/action_follow.cpp
	src/action/action_move.cpp
	src/action/action_patrol.cpp
	src/action/action_repair.cpp
	src/action/action_research.cpp
	src/action/action_resource.cpp
	src/action/action_returngoods.cpp
	src/action/actions.cpp
	src/action/action_spellcast.cpp
	src/action/action_stand.cpp
	src/action/action_still.cpp
	src/action/action_train.cpp
	src/action/action_unload.cpp
	src/action/action_upgradeto.cpp
	src/action/command.cpp
)

set(ai_SRCS
	src/ai/ai_building.cpp
	src/ai/ai.cpp
	src/ai/ai_force.cpp
	src/ai/ai_magic.cpp
	src/ai/ai_plan.cpp
	src/ai/ai_resource.cpp
	src/ai/script_ai.cpp
)

set(beos_SRCS
	src/beos/beos.cpp
)

set(editor_SRCS
	src/editor/editloop.cpp
	src/editor/editor.cpp
	src/editor/edmap.cpp
	src/editor/script_editor.cpp
)

set(game_SRCS
	src/game/game.cpp
	src/game/loadgame.cpp
	src/game/replay.cpp
	src/game/savegame.cpp
	src/game/trigger.cpp
)

set(guichan_SRCS
	src/guichan/cliprectangle.cpp
	src/guichan/color.cpp
	src/guichan/defaultfont.cpp
	src/guichan/exception.cpp
	src/guichan/focushandler.cpp
	src/guichan/gfont.cpp
	src/guichan/graphics.cpp
	src/guichan/gui.cpp
	src/guichan/guichan.cpp
	src/guichan/image.cpp
	src/guichan/imagefont.cpp
	src/guichan/key.cpp
	src/guichan/keyinput.cpp
	src/guichan/mouseinput.cpp
#	src/guichan/opengl/opengl.cpp
#	src/guichan/opengl/openglgraphics.cpp
#	src/guichan/opengl/openglimageloader.cpp
	src/guichan/rectangle.cpp
	src/guichan/sdl/gsdl.cpp
	src/guichan/sdl/sdlgraphics.cpp
	src/guichan/sdl/sdlimageloader.cpp
	src/guichan/sdl/sdlinput.cpp
	src/guichan/widget.cpp
	src/guichan/widgets/button.cpp
	src/guichan/widgets/container.cpp
	src/guichan/widgets/dropdown.cpp
	src/guichan/widgets/checkbox.cpp
	src/guichan/widgets/icon.cpp
	src/guichan/widgets/label.cpp
	src/guichan/widgets/listbox.cpp
	src/guichan/widgets/radiobutton.cpp
	src/guichan/widgets/scrollarea.cpp
	src/guichan/widgets/slider.cpp
	src/guichan/widgets/textbox.cpp
	src/guichan/widgets/textfield.cpp
	src/guichan/widgets/window.cpp
)

set(map_SRCS
	src/map/map.cpp
	src/map/map_draw.cpp
	src/map/map_fog.cpp
	src/map/map_radar.cpp
	src/map/map_save.cpp
	src/map/map_wall.cpp
	src/map/minimap.cpp
	src/map/script_map.cpp
	src/map/script_tileset.cpp
	src/map/tileset.cpp
)

set(network_SRCS
	src/network/commands.cpp
	src/network/lowlevel.cpp
	src/network/master.cpp
	src/network/netconnect.cpp
	src/network/network.cpp
)

set(particle_SRCS
	src/particle/graphicanimation.cpp
	src/particle/chunkparticle.cpp
	src/particle/particlemanager.cpp
	src/particle/smokeparticle.cpp
	src/particle/staticparticle.cpp
)

set(pathfinder_SRCS
	src/pathfinder/astar.cpp
	src/pathfinder/pathfinder.cpp
	src/pathfinder/script_pathfinder.cpp
)

set(sound_SRCS
	src/sound/mikmod.cpp
	src/sound/music.cpp
	src/sound/ogg.cpp
	src/sound/script_sound.cpp
	src/sound/sound.cpp
	src/sound/sound_id.cpp
	src/sound/sound_server.cpp
	src/sound/unitsound.cpp
	src/sound/wav.cpp
)

set(stratagusmain_SRCS
	src/stratagus/construct.cpp
	src/stratagus/groups.cpp
	src/stratagus/iolib.cpp
	src/stratagus/luacallback.cpp
	src/stratagus/mainloop.cpp
	src/stratagus/missile.cpp
	src/stratagus/player.cpp
	src/stratagus/script.cpp
	src/stratagus/script_missile.cpp
	src/stratagus/script_player.cpp
	src/stratagus/script_spell.cpp
	src/stratagus/selection.cpp
	src/stratagus/spells.cpp
	src/stratagus/stratagus.cpp
	src/stratagus/title.cpp
	src/stratagus/translate.cpp
	src/stratagus/util.cpp
)

set(tolua_SRCS
	src/tolua/tolua_event.cpp
	src/tolua/tolua_is.cpp
	src/tolua/tolua_map.cpp
	src/tolua/tolua_push.cpp
	src/tolua/tolua_to.cpp
)

set(ui_SRCS
	src/ui/botpanel.cpp
	src/ui/button_checks.cpp
	src/ui/icons.cpp
	src/ui/interface.cpp
	src/ui/mainscr.cpp
	src/ui/menu_proc.cpp
	src/ui/menus.cpp
	src/ui/mouse.cpp
	src/ui/script_ui.cpp
	src/ui/ui.cpp
	src/ui/widgets.cpp
)

set(unit_SRCS
	src/unit/build.cpp
	src/unit/depend.cpp
	src/unit/script_unit.cpp
	src/unit/script_unittype.cpp
	src/unit/unit_cache.cpp
	src/unit/unit.cpp
	src/unit/unit_draw.cpp
	src/unit/unit_find.cpp
	src/unit/unit_manager.cpp
	src/unit/unit_save.cpp
	src/unit/unittype.cpp
	src/unit/upgrade.cpp
)

set(video_SRCS
	src/video/cursor.cpp
	src/video/font.cpp
	src/video/graphic.cpp
	src/video/linedraw.cpp
	src/video/mng.cpp
	src/video/movie.cpp
	src/video/png.cpp
	src/video/sdl.cpp
	src/video/sprite.cpp
	src/video/video.cpp
)

set(win32_SRCS
	src/win32/attachconsole.cpp
	src/win32/stratagus.rc
)

set(tolua_cpp_DEPS
	src/tolua/ai.pkg
	src/tolua/editor.pkg
	src/tolua/font.pkg
	src/tolua/game.pkg
	src/tolua/map.pkg
	src/tolua/minimap.pkg
	src/tolua/network.pkg
	src/tolua/particle.pkg
	src/tolua/pathfinder.pkg
	src/tolua/player.pkg
	src/tolua/sound.pkg
	src/tolua/stratagus.pkg
	src/tolua/trigger.pkg
	src/tolua/ui.pkg
	src/tolua/unit.pkg
	src/tolua/unittype.pkg
	src/tolua/upgrade.pkg
	src/tolua/video.pkg
)

set(stratagus_SRCS
	${action_SRCS}
	${ai_SRCS}
	${editor_SRCS}
	${game_SRCS}
	${guichan_SRCS}
	${map_SRCS}
	${network_SRCS}
	${particle_SRCS}
	${pathfinder_SRCS}
	${sound_SRCS}
	${stratagusmain_SRCS}
	${tolua_SRCS}
	${ui_SRCS}
	${unit_SRCS}
	${video_SRCS}
	${CMAKE_CURRENT_BINARY_DIR}/tolua.cpp
)

# Additional platform checks

if(UNIX AND CMAKE_SYSTEM_NAME MATCHES BSD)
	set(BSD true)
endif()

if(UNIX AND CMAKE_SYSTEM_NAME MATCHES Linux AND EXISTS /etc/maemo_version)
	set(MAEMO true)
endif()

# Find all libraries

option(ENABLE_STATIC "Compile Stratagus as static executable" OFF)

if(ENABLE_STATIC)
	set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib;.a")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

find_package(Lua51 REQUIRED)
find_package(PNG REQUIRED)
find_package(SDL REQUIRED)
find_package(Tolua++ REQUIRED)
find_package(ZLIB REQUIRED)

if(MAEMO)
	find_package(LibOSSO REQUIRED)
endif()

find_package(BZip2)
find_package(Mikmod)
find_package(MNG)
find_package(OggVorbis)
find_package(Theora)

find_package(X11)
find_package(OpenGL)
find_package(OpenGLES)
find_package(SDL_gles)

find_package(Sqlite)

include(CheckFunctionExists)

if(WIN32)
	enable_language(RC)
	include(CMakeDetermineRCCompiler)

	if(MINGW)
		set(CMAKE_RC_COMPILER_INIT windres)
		set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
	endif()
endif()

# Options for compiling

if(WIN32)
	option(ENABLE_STDIO_REDIRECT "Redirect Stratagus console output to files on Windows" OFF)
endif()

#option(ENABLE_TOUCHSCREEN "Use touchscreen input" ON)

option(WITH_BZIP2 "Compile Stratagus with BZip2 compression support" ON)
option(WITH_MIKMOD "Compile Stratagus with Mikmod sound library" ON)
option(WITH_MNG "Compile Stratagus with MNG image library" ON)
option(WITH_OGGVORBIS "Compile Stratagus with OGG/Vorbis sound library" ON)
option(WITH_THEORA "Compile Stratagus with Theroa video library" ON)

option(WITH_X "Compile Stratagus with X Window System clipboard pasting support" ON)
#option(WITH_OPENGL "Use OpenGL for rendering" ON)
option(WITH_OPENGLES "Compile Stratagus with OpenGL ES 1.1 rendering support instead OpenGL" OFF)

# Stratagus definitions

add_definitions(${PNG_DEFINITIONS} -DUSE_ZLIB)
include_directories(${LUA_INCLUDE_DIR} ${PNG_INCLUDE_DIR} ${SDL_INCLUDE_DIR} ${TOLUA++_INCLUDE_DIR} ${ZLIB_INCLUDE_DIRS})
set(stratagus_LIBS ${stratagus_LIBS} ${LUA_LIBRARIES} ${PNG_LIBRARIES} ${SDL_LIBRARY} ${ZLIB_LIBRARIES})

if(WIN32 AND NOT ENABLE_STDIO_REDIRECT)
	add_definitions(-DNO_STDIO_REDIRECT)
endif()

# OpenGL ES 1.1 Maemo hack
if(MAEMO)
	set(WITH_OPENGLES ON)
endif()

# TODO: Add support for touchscreen input (not only for MAEMO)
#if(ENABLE_TOUCHSCREEN)
#
#endif()

if(WITH_BZIP2 AND BZIP2_FOUND)
	add_definitions(-DUSE_BZ2LIB ${BZIP2_DEFINITIONS})
	include_directories(${BZIP2_INCLUDE_DIR})
	set(stratagus_LIBS ${stratagus_LIBS} ${BZIP2_LIBRARIES})
endif()

if(WITH_MIKMOD AND MIKMOD_FOUND)
	add_definitions(-DUSE_MIKMOD ${MIKMOD_DEFINITIONS})
	include_directories(${MIKMOD_INCLUDE_DIR})
	set(stratagus_LIBS ${stratagus_LIBS} ${MIKMOD_LIBRARY})
endif()

if(WITH_MNG AND MNG_FOUND)
	add_definitions(-DUSE_MNG)
	include_directories(${MNG_INCLUDE_DIR})
	set(stratagus_LIBS ${stratagus_LIBS} ${MNG_LIBRARY})
endif()

if(WITH_OGGVORBIS AND OGGVORBIS_FOUND)
	add_definitions(-DUSE_VORBIS)
	include_directories(${OGGVORBIS_INCLUDE_DIR})
	set(stratagus_LIBS ${stratagus_LIBS} ${OGG_LIBRARY} ${VORBIS_LIBRARY})
endif()

if(WITH_THEORA AND THEORA_FOUND)
	add_definitions(-DUSE_THEORA)
	include_directories(${THEORA_INCLUDE_DIR})
	set(stratagus_LIBS ${stratagus_LIBS} ${THEORA_LIBRARY})
endif()

if(WITH_X AND X11_FOUND)
	add_definitions(-DHAVE_X11)
	include_directories(${X11_INCLUDE_DIR})
	set(stratagus_LIBS ${stratagus_LIBS} ${X11_X11_LIB})
endif()

# TODO: Add OpenGL support optional
#if(WITH_OPENGL AND OPENGL_FOUND)
if(NOT WITH_OPENGLES)
#	add_definitions(-DUSE_OPENGL)
	include_directories(${OPENGL_INCLUDE_DIR})
	set(stratagus_LIBS ${stratagus_LIBS} ${OPENGL_LIBRARIES})
endif()

# TODO: Add OpenGL ES 1.1 support together with OpengL
if(WITH_OPENGLES AND OPENGLES_FOUND AND SDLGLES_FOUND)
	add_definitions(-DUSE_GLES)
	include_directories(${OPENGLES_INCLUDE_DIR} ${SDLGLES_INCLUDE_DIR})
	set(stratagus_LIBS ${stratagus_LIBS} ${OPENGLES_LIBRARIES} ${SDLGLES_LIBRARY})
endif()

# Platform definitions

if(WIN32)
	add_definitions(-DUSE_WIN32)
	set(stratagus_SRCS ${stratagus_SRCS} ${win32_SRCS})
	set(stratagus_LIBS ${stratagus_LIBS} dsound dxguid winmm ws2_32)
endif()

if(APPLE)
	add_definitions(-DUSE_MAC)
endif()

if(BSD)
	add_definitions(-DBSD)
endif()

if(BEOS)
	add_definitions(-DUSE_BEOS)
	set(stratagus_SRCS ${stratagus_SRCS} ${beos_SRCS})
endif()

if(MAEMO)
	add_definitions(-DUSE_MAEMO)
endif()

check_function_exists("strcasestr" HAVE_STRCASESTR)
check_function_exists("strnlen" HAVE_STRNLEN)

if(HAVE_STRCASESTR)
	add_definitions(-DHAVE_STRCASESTR)
endif()

if(HAVE_STRNLEN)
	add_definitions(-DHAVE_STRNLEN)
endif()

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

# Compile Stratagus

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tolua.cpp COMMAND ${TOLUA++_APP} -L stratagus.lua -o ${CMAKE_CURRENT_BINARY_DIR}/tolua.cpp stratagus.pkg DEPENDS ${tolua_cpp_DEPS} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/tolua)
add_executable(stratagus WIN32 ${stratagus_SRCS})
target_link_libraries(stratagus ${stratagus_LIBS})

if(ENABLE_STATIC AND WIN32 AND MINGW)
	set_target_properties(stratagus PROPERTIES LINK_FLAGS "${LINK_FLAGS} -static-libgcc")
endif()

########### next target ###############

set(metaserver_SRCS
	src/metaserver/cmd.cpp
	src/metaserver/db.cpp
	src/metaserver/games.cpp
	src/metaserver/main.cpp
	src/metaserver/netdriver.cpp
	src/network/lowlevel.cpp
)

if(SQLITE_FOUND)
	add_executable(metaserver ${metaserver_SRCS})
	target_link_libraries(metaserver ${SDL_LIBRARY} ${SQLITE_LIBRARIES})

	if(ENABLE_STATIC AND WIN32 AND MINGW)
		set_target_properties(metaserver PROPERTIES LINK_FLAGS "${LINK_FLAGS} -static-libgcc")
	endif()
endif()

########### next target ###############

set(png2stratagus_SRCS
	tools/png2stratagus.cpp
)

add_executable(png2stratagus ${png2stratagus_SRCS})
target_link_libraries(png2stratagus ${PNG_LIBRARY} ${ZLIB_LIBRARIES})

if(ENABLE_STATIC AND WIN32 AND MINGW)
	set_target_properties(png2stratagus PROPERTIES LINK_FLAGS "${LINK_FLAGS} -static-libgcc")
endif()

########### install files ###############

install(TARGETS stratagus DESTINATION games)
