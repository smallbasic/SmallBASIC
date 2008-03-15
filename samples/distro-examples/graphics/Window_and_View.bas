#!/usr/local/bin/sbasic -g
' hexagon.bas
' 28/05/2000

segs=10
prts=6

dim pt(segs-1,(prts+2)*2)

sf=.95
x=80-10:y=0:cx=80:cy=80
c=cos(pi/2):s=sin(pi/2)
c1=cos(pi/(prts*6)):s1=sin(pi/(prts*6))

for j=0 to segs-1
	sx=x+cx:sy=cy-y

	pt(j,0) = sx
	pt(j,1) = sy

	for i=0 to prts
		sx=x+cx:sy=cy-y

		pt(j,i*2+2)   = sx
		pt(j,i*2+3) = sy

		xn=x*c-y*s
		y=x*s+y*c
		x=xn
	next
	xn=sf*(x*c1-y*s1)
	y=sf*(x*s1+y*c1)
	x=xn
next

x1=ymax/8:y1=ymax/8
x2=ymax*3/4:y2=ymax*3/4
wx1=0:wy1=0
wx2=160:wy2=160
repeat
	cls
	view
	? "VIEW/WINDOW demo"
	? "Keys: ,.-=vbnm"
	view x1,y1,x2,y2,15,2
	window wx1,wy1,wx2,wy2

	for j=0 to segs-1
		pset pt(j,0), pt(j,1)
		for i=0 to prts
			line pt(j,i*2+2), pt(j,i*2+3) color i
		next
	next

	repeat:k=inkey$:until k<>""
	
	if k=","
		x1=x1-1
		x2=x2-1
	elif k="."
		x1=x1+1
		x2=x2+1
	elif k="="
		wx2=wx2+1
		wy2=wy2+1
	elif k="-"
		wx2=wx2-1
		wy2=wy2-1
	elif k="n"
		wx1=wx1+1
		wx2=wx2+1
	elif k="v"
		wy1=wy1-1
		wy2=wy2-1
	elif k="m"
		wx1=wx1-1
		wx2=wx2-1
	elif k="b"
		wy1=wy1+1
		wy2=wy2+1
	fi

until k="q"
end


