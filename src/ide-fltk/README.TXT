SB-IDE using FLTK

Commenced sometime around early Nov 2004. Mostly coded
during my boring train ride to/from work. Initially built
using cygwin under w2k.

===============================================================================
[Issues following 0.9.6.0 release]

1. File with DOS newlines open with <cr> - should be pre-processed and
   removed (perhaps FLTK enhancement)

2. Indent.bas - handle if/then on same line with chars following "then"

3. Update DRAW command as per instructions by Keijo

4. Linux release:
   o    minor compilation problems. fixed
   o    

5. Apparently runs crappy on Win98 - update the installer to only accept
   Win2K or XP (sorry folks - why don't you try fedora core or slackware)
   
6. Current FLTK build seems to have problems (as at 12/1/2005)

7. Try to use same function keys as sbpad.

8. Help menu commands to launch html help and release notes

9. Font and colour menu commands

10. Toolbar

11. Remove any error in the status bar before running a program.

===============================================================================
[Tasks for 0.9.6.0 release]

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
  - toggle menu for turbo mode like in the "menu" fltk sample *done*
  - tabs along top or bottom *done*
  - multiple editor tabs 
  - undo (ask FLTK team) *done*
  - full screen menu option *done*
  - startup argument to run in "application" mode *done* tabs at the bottom
  - right-control deletes selection- modified Fl_compose.cxx
  - some kind of bookmark system allowing the user to easily return
    to a favourite application. what if the bookmark program was a .bas?
    added capability to do this as a smallbasic program
  - jump to the line number of the error when compilation fails *done*
  - autorun.bas file which launches after 
  - idea from audacity program - scan .bas files in $HOME/Plug-Ins (or such)
    for each file: scan for special comment lines at the start of file, eg
    'editor-plug-in  -or-
    'output-plug-in
    'menu &Edit/&Paste
    ...
    in the menu callback
     - determine which .bas was invoked
     - check if program already running
     - if edit option, set opt_command to the name of the currently edited file
       invoke executor, redraw the edit window
       if output option, normal exec
  - View/Edit Font Size/9-10-11-12
    View/Output Font Size/9-10-11-12

2. run-time
   - Forms commands (input types, checkbox radio, text hyperlink, button)
     partially completed 18/11, more or less completed 20/11
   - Load and display images  - ask FLTK group why no JPG and PNG support 
   - Load images from http server *done* - apart from blocking/lockpup when
     host not found
   - Load .bas from http server - not quite so simple since SB is single 
     threaded and blocking sockets block for a very long time. (in windoze)
   - html text using fltk help control *done* (for html command)
   - sound ?
   - updated DRAW code *done*
   - handle "repeat key$ = inkey until len(key$) > 0" w/out high cpu use *done*
   - modules - ensure that the flite speech synth works (under cygwin)
     runs fine using -m switch but the speech does not run on subsequent invokations
   - update do_cmd_html(), cmd_image to use par_massget
   - scrollable output window - too many problems with input fields

3. Help Tab
   - using FLTK help widget - doesn't appear to be completed (confirmed on 
     FLTK group by matthiasm) - patched/fixed

4. Output Text Tab (removed)

5. Program icon in windows *done* 

6. About Box
   * what version will this be?

7. Settings dialog
   - setting for turbo mode *done*
   - build with fluid

8. Scrollable output window
   - large fixed size with scroll bars - optional like in sbpad.exe

9. Release packaging. Do we include cygwin1.dll in windows release?
    (mingw does not contain all the api's required for full build).
   * latest FLTK code

10. Linux build

===============================================================================
  .,,.·´¯`·.. ¸><((((º>¸.  .·´¯`·.. ¸><((((º>¸.  .·´¯`·.. ¸><((((º>¸.  
.><((((º>`·.¸¸.·´¯`·.¸.·´¯`·...¸><((((º>  .,,.·´¯`·.. ¸><((((º>¸.  
   .....,,.><((((º>`·.¸¸.·´¯`·.¸.·´¯`·...¸><((((º> .·´¯`·.. ¸><((((º>¸.  

$Id: README.TXT,v 1.13 2005-01-20 03:27:40 zeeb90au Exp $

