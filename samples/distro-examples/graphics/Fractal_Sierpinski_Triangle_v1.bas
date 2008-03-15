#sec:Main
' sier.bas
' Method 1 to draw Sierpinski Triangle
' Example of chaos in random numbers
' by TimC

randomize(1)
cls
#rect 0,0,160,160 color o filled
x =80
y =0
? cat(3);"Patience...";cat(0)
for j = 1 to 8000 : doit : next j
color(0)
? cat(3);"Done";cat(0)
pause

sub doit
  a = rnd  
  if(a < .333) then
     x = (x/2 ) + 40
     y = y/2 
     c =11
  elseif (a < .666 )then 
     x = x/2 
     y = ((160 - y)/2 ) + y
     c =12
  else  
     x = ((160 - x)/2+x)  
     y = ((160 - y)/2)+y 
     c =1
  endif 
  color c: pset x,y  color c
end