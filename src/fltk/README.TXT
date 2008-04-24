Copyright (c) 2002-2008 Chris Warren-Smith

1.  About 

    Commenced sometime around early Nov 2004. Gave me something
    to do during my boring train ride to/from work.


2.  FLTK patches

    5. scandir.c line 31
    # if defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
    
    6. rgbImage.cxx
    uncomment the write_jpeg code block
    #undef INT32
    extern "C" {
    #include <stdio.h>
    #include "../images/libjpeg/jpeglib.h"
    }
    variable data is now "pixels_"
    comment out line begining with "if ( depth!=3 )" 

    then in jconfig.h
    #if defined(WIN32) || defined(__EMX__) || defined(__MINGW32__)
    typedef unsigned char boolean;
    #endif


3.  How to build under Linux
    
    Install FLTK, see www.fltk.org.
    Update the FLTK sources as per step 2.

    Build FLTK libraries:
    ./configure
    make
    sudo make install
    
    cd to sb root directory
 
    ./configure --enable-fltk
    sudo make install


4.  How to build under Windows (W2K/XP)
   
    Install mingw+msys tools (www.mingw.org)
     - use the perl from cygwin by creating the file
       c:\MinGW\msys\1.0\bin\perl
       with this line:
       /c/cygwin/bin/perl.exe $*
       The cygwin perl knows about unix syle paths whereas
       the ActiveState perl uses drive lettered paths

    refer to "How to build under Linux"

7.  Release tags:

    SB_FLTK_0_9_7 (5 Feb 2006)
    SB_FLTK_0_9_6_4 (2 Sept 2005)
    SB_FLTK_0_9_6_1 (16 May 2005)


8. Indent settings:

-bap 
-br 
-brs
-cdb
-cdw 
-ce 
-cli0
-fca
-i2
-l95
-lc95
-lp
-nbbo
-nbc 
-ncs 
-nip 
-npcs 
-npsl 
-nut
-ppi0
-sc 


=======

Future ideas:

- Add/fix ability for applications to set the window dimensions
- Add button color support
- Update form handling to be like SDL implementation
- Fix up fltk AnsiWidget.saveImage()

- Integrate online help with web site help info
- Add break point, variable inspection, step/resume options.
 

$Id: README.TXT,v 1.23 2006/10/24 21:07:56 zeeb90au Exp $

