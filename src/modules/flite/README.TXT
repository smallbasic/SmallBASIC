[ Flite is a small, fast run-time synthesis engine, designed for
  embedded systems and servers.  Flite was developed by Alan W Black and
  Kevin Lenzo at Carnegie Mellon University, in Pittsburgh.  The
  city with TTS at its heart. ]

The SB-flite module was build using flite-1.2-release-February-2003
along with cygwin 1.5.10

How to build:

 o  Build sb for cygwin. It should also work fine under linux, I just
    haven't test this yet.

 o  Download and install the flite package from:
    http://www.speech.cs.cmu.edu/flite/

 o  Follow the readme instructions to build the package. Check to ensure
    the build was successful by running flite.exe and flite_time.exe

 o  Update the TOP variable in the SB-flite module Makefile to point to 
    where you installed flite.

 o  Build sb-flite using make cygwin and then make install. On my machine
    the DLL is 4604928 bytes.

 o  Test using sbasic.exe -q -m /usr/local/lib/sbasic/modules/ test.bas

See the flite README file for comment about Voice quality (or lack thereof)

Enjoy!
Chris
