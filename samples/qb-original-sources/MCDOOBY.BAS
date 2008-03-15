1 CLS:REM ** "The Treasure of McDooby's Mansion" Version 2.0
2 CLS:COLOR 9
3 PRINT TAB(10);"****************************"
4 PRINT TAB(10);"*     The Treasure of      *"
5 PRINT TAB(10);"*    McDooby's Mansion     *"
6 PRINT TAB(10);"*         Version 2        *"
7 PRINT TAB(10);"*     By: Jason Wells      *"
8 PRINT TAB(10);"****************************"
9 COLOR 4:PRINT TAB(10);"Press SPACE bar to begin."
10 COLOR 25:PRINT TAB(10);"(Be sure the CapsLock is on.)"
11 A$=INKEY$:IF A$<>CHR$(32) THEN 11
12 COLOR 15
13 GOTO 27
14 SOUND 587.33,15:SOUND 440,15
17 FOR T=1 TO 2000:NEXT T
21 RETURN
22 CLS:REM *** Endless Woods
23 COLOR 10,3:PRINT"You are in a forest with strange trees everywhere."
24 GOSUB 1000:GOTO 22
27 CLS:COLOR 12,1:REM *** Near House
28 PRINT"You can see a house to the NORTH, woods to the SOUTH, EAST, and WEST."
29 GOSUB 1000
30 IF C$="WEST" OR C$="S" OR C$="EAST" THEN 22
31 IF C$="NORTH" THEN 33
32 GOTO 27
33 CLS:COLOR 14,0:REM *** Front Door
34 PRINT"The front door of the house. Forests surround you on the EAST and WEST sides."
35 INPUT"Do you want to ring the DOORBELL(Y/N)?",Z$:IF Z$="Y" THEN GOSUB 14
36 GOSUB 1000
38 IF C$="NORTH" THEN 42
39 IF C$="EAST" OR C$="W" THEN 22
40 IF C$="SOUTH" THEN 27
41 GOTO 33
42 CLS:COLOR 15,6:REM *** Hallway
44 PRINT"A grand entrance hall. Exits are in all directions."
46 GOSUB 1000
48 IF C$="EAST" THEN 52
49 IF C$="SOUTH" THEN 33
50 IF C$="WEST" THEN 83
51 IF C$="NORTH" THEN 69
52 COLOR 6,0:CLS:REM *** Kitchen
53 PRINT"A very old kitchen. There are more cracks on the wall than you can count."
54 IF J=0 THEN COLOR 14:PRINT"There is a JAR on the counter."
55 IF J=1 THEN GOTO 57
56 COLOR 14:INPUT"Do you want to get the JAR(Y/N)";Z$:IF Z$="Y" THEN J=1
57 GOSUB 1000:IF C$="WEST" THEN 42
58 IF C$="NORTH" THEN 60
59 GOTO 52
60 CLS:REM *** Dining Room
61 COLOR 15:PRINT"This is a nice dining room.";:IF K=0 THEN COLOR 14:PRINT"A KEY is on the table."
62 IF K=1 THEN GOSUB 1000
63 INPUT"Do you want to get the KEY(Y/N)";Z$:IF Z$="Y" THEN K=1:GOSUB 1000:GOTO 66
64 IF Z$="N" THEN GOSUB 1000
66 IF C$="SOUTH" THEN 52
67 IF C$="WEST" THEN 69
68 GOTO 60
69 COLOR 11,0:CLS:REM *** Library
70 PRINT"You are in the library. Many books are on the walls."
71 IF S=1 THEN GOSUB 1000:GOTO 78
72 IF S=0 THEN COLOR 14:PRINT"A sword is on the shelf."
73 COLOR 11:INPUT"Do you want to get the SWORD(Y/N)";Z$
74 IF Z$="N" THEN GOSUB 1000:GOTO 78
76 IF Z$="Y" THEN S=1:GOSUB 1000
78 IF C$="EAST" THEN 60
79 IF C$="SOUTH" THEN 42
80 GOTO 69
83 COLOR 13,0:CLS:REM *** Ballroom
84 PRINT"You have entered a very nice ballroom. There is a loose PANEL on the NORTH wall."
85 GOSUB 1000
86 IF C$="EAST" THEN 42
87 IF C$="NORTH" THEN 90
88 GOTO 83
90 CLS:REM *** Secret passage
91 COLOR 8
92 PRINT"A very dark passage. The stairs seem to lead DOWN into the darkness."
93 PRINT"The only ways out are UP and DOWN."
95 GOSUB 1000
96 IF C$="DOWN" THEN 100
97 IF C$="UP" THEN 83
98 GOTO 90
100 CLS:REM *** In front of door
101 COLOR 15:PRINT"A massive DOOR stands in front of you to the NORTH."
102 GOSUB 1000
103 IF C$="EAST" AND K=1 THEN DOOR=1:PRINT"DOOR unlocked.":GOSUB 1000
104 IF C$="UP" THEN 90
105 IF C$="EAST" AND K=0 THEN 108
106 IF C$="EAST" AND DOOR=1 THEN 110
108 PRINT"The large DOOR is locked."
110 CLS:REM *** Troll's cave
113 IF T=0 THE COLOR 2:PRINT"A TROLL is here to meet you."
114 IF S=1 THEN 116
115 IF S=0 THEN COLOR 20:PRINT"BAP!POW!WHAM!OOOF! That darn TROLL did a smashing job of killing you."
116 PRINT"You successfully defeat the TROLL.":T=1:GOSUB 1000
118 IF C$="WEST" THEN 100
119 IF C$="EAST" THEN 122
120 GOTO 100
122 CLS:REM *** Sorcerer's room
123 REM *** COOLNESS ***
124 COLOR 3:PRINT"A room with a strange glowing GEM."
125 IF SO=0 THEN COLOR 13:PRINT"Unfortunatly a sorcerer gaurds it(MCDOOBY!!!)."
126 IF J=0 THEN PRINT"The wizards magic turns you into sub-atomic particles blown away by the prevailing wind.":GOTO 2000
129 IF J=1 THEN PRINT"You pour the jar of water on the wizard. He dissolves into a puddle of orange ooze.":SO=1:GOSUB 1000
130 IF C$="WEST" THEN 110
131 IF C$="NORTH" THEN PRINT"You take the";:COLOR 12:PRINT" GEM ";:COLOR 3:PRINT"on your way out of the mansion":GOTO 3000
132 GOTO 122
1000 REM Get command from user.
1002 INPUT"Enter command";C$
1003 IF C$="Q" THEN END
1004 IF C$="B" THEN RUN"BOSS.BAS"
1006 IF C$="H" THEN RUN"HELP.BAS"
1007 IF C$="INV" THEN GOSUB 4000
1008 RETURN
2000 REM G A M E  O V E R
2002 FOR Z=1 TO 7:PRINT:NEXT Z:COLOR 28
2004 PRINT"You really screwed up this time."
2005 COLOR 12
2006 PLAY "MST64L6N3N7N3N7L3N2"
2008 PRINT"1) Try again"
2009 PRINT"2) I give up"
2010 INPUT"Enter choice -->",Q$
2012 IF Q$="1" THEN 27: REM CLEAR - GOTO 27
2014 IF Q$="2" THEN END
2016 GOTO 2010
3000 REM THE END
3002 CLS:C$="YOU WIN!!"
3004 FOR T=1 TO 2000:PRINT C$;:NEXT T
3006 PLAY"T169 L9 MS ABCDEFGACEGECAFEDEDEDGDCCEDABBCAEGFC"
3010 A$=INKEY$:IF A$="" THEN 3006
3012 GOTO 2
4000 REM *** Inventory
4002 CLS:PRINT"Inventory"
4004 PRINT"---------"
4006 IF J=1 THEN PRINT"JAR"
4008 IF K>1 THEN PRINT"KEY"
4010 IF S>1 THEN PRINT"SWORD"
4012 IF S=0 AND K=0 AND J=0 THEN PRINT"Nothing."
4014 RETURN
