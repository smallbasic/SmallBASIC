REM $Id: welcome.bas,v 1.3 2005-01-09 00:13:23 zeeb90au Exp $
? "Welcome to SmallBASIC (FLTK) 0.9.6.1"
img= "./Bas-Home/logo.gif"
if (exist(img)) then
    open img as #1
    image #1, 0, 1,25
    close #1
fi
