10 REM "The Vial of Doom"  Copyright (c) 1983 by Roger M. Wilcox                   Revision number 3
20 KEY OFF:CLS:LOCATE 14:PRINT"I know the verbs STICK, SWING, and PLUCK.":DEF SEG=&H1700:DEFINT A-Z:DIM D$(6),PLACE$(21),PLACE(21,6),OB$(40),OB(40),VERB$(55),NOUN$(48):OFFSET=1:FOR X=1 TO 40:READ Y:POKE X,Y:NEXT
30 FOR X=1 TO 6:READ D$(X):NEXT:FOR X=1 TO 21:READ PLACE$(X):FOR Y=1 TO 6:READ PLACE(X,Y):NEXT Y,X:FOR X=1 TO 40:READ OB$(X),OB(X):NEXT:FOR X=1 TO 55:READ VERB$(X):NEXT:FOR X=1 TO 48:READ NOUN$(X):NEXT:CP=1:Q$=CHR$(34)
40 START=0:Y=960:CALL OFFSET(START,Y):LOCATE 1:IF LEFT$(PLACE$(CP),1)="*" THEN PRINT MID$(PLACE$(CP),2);ELSE PRINT"You are in "PLACE$(CP);
50 PRINT". ";:IF CP=1 OR CP=17 THEN OB(6)=CP
60 Y=1:FOR X=1 TO 40:IF OB(X)<>CP THEN 90 ELSE IF Y THEN PRINT"Visible items:":PRINT:Y=0
70 IF POS(0)+LEN(OB$(X))>78 THEN PRINT
80 PRINT OB$(X)". ";
90 NEXT:Y=1:FOR X=1 TO 6:IF PLACE(CP,X)=0 THEN 110 ELSE IF Y THEN PRINT:PRINT:PRINT"Obvious exits:";:Y=0
100 PRINT" "D$(X);
110 NEXT:PRINT:PRINT STRING$(80,220)
120 IF CP>4 THEN IF OB(5)=23 AND OB(4)<>23 OR WV THEN 1220
130 LOCATE 13:START=1920:Y=79:CALL OFFSET(START,Y):A=0:B=0:A$="":B$="":PRINT"  ------"CHR$(26);:INPUT" What now";A$:IF A$="" THEN 130 ELSE START=2080:Y=880:CALL OFFSET(START,Y):X=2
140 IF MID$(A$,X,1)="" THEN 170 ELSE IF MID$(A$,X,1)<>" " THEN X=X+1:GOTO 140
150 B$=MID$(A$,X+1):A$=LEFT$(A$,X-1)
160 IF LEFT$(B$,1)=" " THEN B$=MID$(B$,2):GOTO 160
170 IF LEFT$(A$,1)>"Z" OR MID$(A$,2,1)>"Z" THEN PRINT"Use only capital letters, please.":GOTO 130 ELSE A1$=LEFT$(A$,4):B1$=LEFT$(B$,4):FOR X=1 TO 55:IF A1$=VERB$(X) THEN A=X
180 NEXT:IF A=0 THEN PRINT"I don't know how to "Q$A$Q$" something.":GOTO 130 ELSE IF B$="" OR B1$="GAME" THEN 200 ELSE FOR X=1 TO 48:IF B1$=NOUN$(X) THEN B=X
190 NEXT:IF B=0 THEN PRINT"I don't know what "Q$B$Q$" is.":GOTO 130
200 IF TR>0 THEN TR=TR-1:PRINT"You have"TR"seconds!":IF TR=0 THEN 1130
210 ON A GOTO 220,230,220,230,220,230,220,230,220,230,220,230,750,750,750,990,990,240,240,290,290,330,370,370,390,410,420,430,460,500,520,550,590,590,600,240,620,630,640,660,670,680,700,730,680,420,1000,1160,1170,1180,1190,1190,990,1010,740
220 D=(A+1)/2:GOTO 1020
230 D=A/2:GOTO 1020
240 IF B=0 THEN 40 ELSE IF B=2 OR B=3 THEN IF OB(2)<>CP AND OB(2)<23 THEN 1210 ELSE PRINT"The portal has a golden inscription.":GOTO 130
250 IF B=15 THEN IF OB(14)<>CP THEN 1250 ELSE PRINT"Try frisking him.":GOTO 40 ELSE IF B=7 AND OB(5)<23 THEN 1230
260 IF B=7 THEN PRINT"The vial is in the shape of an octagonal, highly polished cone with a thick,":PRINT"glasslike shell containing a scintillating red liquid. The chain will easily fitaround your neck, but attempting this seems deadly.":GOTO 120
270 IF B=8 THEN IF OB(6)<>CP THEN 1210 ELSE PRINT"There is something big at the bottom of it.":GOTO 120
280 IF B=5 OR B=6 THEN 390 ELSE IF B=30 THEN IF OB(30)=CP THEN 1250 ELSE IF OB(28)<>CP THEN 1210 ELSE PRINT"Zap! You have just been turned to stone!!":GOTO 1050 ELSE 1250
290 PRINT"You are currently holding the following:":A$="Nothing at all":FOR X=1 TO 37:IF OB(X)<23 THEN 310 ELSE A$="":IF POS(0)+LEN(OB$(X))>77 THEN PRINT
300 PRINT OB$(X)". ";
310 NEXT:PRINT A$:IF WV THEN PRINT"(You're wearing the vial)"
320 GOTO 120
330 IF OB(1)<23 THEN PRINT"You have nothing to dig with.":GOTO 120 ELSE IF CP=1 AND OB(35)=0 THEN PRINT"Crash!! You fall through the sand, and it piles back on top of you.":CP=3:GOTO 40
340 IF CP=3 THEN IF WV=0 THEN PRINT"You don't have the strength to get back out.":GOTO 120 ELSE PRINT"ROWWWWR! With a surge of greath strength, you get back to the sand dune!!":CP=1:GOTO 40
350 IF CP=1 THEN PLACE(CP,6)=3:CP=3:PLACE(CP,5)=1:PRINT"You've made a tunnel right through the sand dune! That's Incredible!!":GOTO 40
360 PRINT"You found nothing.":GOTO 120
370 IF B=0 THEN 1240 ELSE IF B<>3 THEN 450 ELSE IF OB(2)<>CP THEN 1210
380 PRINT"The portal dematerializes with a "Q$"Whoosh!"Q$". Evidently the pyramid was sealed in":PRINT"a vacuum. You scurry through the portal just before the outside materializes":PRINT"again.":CP=4:GOTO 40
390 IF B=0 THEN 1240 ELSE IF B<>5 AND B<>6 AND B<>21 THEN PRINT"It can't be opened.":GOTO 120 ELSE IF B=5 OR B=6 THEN IF OB(4)<23 THEN 1230 ELSE IF OB(5)>0 THEN 1250 ELSE PRINT"It has a "OB$(5)" in it.":OB(5)=23:GOTO 1060
400 IF OB(17)<>CP THEN 1210 ELSE INPUT"With what";A$:IF A$<>"VIAL" THEN PRINT"Sorry, no go.":GOTO 120 ELSE IF OB(5)<23 THEN 1230 ELSE PRINT"Creeeek!":OB(17)=0:PLACE(CP,3)=20:GOTO 40
410 IF B=0 THEN 1240 ELSE IF B<>7 THEN PRINT"You can't wear a "B$".":GOTO 120 ELSE IF OB(5)<23 THEN 1230 ELSE PRINT"Ok":WV=1:GOTO 120
420 IF B=0 THEN 1240 ELSE IF B<>7 THEN 450 ELSE IF OB(5)<23 THEN 1230 ELSE IF OB(8)<>CP THEN 450 ELSE PRINT"The clerk is now hypnotized.":OB(9)=CP:OB(8)=0:GOTO 40
430 IF B=0 THEN 1240 ELSE IF B=45 THEN 1220 ELSE IF B=47 THEN PRINT "Sorry. That Adventure takes place:":PRINT Q$"A long time ago, in a galaxy far, far away...."Q$:GOTO 120 ELSE IF B<>44 THEN PRINT"Nothing happens.":GOTO 120
440 IF CP<5 OR CP=8 THEN IF OB(35)>0 THEN PRINT"That's already being done....":GOTO 120 ELSE PRINT"Sorry. There seems to be some Chaotic interference in this area.":GOTO 120
450 LAW=1:IF OB(12)=CP THEN PRINT"The guard is off-guard!":OB(12)=0:OB(33)=CP:GOTO 40 ELSE IF OB(24)=CP THEN PRINT"The gate attendant seems friendlier already!":GOTO 120 ELSE PRINT"Ok":GOTO 120
460 IF B=0 THEN 1240 ELSE IF B=8 THEN IF OB(6)<>CP THEN 1210 ELSE IF OB(28)=CP THEN PRINT"Basilisk won't let you.":GOTO 120 ELSE IF LAW=0 THEN PRINT"You would drown! What do you think you are, a god?":GOTO 120 ELSE CP=7:GOTO 40
470 IF B=22 THEN IF OB(18)<>CP THEN 1210 ELSE CP=21:GOTO 40 ELSE IF B=26 OR B=27 THEN IF OB(25)<>CP AND OB(38)<>CP THEN 1210 ELSE PRINT"Ok. You fly through the air, and land somewhere else.":IF CP=13 THEN CP=12:GOTO 40 ELSE CP=13:GOTO 40
480 IF B=28 THEN IF OB(26)<>CP AND OB(39)<>CP THEN 1210 ELSE PRINT"Ok. You ride for a while, then get off somewhere else.":IF CP=14 THEN CP=15:GOTO 40 ELSE CP=14:GOTO 40
490 IF B>34 AND B<41 THEN D=B-34:GOTO 1020 ELSE PRINT"You can't go to a "B$".":GOTO 120
500 IF B=0 THEN 1240 ELSE IF B<>23 THEN PRINT"It didn't stick.":GOTO 120 ELSE IF OB(20)=CP THEN PRINT"Are you crazy? One bite and you're dead!":GOTO 120 ELSE IF OB(19)<>CP THEN 1210 ELSE INPUT"Into where (one word)";A$:A$=LEFT$(A$,4)
510 IF A$<>"CONT" THEN PRINT"Sorry, no go.":GOTO 120 ELSE IF OB(5)<23 THEN 1230 ELSE PRINT"Squirtt! The container fills with venom, and just as quickly, the cobra awakes!":OB(3)=0:OB(21)=23:OB(19)=0:OB(20)=CP:GOTO 40
520 IF B=0 THEN 1240 ELSE IF B=9 OR B=42 THEN PRINT"Be more specific.":GOTO 120 ELSE IF OB(10)<23 THEN PRINT"You have nothing to kill with!":GOTO 120 ELSE IF B=30 THEN PRINT"Sorry -- too tough!":GOTO 120
530 IF B<>23 THEN PRINT"I cannot allow you to do that, Frodo.":GOTO 120 ELSE IF OB(19)=CP THEN PRINT"The venom needs to be from a live cobra to work. If you were to kill this one,  it would be useless.":GOTO 120 ELSE IF OB(20)<>CP THEN 1210
540 IF LAW=0 THEN PRINT"The cobra withstands your blows!":GOTO 120 ELSE PRINT"Sparrrk! The cobra vanishes in a cloud of electrical smoke!!":OB(20)=0:GOTO 40
550 IF B=39 THEN PRINT"Barf!":GOTO 120 ELSE IF B=16 THEN IF OB(33)<>CP THEN 990 ELSE IF OB(13)<23 THEN 1230 ELSE PRINT"The pill lands in the guard's Thermos, he drinks, and promptly falls asleep.":OB(33)=0:OB(14)=CP:OB(13)=0:GOTO 40
560 IF B=11 OR B=12 THEN IF OB(10)<23 THEN 1230 ELSE IF OB(7)<>CP THEN PRINT"Ok":OB(10)=CP:GOTO 120 ELSE PRINT"Arrg! The dagger lodges into the octopus, and allows you to escape north!":OB(10)=0:CP=1:GOTO 1080
570 IF B<>2 THEN 990 ELSE IF OB(2)<23 THEN 1230 ELSE IF CP>1 OR OB(35)=0 THEN PRINT"Ok":OB(2)=CP:GOTO 120
580 PRINT"Whump! Chaos is down! Law wins the fight, and says:":PRINT Q$"Make the mixture here!"Q$" P.S.: Why not look at the debris of the pyramid?":OB(2)=0:OB(31)=8:OB(35)=0:OB(40)=8:GOTO 120
590 IF B=0 THEN 1240 ELSE IF B<>15 THEN PRINT"Sorry, no can do.":GOTO 120 ELSE IF OB(33)=CP OR OB(12)=CP THEN PRINT"Guard won't let you.":GOTO 120 ELSE IF OB(14)<>CP THEN 1210 ELSE PRINT"A "OB$(15)" fell out.":OB(15)=CP:GOTO 40
600 IF B=0 THEN 1240 ELSE IF B<>29 THEN PRINT"Unshowable.":GOTO 120 ELSE IF OB(27)<23 THEN 1230 ELSE IF OB(28)<>CP THEN 450
610 PRINT"Zap! The "Q$"mirror"Q$" has just transformed the basilisk to stone via its own gaze!":OB(28)=0:OB(30)=CP:GOTO 40
620 IF OB(22)<23 THEN PRINT"You have nothing to suck with.":GOTO 120 ELSE IF OB(34)<>CP THEN PRINT"There is nothing here to suck.":GOTO 120 ELSE PRINT"Whooosh! The octopus woke up!!":OB(22)=0:OB(23)=23:OB(34)=0:OB(7)=CP:GOTO 40
630 PRINT"Not YET!!!":GOTO 120
640 IF OB(32)<>CP AND OB(32)<>23 THEN 1260
650 PRINT"In a soundless concussion of darkness, you find yourself on the far side of the mountain! The victory is yours -- and the galaxy's.":GOTO 1050
660 IF B=0 THEN 1240 ELSE IF B<>34 THEN PRINT"Huh?":GOTO 120 ELSE 640
670 IF OB(28)<>CP THEN 1260 ELSE IF OB(29)>0 THEN PRINT"Already been done.":GOTO 120 ELSE PRINT"Pook! One of its eyes pops out in your hand, but instantly grows back!":PRINT"It's mad, now!!":OB(29)=23:GOTO 120
680 IF OB(15)<23 THEN PRINT"You have nothing to "A$" with.":GOTO 120 ELSE IF B<>0 AND B<>15 PRINT"That wouldn't do any good anyway.":GOTO 120 ELSE IF OB(12)<>CP THEN PRINT"How?":GOTO 120
690 PRINT"He says, "Q$"Thanks!"Q$", goes into the store, and gives you a fire opal. He":PRINT"promptly "Q$"forgets"Q$" you and goes back on duty.":OB(15)=0:OB(16)=23:GOTO 40
700 IF OB(35)=1 THEN PRINT"Hit Chaos with something.":GOTO 120 ELSE IF OB(8)=CP THEN PRINT"Try hypnotizing him.":GOTO 120 ELSE IF CP=1 AND OB(1)=2 THEN PRINT"Go west.":GOTO 120
710 IF CP<5 OR OB(5)=0 THEN PRINT"Sorry, I can't.":GOTO 120
720 PRINT"If you find that you're in trouble, why not use the power of Law to your":PRINT"advantage? After all, Law is on your side!":GOTO 120
730 PRINT"Coward!":GOTO 1050
740 IF B=0 THEN 1240 ELSE IF B<>10 THEN PRINT"You can't.":GOTO 120 ELSE PRINT"Be more specific as to how.":GOTO 120
750 IF B=0 THEN 1240 ELSE IF B=46 THEN 290 ELSE IF B=3 OR B>7 AND B<11 OR B=15 OR B>20 AND B<24 OR B>24 AND B<29 OR B=30 OR B=41 OR B=42 OR B>43 THEN PRINT"It's beyond your power to do that.":GOTO 120
760 IF B>34 AND B<40 THEN PRINT"I don't get it.":GOTO 120 ELSE IF B=40 THEN PRINT"...and boogie!":GOTO 120 ELSE NA=CP:NB=23:Y=0:FOR X=1 TO 32:IF OB(X)=23 THEN Y=Y+1
770 NEXT:IF OB(37)=23 THEN Y=Y+1
780 IF Y>6 THEN PRINT"You are currently holding too much. Try: TAKE INVENTORY":GOTO 120 ELSE 800
790 IF NA=CP THEN 1210 ELSE 1230
800 IF OB(8)=CP AND B>10 AND B<17 THEN PRINT"The clerk says you'll have to pay cash.":GOTO 120
810 ON B GOTO 820,830,,850,860,860,870,,,,880,880,890,890,,900,910,910,920,920,,,,930,,,,,940,,950,960,970,970:GOTO 980
820 IF OB(1)<>NA THEN 790 ELSE PRINT"Ok":OB(1)=NB:GOTO 40
830 IF OB(2)<>NA THEN 790 ELSE IF OB(35)>0 THEN OB(2)=NB:IF NA=CP THEN PRINT"With a great warring strength, you lift the pyramid!":GOTO 40 ELSE PRINT"Ok":GOTO 40 ELSE IF WV=1 THEN PRINT"Your Chaotic strength can budge it, but not lift it.":GOTO 120
840 PRINT"Too big!":GOTO 120
850 IF OB(3)=NA THEN PRINT"Ok":OB(3)=NB:GOTO 40 ELSE IF OB(21)<>NA THEN 790 ELSE PRINT"Ok":OB(21)=NB:GOTO 1100
860 IF OB(4)<>NA THEN 790 ELSE PRINT"Ok":OB(4)=NB:IF OB(36)<>4 AND OB(37)<>4 THEN PRINT"A mummy has come to life!":OB(36)=CP:GOTO 40 ELSE 40
870 IF OB(5)<>NA THEN 790 ELSE PRINT"Ok":OB(5)=NF:WV=0:IF CP=1 AND NB=CP AND GLOW THEN 1140 ELSE 40
880 IF OB(10)<>NA THEN 790 ELSE PRINT"Ok":OB(10)=NB:GOTO 40
890 IF OB(11)<>NA THEN 790 ELSE PRINT"Ok":OB(11)=NB:GOTO 1100
900 IF OB(13)<>NA THEN 790 ELSE PRINT"Ok":OB(13)=NB:GOTO 40
910 IF OB(15)<>NA THEN 790 ELSE PRINT"Ok":OB(15)=NB:IF OB(14)=CP THEN 1090 ELSE 40
920 IF OB(16)<>NA THEN 790 ELSE PRINT"Ok":OB(16)=NB:GOTO 1100
930 IF OB(22)=NA THEN PRINT"Ok":OB(22)=NB:GOTO 40 ELSE IF OB(23)<>NA THEN 790 ELSE PRINT"Ok":OB(23)=NB:GOTO 1100
940 IF OB(27)<>NA THEN 790 ELSE PRINT"Ok":OB(27)=NB:GOTO 40
950 IF OB(29)<>NA THEN 790 ELSE PRINT"Ok":OB(29)=NB:GOTO 1100
960 IF OB(31)<>NA THEN 790 ELSE PRINT"Ok":OB(31)=NB:GOTO 1100
970 IF OB(32)<>NA THEN 790 ELSE PRINT"Ok":OB(32)=NB:GOTO 40
980 IF OB(37)<>NA THEN 790 ELSE PRINT"Ok":OB(37)=NB:GOTO 40
990 IF B=0 THEN 1240 ELSE IF B=3 OR B>7 AND B<11 OR B=15 OR B>20 AND B<>24 AND B<29 OR B=30 OR B>30 AND B<41 OR B>43 THEN 1230 ELSE NA=23:NB=CP:GOTO 810
1000 IF B=0 THEN 1240 ELSE IF B<>7 OR WV=0 THEN PRINT"You're not wearing it.":GOTO 120 ELSE PRINT"Ok":WV=0:GOTO 120
1010 IF B=0 THEN STOP:GOTO 40 ELSE IF B<>7 THEN PRINT"There's no reason to go around breaking "B$"S.":GOTO 120 ELSE PRINT"Sorry, the vial of Chaos is indestructable.":GOTO 120
1020 IF PLACE(CP,D)=0 THEN PRINT"There is no way to go in that direction.":GOTO 120 ELSE IF OB(7)=CP THEN PRINT"The octopus has you in a tight bind with its tentacles!":GOTO 120
1030 IF D=2 AND CP=11 AND LAW=0 THEN PRINT"The gate attendant won't let you through without a ticket.":GOTO 120 ELSE IF OB(20)=CP THEN PRINT"The cobra won't let you leave!":GOTO 120 ELSE IF OB(36)=CP THEN PRINT"The mummy won't let you leave!":GOTO 120
1040 LAW=0:IF CP=4 THEN CP=OB(2):GOTO 40 ELSE CP=PLACE(CP,D):GOTO 40
1050 PRINT:INPUT"The adventure has ended. Care to try again";A$:A$=LEFT$(A$,1):IF A$="Y" THEN RUN ELSE IF A$="N" THEN CLS:NEW ELSE 1050
1060 PRINT"   Suddenly, the spirit of "Q$"Law"Q$" appears, and says:":PRINT"The vial is controlled by "Q$"Chaos,"Q$" the bad guy. To destroy the vial (although":PRINT"it may give you a little strength to escape), you must get the following"
1070 PRINT"ingredients:":PRINT"   Turquois gem, fire opal, cobra venom, basilisk eye, and octopus ink. Combine these by an alabaster bowl, then put the vial in, and *RUN*!!":GOTO 120
1080 PRINT"   Suddenly, Law and Chaos appear to the west and east respectively in the formsof giants. They start battling it out with lightning, but quickly reach a stale-mate. In the confusion, Law gives you great strength.":OB(35)=1:GOTO 40
1090 PRINT"The guard instantly wakes up, and says he'll give you anything if you:":PRINT"(1) -- don't tell his boss he was sleeping on the job, and":PRINT"(2) -- bribe him.":OB(14)=0:OB(12)=CP:GOTO 40
1100 IF NB=23 THEN GLOW=0:GOTO 40
1110 IF OB(11)=1 AND OB(16)=1 AND OB(21)=1 AND OB(23)=1 AND OB(29)=1 AND OB(31)=1 THEN PRINT"The mixture has begun to glow deep red.":GLOW=1
1120 GOTO 40
1130 PRINT"******  B O O M ! ! !  ******":PRINT"Your efforts weren't in vain, but you failed to save your own life! You're dead!";:GOTO 1050
1140 OB(5)=CP:PRINT"The vial vibrates, getting ready to explode. Oh, by the way --":PRINT"** YET ** !!":INPUT A$:A$=LEFT$(A$,3):IF A$<>"RUN" THEN 1130
1150 CP=18:PRINT"The vial goes off in a red, Chaotic mushroom cloud.":PRINT"You have only six (6) seconds until the fireball reaches you!":TR=6:GOTO 40
1160 OPEN "O",1,"VIAL.DAT":FOR X=1 TO 40:PRINT#1,OB(X):NEXT:PRINT#1,WV,LAW,PLACE(10,3),CP,TR,GLOW:CLOSE:PRINT"Ok":GOTO 120
1170 OPEN "I",1,"VIAL.DAT":FOR X=1 TO 40:INPUT #1,OB(X):NEXT:INPUT #1,WV,LAW,PLACE(10,3),CP,TR,GLOW:CLOSE:GOTO 40
1180 IF B=0 THEN 1240 ELSE IF B<>41 AND B<>3 THEN PRINT"There is nothing written on a "B$".":GOTO 120 ELSE IF OB(2)<>CP AND OB(2)<>23 THEN 1210 ELSE PRINT"It translates into, "Q$"Touch and go."Q$:GOTO 120
1190 IF B=0 THEN 1240 ELSE IF B=45 THEN PRINT"You can't hit Chaos directly. Try another approach.":GOTO 120 ELSE IF B<>42 OR WV=0 THEN PRINT"Nothing happens.":GOTO 120
1200 IF OB(36)=CP THEN PRINT"Baam! You made it fly apart!":OB(36)=0:OB(37)=CP:GOTO 40
1210 PRINT"You don't see it here.":GOTO 120
1220 PRINT"The powers of the vial have taken over your body! You're posessed by Chaos!!!":GOTO 1050
1230 PRINT"You don't have it.":GOTO 120
1240 PRINT A$" what?":GOTO 120
1250 PRINT"You see nothing special.":GOTO 120
1260 PRINT"You can't do that...yet!":GOTO 120
1270 DATA &H55,&H1E,&H06,&H8B,&HEC,&H8B,&H76,&H0C,&H8B,&H04,&H8B,&H76,&H0A,&H8B,&H0C,&H8B,&HF0,&HB8,&H00,&HB0,&H8E,&HD8,&H8E,&HC0,&HC7,&H04,&H20,&H07,&H8B,&HFE,&H47,&H47,&HF2,&HA5,&H07,&H1F,&H5D,&HCA,&H04,&H00
1280 DATA north,south,east,west,up,down
1290 DATA *You're on a sand dune,,,8,2,,
1300 DATA a desert,,,1,,,
1310 DATA *You're at the bottom of a sandy hole,,,,,,
1320 DATA a tomb chamber,,,,3,,
1330 DATA the city,6,,9,8,,
1340 DATA a pawn shop,,5,,,,
1350 DATA a mysterious inky lake,1,17,,,,
1360 DATA *You're on top of a mountain,,,5,1,,
1370 DATA front of a jewelry store,,10,,5,,
1380 DATA *You're at the foot of a large zoo gateway,9,11,,,,
1390 DATA the entrance to an airport,10,12,,,,
1400 DATA an airport,11,,,,,
1410 DATA an airport,14,,,,,
1420 DATA a bus terminal,13,,,,,
1430 DATA a bus terminal,16,,,,,
1440 DATA a forest,17,15,,,,
1450 DATA a forest,,16,,,,
1460 DATA *You're on the side of a mountain,,,,,19,
1470 DATA *You're on the side of a mountain,,,,,19,18
1480 DATA a zoo,,,,10,,
1490 DATA the cobra's cage,,,,,20,
1500 DATA Shovel,2,Pyramid with a stone portal,3,Tiny plastic container,4,Lead box,4,Twinkling vial with thin chain attached,,Mysterious-looking lake,1,=Deadly giant octopus=,,Store clerk,6,Hypnotized clerk,,Large dagger,6,Turquois gem,6
1510 DATA Guard with a Thermos of coffee,9,Sleeping pill,6,Sleeping guard,,Wad of money,,Fire opal,,Closed zoo gate,10,Cobra's cage,20,Sleeping cobra,21,=Deadly cobra,,Container full of venom,,Empty liquid sucker,11
1520 DATA Sucker full of octopus ink,,Gate attendant,11,Big airplane,12,Large bus,14,"Flat, reflective stone",16,Basilisk,17,Basilisk eye,,Stone basilisk,,Bowl made of alabaster,,Wishing rock,18,Distracted guard,
1530 DATA Sleeping giant octopus,7,Law and Chaos are still in battle,,Animated mummy,,Unwound ace bandages,,Big airplane,13,Large bus,15,Pyramid rubble,
1540 DATA NORT,N,SOUT,S,EAST,E,WEST,W,UP,U,DOWN,D,GET,TAKE,.,DROP,PUT,LOOK,L,INVE,I,DIG,TOUC,FEEL,OPEN,WEAR,WAVE,USE,GO,STIC,KILL,THRO,FRIS,SEAR,SHOW,EXAM,SUCK,RUN,WISH,MAKE,PLUC,PAY,HELP,QUIT,BRIB,SWIN,REMO,SAVE,LOAD,READ,HIT,PUNC,P,BREA,HYPN
1550 DATA SHOV,PYRA,PORT,CONT,LEAD,BOX,VIAL,LAKE,OCTO,CLER,DAGG,KNIF,TURQ,GEM,GUAR,PILL,WAD,MONE,FIRE,OPAL,GATE,CAGE,COBR,SUCK,ATTE,AIRP,PLAN,BUS,STON,BASI,EYE,BOWL,ROCK,WISH,NORT,SOUT,EAST,WEST,UP,DOWN,INSC,MUMM,BAND,LAW,CHAO,INVE,FORC,RUBB
