REM $Id: welcome.bas,v 1.10 2006-01-19 05:38:42 zeeb90au Exp $
? "Welcome to SmallBASIC (FLTK) "+SBVER
img= env("Bas_Home")+"logo.gif"
if (exist(img)) then
  open img for input as #1
  image #1, 0, 1,-50, 100,100
  close #1
fi
