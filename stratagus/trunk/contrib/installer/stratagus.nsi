; The name of the installer
Name "FreeCraft"

OutFile "freecraft-030311-win32.exe"
Icon "freecraft.ico"

InstallDir $PROGRAMFILES\FreeCraft

DirText "FreeCraft will be installed to the specified location"

ComponentText "This will install FreeCraft. Select what you want installed."
EnabledBitmap bitmap1.bmp
DisabledBitmap bitmap2.bmp
InstType "WC2"
InstType "Base Only"

; Base Files
Section "Base (required)"
  SectionIn RO
  SetOutPath $INSTDIR
  File /r "C:\projects\freecraft-030311\*.*"
  WriteUninstaller $INSTDIR\uninst.exe
SectionEnd

Section "Use WC2 Data"
  SectionIn 1
  StrCpy $1 "1"
  SetOutPath $INSTDIR
; Exec "command /c set cdrom=e:"
;  Exec $INSTDIR\build.bat
  MessageBox MB_OK "To use WC2 data:$\n\
First edit build.bat, insert the WC2 CDRom, and then run build.bat."
SectionEnd

Section "Start Menu Shortcuts"
  SectionIn 1
  SectionIn 2
  CreateDirectory "$SMPROGRAMS\FreeCraft"
  StrCmp $1 "1" 0 NoWC2Shortcut
  CreateShortCut "$SMPROGRAMS\FreeCraft\FreeCraft (WC2).lnk" "$INSTDIR\freecraft.exe" ""
  CreateShortCut "$SMPROGRAMS\FreeCraft\Edit build.bat.lnk" "notepad.exe" "$INSTDIR\build.bat" ""
  CreateShortCut "$SMPROGRAMS\FreeCraft\Run build.bat.lnk" "$INSTDIR\build.bat" ""
NoWC2Shortcut:
  CreateShortCut "$SMPROGRAMS\FreeCraft\Uninstall FreeCraft.lnk" "$INSTDIR\uninst.exe" ""
SectionEnd

Uninstalltext "This will uninstall FreeCraft."

Section "Uninstall"
	RMDir /r $SMPROGRAMS\FreeCraft
	RMDir /r $INSTDIR
SectionEnd


