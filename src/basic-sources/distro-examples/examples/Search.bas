FUNC cmp(x,y)
cmp=!(x=y)
END

DIM A(5)
FOR i=0 TO 5
    A(i)=5-i
NEXT
SEARCH A, 4, r USE cmp(x,y)
PRINT r:REM prints 1
PRINT A(r): REM prints 4

SEARCH A, 4, r
PRINT r:REM prints 1
PRINT A(r):REM prints 4

