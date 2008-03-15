#!/usr/local/bin/sbasic -g
dy=ymax/16
for i=0 to 15
	rect 0,i*dy,xmax,i*dy+dy color i filled
	at 0,i*dy:? i;
next
pause

