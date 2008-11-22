;
; FLTK windows NSIS installer script
;

;--------------------------------
;Include Modern UI
!include "MUI.nsh"

;--------------------------------
;General
  ;Name and file
  Name "SmallBASIC"
  OutFile "sbasic_0.10.2.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\SBW32\FLTK_0.10.2"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\SmallBASIC\FLTK_0.10.2" ""

;--------------------------------
;Interface Settings
  !define MUI_ABORTWARNING

;--------------------------------
;Pages
  !insertmacro MUI_PAGE_LICENSE "..\..\documentation\LICENSE"
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
Section "Start Menu Shortcut"
  CreateDirectory "$SMPROGRAMS\SmallBASIC 0.10.2"
  SetOutPath $INSTDIR
  CreateShortCut "$SMPROGRAMS\SmallBASIC 0.10.2\SmallBASIC.lnk" "$INSTDIR\sbasici.exe"
  CreateShortCut "$SMPROGRAMS\SmallBASIC 0.10.2\Uninstall.lnk" "$INSTDIR\uninstall.exe"
SectionEnd

Section "Quick Launch Shortcut"
  SetOutPath $INSTDIR
  CreateShortCut "$QUICKLAUNCH\SmallBASIC.lnk" "$INSTDIR\sbasici.exe" "" "$INSTDIR\sbasici.exe" 0
SectionEnd

Section "Desktop Shortcut"
  SetOutPath $INSTDIR
  CreateShortCut "$DESKTOP\SmallBASIC.lnk" "$INSTDIR\sbasici.exe" "" "$INSTDIR\sbasici.exe" 0
SectionEnd

Section "Create .BAS file association"
  WriteRegStr HKCR ".bas" "" "SmallBASIC"
  WriteRegStr HKCR "SmallBASIC" "" "SmallBASIC"
  WriteRegStr HKCR "SmallBASIC\DefaultIcon" "" "$INSTDIR\sbasici.exe,0"
  WriteRegStr HKCR "SmallBASIC\shell" "" "open"
  WriteRegStr HKCR "SmallBASIC\shell\open\command" "" '"$INSTDIR\sbasici.exe" -e "%1"'
  WriteRegStr HKCR "SmallBASIC\shell" "" "run"
  WriteRegStr HKCR "SmallBASIC\shell\run\command" "" '"$INSTDIR\sbasici.exe" -r "%1"'
SectionEnd

Section "SmallBASIC" SecMain
  SetOutPath "$INSTDIR"
  File sbasici.exe
  File ..\..\documentation\sbasic_ref.csv
  SetOutPath $INSTDIR\plugins
  File "..\..\plugins\*.*"

  ;Store installation folder
  WriteRegStr HKCU "Software\SmallBASIC\FLTK_0.10.2" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

;--------------------------------
;Descriptions
  ;Language strings
  LangString DESC_SecMain ${LANG_ENGLISH} "Program executable."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecMain} $(DESC_SecMain)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"
  ; Remove registry keys
  DeleteRegKey /ifempty HKCU "Software\SmallBASIC\FLTK_0.10.2"

  ; Remove files and uninstaller
  Delete $INSTDIR\sbasici.exe
  Delete $INSTDIR\Uninstall.exe
  Delete $INSTDIR\sbasic_ref.csv
  Delete $INSTDIR\plugins\*.*

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\SmallBASIC 0.10.2\*.*"
  Delete "$QUICKLAUNCH\SmallBASIC.lnk"
  Delete "$DESKTOP\SmallBASIC.lnk"

  ; Remove directories used
  RMDir "$SMPROGRAMS\SmallBASIC 0.10.2"
  RMDir "$INSTDIR\plugins"
  RMDir "$INSTDIR"

SectionEnd

