'
' *****************************************************************
'
'                       FROM CALCULUS TO CHAOS
'       
'                    An Introduction to Dynamics
'
'                  
'                                by
'
'                           David Acheson
'
' ******************************************************************
'
'        The program which follows is part of the above book,
'            published in 1997 by Oxford University Press.
'                   ISBN 0-19-850077-7 (paperback)
'              
'
'                   Copyright ¸ David Acheson 1997
'
'
'
'
'                       Program 4:  NPHASE.BAS
'
'
n = 3
DIM x(n), xc(n), f(n), c1(n), c2(n), c3(n), c4(n)

REM ****** Setting up graphics ******
CLS
PAINT 1, 1, 9
xm = 30: ym = 60: tm = 15
REM VIEW 180, 17, 595, 432, 0, 14
' ---
xdif = 4
xstart = TXTW("0")*10 + xdif
xydim  = MIN(xmax - xstart, ymax - xdif)
xstart = (xmax - xydim) - xdif
VIEW xstart, xdif/2, xstart+xydim, xdif/2+xydim, 0, 13
' ---

WINDOW -xm*1000, -ym*1000, xm*1000, ym*1000
LINE -xm*1000, 0, xm*1000, 0, 11
LINE 0, -ym*1000, 0, ym*1000, 11
LOCATE 1, 1: PRINT xm;", ";ym
LOCATE 5, 1: PRINT "xi"
LOCATE 10, 2: PRINT "Time"
  
REM ****** Step-by-step method ******
 
LOCATE 3, 1: PRINT "try r=26"
LOCATE 4, 1: INPUT "r"; r

t = 0: xc(1) = 5: xc(2) = 5: xc(3) = 5
h = .003
   
REPEAT
	GOSUB Runge
	t = t + h
	PSET xc(1)*1000, xc(3)*1000, 14
	LOCATE 11, 1: PRINT t
UNTIL ABS(t - tm) < h / 2

LOCATE 6, 1
FOR i = 1 TO n:PRINT xc(i): NEXT
END
 
REM ----------------------------------

LABEL Equations
   
	f(1) = 10    * (x(2) - x(1))
	f(2) = -x(1) * x(3) + r * x(1) - x(2)
	f(3) = x(1)  * x(2) - 8 * x(3) / 3
	RETURN

REM ----------------------------------

LABEL Runge

	FOR i = 1 TO n: x(i) = xc(i): NEXT
    GOSUB Equations
	FOR i = 1 TO n: c1(i) = h * f(i): NEXT
  
	FOR i = 1 TO n: x(i) = xc(i) + c1(i) / 2: NEXT
	GOSUB Equations
	FOR i = 1 TO n: c2(i) = h * f(i): NEXT
  
	FOR i = 1 TO n: x(i) = xc(i) + c2(i) / 2: NEXT
	GOSUB Equations
	FOR i = 1 TO n: c3(i) = h * f(i): NEXT
  
	FOR i = 1 TO n: x(i) = xc(i) + c3(i): NEXT
	GOSUB Equations
	FOR i = 1 TO n: c4(i) = h * f(i): NEXT
    
	FOR i = 1 TO n
		xc(i) = xc(i) + (c1(i) + 2 * c2(i) + 2 * c3(i) + c4(i)) / 6
	NEXT
RETURN

