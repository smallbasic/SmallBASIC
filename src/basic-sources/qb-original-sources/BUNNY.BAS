10 KEY OFF: SCREEN 0: COLOR 7, 1: PALETTE: CLS : RANDOMIZE TIMER
20 NO = 9: R = 5: ND = 6: NR = 27: MOVE = 0: NI = 2
30 DIM R$(NR), OB$(NO), O2$(NO), OB(NO), MA(28, 6), C$(11)
35 CLS : GOSUB 27000: CLS
40 IF RND > .5 THEN PRINT "I'll be back in a sec..." ELSE PRINT "Please wait..."
50 GOSUB 25000: GOSUB 26000
55 VIEW PRINT 1 TO 25: CLS
60 COLOR 0, 7: LOCATE 25, 41: PRINT "Score:";: COLOR 7, 1: VIEW PRINT 1 TO 24: LOCATE 1, 1
90 GOSUB 700
91 IF FFF THEN FFF = 0: GOTO 820
92 GOSUB 600: GOSUB 300
100 IF VAMP THEN VAMPT = VAMPT + 1: IF VAMPT = 2 THEN PRINT "The vampire decides to fang you.": GOTO 810
102 IF R = 25 AND NOT DBUN THEN RABTI = RABTI + 1 ELSE RABTI = 0
104 IF R = 25 AND NOT DBUN THEN IF RABTI = 3 THEN PRINT "The vampire bunny nibbles a little on your leg.": GOTO 810
108 IF C$(1) <> "" THEN CM$ = C$(1): BF = 0: GOTO 126
109 CM$ = "": LINE INPUT ">"; CM$: IF CM$ = "" THEN PRINT "What's that?": GOTO 109
110 FOR I = 1 TO LEN(CM$): G$ = MID$(CM$, I, 1): IF ASC(G$) > 96 AND ASC(G$) < 123 THEN G = ASC(G$) - 32: MID$(CM$, I) = CHR$(G)
111 NEXT
114 FOR I = 1 TO LEN(CM$)
115 IF MID$(CM$, I, 4) = "THE " THEN G$ = LEFT$(CM$, I - 1): GG$ = MID$(CM$, I + 4): CM$ = G$ + GG$: GOTO 115 ELSE 116
116 NEXT: C = 0
117 Z = LEN(CM$): L = 0: FOR I = 1 TO Z
118 L = L + 1
119 IF MID$(CM$, L, 3) = "AND" THEN C = C + 1: C$(C) = LEFT$(CM$, L - 2): G$ = MID$(CM$, L + 4): CM$ = G$: L = 1
120 IF MID$(CM$, L, 4) = "THEN" THEN C = C + 1: C$(C) = LEFT$(CM$, L - 2): G$ = MID$(CM$, L + 5): CM$ = G$: L = 1
121 IF MID$(CM$, L, 1) = "." THEN IF LEN(CM$) <> L THEN C = C + 1: C$(C) = LEFT$(CM$, L - 1): G$ = MID$(CM$, L + 1): CM$ = G$: L = 1
122 IF MID$(CM$, L, 2) = "  " THEN G$ = LEFT$(CM$, L - 1): GG$ = MID$(CM$, L + 3): CM$ = G$ + GG$: GOTO 122
123 IF LEFT$(CM$, 1) = " " THEN CM$ = RIGHT$(CM$, LEN(CM$) - 1)
124 IF RIGHT$(CM$, 1) = "." THEN CM$ = LEFT$(CM$, LEN(CM$) - 1)
125 NEXT: C$(C + 1) = CM$: IF C$(1) <> "" THEN CM$ = C$(1)
126 BF = 0: FOR I = 2 TO 10: C$(I - 1) = C$(I): C$(I) = "": NEXT
200 C = 0: V$ = "": N$ = "": W1$ = "": W2$ = ""
202 C = C + 1: IF C > LEN(CM$) THEN 208
204 W$ = MID$(CM$, C, 1): IF W$ = " " THEN 208
206 V$ = V$ + W$: GOTO 202
208 C = C + 1: IF C > LEN(CM$) THEN 214
210 W$ = MID$(CM$, C, 1): IF W$ = " " THEN 214
212 N$ = N$ + W$: GOTO 208
214 C = C + 1: IF C > LEN(CM$) THEN 220
216 W$ = MID$(CM$, C, 1): IF W$ = " " THEN 220
218 W1$ = W1$ + W$: GOTO 214
220 C = C + 1: IF C > LEN(CM$) THEN 226
222 W$ = MID$(CM$, C, 1): IF W$ = " " THEN 226
224 W2$ = W2$ + W$: GOTO 220
226 IF V$ = "" THEN PRINT "Come again?": GOTO 109
227 IF LEN(V$) > 5 THEN V$ = LEFT$(V$, 5)
228 IF LEN(N$) > 5 THEN N$ = LEFT$(N$, 5)
229 IF LEN(W1$) > 5 THEN W1$ = LEFT$(W1$, 5)
230 IF LEN(W2$) > 5 THEN W2$ = LEFT$(W2$, 5)
234 IF V$ = "G" OR V$ = "AGAIN" THEN CM$ = VVV$: V$ = V1$: N$ = VV$: W1$ = V2$: W2$ = V3$
235 IF N$ = "IT" THEN N$ = VV$
236 VV$ = N$: VVV$ = CM$: V1$ = V$: V2$ = W1$: V3$ = W2$
240 IF N$ = "SILVE" THEN N$ = "CROSS"
250 IF N$ = "SCRAP" OR N$ = "SHEET" THEN N$ = "PAPER"
255 IF W1$ = "SCRAP" OR W1$ = "SHEET" THEN W1$ = "PAPER"
260 IF N$ = "QUILL" THEN N$ = "PEN"
262 IF W1$ = "QUILL" THEN W1$ = "PEN"
265 IF N$ = "GRAY" THEN N$ = "OOZE"
266 IF N$ = "WOODE" THEN N$ = "CRANK"
267 IF N$ = "WEBS" THEN N$ = "COBWE"
268 IF W2$ = "GRAY" THEN W2$ = "OOZE"
269 IF W1$ = "SILVE" THEN S1$ = "CROSS"
290 MOVE = MOVE + 1: GOSUB 300: GOTO 320
300 A = CSRLIN: VIEW PRINT 1 TO 25: LOCATE 25, 1: COLOR 0, 7: PRINT LO$ + SPACE$(40 - LEN(LO$));
305 IF SCORE > 9 THEN S = 12 ELSE S = 11
309 IF MOVE > 999 THEN V = 9 ELSE IF MOVE > 99 THEN V = 10 ELSE IF MOVE > 9 THEN V = 11 ELSE V = 12
310 LOCATE 25, 47: PRINT STR$(SCORE) + SPACE$(S);
315 LOCATE 25, 60: PRINT "Moves:"; MOVE; SPACE$(V-1);: VIEW PRINT 1 TO 24: LOCATE A, 1: COLOR 7, 1: RETURN
320 IF R < 11 AND R <> 7 THEN 340
325 IF OB(4) = -1 OR OB(4) = R THEN 340
327 IF NOT LAMP THEN 330
328 IF OB(1) = -1 OR OB(1) = R THEN 340
330 IF CM$ <> "LIGHT LAMP" AND CM$ <> "TURN ON LAMP" THEN PRINT "You are in total darkness.": GOSUB 300
FOR TEMP = 1 TO 11: C$(TEMP) = "": NEXT: GOTO 109
340 IF CM$ = "TALK INTO WELL" OR CM$ = "TALK TO WELL" OR CM$ = "TALK WELL" THEN IF R = 4 THEN PRINT "Echo...echo....echo......echo...": GOTO 100
400 IF V$ = "PUT" AND N$ = "ON" THEN V$ = "WEAR": N$ = W1$
402 IF V$ = "GET" AND N$ = "ON" THEN V$ = "WEAR": N$ = W1$
404 IF V$ = "TAKE" AND N$ = "OFF" THEN V$ = "UNWEA": N$ = W1$
405 IF V$ = "GET" AND N$ = "OFF" THEN V$ = "UNWEA": N$ = W1$
407 IF V$ = "THROW" AND N$ = "WATER" THEN V$ = "POUR"
408 IF V$ = "THROW" AND N$ = "BUCKE" THEN V$ = "POUR"
409 IF VAMP AND V$ = "POUR" THEN IF N$ = "WATER" OR N$ = "BUCKE" THEN W1$ = "ON": W2$ = "VAMPI"
410 IF V$ = "TAKE" AND W1$ = "OFF" THEN V$ = "UNWEA"
420 IF V$ = "GET" AND W1$ = "OFF" THEN V$ = "UNWEA"
590 GOTO 2000
600 FOR I = 0 TO NO - 1: IF OB(I) = R THEN PRINT "You see "; OB$(I); " here."
610 NEXT: RETURN
700 IF R < 11 AND R <> 7 THEN 710
701 IF OB(4) = -1 OR OB(4) = R THEN 710
702 IF NOT LAMP THEN 704
703 IF OB(1) = -1 OR OB(1) = R THEN 710
704 LO$ = "Darkness": GOSUB 300: RETURN
710 IF R = 100 THEN LO$ = "Outside Graveyard": GOSUB 300: GOTO 25400
715 ON R GOSUB 40000, 40060, 40080, 40100, 40140, 40160, 40180, 40200, 40220, 40250, 40270, 40290, 40300, 40310, 40330, 40350, 40360, 40370, 40390, 40410, 40420, 40430, 40440, 40450, 40470, 40490, 40510
720 RETURN
750 IF NOT LAMP THEN 770
760 IF OB(1) = -1 OR OB(1) = R THEN RETURN
770 LO$ = "Darkness": GOSUB 300: PRINT "Uh-oh, now you can't see!": RETURN
800 FOR TEMP = 1 TO 11: C$(TEMP) = "": NEXT: MOVE = MOVE - 1: GOSUB 300: RETURN
810 PRINT "          ***** You have died *****": GOSUB 25420: GOTO 3440
820 PRINT "          ***** You have died *****": GOSUB 25420: PRINT "New game or undo(y/n/u)?"
830 I$ = INKEY$: IF I$ <> "Y" AND I$ <> "y" AND I$ <> "N" AND I$ <> "n" AND I$ <> "U" AND I$ <> "u" THEN 830
840 IF I$ = "Y" OR I$ = "y" THEN RUN ELSE IF I$ = "U" OR I$ = "u" THEN GOSUB 800: GOTO 90
850 COLOR 7, 0: CLS : KEY ON: END
1000 IF NO = 0 THEN RETURN
1010 FOR I = 0 TO NO - 1
1020 IF O2$(I) = N$ THEN FL = 1: RO = OB(I): GOTO 1050
1030 NEXT
1040 FL = 0: RETURN
1050 RETURN
2000 IF V$ <> "GO" AND V$ <> "WALK" AND V$ <> "RUN" AND V$ <> "CRAWL" AND V$ <> "N" AND V$ <> "S" AND V$ <> "CLIMB" AND V$ <> "E" AND V$ <> "W" AND V$ <> "U" AND V$ <> "D" THEN 2500
2010 IF N$ = "NORTH" OR V$ = "N" THEN DI = 0: GOTO 2400
2020 IF N$ = "SOUTH" OR V$ = "S" THEN DI = 1: GOTO 2400
2030 IF N$ = "EAST" OR V$ = "E" THEN DI = 2: GOTO 2400
2040 IF N$ = "WEST" OR V$ = "W" THEN DI = 3: GOTO 2400
2050 IF N$ = "UP" OR N$ = "UPWAR" OR V$ = "U" THEN DI = 4: GOTO 2400
2060 IF N$ = "DOWN" OR N$ = "DOWNW" OR V$ = "D" THEN DI = 5: GOTO 2400
2070 IF R = 1 AND N$ = "COFFI" THEN PRINT "Are you crazy?": GOTO 100
2080 IF R = 3 THEN IF N$ = "STREA" OR N$ = "RIVER" THEN V$ = "S": GOTO 2000
2090 IF R = 6 THEN IF N$ = "STREA" OR N$ = "RIVER" THEN V$ = "N": GOTO 2000
2100 IF R = 9 AND N$ = "HOLE" THEN V$ = "D": GOTO 2000
2110 IF R = 4 AND N$ = "WELL" THEN V$ = "D": GOTO 2000
2120 IF R = 7 AND N$ = "CREVI" THEN V$ = "S": GOTO 2000
2130 IF R = 8 AND N$ = "HOLE" THEN V$ = "D": GOTO 2000
2135 IF R = 7 AND N$ = "ROPE" THEN DI = 4: GOTO 2400
2136 IF R = 4 AND N$ = "ROPE" AND ROPE THEN DI = 5: GOTO 2400
2140 IF R = 27 AND N$ = "HOLE" THEN V$ = "U": GOTO 2000
2145 IF R = 14 AND N$ = "HOLE" THEN V$ = "U": GOTO 2000
2148 IF R = 2 AND N$ = "HOLE" THEN V$ = "E": GOTO 2000
2200 IF R = 3 AND N$ = "HOLE" THEN V$ = "W": GOTO 2000
2390 PRINT "You can't go that way!": GOSUB 800: GOTO 109
2400 IF MA(R, DI) > 0 AND MA(R, DI) < 128 THEN R = MA(R, DI): GOTO 90
2402 IF MA(R, DI) = 203 THEN PRINT "The ooze blocks your path.": GOTO 100
2404 IF MA(R, DI) = 204 THEN PRINT "The bunny won't let you!": GOTO 100
2405 IF MA(R, DI) = 200 THEN PRINT "As you enter the stream, your mouth gets a little water in it.  Suddenly you    have convulsions, choke, and drown.": GOTO 820
2408 IF MA(R, DI) = 201 AND NOT ROPE THEN 2390
2409 IF MA(R, DI) = 201 AND NOT LAMP THEN PRINT "It's too dark down there.": GOTO 100
2410 IF MA(R, DI) = 201 THEN IF OB(1) <> 7 AND OB(1) <> -1 THEN PRINT "It's too dark down there.": GOTO 100
2411 IF MA(R, DI) = 240 AND NI > 1 THEN PRINT "You're too heavy to climb up the well.": GOTO 100
2412 IF MA(R, DI) = 240 THEN PRINT "You climb up the rope.": R = 4: GOTO 90
2413 IF MA(R, DI) = 201 AND NI > 1 THEN PRINT "As you attempt to climb down the rope, it snaps, and you plunge down the well.": GOTO 820
2415 IF MA(R, DI) = 201 THEN PRINT "You make your way down the rope.": R = 7: GOTO 90
2419 IF MA(R, DI) = 700 THEN PRINT "The vampire, unfortunately, gives you a nasty bite.": GOTO 810
2420 IF MA(R, DI) = 314 AND NOT WEAC THEN PRINT "The skeleton pushes you away from the hole.": GOTO 100
2430 IF MA(R, DI) = 314 THEN PRINT "The skeleton allows you to enter.": R = 14: GOTO 90
2480 GOTO 2390
2500 IF V$ <> "GET" AND V$ <> "TAKE" THEN 2580
2510 ZH = 0: GOSUB 1000: IF N$ = "WATER" THEN IF OB(2) = -1 THEN V$ = "FILL": N$ = "BUCKE": GOTO 2000
2520 IF FL = 0 THEN PRINT "You can't get that!": GOSUB 800: GOTO 109
2530 IF RO = -1 THEN PRINT "You already have that!": GOSUB 800: GOTO 109
2550 IF RO <> R THEN PRINT "You can't get that!": GOSUB 800: GOTO 109
2560 IF NI = 5 THEN PRINT "You carrying too much!": GOSUB 800: GOTO 109
2565 IF NOT CROS AND I = 3 THEN CROS = -1: SCORE = SCORE + 10: GOSUB 300
2566 IF NOT CARP AND I = 0 THEN CARP = -1: SCORE = SCORE + 2: GOSUB 300
2570 NI = NI + 1: OB(I) = -1: PRINT "Taken.": GOTO 100
2580 IF V$ = "HINT" OR V$ = "HELP" THEN PRINT "Look at the README file."
2581 IF V$ = "TURN" AND W1$ = "OFF" THEN V$ = "UNLIG"
2582 IF V$ = "TURN" AND W1$ = "ON" THEN V$ = "LIGHT"
2585 IF V$ = "TURN" AND N$ = "OFF" THEN V$ = "UNLIG": N$ = W1$
2586 IF V$ = "TURN" AND N$ = "ON" THEN V$ = "LIGHT": N$ = W1$
2588 IF V$ = "PUT" AND N$ = "DOWN" THEN V$ = "DROP": N$ = W1$: GOTO 2500
2590 IF V$ = "PICK" AND N$ = "UP" THEN V$ = "TAKE": N$ = W1$: GOTO 2500
2600 IF V$ <> "DROP" THEN 2700
2605 IF W1$ <> "" THEN V$ = "PUT": GOTO 2000
2610 GOSUB 1000
2620 IF FL = 0 THEN PRINT "You don't have that!": GOSUB 800: GOTO 109
2635 IF RO = -2 THEN RO = -1
2640 IF RO <> -1 THEN PRINT "You don't have that!": GOSUB 800: GOTO 109
2650 NI = NI - 1: OB(I) = R: PRINT "Dropped.": GOTO 100
2700 IF V$ <> "INVEN" AND V$ <> "I" THEN 2800
2710 FL = 0: PRINT "You have:"
2715 IF WEAC THEN PRINT " a silver cross (being worn)": FL = 1
2716 IF BUCK AND OB(2) = -1 THEN PRINT " a bucket (with water)": FL = 1
2717 IF LAMP AND OB(1) = -1 THEN PRINT " a lamp (providing light)": FL = 1
2718 IF OB(4) = -1 THEN PRINT " a torch (providing light)": FL = 1
2720 FOR I = 0 TO NO - 1
2725 IF WEAC AND I = 3 THEN 2740
2726 IF BUCK AND I = 2 AND OB(I) = -1 THEN GOTO 2740
2727 IF LAMP AND I = 1 AND OB(I) = -1 THEN GOTO 2740
2728 IF I = 4 AND OB(I) = -1 THEN 2740
2730 IF OB(I) = -1 THEN PRINT " "; OB$(I): FL = 1
2740 NEXT
2745 IF FL = 0 THEN PRINT " nothing"
2760 GOTO 100
2800 IF V$ <> "LOOK" AND V$ <> "L" THEN 2900
2810 IF N$ <> "" THEN 2901
2820 GOTO 90
2900 IF V$ <> "EXAMI" AND V$ <> "SEARC" THEN 3400
2901 IF N$ = "FIELD" AND R <> 5 THEN PRINT "There isn't a field here!": GOSUB 800: GOTO 109
2904 IF N$ = "GROUN" OR N$ = "FLOOR" OR N$ = "FIELD" THEN IF R = 5 AND NOT CARS THEN CARS = -1: PRINT "You find a carrot.": OB(0) = 5: GOTO 100
2906 IF N$ = "GROUN" OR N$ = "FLOOR" OR N$ = "FIELD" THEN PRINT "You see nothing unusual.": GOTO 100
2908 IF N$ = "COFFI" AND R = 1 THEN PRINT "It is carved of marble.": GOTO 100
2909 IF N$ = "SKELE" AND R = 9 THEN PRINT "It looks pretty bony.": GOTO 100
3000 IF N$ = "WELL" AND R = 4 THEN PRINT "It has a pole connected to some rope and a wooden crank.  The well is dark and  empty.": GOTO 100
3001 IF N$ = "ROPE" THEN IF R = 4 OR R = 7 THEN PRINT "It doesn't look very sturdy.": GOTO 100
3002 IF N$ = "WELL" AND R = 7 THEN PRINT "It is full of cracks.": GOTO 100
3004 IF N$ = "OOZE" AND R = 19 AND NOT DGRA THEN PRINT "It is big, ugly, and slimy.": GOTO 100
3005 IF N$ = "COBWE" AND NOT BCOB THEN PRINT "The are blocking the east passage.": GOTO 100
3006 IF N$ = "LEVER" AND R = 26 THEN PRINT "It is rusty, but useable.": GOTO 100
3008 IF N$ = "BUNNY" AND NOT DBUN AND R = 25 THEN PRINT "It looks extremely vicious for its small size.": GOTO 100
3009 IF N$ = "VAMPI" AND VAMP THEN PRINT "It looks at you with an unnerving gaze.": GOTO 100
3010 IF N$ = "TOMBS" OR N$ = "GRAVE" THEN IF R < 11 AND R <> 7 THEN PRINT "Many are old and rotted, and some which are blank make you shiver.": GOTO 100
3011 IF N$ = "CRANK" AND R = 4 THEN PRINT "It is cracking, but usable.": GOTO 100
3012 IF N$ = "STREA" OR N$ = "RIVER" THEN IF R = 6 OR R = 3 THEN PRINT "Though it has no life in it, it looks shallow enough to cross.": GOTO 100
3013 IF N$ = "POLE" AND R = 4 THEN PRINT "It has many holes in it.": GOTO 100
3014 IF N$ = "MIRRO" AND R = 24 THEN IF BRMI THEN PRINT "It is broken into shards, and there is a passage behind it." ELSE PRINT "It is not well polished."
3015 IF N$ = "MIRRO" AND R = 24 THEN GOTO 100
3016 IF N$ = "WALLS" OR N$ = "WALL" THEN PRINT "There are enough of them.": GOTO 100
3200 GOSUB 1000
3202 IF RO <> R AND RO <> -1 AND OB(I) <> (R - 128) THEN PRINT "That's not here!": GOSUB 800: GOTO 109
3204 IF N$ = "CARRO" THEN PRINT "It looks nice and juicy.": GOTO 100
3208 IF N$ = "LAMP" THEN IF LAMP THEN PRINT "The lamp lets out a light glow." ELSE PRINT "The lamp has been turned off."
3210 IF N$ = "LAMP" THEN 100
3215 IF N$ = "BUCKE" THEN IF BUCK THEN PRINT "The bucket is filled with water." ELSE PRINT "The bucket is empty."
3216 IF N$ = "BUCKE" THEN 100
3218 IF N$ = "CROSS" THEN PRINT "It is rough and tarnished.": GOTO 100
3300 IF N$ = "PAPER" THEN PRINT "It has some writing on it.": GOTO 100
3310 IF N$ = "KEY" THEN PRINT "It is fairly shiny.": GOTO 100
3312 IF N$ = "TORCH" THEN PRINT "It is burning fairly slowly.": GOTO 100
3314 IF N$ = "PEN" THEN PRINT "It does not have any ink.": GOTO 100
3316 IF N$ = "APPLE" THEN PRINT "It looks a little too wormy to be edible.": GOTO 100
3318 PRINT "You see nothing unusual.": GOTO 100
3400 IF V$ <> "QUIT" AND V$ <> "Q" AND V$ <> "END" THEN 3500
3405 GOSUB 25420
3410 INPUT "Are you sure you want to quit <Y/N>"; QU$
3420 IF QU$ = "N" OR QU$ = "n" THEN GOSUB 800: GOTO 109
3430 IF QU$ <> "Y" AND QU$ <> "y" THEN 3410
3440 PRINT "Play again <Y/N>?"
3450 I$ = INKEY$: IF I$ <> "Y" AND I$ <> "y" AND I$ <> "N" AND I$ <> "n" THEN 3450
3455 IF I$ = "Y" OR I$ = "y" THEN RUN ELSE COLOR 7, 0: CLS : KEY ON: END
3500 IF V$ <> "ENTER" THEN 3510
3505 V$ = "GO": GOTO 2000
3510 IF V$ <> "JUMP" AND V$ <> "LEAP" THEN 3640
3520 IF N$ = "" THEN PRINT "Wheeeeee!": GOTO 100
3530 IF N$ <> "OVER" THEN PRINT "That won't do any good.": GOSUB 800: GOTO 109
3535 IF R = 19 AND NOT DGRA AND W1$ = "OOZE" OR W1$ = "GREY" THEN PRINT "The ooze expands, forcing you to postpone your jump.": GOSUB 800: GOTO 109
3538 IF R = 25 AND NOT DBUN AND W1$ = "BUNNY" OR W1$ = "RABBI" THEN PRINT "The bunny manages to bite your leg.": GOTO 820
3600 IF W1$ <> "STREA" AND W1$ <> "RIVER" THEN PRINT "That won't do anything!": GOTO 100
3610 IF R <> 3 AND R <> 6 THEN PRINT "That won't do anything!": GOTO 100
3620 IF R = 3 THEN R = 6 ELSE R = 3
3630 PRINT "You leap over the stream.": GOTO 90
3640 IF V$ <> "READ" THEN 3680
3650 IF N$ <> "PAPER" THEN PRINT "You can't do that!": GOSUB 800: GOTO 109
3660 IF OB(6) <> -1 THEN PRINT "You don't have that!": GOSUB 800: GOTO 109
3670 PRINT "It says, `Pushing is better than pulling'.": GOTO 100
3680 IF V$ <> "LIGHT" THEN 3730
3685 IF N$ = "COBWE" THEN 3697
3690 GOSUB 1000: IF FL = 0 THEN PRINT "You don't have that!": GOSUB 800: GOTO 109
3692 IF OB(I) <> -1 THEN PRINT "You don't have that!": GOSUB 800: GOTO 109
3695 IF N$ <> "TORCH" AND N$ <> "COBWE" AND N$ <> "LAMP" THEN PRINT "You can't do that!": GOSUB 800: GOTO 109
3697 IF N$ = "COBWE" AND R = 23 AND BCOB = 0 THEN PRINT "The cobwebs light and burn into dust.": SCORE = SCORE + 5: GOSUB 300: BCOB = -1: MA(23, 2) = 24: GOTO 100
3699 IF N$ = "COBWE" THEN PRINT "You can't do that now!": GOSUB 800: GOTO 109
3700 IF N$ = "TORCH" AND OB(4) = -1 THEN PRINT "It's already lit!": GOTO 100
3710 IF N$ = "LAMP" AND OB(1) = -1 AND LAMP THEN PRINT "It's already lit!": GOTO 100
3720 IF N$ = "LAMP" AND NOT LAMP THEN PRINT "The lamp lets out a warm glow.": LAMP = -1: IF LO$ = "Darkness" THEN 90 ELSE 100
3730 IF V$ <> "LOWER" THEN 3780
3740 IF N$ <> "ROPE" AND N$ <> "WELL" THEN PRINT "You can't do that now!": GOSUB 800: GOTO 109
3750 IF R <> 4 THEN PRINT "You can't do that now!": GOSUB 800: GOTO 109
3760 IF ROPE THEN PRINT "It already is!": GOSUB 800: GOTO 109
3770 ROPE = -1: PRINT "The rope goes down into the well.": GOTO 100
3780 IF V$ <> "RAISE" AND V$ <> "LIFT" THEN 3830
3790 IF N$ <> "ROPE" AND N$ <> "WELL" THEN PRINT "You can't do that now!": GOSUB 800: GOTO 109
3800 IF R <> 4 THEN PRINT "You can't do that now!": GOSUB 800: GOTO 109
3810 IF NOT ROPE THEN PRINT "It already is!": GOSUB 800: GOTO 109
3820 ROPE = 0: PRINT "The rope rises from the well.": GOTO 100
3830 IF V$ <> "FILL" THEN 3880
3840 IF N$ <> "BUCKE" THEN PRINT "You can't do that now!": GOSUB 800: GOTO 109 ELSE IF OB(2) <> -1 THEN PRINT "You can't do that now!": GOSUB 800: GOTO 109
3850 IF R <> 3 AND R <> 6 THEN PRINT "You can't do that now!": GOSUB 800: GOTO 109
3860 IF BUCK THEN PRINT "It already is full!": GOSUB 800: GOTO 109
3870 PRINT "The bucket fills with water.": BUCK = -1: GOTO 100
3880 IF V$ <> "UNLIG" THEN 3940
3890 GOSUB 1000: IF FL = 0 THEN PRINT "You don't have that!": GOSUB 800: GOTO 109
3892 IF OB(I) <> -1 THEN PRINT "You don't have that!": GOSUB 800: GOTO 109
3900 IF N$ <> "TORCH" AND N$ <> "LAMP" THEN PRINT "You can't do that!": GOSUB 800: GOTO 109
3910 IF N$ = "TORCH" AND OB(4) = -1 THEN PRINT "It refuses to go out.": GOTO 100
3920 IF N$ = "LAMP" AND OB(1) = -1 AND NOT LAMP THEN PRINT "It already is!": GOTO 100
3930 IF N$ = "LAMP" AND OB(1) = -1 THEN PRINT "The lamp turns off with a click.": LAMP = 0: GOTO 100
3940 IF V$ <> "OPEN" THEN 4000
3950 IF N$ <> "COFFI" THEN PRINT "You can't do that now!": GOSUB 800: GOTO 109
3960 IF R <> 1 THEN PRINT "You can't do that now!": GOSUB 800: GOTO 109
3970 IF OCOF THEN PRINT "It's already open!": GOSUB 800: GOTO 109
3980 IF DVAM THEN PRINT "The coffin opens.": OCOF = -1: GOTO 100
3990 OCOF = -1: PRINT "As you open the coffin, a vampire reaches out!": VAMP = -1: MA(1, 1) = 700: GOTO 100
4000 IF V$ <> "CLOSE" THEN 4100
4010 IF N$ <> "COFFI" THEN PRINT "You can't do that now!": GOSUB 800: GOTO 109
4020 IF R <> 1 THEN PRINT "You can't do that now!": GOSUB 800: GOTO 109
4030 IF NOT OCOF THEN PRINT "It's not open!": GOSUB 800: GOTO 109
4032 IF VAMP THEN PRINT "The coffin slams shut in the vampire's face.": VAMPT = 0: VAMP = 0: OCOF = 0: MA(1, 1) = 4: GOTO 100
4034 PRINT "The coffin closes.": OCOF = 0: GOTO 100
4100 IF V$ <> "POUR" AND V$ <> "EMPTY" AND V$ <> "SPLAS" THEN 4130
4110 IF N$ <> "WATER" AND N$ <> "BUCKE" THEN PRINT "You can't pour that!": GOSUB 800: GOTO 109
4112 IF OB(2) <> -1 THEN PRINT "You don't have that!": GOSUB 800: GOTO 109
4114 IF NOT BUCK THEN PRINT "You can't do that now!": GOSUB 800: GOTO 109
4116 IF W2$ = "" THEN BUCK = 0: PRINT "You empty the bucket.": GOTO 100
4118 IF W2$ = "OOZE" OR W2$ = "GREY" AND R = 19 AND DGRE = 0 THEN PRINT "The ooze dramatically grows, splattering you with a deadly force.": GOTO 820
4120 IF W2$ = "RABBI" OR W2$ = "BUNNY" AND R = 24 AND DBUN = 0 THEN PRINT "The bunny moves, avoiding the splash of water.": BUCK = 0: GOTO 100
4122 IF W2$ = "APPLE" THEN IF OB(8) = -1 OR OB(8) = R THEN PRINT "The apple is splashed with water.": BUCK = 0: APSO = -1: GOTO 100
4123 IF W2$ = "CARRO" THEN IF OB(0) = -1 OR OB(0) = R THEN PRINT "The carrot is splashed with water.": SCORE = SCORE + 10: GOSUB 300: BUCK = 0: CASO = -1: GOTO 100
4124 IF W2$ = "SKELE" AND R = 9 THEN PRINT "The water flies through the skeltal body.": BUCK = 0: GOTO 100
4125 IF W2$ = "VAMPI" AND VAMP THEN PRINT "The vampire screams and gently dissolves in the stream of water.": MA(1, 1) = 4: BUCK = 0: VAMP = 0: OB(3) = 1: GOTO 100
4128 PRINT "I'm not so sure about that.": GOSUB 800: GOTO 109
4130 IF V$ <> "WEAR" THEN 4150
4140 IF N$ <> "CROSS" THEN PRINT "You can't do that!": GOSUB 800: GOTO 109
4142 IF OB(3) <> -1 THEN PRINT "You don't have that!": GOSUB 800: GOTO 109
4144 IF WEAC THEN PRINT "You are already wearing the cross!": GOSUB 800: GOTO 109
4146 PRINT "It fits rather nice.": WEAC = -1: GOTO 100
4150 IF V$ = "PUT" AND W1$ = "ON" THEN IF N$ = "CROSS" THEN V$ = "WEAR": GOTO 4130
4152 IF V$ = "GET" AND W1$ = "ON" THEN IF N$ = "CROSS" THEN V$ = "WEAR": GOTO 4130
4154 IF V$ = "GET" AND W1$ = "OFF" THEN V$ = "UNWEAR": GOTO 2000
4156 IF V$ = "TAKE" AND W1$ = "OFF" THEN V$ = "UNWEAR": GOTO 2000
4200 IF V$ <> "PUT" THEN 4310
4205 IF W1$ <> "ON" AND W1$ <> "ONTO" THEN 4220
4211 IF N$ = "WATER" OR N$ = "BUCKE" THEN V$ = "POUR": GOTO 2000
4212 IF N$ <> "TORCH" THEN 4217
4213 IF OB(4) <> -1 THEN PRINT "You don't have that!": GOSUB 800: GOTO 109
4215 IF W2$ = "GREY" OR W2$ = "OOZE" THEN IF R = 19 AND DGRE = 0 THEN DGRE = -1: MA(19, 3) = 18: OB(4) = 22: NI = NI - 1: PRINT "The ooze dissolves the torch, lighting it instantly.  The ooze melts away into  nothing.": GOSUB 750: GOTO 100
4216 IF W2$ = "COBWE" AND R = 23 AND NOT BCOB THEN PRINT "The cobwebs light and burn away into dust.": BCOB = -1: MA(23, 2) = 24: SCORE = SCORE + 5: GOSUB 300: GOTO 100
4217 PRINT "That probably won't do much.": GOSUB 800: GOTO 109
4220 IF W1$ <> "IN" AND W1$ <> "INTO" THEN 4300
4221 IF N$ = "WATER" AND W2$ = "BUCKE" THEN V$ = "FILL": N$ = "BUCKE": GOTO 2000
4222 IF W2$ = "OOZE" OR W2$ = "GREY" THEN 4215
4224 IF W2$ = "COBWE" THEN 4215
4230 IF W2$ <> "WELL" THEN 4242
4235 IF R <> 4 THEN PRINT "That's not here!": GOSUB 800: GOTO 109
4238 GOSUB 1000: IF FL = 0 THEN PRINT "You don't have that!": GOSUB 800: GOTO 109
4239 IF RO <> -1 THEN PRINT "You don't have that!": GOSUB 800: GOTO 109
4240 PRINT "Enuooooooooauuuuoooooclunk!": OB(I) = 7: NI = NI - 1: GOTO 100
4242 IF W2$ <> "STREA" AND W2$ <> "WATER" AND W2$ <> "BUCKE" AND W2$ <> "RIVER" THEN 4300
4244 IF W2$ = "BUCKE" THEN IF ((OB(2) <> -1) AND (OB(2) <> R)) THEN PRINT "You can't see any bucket here!": GOSUB 800: GOTO 109
4245 IF W2$ = "BUCKE" AND NOT BUCK AND N$ = "WATER" THEN V$ = "GET": GOTO 2000
4246 IF W2$ = "BUCKE" AND NOT BUCK THEN PRINT "Unfortunately, that's not going to be very helpful.": GOSUB 800: GOTO 109
4248 IF W2$ = "WATER" THEN IF R = 3 OR R = 6 THEN 4250
4249 IF W2$ = "WATER" THEN IF OB(2) = -1 OR BUCK THEN 4250 ELSE PRINT "That's not here!": GOSUB 800: GOTO 100
4250 IF W2$ = "STREA" OR W2$ = "RIVER" THEN IF R <> 3 AND R <> 6 THEN PRINT "You don't see any!": GOSUB 800: GOTO 109
4255 GOSUB 1000: IF FL = 0 THEN PRINT "You don't have that!": GOSUB 800: GOTO 109
4258 IF OB(I) <> -1 THEN PRINT "You don't have that!": GOSUB 800: GOTO 109
4260 IF N$ = "CARRO" THEN IF R = 3 OR R = 6 THEN PRINT "The carrot soaks up the water.": SCORE = SCORE + 10: GOSUB 300: CASO = -1: GOTO 100
4262 IF N$ = "CARRO" AND OB(2) = -1 AND BUCK THEN PRINT "The carrot soaks up the water.": SCORE = SCORE + 10: GOSUB 300: CASO = -1: GOTO 100
4265 IF N$ = "APPLE" THEN IF R = 3 OR R = 6 THEN PRINT "The apple soaks up the water.": APSO = -1: GOTO 100
4268 IF N$ = "APPLE" AND OB(2) = -1 AND BUCK THEN PRINT "The apple soaks up the water.": APSO = -1: GOTO 100
4300 PRINT "That's not going to do much for this adventure.": GOSUB 800: GOTO 109
4310 IF V$ <> "PUSH" THEN 4350
4315 IF N$ = "COFFI" AND R = 1 THEN PRINT "It is much too heavy.": GOTO 100
4317 IF N$ = "MIRRO" AND R = 24 AND NOT BRMI THEN PRINT "It is attached to the wall.": GOTO 100
4319 IF N$ = "THRON" AND R = 25 THEN PRINT "It is bolted to the wall.": GOTO 100
4320 IF N$ <> "LEVER" OR R <> 26 THEN PRINT "You don't need to do that.": GOSUB 800: GOTO 109
4330 IF PLEV THEN PRINT "Nothing happens.": GOTO 100
4340 PRINT "You hear a loud `clunk' to the south.": MA(8, 5) = 27: PLEV = -1: MA(27, 4) = 8: GOTO 100
4350 IF V$ <> "PULL" THEN 4375
4355 IF N$ <> "LEVER" THEN 4315
4360 IF R <> 26 THEN PRINT "You can't do that now!": GOSUB 800: GOTO 109
4370 PRINT "The roof caves in on you.": GOTO 820
4375 IF V$ <> "TURN" THEN 4400
4380 IF N$ <> "CRANK" THEN PRINT "You can't do that now!": GOSUB 800: GOTO 109
4390 IF R <> 4 THEN PRINT "You can't do that now!": GOSUB 800: GOTO 109
4395 IF ROPE THEN PRINT "The rope rises from the well.": ROPE = 0: GOTO 100
4398 PRINT "The pole turns, and the rope goes down into the well.": ROPE = -1: GOTO 100
4400 IF V$ = "WAIT" OR V$ = "Z" THEN PRINT "Time passes...": GOTO 100
4410 IF V$ <> "EAT" AND V$ <> "TASTE" THEN 4450
4415 GOSUB 1000: IF FL = 0 THEN PRINT "You can't do that now!": GOSUB 800: GOTO 100
4418 IF OB(I) <> -1 THEN PRINT "You can't do that now!": GOSUB 800: GOTO 100
4420 IF I <> 0 AND I <> 8 THEN PRINT "You can't eat that!": GOSUB 800: GOTO 100
4430 IF I = 8 AND APSO THEN PRINT "You take a bite of the apple and feel a strange shiver running through the body.You gasp for breath as you clutch your throut.  You fall down, the apple rolling"
4432 IF I = 8 AND APSO THEN PRINT "out of your limb hand.": GOTO 820
4435 IF I = 0 AND CASO THEN PRINT "You munch a little out of the carrot and suddenly feel your lungs bursting.  Youcollapse on the ground, carrot clutched in your cool hand.": GOTO 820
4440 IF I = 8 THEN PRINT "You take a bite out of the apple and spit it out.": GOTO 100
4442 PRINT "Mmmmmnn...the carrot is delicous.  You gobble the whole thing up.": OB(0) = 0: GOTO 100
4450 IF V$ = "SOAK" AND W1$ = "" THEN V$ = "PUT": W1$ = "IN": W2$ = "WATER": GOTO 2000
4452 IF V$ = "SOAK" THEN V$ = "PUT": GOTO 2000
4460 IF V$ <> "BREAK" AND V$ <> "SMASH" THEN 4490
4465 IF N$ <> "MIRRO" THEN PRINT "There's either no point or you don't have the object.": GOSUB 800: GOTO 109
4470 IF R <> 24 THEN PRINT "There's either no point or you don't have the object.": GOSUB 800: GOTO 109
4480 IF BRMI THEN PRINT "It's already broken!": GOSUB 800: GOTO 109
4485 BRMI = -1: PRINT "The mirror breaks into little pieces, revealing a passage to the east.": SCORE = SCORE + 8: GOSUB 300: MA(24, 2) = 25: GOTO 100
4490 IF V$ <> "GIVE" THEN 4520
4492 IF W1$ = "TO" THEN W1$ = N$: N$ = W2$
4495 IF N$ <> "BUNNY" THEN 4510
4498 IF R <> 25 THEN PRINT "I don't see a bunny--do you?": GOSUB 800: GOTO 109
4499 IF DBUN THEN PRINT "I don't see a bunny--do you?": GOSUB 800: GOTO 109
4500 N$ = W1$: GOSUB 1000: IF FL = 0 THEN PRINT "You don't have that!": GOSUB 800: GOTO 109
4502 IF OB(I) <> -1 THEN PRINT "You don't have that!": GOSUB 800: GOTO 109
4505 IF N$ <> "CARRO" THEN PRINT "The bunny refuses your offer.": GOSUB 800: GOTO 109
4507 IF NOT CASO THEN PRINT "The bunny gobbles up the carrot but is not satisfied.": OB(0) = 0: GOTO 100
4509 IF CASO THEN PRINT "The bunny gobbles the carrot up, makes a startled gasp, and dies.  The bunny    disappears into a black fog.": SCORE = SCORE + 10: GOSUB 300: DBUN = -1: GOTO 100
4510 PRINT "I'm not sure it will accept your offer.": GOSUB 800: GOTO 109
4520 IF V$ <> "SIT" THEN 4540
4530 IF N$ = "ON" THEN N$ = W1$
4532 IF N$ <> "THRON" THEN PRINT "That probably won't work.": GOSUB 800: GOTO 109
4534 IF R <> 25 THEN PRINT "That probably won't work.": GOSUB 800: GOTO 109
4535 IF STHR THEN PRINT "You sit on the throne and you get blown away by a....hold it....that was anotheradventure.  Nothing happens so you stand up again.": GOTO 100
4538 PRINT "As you sit on the throne, a passage opens to the east!  Since the throne is     uncomfortable, you stand up again.": SCORE = SCORE + 2: STHR = -1: GOSUB 300: MA(25, 2) = 100: GOTO 100
4540 IF V$ = "BURN" THEN V$ = "LIGHT": GOTO 2000
4542 IF V$ = "NORTH" THEN V$ = "N": GOTO 2000
4544 IF V$ = "SOUTH" THEN V$ = "S": GOTO 2000
4546 IF V$ = "EAST" THEN V$ = "E": GOTO 2000
4547 IF V$ = "WEST" THEN V$ = "W": GOTO 2000
4548 IF V$ = "UP" THEN V$ = "U": GOTO 2000
4549 IF V$ = "DOWN" THEN V$ = "D": GOTO 2000
4550 IF V$ <> "UNWEA" THEN 4565
4555 IF N$ <> "CROSS" THEN PRINT "That doesn't make sense!": GOSUB 800: GOTO 109
4556 IF NOT WEAC THEN PRINT "You're not wearing it!": GOSUB 800: GOTO 109
4560 PRINT "You slide the cross off your neck.": WEAC = 0: GOTO 100
4565 IF V$ = "DESTR" AND N$ = "MIRRO" THEN V$ = "BREAK": GOTO 2000
4570 IF V$ <> "DESTR" AND V$ <> "KILL" AND V$ <> "ATTAC" THEN 4590
4575 IF N$ = "SKELE" AND R = 9 THEN PRINT "The skeleton repels your attack.": GOTO 100
4580 IF N$ = "VAMPI" AND VAMP THEN PRINT "The vampire ignores your attack.": GOTO 100
4585 IF N$ = "OOZE" AND NOT DGRE AND R = 19 THEN PRINT "The ooze lets out a splatter with killing force.": GOTO 820
4588 IF N$ = "BUNNY" AND NOT DBUN AND R = 25 THEN PRINT "The bunny bites you in defense.": GOTO 820
4589 PRINT "That's not important to this adventure.": GOSUB 800: GOTO 109
4590 IF V$ = "TALK" THEN PRINT "That won't do much.": GOSUB 800: GOTO 109
4595 IF V$ <> "DRINK" AND V$ <> "SIP" THEN 4620
4600 IF N$ <> "WATER" THEN PRINT "That won't do much.": GOSUB 800: GOTO 109
4605 IF R = 3 OR R = 6 THEN 4615
4608 IF OB(2) = -1 OR OB(2) = R THEN IF BUCK THEN 4615
4610 PRINT "That won't do much.": GOSUB 800: GOTO 109
4615 PRINT "You sip the water, and feel very sick.  You lungs turn inside out, your breath  stops, and you fall on the floor, dead.": GOTO 820
4620 IF V$ = "SCORE" THEN GOSUB 25420: GOSUB 800: GOTO 108
10000 PRINT "Sorry, but I don't know that verb.  When the programmer thought of it, he was   either asleep, out of it, ticked off at the world, or out to lunch.": GOSUB 800: GOTO 109
25000 IF NR = 0 THEN RETURN
25030 FOR I = 1 TO NR
25040 FOR J = 0 TO ND - 1
25050 READ MA(I, J): NEXT: NEXT: RETURN
25100 DATA 0,4,0,0,0,0: 'ROOM 1
25110 DATA 0,0,3,0,0,0: 'ROOM 2
25120 DATA 0,200,0,2,0,0: 'ROOM 3
25130 DATA 1,8,5,0,0,201: 'ROOM 4
25140 DATA 0,8,6,4,0,0: 'ROOM 5
25150 DATA 200,9,10,5,0,0: 'ROOM 6
25160 DATA 0,11,0,0,240,0: 'ROOM 7
25170 DATA 5,0,9,4,0,0: 'ROOM 8
25180 DATA 6,0,10,8,0,314: 'ROOM 9
25190 DATA 6,0,0,9,0,0: 'ROOM 10
25200 DATA 7,0,0,0,0,0: 'ROOM 11
25210 DATA 0,0,0,0,0,0: 'ROOM 12
25220 DATA 16,12,17,17,0,0: 'ROOM 13
25230 DATA 0,0,15,0,9,0: 'ROOM 14
25240 DATA 15,19,15,14,0,0: 'ROOM 15
25250 DATA 13,22,20,21,0,0: 'ROOM 16
25260 DATA 13,20,16,16,0,0: 'ROOM 17
25270 DATA 0,0,19,17,0,0: 'ROOM 18
25280 DATA 15,0,0,203,0,0: 'ROOM 19
25290 DATA 13,17,17,16,0,0: 'ROOM 20
25300 DATA 16,23,13,20,0,0: 'ROOM 21
25310 DATA 17,21,20,16,0,0: 'ROOM 22
25320 DATA 21,26,0,0,0,0: 'ROOM 23
25330 DATA 0,0,0,23,0,0: 'ROOM 24
25340 DATA 0,0,0,24,0,0: 'ROOM 25
25350 DATA 23,27,0,0,0,0: 'ROOM 26
25360 DATA 26,0,0,0,0,0: 'ROOM 27
25400 PRINT "You go through the tunnel and end up outside the graveyard.  When you realize   where you are, you hurry along the path back into town, overcome with joy,"
25405 PRINT "shouting the news.  People rush out of their houses to greet you with a hero's  welcome."
25410 GOSUB 25420: GOTO 3440
25420 PRINT "You have scored"; SCORE; "out of 60 points."
25422 IF SCORE < 11 THEN RANK$ = "Novice Adventurer": GOTO 25490
25430 IF SCORE < 21 THEN RANK$ = "Graveyard Explorer": GOTO 25490
25440 IF SCORE < 31 THEN RANK$ = "Adventurer": GOTO 25490
25450 IF SCORE < 41 THEN RANK$ = "Monster Basher": GOTO 25490
25460 IF SCORE < 51 THEN RANK$ = "Expert Adventurer": GOTO 25490
25470 IF SCORE < 60 THEN RANK$ = "Exterminator": GOTO 25490
25480 RANK$ = "Bunny Slayer"
25490 PRINT "You have reached the rank of "; RANK$; ".": RETURN
26000 IF NO = 0 THEN RETURN
26010 FOR I = 0 TO NO - 1
26020 READ OB$(I), O2$(I), OB(I): NEXT: RETURN
26100 DATA "a carrot",CARRO,0
26110 DATA "a lamp",LAMP,2
26120 DATA "a bucket",BUCKE,11
26130 DATA "a silver cross",CROSS,0
26140 DATA "a torch",TORCH,14
26150 DATA "a key",KEY,18
26160 DATA "a scrap of paper",PAPER,8
26170 DATA "a quill pen",PEN,-1
26180 DATA "an apple",APPLE,-1
27000 CC$ = CHR$(34)
27010 PRINT "    The voice still echoes in your head.  " + CC$ + "Remember--if the bunny is not dead bymidnight, the countryside will be ruled by evil."; CC$
27020 PRINT "    You shiver as you think of the people dead in the street with bite marks on their neck.  The cattle dead by the hundreds.  The stories of a rampaging"
27030 PRINT "vampire bunny.  A chill runs up your spine."
27040 PRINT "    And lastly, you remember the strangeness of the graveyard the last time you saw it.  The misty fog surrounding it.  And now you're in the graveyard, alone,"
27050 PRINT "armed only with your apple and your trusty quill pen."
27052 FOR I = 1 TO 4
27054 LOCATE 10, 24: PRINT "The Night of the Vampire Bunnies"
27055 LOCATE 10, 24: PRINT SPACE$(40): NEXT
27056 LOCATE 10, 24: PRINT "The Night of the Vampire Bunnies"
27058 LOCATE 14, 31: PRINT "Made by Jason Dyer"
27060 LOCATE 24, 28: PRINT "Press any key to continue";
27070 I$ = INKEY$: IF I$ = "" THEN 27070
27080 CLS : RETURN
40000 LO$ = "At Coffin": PRINT LO$
40010 PRINT "You are at a coffin in a graveyard, with walls to the north, east, and west.  A": MES$ = "blue mist hangs over the graveyard.  "
40015 IF VAMP THEN PRINT MES$ + "The coffin is open with a vampire in it!": RETURN
40020 IF OCOF THEN PRINT MES$ + "The coffin is open." ELSE PRINT MES$ + "The coffin is closed."
40030 IF NOT OCOF THEN RETURN
40040 FOR I = 0 TO 8: IF OB(I) = .1 THEN PRINT "There is "; OB$(I); " in the coffin."
40050 NEXT: RETURN
40060 LO$ = "Graveyard": PRINT LO$
40070 PRINT "You are in a graveyard surrounded by walls.  There is fog here so dense you can barely see 2 meters in front of you.  A hole in the east wall allows exit from  this gloomy place.": RETURN
40080 LO$ = "Graveyard": PRINT LO$: IF NOT JRI THEN JRI = -1: SCORE = SCORE + 3: GOSUB 300
40090 PRINT "You are in a graveyard with a stream to the south.  Walls surround the other    directions, but a hole in the west wall allows passage.  It is very cool here.": RETURN
40100 LO$ = "Graveyard": PRINT LO$
40110 PRINT "You are in a dark graveyard.  A wall covered with moss is to the west, a coffin is in sight to the north, and an empty well lies in the middle of all the fog.": IF ROPE THEN PRINT "A rope leads down the damp well."
40120 RETURN
40140 LO$ = "Carrot Field": PRINT LO$
40150 PRINT "You are in a carrot field in the middle of a graveyard.  Fog is all around you. A wall is to the north, too high to climb.": RETURN
40160 LO$ = "Graveyard": PRINT LO$
40170 PRINT "You are in a foggy graveyard.  A stream is to the north, a field can be seen to the west, and all the rest only fog can be seen.  The stream looks shallow      enough to cross.": RETURN
40180 LO$ = "Bottom of Well": PRINT LO$: IF NOT WELL THEN WELL = -1: SCORE = SCORE + 5: GOSUB 300
40190 PRINT "You are at the bottom of an empty well.  The surrounding cracks are rather big, and a crevice to the south is big enough for you to go through.  A rope hangs   down allowing you to climb up.": RETURN
40200 LO$ = "Graveyard": PRINT LO$
40205 IF PLEV THEN PRINT "You are in a walled graveyard.  A black wall is to the south.  A gaping hole is in the middle of all the gravestones.": RETURN
40210 PRINT "You are in a walled graveyard.  A black wall is to the south.  Many tombstones  are in this area--so many, in fact, you have to climb over some of them.": RETURN
40220 LO$ = "At Hole": PRINT LO$
40230 PRINT "You are in a gloomy graveyard, with a wall to the south, and plenty of fog all  around you.  A large hole surrounded by gravestones leads down into gloom."
40240 IF WEAC THEN PRINT "A skeleton that was guarding the hole faces toward you, but when he sees the    cross you are wearing, recoils in terror.": RETURN
40245 PRINT "A skeleton that was guarding the hole faces towards you, and holds his stand,   not allowing you to you into the hole.": RETURN
40250 LO$ = "Graveyard": PRINT LO$
40260 PRINT "You are at the corner of a walled graveyard.  Walls connect to the south and    east, and mist hangs over the sky like a prowler.  A chill runs through your    back.": RETURN
40270 LO$ = "Crevice": PRINT LO$
40280 PRINT "You are in a large crevice in a well.  Small amounts of water trickle down the  sides.  Exit can be reached to the north.": RETURN
40290 LO$ = "Maze": PRINT "You have a nervousness about this passage, but you trudge on, and end up gettingkilled in a cave-in.": R = 13: FFF = -1: RETURN
40300 LO$ = "Maze": PRINT "You are in a dark maze.": RETURN
40310 LO$ = "Dark Tunnel": PRINT LO$
40320 PRINT "You are in a very dark tunnel.  The tunnel heads to the east, while a hole in   the ceiling allows exit from the darkness.": RETURN
40330 LO$ = "Dark Tunnel": PRINT LO$
40340 PRINT "You are in a tunnel of extreme darkness.  You can barely make out exits to all  directions except up and down.": RETURN
40350 LO$ = "Maze": PRINT "You are in a damp maze.": RETURN
40360 LO$ = "Maze": PRINT "You are in a dank maze.": RETURN
40370 LO$ = "Long Corridor": PRINT LO$
40375 IF NOT SCORELONG THEN SCORELONG = -1: SCORE = SCORE + 5: GOSUB 300
40380 PRINT "You are in a very long tunnel streaching endlessly to the east and west.  Your  feet ache as you walk down it.": RETURN
40390 LO$ = "Slimy Room": PRINT LO$
40400 PRINT "You are in a medium-size room covered with slime from ceiling to floor.  Your   feet stick to the slippery ground.": IF NOT DGRE THEN PRINT "Some grey ooze guards the westward corridor."
40405 RETURN
40410 LO$ = "Maze": PRINT "You are in a dreary maze.": RETURN
40420 LO$ = "Maze": PRINT "You are in a maze.": RETURN
40430 LO$ = "Maze": PRINT "You are in a devlish maze.": RETURN
40440 LO$ = "Torch Room": PRINT LO$
40445 PRINT "You are in a large marble room with torchholders everywhere.  However, pretty": IF BCOB THEN PRINT "much all of them are empty.  You can leave to the north, south, or east.": RETURN
40447 PRINT "much all of them are empty.  You can leave to then south and north, but the eastpassage is blocked by cobwebs.": RETURN
40450 LO$ = "Dead End": PRINT LO$
40460 IF BRMI THEN PRINT "You are in a seeminly dead end, with the only exit to the west, but a shattered mirror shows a passage to the east.": RETURN
40465 PRINT "You are in a dead end, with a large mirror covering the east wall, and an exit  to the west.": RETURN
40470 LO$ = "Bunny Room": PRINT LO$
40472 IF NOT DBUN THEN PRINT "You are in a large chamber that is sparsely furnished except for a throne boltedto the east wall.  Sitting next to the throne is a bunny which looks very much"
40475 IF NOT DBUN THEN PRINT "like the vampire bunny you are looking for.  A passage exits west from this veryunfriendly place.": RETURN
40480 IF NOT STHR THEN PRINT "You are in a large chamber that is sparsely furnished except for a throne boltedto the east wall.  A passage exits west from this barren place.": RETURN
40485 PRINT "You are in a large chamber that is sparsely furnished except for a throne boltedto the east wall.  Two passages allow exit -- one to the east, one to the west.": RETURN
40490 LO$ = "Cramped Corridor": PRINT LO$
40500 PRINT "You are in a very cramped corridor going north/south.  A lever is set into the  west wall.  The lever seems to be pretty worn from use.": RETURN
40510 LO$ = "Sandy Room": PRINT LO$
40520 IF NOT PLEV THEN PRINT "You are in a extremely sandy room with the only exit going north.  You feel a   little nervous that the ceiling will cave in on you.": RETURN
40530 PRINT "You are in a very sandy room with an exit to the north and a hole in the roof.  You feel a little nervous that the ceiling will cave in on you.": RETURN

