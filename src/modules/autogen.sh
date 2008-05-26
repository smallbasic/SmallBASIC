rm -rf autom4te.cache aclocal.m4
touch NEWS README AUTHORS ChangeLog
aclocal
autoheader
autoconf
automake -a
libtoolize
