#sec:Main
' sier2.bs
' From "Fractals" by Hans Lauwerier
' Heavily modified for SB :-)
' Method 2 to draw Sierpinski Triangle

cls
c=3
rect 0,0,160,160 color o filled
p=5: dim T(p): a= SQR(3)
for m=0 to p
  for n=0 to 3^m-1
    n1=n
    for l = 0 to m-1
      t(l) = n1 mod 3 : n1=n1\3: next l
    x=0: y=0
    for k=0 to m-1
      x = x+cos((4*t(k)+1)*PI/6)/2^k
      y= y+sin((4*t(k)+1)*PI/6)/2^k
    next k
    u1 =45*(x+a/2^(m+1))+80
    u2=45*(x-a/2^(m+1))+80
    v1=45*(y-1/2^(m+1))+100
    v2=45*(y+1/2^m)+100
    x = 45*x+80
    y = 45*y+100
    line u1, v1, x, v2 COLOR c
    line x,v2, u2, v1 COLOR c
    line u2,v1,u1,v1 COLOR c
    next n
    c=c+2
  next m
  beep
  pause