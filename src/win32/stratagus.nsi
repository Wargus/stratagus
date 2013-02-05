;       _________ __                 __
;      /   _____//  |_____________ _/  |______     ____  __ __  ______
;      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
;      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
;     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
;             \/                  \/          \//_____/            \/
;  ______________________                           ______________________
;                        T H E   W A R   B E G I N S
;         Stratagus - A free fantasy real time strategy game engine
;
;    stratagus.nsi - Windows NSIS Installer for Stratagus
;    Copyright (C) 2010-2012  Pali Rohár <pali.rohar@gmail.com>
;
;    This program is free software: you can redistribute it and/or modify
;    it under the terms of the GNU General Public License as published by
;    the Free Software Foundation, either version 2 of the License, or
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

;--------------------------------

!ifdef QUIET
!verbose 2
!endif

!define redefine "!insertmacro redefine"
!macro redefine symbol value
!undef ${symbol}
!define ${symbol} "${value}"
!macroend

!include "MUI2.nsh"
!include "FileFunc.nsh"

;--------------------------------

!define NAME "Stratagus"
!define DESCRIPTION "Strategy Gaming Engine"
!define HOMEPAGE "https://launchpad.net/stratagus"
!define LICENSE "GPL v2"
!define COPYRIGHT "Copyright (c) 1998-2012 by The Stratagus Project"

;--------------------------------

!define ICON "stratagus.ico"
!define EXE "stratagus.exe"
!define UNINSTALL "uninstall.exe"
!define INSTALLER "${NAME}-${VERSION}.exe"
!define INSTALLDIR "$PROGRAMFILES\${NAME}\"
!define LANGUAGE "English"

!ifdef DBG

${redefine} EXE "stratagus-dbg.exe"
!ifdef x86_64
${redefine} INSTALLER "${NAME}-${VERSION}-debug-x86_64.exe"
${redefine} INSTALLDIR "$PROGRAMFILES64\${NAME}-dbg\"
${redefine} NAME "Stratagus (debug 64 bit)"
!else
${redefine} INSTALLER "${NAME}-${VERSION}-debug.exe"
${redefine} INSTALLDIR "$PROGRAMFILES\${NAME}-dbg\"
${redefine} NAME "Stratagus (debug)"
!endif

!else

!ifdef x86_64
${redefine} INSTALLER "${NAME}-${VERSION}-x86_64.exe"
${redefine} INSTALLDIR "$PROGRAMFILES64\${NAME}\"
${redefine} NAME "Stratagus (64 bit)"
!endif

!endif

!define REGKEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}"

;--------------------------------

LangString INSTALLER_RUNNING ${LANG_ENGLISH} "${NAME} Installer is already running"
LangString GAMES_AVAILABLE ${LANG_ENGLISH} "Some ${NAME} Games are installed and available on system.$\nFirst uninstall all ${NAME} Games, then ${NAME}"
LangString REMOVEPREVIOUS ${LANG_ENGLISH} "Removing previous installation"
LangString REMOVECONFIGURATION ${LANG_ENGLISH} "Removing configuration files:"
LangString DESC_REMOVEEXE ${LANG_ENGLISH} "Remove ${NAME} executable"
LangString DESC_REMOVECONF ${LANG_ENGLISH} "Remove all other configuration files and directories in ${NAME} install directory created by user or ${NAME}"

!ifdef x86_64
LangString x86_64_ONLY ${LANG_ENGLISH} "This version is for 64 bits computers only"
!endif

;--------------------------------

SetCompressor lzma

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

VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "${NAME} Installer - ${DESCRIPTION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${VERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "InternalName" "${NAME} Installer"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "${COPYRIGHT}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "License" "${LICENSE}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "Homepage" "${HOMEPAGE}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "OriginalFilename" "${INSTALLER}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "${NAME} Installer"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "${VERSION}"
VIProductVersion "${VIVERSION}"

BrandingText "${NAME} - ${DESCRIPTION}, version ${VERSION}  ${HOMEPAGE}"
ShowInstDetails Show
ShowUnInstDetails Show
XPStyle on
RequestExecutionLevel admin

;--------------------------------

Function .onInit

	System::Call 'kernel32::CreateMutexA(i 0, i 0, t "${NAME}") i .r1 ?e'
	Pop $0
	StrCmp $0 0 +3

	MessageBox MB_OK|MB_ICONEXCLAMATION "$(INSTALLER_RUNNING)"
	Abort

!ifdef x86_64

	System::Call "kernel32::GetCurrentProcess() i .s"
	System::Call "kernel32::IsWow64Process(i s, *i .r0)"
	IntCmp $0 0 0 0 +3

	MessageBox MB_OK|MB_ICONSTOP "$(x86_64_ONLY)"
	Abort

!endif

FunctionEnd

;--------------------------------

Function un.onInit

	ClearErrors
	${GetParameters} $0
	${GetOptions} "$0" "/P" $1
	IfErrors 0 +6

	ClearErrors
	EnumRegValue $0 HKLM "${REGKEY}\Games" 0
	IfErrors +3

	MessageBox MB_OK|MB_ICONSTOP "$(GAMES_AVAILABLE)"
	Abort

FunctionEnd

;--------------------------------

Section "-${NAME}" UninstallPrevious

	SectionIn RO

	ReadRegStr $0 HKLM "${REGKEY}" "InstallLocation"
	StrCmp $0 "" +7

	DetailPrint "$(REMOVEPREVIOUS)"
	SetDetailsPrint none
	ExecWait "$0\${UNINSTALL} /S /P _?=$0"
	Delete "$0\${UNINSTALL}"
	RMDir $0
	SetDetailsPrint lastused

SectionEnd

Section "${NAME}"

	SectionIn RO

	SetOutPath $INSTDIR
	File "${EXE}"
	WriteRegStr HKLM "${REGKEY}" "DisplayName" "${NAME}"
	WriteRegStr HKLM "${REGKEY}" "UninstallString" "$\"$INSTDIR\${UNINSTALL}$\""
	WriteRegStr HKLM "${REGKEY}" "QuietUninstallString" "$\"$INSTDIR\${UNINSTALL}$\" /S"
	WriteRegStr HKLM "${REGKEY}" "InstallLocation" "$INSTDIR"
	WriteRegStr HKLM "${REGKEY}" "DisplayIcon" "$\"$INSTDIR\${EXE}$\",0"
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

	Delete "$INSTDIR\${EXE}"
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

!ifdef UPX

!ifndef UPX_FLAGS
!define UPX_FLAGS "-9"
!else
${redefine} UPX_FLAGS "${UPX_FLAGS} -9"
!endif

!ifdef QUIET
${redefine} UPX_FLAGS "${UPX_FLAGS} -q"
!endif

!packhdr "exehead.tmp" "${UPX} ${UPX_FLAGS} exehead.tmp"

!endif

;!finalize "gpg --armor --sign --detach-sig %1"

;--------------------------------
