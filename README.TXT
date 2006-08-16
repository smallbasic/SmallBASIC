
===============================================================================
Installation from source for FLTK, N770(GTK) or command line versions

> sh autogen.sh

For FLTK version (see sync/ide-fltk/README.TXT for further details)
> ./configure --enable-fltk
> make -s

For GTK version
> ./configure --enable-gtk --disable-hildon
> make -s

For N770 version
> ./configure --enable-gtk
> make deb -s

For non-graphical console version (cygwin or linux)
> ./configure
> make

===============================================================================

Installation from source step-by-step for other versions:

1. Unpack files to a directory

	for example /usr/src/sbasic

2. Enter to sync directory
	
	# cd /usr/src/sbasic/sync

3. Run build-script
	
	# make menuconfig

4. select "build compiler"

5. select one driver

	SB now its created and installed

	# sbasic

6. optional -> select "build modules"

7. optional -> select "build documents"

	7.1. select "info pages"

		Now, you can browse info pages by 

		# info sbasic

	7.2. select "html pages"

		Now, you can read the documentation from a web browser

	7.3. select "man page"

		Now, you can browse man-page by 

		# man sbasic


===============================================================================
Directories

sync/
The source code directory tree

bin/
Final binaries

ext/ 
Contains extra packages.
Those packages may be required.




