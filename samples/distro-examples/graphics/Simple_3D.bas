#!/usr/local/bin/sbasic -g
' 3d.bas
' 07/06/2000

d=100*(ymax/160):t=.7:p=1.3
redraw
repeat
	k=inkey$
	if k="1":t=t+.1:redraw
	elif k="2":t=t-.1:redraw
	elif k="3":p=p+.1:redraw
	elif k="4":p=p-.1:redraw
	elif k="5":d=d+10:redraw
	elif k="6":d=d-10:redraw
	endif
until k="q"
end

'
'	sxediasmos olvn v
'
sub redraw
cls
? "1/2=+-è, 3/4=+-ö, 5/6=+-d, q=quit"

st=sin(t):ct=cos(t)
sp=sin(p):cp=cos(p)

restore cubeData
calcPt
pset sx,sy
for i=1 to 15
	calcPt
	line sx,sy
next
end

'
'	ypologismos sx,sy (3d->2d)
'
sub calcPt
read x,y,z
xe=-x*st+y*ct
ye=-x*ct*cp-y*st*cp+z*sp
ze=-x*sp*ct-y*st*sp-z*cp+4
sx=d*xe/ze+xmax/2
sy=d*ye/ze+ymax/2
end

'
'	o kybos (lineto)
'
label cubeData
1 data 1,1,1
2 data 1,-1,1
3 data -1,-1,1
4 data -1,1,1
5 data 1,1,1
6 data 1,1,-1
7 data 1,-1,-1
8 data -1,-1,-1
9 data -1,1,-1
10 data 1,1,-1
11 data 1,-1,1
12 data 1,-1,-1
13 data -1,-1,1
14 data -1,-1,-1
15 data -1,1,1
16 data -1,1,-1


