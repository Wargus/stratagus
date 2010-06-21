;--------------------------------

Name "Stratagus"
OutFile "stratagus-install.exe"
Icon "contrib/stratagus.ico"
InstallDir $PROGRAMFILES\Stratagus
BrandingText " "

;--------------------------------

Function .onInit
 
	ReadRegStr $R0 HKLM "Software\Stratagus" "InstallDir"
	StrCmp $R0 "" done
		 
	MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION "Stratagus is already installed. $\n$\nClick `OK` to remove the previous version or `Cancel` to cancel this upgrade." IDOK uninstall
	Abort

uninstall:

	ClearErrors
	ExecWait "$R0\uninstall.exe _?=$R0"
	RMDir /r $INSTDIR

done:

FunctionEnd

;--------------------------------

Page directory
Page instfiles

;--------------------------------

Section ""

	SetOutPath $INSTDIR
	File "stratagus.exe"
	WriteRegStr HKLM "Software\Stratagus" "InstallDir" $INSTDIR
	WriteUninstaller $INSTDIR\uninstall.exe

SectionEnd

;--------------------------------

Section "Uninstall"

	RMDir /r $INSTDIR
	DeleteRegKey /ifempty HKLM "Software\Stratagus"

SectionEnd

