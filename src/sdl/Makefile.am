# SmallBASIC for SDL
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
# 

# Not yet completed. This file needs to be integrated with configure.in
# Here are some links to information about using autotools with SDL:
# http://david.acz.org/sdl.html
# http://www-128.ibm.com/developerworks/linux/library/l-pirates2/

#INCLUDES = 

bin_PROGRAMS = sbasic

bin_PROGRAMS : ../../libsb_common.a

sbasic_SOURCES =                            \
    dev_sdl.c                               


sbasic_LDADD = @PACKAGE_LIBS@ -L$(top_srcdir)/sync -lsb_common
