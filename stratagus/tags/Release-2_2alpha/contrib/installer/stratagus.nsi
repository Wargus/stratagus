; The name of the installer
Name "Stratagus"

OutFile "stratagus-030311-win32.exe"
Icon "stratagus.ico"

InstallDir $PROGRAMFILES\Stratagus

DirText "Stratagus will be installed to the specified location"

ComponentText "This will install Stratagus. Select what you want installed."
EnabledBitmap bitmap1.bmp
DisabledBitmap bitmap2.bmp
InstType "WC2"
InstType "Base Only"

; Base Files
Section "Base (required)"
  SectionIn RO
  SetOutPath $INSTDIR
  File /r "C:\projects\stratagus-030311\*.*"
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
  CreateDirectory "$SMPROGRAMS\Stratagus"
  StrCmp $1 "1" 0 NoWC2Shortcut
  CreateShortCut "$SMPROGRAMS\Stratagus\Stratagus (WC2).lnk" "$INSTDIR\stratagus.exe" ""
  CreateShortCut "$SMPROGRAMS\Stratagus\Edit build.bat.lnk" "notepad.exe" "$INSTDIR\build.bat" ""
  CreateShortCut "$SMPROGRAMS\Stratagus\Run build.bat.lnk" "$INSTDIR\build.bat" ""
NoWC2Shortcut:
  CreateShortCut "$SMPROGRAMS\Stratagus\Uninstall Stratagus.lnk" "$INSTDIR\uninst.exe" ""
SectionEnd

Uninstalltext "This will uninstall Stratagus."

Section "Uninstall"
	RMDir /r $SMPROGRAMS\Stratagus
	RMDir /r $INSTDIR
SectionEnd


