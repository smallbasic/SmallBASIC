' draw an analogical-clock
' inachus@freemail.gr
color 0, 15
cls

' center
cx = xmax/2
cy = (ymax-txth("Q"))/2

' default radius
r  = cy * 0.9
er = cy * 1

w2 = txtw("99")
tr = r * 0.9
ir = tr - w2/2
hr = ir * 0.6
mr = ir * 0.7
sr = ir * 0.8

' draw frame
circle cx, cy, er color 7 filled
circle cx, cy, er color 0
circle cx, cy,  r color 15 filled
circle cx, cy,  r color 0

' draw hours
color 14, 4
for i=1 to 12
	s  = ltrim(str(i))
	sw = txtw(s): sh = txth(s)

	sa = i * (2*PI)/12
	dx = tr * sin(sa)
	dy = tr * cos(sa)
	x = cx+dx: y = cy-dy
	circle x, y, w2 color 4 filled
'	circle x, y, w2 color 0
	at x-sw/2, y-sh/2
	print s;
next

' display loop
while 1
	start = timer
	timehms start, hour, mint, ssec

    hours = hour+(mint/60)
    if ( hours > 12 ) then hours -= 12

    ha  = hours * (2*PI)/12
    ma  = mint  * (2*PI)/60
    sa  = ssec  * (2*PI)/60

	draw_arrow sa, sr, 8
	draw_arrow ma, mr, 4
	draw_arrow ha, hr, 2

	color 0, 15
	at 0, ymax-txth("Q")
	print time;

	if ucase(inkey)="Q" then exit
	
	while start - timer = 0: wend

	draw_arrow sa, sr, 15
	draw_arrow ma, mr, 15
	draw_arrow ha, hr, 15
wend
end

sub draw_arrow(a, r, c)
color c
dx = r * cos(a-pi/2)
dy = r * sin(a-pi/2)
line cx, cy, cx+dx, cy+dy
end

