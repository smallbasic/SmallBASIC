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

2. run-time
   - Forms commands (input types, checkbox radio, text hyperlink, button)
     partially completed 18/11
   - Load and display images
   - Load .bas from web server
   - html text using fltk help control
   - sound ?

3. Help Tab
   - using FLTK help widget

4. Output Text Tab

5. Program icon in windows

6. About Box
   - what version will this be?

7. Grey/disable/handle editor menus when in run mode

8. Settings dialog
   - setting for turbo mode
   - build with fluid

9. Scrollable output window
   - large fixed size with scroll bars - optional like in sbpad.exe

10. Release packaging. Do we include cygwin1.dll in windows release?
    (mingw does not contain all the api's required for full build).

11. Linux build
