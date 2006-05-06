# Microsoft Developer Studio Project File - Name="stratagus" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=stratagus - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "stratagus.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "stratagus.mak" CFG="stratagus - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "stratagus - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "stratagus - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "stratagus - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "src\include" /I "include" /I "src\guichan\include" /D "NDEBUG" /D "USE_WIN32" /D "USE_MNG" /D "USE_ZLIB" /D "USE_BZ2LIB" /D "USE_MIKMOD" /D "USE_VORBIS" /D "USE_THEORA" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib winmm.lib opengl32.lib SDL.lib SDLmain.lib zlib.lib libbz2.lib libpng.lib lua.lib ogg_static.lib vorbis_static.lib mikmod.lib theora_static.lib libmng.lib /nologo /stack:0x2000000 /subsystem:windows /profile /machine:I386 /libpath:"lib"

!ELSEIF  "$(CFG)" == "stratagus - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "src\include" /I "include" /I "src\guichan\include" /D "_DEBUG" /D "DEBUG" /D "USE_WIN32" /D "USE_MNG" /D "USE_ZLIB" /D "USE_BZ2LIB" /D "USE_MIKMOD" /D "USE_VORBIS" /D "USE_THEORA" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib winmm.lib opengl32.lib SDL.lib SDLmain.lib zlib.lib libbz2.lib libpng.lib lua.lib ogg_static.lib vorbis_static.lib mikmod.lib theora_static.lib libmng.lib /nologo /stack:0x2000000 /subsystem:windows /debug /machine:I386 /nodefaultlib:"MSVCRT" /libpath:"lib"
# SUBTRACT LINK32 /profile

!ENDIF 

# Begin Target

# Name "stratagus - Win32 Release"
# Name "stratagus - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "src"

# PROP Default_Filter ""
# Begin Group "action"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\action\action_attack.cpp
# End Source File
# Begin Source File

SOURCE=.\src\action\action_board.cpp
# End Source File
# Begin Source File

SOURCE=.\src\action\action_build.cpp
# End Source File
# Begin Source File

SOURCE=.\src\action\action_die.cpp
# End Source File
# Begin Source File

SOURCE=.\src\action\action_follow.cpp
# End Source File
# Begin Source File

SOURCE=.\src\action\action_move.cpp
# End Source File
# Begin Source File

SOURCE=.\src\action\action_patrol.cpp
# End Source File
# Begin Source File

SOURCE=.\src\action\action_repair.cpp
# End Source File
# Begin Source File

SOURCE=.\src\action\action_research.cpp
# End Source File
# Begin Source File

SOURCE=.\src\action\action_resource.cpp
# End Source File
# Begin Source File

SOURCE=.\src\action\action_returngoods.cpp
# End Source File
# Begin Source File

SOURCE=.\src\action\action_spellcast.cpp
# End Source File
# Begin Source File

SOURCE=.\src\action\action_stand.cpp
# End Source File
# Begin Source File

SOURCE=.\src\action\action_still.cpp
# End Source File
# Begin Source File

SOURCE=.\src\action\action_train.cpp
# End Source File
# Begin Source File

SOURCE=.\src\action\action_unload.cpp
# End Source File
# Begin Source File

SOURCE=.\src\action\action_upgradeto.cpp
# End Source File
# Begin Source File

SOURCE=.\src\action\actions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\action\command.cpp
# End Source File
# End Group
# Begin Group "ai"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\ai\ai.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ai\ai_building.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ai\ai_force.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ai\ai_magic.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ai\ai_plan.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ai\ai_resource.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ai\script_ai.cpp
# End Source File
# End Group
# Begin Group "editor"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\editor\editloop.cpp
# End Source File
# Begin Source File

SOURCE=.\src\editor\editor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\editor\edmap.cpp
# End Source File
# Begin Source File

SOURCE=.\src\editor\script_editor.cpp
# End Source File
# End Group
# Begin Group "game"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\game\campaign.cpp
# End Source File
# Begin Source File

SOURCE=.\src\game\game.cpp
# End Source File
# Begin Source File

SOURCE=.\src\game\intro.cpp
# End Source File
# Begin Source File

SOURCE=.\src\game\loadgame.cpp
# End Source File
# Begin Source File

SOURCE=.\src\game\savegame.cpp
# End Source File
# Begin Source File

SOURCE=.\src\game\trigger.cpp
# End Source File
# End Group
# Begin Group "guichan"

# PROP Default_Filter ""
# Begin Group "sdl"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\guichan\sdl\gsdl.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\sdl\sdlgraphics.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\sdl\sdlimageloader.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\sdl\sdlinput.cpp
# End Source File
# End Group
# Begin Group "widgets"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\guichan\widgets\button.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\widgets\checkbox.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\widgets\container.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\widgets\dropdown.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\widgets\icon.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\widgets\label.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\widgets\listbox.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\widgets\radiobutton.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\widgets\scrollarea.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\widgets\slider.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\widgets\textbox.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\widgets\textfield.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\widgets\window.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\guichan\cliprectangle.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\color.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\defaultfont.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\exception.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\focushandler.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\gfont.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\graphics.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\gui.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\guichan.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\image.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\imagefont.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\key.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\keyinput.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\mouseinput.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\rectangle.cpp
# End Source File
# Begin Source File

SOURCE=.\src\guichan\widget.cpp
# End Source File
# End Group
# Begin Group "map"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\map\map.cpp
# End Source File
# Begin Source File

SOURCE=.\src\map\map_draw.cpp
# End Source File
# Begin Source File

SOURCE=.\src\map\map_fog.cpp
# End Source File
# Begin Source File

SOURCE=.\src\map\map_radar.cpp
# End Source File
# Begin Source File

SOURCE=.\src\map\map_save.cpp
# End Source File
# Begin Source File

SOURCE=.\src\map\map_wall.cpp
# End Source File
# Begin Source File

SOURCE=.\src\map\minimap.cpp
# End Source File
# Begin Source File

SOURCE=.\src\map\script_map.cpp
# End Source File
# Begin Source File

SOURCE=.\src\map\script_tileset.cpp
# End Source File
# Begin Source File

SOURCE=.\src\map\tileset.cpp
# End Source File
# End Group
# Begin Group "network"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\network\commands.cpp
# End Source File
# Begin Source File

SOURCE=.\src\network\lowlevel.cpp
# End Source File
# Begin Source File

SOURCE=.\src\network\master.cpp
# End Source File
# Begin Source File

SOURCE=.\src\network\netconnect.cpp
# End Source File
# Begin Source File

SOURCE=.\src\network\network.cpp
# End Source File
# End Group
# Begin Group "pathfinder"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\pathfinder\astar.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pathfinder\pathfinder.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pathfinder\script_pathfinder.cpp
# End Source File
# End Group
# Begin Group "sound"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\sound\mikmod.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sound\music.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sound\ogg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sound\script_sound.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sound\sound.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sound\sound_id.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sound\sound_server.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sound\unitsound.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sound\wav.cpp
# End Source File
# End Group
# Begin Group "stratagus"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\stratagus\construct.cpp
# End Source File
# Begin Source File

SOURCE=.\src\stratagus\groups.cpp
# End Source File
# Begin Source File

SOURCE=.\src\stratagus\iolib.cpp
# End Source File
# Begin Source File

SOURCE=.\src\stratagus\mainloop.cpp
# End Source File
# Begin Source File

SOURCE=.\src\stratagus\missile.cpp
# End Source File
# Begin Source File

SOURCE=.\src\stratagus\player.cpp
# End Source File
# Begin Source File

SOURCE=.\src\stratagus\script.cpp
# End Source File
# Begin Source File

SOURCE=.\src\stratagus\script_missile.cpp
# End Source File
# Begin Source File

SOURCE=.\src\stratagus\script_player.cpp
# End Source File
# Begin Source File

SOURCE=.\src\stratagus\script_spell.cpp
# End Source File
# Begin Source File

SOURCE=.\src\stratagus\selection.cpp
# End Source File
# Begin Source File

SOURCE=.\src\stratagus\spells.cpp
# End Source File
# Begin Source File

SOURCE=.\src\stratagus\stratagus.cpp
# End Source File
# Begin Source File

SOURCE=.\src\stratagus\translate.cpp
# End Source File
# Begin Source File

SOURCE=.\src\stratagus\util.cpp
# End Source File
# End Group
# Begin Group "tolua"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\tolua\tolua.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tolua\tolua_event.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tolua\tolua_is.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tolua\tolua_map.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tolua\tolua_push.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tolua\tolua_to.cpp
# End Source File
# End Group
# Begin Group "ui"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\ui\botpanel.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ui\button_checks.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ui\icons.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ui\interface.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ui\mainscr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ui\menu_proc.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ui\menus.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ui\mouse.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ui\script_ui.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ui\ui.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ui\widgets.cpp
# End Source File
# End Group
# Begin Group "unit"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\unit\depend.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unit\script_unit.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unit\script_unittype.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unit\unit.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unit\unit_cache.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unit\unit_draw.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unit\unit_find.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unit\unittype.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unit\upgrade.cpp
# End Source File
# End Group
# Begin Group "video"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\video\cursor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\video\font.cpp
# End Source File
# Begin Source File

SOURCE=.\src\video\graphic.cpp
# End Source File
# Begin Source File

SOURCE=.\src\video\linedraw.cpp
# End Source File
# Begin Source File

SOURCE=.\src\video\mng.cpp
# End Source File
# Begin Source File

SOURCE=.\src\video\movie.cpp
# End Source File
# Begin Source File

SOURCE=.\src\video\png.cpp
# End Source File
# Begin Source File

SOURCE=.\src\video\sdl.cpp
# End Source File
# Begin Source File

SOURCE=.\src\video\sprite.cpp
# End Source File
# Begin Source File

SOURCE=.\src\video\video.cpp
# End Source File
# End Group
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "include"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\include\actions.h
# End Source File
# Begin Source File

SOURCE=.\src\include\ai.h
# End Source File
# Begin Source File

SOURCE=.\src\include\animation.h
# End Source File
# Begin Source File

SOURCE=.\src\include\campaign.h
# End Source File
# Begin Source File

SOURCE=.\src\include\commands.h
# End Source File
# Begin Source File

SOURCE=.\src\include\construct.h
# End Source File
# Begin Source File

SOURCE=.\src\include\cursor.h
# End Source File
# Begin Source File

SOURCE=.\src\include\depend.h
# End Source File
# Begin Source File

SOURCE=.\src\include\editor.h
# End Source File
# Begin Source File

SOURCE=.\src\include\font.h
# End Source File
# Begin Source File

SOURCE=.\src\include\icons.h
# End Source File
# Begin Source File

SOURCE=.\src\include\interface.h
# End Source File
# Begin Source File

SOURCE=.\src\include\iocompat.h
# End Source File
# Begin Source File

SOURCE=.\src\include\iolib.h
# End Source File
# Begin Source File

SOURCE=.\src\include\map.h
# End Source File
# Begin Source File

SOURCE=.\src\include\master.h
# End Source File
# Begin Source File

SOURCE=.\src\include\menus.h
# End Source File
# Begin Source File

SOURCE=.\src\include\minimap.h
# End Source File
# Begin Source File

SOURCE=.\src\include\missile.h
# End Source File
# Begin Source File

SOURCE=.\src\include\movie.h
# End Source File
# Begin Source File

SOURCE=.\src\include\myendian.h
# End Source File
# Begin Source File

SOURCE=.\src\include\net_lowlevel.h
# End Source File
# Begin Source File

SOURCE=.\src\include\netconnect.h
# End Source File
# Begin Source File

SOURCE=.\src\include\network.h
# End Source File
# Begin Source File

SOURCE=.\src\include\pathfinder.h
# End Source File
# Begin Source File

SOURCE=.\src\include\player.h
# End Source File
# Begin Source File

SOURCE=.\src\include\script.h
# End Source File
# Begin Source File

SOURCE=.\src\include\script_sound.h
# End Source File
# Begin Source File

SOURCE=.\src\include\settings.h
# End Source File
# Begin Source File

SOURCE=.\src\include\sound.h
# End Source File
# Begin Source File

SOURCE=.\src\include\sound_server.h
# End Source File
# Begin Source File

SOURCE=.\src\include\spells.h
# End Source File
# Begin Source File

SOURCE=.\src\include\stratagus.h
# End Source File
# Begin Source File

SOURCE=.\src\include\tileset.h
# End Source File
# Begin Source File

SOURCE=.\src\include\translate.h
# End Source File
# Begin Source File

SOURCE=.\src\include\trigger.h
# End Source File
# Begin Source File

SOURCE=.\src\include\ui.h
# End Source File
# Begin Source File

SOURCE=.\src\include\unit.h
# End Source File
# Begin Source File

SOURCE=.\src\include\unitsound.h
# End Source File
# Begin Source File

SOURCE=.\src\include\unittype.h
# End Source File
# Begin Source File

SOURCE=.\src\include\upgrade.h
# End Source File
# Begin Source File

SOURCE=.\src\include\upgrade_structs.h
# End Source File
# Begin Source File

SOURCE=.\src\include\util.h
# End Source File
# Begin Source File

SOURCE=.\src\include\video.h
# End Source File
# Begin Source File

SOURCE=.\src\include\wav.h
# End Source File
# Begin Source File

SOURCE=.\src\include\widgets.h
# End Source File
# End Group
# Begin Group "guichan."

# PROP Default_Filter ""
# Begin Group "sdl."

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\guichan\include\guichan\sdl\sdlgraphics.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\sdl\sdlimageloader.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\sdl\sdlinput.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\sdl\sdlpixel.h
# End Source File
# End Group
# Begin Group "widgets."

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\guichan\include\guichan\widgets\button.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\widgets\checkbox.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\widgets\container.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\widgets\dropdown.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\widgets\icon.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\widgets\label.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\widgets\listbox.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\widgets\radiobutton.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\widgets\scrollarea.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\widgets\slider.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\widgets\textbox.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\widgets\textfield.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\widgets\window.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\guichan\include\guichan\actionlistener.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\allegro.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\basiccontainer.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\cliprectangle.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\color.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\defaultfont.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\exception.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\focushandler.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\font.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\graphics.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\gsdl.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\gui.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\image.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\imagefont.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\imageloader.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\input.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\key.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\keyinput.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\keylistener.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\listmodel.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\mouseinput.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\mouselistener.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\opengl.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\platform.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\rectangle.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\widget.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan\x.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\ai\ai_local.h
# End Source File
# Begin Source File

SOURCE=.\src\guichan\include\guichan.h
# End Source File
# Begin Source File

SOURCE=.\src\video\intern_video.h
# End Source File
# Begin Source File

SOURCE=".\src\tolua\tolua++.h"
# End Source File
# Begin Source File

SOURCE=.\src\tolua\tolua_event.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\contrib\stratagus.ico
# End Source File
# Begin Source File

SOURCE=.\src\stratagus.rc
# End Source File
# End Group
# End Target
# End Project
