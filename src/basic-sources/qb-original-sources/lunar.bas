30  print tab(30); "LUNAR LANDER"
31  print
50  print tab(30); "by Dave Ahl"
35  print chr$(7)
40  for i=1 to 10: print: next i
100 input "Do you want instructions",a$
110 if (left$(a$,1)<>"Y") and (left$(a$, 1) <> "y") then 390
160 print
200 print"You are landing on the moon and and have taken over manual"
210 print"control 1000 feet above a good landing spot. You have a down-"
220 print"ward velocity of 50 feet/sec. 150 units of fuel remain."
225 print
230 print"Here are the rules that govern your APOLLO space-craft:": print
240 print"(1) After each second the height, velocity, and remaining fuel"
250 print"    will be reported via DIGBY your on-board computer."
260 print"(2) After the report a '?' will appear. Enter the number"
270 print"    of units of fuel you wish to burn during the next"
280 print"    second. Each unit of fuel will slow your descent by"
290 print"    1 foot/sec."
310 print"(3) The maximum thrust of your engine is 30 feet/sec/sec"
320 print"    or 30 units of fuel per second."
330 print"(4) When you contact the lunar surface. your descent engine"
340 print"    will automatically shut down and you will be given a"
350 print"    report of your landing speed and remaining fuel."
360 print"(5) If you run out of fuel the '?' will no longer appear"
370 print"    but your second by second report will continue until"
380 print"    you contact the lunar surface.":print
390 print"Beginning landing procedure..........":print
400 print"DIGBY WISHES YOU GOOD LUCK !!!!!!!"
420 print:print
430 print"SEC  FEET      SPEED     FUEL     PLOT OF DISTANCE"
450 print
455 t=0:h=1000:v=50:f=150
490 print t;tab(6);h;tab(16);v;tab(26);f;tab(35);"I";tab(h/15);"*"
500 input b
510 if b<0 then 650
520 if b>30 then b=30
530 if b>f then b=f
540 v1=v-b+5
560 f=f-b
570 h=h-.5*(v+v1)
580 if h<=0 then 670
590 t=t+1
600 v=v1
610 if f>0 then 490
615 if b=0 then 640
620 print"**** OUT OF FUEL ****":print chr$(7)+chr$(7)+chr$(7)+chr$(7)+chr$(7)
640 print t;tab(6);h;tab(16);v;tab(26);f;tab(35);"I";tab(h/15);"*"
650 b=0
660 goto 540
670 print"***** CONTACT *****"
680 h=h+ .5*(v1+v)
690 if b=5 then 720
700 d=(-v+sqr(v*v+h*(10-2*b)))/(5-b)
710 goto 730
720 d=h/v
730 v1=v+(5-b)*d
760 print"Touchdown at";t+d;"seconds."
770 print"Landing velocity=";v1;"feet/sec."
780 print f;"units of fuel remaining."
790 if v1<>0 then 810
800 print"Congratulations! A perfect landing!!"
805 print"Your license will be renewed.............later."
810 if abs(v1)<2 then 840
815 if v1>50 then print "You totalled an entire mountain !!!!!":goto 830
816 if v1>30 and v1<50 then print "Your ship is a wreck !!!!!":goto 830
817 if v1>10 and v1<30 then print "You blasted a huge crater !!!!!":goto 830
818 if v1>5 and v1<10 then print "Your ship is a heap of junk !!!!!":goto 830
820 print"You blew it!!!!!!"
830 print"Your family will be notified..............by post."
840 print:print:print
850 input "Another mission", a$
860 if (left$(a$,1)<>"N") and (left$(a$, 1) <> "n") then 390
870 print:print"Control out.":print
900 end
