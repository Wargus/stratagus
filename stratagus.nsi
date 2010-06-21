;--------------------------------

Name "Stratagus"
OutFile "stratagus-install.exe"
Icon "contrib/stratagus.ico"
BrandingText " "

!ifdef AMD64
InstallDir $PROGRAMFILES64\Stratagus
!else
InstallDir $PROGRAMFILES\Stratagus
!endif

;--------------------------------

!ifdef AMD64

Function .onInit

	System::Call "kernel32::GetCurrentProcess() i .s"
	System::Call "kernel32::IsWow64Process(i s, *i .r0)"

	IntCmp $0 0 0 end

	MessageBox MB_OK|MB_ICONSTOP "This version is for 64 bits computers only."
	Abort

end:

FunctionEnd

!endif

Function Uninstall

	ReadRegStr $R0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Stratagus" "InstallLocation"
	StrCmp $R0 "" end
	DetailPrint "Removing previous installation"
	ExecWait "$R0\uninstall.exe /S _?=$R0"
	RMDir /r $R0

end:

FunctionEnd

;--------------------------------

Page directory
Page instfiles

;--------------------------------

Section ""

	Call Uninstall

	SetOutPath $INSTDIR
	File "stratagus.exe"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Stratagus" "DisplayName" "Stratagus"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Stratagus" "UninstallString" "$\"$INSTDIR\uninstall.exe$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Stratagus" "QuietUninstallString" "$\"$INSTDIR\uninstall.exe$\" /S"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Stratagus" "InstallLocation" "$INSTDIR"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Stratagus" "DisplayIcon" "$\"$INSTDIR\stratagus.exe$\",0"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Stratagus" "NoModify" 1
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Stratagus" "NoRepair" 1
	WriteUninstaller $INSTDIR\uninstall.exe

SectionEnd

;--------------------------------

Uninstalltext "This will uninstall Stratagus."

Section "Uninstall"

	RMDir /r $INSTDIR
	DeleteRegKey /ifempty HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Stratagus"

SectionEnd

