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
#    Copyright (C) 2011-2014  Pali Rohár <pali.rohar@gmail.com>, cybermind <cybermindid@gmail.com>
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

#########################
# Version configuration #
#########################
# Stratagus major version
set(STRATAGUS_MAJOR_VERSION 3)
# Stratagus minor version (maximum 99)
set(STRATAGUS_MINOR_VERSION 3)
# Stratagus patch level (maximum 99)
set(STRATAGUS_PATCH_LEVEL 1)
# Stratagus patch level 2
set(STRATAGUS_PATCH_LEVEL2 0)
#########################

###############################
# Build vendored dependencies #
###############################
# Build lua 5.1
option(BUILD_VENDORED_LUA OFF)
###############################

project(stratagus)
cmake_minimum_required(VERSION 2.9)
cmake_policy(VERSION 3.10..3.20.2)

# useful for clangd LSP server
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(EXISTS ${WIN32_CMAKE_PREFIX_PATH})
	list(APPEND CMAKE_PREFIX_PATH "${WIN32_CMAKE_PREFIX_PATH}")
	message("Using prefix path ${CMAKE_PREFIX_PATH}")
endif()

# C++ standard
set(CMAKE_CXX_STANDARD 17)

set(STRATAGUS_VERSION "${STRATAGUS_MAJOR_VERSION}.${STRATAGUS_MINOR_VERSION}.${STRATAGUS_PATCH_LEVEL}")
set(STRATAGUS_VERSION_FULL "${STRATAGUS_VERSION}.${STRATAGUS_PATCH_LEVEL2}")

if(${STRATAGUS_PATCH_LEVEL2} GREATER 0)
	set(STRATAGUS_VERSION "${STRATAGUS_VERSION_FULL}")
endif()

# Stratagus sources

include_directories(
	src/include
	src/guichan/include
	src/guichan/include/guichan
        src/third_party/
	${CMAKE_CURRENT_BINARY_DIR}
)

set(action_SRCS
	src/action/action_attack.cpp
	src/action/action_board.cpp
	src/action/action_build.cpp
	src/action/action_built.cpp
	src/action/action_defend.cpp
	src/action/action_die.cpp
	src/action/action_explore.cpp
	src/action/action_follow.cpp
	src/action/action_move.cpp
	src/action/action_patrol.cpp
	src/action/action_repair.cpp
	src/action/action_research.cpp
	src/action/action_resource.cpp
	src/action/action_spellcast.cpp
	src/action/action_still.cpp
	src/action/action_train.cpp
	src/action/action_unload.cpp
	src/action/action_upgradeto.cpp
	src/action/actions.cpp
	src/action/command.cpp
)
source_group(action FILES ${action_SRCS})

set(animation_SRCS
	src/animation/animation.cpp
	src/animation/animation_attack.cpp
	src/animation/animation_die.cpp
	src/animation/animation_exactframe.cpp
	src/animation/animation_frame.cpp
	src/animation/animation_goto.cpp
	src/animation/animation_ifvar.cpp
	src/animation/animation_label.cpp
	src/animation/animation_luacallback.cpp
	src/animation/animation_move.cpp
	src/animation/animation_randomgoto.cpp
	src/animation/animation_randomrotate.cpp
	src/animation/animation_randomsound.cpp
	src/animation/animation_randomwait.cpp
	src/animation/animation_rotate.cpp
	src/animation/animation_setplayervar.cpp
	src/animation/animation_setvar.cpp
	src/animation/animation_sound.cpp
	src/animation/animation_spawnmissile.cpp
	src/animation/animation_spawnunit.cpp
	src/animation/animation_unbreakable.cpp
	src/animation/animation_wait.cpp
	src/animation/animation_wiggle.cpp
)
source_group(animation FILES ${animation_SRCS})

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
	src/guichan/rectangle.cpp
	src/guichan/sdl/gsdl.cpp
	src/guichan/sdl/sdlgraphics.cpp
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

set(map_SRCS
	src/map/fov.cpp
	src/map/fow.cpp
	src/map/fow_utils.cpp
	src/map/map.cpp
	src/map/map_draw.cpp
	src/map/map_fog.cpp
	src/map/map_radar.cpp
	src/map/map_wall.cpp
	src/map/mapfield.cpp
	src/map/minimap.cpp
	src/map/script_map.cpp
	src/map/script_tileset.cpp
	src/map/tileset.cpp
)
source_group(map FILES ${map_SRCS})

set(missile_SRCS
	src/missile/missile.cpp
	src/missile/missile_cliptotarget.cpp
	src/missile/missile_continuous.cpp
	src/missile/missile_cycleonce.cpp
	src/missile/missile_deathcoil.cpp
	src/missile/missile_fire.cpp
	src/missile/missile_flameshield.cpp
	src/missile/missile_hit.cpp
	src/missile/missile_landmine.cpp
	src/missile/missile_none.cpp
	src/missile/missile_parabolic.cpp
	src/missile/missile_pointotpointwithhit.cpp
	src/missile/missile_pointtopoint.cpp
	src/missile/missile_pointtopointbounce.cpp
	src/missile/missile_pointtopointcycleonce.cpp
	src/missile/missile_stay.cpp
	src/missile/missile_straightfly.cpp
	src/missile/missile_tracer.cpp
	src/missile/missile_whirlwind.cpp
	src/missile/missileconfig.cpp
	src/missile/script_missile.cpp
)
source_group(missile FILES ${missile_SRCS})

set(network_SRCS
	src/network/commands.cpp
	src/network/net_lowlevel.cpp
	src/network/net_message.cpp
	src/network/netconnect.cpp
	src/network/network.cpp
	src/network/netsockets.cpp
        src/network/online_service.cpp
        src/network/mdns.cpp
)
source_group(network FILES ${network_SRCS})

set(particle_SRCS
	src/particle/graphicanimation.cpp
	src/particle/chunkparticle.cpp
	src/particle/particlemanager.cpp
	src/particle/radialparticle.cpp
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
	src/sound/music.cpp
	src/sound/script_sound.cpp
	src/sound/sound.cpp
	src/sound/sound_id.cpp
	src/sound/sound_server.cpp
	src/sound/unitsound.cpp
)
source_group(sound FILES ${sound_SRCS})

set(spell_SRCS
	src/spell/script_spell.cpp
	src/spell/spell_adjustvariable.cpp
	src/spell/spell_adjustvital.cpp
	src/spell/spell_areaadjustvital.cpp
	src/spell/spell_areabombardment.cpp
	src/spell/spell_capture.cpp
	src/spell/spell_demolish.cpp
	src/spell/spell_luacallback.cpp
	src/spell/spell_polymorph.cpp
	src/spell/spell_spawnmissile.cpp
	src/spell/spell_spawnportal.cpp
	src/spell/spell_summon.cpp
	src/spell/spell_teleport.cpp
	src/spell/spells.cpp
)
source_group(spell FILES ${spell_SRCS})

set(stratagusmain_SRCS
	src/stratagus/construct.cpp
	src/stratagus/groups.cpp
	src/stratagus/iolib.cpp
	src/stratagus/luacallback.cpp
	src/stratagus/main.cpp
	src/stratagus/mainloop.cpp
	src/stratagus/parameters.cpp
	src/stratagus/player.cpp
	src/stratagus/script.cpp
	src/stratagus/script_player.cpp
	src/stratagus/selection.cpp
	src/stratagus/stratagus.cpp
	src/stratagus/title.cpp
	src/stratagus/translate.cpp
	src/stratagus/util.cpp
)
source_group(stratagusmain FILES ${stratagus_SRCS})

set(ui_SRCS
	src/ui/botpanel.cpp
	src/ui/button_checks.cpp
	src/ui/contenttype.cpp
	src/ui/icons.cpp
	src/ui/interface.cpp
	src/ui/mainscr.cpp
	src/ui/uibuttons_proc.cpp
	src/ui/mouse.cpp
	src/ui/popup.cpp
	src/ui/script_ui.cpp
	src/ui/statusline.cpp
	src/ui/ui.cpp
	src/ui/uitimer.cpp
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
	src/unit/unitptr.cpp
	src/unit/unittype.cpp
	src/unit/upgrade.cpp
)
source_group(unit FILES ${unit_SRCS})

set(video_SRCS
	src/video/color.cpp
	src/video/cursor.cpp
	src/video/font.cpp
	src/video/graphic.cpp
	src/video/linedraw.cpp
        src/video/mng.cpp
	src/video/movie.cpp
	src/video/png.cpp
	src/video/sdl.cpp
	src/video/video.cpp
        src/video/shaders.cpp
)
source_group(video FILES ${video_SRCS})

set(win32_SRCS
	src/win32/SetupConsole_win32.cpp
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
	src/tolua/translate.pkg
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
	${animation_SRCS}
	${ai_SRCS}
	${editor_SRCS}
	${game_SRCS}
	${guichan_SRCS}
	${map_SRCS}
	${missile_SRCS}
	${network_SRCS}
	${particle_SRCS}
	${pathfinder_SRCS}
	${sound_SRCS}
	${spell_SRCS}
	${stratagusmain_SRCS}
	${ui_SRCS}
	${unit_SRCS}
	${video_SRCS}
        ${hqx_SRCS}
        ${xbrz_SRCS}
	${CMAKE_CURRENT_BINARY_DIR}/tolua.cpp
)

set(stratagus_guichan_HDRS
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

set(stratagus_action_HDRS
	src/include/action/action_attack.h
	src/include/action/action_board.h
	src/include/action/action_build.h
	src/include/action/action_built.h
	src/include/action/action_defend.h
	src/include/action/action_die.h
	src/include/action/action_follow.h
	src/include/action/action_move.h
	src/include/action/action_patrol.h
	src/include/action/action_repair.h
	src/include/action/action_research.h
	src/include/action/action_resource.h
	src/include/action/action_spellcast.h
	src/include/action/action_still.h
	src/include/action/action_train.h
	src/include/action/action_unload.h
	src/include/action/action_upgradeto.h
)

set(stratagus_animation_HDRS
	src/include/animation/animation_attack.h
	src/include/animation/animation_die.h
	src/include/animation/animation_exactframe.h
	src/include/animation/animation_frame.h
	src/include/animation/animation_goto.h
	src/include/animation/animation_ifvar.h
	src/include/animation/animation_label.h
	src/include/animation/animation_luacallback.h
	src/include/animation/animation_move.h
	src/include/animation/animation_randomgoto.h
	src/include/animation/animation_randomrotate.h
	src/include/animation/animation_randomsound.h
	src/include/animation/animation_randomwait.h
	src/include/animation/animation_rotate.h
	src/include/animation/animation_setplayervar.h
	src/include/animation/animation_setvar.h
	src/include/animation/animation_sound.h
	src/include/animation/animation_spawnmissile.h
	src/include/animation/animation_spawnunit.h
	src/include/animation/animation_unbreakable.h
	src/include/animation/animation_wait.h
	src/include/animation/animation_wiggle.h
)

set(stratagus_spell_HDRS
	src/include/spell/spell_adjustvariable.h
	src/include/spell/spell_adjustvital.h
	src/include/spell/spell_areaadjustvital.h
	src/include/spell/spell_areabombardment.h
	src/include/spell/spell_capture.h
	src/include/spell/spell_demolish.h
	src/include/spell/spell_luacallback.h
	src/include/spell/spell_polymorph.h
	src/include/spell/spell_spawnmissile.h
	src/include/spell/spell_spawnportal.h
	src/include/spell/spell_summon.h
	src/include/spell/spell_teleport.h
)

set(stratagus_generic_HDRS
	src/ai/ai_local.h
	src/video/intern_video.h
	src/video/renderer.h
	src/include/actions.h
	src/include/ai.h
	src/include/animation.h
	src/include/color.h
	src/include/commands.h
	src/include/construct.h
	src/include/cursor.h
	src/include/depend.h
	src/include/editor.h
	src/include/online_service.h
	src/include/font.h
	src/include/fov.h
	src/include/fow.h
	src/include/fow_utils.h
	src/include/game.h
	src/include/icons.h
	src/include/interface.h
	src/include/iocompat.h
	src/include/iolib.h
	src/include/luacallback.h
	src/include/map.h
	src/include/menus.h
	src/include/minimap.h
	src/include/missile.h
	src/include/missileconfig.h
	src/include/movie.h
	src/include/myendian.h
	src/include/net_lowlevel.h
	src/include/net_serialization.h
	src/include/net_message.h
	src/include/netconnect.h
	src/include/network.h
	src/include/network/netsockets.h
	src/include/parameters.h
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
	src/include/ui/contenttype.h
	src/include/ui/popup.h
	src/include/ui/statusline.h
	src/include/ui/uitimer.h
	src/include/ui.h
	src/include/unit.h
	src/include/unit_cache.h
	src/include/unit_find.h
	src/include/unit_manager.h
	src/include/unitptr.h
	src/include/unitsound.h
	src/include/unittype.h
	src/include/upgrade.h
	src/include/upgrade_structs.h
	src/include/util.h
	src/include/vec2i.h
	src/include/version.h
	src/include/video.h
	src/include/shaders.h
	src/include/viewport.h
	src/include/wav.h
	src/include/widgets.h
	src/include/SetupConsole_win32.h
)


source_group(include FILES ${stratagus_generic_HDRS})
source_group(include\\action FILES ${stratagus_action_HDRS})
source_group(include\\animation FILES ${stratagus_animation_HDRS})
source_group(include\\guichan FILES ${stratagus_guichan_HDRS})
source_group(include\\spell FILES ${stratagus_spell_HDRS})


set(stratagus_HDRS
	${stratagus_generic_HDRS}
	${stratagus_action_HDRS}
	${stratagus_animation_HDRS}
	${stratagus_guichan_HDRS}
	${stratagus_spell_HDRS}
	${CMAKE_CURRENT_BINARY_DIR}/version-generated.h
	version-generated.h # This line is needed for CMake when generating Doxygen documentation
)

# Configuration types
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Profile" CACHE STRING "" FORCE)

# Additional platform checks

if(UNIX AND CMAKE_SYSTEM_NAME MATCHES BSD)
	set(BSD true)
endif()

if(UNIX AND CMAKE_SYSTEM_NAME MATCHES Linux)
	set(LINUX true)
endif()

if(WIN32 AND MSVC AND NOT CMAKE_PREFIX_PATH)
  # use a default
  set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../dependencies)
endif()

# Find all libraries

option(ENABLE_STATIC "Compile Stratagus as static executable" OFF)

if(ENABLE_STATIC)
	set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib;.a")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

if(APPLE)
	list(APPEND CMAKE_PREFIX_PATH /opt/homebrew/)
endif()

if(NOT WIN32 OR NOT MSVC)
	include(CheckCXXSourceCompiles)
	set(FS_SRC "
	#include <experimental/filesystem> 
	namespace fs = std::experimental::filesystem;
	int main(int argc, char **argv) {
		fs::path p = fs::path(\".\");
		if (fs::absolute(p).is_absolute()) {
			return 0;
		} else {
			return 1;
		}
	}
	")
	check_cxx_source_compiles("${FS_SRC}" HAS_17_FS)
	if(NOT HAS_17_FS)
		set(CMAKE_REQUIRED_LIBRARIES stdc++fs)
		check_cxx_source_compiles("${FS_SRC}" HAS_EXP_17_FS_WITH_STDC)
		if(HAS_EXP_17_FS_WITH_STDC)
			set(stratagus_LIBS ${stratagus_LIBS} stdc++fs)
		else()
			set(CMAKE_REQUIRED_LIBRARIES c++fs)
			check_cxx_source_compiles("${FS_SRC}" HAS_EXP_17_FS_WITH_CLIB)
			if(HAS_EXP_17_FS_WITH_CLIB)
				set(stratagus_LIBS ${stratagus_LIBS} c++fs)
			else()
				message(FATAL_ERROR "I don't know how to compile with C++17 filesystem support on your system")
			endif()
		endif()
	endif()
endif()

include(ExternalProject)
if(BUILD_VENDORED_LUA)
  ExternalProject_Add( Lua51B
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/third-party/lua-5.1.5/
    PREFIX "${CMAKE_CURRENT_BINARY_DIR}/lua/"
    INSTALL_COMMAND ""
  )

  set(Lua51_FOUND true)
  if(MSVC)
    set(LUA_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/lua/src/Lua51B-build/$<CONFIGURATION>/lua51.lib")
    set(LUA_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/lua/src/Lua51B-build/")
    set(TOLUA++_FOUND true)
    set(TOLUA++_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/third-party/lua-5.1.5/toluapp/include/)
    set(TOLUA++_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/lua/src/Lua51B-build/$<CONFIGURATION>/toluapp51.lib")
    set(TOLUA++_APP "${CMAKE_CURRENT_BINARY_DIR}/lua/src/Lua51B-build/$<CONFIGURATION>/toluapp.exe")
  else()
	set(LUA_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/lua/src/Lua51B-build/liblua51.a")
	set(LUA_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/lua/src/Lua51B-build/")
	set(TOLUA++_FOUND true)
	set(TOLUA++_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/third-party/lua-5.1.5/toluapp/include/)
	set(TOLUA++_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/lua/src/Lua51B-build/libtoluapp51.a")
	set(TOLUA++_APP "${CMAKE_CURRENT_BINARY_DIR}/lua/src/Lua51B-build/toluapp")
  endif(MSVC)
else()
  find_package(Lua51 REQUIRED)
  find_package(Tolua++ REQUIRED)
endif()

find_package(PNG REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenMP)

if(WIN32)
	find_package(MakeNSIS)
endif()

if(ENABLE_DOC)
find_package(Doxygen
             REQUIRED dot
             OPTIONAL_COMPONENTS mscgen dia)
endif()

find_package(BZip2)
find_package(StackTrace)
find_package(MNG)
find_package(OggVorbis)
find_package(Theora)

find_package(Doxygen)
find_package(SelfPackers)

include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckSymbolExists)

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
	option(ENABLE_STDIO_REDIRECT "Redirect Stratagus console output to files on Windows" ON)
	option(ENABLE_NSIS "Create Stratagus Window NSIS Installer" OFF)
endif()

option(ENABLE_TOUCHSCREEN "Use touchscreen input" OFF)

option(EAGER_LOAD "Load all game data at startup, may avoid stutter during gameplay at the cost of memory" OFF)

option(WITH_BZIP2 "Compile Stratagus with BZip2 compression support" ON)
option(WITH_MNG "Compile Stratagus with MNG image library" ON)
option(WITH_OGGVORBIS "Compile Stratagus with OGG/Vorbis sound library" ON)
option(WITH_THEORA "Compile Stratagus with Theroa video library" ON)
option(WITH_STACKTRACE "Compile Stratagus with StackTrace library" ON)
option(WITH_OPENMP "Compile Stratagus with OpenMP for parallel processing" ON)

option(ENABLE_DOC "Generate Stratagus source code documentation with Doxygen" OFF)
option(ENABLE_DEV "Install Stratagus game development headers files" OFF)
option(ENABLE_UPX "Compress Stratagus executable binary with UPX packer" OFF)
option(ENABLE_STRIP "Strip all symbols from executables" OFF)
option(ENABLE_USEGAMEDIR "Place all files created by Stratagus(logs, savegames) in game directory(old behavior), otherwise place everything in user directory(new behavior)" OFF)
option(ENABLE_MULTIBUILD "Compile Stratagus on all CPU cores simltaneously in MSVC" ON)

# Install paths
set(BINDIR "bin" CACHE PATH "Where to install user binaries")
set(SBINDIR "sbin" CACHE PATH "Where to install system binaries")
set(GAMEDIR "games" CACHE PATH "Where to install games binaries")
set(DOCDIR "share/doc/stratagus" CACHE PATH "Sets the doc directory to a non-default location.")
set(MANDIR "share/man/man6" CACHE PATH "Sets the man directory to a non-default location.")
set(PIXMAPSDIR "share/pixmaps" CACHE PATH "Sets the pixmaps directory to a non-default location.")
set(STRATAGUS_HEADERS "include" CACHE PATH "Where to install stratagus headers.")

if(NOT IS_ABSOLUTE "${PIXMAPSDIR}")
	set(PIXMAPSDIRABS "${CMAKE_INSTALL_PREFIX}/${PIXMAPSDIR}")
else()
	set(PIXMAPSDIRABS "${PIXMAPSDIR}")
endif()

if (WIN32)
  	set(SDL2_LIBRARY ${SDL2_LIBRARY} ${SDL2main_LIBRARY})
  	message(${SDL2_LIBRARY})
endif()

# Stratagus definitions

add_definitions(-DUSE_ZLIB -DPIXMAPS=\"${PIXMAPSDIRABS}\")
include_directories(${LUA_INCLUDE_DIR} ${SDL2_INCLUDE_DIR} ${SDL2_MIXER_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${TOLUA++_INCLUDE_DIR} ${ZLIB_INCLUDE_DIRS})
set(stratagus_LIBS ${stratagus_LIBS} ${LUA_LIBRARIES} ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} ${TOLUA++_LIBRARY} ${ZLIB_LIBRARIES})

if(WIN32 AND NOT ENABLE_STDIO_REDIRECT)
	add_definitions(-DNO_STDIO_REDIRECT)
endif()

if(WITH_BZIP2 AND BZIP2_FOUND)
	add_definitions(-DUSE_BZ2LIB ${BZIP2_DEFINITIONS})
	include_directories(${BZIP2_INCLUDE_DIR})
	set(stratagus_LIBS ${stratagus_LIBS} ${BZIP2_LIBRARIES})
endif()

if(WITH_MNG AND MNG_FOUND)
	add_definitions(-DUSE_MNG)
	include_directories(${MNG_INCLUDE_DIR})
	set(stratagus_LIBS ${stratagus_LIBS} ${MNG_LIBRARY})
	if (MSVC)
		include_directories(${JPEG_INCLUDE_DIR})
		set(stratagus_LIBS ${stratagus_LIBS} ${JPEG_LIBRARY})
	endif()
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

if(WITH_STACKTRACE AND STACKTRACE_FOUND)
	add_definitions(-DUSE_STACKTRACE ${STACKTRACE_DEFINITIONS})
	include_directories(${STACKTRACE_PROJECT_DIR})
	set(stratagus_LIBS ${stratagus_LIBS} ${STACKTRACE_LIBRARY})
endif()

# Platform definitions

if(WIN32)
	# define winXP SP2 as minimum for everything
	add_definitions(-DUSE_WIN32 -DNTDDI_VERSION=0x05010200 -D_WIN32_WINNT=0x0501 -DWINVER=0x0501)
	set(stratagus_SRCS ${stratagus_SRCS} ${win32_SRCS})
	set(stratagus_LIBS ${stratagus_LIBS} dsound winmm ws2_32 dbghelp)
endif()

if (WIN32 AND MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE=1 -DNOMINMAX)
endif()

if (WIN32 AND MINGW)
	set(stratagus_LIBS ${stratagus_LIBS} dxguid)
endif()

if(APPLE)
	add_definitions(-DUSE_MAC)
	set(SDL2_LIBRARY ${SDL2_LIBRARY} SDL2main)
endif()

if(BSD)
	add_definitions(-DUSE_BSD)
endif()

if(LINUX)
	add_definitions(-DUSE_LINUX)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -ggdb")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -ggdb")
endif()

if(BEOS)
	add_definitions(-DUSE_BEOS)
	set(stratagus_SRCS ${stratagus_SRCS} ${beos_SRCS})
endif()

if(HAIKU)
	set(stratagus_LIBS ${stratagus_LIBS} -lnetwork)
endif()

# Stratagus needs to have char by default signed
# No idea how to tell this to other compilers
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsigned-char")
endif()
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsigned-char")
endif()
if(WIN32 AND MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Profile")
        find_package(coz-profiler)
        if(COZ-PROFILER_FOUND)
		add_definitions(-DHAVE_COZ_PROFILER)
		include_directories(${COZ_INCLUDE_DIRS})
		set(stratagus_LIBS ${stratagus_LIBS} ${COZ_LIBRARIES} -ldl)
	endif()
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg -g -ggdb")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg -g -ggdb")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
endif()
# TODO: Add support for other compilers

check_type_size(errno_t ERRNOT)
check_function_exists("strcpy_s" HAVE_STRCPYS)
check_function_exists("strncpy_s" HAVE_STRNCPYS)
check_function_exists("strcasestr" HAVE_STRCASESTR)
check_function_exists("strnlen" HAVE_STRNLEN)
check_function_exists("getopt" HAVE_GETOPT)

# mingw-w64 does not have strcat_s in any include file, but function symbol in library exists
# so rather check if we have strcat_s in string.h file
check_symbol_exists("strcat_s" "string.h" HAVE_STRCATS)

if(HAVE_ERRNOT)
	add_definitions(-DHAVE_ERRNOT)
endif()

if(HAVE_STRCPYS)
	add_definitions(-DHAVE_STRCPYS)
endif()

if(HAVE_STRNCPYS)
	add_definitions(-DHAVE_STRNCPYS)
endif()

if(HAVE_STRCATS)
	add_definitions(-DHAVE_STRCATS)
endif()

if(HAVE_STRCASESTR)
	add_definitions(-DHAVE_STRCASESTR)
endif()

if(HAVE_STRNLEN)
	add_definitions(-DHAVE_STRNLEN)
endif()

if(HAVE_GETOPT)
	add_definitions(-DHAVE_GETOPT)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions(-DDEBUG)
	if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
		elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -glldb")
		endif()
	endif()
endif()

if(ENABLE_STRIP)
	if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUC)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s")
	endif()
	# TODO: Add support for other compilers
endif()

if(ENABLE_USEGAMEDIR)
	add_definitions(-DUSE_GAME_DIR)
endif()

if(ENABLE_TOUCHSCREEN)
	add_definitions(-DUSE_TOUCHSCREEN)
endif()

if(EAGER_LOAD)
	# nothing
else()
	add_definitions(-DDYNAMIC_LOAD)
endif()

if(ENABLE_MULTIBUILD)
    if(WIN32 AND MSVC)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
      message(STATUS "Added parallel build arguments to CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
    endif()
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

if(CMAKE_BUILD_TYPE STREQUAL "Profile")
	message("Profiling mode: Yes (Disable by param -DCMAKE_BUILD_TYPE=Release)")
	if(COZ-PROFILER_FOUND)
		message("Coz profiler enabled build: Yes")
	else()
		message("Coz profiler enabled build: No")
	endif()
else()
	message("Profiling mode: No (Enable by param -DCMAKE_BUILD_TYPE=Profile)")
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

if(ENABLE_USEGAMEDIR)
	message("Place game files in: game directory (Place in user directory with -DENABLE_USEGAMEDIR=OFF)")
else()
	message("Place game files in: user directory (Place in game directory with -DENABLE_USEGAMEDIR=ON)")
endif()

if(ENABLE_MULTIBUILD)
	message("Parallel building in MSVC: Yes (Disable by param -DENABLE_MULTIBUILD=OFF)")
else()
	message("Parallel building in MSVC: No (Enable by param -DENABLE_MULTIBUILD=ON)")
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
elseif(HAIKU)
	message("Platform: Haiku")
elseif(LINUX)
	message("Platform: Linux")
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

if(EAGER_LOAD)
	message("Eager load: Yes (Disable by param -DEAGER_LOAD=OFF)")
else()
	message("Eager load: No (Enable by param -DEAGER_LOAD=ON)")
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
log_package("Mng" "MNG")
log_package("Ogg/Vorbis" "OGGVORBIS")
log_package("StackTrace" "STACKTRACE")
log_package("Theora" "THEORA")
log_package("OpenMP" "OPENMP")

message("==================================")

# Compile Stratagus

if(NOT CMAKE_CROSSCOMPILING)
	# Generate header file version-generated.h with git revision
	# Run tool genversion at compile time which (re)generate file version-generated.h when needed
	set(genversion_SRCS tools/genversion.cpp)
	source_group(genversion FILES ${genversion_SRCS})
	add_executable(genversion ${genversion_SRCS})
	add_custom_target(version-generated.h ALL
		genversion ${CMAKE_CURRENT_BINARY_DIR}/version-generated.h "${STRATAGUS_VERSION}"
		DEPENDS genversion
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		VERBATIM
	)
	set_source_files_properties(version-generated.h PROPERTIES GENERATED true)
	set_source_files_properties(src/include/version.h PROPERTIES OBJECT_DEPENDS version-generated.h) # This is needed for old CMake which does not generate correct dependences
	list(APPEND CLEAN_FILES version-generated.h)
else()
	# Generate header file version-generated.h without git revision
	# When cross compiling we cannot build host binary, so create file at cmake time
	if(NOT "${STRATAGUS_VERSION}" STREQUAL "${STRATAGUS_VERSION_CACHED}" OR NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/version-generated.h)
		file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/version-generated.h
			"/* ${STRATAGUS_MAJOR_VERSION} ${STRATAGUS_MINOR_VERSION} ${STRATAGUS_PATCH_LEVEL} ${STRATAGUS_PATCH_LEVEL2} -1 */\n"
			"/* This file is autogenerated, do not modify it! */\n"
			"#define StratagusMajorVersion ${STRATAGUS_MAJOR_VERSION}\n"
			"#define StratagusMinorVersion ${STRATAGUS_MINOR_VERSION}\n"
			"#define StratagusPatchLevel ${STRATAGUS_PATCH_LEVEL}\n"
			"#define StratagusPatchLevel2 ${STRATAGUS_PATCH_LEVEL2}\n"
		)
		set(STRATAGUS_VERSION_CACHED ${STRATAGUS_VERSION} CACHE INTERNAL "" FORCE)
	endif()
	list(APPEND CLEAN_FILES version-generated.h)
endif()

add_custom_command(OUTPUT tolua.cpp
	COMMAND ${TOLUA++_APP} ARGS -L stratagus.lua -o ${CMAKE_CURRENT_BINARY_DIR}/tolua.cpp stratagus.pkg
	DEPENDS ${tolua_FILES}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/tolua
	VERBATIM
)

if (ENABLE_STDIO_REDIRECT)
   add_executable(stratagus WIN32 ${stratagus_SRCS} ${stratagus_HDRS})
else ()
   add_executable(stratagus ${stratagus_SRCS} ${stratagus_HDRS})
endif ()
target_link_libraries(stratagus ${stratagus_LIBS})

if(OpenMP_CXX_FOUND)
	add_definitions(-DUSE_OPENMP)
	target_compile_options(stratagus PRIVATE ${OpenMP_CXX_FLAGS})
	target_link_libraries(stratagus OpenMP::OpenMP_CXX)
endif()


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set_target_properties(stratagus PROPERTIES OUTPUT_NAME stratagus-dbg)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Profile")
	set_target_properties(stratagus PROPERTIES OUTPUT_NAME stratagus-prof)
endif()

if(WIN32 AND MINGW AND ENABLE_STATIC)
	set_target_properties(stratagus PROPERTIES LINK_FLAGS "${LINK_FLAGS} -static-libgcc -static-libstdc++ -Wl,--stack,10485760")
endif()

if(WIN32)
  if(MSVC)
	target_compile_options(stratagus PRIVATE "$<$<CONFIG:Release>:/Zi>")
	target_link_options(stratagus PRIVATE "$<$<CONFIG:Release>:/DEBUG>")
	target_link_options(stratagus PRIVATE "$<$<CONFIG:Release>:/OPT:REF>")
	target_link_options(stratagus PRIVATE "$<$<CONFIG:Release>:/OPT:ICF>")
  endif()
  set_target_properties(stratagus PROPERTIES LINK_FLAGS "${LINK_FLAGS} -lSDL2main")
  set(CMAKE_EXE_LINKER_FLAGS_PROFILE "${CMAKE_EXE_LINKER_FLAGS}")
endif()
if(APPLE)
  set_target_properties(stratagus PROPERTIES LINK_FLAGS "${LINK_FLAGS} ${SDL2main_LIBRARY}")
  set(CMAKE_EXE_LINKER_FLAGS_PROFILE "${CMAKE_EXE_LINKER_FLAGS}")
endif()

if(BUILD_VENDORED_LUA)
  add_dependencies(stratagus Lua51B)
endif()

########### next target ###############

set(png2stratagus_SRCS
	tools/png2stratagus.cpp
)
source_group(png2stratagus FILES ${png2stratagus_SRCS})

set_source_files_properties(png2stratagus_SRCS PROPERTIES INCLUDE_DIRECTORIES "${PNG_INCLUDE_DIR}")
set_source_files_properties(png2stratagus_SRCS PROPERTIES COMPILE_DEFINITIONS "${PNG_DEFINITIONS}")

add_executable(png2stratagus ${png2stratagus_SRCS})
target_link_libraries(png2stratagus ${PNG_LIBRARY} ${ZLIB_LIBRARIES})

if(WIN32 AND MINGW AND ENABLE_STATIC)
	set_target_properties(png2stratagus PROPERTIES LINK_FLAGS "${LINK_FLAGS} -static-libgcc -static-libstdc++")
endif()


########### next target ###############

set(gameheaders_HDRS
	gameheaders/stratagus-game-installer.nsi
	gameheaders/stratagus-gameutils.h
	gameheaders/stratagus-game-launcher.h
	gameheaders/stratagus-tinyfiledialogs.h
)
source_group(gameheaders FILES ${gameheaders_HDRS})

########### next target ###############

set(doxygen_FILES
	${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
	doc/Doxyfile-footer.html
	doc/Doxyfile-header.html
	${stratagus_SRCS}
	${stratagus_HDRS}
	${gameheaders_HDRS}
	${png2stratagus_SRCS}
)

if(ENABLE_DOC AND DOXYGEN_FOUND)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
	add_custom_command(OUTPUT doxygen/index.html
		COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
		DEPENDS ${doxygen_FILES}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMENT "Generating Stratagus souce code documentation with Doxygen"
		VERBATIM
	)
	add_custom_target(doc ALL DEPENDS doxygen/index.html)
	list(APPEND CLEAN_FILES doxygen)
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
		VERBATIM
	)
endmacro()

if(ENABLE_UPX AND SELF_PACKER_FOR_EXECUTABLE)
	self_packer(stratagus)
	self_packer(png2stratagus)
endif()

########### next target ###############

if(WIN32 AND ENABLE_NSIS AND MAKENSIS_FOUND)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/COPYING DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
	file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/win32/stratagus.ico DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
	# ship our DLLs
	file(GLOB WINDLLS ${CMAKE_PREFIX_PATH}/bin/*.dll)
	file(COPY ${WINDLLS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
	add_custom_command(OUTPUT Stratagus-${STRATAGUS_VERSION}${MAKENSIS_SUFFIX}
		COMMAND ${MAKENSIS} ARGS ${MAKENSIS_FLAGS} -DVERSION=${STRATAGUS_VERSION} -DVIVERSION=${STRATAGUS_VERSION_FULL} ${CMAKE_CURRENT_SOURCE_DIR}/src/win32/stratagus.nsi
		DEPENDS src/win32/stratagus.nsi stratagus COPYING
		COMMENT "Generating Stratagus Windows NSIS Installer"
		VERBATIM
	)
	add_custom_target(nsis ALL DEPENDS Stratagus-${STRATAGUS_VERSION}${MAKENSIS_SUFFIX})
endif()

if (WIN32)
	add_executable(midiplayer WIN32 "src/sound/win32/midiplayer.c")
	set_target_properties(midiplayer PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
	set_target_properties(midiplayer PROPERTIES OUTPUT_NAME "stratagus-midiplayer")
endif()

########### install files ###############

install(TARGETS stratagus DESTINATION ${GAMEDIR})
install(TARGETS png2stratagus DESTINATION ${BINDIR})
if (WIN32)
	install(TARGETS midiplayer DESTINATION ${GAMEDIR})
endif()

if(ENABLE_DOC AND DOXYGEN_FOUND)
	install(FILES doc/stratagus.6 DESTINATION ${MANDIR})
	install(FILES
		doc/development.html
		doc/faq.html
		doc/gpl.html
		doc/guichan-copyright.txt
		doc/changelog.html
		doc/index.html
		doc/install.html
		doc/media.html
		doc/README-SDL.txt
		DESTINATION share/doc/stratagus
	)
	install(DIRECTORY doc/graphics doc/scripts ${CMAKE_CURRENT_BINARY_DIR}/doxygen DESTINATION ${DOCDIR})
endif(ENABLE_DOC AND DOXYGEN_FOUND)

if(ENABLE_DEV)
	install(FILES ${gameheaders_HDRS} DESTINATION ${STRATAGUS_HEADERS})
endif()

########### clean files ###############

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${CLEAN_FILES}")

# uninstall target
if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()
