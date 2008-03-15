#sec:Main
' cloud.bas
' From "Fractals" by Hans Lauwerier
' Shows Orbits of a dynamical system

cls
a = 3.5: b=-3: x=3.21: y=6.54
sub1

for n=0 to 10000 : pset x,y
  z=x: x=y+w
  sub1
  y=w-z
next n
beep: pause

sub sub1
if x>1 then w=a*x+b*(x-1): exit sub
if x<-1 then w=a*x+b*(x+1):  exit sub
w = a*x: exit sub: end
end sub

