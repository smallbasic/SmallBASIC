;NSIS Modern User Interface

;--------------------------------
;Include Modern UI
!include "MUI.nsh"

;--------------------------------
;General
  ;Name and file
  Name "SmallBASIC (SDL)"
  OutFile "sbasic_0_9_8.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\SBW32\SDL_0.9.8"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\SmallBASIC\SDL_0.9.8" ""

;--------------------------------
;Interface Settings
  !define MUI_ABORTWARNING

;--------------------------------
;Pages
  !insertmacro MUI_PAGE_LICENSE "..\doc\LICENSE"
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
  CreateDirectory "$SMPROGRAMS\SmallBASIC 0.9.8"
  SetOutPath $INSTDIR
  CreateShortCut "$SMPROGRAMS\SmallBASIC 0.9.8\SmallBASIC.lnk" "$INSTDIR\sbasic.exe" "-q welcome.bas"
  CreateShortCut "$SMPROGRAMS\SmallBASIC 0.9.8\Uninstall.lnk" "$INSTDIR\uninstall.exe"
SectionEnd

Section "Quick Launch Shortcut"
  SetOutPath $INSTDIR
  CreateShortCut "$QUICKLAUNCH\SmallBASIC.lnk" "$INSTDIR\sbasic.exe" "" "$INSTDIR\sbasic.exe" 0
SectionEnd

Section "Desktop Shortcut"
  SetOutPath $INSTDIR
  CreateShortCut "$DESKTOP\SmallBASIC.lnk" "$INSTDIR\sbasic.exe" "" "$INSTDIR\sbasic.exe" 0
SectionEnd

Section "Create .BAS file association"
  WriteRegStr HKCR ".bas" "" "SmallBASIC"
  WriteRegStr HKCR "SmallBASIC" "" "SmallBASIC"
  WriteRegStr HKCR "SmallBASIC\DefaultIcon" "" "$INSTDIR\sbasic.exe,0"
  WriteRegStr HKCR "SmallBASIC\shell" "" "run"
  WriteRegStr HKCR "SmallBASIC\shell\run\command" "" '"$INSTDIR\sbasic.exe" -q "%1"'
SectionEnd

Section "SmallBASIC (SDL)" SecMain
  SetOutPath "$INSTDIR"
  File sbasic.exe
  File welcome.bas
  File SDL.dll
  File SDL_image.dll

  File "..\basic-sources\distro-examples\apps\Calendar.bas"
  File "..\basic-sources\distro-examples\apps\Charmap.bas"
  File "..\basic-sources\distro-examples\apps\Easter_Butcher.bas"
  File "..\basic-sources\distro-examples\apps\Easter_Carter.bas"
  File "..\basic-sources\distro-examples\apps\Eliza.bas"
  File "..\basic-sources\distro-examples\apps\Euro_calculator.bas"
  File "..\basic-sources\distro-examples\apps\Factor.bas"
  File "..\basic-sources\distro-examples\apps\Primes.bas"
  File "..\basic-sources\distro-examples\apps\TinyBASIC.bas"
  File "..\basic-sources\distro-examples\apps\Weekday.bas"
  File "..\basic-sources\distro-examples\apps\palm_cli.bas"
  File "..\basic-sources\distro-examples\graphics\Chaos_1xt.bas"
  File "..\basic-sources\distro-examples\graphics\Chaos_NPhase.bas"
  File "..\basic-sources\distro-examples\graphics\Charts.bas"
  File "..\basic-sources\distro-examples\graphics\Colors.bas"
  File "..\basic-sources\distro-examples\graphics\Fractal_Cloud.bas"
  File "..\basic-sources\distro-examples\graphics\Fractal_Mandelbrot.bas"
  File "..\basic-sources\distro-examples\graphics\Fractal_Sierpinski_Triangle.bas"
  File "..\basic-sources\distro-examples\graphics\Fractal_Sierpinski_Triangle_v1.bas"
  File "..\basic-sources\distro-examples\graphics\Fractal_Tree.bas"
  File "..\basic-sources\distro-examples\graphics\Hexagon.bas"
  File "..\basic-sources\distro-examples\graphics\Simple_3D.bas"
  File "..\basic-sources\distro-examples\graphics\Sin-Cos-Tan.bas"
  File "..\basic-sources\distro-examples\graphics\Sketch.bas"
  File "..\basic-sources\distro-examples\graphics\Window_and_View.bas"
  File "..\basic-sources\distro-examples\graphics\clock.bas"

  ;Store installation folder
  WriteRegStr HKCU "Software\SmallBASIC\SDL_0.9.8" "" $INSTDIR
  
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
  DeleteRegKey /ifempty HKCU "Software\SmallBASIC\SDL_0.9.8"

  ; Remove files and uninstaller
  Delete $INSTDIR\sbasic.exe
  Delete $INSTDIR\*.bas

  Delete $INSTDIR\SDL.dll
  Delete $INSTDIR\SDL_image.dll

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\SmallBASIC 0.9.8\*.*"
  Delete "$QUICKLAUNCH\SmallBASIC.lnk"
  Delete "$DESKTOP\SmallBASIC.lnk"

  ; Remove directories used
  RMDir "$SMPROGRAMS\SmallBASIC 0.9.8"
  RMDir "$INSTDIR"

SectionEnd

