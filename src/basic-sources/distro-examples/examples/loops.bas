
PRINT "=== For/Next"

FOR C=1 TO 9
	PRINT C
NEXT

PRINT "=== While/Wend"

C=1
WHILE C<10
	PRINT C
	C=C+1
WEND

PRINT "=== Repeat/Until"

C=1
REPEAT
	PRINT C
	C=C+1
UNTIL C=10

PRINT "=== For-IN/Next"

A=[1,2,3,4,5,6,7,8,9]
FOR C IN A
	PRINT C
NEXT


