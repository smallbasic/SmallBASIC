5 REM done by Chris Kerton
10 CLS
20 PRINT TAB(35)"***************"
30 PRINT TAB(35)"*THE ADVENTURE*"
40 PRINT TAB(35)"***************"
50 PRINT:PRINT
60 PRINT TAB(35)"Written by Chris Kerton (C-Tek)"
70 PRINT TAB(35)"==============================="
80 PRINT
90 INPUT"Please enter your name";N$
96 PLAY"c4f.c8f8.c16f8.g16a2f2"
100 INPUT"Would you like instructions (y or n)";IN$
110 IF IN$="n" THEN GOTO 310
120 IF IN$="y" THEN GOTO 125
125 CLS
130 PRINT TAB(30)"######################################"
140 PRINT TAB(30)"#THE INSTRUCTIONS FOR 'The Adventure'#"
150 PRINT TAB(30)"######################################"
160 PRINT:PRINT
170 PRINT"Okay ";N$;" 'ADVENTURE' is a fantasy, role-playing game."
180 PRINT"The object of the game is to make it to the end and win."
190 PRINT"To do this you will need to use your imagination and wit."
200 PRINT"---------------------------------------------------------"
210 PRINT:PRINT
220 PRINT TAB(35)"BASIC CONTROLS"
230 PRINT TAB(35)"=============="
240 PRINT
250 PRINT"S-key means go south----------------E-key means go east"
260 PRINT"N-key means go north----------------W-key means go west"
270 PRINT"L-key means look--------------------K-key means kill"
280 PRINT"CTRL and run/stop means quit--------T-key means talk"
290 PRINT"-------------------------------------------------------"
300 PRINT:PRINT
310 INPUT"Would you like to start (y or n)";ST$
320 IF ST$="n" THEN GOTO 9990
330 IF ST$="y" THEN GOTO 340
340 CLS
350 PRINT TAB(35)"***************"
360 PRINT TAB(35)"*THE ADVENTURE*"
370 PRINT TAB(35)"***************"
380 PRINT:PRINT
390 PRINT TAB(10)"Designed by Chris Kerton------------C-Tek industries"
400 PRINT TAB(10)"<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>"
410 PRINT TAB(10)"Copyright 1988-12-22----------------Volume One"
420 PRINT:PRINT:PRINT
430 PRINT"You are outside of your local high school."
440 PRINT"It will probably be another boring uneventful day."
450 INPUT"What would you like to do";CH1$
460 IF CH1$="l" THEN GOTO 525
470 IF CH1$="s" THEN GOTO 940
480 IF CH1$="n" THEN GOTO 890
490 IF CH1$="e" THEN GOTO 810
500 IF CH1$="w" THEN GOTO 850
510 IF CH1$="k" THEN GOTO 660
520 IF CH1$="t" THEN GOTO 785
525 PRINT:PRINT
530 PRINT"To your north is the school entrance, with your usual"
540 PRINT"group of wasteoids who smoke things with weird smells,"
550 PRINT"and the entrance to the school."
560 PRINT:PRINT
570 PRINT"To your south you see the stairway which you have just"
580 PRINT"descended and the usual group of jocks hanging around, waiting"
590 PRINT"for the morning bell to ring while they talk about hockey."
600 PRINT:PRINT
610 PRINT"To your east you see nothing of importance except the rest of the"
620 PRINT"school and to the west you see the school parking lot with"
630 PRINT"a couple of school buses pulling in that are about to"
640 PRINT"drop off their load of shit, *OOPS*, I mean students."
650 GOTO 450
660 PRINT:PRINT
670 PRINT"Kill who!? The students? Man, you must be crazy!"
680 PRINT"But since you said so:"
690 PRINT:PRINT
700 PRINT"You attack the nearest unsuspecting person which happens"
710 PRINT"to be a wasteoid with a leather jacket, spike bands, and"
720 PRINT"various other things a street gang person might have."
730 PRINT
740 PRINT"Three of the wasteoids, who look tough, immediately jump foward"
750 PRINT"to save their buddy you have just attacked."
760 PRINT
770 PRINT"They pull out a switch-blade knife and then *#fasdfsg!@@##%$@%^BN!!!"
774  INPUT"Would you like to start over (y or n)";QU2$
775  IF QU2$="y" THEN GOTO 5
780  IF QU2$="n" THEN GOTO 9990
785 PRINT:PRINT
790 PRINT"As you babble on to nobody in particular, a few people turn"
800 PRINT"to look at you strangely and then go back to what they were doing."
805 GOTO 450
810 PRINT:PRINT
820 PRINT"You find yourself walking toward a wall which is part of the"
830 PRINT"school and is also a dead end."
840 GOTO 450
850 PRINT:PRINT
860 PRINT"You are in the school parking lot with a few cars"
870 PRINT"and a bus is dropping off some students."
880 GOTO 1060
890 PRINT:PRINT
900 PRINT"You are standing in front of the school doors"
910 PRINT"with a few wasteoids standing around going about their own"
920 PRINT"business."
930 GOTO 2150
940 PRINT:PRINT
950 PRINT"As you are going up the stairs you meet up with"
960 PRINT"two of your friends, Larry and Moe. They greet you"
970 PRINT"with 'hey ";N$;" hows it goin?'"
975 PRINT
980 INPUT"What do you want to do";CH2$
990 IF CH2$="l" THEN GOTO 1570
1000 IF CH2$="s" THEN GOTO 1690
1010 IF CH2$="n" THEN GOTO 2000
1020 IF CH2$="e" THEN GOTO 2020
1030 IF CH2$="w" THEN GOTO 2020
1040 IF CH2$="k" THEN GOTO 2070
1050 IF CH2$="t" THEN GOTO 1770
1060 PRINT:PRINT
1070 INPUT"What do you want to do";CH3$
1080 IF CH3$="l" THEN GOTO 1150
1090 IF CH3$="s" THEN GOTO 1260
1100 IF CH3$="n" THEN GOTO 1320
1110 IF CH3$="e" THEN GOTO 1400
1120 IF CH3$="w" THEN GOTO 1410
1130 IF CH3$="k" THEN GOTO 1470
1140 IF CH3$="t" THEN GOTO 1540
1150 PRINT:PRINT
1160 PRINT"To the south you see a muddy field, the school sign, and"
1170 PRINT"the street."
1180 PRINT
1190 PRINT"To the north you see the end of the school and a moving"
1200 PRINT"bus."
1210 PRINT
1220 PRINT"To the east you see where you have just came from."
1230 PRINT
1240 PRINT"To the west you see the woods."
1250 GOTO 1060
1260 PRINT:PRINT
1270 PRINT"You are walking across the muddy, wet, slippery field"
1280 PRINT"towards the school sign and the street and !@#$%^&*(((!!!!"
1290 PRINT"woo!!!!. You slip in the mud and hit your head on"
1300 PRINT"on the metal post of the sign killing yourself."
1304 INPUT"Would you like to start over (y or n)";QU1$
1305 IF QU1$="y" THEN GOTO 5
1310 IF QU1$="n" THEN GOTO 9990
1320 PRINT:PRINT
1330 PRINT"You start to walk north and notice the bus you saw"
1340 PRINT"is coming straight at you! Apparentely the bus driver"
1350 PRINT"does not see you."
1360 PRINT"You trip over your own feet trying to get out"
1370 PRINT"of the way, you clumsy oaf, and !@#$%^&*()!!!!!!!"
1374 INPUT"Would you like to start over (y or n)";QU3$
1375 IF QU3$="y" THEN GOTO 5
1380 IF QU3$="n" THEN GOTO 9990
1384 INPUT"Would you like to start over (y or n)";QU3$
1385 IF QU3$="y" THEN GOTO 5
1390 PRINT:PRINT
1400 GOTO 430
1410 PRINT:PRINT
1420 PRINT"You go west and slide down the steep bank on your"
1430 PRINT"ass."
1440 PRINT"You are now at the base of the woods."
1450 GOTO 3230
1460 PRINT:PRINT
1470 PRINT"You take a swat at the blackfly buzzing around your"
1480 PRINT"head and with your excellent reflexes, you"
1490 PRINT"manage to squash it. You notice blood spurts"
1500 PRINT"everywhere indicating that the blackfly just got"
1510 PRINT"done feasting on your arm."
1520 GOTO 1060
1530 PRINT:PRINT
1540 PRINT"You find yourself talking to yourself and then"
1550 PRINT"realize, am I going insane?"
1560 GOTO 1060
1570 PRINT:PRINT
1580 PRINT"To the north you see the entrance to the school"
1590 PRINT"and the few wasteoids that have not gone inside yet."
1600 PRINT
1610 PRINT"To the south you see the top of the stairs"
1620 PRINT"and the odd person rushing to get to class."
1630 PRINT
1640 PRINT"To the east you see the side of the school and"
1650 PRINT"to the west you see the school parking lot and the street."
1660 PRINT"You also see Larry and Moe."
1670 GOTO 980
1680 PRINT:PRINT
1690 PRINT"You go south which is to the top of the stairs."
1700 PRINT"Here you see the upper parking lot where most"
1710 PRINT"of the students park their cars."
1720 PRINT"Apparently Larry and Moe have followed and ask"
1730 PRINT"where are you going?"
1740 PRINT"They must think you want to jig school today and"
1750 PRINT"they say is sounds like a good plan."
1760 PRINT:PRINT
1770 PRINT"You ask Larry and Moe what they want to do today"
1780 PRINT"and they say cut class."
1790 PRINT
1800 INPUT"Do you want to cut class (y or n)";CC$
1810 IF CC$="y" THEN GOTO 1880
1820 IF CC$="n" THEN GOTO 1830
1830 PRINT:PRINT
1840 PRINT"Larry and Moe say 'What a Bummer' come on ";N$;","
1850 PRINT"let's go to class."
1860 PRINT"You go back down the stairs and towards the school doors."
1870 PRINT GOTO 890
1880 PRINT:PRINT
1890 GOTO 980
1900 PRINT:PRINT
2000 GOTO 430
2010 PRINT:PRINT
2020 PRINT"You hop over the stair railing and end up catching"
2030 PRINT"your foot on the railing, roll down the hill and"
2040 PRINT"the next thing you know, you wake up in  a hospital"
2050 PRINT"bed with a broken spine!"
2054 INPUT"Would you like to start over (y or n)";QU4$
2055 IF QU4$="y" THEN GOTO 5
2060 IF QU4$="n" THEN GOTO 9990
2070 PRINT:PRINT
2080 PRINT"You grab Moe and try to strangle him."
2090 PRINT"Larry hollers out ";N$;" what the hell do you"
2100 PRINT"think you are doing? You manage to snap Moe's neck"
2110 PRINT"killing him and Larry picks up a big rock and brings"
2120 PRINT"it crashing down on your skull with a crunching"
2130 PRINT"sound...!@#$%^&*()!!!!!"
2134 INPUT"Would you like to start over (y or n)";QU5$
2135 IF QU5$="y" THEN GOTO 5
2140 IF QU5$="n" THEN GOTO 9990
2150 PRINT:PRINT
2160 INPUT"Would you like to enter the school (y or n)";V1$
2170 IF V1$="n" THEN GOTO 420
2180 IF V1$="y" THEN GOTO 2190
2190 PRINT:PRINT
2200 PRINT"You open the door and enter the school."
2210 PRINT:PRINT
2220 INPUT"What would you like to do";V2$
2230 IF V2$="l" THEN GOTO 2300
2240 IF V2$="s" THEN GOTO 890
2250 IF V2$="n" THEN GOTO 2360
2260 IF V2$="e" THEN GOTO 2440
2270 IF V2$="w" THEN GOTO 2500
2280 IF V2$="k" THEN GOTO 690
2290 IF V2$="t" THEN GOTO 2550
2300 PRINT:PRINT
2310 PRINT"You are in a hallway. To your south is the"
2320 PRINT"exit, to the north is the rest of the hallway,"
2330 PRINT"to the east is the entrance to the gym and to"
2340 PRINT"the west another hallway with various classrooms."
2350 GOTO 2210
2360 PRINT:PRINT
2370 PRINT"You walk a bit more and come to hallways"
2380 PRINT"at your west and east. To your north is another"
2390 PRINT"exit. At the ends of the west and east hallways are"
2400 PRINT"exits."
2410 PRINT"The hallways have various doors which are classrooms,"
2420 PRINT"and the library is to your east."
2430 GOTO 2520
2440 PRINT:PRINT
2450 PRINT"You enter the gym and notice that a class is in session."
2460 PRINT"The gym teacher asks you what you want and then says"
2470 PRINT"please leave and do not interrupt my class or you will"
2480 PRINT"get detention!"
2490 GOTO 2210
2500 PRINT:PRINT
2510 PRINT"You walk up the hallway to the exit."
2520 INPUT"What do you want to do";V3$
2530 GOTO 2580
2540 PRINT:PRINT
2550 PRINT"You babble on aimlessly and a couple of students"
2560 PRINT"turn to look at you strangely."
2570 GOTO 2210
2580 IF V3$="l" THEN GOTO 2650
2590 IF V3$="s" THEN GOTO 2300
2600 IF V3$="e" THEN GOTO 2700
2610 IF V3$="w" THEN GOTO 2730
2620 IF V3$="k" THEN GOTO 660
2630 IF V3$="t" THEN GOTO 2760
2640 IF V3$="n" THEN GOTO 3100
2650 PRINT:PRINT
2660 PRINT"To your east is the library, you just came from the south"
2670 PRINT"to your west is nothing of significance and to your north"
2680 PRINT"is a water fountain."
2690 GOTO 2520
2700 PRINT:PRINT
2710 PRINT"You enter the library which is a typical school library."
2720 GOTO 2830
2730 PRINT:PRINT
2740 PRINT"There is a wall in your way!"
2750 GOTO 2520
2760 PRINT:PRINT
2770 PRINT"Do you like talking to yourself idiot stick?"
2780 GOTO 2520
2790 PRINT:PRINT
2800 PRINT"The door seems to be locked! Dam it, shit, darn!"
2810 PRINT"That really burns the shit off your underwear!"
2820 GOTO 2520
2830 PRINT:PRINT
2840 INPUT"What would you like to do";Z$
2850 IF Z$="l" THEN GOTO 2920
2860 IF Z$="s" THEN GOTO 2970
2870 IF Z$="e" THEN GOTO 3020
2880 IF Z$="w" THEN GOTO 2650
2890 IF Z$="k" THEN GOTO 3040
2900 IF Z$="t" THEN GOTO 3140
2910 IF Z$="n" THEN GOTO 3180
2920 PRINT:PRINT
2930 PRINT"To your west is where you just came from, to your"
2940 PRINT"east and south is the rest of the library and"
2950 PRINT"to your north is an exit."
2960 GOTO 2830
2970 PRINT:PRINT
2980 PRINT"You run into a stack of books knocking them over."
2990 PRINT"@#@@#$^^&&%*&*(()!!!!! too bad ";N$;" you will be picking up"
3000 PRINT"books for the rest of the day! Idiot stick!"
3004 INPUT"Would you like to start over (y or n)";QU6$
3005 IF QU6$="y" THEN GOTO 5
3010 IF QU6$="n" THEN GOTO 9990
3020 PRINT:PRINT
3030 GOTO 2980
3040 PRINT:PRINT
3050 PRINT"You try to kill the librarian who is big and fat"
3060 PRINT"and she just knocks you down with a big swipe"
3070 PRINT"and puts her fat ass on your ugly face killing"
3080 PRINT"you!!"
3084 INPUT"Would you like to start over (y or n)";QU7$
3085 IF QU7$="y" THEN GOTO 5
3090 IF QU7$="n" THEN GOTO 9990
3100 PRINT:PRINT
3110 PRINT"You slip on some water that has spilled out of the"
3120 PRINT"fountain @#$%^&*()!!!!!!"
3124 INPUT"Would you like to start over (y or n)";QU8$
3125 IF QU8$="y" THEN GOTO 5
3130 IF QU8$="n" THEN GOTO 9990
3140 PRINT:PRINT
3150 PRINT"Talking to yourself causes people to think"
3160 PRINT"you are weird."
3170 GOTO 2830
3180 PRINT:PRINT
3190 PRINT"You go through the exit meeting up with the principal!"
3200 PRINT"He thinks you want to cut class and you tell him off."
3210 PRINT"For that you will be spending time in detention all week!"
3214 INPUT"Would you like to start over (y or n)";QU9$
3215 IF QU9$="y" THEN GOTO 5
3220 IF QU9$="n" THEN GOTO 9990
3230 PRINT:PRINT
3240 PRINT"<<<<<<<<<<<<<<<<<<<<CHAPTER TWO>>>>>>>>>>>>>>>>>>>>>>>>"
3250 PRINT"                    -----------"
3260 PRINT:PRINT:PRINT
3270 PRINT"              WEIRDOID LAND!!!!!!!!!!!!!!"
3274 INPUT"Would you like to begin (y or n)";BE$
3275 IF BE$="y" THEN GOTO 3230
3276 IF BE$="n" THEN GOTO 9990
9990 CLS
9995 PLAY"c4f.c8f8.c16f8.g16a2f2"
10000 PRINT"*******************************************************************"
10010 PRINT"*******************************************************************"
10020 PRINT"* What a shit head you are ";N$;" you got wanked or killed!       *"
10030 PRINT"* I hope you know what mistake you made and will remember it next *"
10040 PRINT"* time.                                                           *"
10050 PRINT"* Please do not list this program. If you looked at the solutions *"
10060 PRINT"* then the game would be no fun. It is recomended that you just   *"
10070 PRINT"* play the game normally because C-Tek does not know the command  *"
10080 PRINT"* to keep you from listing this game. Keep an eye open for updated*"
10090 PRINT"* versions of this game.                                          *"
10100 PRINT"*                                                                 *"
10110 PRINT"*                                                                 *"
10120 PRINT"*=================================================================*"
10130 PRINT"*             ##THE ADVENTURE##  ##VOLUME ONE##                   *"
10140 PRINT"*=================================================================*"
10150 PRINT"* Nice try ";N$;" we hope you liked this software.......          *"
10160 PRINT"*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*"
10170 PRINT"*Designed by Chris Kerton-------------------------C-Tek industries*"
10180 PRINT"*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*"
10190 PRINT"*Copyright 1988-12-22-----------------------------------Volume One*"
10200 PRINT"*******************************************************************"
10210 INPUT "Would you like to start over (y or n)";ST$
10220 IF ST$="y" THEN GOTO 5
10230 IF ST$="n" THEN GOTO 10240
10240 PRINT"What a wang and a poor sport you are ";N$;" see you later asswipe"
10250 END
