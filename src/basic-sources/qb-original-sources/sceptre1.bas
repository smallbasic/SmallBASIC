100 GOSUB 8000
200 REM new room
201 CLS
210 ? "you're ";R$(R)
220 ?
221 IF R=1 THEN ? "P-U!"
222 IF R=3 THEN ? "Sshhh!"
223 IF R=6 THEN ? "If you leave without the sceptre piece  you LOSE!"
224 IF R=10 AND L(13)<>-1 THEN ? "You have left without the sceptre piece therefore you lose.":END
225 IF R=10 AND L(13)=-1 THEN ? "You have the piece! You win! Look soon  for SceptreQuest II!":END
240 V$="you see "
250 FOR I=1 TO N9:IF L(I)=R THEN ? V$;O$(I):V$="        "
260 NEXT I
300 ?
310 ?"you can go:  ";
320 FOR I=1 TO 4
330 IF R(R,I)<>0 THEN ? D$(I)" ";
340 NEXT I
341 ?
400 ?:?"now what";
410 INPUT A$:?
420 N$="":V$="":V=0:N=0:J=0
430 L=LEN(A$):FOR I=1 TO L
440 IF MID$(A$,I,1)=" " THEN V$=LEFT$(A$,I-1):J=I:I=L
450 NEXT:IF J=0 THEN V$=A$:GOTO 550
460 FOR I=L TO J STEP -1
470 IF MID$(A$,I,1)=" " THEN N$=MID$(A$,I+1):I=J
480 NEXT
500 T$=LEFT$(N$,4)
510 FOR I=1 TO N9
520 IF T$=N$(I) THEN N=I:I=N9
530 NEXT
550 T$=LEFT$(V$,3)
560 FOR I=1 TO V9
570 IF T$=V$(I) THEN V=I:I=V9
580 NEXT
600 IF V=0 THEN ?"i don't know that verb.":GOTO 400
610 IF V<9 THEN 1200
620 ON V-8 GOTO 1000,1400,1600,1800,2000,2000,2000,2200,2400,2400,2600,2800
630 ON V-20 GOTO 3000,3200,3400,3600,3800,4000,4000
640 GOTO 600
900 IF N$="" THEN ?"you need to say what to ";V$;".":GOTO 400
910 ?"what is an ";N$;"?":GOTO 400
930 NX=0:I=L(N)
940 IF I<>R AND I<>-1 THEN ?"i don't see it here.":NX=1
950 RETURN
1000 IF N$="" THEN ?"go where?":GOTO 400
1010 IF N=0 THEN V$=N$:GOTO 550
1020 ?"use compass directions.":GOTO 400
1200 REM movement
1210 V=INT((V+1)/2):I=R(R,V)
1220 IF I>0 THEN R=I:GOTO 200
1221 IF R=7 AND V=3 THEN ?"the wall blocks the way."
1230 ?"you can't go that way."
1400 REM inv
1410 Z=0:FOR I=1 TO N9
1420 IF L(I)=-1 THEN ?"  ";O$(I):Z=Z+1
1430 NEXT I
1440 IF Z=0 THEN ?"  nothing"
1450 GOTO 400
1600 REM take
1610 IF N=0 THEN 900
1620 IF L(N)=-1 THEN ?"you have it already!":GOTO 400
1630 GOSUB 930:IF NX=1 THEN 400
1640 IF A(N)=1 THEN ?"you can't take that.":GOTO 400
1650 L(N)=-1
1660 ?"taken.":GOTO 400
1800 REM drop
1810 IF N=0 THEN 900
1820 IF L(N)<>-1 THEN ?"you don't have it!":GOTO 400
1822 IF N=6 THEN ?"you only spilled a little bit":GOTO 400
1830 L(N)=R
1840 ?"dropped":GOTO 400
2000 REM look,exa,search
2010 IF N=0 THEN GOTO 200
2020 GOSUB 930:IF NX=1 THEN 400
2030 ? ED$(N)
2040 IF R=1 AND N=1 THEN ?"you see a small coin.":L(9)=1
2050 GOTO 400
2200 REM open
2210 IF N=0 THEN 900
2220 GOSUB 930:IF NX=1 THEN 400
2230 IF N=14 THEN ?"creak. It opens, allowing way to the east.":R(6,3)=10:ED$(14)="the gate is open.":GOTO 400
2240 IF N=11 THEN ?"how? he'll just get mad.":GOTO 400
2250 IF N=10 THEN ?"BLASPHEMY! Appolo strikes you down!":END
2260 ?"you can't open that.":GOTO 400
2400 REM eat/drink
2410 IF N=0 THEN 900
2411 GOSUB 930:IF NX=1 THEN 400
2412 IF N=3 THEN ?"you drink the beer, leaving an empty    glass.":O$(3)="an empty GLASS":ED$(3)="it's empty":N$(3)="glas":GOTO 400
2413 IF N=4 THEN ?"yum!":L(4)=90:GOTO 400
2414 IF N=6 THEN ?"ak! The water is poison,acid,nitrogly   cerine combined! You're dead!":END
2420 ?"you can't digest that.":GOTO 400
2430 IF N=11 THEN ?"that's disgusting, and it won't work    either.":GOTO 400
2600 REM vomit
2610 ?"blarrrrrrggggggg!!!!"
2620 IF R<>1 THEN ?"you are immediately arrested for litt-  ering. The End.":END
2630 ?"you add to the vomit already there.":GOTO 400
2800 REM fill(oh boy)
2810 IF N=0 THEN 900
2820 GOSUB 930:IF NX=1 THEN 400
2830 IF N=3 AND N$(3)="glas" AND R=5 THEN ?"you fill the glass with water from the fountain.":L(6)=-1:BG=99:ED$(3)="it's full.":GOTO 400
2840 IF N=3 AND N$(3)="beer" AND R=5 THEN ?"you should maybe drink the beer first.":GOTO 400
2850 IF R=1 THEN ?"NO! No Way! Forget It! You're not mak-  ing a mess of MY city!":GOTO 400
2860 ?"you can't fill anything in this situ-   ation.":GOTO 400
3000 REM read
3010 IF N=0 THEN 900
3020 GOSUB 930:IF NX=1 THEN 400
3030 IF N=7 THEN ?"The plaque reads: WARNING! This water   is poisonous, acidic and everything     else, so watch out!":GOTO 400
3040 IF N=8 THEN ?"HI! This is the designer of this game.  Welcome to my city, which happens to be the hiding place of the wizard's first  piece!":GOTO 3100
3050 ?"there's no writing":GOTO 400
3100 ?"If you find it, leave the city with it  to win. Don't fail me and good luck!":GOTO 400
3200 REM throw
3210 IF N=0 THEN 900
3220 GOSUB 930:IF NX=1 THEN 400
3230 IF N=3 AND BG=99 AND R=7 THEN ?"the bottle of water hits the wall. He   screams in pain and dissolves before    your eyes!":R(7,3)=8
3235 IF R(7,3)=8 THEN ?"a secret room to the east is revealed.":O$(11)="a dead WALL":ED$(11)="it's dead."
3236 ?"it rebounds and lands harmlessly where  it began." :GOTO 400
3400 REM give
3410 IF N=0 THEN 900
3420 GOSUB 930:IF NX=1 THEN 400
3430 IF R<>4 AND R<>7 THEN ?"to who?":GOTO 400
3440 IF R=7 THEN ?"The wall refuses to be bribed.":GOTO 400
3450 IF R=4 AND N<>9 THEN ?"The priest insists on money.":GOTO 400
3460 ?"the priest smiles and takes your coin.  He points westward.":L(9)=90:R(4,4)=3:GOTO 400
3600 REM talk
3610 IF N=0 THEN ?"blah.blah.":GOTO 400
3620 IF R<>4 AND R<>7 THEN ?"blah.blah. Passerbys look at you oddly.":GOTO 400
3630 IF R=4 AND N=12 THEN ?"``Pay to Pray, sir.''":GOTO 400
3640 IF R=7 AND N=11 THEN ?"``I am the guardian to sceptre piece    one!''":GOTO 400
3650 ?"are you possibly insane?":GOTO 400
3800 REM pray
3810 IF R<>3 THEN ?"you get down on your knees and pray,    but nothing happens. Maybe you should   go to a shrine.":GOTO 400
3820 ?"Appolo speaks and gives you enlighten-  ment:To find the sceptre piece, there   is a secret exit south of here!":R(3,2)=7:GOTO 400
4000 REM move/push
4010 IF N=0 THEN 900
4020 GOSUB 930:IF NX=1 THEN 400
4030 IF N=11 THEN ?"You dare lay your filthy hands on me?!  DIE!!!!!!                               ZAP!!!!!!":END
4040 IF A(N)=1 THEN ?"it won't move":GOTO 400
4050 ?"it moves.":GOTO 400
8000 'SCREEN 1:SCREEN 0,0,0
8005 'KEY OFF
8010 ? TAB(5);"SCEPTREQUEST I"
8020 ?:?
8030 ? TAB(5);"You are Giglamesht, the famous Greek adventurer, in search of the famous Athena Royal Sceptre. It is said the sceptre was broken by an evil wizard and hidden in places all over Greece. It is also said ";
8032 ? "that whoever finds the sceptre pieces   can become ruler of all the Greek empi- re. Can you find the seven broken       pieces? Solve my seven games? We shall  see."
8040 ?:?
8050 FOR X=1 TO 10000
8060 NEXT X
8070 CLS
9000 R9=10
9010 DIM R$(R9),R(R9,4)
9020 FOR I=1 TO R9
9030 READ R$(I)
9040 FOR J=1 TO 4
9050 READ R(I,J)
9060 NEXT J
9062 NEXT I
9070 DATA "in the vomitorium",0,0,2,0
9080 DATA "in a restaurant",0,5,0,1
9090 DATA "in appolo's shrine",0,0,4,0
9100 DATA "at appolo's temple",0,0,5,0
9110 DATA "in vathen's central square",2,9,6,4
9120 DATA "at vathen's city gate",0,0,0,5
9130 DATA "in a secret room",3,0,0,0
9140 DATA "in a secret room",0,0,0,7
9150 DATA "in the city library",5,0,0,0
9160 DATA "outside vathen",0,0,0,0
9450 DIM D$(4)
9460 FOR I=1 TO 4:READ D$(I)
9461 NEXT I
9470 DATA "north","south","east","west"
9500 N9=14
9510 DIM O$(N9),ED$(N9),L(N9),A(N9),N$(N9)
9520 FOR I=1 TO N9
9530 READ O$(I),ED$(I),L(I),A(I),N$(I)
9540 NEXT
9550 DATA "much VOMIT","it is green and disgusting",1,1,"vomi"
9560 DATA "a large TRENCH","the vomit is in it",1,1,"tren"
9570 DATA "a glass of BEER","it looks good",2,2,"beer"
9580 DATA "a roast CHICKEN","it looks appetizing",2,2,"chic"
9590 DATA "a playing FOUNTAIN","it overflows with water",5,1,"foun"
9600 DATA "full of WATER","there isn't much",5,1,"wate"
9610 DATA "a brass PLAQUE","it has writing etched on it",5,1,"plaq"
9620 DATA "an old BOOK","it looks ages old",9,2,"book"
9630 DATA "a gold COIN","it has the symbol of appolo's temple on it",0,2,"coin"
9640 DATA "a marble ALTAR","it is appolo's personal one",3,1,"alta"
9650 DATA "an odd-looking WALL","it's not made of stone; it is made of flesh! It speaks! 'I AM THE GAURDIAN OF THE FIRST SCEPTRE PIECE! BEHOLD ME!'",7,1,"wall"
9660 DATA "an old PRIEST","he looks kindly  but greedy",4,1,"prie"
9670 DATA "the SCEPTRE piece!","it shines of gold and silver",8,2,"scep"
9680 DATA "the city GATE","it is closed",6,1,"gate"
9900 V9=27
9910 DIM V$(V9)
9920 FOR I=1 TO V9
9930 READ V$(I):NEXT I
9940 DATA "n","nor","s","sou","e","eas","w","wes"
9950 DATA "go","inv","tak","dro","loo","exa","sea","ope","eat","dri","vom","fil","rea","thr","giv","tal","pra"
9960 DATA "mov","pus"
9990 R=9:BG=1
9999 RETURN
