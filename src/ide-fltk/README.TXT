SB-IDE using FLTK

Commenced sometime around early Nov 2004. Mostly coded
during my boring train ride to/from work. Initially built
using cygwin under w2k.

TODO:

1. editor
  - update display/selection colours
  - Display the row+column number of the cursor    
    TextDisplay.h:
    int position_to_linecol(mCursorPos, int* lineNum, int* column);
  - experiment with alternative caret
  - status bar and title heading. include some indication program is 
    running.
  - read-only when program is running but scrollbars still function
  - Grey/disable/handle editor menus when in run mode

2. run-time
   - Forms commands (input types, checkbox radio, text hyperlink, button)
     partially completed 18/11, more or less completed 20/11
   - Load and display images
   - Load .bas from web server
   - html text using fltk help control
   - sound ?

3. Help Tab
   - using FLTK help widget

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

10. Linux build

$Id: README.TXT,v 1.3 2004-11-21 22:38:23 zeeb90au Exp $
