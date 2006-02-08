
# see README.TXT or the discussion forum for these details
touch NEWS README AUTHORS ChangeLog
rm -rf autom4te.cache aclocal.m4

aclocal
autoheader
autoconf
automake -a

