# Microsoft Developer Studio Project File - Name="metaserver" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=metaserver - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "metaserver.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "metaserver.mak" CFG="metaserver - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "metaserver - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "metaserver - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "metaserver - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "metaserver___Win32_Release"
# PROP BASE Intermediate_Dir "metaserver___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "metaserver___Win32_Release"
# PROP Intermediate_Dir "metaserver___Win32_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "src\include" /I "include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "USE_WIN32" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SDL.lib SDLmain.lib /nologo /subsystem:console /machine:I386 /libpath:"lib"

!ELSEIF  "$(CFG)" == "metaserver - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "metaserver___Win32_Debug"
# PROP BASE Intermediate_Dir "metaserver___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "metaserver___Win32_Debug"
# PROP Intermediate_Dir "metaserver___Win32_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ  /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "src\include" /I "include" /D "_DEBUG" /D "DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "USE_WIN32" /FR /YX /FD /GZ  /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib SDL.lib SDLmain.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"lib"

!ENDIF 

# Begin Target

# Name "metaserver - Win32 Release"
# Name "metaserver - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "src"

# PROP Default_Filter ""
# Begin Group "metaserver"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\metaserver\cmd.c
# End Source File
# Begin Source File

SOURCE=.\src\metaserver\main.c
# End Source File
# Begin Source File

SOURCE=.\src\metaserver\netdriver.c
# End Source File
# Begin Source File

SOURCE=.\src\metaserver\query.c
# End Source File
# End Group
# Begin Group "network"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\network\lowlevel.c
# End Source File
# End Group
# Begin Group "stratagus"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\stratagus\util.c
# End Source File
# End Group
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "include"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\include\net_lowlevel.h
# End Source File
# Begin Source File

SOURCE=.\src\include\stratagus.h
# End Source File
# Begin Source File

SOURCE=.\src\include\util.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\metaserver\cmd.h
# End Source File
# Begin Source File

SOURCE=.\src\metaserver\netdriver.h
# End Source File
# Begin Source File

SOURCE=.\src\metaserver\query.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
