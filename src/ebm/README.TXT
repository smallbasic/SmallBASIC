SmallBASIC for eBookman version 0.9.0i

http://smallbasic.sourceforge.net

BASIC for the eBookMan

Copy .bas source files onto your machine. Load SmallBASIC and
then click the desired .bas file; the program will be compiled 
and executed.

Once in a running program the following system options are available:
  Break        - stop executing the program and return to the sb prompt.
  Keyboard     - eBookman popup keyboard. Used while executing the
                 input statement.
  Turbo On/Off - Turbo On allows for faster execution at the expense 
                 of greater battery drain.
  Close        - Quit SmallBASIC.

When you return to the file list, you should find a matching .sbx file 
for the previously executed program. This contains compiled SmallBASIC
byte-code allowing you to return to the application without re-compilation.

Be aware that SmallBASIC for the eBookman will place a heavy demand on your
batteries so please use re-chargables ---- REDUCE, RECYCLE AND REUSE ----

Use the forum on http://smallbasic.sourceforge.net for comments or 
suggestions. 

You can also use this site for sharing your SmallBASIC programs.

RELEASE HISTORY:

0.9.0i - 4 July 2004

- Fixed one more problem relating to editing files that had been copied 
  from the desktop. Random chars appearing after the end of the file.
  TapWrite 1.3 will also contain this fix.

- The open/launch page now allows basic source files with any cAsE for the
  file name extension.

- Updates for PEN(0), PEN(3). This should work more or less as expected now.
  You need to call PEN ON for these to have any effect. 

- The point(x,y) function now returns the correct colour. 

Thanks to Snoopy42 for finding and reporting the two above problems.

- Minor updates to osd_events() to help conserve battery power.

- Fixed the display of the "Opening ..." message when opening large 
  text files in the editor.

- Added a new procedure and two new functions:

  image #n, index, x, y  -- Paints an image from an image library
  imagew #n, index       -- Returns the image width
  imageh #n, index       -- Returns the image height

  For the ebookman platform the image library should contain images in the 
  package file format. You can create this file using SnarfWriter as follows: 

  1. Create your program images and save as either JPG, GIF or PNG
     in a clean directory (GIMP is a powerful image editor).

  2. Create a file imagelib.snarf with the following text:
   
     make-index-page=false
     site-name=imagelib
     out-path=
     site=file://./index.html
     read-images=true

  3. Create an index.html file like this:
     
     <img src=pic1.jpg>
     <img src=pic2.jpg>
     <img src=pic3.jpg>

  4. Run SnarfWriter like this:

     java -jar SnarfWriter.jar imagelib.snarf
   
  5. Upload the resulting imagelib.mhtml to your ebookman

  6. Your BASIC program with look something like this:

     open "globe.mhtml" for input as #1
     repeat
        for index = 2 to 11
          image #1, index, 10, 10
        next index
     until 
     close #1
  
  7. Cre8 cools games and share them with the SmallBASIC community :-)


0.9.0h - 30 April 2004

- Fixed the problem with saving files that had been copied from the desktop. 
  This was caused by a bug in the ebjlib 'File' class, also causing the same 
  problem in TapWrite.
  
  More info in case you are interested...
  In the File::Open() method using readMode, the ebo_locate_object() 
  function is called to first determine the file length. This length value 
  was later used as the size argument in the call to ebo_unmap() to close 
  the file. For desktop files, this length value is shorter than the value 
  that ebo_mapin() allocates and returns. This meant that ebo_unmap() was 
  not completely unmapping the file. When the file was later opened for 
  writing, ebo_new() returned the error EBO_DESTROY_IN_USE. 
  The solution was to always unmap using the value returned by mapin();

- Fixed a display problem in the Open file list where sbx/bas files were
  sometimes not lining up correctly.
- Improved Open file list redisplay time (by caching the html text).
- The editor delete key now behaves like a normal delete key and not a 
  backspace key.
- Opening a file in append mode now works (only for text files). This 
  allows you to code something like this:

open "temp.txt" for output as #1
print #1, "hello"
close #1

rem time passes...
open "temp.txt" for append as #1
print #1, "hello again"
close #1

open "temp.txt" for input as #1
while not eof(1)
 input #1; s
 ? s
wend
close #1


0.9.0g - 19 April 2004

- Based on Nicholas' 0.9 series basic kernel
- Implemented fancy new IDE interface incorporating the TapWrite edit function
- ebjlib files are now maintained in the TapWrite source forge cvs 
- linked with libasm for increased performace
- added a html command. This allows you to write a program like this:

s="<font size=2><input type=button value=hello><br>what about that?"
s+="<input type=text name=foo>"
html s,"hello",15,55,190,90
? "you entered "+env("foo")

  Note, the title and window coordinates arguments are optional


0.8.2f - 26 July 2003

- Fixed display problems in the help system

0.8.2e - 16 July 2003

- Added selectable font support
- Improved ANSI driver performance
- New look file open dialog
- Added RS232 Support. Tested with this:

print "Press 'q' for exit..." 
open "com1:57600" AS #2
while !eof(2) 
  ' receive 
  if lof(2) 
    k=input$(1000,2) 
    print k; 
  fi 
  ' send 
  k=inkey$ 
  if k != 0 then
     if k="q" then exit
     print #2, k; 
     print cat(1);k;cat(0); 
  fi 
wend 
close #2

0.8.2c - 2 June 2002

- Added flush image escape to ansi driver \033[F. This causes imgUpdate()
  to be called.
- Sorted file names in File Dialog

0.8.2b - 25 May 2002

- Improved "Run" dialog
- Further improved ANSI text out performance

0.8.2a - 12 May 2002 

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

0.1 - 26 January 2002

- First release


