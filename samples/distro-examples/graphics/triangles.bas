REM SmallBASIC
REM created: 12/12/2016
const r2=360/3
const r3=r2*2
func mk
  r.x=rnd*1000 mod xmax
  r.y=10+(rnd*1000 mod ymax)
  r.s=(rnd*1000 mod 50)+5
  r.c=rnd*1000 mod 15
  r.d=iff(rnd>.5,1,-1)
  r.ri=(rnd*1000 mod 30)+5
  if rnd>.5 then r.ri=-r.ri
  r.r=0
  mk=r
end
sub d(byref r)
  a1=(r.r)*pi/180
  a2=(r.r+r2)*pi/180
  a3=(r.r+r3)*pi/180
  x1=r.x+r.s*cos(a1)
  y1=r.y+r.s*sin(a1)
  x2=r.x+r.s*cos(a2)
  y2=r.y+r.s*sin(a2)
  x3=r.x+r.s*cos(a3)
  y3=r.y+r.s*sin(a3)
  line x1,y1,x2,y2 COLOR r.c
  line x2,y2,x3,y3 COLOR r.c
  line x3,y3,x1,y1 COLOR r.c
  if (r.s>190) then
   circle r.x,r.y,r.s color r.c
  fi
end
sub go
  dim a
  n=50
  for i = 0 to n
    a << mk
  next i
  while 1
    cls
    for i = 0 to n
      d(a(i))
      a(i).r = (a(i).r + a(i).ri) mod 360
      a(i).s++
      if a(i).s>200 then a(i).s=1
    next i
    showpage
    delay 100
  wend
end
go
