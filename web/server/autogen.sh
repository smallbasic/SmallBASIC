# This file is part of SmallBASIC
#
# Copyright(C) 2001-2016 Chris Warren-Smith.
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
#

rm -f acinclude.m4
rm -rf autom4te.cache aclocal.m4

ln -s ../../AUTHORS
ln -s ../../NEWS
ln -s ../../README
ln -s ../../ChangeLog

libtoolize
aclocal
autoheader
autoconf
automake -a

