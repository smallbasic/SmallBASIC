FUNC d(x,y)
d=y*SIN(x)	
END

DIFFEQN 0, 1, 1, 1000, 1e-6, y, e USE d(x,y)
IF e=0 THEN
	PRINT y
ELSE
	PRINT "error"
FI
END

