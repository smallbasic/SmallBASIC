'app-plug-in
'menu Games/Klotski Puzzle

#sec:Main
' Klotski_puzzle.bas
' 11/06/2005
' Ported by Jorge Hernandez

PEN ON
DIM C(20)
dim a(10,10)
c(0)=-val("&h000000")
c(1)=-val("&h00FF00")
c(2)=-val("&hFF0000")
c(3)=-val("&h0000FF")
c(4)=-val("&hC0FF00")
c(5)=-val("&hF6A500")
c(6)=-val("&h808080")
c(7)=-val("&hFF00FF")
c(8)=-val("&h800000")
c(9)=-val("&h808000")
c(10)=-val("&h00FFFF")
c(15)=-val("&hFFFFFF")

5 '
M=5:N=4
a(1,1)=1:a(1,2)=2:a(1,3)=2:a(1,4)=3
a(2,1)=1:a(2,2)=2:a(2,3)=2:a(2,4)=3
a(3,1)=4:a(3,2)=5:a(3,3)=5:a(3,4)=6
a(4,1)=4:a(4,2)=7:a(4,3)=11:a(4,4)=6
a(5,1)=9:a(5,2)=15:a(5,3)=15:a(5,4)=10

10 'cls
rect 36,36,203,244 COLOR 0
rect 35,256,80,277 COLOR 0
rect 150,256,202,277 COLOR 0
AT 42,256

print "Quit"
AT 156,256
print "Reset"
AT 35,5
print "Klotski Puzzle"
AT 230,30
print "Move"
AT 230,50
print "Red"
AT 230,70
print "Block"
AT 230,120
print "To"
AT 230,170
print "Lower"
AT 230,190
print "Center"
f1=0:f2=1
for I=1 to M:for J=1 to N
y=I*40:x=J*40
RECT x,y,x+40,y+40,c(a(I,J)) FILLED
next J:next I

15'
while pen(0)=0:wend
CX=PEN(4):CY=PEN(5)
if CX>35 and CX<80 and CY>256 and CY<277 then END
if CX>152 and CX<202 and CY>260 and CY<280 then goto 5
TX=CX:TY=CY
J=INT(CX/40):I=INT(CY/40)
p=a(I,J):if p=15 then 15

'while pen(0) =0:wend
while pen(3)
CX=PEN(4):CY=PEN(5)
wend
DX=ABS(CX-TX):DY=ABS(CY-TY)
IF (DX>DY) THEN
    IF (CX>TX) THEN
    de$="r"
    else
    de$="l"
    ENDIF
ENDIF
IF (DY>DX) THEN
    IF (CY>TY) THEN
    de$="d"
    else
    de$="u"
    ENDIF
ENDIF
'while pen(3):wend
IF de$="r" THEN GOSUB 200:IF (f1*f2) = 1 THEN GOSUB 300
IF de$="l" THEN GOSUB 400:IF (f1*f2) = 1 THEN GOSUB 500
IF de$="d" THEN GOSUB 600:IF (f1*f2) = 1 THEN GOSUB 700
IF de$="u" THEN GOSUB 800:IF (f1*f2) = 1 THEN GOSUB 900
GOTO 10

200 ' Empty to the right ?
     O=N-1
    FOR I = 1 TO M
    FOR J = 1 TO O
    if (a(I,J)= P) AND (a(I,J+1)=15)THEN
     f1 = 1
    endif
    if (a(I,J)= P) AND (a(I,J+1)<>15) AND (a(I,J+1)<>P )THEN
    f2 = 0
     endif
if a(I,N)=P then  f2 = 0     
    NEXT J
    NEXT I
RETURN

300 ' Movement to the right
O = N-1
        FOR I= 1 TO M:FOR J=O TO 1 STEP -1
        IF a(I,J)=P THEN
        a(I,J+1)=P
        IF a(I,J-1)=P THEN a(I,J)=P ELSE a(I,J)=15
     endif
    NEXT J
    NEXT I
RETURN

400 ' Empty to the left ?
    FOR I=1 TO M:FOR J=2 TO N
    IF (a(I,J)= P) AND (a(I,J-1)=15) THEN f1 = 1
    IF (a(I,J)= P) AND (a(I,J-1)<>15) AND (a(I,J-1)<>P) THEN f2 = 0
    IF a(I,1)=P then f2 = 0
    NEXT J
    NEXT I
RETURN

500 ' Movement to left
      FOR I= 1 TO M:FOR J=1 TO N
      IF a(I,J)=P AND J>1 THEN
      a(I,J-1)=P
      IF a(I,J+1)=P THEN a(I,J)=P ELSE a(I,J)=15
      ENDIF 
    NEXT J
    NEXT I
RETURN

600 ' Empy below ?
    O = M-1
    FOR I=1 TO O:FOR J=1 TO N
    IF (a(I,J)= P) AND (a(I+1,J)=15) THEN f1 = 1
    IF (a(I,J)= P) AND (a(I+1,J)<>15) AND (a(I+1,J)<>P) THEN f2 = 0
    IF a(M,J)=P then f2 = 0
    NEXT J
    NEXT I
RETURN

700 '  Movement down
O = M-1
    FOR I = O TO 1 STEP -1:FOR J=1 TO N
      IF a(I,J)=P THEN
      a(I+1,J)=P
      IF a(I-1,J)=P THEN a(I,J)=P ELSE a(I,J)=15
      ENDIF
    NEXT J
    NEXT I
RETURN

800 '  Empty up ?
    FOR I=2 TO M:FOR J=1 TO N
    IF (a(I,J)= P) AND (a(I-1,J)=15) THEN f1 = 1
    IF (a(I,J)= P) AND (a(I-1,J)<>15) AND (a(I-1,J)<>P) THEN f2 = 0
    IF a(1,J)=P then f2 = 0
    NEXT J
    NEXT I
RETURN

900 '  Movement up
    FOR I = 2 TO M:FOR J=1 TO N
      IF a(I,J)=P THEN
      a(I-1,J)=P
      IF a(I+1,J)=P THEN a(I,J)=P ELSE a(I,J)=15
    ENDIF
    NEXT J
    NEXT I
RETURN
