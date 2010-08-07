;
;    stratagus.nsi - Windows NSIS Installer for Stratagus
;    Copyright (C) 2010  Pali Rohár <pali.rohar@gmail.com>
;
;    This program is free software: you can redistribute it and/or modify
;    it under the terms of the GNU General Public License as published by
;    the Free Software Foundation, either version 3 of the License, or
;    (at your option) any later version.
;
;    This program is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.
;
;    You should have received a copy of the GNU General Public License
;    along with this program.  If not, see <http://www.gnu.org/licenses/>.
;
;

!include "MUI.nsh"

;--------------------------------

!define NAME "Stratagus"
!define VERSION "2.2.5.2"
!define HOMEPAGE "https://launchpad.net/stratagus"
!define LICENSE "GPL v2"
!define COPYRIGHT "Copyright (c) 1998-2010 by The Stratagus Project and Pali Rohar"

;--------------------------------

!define ICON "contrib/stratagus.ico"
!define STRATAGUS "stratagus.exe"
!define UNINSTALL "uninstall.exe"
!define INSTALLER "${NAME}-${VERSION}.exe"
!define INSTALLDIR "$PROGRAMFILES\${NAME}\"
!define LANGUAGE "English"

!ifdef AMD64
!undef INSTALLER
!define INSTALLER "${NAME}-${VERSION}-x86_64.exe"
!undef INSTALLDIR
!define INSTALLDIR "$PROGRAMFILES64\${NAME}\"
!undef NAME
!define NAME "Stratagus (64 bit)"
!endif

!define REGKEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}"

;--------------------------------

LangString REMOVEPREVIOUS ${LANG_ENGLISH} "Removing previous installation"
LangString REMOVECONFIGURATION ${LANG_ENGLISH} "Removing configuration files:"
LangString DESC_REMOVEEXE ${LANG_ENGLISH} "Remove ${NAME} executable"
LangString DESC_REMOVECONF ${LANG_ENGLISH} "Remove all other configuration files and directories in ${NAME} install directory created by user or ${NAME}"

!ifdef AMD64
LangString AMD64ONLY ${LANG_ENGLISH} "This version is for 64 bits computers only."
!endif

;--------------------------------

!define MUI_ICON "${ICON}"
!define MUI_UNICON "${ICON}"

!define MUI_ABORTWARNING
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_UNFINISHPAGE_NOAUTOCLOSE

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "COPYING"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_COMPONENTS
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "${LANGUAGE}"

;--------------------------------

Name "${NAME}"
Icon "${ICON}"
OutFile "${INSTALLER}"
InstallDir "${INSTALLDIR}"
InstallDirRegKey HKLM "${REGKEY}" "InstallLocation"

VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "${NAME} Installer - Strategy Gaming Engine"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${VERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "InternalName" "${NAME} Installer"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "${COPYRIGHT}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "License" "${LICENSE}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "Homepage" "${HOMEPAGE}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "OriginalFilename" "${INSTALLER}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "${NAME} Installer"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "${VERSION}"
VIProductVersion "${VERSION}"

BrandingText "${NAME} - ${VERSION}  ${HOMEPAGE}"
ShowInstDetails Show
ShowUnInstDetails Show
XPStyle on
RequestExecutionLevel admin

;--------------------------------

!ifdef AMD64

Function .onInit

	System::Call "kernel32::GetCurrentProcess() i .s"
	System::Call "kernel32::IsWow64Process(i s, *i .r0)"
	IntCmp $0 0 0 0 end

	MessageBox MB_OK|MB_ICONSTOP "$(AMD64ONLY)"
	Abort

end:

FunctionEnd

!endif

;--------------------------------

Section "-${NAME}" UninstallPrevious

	SectionIn RO

	ReadRegStr $R0 HKLM "${REGKEY}" "InstallLocation"
	StrCmp $R0 "" end

	DetailPrint "$(REMOVEPREVIOUS)"
	SetDetailsPrint none
	ExecWait "$R0\${UNINSTALL} /S _?=$R0"
	Delete "$R0\${UNINSTALL}"
	RMDir $R0
	SetDetailsPrint lastused

end:

SectionEnd

Section "${NAME}"

	SectionIn RO

	SetOutPath $INSTDIR
	File "${STRATAGUS}"
	WriteRegStr HKLM "${REGKEY}" "DisplayName" "${NAME}"
	WriteRegStr HKLM "${REGKEY}" "UninstallString" "$\"$INSTDIR\${UNINSTALL}$\""
	WriteRegStr HKLM "${REGKEY}" "QuietUninstallString" "$\"$INSTDIR\${UNINSTALL}$\" /S"
	WriteRegStr HKLM "${REGKEY}" "InstallLocation" "$INSTDIR"
	WriteRegStr HKLM "${REGKEY}" "DisplayIcon" "$\"$INSTDIR\${STRATAGUS}$\",0"
	WriteRegStr HKLM "${REGKEY}" "DisplayVersion" "${VERSION}"
	WriteRegStr HKLM "${REGKEY}" "HelpLink" "${HOMEPAGE}"
	WriteRegStr HKLM "${REGKEY}" "URLUpdateInfo" "${HOMEPAGE}"
	WriteRegStr HKLM "${REGKEY}" "URLInfoAbout" "${HOMEPAGE}"
	WriteRegDWORD HKLM "${REGKEY}" "NoModify" 1
	WriteRegDWORD HKLM "${REGKEY}" "NoRepair" 1
	WriteUninstaller "$INSTDIR\${UNINSTALL}"

SectionEnd

;--------------------------------

Section "un.${NAME}" Executable

	SectionIn RO

	Delete "$INSTDIR\${STRATAGUS}"
	Delete "$INSTDIR\${UNINSTALL}"
	RMDir "$INSTDIR"
	DeleteRegKey /ifempty HKLM "${REGKEY}"

SectionEnd

Section /o "un.Configuration" Configuration

	DetailPrint "$(REMOVECONFIGURATION)"
	RMDir /r "$INSTDIR"

SectionEnd

!insertmacro MUI_UNFUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${Executable} $(DESC_REMOVEEXE)
!insertmacro MUI_DESCRIPTION_TEXT ${Configuration} $(DESC_REMOVECONF)
!insertmacro MUI_UNFUNCTION_DESCRIPTION_END

;--------------------------------

