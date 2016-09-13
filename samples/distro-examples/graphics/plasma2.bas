DIM r(256)
DIM g(256)
DIM b(256)
LET f=0
CLS

FOR x=0 TO 255
  LET r(x)=255-CEIL((SIN(PI*2*x/255)+1)*127)
  LET g(x)=CEIL((SIN(PI*2*x/127)+1)*64)
  LET b(x)=255-r(x)
NEXT x

bw=xmax/3
bh=ymax/3
t1=timer
maxFps=0

while 1
  t=TICKS
  FOR y=0 TO bh
   FOR x=0 TO bw
    LET c1=SIN(x/50+f+y/200)
    LET c2=SQR((SIN(0.8*f)*400-x+400)*(SIN(0.8*f)*400-x+400)+(COS(1.2*f)*240-y+240)*(COS(1.2*f)*240-y+240))
    LET c2=SIN(c2/50)
    LET c3=(c1+c2)/2
    LET res=(c3+1)*127

    color RGB(r(res),g(res),b(res))
    pset x,y
  NEXT x
 NEXT y

 fps=1000/(TICKS-t)
 if (fps>maxfps) then
   maxfps = fps
 endif
 iter++
 at bw+10,0
 print format("Fps:  ###.##", fps)
 at bw+10,20
 print format("Best: ###.##", maxfps)
 at bw+10,40
 print format("Cnt:  ###", iter)
 at bw+10,60
 print format("Elap: ###", timer-t1)

 SHOWPAGE
 f += 0.1
wend

