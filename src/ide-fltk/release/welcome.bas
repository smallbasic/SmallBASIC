REM $Id: welcome.bas,v 1.2 2004-12-16 22:15:27 zeeb90au Exp $
? "Welcome to SmallBASIC (FLTK) 0.9.6.0"
img= "./Bas-Home/logo.gif"
if (exist(img)) then
    open img as #1
    image #1, 0, 1,25
    close #1
fi
