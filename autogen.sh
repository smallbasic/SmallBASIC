# This file is part of SmallBASIC
#
# Copyright(C) 2001-2012 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
#

pkg-config --version > /dev/null || echo "Please install pkg-config"

rm -f acinclude.m4

if [ "x$1" != "x--enable-sdl" ]; then
  echo "AC_DEFUN([AM_PATH_SDL])" > acinclude.m4
fi

rm -rf autom4te.cache aclocal.m4
aclocal
autoheader
autoconf
automake -a

