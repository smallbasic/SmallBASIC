DECLARE FUNCTION bob! (a!, b!)
DECLARE FUNCTION frac! (a!)
DECLARE FUNCTION frem! (a!, b!)
CLS
PRINT "x", "y", "MOD", "REM", "BOB"
FOR i = 1 TO 1000
    READ x, y
    IF y = 0 THEN EXIT FOR
    PRINT x, y, x MOD y, frem(x, y), bob(x, y)
NEXT
END

DATA 1,2
DATA 1,-2
DATA -1,2
DATA -1,-2
DATA 16.3,42.1
DATA 3,33
DATA 33,5
DATA 0,0

FUNCTION bob (a, b)
bob = frem(a, b) + b * (SGN(a) <> SGN(b))
END FUNCTION

FUNCTION frac (a)
frac = ABS(a) - INT(ABS(b))
END FUNCTION

FUNCTION frem (a, b)
frem = INT(a) - b * (a \ b)
END FUNCTION

