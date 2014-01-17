#!/usr/local/bin/sbasic -g
' ball.bas
' 29/05/2000
color 0,15
cls
randomize ticks
dx=-1:dy=1
x=rnd*120+8:y=rnd*120+8
ox=x:oy=y
rect 0,0,159,139
at 0,150:? "Press any key to exit...";
repeat
  c = iff(c==14,1,c+1)
  rect ox-7,oy-7,ox+7,oy+7 color 15
  rect x-3,y-3,x+3,y+3 color c filled
  if y>131 or y<9
    dy=-dy
    sound 990,20
  fi
  if x>151 or x<9
    dx=-dx
    sound 990,20
  fi
  ox=x:oy=y
  x=x+dx
  y=y+dy
  delay 10
until (inkey$ <> "")
end
