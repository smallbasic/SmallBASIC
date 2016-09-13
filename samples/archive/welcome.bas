REM $Id: welcome.bas,v 1.11 2006-02-04 12:41:31 zeeb90au Exp $

img= env("Bas_Home")+"logo.gif"
if (!exist(img)) then
  ? "Welcome to SmallBASIC (FLTK) "+SBVER  
else
  open img for input as #1
  w = imagew(#1,0)
  h = imageh(#1,0)
  
  randomize
  while 1
    x = rnd*100000%(xmax-w)
    y = rnd*100000%(ymax-h)
    image #1,0, x,y,0,0,w,h
    delay 500
    rect x,y,x+w,y+h,15 filled
  wend
  close #1
fi
