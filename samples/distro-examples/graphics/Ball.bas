#!/usr/local/bin/sbasic -g
' ball.bas
' 29/05/2000

color 0,15
cls
randomize ticks
dx=1:dy=-1
x=rnd*120+8:y=rnd*120+8
ox=x:oy=y
rect 0,0,159,139
at 0,150:? "Press any key to exit...";
repeat
	rect ox-2,oy-2,ox+2,oy+2 color 15
	rect x-2,y-2,x+2,y+2 filled
	if y>135 or y<5
		dy=-dy
		sound 990,20
	fi
	if x>155 or x<5
		dx=-dx
		sound 990,20
	fi
	ox=x:oy=y
	x=x+dx
	y=y+dy
until (inkey$ <> "")
end

