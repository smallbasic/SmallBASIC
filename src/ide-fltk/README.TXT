Copyright (c) 2002-2006 Chris Warren-Smith

1.  About 

    Commenced sometime around early Nov 2004. Gave me something
    to do during my boring train ride to/from work.


2.  FLTK patches

    1. compose.cxx: disabled right-ctrl from deleting the current selection
    by commenting the following lines
    // See if they type the compose prefix key:
    //   if (i == RightCtrlKey || i == 0xff20/* Multi-Key */) {
    //     compose_state = 1;
    //     return true;
    //   }
   
    2. scrollrect.cxx: 
    in is_visible() - comments suggest the test should be for NOT-unicode, 
    since this would be the case with Win98:
    
    if (!has_unicode()) {
    
    4. filename.h line 52
    #elif defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
    
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

    7. for linux build, update clip.cxx
    add if (xftc) before call to XftDrawSetClip(xftc, r);


3.  How to build under Linux
    
    Install FLTK - www.fltk.org.
    Update the FLTK sources as per step 2.
    cd sbrs3/sync/ide-fltk
    make linux -s


4.  How to build under Windows (W2K/XP)
   
    Install mingw/msys tools
    Install FLTK 
    Update the FLTK sources as per step 2.
    cd sbrs3/sync/ide-fltk
    make -s


5.  Generate the help files

    Build the command line version of SmallBASIC:
    make fb;make install
    Run the following command from within the directory containing
    the sbfltk executable.
    mkdir help
    cd help
    cp ../../doc/guide.texi .
    cp ../../doc/texi2html.bas .
    ./texi2html guide.texi


6.  Generate the keyword expansion file

    Run the following command from within the above 'help' directory:
    sbasic.exe -pkw | sort | uniq  > keywords.txt


7.  Release tags:

    SB_FLTK_0_9_7 (5 Feb 2006)
    SB_FLTK_0_9_6_4 (2 Sept 2005)
    SB_FLTK_0_9_6_1 (16 May 2005)


$Id: README.TXT,v 1.20 2006-02-04 20:50:24 zeeb90au Exp $

