SmallBASIC
Current version: 0.8.2b

Platform:   PalmOS 3.1+
            Unix (console, framebuffer, svgalib, sdl, X)
            Win32 (GUI, console)
            DOS (djgpp)
            eBookman (2.0+)

http://smallbasic.sourceforge.net

DEVELOPERS:
--------------------------------------------------------------------------
Nicholas Christopoulos      wired_gr@yahoo.com
Gary A. Clark               clarkg@fireserve.net
Bob Reiss                   reissb@usa.net
Chris Warren-Smith          cwarrens@twpo.com.au

--------------------------------------------------------------------------
CVS FILES

AnsiWindow.cpp
AnsiWindow.h
Dialog.cpp
Dialog.h
ebjlib.cpp
ebjlib.h
ebm.h
ebm_fs.cpp
ebm_main.cpp
ebm_main.h
ebm_mem.cpp
ebm_osd.cpp
FileDlg.cpp
FileDlg.h
Form.cpp
Form.h
HTMLWindow.cpp
HTMLWindow.h
initial.mom
input.cpp
libraries.link
Makefile
os.def
os.link
piezo.cpp
resource.cpp
resource.h
resource.rcp
SmallBASIC.atts
SmallBASIC_icon.bmp
standalone.link

Note that files that are part of the EbjLib library may
not be the latest available versions. 

--------------------------------------------------------------------------
RELEASE HISTORY:
--------------------------------------------------------------------------

2 June 2002 Release 0.8.2c

- Added flush image escape to ansi driver \033[F. This causes imgUpdate()
  to be called.
- Sorted file names in File Dialog

--------------------------------------------------------------------------

25 May 2002 Release 0.8.2b

- Improved "Run" dialog
- Further improved ANSI text out performance

--------------------------------------------------------------------------

12 May 2002 Release 0.2 (0.8.2)    [0.8.2a]

- New SB commands - BPUTC, BGETC
- Implemented FILES command - dev_create_file_list()
- Implemented COPY command - dev_fcopy()
- Implemented ENVIRONMENT command.
- Implemented FRE command - returns ebm memory and battery info (see help)
- Optimised TLOAD command. 
- Provided mechanism for sbasic programmers to provide an about box.
- Selecting the menu while running an application now pauses program
  execution.
- Corrected behaviour with open file errors; now results in a 
  runtime error.
- Absolutely way cool on-line help!
- Removed the 2 pixel window border.
- Partial MMC file support; reading and listing files from sb code only. 
  MMC write is not yet implemented in the operating system.
- Updated command line editor. Entered commands are now sent directly
  to the interpreter. You can tap and run simple one line programs 
  using the keyboard. Use : to separate program statements.
- optional SmallBASIC.ini settings control creation of .sbx files 
  and enabling verbose compilation. settings are:
  genexe=0  
  quiet=0
- minor enhancements to ebjlib library
- Optimised ansi text out and window scrolling performance
- Experimental new forms mode commands - BUTTON, TEXT and 
  DOFORM (not yet working correctly)

--------------------------------------------------------------------------
26 January 2002 - Australia Day

Release 0.1 for eBookman - By Chris Warren-Smith

The first non c/c++ programming environment for the Franklin eBookman
is finally here!

This release is based on SmallBASIC 0.8.0.

Synchronise .bas source files onto your machine. Load SmallBASIC and
then choose File/List. Click the desired .bas file and the program 
will be compiled and executed.

Once in a running program the following system options are available:
  Break        - stop executing the program and return to the sb prompt.
  Keyboard     - eBookman popup keyboard. Used while executing the
                 input statement.
  Turbo On/Off - Turbo On allows for faster execution at the expense 
                 of greater battery drain.
  Close        - Quit SmallBASIC.

When you return to the file list, you should find a matching .sbx file 
for the previously executed program. This contains compiled SmallBASIC
byte codes allowing you to return to the application without re-compilation.

Note some of the file system verbs are not yet fully implemented. Refer to
ebm_fs.cpp in the source release.

Be aware that SmallBASIC for the eBookman will place a heavy demand on your
batteries so please use re-chargables - reduce, recycle and reuse. 

Use the forum on http://smallbasic.sourceforge.net for comments or suggestions. 
You can also use this site for sharing your SmallBASIC programs.


