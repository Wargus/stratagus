#       _________ __                 __
#      /   _____//  |_____________ _/  |______     ____  __ __  ______
#      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
#      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
#     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
#             \/                  \/          \//_____/            \/
#  ______________________                           ______________________
#                        T H E   W A R   B E G I N S
#         Stratagus - A free fantasy real time strategy game engine
#
#    CMakeLists.txt
#    Copyright (C) 2011  Pali Rohár <pali.rohar@gmail.com>
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#

project(stratagus)
cmake_minimum_required(VERSION 2.6)
set(STRATAGUS_VERSION 2.2.6)

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
source_group(action FILES ${action_SRCS})

set(ai_SRCS
	src/ai/ai_building.cpp
	src/ai/ai.cpp
	src/ai/ai_force.cpp
	src/ai/ai_magic.cpp
	src/ai/ai_plan.cpp
	src/ai/ai_resource.cpp
	src/ai/script_ai.cpp
)
source_group(ai FILES ${ai_SRCS})

set(beos_SRCS
	src/beos/beos.cpp
)
source_group(beos FILES ${beos_SRCS})

set(editor_SRCS
	src/editor/editloop.cpp
	src/editor/editor.cpp
	src/editor/edmap.cpp
	src/editor/script_editor.cpp
)
source_group(editor FILES ${editor_SRCS})

set(game_SRCS
	src/game/game.cpp
	src/game/loadgame.cpp
	src/game/replay.cpp
	src/game/savegame.cpp
	src/game/trigger.cpp
)
source_group(game FILES ${game_SRCS})

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
source_group(guichan FILES ${guichan_SRCS})

set(maemo_SRCS
	src/maemo/maemo.cpp
)
source_group(maemo FILES ${maemo_SRCS})

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
source_group(map FILES ${map_SRCS})

set(network_SRCS
	src/network/commands.cpp
	src/network/lowlevel.cpp
	src/network/master.cpp
	src/network/netconnect.cpp
	src/network/network.cpp
)
source_group(network FILES ${network_SRCS})

set(particle_SRCS
	src/particle/graphicanimation.cpp
	src/particle/chunkparticle.cpp
	src/particle/particlemanager.cpp
	src/particle/smokeparticle.cpp
	src/particle/staticparticle.cpp
)
source_group(particle FILES ${particle_SRCS})

set(pathfinder_SRCS
	src/pathfinder/astar.cpp
	src/pathfinder/pathfinder.cpp
	src/pathfinder/script_pathfinder.cpp
)
source_group(pathfinder FILES ${pathfinder_SRCS})

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
source_group(sound FILES ${sound_SRCS})

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
source_group(stratagusmain FILES ${stratagus_SRCS})

set(ui_SRCS
	src/ui/botpanel.cpp
	src/ui/button_checks.cpp
	src/ui/icons.cpp
	src/ui/interface.cpp
	src/ui/mainscr.cpp
	src/ui/menu_proc.cpp
	src/ui/mouse.cpp
	src/ui/script_ui.cpp
	src/ui/ui.cpp
	src/ui/widgets.cpp
)
source_group(ui FILES ${ui_SRCS})

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
source_group(unit FILES ${unit_SRCS})

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
source_group(video FILES ${video_SRCS})

set(win32_SRCS
	src/win32/attachconsole.cpp
	src/win32/stratagus.rc
)
source_group(win32 FILES ${win32_SRCS})

set(tolua_FILES
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
	src/tolua/stratagus.lua
)
source_group(tolua FILES ${tolua_FILES})

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
	${ui_SRCS}
	${unit_SRCS}
	${video_SRCS}
	${CMAKE_CURRENT_BINARY_DIR}/tolua.cpp
)

set(stratagus_HDRS
	src/ai/ai_local.h
	src/video/intern_video.h
	src/video/renderer.h
	src/include/actions.h
	src/include/ai.h
	src/include/animation.h
	src/include/commands.h
	src/include/construct.h
	src/include/cursor.h
	src/include/depend.h
	src/include/editor.h
	src/include/font.h
	src/include/icons.h
	src/include/interface.h
	src/include/iocompat.h
	src/include/iolib.h
	src/include/luacallback.h
	src/include/maemo.h
	src/include/map.h
	src/include/master.h
	src/include/menus.h
	src/include/minimap.h
	src/include/missile.h
	src/include/movie.h
	src/include/myendian.h
	src/include/netconnect.h
	src/include/net_lowlevel.h
	src/include/network.h
	src/include/particle.h
	src/include/pathfinder.h
	src/include/player.h
	src/include/replay.h
	src/include/results.h
	src/include/script.h
	src/include/script_sound.h
	src/include/settings.h
	src/include/sound.h
	src/include/sound_server.h
	src/include/spells.h
	src/include/stratagus.h
	src/include/tile.h
	src/include/tileset.h
	src/include/title.h
	src/include/translate.h
	src/include/trigger.h
	src/include/ui.h
	src/include/unit_cache.h
	src/include/unit.h
	src/include/unit_manager.h
	src/include/unitsound.h
	src/include/unittype.h
	src/include/upgrade.h
	src/include/upgrade_structs.h
	src/include/util.h
	src/include/vec2i.h
	src/include/version.h
	src/include/video.h
	src/include/wav.h
	src/include/widgets.h
	src/guichan/include/guichan/actionlistener.h
	src/guichan/include/guichan/allegro.h
	src/guichan/include/guichan/basiccontainer.h
	src/guichan/include/guichan/cliprectangle.h
	src/guichan/include/guichan/color.h
	src/guichan/include/guichan/defaultfont.h
	src/guichan/include/guichan/exception.h
	src/guichan/include/guichan/focushandler.h
	src/guichan/include/guichan/font.h
	src/guichan/include/guichan/graphics.h
	src/guichan/include/guichan/gsdl.h
	src/guichan/include/guichan/gui.h
	src/guichan/include/guichan.h
	src/guichan/include/guichan/imagefont.h
	src/guichan/include/guichan/image.h
	src/guichan/include/guichan/imageloader.h
	src/guichan/include/guichan/input.h
	src/guichan/include/guichan/key.h
	src/guichan/include/guichan/keyinput.h
	src/guichan/include/guichan/keylistener.h
	src/guichan/include/guichan/listmodel.h
	src/guichan/include/guichan/mouseinput.h
	src/guichan/include/guichan/mouselistener.h
	src/guichan/include/guichan/platform.h
	src/guichan/include/guichan/rectangle.h
	src/guichan/include/guichan/sdl/sdlgraphics.h
	src/guichan/include/guichan/sdl/sdlimageloader.h
	src/guichan/include/guichan/sdl/sdlinput.h
	src/guichan/include/guichan/sdl/sdlpixel.h
	src/guichan/include/guichan/widget.h
	src/guichan/include/guichan/widgets/button.h
	src/guichan/include/guichan/widgets/container.h
	src/guichan/include/guichan/widgets/dropdown.h
	src/guichan/include/guichan/widgets/checkbox.h
	src/guichan/include/guichan/widgets/icon.h
	src/guichan/include/guichan/widgets/label.h
	src/guichan/include/guichan/widgets/listbox.h
	src/guichan/include/guichan/widgets/radiobutton.h
	src/guichan/include/guichan/widgets/scrollarea.h
	src/guichan/include/guichan/widgets/slider.h
	src/guichan/include/guichan/widgets/textbox.h
	src/guichan/include/guichan/widgets/textfield.h
	src/guichan/include/guichan/widgets/window.h
	src/guichan/include/guichan/x.h
)
source_group(include FILES ${stratagus_HDRS})

# Additional platform checks

if(UNIX AND CMAKE_SYSTEM_NAME MATCHES BSD)
	set(BSD true)
endif()

find_package(PkgConfig QUIET REQUIRED)

# Check if platform is Maemo
if(UNIX AND CMAKE_SYSTEM_NAME MATCHES Linux)
	pkg_check_modules(MAEMO_VERSION maemo-version)
	if(MAEMO_VERSION_FOUND)
		set(MAEMO true)
	endif()
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
	pkg_check_modules(LIBOSSO REQUIRED libosso)
endif()

if(WIN32)
	find_package(MakeNSIS)
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
find_package(Doxygen)
find_package(SelfPackers)

include(CheckFunctionExists)

# Windows RC compiler definitions

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
	option(ENABLE_NSIS "Create Stratagus Window NSIS Installer" OFF)
endif()

option(ENABLE_TOUCHSCREEN "Use touchscreen input" OFF)

option(WITH_BZIP2 "Compile Stratagus with BZip2 compression support" ON)
option(WITH_MIKMOD "Compile Stratagus with Mikmod sound library" ON)
option(WITH_MNG "Compile Stratagus with MNG image library" ON)
option(WITH_OGGVORBIS "Compile Stratagus with OGG/Vorbis sound library" ON)
option(WITH_THEORA "Compile Stratagus with Theroa video library" ON)

option(WITH_X11 "Compile Stratagus with X11 clipboard pasting support" ON)

option(ENABLE_DOC "Generate Stratagus source code documentation with Doxygen" OFF)
option(ENABLE_DEV "Install Stratagus game development headers files" OFF)
option(ENABLE_UPX "Compress Stratagus executable binary with UPX packer" OFF)
option(ENABLE_STRIP "Strip all symbols from executables" OFF)

if(NOT WITH_RENDERER)
	if(OPENGL_FOUND)
		set(WITH_RENDERER "OpenGL")
	elseif(OPENGLES_FOUND AND SDLGLES_FOUND)
		set(WITH_RENDERER "OpenGLES")
	else()
		set(WITH_RENDERER "NativeSDL")
	endif()
endif()

# Stratagus definitions

add_definitions(${PNG_DEFINITIONS} -DUSE_ZLIB)
include_directories(${LUA_INCLUDE_DIR} ${PNG_INCLUDE_DIR} ${SDL_INCLUDE_DIR} ${TOLUA++_INCLUDE_DIR} ${ZLIB_INCLUDE_DIRS})
set(stratagus_LIBS ${stratagus_LIBS} ${LUA_LIBRARIES} ${PNG_LIBRARIES} ${SDL_LIBRARY} ${TOLUA++_LIBRARY} ${ZLIB_LIBRARIES})

if(WIN32 AND NOT ENABLE_STDIO_REDIRECT)
	add_definitions(-DNO_STDIO_REDIRECT)
endif()

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

if(WITH_X11 AND X11_FOUND)
	add_definitions(-DHAVE_X11)
	include_directories(${X11_INCLUDE_DIR})
	set(stratagus_LIBS ${stratagus_LIBS} ${X11_X11_LIB})
endif()

if(WITH_RENDERER STREQUAL "OpenGL" AND OPENGL_FOUND)
	add_definitions(-DUSE_OPENGL)
	include_directories(${OPENGL_INCLUDE_DIR})
	set(stratagus_LIBS ${stratagus_LIBS} ${OPENGL_LIBRARIES})
elseif(WITH_RENDERER STREQUAL "OpenGLES" AND OPENGLES_FOUND AND SDLGLES_FOUND)
	add_definitions(-DUSE_GLES)
	include_directories(${OPENGLES_INCLUDE_DIR} ${SDLGLES_INCLUDE_DIR})
	set(stratagus_LIBS ${stratagus_LIBS} ${OPENGLES_LIBRARIES} ${SDLGLES_LIBRARY})
elseif(WITH_RENDERER STREQUAL "NativeSDL")
	message(FATAL_ERROR "Compile Stratagus without OpenGL or OpenGL ES 1.1 rendering is not supported yet")
else()
	message(FATAL_ERROR "Specified rendering support was not found in system. Choose: NativeSDL OpenGL OpenGLES")
endif()

# Platform definitions

if(WIN32)
	add_definitions(-DUSE_WIN32)
	set(stratagus_SRCS ${stratagus_SRCS} ${win32_SRCS})
	set(stratagus_LIBS ${stratagus_LIBS} dsound dxguid winmm ws2_32)
endif()

if (WIN32 AND MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE=1)
endif()

if(APPLE)
	add_definitions(-DUSE_MAC)
	add_definitions(-D__unix)
endif()

if(BSD)
	add_definitions(-DUSE_BSD)
	add_definitions(-D__unix)
endif()

if(BEOS)
	add_definitions(-DUSE_BEOS)
	set(stratagus_SRCS ${stratagus_SRCS} ${beos_SRCS})
endif()

if(MAEMO)
	add_definitions(-DUSE_MAEMO ${LIBOSSO_CFLAGS})
	include_directories(${LIBOSSO_INCLUDE_DIRS})
	set(stratagus_SRCS ${stratagus_SRCS} ${maemo_SRCS})
	set(stratagus_LIBS ${stratagus_LIBS} ${LIBOSSO_LIBRARIES})
	# Hack/Bug: On Maemo is not possible to disable touch screen input
	set(ENABLE_TOUCHSCREEN ON)
endif()

# Stratagus needs to have char by default signed
# No idea how to tell this to other compilers
if(CMAKE_COMPILER_IS_GNUCC)
	add_definitions(-fsigned-char)
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

if(ENABLE_STRIP)
	set(CMAKE_EXE_LINKER_FLAGS "-s")
else()
	set(CMAKE_EXE_LINKER_FLAGS "")
endif()

if(ENABLE_TOUCHSCREEN)
	add_definitions(-DUSE_TOUCHSCREEN)
endif()

# Print compile info

message("==================================")

macro(log_package PACKAGE_NAME PACKAGE)
	if(NOT WITH_${PACKAGE})
		message("${PACKAGE_NAME}: Disabled (Enable by param -DWITH_${PACKAGE}=ON)")
	elseif(WITH_${PACKAGE} AND ${PACKAGE}_FOUND)
		message("${PACKAGE_NAME}: Found and enabled (Disable by param -DWITH_${PACKAGE}=OFF)")
	else()
		message("${PACKAGE_NAME}: Not Found")
	endif()
endmacro()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	message("Debug mode: Yes (Disable by param -DCMAKE_BUILD_TYPE=Release)")
else()
	message("Debug mode: No (Enable by param -DCMAKE_BUILD_TYPE=Debug)")
endif()

if(ENABLE_STRIP)
	message("Strip executables: Yes (Disable by param -DENABLE_STRIP=OFF)")
else()
	message("Strip executables: No (Enable by param -DENABLE_STRIP=ON)")
endif()

if(ENABLE_STATIC)
	message("Static linking: Yes (Disable by param -DENABLE_STATIC=OFF)")
else()
	message("Static linking: No (Enable by param -DENABLE_STATIC=ON)")
endif()

if(WIN32 AND MSVC)
	message("Platform: Windows (MSVC)")
elseif(WIN32 AND MINGW)
	message("Platform: Windows (MinGW)")
elseif(WIN32)
	message("Platform: Windows (Other)")
elseif(APPLE)
	message("Platform: Mac")
elseif(BSD)
	message("Platform: BSD")
elseif(BEOS)
	message("Platform: BEOS")
elseif(MAEMO)
	message("Platform: Maemo")
else()
	message("Platform: Other/No platform code")
endif()

if(WIN32 AND ENABLE_STDIO_REDIRECT)
	message("Redirect stdio: Yes (Disable by param -DENABLE_STDIO_REDIRECT=OFF)")
elseif(NOT WIN32)
	message("Redirect stdio: Unsupported on this platform")
else()
	message("Redirect stdio: No (Enable by param -DENABLE_STDIO_REDIRECT=ON)")
endif()

if(ENABLE_TOUCHSCREEN)
	message("Touchscreen input: Yes (Disable by param -DENABLE_TOUCHSCREEN=OFF)")
else()
	message("Touchscreen input: No (Enable by param -DENABLE_TOUCHSCREEN=ON)")
endif()

if(ENABLE_DOC AND DOXYGEN_FOUND)
	message("Doxygen documentation: Yes (Disable by param -DENABLE_DOC=OFF)")
else()
	message("Doxygen documentation: No (Enable by param -DENABLE_DOC=ON)")
endif()

if(ENABLE_DEV)
	message("Game development files: Yes (Disable by param -DENABLE_DEV=OFF)")
else()
	message("Game development files: No (Enable by param -DENABLE_DEV=ON)")
endif()

if(ENABLE_UPX AND SELF_PACKER_FOR_EXECUTABLE)
	message("Upx packer: Yes (Disable by param -DENABLE_UPX=OFF)")
else()
	message("Upx packer: No (Enable by param -DENABLE_UPX=ON)")
endif()

if(WIN32 AND ENABLE_NSIS AND MAKENSIS_FOUND)
	message("NSIS Installer: Yes (Disable by param -DENABLE_NSIS=OFF)")
elseif(NOT WIN32)
	message("NSIS Installer: Unsupported on this platform")
else()
	message("NSIS Installer: No (Enable by param -DENABLE_NSIS=ON)")
endif()

log_package("Bzip2" "BZIP2")
log_package("MikMod" "MIKMOD")
log_package("Mng" "MNG")
log_package("Ogg/Vorbis" "OGGVORBIS")
log_package("Theora" "THEORA")
log_package("X11" "X11")

if(WITH_RENDERER STREQUAL "OpenGL" AND OPENGL_FOUND)
	message("Renderer: OpenGL")
elseif(WITH_RENDERER STREQUAL "OpenGLES" AND OPENGLES_FOUND AND SDLGLES_FOUND)
	message("Renderer: OpenGL ES 1.1")
else()
	message("Renderer: Native SDL")
endif()

message("==================================")

# Compile Stratagus

add_custom_command(OUTPUT tolua.cpp
	COMMAND ${TOLUA++_APP} ARGS -L stratagus.lua -o ${CMAKE_CURRENT_BINARY_DIR}/tolua.cpp stratagus.pkg
	DEPENDS ${tolua_FILES}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/tolua
)

add_executable(stratagus WIN32 ${stratagus_SRCS} ${stratagus_HDRS})
target_link_libraries(stratagus ${stratagus_LIBS})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set_target_properties(stratagus PROPERTIES OUTPUT_NAME stratagus-dbg)
endif()

if(WIN32 AND MINGW AND ENABLE_STATIC)
	set_target_properties(stratagus PROPERTIES LINK_FLAGS "${LINK_FLAGS} -static-libgcc -Wl,--stack,10485760")
endif()

########### next target ###############

set(metaserver_SRCS
	metaserver/cmd.cpp
	metaserver/db.cpp
	metaserver/games.cpp
	metaserver/main.cpp
	metaserver/netdriver.cpp
	src/network/lowlevel.cpp
)

set(metaserver_HDRS
	metaserver/cmd.h
	metaserver/db.h
	metaserver/games.h
	metaserver/netdriver.h
)

source_group(metaserver FILES ${metaserver_SRCS} ${metaserver_HDRS})

if(SQLITE_FOUND)
	add_executable(metaserver ${metaserver_SRCS} ${metaserver_HDRS})
	target_link_libraries(metaserver ${SDL_LIBRARY} ${SQLITE_LIBRARIES})

	if(WIN32 AND MINGW AND ENABLE_STATIC)
		set_target_properties(metaserver PROPERTIES LINK_FLAGS "${LINK_FLAGS} -static-libgcc")
	endif()
endif()

########### next target ###############

set(png2stratagus_SRCS
	tools/png2stratagus.cpp
)
source_group(png2stratagus FILES ${png2stratagus_SRCS})

add_executable(png2stratagus ${png2stratagus_SRCS})
target_link_libraries(png2stratagus ${PNG_LIBRARY} ${ZLIB_LIBRARIES})

if(WIN32 AND MINGW AND ENABLE_STATIC)
	set_target_properties(png2stratagus PROPERTIES LINK_FLAGS "${LINK_FLAGS} -static-libgcc")
endif()


########### next target ###############

set(gameheaders_HDRS
	gameheaders/stratagus-game-installer.nsi
	gameheaders/stratagus-game-launcher.h
	gameheaders/stratagus-maemo-extract.h
)
source_group(gameheaders FILES ${gameheaders_HDRS})

########### next target ###############

set(doxygen_FILES
	${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
	doc/Doxyfile-footer.html
	doc/Doxyfile-header.html
	${stratagus_SRCS}
	${stratagus_HDRS}
	${metaserver_SRCS}
	${metaserver_HDRS}
	${gameheaders_HDRS}
	${png2stratagus_SRCS}
)

if(ENABLE_DOC AND DOXYGEN_FOUND)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
	add_custom_command(OUTPUT doxygen/index.html
		COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
		DEPENDS ${doxygen_FILES}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMENT "Generating Stratagus souce code documentation with Doxygen" VERBATIM
	)
	add_custom_target(doc ALL DEPENDS doxygen/index.html)
	set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES doxygen)
endif()

########### next target ###############

macro(self_packer PACKER_TARGET)
	get_target_property(PACKER_NAME ${PACKER_TARGET} OUTPUT_NAME)
	if(NOT PACKER_NAME)
		set(PACKER_NAME ${PACKER_TARGET})
	endif()
	add_custom_command(TARGET ${PACKER_TARGET} POST_BUILD
		COMMAND ${SELF_PACKER_FOR_EXECUTABLE}
		ARGS ${SELF_PACKER_FOR_EXECUTABLE_FLAGS} ${PACKER_NAME}${CMAKE_EXECUTABLE_SUFFIX}
	)
endmacro()

if(ENABLE_UPX AND SELF_PACKER_FOR_EXECUTABLE)
	self_packer(stratagus)
	self_packer(png2stratagus)
	if(SQLITE_FOUND)
		self_packer(metaserver)
	endif()
endif()

########### next target ###############

if(WIN32 AND ENABLE_NSIS AND MAKENSIS_FOUND)
	file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/COPYING DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
	file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/win32/stratagus.ico DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
	add_custom_command(OUTPUT Stratagus-${STRATAGUS_VERSION}${MAKENSIS_SUFFIX}
		COMMAND ${MAKENSIS} ARGS ${MAKENSIS_FLAGS} ${CMAKE_CURRENT_SOURCE_DIR}/src/win32/stratagus.nsi
		DEPENDS src/win32/stratagus.nsi stratagus COPYING
		COMMENT "Generating Stratagus Windows NSIS Installer" VERBATIM
	)
	add_custom_target(nsis ALL DEPENDS Stratagus-${STRATAGUS_VERSION}${MAKENSIS_SUFFIX})
endif()

########### install files ###############

install(TARGETS stratagus DESTINATION games)
install(TARGETS png2stratagus DESTINATION bin)

if(SQLITE_FOUND)
	install(TARGETS metaserver DESTINATION sbin)
endif()

if(ENABLE_DOC AND DOXYGEN_FOUND)
	install(FILES doc/stratagus.6 DESTINATION share/man/man6)
	install(FILES
		doc/development.html
		doc/faq.html
		doc/gpl.html
		doc/guichan-copyright.txt
		doc/changelog.html
		doc/index.html
		doc/install.html
		doc/media.html
		doc/metaserver_protocol.txt
		doc/README-SDL.txt
		DESTINATION share/doc/stratagus
	)
	install(DIRECTORY doc/graphics doc/scripts ${CMAKE_CURRENT_BINARY_DIR}/doxygen DESTINATION share/doc/stratagus)
endif(ENABLE_DOC AND DOXYGEN_FOUND)

if(ENABLE_DEV)
	install(FILES ${gameheaders_HDRS} DESTINATION include)
endif()
