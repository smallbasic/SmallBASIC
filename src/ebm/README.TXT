SmallBASIC for eBookman version 0.9.0g

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

0.9.0g - xx April 2004

- Based on Nicholas' 0.9 series basic kernel
- Implemented fancy new IDE interface incorporating the TapWrite edit function

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


