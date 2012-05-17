pkg-config --version > /dev/null || echo "Please install pkg-config"
echo "Delete acinclude.m4 to build SDL"
echo "AC_DEFUN([AM_PATH_SDL])" > acinclude.m4

rm -rf autom4te.cache aclocal.m4
aclocal
autoheader
autoconf
automake -a

