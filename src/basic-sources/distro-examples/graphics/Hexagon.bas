#!/usr/local/bin/sbasic -g
' hexagon.bas
' 28/05/2000

sf=.95
x=(ymax/2)-10:y=0:cx=xmax/2:cy=ymax/2
c=cos(pi/3):s=sin(pi/3)
c1=cos(pi/36):s1=sin(pi/36)

cls
view xmax/20,xmax/20,xmax/1.2,ymax/1.2,15,2

for j=1 to 30
	sx=x+cx:sy=cy-y
	pset sx,sy
	for i=0 to 6
		sx=x+cx:sy=cy-y
		line sx,sy color i*6
		xn=x*c-y*s
		y=x*s+y*c
		x=xn
	next
	xn=sf*(x*c1-y*s1)
	y=sf*(x*s1+y*c1)
	x=xn
next

