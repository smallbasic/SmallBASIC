#!/usr/local/bin/sbasic -g
'
'	TIC-TAC-TOE
'
'	SmallBASIC 0.5.6 example
'

DIM MAT(9), PW(8), ST(3)

LABEL RESTART
FOR I=1 TO 9
	MAT(I)=0
NEXT

cls
? cat(1);"Tic-Tac-Toe";cat(0)
? chr$(27)+"[92m"
? "A SmallBASIC example"

at 0,130
? "X=USER, O=CPU" 
? "Are you want to play first"; 
INPUT sel

WINNER=0
IF UCASE$(LEFT$(sel,1))="Y" THEN
	PLAYER=-1
ELSE 
	PLAYER=1
FI

' start
WHILE 1
	IF PLAYER=-1 
		GOSUB DISPLAY
		GOSUB USER
	ELSE
		GOSUB COMPUTER
	FI
	GOSUB CHECK
	PLAYER=-PLAYER
	IF WINNER THEN 1000
WEND

1000 ' EXIT LOOP
GOSUB DISPLAY
at 0,130:? cat(1);
IF WINNER=-1
	? "YOU WIN!"
ELIF WINNER=1
	? "I WIN!"
ELSE
	? "TIE!"
FI
? cat(0)
at 0,148
INPUT "PLAY AGAIN?",SEL
IF UCASE$(LEFT$(SEL,1))="Y" THEN GOTO RESTART
END

'===================
LABEL USER
pen on
while 1
	if pen(0)
		x=pen(4):y=pen(5)
		p=int((x-20)/40)+1
		if y>80
			p=p+6
		elif y>40
			p=p+3
		fi
		if mat(p)=0 then 3000 else beep
	fi
wend
3000 ' exit while
pen off
MAT(P)=PLAYER
RETURN

'===================
LABEL COMPUTER
PLAYER=1
GOSUB Check

' WINNER MOVE
FOR I=1 TO 8
	IF PW(I)=2
		GOSUB SELPOS
		GOTO CCFIN
	FI
NEXT

' DEFENCE MOVE
FOR I=1 TO 8
	IF PW(I)=-2
		GOSUB SELPOS
		GOTO CCFIN
	FI
NEXT
'
' SIMPLE MOVE - THIS LETS USER TO WIN
' BECAUSE IT DOES NOT CALCULATE THE
' FREE CELLS
'
IF MAT(5)=0
  P=5
ELSE
  FOR I=1 TO 9
    IF MAT(I)=0 THEN
      P=I
      GOTO CCFIN
    FI
  NEXT
FI

LABEL CCFIN
MAT(P)=PLAYER
RETURN

'===================
LABEL SELPOS
IF I < 4
	ST(1)=(I-1)*3+1
	ST(2)=(I-1)*3+2
	ST(3)=(I-1)*3+3
ELIF I < 7
	ST(1)=(I-3)
	ST(2)=(I-3)+3
	ST(3)=(I-3)+6
ELIF I=7
	ST(1)=1:ST(2)=5:ST(3)=9
ELSE
	ST(1)=3:ST(2)=5:ST(3)=7
FI

IF MAT(ST(1))=0
	P=ST(1)
ELIF MAT(ST(2))=0
	P=ST(2)
ELSE
	P=ST(3)
FI
RETURN

'===================
LABEL Check
FOR i=1 TO 3
	PW(i)=MAT((i-1)*3+1)
	PW(i)=PW(i)+MAT((i-1)*3+2)
	PW(i)=PW(i)+MAT((i-1)*3+3)
	PW(i+3)=MAT(i)+MAT(i+3)+MAT(i+6)
NEXT
PW(7)=MAT(1)+MAT(5)+MAT(9)
PW(8)=MAT(3)+MAT(5)+MAT(7)
FOR i=1 TO 8
	IF PW(i)=-3 THEN WINNER=-1
	IF PW(i)= 3 THEN WINNER=1
NEXT

IF WINNER=0 THEN 
	CNT=0
	FOR i=1 TO 9
		IF MAT(i) THEN CNT=CNT+1
	NEXT
	IF CNT=9 THEN WINNER=999
FI
RETURN 

'===================
LABEL DISPLAY
cls
rect 0,0,160,140 color 15 filled
line 20,40,140,40
line 20,80,140,80
line 60,0,60,120
line 100,0,100,120
FOR N=1 TO 9 STEP 3
	y=int(n/3)*40+20
	FOR O=0 TO 2
		x=(o+1)*40
		IF MAT(O+N)=1
			circle x,y,10
		ELIF MAT(O+N)=-1
			line x-10,y-10,x+10,y+10
			line x+10,y-10,x-10,y+10
		FI
	NEXT
NEXT
RETURN



