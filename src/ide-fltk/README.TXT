SB-IDE using FLTK

Commenced sometime around early Nov 2004. Mostly coded
during my boring train ride to/from work. Initially built
using cygwin under w2k.

Tasks for initial release:

1. editor
  - update display/selection colours
  - Display the row+column number of the cursor    
    TextDisplay.h:
    int position_to_linecol(mCursorPos, int* lineNum, int* column); *done*
  - experiment with alternative caret- used TextDisplay::BLOCK_CURSOR *done*
  - status bar and title heading. include some indication program is 
    running. *done*
  - read-only when program is running but scrollbars still function *done*
  - Grey/disable/handle editor menus when in run mode *done*
  - selectable colors like in the "input" fltk sample
  - toggle menu for turbo mode like in the "menu" fltk sample
  - tabs along top or bottom *done*
  - multiple editor tabs
  - undo (ask FLTK team)
  - full screen menu option
  - startup argument to run in "application" mode
  - right-control deletes selection- modified Fl_compose.cxx

2. run-time
   - Forms commands (input types, checkbox radio, text hyperlink, button)
     partially completed 18/11, more or less completed 20/11
   - Load and display images  - ask FLTK group why no JPG and PNG support 
   - Load images from http server *done* - apart from blocking/lockpup when
     host not found
   - Load .bas from http server
   - html text using fltk help control
   - sound ?
   - updated DRAW code *done*
   - handle "repeat key$ = inkey until len(key$) > 0" w/out high cpu use *done*
   - modules - ensure that the flite speach synth works (under cygwin)
   - update do_cmd_html(), cmd_image to use par_massget

3. Help Tab
   - using FLTK help widget - doesn't appear to be completed. 
     convert ebm help/help widget?

4. Output Text Tab (removed)

5. Program icon in windows

6. About Box
   - what version will this be?

7. Settings dialog
   - setting for turbo mode
   - build with fluid

8. Scrollable output window
   - large fixed size with scroll bars - optional like in sbpad.exe

9. Release packaging. Do we include cygwin1.dll in windows release?
    (mingw does not contain all the api's required for full build).
   - latest FLTK code

10. Linux build

$Id: README.TXT,v 1.6 2004-11-25 11:13:25 zeeb90au Exp $
