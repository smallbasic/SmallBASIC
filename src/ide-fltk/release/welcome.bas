REM $Id: welcome.bas,v 1.9 2005-08-22 04:54:50 zeeb90au Exp $
? "Welcome to SmallBASIC (FLTK) 0.9.6.4"
img= env("Bas_Home")+"logo.gif"
if (exist(img)) then
  open img for input as #1
  image #1, 0, 1,25
  close #1
fi
