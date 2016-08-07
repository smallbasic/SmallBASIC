# This file is part of SmallBASIC
#
# Copyright(C) 2001-2016 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
#

# setup notes:
# sudo apt-get install libtool
# touch AUTHORS NEWS README ChangeLog
# cd src
# ln -s ../../../src/common
# ln -s ../../../src/lib
# ln -s ../../../src/languages
# mkdir libs
# cp ~/.m2/repository/... libs (see jars in run.sh)

rm -f acinclude.m4
rm -rf autom4te.cache aclocal.m4
libtoolize
aclocal
autoheader
autoconf
automake -a
