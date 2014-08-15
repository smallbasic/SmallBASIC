' http://forum.basicprogramming.org/index.php/topic,3408.0.html
' sdl plasma example on: https://gist.github.com/stevenlr/824019
' ported to CMLua by Cybermonkey 08/2014
' ported to OxygenBasic by Peter Wirbelauer  o7.o8.2o14
' ported to X11Basic by vsync o8.o8.2o14

'
' valgrind --tool=callgrind ./src/platform/sdl/sbasicg samples/distro-examples/tests/plasma.bas 
' kcachegrind
'

bw=320
bh=200
DIM c(256)
FOR x=0 TO 255
  r=255-((SIN(PI*2*x/255)+1)*127)
  c(x)= RGB(r/256, (SIN(PI*2*x/127)+1)/4, 1-r/256)
NEXT x
t1=timer
for iter = 1 to 10
  t=TICKS
  FOR y=0 TO bh
    FOR x=0 TO bw
      a=COS(1.2*f)*100-y+100
      b=SIN(0.8*f)*160-x+160
      COLOR ((SIN(x/50+f+y/200)+SIN(SQR(b*b+a*a)/50))/2+1)*127
      pset x,y
    NEXT x
  NEXT y
   fps=TICKSPERSEC/(TICKS-t)
   at bw+10,0
   print format("Fps:  ###.##", fps)
   at bw+10,20
   print format("Cnt:  ###", iter)
   at bw+10,40
   print format("Elap: ###", timer-t1)

  f=f+0.2
  SHOWPAGE
next iter


