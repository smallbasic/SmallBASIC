Copyright (c) 2002-2008 Chris Warren-Smith

0. TODO:

- Fix up fltk AnsiWidget.saveImage()
- Update imagedraw handling to be like SDL implementation
- Integrate online help with web site help info + add keyword index page
- Add support for hashmap types perhaps using gnu clib search.h
- Add break point, variable inspection, step/resume options.

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

3.  How to build under Windows (W2K/XP)
   
    Install mingw+msys tools (www.mingw.org)
     - use the perl from cygwin by creating the file
       c:\MinGW\msys\1.0\bin\perl
       with this line:
       /c/cygwin/bin/perl.exe $*
       The cygwin perl knows about unix syle paths whereas
       the ActiveState perl uses drive lettered paths

    refer to "How to build under Linux"

4. Release tags:

    SB_FLTK_0_9_7 (5 Feb 2006)
    SB_FLTK_0_9_6_4 (2 Sept 2005)
    SB_FLTK_0_9_6_1 (16 May 2005)

$Id: README.TXT,v 1.23 2006/10/24 21:07:56 zeeb90au Exp $

