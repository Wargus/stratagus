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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "src\include" /I "src\movie\vp31\include" /I "include" /D "NDEBUG" /D "USE_WIN32" /D "MAP_REGIONS" /D "USE_SDL" /D "USE_LIBCDA" /D "USE_ZLIB" /D "USE_BZ2LIB" /D "USE_MIKMOD" /D "USE_OGG" /D "USE_MAD" /D "USE_FLAC" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib winmm.lib opengl32.lib /nologo /stack:0x2000000 /subsystem:windows /profile /machine:I386

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "src\include" /I "src\movie\vp31\include" /I "include" /D "_DEBUG" /D "DEBUG" /D "USE_WIN32" /D "MAP_REGIONS" /D "USE_SDL" /D "USE_LIBCDA" /D "USE_ZLIB" /D "USE_BZ2LIB" /D "USE_MIKMOD" /D "USE_OGG" /D "USE_MAD" /D "USE_FLAC" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib winmm.lib opengl32.lib /nologo /stack:0x2000000 /subsystem:windows /debug /machine:I386
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

SOURCE=.\src\action\action_attack.c
# End Source File
# Begin Source File

SOURCE=.\src\action\action_board.c
# End Source File
# Begin Source File

SOURCE=.\src\action\action_build.c
# End Source File
# Begin Source File

SOURCE=.\src\action\action_die.c
# End Source File
# Begin Source File

SOURCE=.\src\action\action_follow.c
# End Source File
# Begin Source File

SOURCE=.\src\action\action_move.c
# End Source File
# Begin Source File

SOURCE=.\src\action\action_patrol.c
# End Source File
# Begin Source File

SOURCE=.\src\action\action_repair.c
# End Source File
# Begin Source File

SOURCE=.\src\action\action_research.c
# End Source File
# Begin Source File

SOURCE=.\src\action\action_resource.c
# End Source File
# Begin Source File

SOURCE=.\src\action\action_returngoods.c
# End Source File
# Begin Source File

SOURCE=.\src\action\action_spellcast.c
# End Source File
# Begin Source File

SOURCE=.\src\action\action_stand.c
# End Source File
# Begin Source File

SOURCE=.\src\action\action_still.c
# End Source File
# Begin Source File

SOURCE=.\src\action\action_train.c
# End Source File
# Begin Source File

SOURCE=.\src\action\action_unload.c
# End Source File
# Begin Source File

SOURCE=.\src\action\action_upgradeto.c
# End Source File
# Begin Source File

SOURCE=.\src\action\actions.c
# End Source File
# Begin Source File

SOURCE=.\src\action\command.c
# End Source File
# End Group
# Begin Group "ai"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\ai\ai.c
# End Source File
# Begin Source File

SOURCE=.\src\ai\ai_building.c
# End Source File
# Begin Source File

SOURCE=.\src\ai\ai_force.c
# End Source File
# Begin Source File

SOURCE=.\src\ai\ai_magic.c
# End Source File
# Begin Source File

SOURCE=.\src\ai\ai_plan.c
# End Source File
# Begin Source File

SOURCE=.\src\ai\ai_resource.c
# End Source File
# Begin Source File

SOURCE=.\src\ai\script_ai.c
# End Source File
# End Group
# Begin Group "editor"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\editor\editloop.c
# End Source File
# Begin Source File

SOURCE=.\src\editor\editor.c
# End Source File
# Begin Source File

SOURCE=.\src\editor\edmap.c
# End Source File
# Begin Source File

SOURCE=.\src\editor\script_editor.c
# End Source File
# End Group
# Begin Group "game"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\game\campaign.c
# End Source File
# Begin Source File

SOURCE=.\src\game\game.c
# End Source File
# Begin Source File

SOURCE=.\src\game\intro.c
# End Source File
# Begin Source File

SOURCE=.\src\game\loadgame.c
# End Source File
# Begin Source File

SOURCE=.\src\game\savegame.c
# End Source File
# Begin Source File

SOURCE=.\src\game\trigger.c
# End Source File
# End Group
# Begin Group "map"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\map\map.c
# End Source File
# Begin Source File

SOURCE=.\src\map\map_draw.c
# End Source File
# Begin Source File

SOURCE=.\src\map\map_fog.c
# End Source File
# Begin Source File

SOURCE=.\src\map\map_rock.c
# End Source File
# Begin Source File

SOURCE=.\src\map\map_save.c
# End Source File
# Begin Source File

SOURCE=.\src\map\map_wall.c
# End Source File
# Begin Source File

SOURCE=.\src\map\map_wood.c
# End Source File
# Begin Source File

SOURCE=.\src\map\minimap.c
# End Source File
# Begin Source File

SOURCE=.\src\map\script_map.c
# End Source File
# Begin Source File

SOURCE=.\src\map\script_tileset.c
# End Source File
# Begin Source File

SOURCE=.\src\map\tileset.c
# End Source File
# End Group
# Begin Group "missile"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\missile\missile.c
# End Source File
# Begin Source File

SOURCE=.\src\missile\script_missile.c
# End Source File
# End Group
# Begin Group "movie"

# PROP Default_Filter ""
# Begin Group "vp31"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\movie\vp31\BlockMapping.c
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\DCT_decode.c
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\dct_globals.c
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\DDecode.c
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\DFrameR.c
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\DSystemDependant.c
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\FrameIni.c
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\Frarray.c
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\Huffman.c
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\IDctPart.c
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\pb_globals.c
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\postproc.c
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\Quantize.c
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\Reconstruct.c
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\unpack.c
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\vfwPback.c
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\vfwpbdll_if.c
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\YUVtofromRGB.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\movie\avi.c
# End Source File
# Begin Source File

SOURCE=.\src\movie\movie.c
# End Source File
# End Group
# Begin Group "network"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\network\commands.c
# End Source File
# Begin Source File

SOURCE=.\src\network\lowlevel.c
# End Source File
# Begin Source File

SOURCE=.\src\network\master.c
# End Source File
# Begin Source File

SOURCE=.\src\network\netconnect.c
# End Source File
# Begin Source File

SOURCE=.\src\network\network.c
# End Source File
# End Group
# Begin Group "pathfinder"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\pathfinder\astar.c
# End Source File
# Begin Source File

SOURCE=.\src\pathfinder\pathfinder.c
# End Source File
# Begin Source File

SOURCE=.\src\pathfinder\script_pathfinder.c
# End Source File
# Begin Source File

SOURCE=.\src\pathfinder\splitter.c
# End Source File
# Begin Source File

SOURCE=.\src\pathfinder\splitter_debug.c
# End Source File
# Begin Source File

SOURCE=.\src\pathfinder\splitter_lowlevel.c
# End Source File
# Begin Source File

SOURCE=.\src\pathfinder\splitter_zoneset.c
# End Source File
# End Group
# Begin Group "sound"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\sound\cdaudio.c
# End Source File
# Begin Source File

SOURCE=.\src\sound\cdda.c
# End Source File
# Begin Source File

SOURCE=.\src\sound\flac.c
# End Source File
# Begin Source File

SOURCE=.\src\sound\libcda.c
# End Source File
# Begin Source File

SOURCE=.\src\sound\mad.c
# End Source File
# Begin Source File

SOURCE=.\src\sound\mikmod.c
# End Source File
# Begin Source File

SOURCE=.\src\sound\music.c
# End Source File
# Begin Source File

SOURCE=.\src\sound\ogg.c
# End Source File
# Begin Source File

SOURCE=.\src\sound\script_sound.c
# End Source File
# Begin Source File

SOURCE=.\src\sound\sdl_audio.c
# End Source File
# Begin Source File

SOURCE=.\src\sound\sound.c
# End Source File
# Begin Source File

SOURCE=.\src\sound\sound_id.c
# End Source File
# Begin Source File

SOURCE=.\src\sound\sound_server.c
# End Source File
# Begin Source File

SOURCE=.\src\sound\unitsound.c
# End Source File
# Begin Source File

SOURCE=.\src\sound\wav.c
# End Source File
# End Group
# Begin Group "stratagus"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\stratagus\construct.c
# End Source File
# Begin Source File

SOURCE=.\src\stratagus\groups.c
# End Source File
# Begin Source File

SOURCE=.\src\stratagus\iolib.c
# End Source File
# Begin Source File

SOURCE=.\src\stratagus\mainloop.c
# End Source File
# Begin Source File

SOURCE=.\src\stratagus\player.c
# End Source File
# Begin Source File

SOURCE=.\src\stratagus\pud.c
# End Source File
# Begin Source File

SOURCE=.\src\stratagus\script.c
# End Source File
# Begin Source File

SOURCE=.\src\stratagus\script_player.c
# End Source File
# Begin Source File

SOURCE=.\src\stratagus\script_spell.c
# End Source File
# Begin Source File

SOURCE=.\src\stratagus\selection.c
# End Source File
# Begin Source File

SOURCE=.\src\stratagus\spells.c
# End Source File
# Begin Source File

SOURCE=.\src\stratagus\stratagus.c
# End Source File
# End Group
# Begin Group "ui"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\ui\botpanel.c
# End Source File
# Begin Source File

SOURCE=.\src\ui\button_checks.c
# End Source File
# Begin Source File

SOURCE=.\src\ui\icons.c
# End Source File
# Begin Source File

SOURCE=.\src\ui\interface.c
# End Source File
# Begin Source File

SOURCE=.\src\ui\mainscr.c
# End Source File
# Begin Source File

SOURCE=.\src\ui\menu_proc.c
# End Source File
# Begin Source File

SOURCE=.\src\ui\menus.c
# End Source File
# Begin Source File

SOURCE=.\src\ui\mouse.c
# End Source File
# Begin Source File

SOURCE=.\src\ui\script_ui.c
# End Source File
# Begin Source File

SOURCE=.\src\ui\ui.c
# End Source File
# End Group
# Begin Group "unit"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\unit\depend.c
# End Source File
# Begin Source File

SOURCE=.\src\unit\script_unit.c
# End Source File
# Begin Source File

SOURCE=.\src\unit\script_unittype.c
# End Source File
# Begin Source File

SOURCE=.\src\unit\unit.c
# End Source File
# Begin Source File

SOURCE=.\src\unit\unit_cache.c
# End Source File
# Begin Source File

SOURCE=.\src\unit\unit_draw.c
# End Source File
# Begin Source File

SOURCE=.\src\unit\unit_find.c
# End Source File
# Begin Source File

SOURCE=.\src\unit\unittype.c
# End Source File
# Begin Source File

SOURCE=.\src\unit\upgrade.c
# End Source File
# End Group
# Begin Group "video"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\video\cursor.c
# End Source File
# Begin Source File

SOURCE=.\src\video\font.c
# End Source File
# Begin Source File

SOURCE=.\src\video\graphic.c
# End Source File
# Begin Source File

SOURCE=.\src\video\linedraw.c
# End Source File
# Begin Source File

SOURCE=.\src\video\png.c
# End Source File
# Begin Source File

SOURCE=.\src\video\sdl.c
# End Source File
# Begin Source File

SOURCE=.\src\video\sprite.c
# End Source File
# Begin Source File

SOURCE=.\src\video\video.c
# End Source File
# End Group
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "include"

# PROP Default_Filter ""
# Begin Group "vp31."

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\movie\vp31\include\BlockMapping.h
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\include\codec_common.h
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\include\codec_common_interface.h
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\include\compdll.h
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\include\dct.h
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\include\Huffman.h
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\include\HuffTables.h
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\include\machine.h
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\include\mcomp.h
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\include\pbdll.h
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\include\PreprocIf.h
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\include\Quantize.h
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\include\rawTypes.h
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\include\Reconstruct.h
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\include\type_aliases.h
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\include\vfw_PB_Interface.h
# End Source File
# Begin Source File

SOURCE=.\src\movie\vp31\include\YUVtofromRGB.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\include\actions.h
# End Source File
# Begin Source File

SOURCE=.\src\include\ai.h
# End Source File
# Begin Source File

SOURCE=.\src\include\avi.h
# End Source File
# Begin Source File

SOURCE=.\src\include\campaign.h
# End Source File
# Begin Source File

SOURCE=.\src\include\cdaudio.h
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

SOURCE=.\src\include\libcda.h
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

SOURCE=.\src\include\pud.h
# End Source File
# Begin Source File

SOURCE=.\src\include\rdtsc.h
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

SOURCE=.\src\include\sound_id.h
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

SOURCE=.\src\include\video.h
# End Source File
# Begin Source File

SOURCE=.\src\include\wav.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\ai\ai_local.h
# End Source File
# Begin Source File

SOURCE=.\src\video\intern_video.h
# End Source File
# Begin Source File

SOURCE=.\src\pathfinder\splitter.h
# End Source File
# Begin Source File

SOURCE=.\src\pathfinder\splitter_local.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\src\stratagus.ico
# End Source File
# Begin Source File

SOURCE=.\src\stratagus.rc
# End Source File
# End Group
# Begin Source File

SOURCE=.\lib\zlib.lib
# End Source File
# Begin Source File

SOURCE=.\lib\ogg_static.lib
# End Source File
# Begin Source File

SOURCE=.\lib\SDL.lib
# End Source File
# Begin Source File

SOURCE=.\lib\SDLmain.lib
# End Source File
# Begin Source File

SOURCE=.\lib\vorbis_static.lib
# End Source File
# Begin Source File

SOURCE=.\lib\vorbisfile_static.lib
# End Source File
# Begin Source File

SOURCE=.\lib\libpng.lib
# End Source File
# Begin Source File

SOURCE=.\lib\libmad.lib
# End Source File
# Begin Source File

SOURCE=.\lib\libFLAC.lib
# End Source File
# Begin Source File

SOURCE=.\lib\libbz2.lib
# End Source File
# Begin Source File

SOURCE=.\lib\lua.lib
# End Source File
# Begin Source File

SOURCE=.\lib\mikmod.lib
# End Source File
# End Target
# End Project
