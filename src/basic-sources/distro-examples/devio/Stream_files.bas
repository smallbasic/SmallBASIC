' seq. file I/O test

F=FREEFILE

' Create & Write
OPEN "test.dat" FOR OUTPUT AS #F
PRINT #F, "Hello, world!"
CLOSE #F

' Open & Read
OPEN "test.dat" FOR INPUT AS #F
INPUT #F, a$, b$
PRINT a$,b$
'LINEINPUT #F, a$
CLOSE #F

' 
PRINT "I read: [";a$;"]+[";b$;"]"

' Append test
OPEN "test.dat" FOR APPEND AS #F
PRINT #F, "One more text line"
CLOSE #F

' EOF & LINE INPUT TEST
OPEN "test.dat" FOR INPUT AS #F
WHILE NOT EOF(F)
	LINEINPUT #F, a$
	PRINT "NL=[";a$;"]"
WEND
CLOSE #F




