rm -rf autom4te.cache aclocal.m4
mkdir -p m4
touch NEWS README AUTHORS ChangeLog
autoreconf -vfi
