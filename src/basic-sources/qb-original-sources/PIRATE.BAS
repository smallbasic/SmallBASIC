100 REM *******************
110 REM *  PIRATE ISALND  *
120 REM *                 *
130 REM *        BY       *
140 REM *   DAVID MENY    *
150 REM *******************
160 CLS
170 DIM A(45,6),O1$(20),O2$(20),TR(14),IN$(14),RO$(45)
171 GOSUB 20000:REM INITIALIZE
172 LOCATE 21,1
173 PRINT "Pirate Island by David Meny"
174 PRINT "Copyright 1985, Menco, Inc."
175 PRINT
176 PRINT "  In your sleep, you dream of valiant pirates who sail across the seas, finding"
177 PRINT "secret treasure hidden deep inside dangerous islands."
178 PRINT "  You dream of adventourous fights to the death between pirates over gold-laced"
179 PRINT "statues or chest of gold coins from a long time ago."
180 PRINT "  You dream and dream until the rocking of the seas wakens you on board the ship"
185 PRINT "'The Eagle's Nest', a pirate ship!":PRINT
190 GOSUB 15000:REM PRINT ROOM DESCRIPTION
200 GOSUB 14000:REM PRINT ROOM CONTENTS
210 IF START=1 THEN MV=MV+1 ELSE 219
211 IF MV=10 THEN TR(2)=RO ELSE 219
212 PRINT "The captain walks up to you."
213 IF FLAG98=1 THEN 216
214 PRINT "'You fool!! We have been sailing for days now and we haven't found Pirate":PRINT "Island. I shouldn't have trusted you, even though you are my grand son.'"
215 PRINT "The captain pulls out his cutlass and decapitates you.":GOTO 2000
216 PRINT "'Congradulations!! We've come ashore on ze' old Pirate Island. Now go ashore by":PRINT "the east starboard deck.' The captain then hands you a bag of silver."
217 PRINT "'Spend it wisely. It's your payment IF you don't come back.'":TR(2)=25:A(39,2)=22:FLAG97=1:TR(14)=200
219 PRINT:INPUT ">",A$
220 IF A$="" THEN PRINT:PRINT "What?":GOTO 210
230 AST=ASC(A$)
240 IF AST<65 OR AST>90 THEN PRINT:PRINT "I can only use capital letters, matey.":GOTO 210
245 IF A$="LOOK UNDER JULIUS" THEN GOSUB 8010:GOTO 210
246 IF A$="LOOK UNDER PIRATE ISLAND" THEN GOSUB 8150:GOTO 210
247 IF A$="LOOK UNDER DAVID MENY" THEN GOSUB 8300:GOTO 210
250 M=LEN(A$):IF M<7 THEN A$=A$+" ":GOTO 250
260 KW=0:B$=LEFT$(A$,3)
270 IF B$="INV" OR B$="I  " THEN GOSUB 1000:GOTO 210
280 IF B$="SCO" THEN GOSUB 1500:GOTO 210
285 IF B$="DIG" THEN GOSUB 2250:GOTO 210
286 IF B$="KNO" THEN GOSUB 6500:GOTO 210
290 IF B$="QUI" OR B$="Q  " THEN GOSUB 2000
300 IF B$="LOO" OR B$="L  " THEN PRINT:GOTO 190
305 IF B$="SWI" THEN GOSUB 8500:GOTO 210
310 IF B$="N  " THEN C$="N  ":KW=1:GOSUB 2500:GOTO 210
320 IF B$="S  " THEN C$="S  ":KW=1:GOSUB 2500:GOTO 210
330 IF B$="E  " THEN C$="E  ":KW=1:GOSUB 2500:GOTO 210
340 IF B$="W  " THEN C$="W  ":KW=1:GOSUB 2500:GOTO 210
350 IF B$="U  " THEN C$="U  ":KW=1:GOSUB 2500:GOTO 210
360 IF B$="D  " THEN C$="D  ":KW=1:GOSUB 2500:GOTO 210
370 N=1
380 IF MID$(A$,N,1)=" " THEN C$=MID$(A$,N+1,3):IF LEFT$(C$,1)<>" " THEN 410 ELSE 400
390 IF N<M THEN N=N+1:GOTO 380
400 PRINT:PRINT "I don't know that word.":GOTO 210
410 IF B$="GO " OR B$="MOV" OR B$="WAL" THEN KW=1:GOSUB 2500:GOTO 210
420 IF B$="GET" OR B$="TAK" THEN GOSUB 3000:GOTO 210
430 IF B$="DRO" OR B$="PUT" THEN GOSUB 3500:GOTO 210
440 IF B$="OPE" THEN GOSUB 4000:GOTO 210
450 IF B$="UNL" THEN GOSUB 4500:GOTO 210
460 IF B$="BUY" THEN GOSUB 5000:GOTO 210
480 IF B$="CLI" THEN GOSUB 6000:GOTO 210
490 IF B$="REA" THEN GOSUB 7000:GOTO 210
510 PRINT:PRINT "I don't know that word."
520 GOTO 210
1000 REM INVENTORY
1010 PRINT "  You are carrying:"
1020 PRINT
1030 FOR I=1 TO 14
1040 IF TR(I)=200 THEN PRINT "    ";IN$(I):FLAG1=1
1050 NEXT I
1060 IF FLAG1<>1 THEN PRINT "    nothing"
1070 FLAG1=0
1080 RETURN
1500 REM SCORE
1510 PRINT
1520 PRINT "Out of 200 points, you got ";SCORE;" points."
1521 PRINT "That gives you the rating of ";
1522 IF SCORE<50 THEN PRINT "a Deck boy."
1523 IF SCORE>50 AND SCORE<100 THEN PRINT "a Sea Man."
1524 IF SCORE>100 AND SCORE<150 THEN PRINT "a Bucaneer."
1525 IF SCORE>150 THEN PRINT "a Sea Captain."
1530 RETURN
2000 REM QUIT
2010 COLOR 0,7:LOCATE 25,50:PRINT "Score:";SCORE:COLOR 7,0:PRINT
2020 PRINT "Out of 200 points, you got ";SCORE;" points."
2021 PRINT "That gives you the rating of ";
2022 IF SCORE<50 THEN PRINT "a Deck boy."
2023 IF SCORE>50 AND SCORE<100 THEN PRINT "a Sea Man."
2024 IF SCORE>100 AND SCORE<150 THEN PRINT "a Bucaneer."
2025 IF SCORE>150 THEN PRINT "a Sea Captain."
2029 IF EGG=1 THEN PRINT:GOTO 9020
2030 PRINT:GOTO 9020
2250 REM DIG ROUTINE
2251 INPUT "With what";F$:F$=LEFT$(F$,3)
2252 IF F$<>"SHO" THEN PRINT "You can't use that to dig with.":RETURN
2260 IF RO<>1 AND RO<>11 AND RO<>21 THEN 2270 ELSE 2290
2270 IF RO>23 THEN PRINT "If you dig on the ship, you might cause a leak.":RETURN
2280 PRINT "The ground is too hard here.":RETURN
2290 IF RO=1 THEN PRINT "You dig into the ground...You hit something! Its a coffin! When you open the":PRINT "coffin, a skeleton pops out, knife first, stabbing you!! The coffin was":PRINT "boobeytrapped!!":RETURN 2000
2300 IF RO=11 THEN PRINT "You dig but you don't find anything.":RETURN
2310 PRINT "You dig in the damp soil and soon you come upon a grating in the ground!":TR(13)=21:RETURN
2500 REM GO ROUTINE
2510 C$=LEFT$(C$,1)
2511 IF C$="S" AND RO=39 AND FLAG97=0 THEN PRINT "Are you sure you want to dive into the water?":RETURN
2520 IF C$="N" AND A(RO,1)=0 THEN PRINT:PRINT "You can't go that way.":RETURN
2530 IF C$="S" AND A(RO,2)=0 THEN PRINT:PRINT "You can't go that way.":RETURN
2540 IF C$="E" AND A(RO,3)=0 THEN PRINT:PRINT "You can't go that way.":RETURN
2550 IF C$="W" AND A(RO,4)=0 THEN PRINT:PRINT "You can't go that way.":RETURN
2560 IF C$="U" AND A(RO,5)=0 THEN PRINT:PRINT "You can't go that way.":RETURN
2570 IF C$="D" AND A(RO,6)=0 THEN PRINT:PRINT "You can't go that way.":RETURN
2580 IF C$="N" THEN RO=A(RO,1)
2590 IF C$="S" THEN RO=A(RO,2)
2600 IF C$="E" THEN RO=A(RO,3)
2610 IF C$="W" THEN RO=A(RO,4)
2620 IF C$="U" THEN RO=A(RO,5)
2630 IF C$="D" THEN RO=A(RO,6)
2640 IF RO=22 AND FLAG2=0 THEN SCORE=SCORE+20:FLAG2=1
2650 IF RO=23 AND FLAG3=0 THEN SCORE=SCORE+20:FLAG3=1
2660 RETURN 190
3000 REM GET ROUTINE
3010 IF C$="CHE" AND RO=23 THEN PRINT "The chest is too large and heavy for you to carry.":RETURN
3020 IF C$="CAP" AND RO=25 THEN PRINT "The captain moves out of the way just as you try to lift him. 'Hey, are you some":PRINT "kind of quier?'":RETURN
3030 IF C$="COO" AND RO=40 THEN PRINT "The cook slaps your hand. 'Not now, someone might see us.'":RETURN
3040 IF C$="STE" AND RO=40 THEN PRINT "The cook slaps your hand. 'Wait till it's done.'":RETURN
3041 IF C$="GAR" AND FLAG98=0 THEN PRINT "'No one messes with my stew makin's.' The cook pulls out a knife and cuts your":PRINT "throat. As you die, you hear the cook mutter, 'Now that'll teach him.'":RETURN 2000
3042 IF C$="GRA" THEN PRINT "The grate is secured to the grate.":RETURN
3050 FLAG7=0:FLAG8=0:OBJECT=0
3060 FOR I=1 TO 14
3070 IF TR(I)=200 THEN FLAG7=FLAG7+1
3080 NEXT I
3090 IF FLAG7>6 THEN PRINT:PRINT "You can't carry anymore.":RETURN
3100 FOR I=1 TO 12
3110 IF TR(I)=RO THEN FLAG8=1
3115 NEXT I
3120 IF FLAG8=0 THEN PRINT:PRINT "There is nothing here to pick up.":RETURN
3130 FOR I=1 TO 14
3140 IF C$=O1$(I) AND TR(I)=RO THEN OBJECT=I
3150 NEXT
3160 IF OBJECT=0 THEN PRINT:PRINT "I don't see that here.":RETURN
3170 TR(OBJECT)=200
3180 PRINT "Ok."
3190 IF TR(1)=200 AND FLAG9=0 THEN SCORE=SCORE+15:FLAG9=1
3200 IF TR(12)=200 AND FLAG10=0 THEN SCORE=SCORE+40:FLAG10=1
3201 IF TR(9)=200 AND FLAG55=0 THEN SCORE=SCORE+60:FLAG55=1
3210 RETURN
3500 REM DROP ROUTINE
3510 OBJECT=0:FOR I=1 TO 14
3520 IF C$=O1$(I) THEN OBJECT=I
3530 NEXT I
3540 IF OBJECT=0 THEN PRINT "You don't have that!":RETURN
3550 PRINT "Ok."
3560 TR(OBJECT)=RO
3570 RETURN
4000 REM OPEN ROUTINE
4010 IF C$<>"DRA" AND C$<>"BOX" AND C$<>"CHE" AND C$<>"DOO" THEN PRINT:PRINT "You can't open that.":RETURN
4011 PRINT
4015 IF C$="DRA" AND RO<>26 THEN PRINT "I don't see a dresser here.":RETURN
4016 IF C$="DRA" AND RO=26 AND FLAG4=1 THEN PRINT "The drawer is already open.":RETURN
4020 IF C$="DRA" AND FLAG4=0 AND RO=26 THEN PRINT "The drawer is now open. Inside you see your pirate book and your sword.":TR(5)=RO:TR(6)=RO:FLAG4=1:RETURN
4030 IF C$="BOX" AND TR(11)<>200 THEN PRINT "You don't even have the box.":RETURN
4040 IF C$="BOX" AND FLAG5=0 AND TR(11)=200 THEN PRINT "Ok, the box is now open. You find a thieve's pick inside.":TR(12)=RO:FLAG5=1:RETURN
4050 IF C$="BOX" AND FLAG5=1 AND TR(11)=200 THEN PRINT "The box is already open.":RETURN
4060 IF C$="CHE" AND RO<>23 THEN PRINT "I don't see the chest here.":RETURN
4070 IF C$="CHE" AND RO=23 AND FLAG70=0 THEN PRINT "You must unlock the chest.":RETURN
4080 IF C$="CHE" AND RO=23 AND FLAG70=1 THEN PRINT "The chest is already open.":RETURN
4090 IF C$="DOO" AND RO<>26 AND RO<>29 THEN PRINT "There is no door here.":RETURN
4100 IF C$="DOO" AND RO=26 AND FLAG11=0 THEN PRINT "The door swings open revealing a passage to the east.":A(26,3)=27:A(27,4)=26:FLAG11=1:RETURN
4110 IF C$="DOO" AND RO=26 AND FLAG11=1 THEN PRINT "The door is already open.":RETURN
4120 IF C$="DOO" AND RO=29 THEN PRINT "The door is locked and is the quarters of the captain.":RETURN
4130 RETURN
4500 REM unlock routine
4510 IF C$<>"DOO" AND C$<>"CHE" AND C$<>"GRA" AND C$<>"BOX" THEN PRINT "I cna't unlock that!":RETURN
4520 IF C$="DOO" THEN PRINT "You don't need to unlock that!":RETURN
4530 IF C$="BOX" THEN PRINT "You don't need to unlock that!":RETURN
4540 INPUT "With what";F$:F$=LEFT$(F$,3)
4550 IF C$="CHE" THEN 4600
4560 IF F$="KEY" THEN PRINT "The key doesn't fit the grate.":RETURN
4570 IF F$<>"PIC" THEN PRINT "You can't use that to unlock the grate!":RETURN
4580 IF FLAG60=1 THEN PRINT "The grate is already unlocked!":RETURN
4590 PRINT "The grate is now unlocked and opened! The grate reveals a passage downward.":A(21,6)=23:A(23,5)=21:FLAG60=1:RETURN
4600 IF F$="PIC" THEN PRINT "The pick doesn't fit the chest's lock.":RETURN
4610 IF F$<>"KEY" THEN PRINT "You can't use that to unlock the chest!":RETURN
4620 IF FLAG70=1 THEN PRINT "The chest is already unlocked!":RETURN
4630 PRINT "The chest is now unlocked and opened! The chest reveals a large, gold Spanish coin!":FLAG70=1:TR(9)=23:RETURN
5000 REM BUY ROUTINE
5010 INPUT "With what?";F$:F$=LEFT$(F$,3)
5020 IF C$<>"GAR" THEN PRINT "You can't buy that!":RETURN
5030 IF FLAG50=1 THEN PRINT "You've already bought the clove of garlic.":RETURN
5040 IF RO<>40 AND TR(4)<>200 THEN PRINT "I don't see the clove of garlic here.":RETURN
5050 IF F$<>"BAG" AND F$<>"SIL" THEN PRINT "'You fool! You'd think I'd seatle for less than silver for my garlic!' You are":PRINT " then decapitated!":RETURN 2000
5060 PRINT "The cook thinks it over then says, 'Well I guess it's a good deal so here.'":PRINT "Then the cooks hands you the clove and takes your bag of silver.":TR(4)=200:TR(14)=255:FLAG50=1
5070 RETURN
6000 REM CLIMB ROUTINE
6010 IF C$<>"MAS" AND C$<>"LAD" THEN PRINT "You can't climb that!":RETURN
6020 IF C$="MAS" AND RO=33 THEN PRINT "You climb up the mast...Soon you are in the crow's nest.":RO=35:PRINT:RETURN 190
6030 IF C$="MAS" AND RO=32 THEN PRINT "You climb up the mast...Soon you are in the crow's nest.":RO=34:PRINT:RETURN 190
6040 IF C$="MAS" THEN PRINT "I can't see a mast here.":RETURN
6050 IF C$="LAD" AND RO=39 THEN PRINT "If you want to go down the ladder just say so.":RETURN
6060 IF C$="LAD" THEN PRINT "I don't see a ladder here.":RETURN
6500 REM KNOCK ROUTINR
6510 INPUT "What do you want to knock on";F$:F$=LEFT$(F$,3)
6520 IF F$<>"DOO" THEN PRINT:PRINT "Awesome!!!":RETURN
6530 IF RO<>29 AND RO<>26 AND RO<>27 THEN PRINT:PRINT "I don't see a door here.":RETURN
6540 IF RO=29 AND FLAG12=0 THEN PRINT:PRINT "You knock on the door and wait. Soon the captain comes and ushers you in.":RO=25:FLAG12=1:PRINT:RETURN 190
6545 IF RO=29 AND FLAG12=1 THEN PRINT:PRINT "There is no answer.":RETURN
6550 PRINT:PRINT "Nothing happens."
6560 RETURN
7000 REM READ ROUTINE
7010 IF C$<>"BOO" AND C$<>"NOT" THEN PRINT:PRINT "You can't read that!":RETURN
7015 IF C$="NOT" THEN 7190
7020 IF TR(5)<>200 THEN PRINT:PRINT "You aren't even carrying the book!":RETURN
7030 PRINT
7060 PRINT "     This is Julius the Pirate's book on pirates. Included"
7070 PRINT "     are many entries on other pirates, ships and treasures."
7080 PRINT "     You skim through the book until you find a large entry"
7090 PRINT "     on an ancient Spanish coin worth thousands of dollar"
7100 PRINT "     and located on an island close by here. The lattitude"
7110 PRINT "     is 45 degrees and the longitude is 24 degrees."
7120 PRINT
7130 PRINT "     You notice that at the bottom of the last page that there"
7140 PRINT "     is some text reading:"
7150 PRINT
7160 PRINT "     For other refrences on Julius the Pirate, look under Julius"
7170 PRINT "     in a card catalog found in every ship library."
7180 RETURN
7190 IF RO<>42 THEN PRINT:PRINT "You can't see any note here.":RETURN
7200 PRINT
7210 PRINT "     To find a book in the many books in our wondourous library,"
7220 PRINT "     look under the subject and then go find the book. I hope"
7230 PRINT "     the books will entertain you and widen your horizons and"
7240 PRINT "     be sure to put the books back, OR ELSE!!!"
7250 PRINT
7260 PRINT "                                        -Jonny the Captain"
7270 RETURN
8000 REM LOOK UNDER ROUTINE
8010 IF RO<>42 THEN PRINT "You can't find information on that here. Try the ship's library open 24 hours a":PRINT "day!":RETURN
8020 PRINT
8030 PRINT "    You look through the card catalog, find a book on Julius"
8040 PRINT "    the Pirate, go get the book along the racks, and find this"
8050 PRINT "    entry:":PRINT
8060 PRINT "    Julius the Pirate was a great pirate, know for his daring"
8070 PRINT "    adventures and finding rare and valuable treasures. His"
8080 PRINT "    most famous treasure find was the Spanish gold coin, an"
8090 PRINT "    original coin since only one was made and it was minted in"
8100 PRINT "    in 1635. Julius died before he could collect the coin but"
8110 PRINT "    left the degrees of the island that the famous coin is"
8120 PRINT "    supposed to be on, in his pirate book passed on through his"
8130 PRINT "    generations. The last known recorded inheritered of the book"
8131 PRINT "    was Humphry Goodfellow, who died in a fire at his house. The"
8132 PRINT "    whereabouts of the book is unknown.":PRINT
8133 PRINT "    You read on till you find the last paragraph, which you are"
8134 PRINT "    shocked by:":PRINT
8135 PRINT "    Julius the Pirate was a great pirate, but he had a problem"
8136 PRINT "    with his left eye, which made him put marks 5 degrees west"
8137 PRINT "    of the original on longitude and latitude maps."
8138 PRINT "    MORE?"
8139 IF INKEY$="" THEN 8139
8140 RETURN
8150 IF RO<>42 THEN PRINT "You can't find information on that here. Try the ship's library open 24 hours a":PRINT "day!":RETURN
8160 PRINT
8170 PRINT "    Pirate Island is one of the most scariest islands in the"
8180 PRINT "    whole ocean. The name of the island has sent chills down"
8190 PRINT "    the spines of many young sea-going sailors. The bravest"
8200 PRINT "    captains have steered away from the island just to stay"
8210 PRINT "    out of the horrors that superstision have provided."
8220 PRINT "    Legends have it that the valuable Spanish gold coin rests"
8230 PRINT "    on Pirate Island but to date, no one has even tried to"
8240 PRINT "    find the coin. But then again, we don't record ALL voyages!"
8250 RETURN
8300 IF RO<>42 THEN PRINT "You can't find information on that here. Try the ship's library open 24 hours a":PRINT "day!":RETURN
8310 PRINT
8320 PRINT "    David Meny is one of the greatest pirates on the seven seas!"
8330 PRINT "    He has written his way out of a jail cell and found the Spanish"
8340 PRINT "    silver coin, second in value to only the famous gold coin made"
8350 PRINT "    by the Spainards. He continues to roam the free world"
8360 PRINT "    in search of adventure and intrigue."
8370 RETURN
8500 REM SWIM ROUTINE
8510 IF RO<>11 AND RO<>44 AND RO<>45 AND RO<>22 THEN PRINT "You can't swim here!":RETURN
8520 IF RO=44 OR RO=45 THEN PRINT "You swim around for awhile then end up where you are now.":RETURN
8530 IF RO=22 THEN PRINT "The water is to shallow to swim here.":RETURN
8540 PRINT "You swim out into the bay...."
8550 RO=44:RETURN 190
8990 END
9000 REM end of game
9010 PRINT "You have died. Too bad."
9020 INPUT "Would you like to start over";F$
9030 F$=LEFT$(F$,1)
9040 IF F$="Y" OR F$="y" THEN GOTO 9090
9050 IF F$="N" OR F$="n" THEN GOTO 9070
9060 PRINT "Please answer the question.":GOTO 9020
9070 PRINT:PRINT "Ok. See you next time on Pirate Island."
9080 FOR I=1 TO 100:NEXT I:CLS:END
9090 FLAG1=0:FLAG2=0:FLAG3=0:FLAG4=0:FLAG5=0:FLAG6=0:FLAG6=0:FLAG7=0:FLAG8=0:FLAG9=0:FLAG10=0:FLAG11=0:FLAG12=0:FLAG13=0:FLAG14=0:FLAG15=0:FLAG40=0:FLAG55=0:FLAG50=0:FLAG60=0:FLAG70=0:FLAG95=0:FLAG96=0:FLAG97=0:FLAG99=0:FLAG99=0:FLAG100=0
9100 RO=26
9110 TR(1)=35:TR(2)=25:TR(3)=8:TR(4)=40:TR(5)=255:TR(6)=255:TR(7)=23:TR(8)=40:TR(9)=255:TR(10)=36:TR(11)=45:TR(12)=255:TR(13)=255:TR(14)=255
9111 A(26,3)=0:A(27,4)=0:A(21,6)=0:A(23,5)=0
9120 CLS:GOTO 172
14000 REM PRINT ROOM CONTENTS
14020 FLAG99=0:JKH=0:FOR I=1 TO 14
14030 IF TR(I)=RO THEN JKH=I
14031 IF JKH<>I THEN 14040
14035 IF FLAG99=0 THEN PRINT O2$(JKH) ELSE PRINT O2$(JKH)
14036 FLAG99=1:JKH=I
14040 NEXT
14050 RETURN
15000 REM ROOM DESCRIPTION
15010 COLOR 0,7
15020 LOCATE 25,1
15030 PRINT "                                                                               "
15040 LOCATE 25,3:PRINT RO$(RO)
15050 LOCATE 25,50:PRINT "Score:";SCORE
15060 COLOR 7,0
15065 PRINT RO$(RO)
15066 IF RO>39 THEN 15090
15070 ON RO GOSUB 15110,15150,15150,15175,15210,15240,15280,15320,15210,15210,15360,15400,15210,15450,15490,15520,15560,15450,15450,15600,15640,15680,15720,15760,15800,15850,15890,15920,15950,16000,16040,16080,16120,16170,16170,16190,16230,16270,16320
15071 IF RO<>25 THEN 15080
15072 PRINT:PRINT "The captain looks at you the says, 'Well what's then longitude of Pirate":INPUT "Island";F1:INPUT "'Ok, now what's the latitude";F2
15073 PRINT "'You better be right or you'll regret you ever woke up this morning! Now leave.'":IF F1=F4 AND F2=F3 THEN FLAG98=1:START=1:RO=29:PRINT:RETURN 190
15074 START=1:RO=29:FLAG98=0:RETURN 190
15080 RETURN
15090 ST=RO-39:ON ST GOSUB 16360,16410,16440,16480,16510,16550
15100 RETURN
15110 REM ROOM 1
15120 PRINT "You are on the top of a high cliff overlooking the blue waters of the ocean. A"
15130 PRINT "path leads down. The soil is damp here."
15140 RETURN
15150 REM ROOM 2 AND 3
15160 PRINT "You are in a large rain forest."
15170 RETURN
15175 REM ROOM 4
15180 PRINT "You are on a small path leading out of a large rain forest to the west. In the"
15190 PRINT "center of the path there is a large tree."
15200 RETURN
15210 REM ROOM 5,9,10 AND 13
15220 PRINT "This is a thick forest covered by many trees, all of which are unclimbable."
15230 RETURN
15240 REM ROOM 6
15250 PRINT "You are standing below a large cliff. A path leads up to the cliff while an"
15260 PRINT "east/west path heads towards a cove and a meadow."
15270 RETURN
15280 REM room 7
15290 PRINT "You are near a large rock which has probably stood here for many hundreds of"
15300 PRINT "years. A small path leads south."
15310 RETURN
15320 REM room 8
15330 PRINT "You are in a large tree above a small path. The many leaves and branches"
15340 PRINT "prevent you from going any further up, while a large stump leads down."
15350 RETURN
15360 REM room 11
15370 PRINT "You are standing on the shore of an inland cove. The clear, blue waters of"
15380 PRINT "the ocean are of to the east, while a murky smell comes from the south. The"
15385 PRINT "soil is damp here."
15390 RETURN
15400 REM room 12
15410 PRINT "You are standing in a large meadow where birds sing and deer play. The tall,"
15420 PRINT "lush grass beneath you is stamped out, forming a path to the north, south and"
15430 PRINT "west."
15440 RETURN
15450 REM rooms 14,18 and 19
15460 PRINT "This is a murky, dismal swamp where hideous creatures hide when they are not on"
15470 PRINT "the hunt. A path leads to the west."
15480 RETURN
15490 PRINT "You are standing, or rather were standing, in a large pond of quick sand."
15500 PRINT "You are slowly...slowly...slowly...sinking."
15510 RETURN 9000
15520 REM room 16
15530 PRINT "You are walking along a beach path which surrounds the southwest part of the"
15540 PRINT "island. To the east you can see a beach, while to the west comes a murky smell."
15550 RETURN
15560 REM room 17
15570 PRINT "This is a sandy beachy coming out of the water. Palm tress line the shore and"
15580 PRINT "a path leads to the west and to the east. Another path heads north."
15590 RETURN
15600 REM room 20
15610 PRINT "You are standing in a dark cave leading out of a swamp. From the east comes"
15620 PRINT "squeaky noises (Maybe you aren't the only one on this island)."
15630 RETURN
15640 REM room 21
15650 PRINT "You are in a cave with many bats. The ground is damp here. ";:IF TR(4)<>200 THEN PRINT " The bats see you and attack.":PRINT "You have many bat bites and slowly...slowly...slowly die.":RETURN 9000
15660 PRINT " The bats, seeing your garlic, stay far back."
15670 RETURN
15680 REM room 22
15690 PRINT "You are in a shallow cove, so shallow that you can stand up. To the north there"
15700 PRINT "is a sandy beach, while to all other directions there is deep water."
15710 FLAG40=1:RETURN
15720 REM room 23
15730 PRINT "You are in a secret cave below a grate. From up above you can hear squeaky"
15740 PRINT "noises."
15750 RETURN
15760 REM room 24
15770 PRINT "This is the wheelhouse of the ship where the ship is directed. A navigator is"
15780 PRINT "here directing the wheel over the blue water of the ocean."
15790 RETURN
15800 REM room 25
15810 PRINT "You are in the captain's quarters, a large room with many sea equipment along"
15820 PRINT "the including a compass and an old relic made out of gold. There is a large,"
15830 PRINT "wooden desk here containg top-secret information."
15840 RETURN
15850 REM room 26
15860 PRINT "You are in your cabin, a small room with minor furnishments. The only"
15870 PRINT "furnishments are a bed and a dresser with only one drawer. A large wooden door"
15871 PRINT "is on the east wall."
15880 RETURN
15890 REM room 27
15900 PRINT "You are in the north end of a hallway running north to south."
15910 RETURN
15920 REM room 28
15930 PRINT "You are on the port section of the ship's deck. To the north there is the "
15940 PRINT "ship's wheelhouse. Also the mid deck is to the south."
15945 RETURN
15950 REM room 29
15960 PRINT "You are on the ship's port deck. To the west is another portdeck and to the"
15970 PRINT "south there is the mid deck. To the east there is a passageway leading to the"
15980 PRINT "wheelhouse and to the north there is a large door leading to the captain's"
15981 PRINT "quarters."
15990 RETURN
16000 REM room 30
16010 PRINT "You are in the crew's cabin portion of the ship. There are many bunks and gear"
16020 PRINT "here that belongs to other people, so don't touch it."
16030 RETURN
16040 REM room 31
16050 PRINT "This is a hallway running north to south. To the west is the crew's cabin while"
16060 PRINT "up there is the mid deck."
16070 RETURN
16080 REM room 32
16090 PRINT "You are on the ship's mid deck. Another mid deck is off to the east while the"
16100 PRINT "ship's port and starboard decks are off to the north and south. A large mast is"
16105 PRINT "here with a crow's nest on top of it."
16110 RETURN
16120 REM room 33
16130 PRINT "You are on one of two mid decks. This mid deck has a ladder leading down to the"
16140 PRINT "decks below. Also the port and starboards are off to the north and south while"
16150 PRINT "a large mast is above you. On the top of the mast, there is a crow's nest."
16160 RETURN
16170 REM room 34 and 35
16180 PRINT "You are in a crow's nest, a lookout site for land, storms and other ships."
16181 RETURN
16190 REM room 36
16200 PRINT "You are in the ship's mess hall where every day hungry crewmen are fed. To the"
16210 PRINT "south is the galley and to the east is a hallway."
16220 RETURN
16230 REM room 37
16240 PRINT "You are in a hallway running north to south. To the west is the ship's mess"
16250 PRINT "hall."
16260 RETURN
16270 REM room 38
16280 PRINT "You are on the starboard deck of the ship. To the east there is another"
16290 PRINT "starboard deck, while to the north there is a mid deck. To the south there is"
16300 PRINT "the ship's library."
16310 RETURN
16320 REM room 39
16330 PRINT "This is the ship's starboard deck. There is a ladder here, leading down to a"
16340 PRINT "hallway and a deck to the north and to the west."
16341 IF RO<>39 THEN 15100
16342 IF FLAG40<>1 THEN 15100
16343 PRINT
16344 IF RO=39 AND FLAG40=1 AND TR(9)=200 THEN PRINT "As you come back aboard the ship, the captain greets you. 'Welcome back, matey!":PRINT "Congradulations, you found the coin. Now we will all be":PRINT "rich!'":SCORE=SCORE+55:EGG=1:RETURN 2000
16345 IF RO=39 AND FLAG40=1 AND TR(9)<>200 THEN PRINT "As you come back aboard the ship, the captain greets you. 'Welcome back, matey!":PRINT "What? You didn't find the coin. I'll kill you for that!' The captain"
16346 PRINT "then pulls out his sword and kills you!":RETURN 2000
16350 RETURN
16360 REM room 40
16370 PRINT "This is the ship's galley where food is prepared. A large stove is against the"
16380 PRINT "south wall and many ingredients awaiting to be prepared in the stew the cook is"
16390 PRINT "making."
16400 RETURN
16410 REM room 41
16420 PRINT "You are in the south end of a hallway running north to south."
16430 RETURN
16440 REM room 42
16450 PRINT "You are in the ship's library with many books lining the wall. A card catalog is"
16460 PRINT "here and a card catalog reference note is pinned on top of the card catalog."
16470 RETURN
16480 REM room 43
16490 PRINT "You are under water. You are sinking in the soft sand......."
16500 RETURN 9000
16510 REM ROOM 44
16520 PRINT "You are swimming on the top of the water in an inland cove. Off to the east is"
16530 PRINT "the shore of the cove."
16540 RETURN
16550 REM room 45
16560 PRINT "You are on the bottom of a cove. The sand here is sturdy, thus you don't sink."
16570 RETURN
19990 END
20000 REM INITIALIZE
20010 RO=26:SCORE=0
20015 F3=45:F4=29
20020 FOR I=1 TO 45
20030 FOR J=1 TO 6
20040 READ A(I,J)
20050 NEXT J
20060 NEXT I
20070 FOR I=1 TO 14
20080 READ O1$(I),O2$(I),TR(I),IN$(I)
20090 NEXT I
20100 FOR I=1 TO 45
20110 READ RO$(I)
20120 NEXT I
20130 RETURN
25000 REM DATA STATEMENTS
25010 DATA 0,0,0,0,0,6:REM ROOM 1
25020 DATA 2,2,3,2,0,0:REM ROOM 2
25030 DATA 2,3,4,0,6,0:REM ROOM 3
25040 DATA 0,0,0,3,8,0:REM ROOM 4
25050 DATA 0,5,10,9,0,0:REM ROOM 5
25060 DATA 0,0,12,11,1,3:REM ROOM 6
25070 DATA 0,17,0,0,0,0:REM ROOM 7
25080 DATA 0,0,0,0,0,4:REM ROOM 8
25090 DATA 5,10,13,9,0,0:REM ROOM 9
25100 DATA 5,19,10,9,0,0:REM ROOM 10
25110 DATA 6,15,12,0,0,0:REM ROOM 11
25120 DATA 6,16,0,11,0,0:REM ROOM 12
25130 DATA 9,0,0,18,0,0:REM ROOM 13
25140 DATA 0,19,14,20,0,0:REM ROOM 14
25150 DATA 0,0,0,0,0,0:REM ROOM 15
25160 DATA 12,0,17,15,0,0:REM ROOM 16
25170 DATA 7,22,18,16,0,0:REM ROOM 17
25180 DATA 13,0,0,17,0,0:REM ROOM 18
25190 DATA 14,18,19,10,0,0:REM ROOM 19
25200 DATA 0,0,21,14,0,0:REM ROOM 20
25210 DATA 0,0,0,20,0,0:REM ROOM 21
25220 DATA 17,39,43,43,43,43:REM ROOM 22
25230 DATA 0,0,0,0,21,0:REM ROOM 23
25240 DATA 0,0,29,28,0,0:REM ROOM 24
25250 DATA 0,29,0,0,0,0:REM ROOM 25
25260 DATA 0,0,0,0,0,0:REM ROOM 26
25270 DATA 0,31,0,0,0,0:REM ROOM 27
25280 DATA 24,32,29,0,0,0:REM ROOM 28
25290 DATA 0,33,24,28,0,0:REM ROOM 29
25300 DATA 0,0,31,0,0,0:REM ROOM 30
25310 DATA 27,37,0,30,33,0:REM ROOM 31
25320 DATA 28,38,33,0,0,0:REM ROOM 32
25330 DATA 29,39,0,32,0,31:REM ROOM 33
25340 DATA 0,0,0,0,0,32:REM ROOM 34
25350 DATA 0,0,0,0,0,33:REM ROOM 35
25360 DATA 0,40,37,0,0,0:REM ROOM 36
25370 DATA 31,41,0,36,39,0:REM ROOM 37
25380 DATA 32,42,39,0,0,0:REM ROOM 38
25390 DATA 33,0,0,38,0,37:REM ROOM 39
25400 DATA 36,0,0,0,0,0:REM ROOM 40
25410 DATA 37,0,0,0,0,0:REM ROOM 41
25420 DATA 38,0,0,0,0,0:REM ROOM 42
25430 DATA 0,0,0,0,0,0:REM ROOM 43
25431 DATA 0,0,11,0,0,45:REM ROOM 44
25432 DATA 0,0,0,0,44,0:REM ROOM 45
25440 DATA "SHO","There is an old shovel here.",35,"a shovel"
25450 DATA "CAP","The captain of the ship is here.",25,""
25460 DATA "KEY","There is a shiny key laying near you.",8,"a key"
25470 DATA "GAR","There is a clove of garlic near you.",40,"a clove of garlic"
25480 DATA "BOO","There is a bounded book here.",255,"a book"
25490 DATA "SWO","A sharp sword is here.",255,"a sharp sword"
25500 DATA "CHE","There is an old pirate chest here.",23,""
25510 DATA "COO","The ship's cook is here, cooking a stew.",40,""
25520 DATA "COI","There is a Spanish coin here.",255,"a gold coin"
25530 DATA "TRA","There is a tray here full of discarded food.",36,"a tray"
25531 DATA "BOX","Near you, there is small box.",45,"a box"
25532 DATA "PIC","There is a thieve's pick here.",255,"a thieve's pick"
25533 DATA "GRA","There is a metal grate here.",255,""
25534 DATA "BAG","A bag of silver coins is here.",255,"a bag of silver coins"
25535 DATA High Cliff,Rain Forest,Rain Forest,Small Path,Thick Forest,Near Cliff
25540 DATA Rock,Up A Tree,Thick Forest,Thick Forest,Inland Cove,Meadow
25550 DATA Thick Forest,Swamp,Quicksand,Beach Path,Sandy Beach,Swamp,Swamp
25560 DATA Dark Cave,Bat Cave,Shallow Cove,Secret Cave,Wheelhouse
25570 DATA Captain's Quarters,Your Cabin,Hallway,Port Deck,Port Deck
25580 DATA Crew's Cabin,Hallway,Mid Deck,Mid Deck,Crow's Nest,Crow's Nest
25590 DATA Mess Hall,Hallway,Starboard Deck,Starboard Deck,Galley,Hallway
25600 DATA Ship's Library,Under Water,Top of the Bay,Bottom of the Bay,Hallway,Starboard Dec
