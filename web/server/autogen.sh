# This file is part of SmallBASIC
#
# Copyright(C) 2001-2016 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
#

# setup notes:
# touch AUTHORS NEWS README ChangeLog
# cd src
# ln -s ../../../src/common
# ln -s ../../../src/lib
# ln -s ../../../src/languages
# mkdir libs
# cp ~/.m2/repository/io/undertow/undertow-core/1.3.23.Final/undertow-core-1.3.23.Final.jar libs

rm -f acinclude.m4
rm -rf autom4te.cache aclocal.m4
libtoolize
aclocal
autoheader
autoconf
automake -a

