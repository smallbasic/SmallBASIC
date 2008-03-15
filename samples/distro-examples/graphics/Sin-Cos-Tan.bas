#sec:Main
' sin-cos-tan.bas
' Based on another demo file

cls
at 0,ymax/2+txth("Q")
color 1: ? "sin(x)":
color 8: ? "cos(x)":
color 12: ? "tan(x)"
line 0,ymax/2,xmax,ymax/2
for i=0 to xmax
pset i,ymax/2-sin(i*2*pi/ymax)*ymax/4  color 1
pset i,ymax/2-cos(i*2*pi/ymax)*ymax/4 color 8
pset i,ymax/2-tan(i*2*pi/ymax)*ymax/4 color 12
next

