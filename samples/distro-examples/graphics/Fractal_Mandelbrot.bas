'mandel v 2.0
'jfox@airmail.net
CLS
maxcolor=if(bpp>8,30,16)
leftside=-2
top=1.25
xside=2.5
yside=-2.5
xscale=xside/xmax
yscale=yside/ymax
FOR y=1 to ymax/2
	FOR x=1 to xmax
		cx=x*xscale+leftside
		cy=y*yscale+top
		zx=0
		zy=0
		cc=0
		WHILE (zx*zx+zy*zy<4 and cc<maxcolor)
			tempx=zx*zx-zy*zy+cx
			zy=2*zx*zy+cy
			zx=tempx
			cc=cc+1
		WEND
		rc=if(bpp>8,RGBF(cc/30,cc/30,cc/30),cc)
		PSET x,y,rc
		PSET x,ymax-y,rc
	NEXT
NEXT
BEEP

PEN ON
REPEAT:UNTIL PEN(0)
PEN OFF
