;NSIS Modern User Interface
;Basic Example Script
;Written by Joost Verburg

;--------------------------------
;Include Modern UI
!include "MUI.nsh"

;--------------------------------
;General
  ;Name and file
  Name "SmallBASIC (FLTK/Cygwin)"
  OutFile "sbasic.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\SBW32\FLTK"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\SmallBASIC\FLTK" ""

;--------------------------------
;Interface Settings
  !define MUI_ABORTWARNING

;--------------------------------
;Pages
  !insertmacro MUI_PAGE_LICENSE "..\..\doc\LICENSE"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"
  CreateDirectory "$SMPROGRAMS\Example2"
  CreateShortCut "$SMPROGRAMS\Example2\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\Example2\Example2 (MakeNSISW).lnk" "$INSTDIR\makensisw.exe" "" "$INSTDIR\makensisw.exe" 0
SectionEnd

Section "SmallBASIC (FLTK/Cygwin)" SecMain
  SetOutPath "$INSTDIR"
  File sbfltk.exe
  File cygwin1.dll
  File readme.html
  
  ;Store installation folder
  WriteRegStr HKCU "Software\SmallBASIC\FLTK" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

;--------------------------------
;Descriptions
  ;Language strings
  LangString DESC_SecMain ${LANG_ENGLISH} "Program executable and required DLL file."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecMain} $(DESC_SecMain)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"
  ; Remove registry keys
  DeleteRegKey /ifempty HKCU "Software\SmallBASIC\FLTK"

  ; Remove files and uninstaller
  Delete $INSTDIR\sbfltk.exe
  Delete $INSTDIR\cygwin1.dll
  Delete $INSTDIR\readme.html
  Delete "$INSTDIR\Uninstall.exe"

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Example2\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\Example2"
  RMDir "$INSTDIR"


SectionEnd

