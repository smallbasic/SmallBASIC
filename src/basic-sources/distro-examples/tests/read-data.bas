#!/usr/bin/sbasic -g
' READ/DATA 

restore 500
DIM A(3)

FOR I=1 TO 3
	READ A(I)
	? A(I)
NEXT
END

500
DATA 0.3333, 1, "HELLO"



