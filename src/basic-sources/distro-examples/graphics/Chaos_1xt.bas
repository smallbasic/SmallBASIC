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
'                        Program 1:  1XT.BAS
'
'
REM ****** Setting up graphics ******

CLS
xm = 3: tm = 5
  
' ---
xdif = 4
xstart = TXTW("0")*10 + xdif
xydim  = MIN(xmax - xstart, ymax - xdif)
xstart = (xmax - xydim) - xdif
VIEW xstart, xdif/2, xstart+xydim, xdif/2+xydim, 0, 13
' ---

  WINDOW 0, -xm*1000, tm*1000, xm*1000
  LINE 0, 0, tm*1000, 0, 11: LINE 0, -xm*1000, 0, xm*1000, 11
  LOCATE 1, 1: PRINT tm;", "; xm

 
REM ****** Function f(x,t) ******
 
  DEF fnf(x, t)
  	fnf = (1 + t) * x + 1 - 3 * t + t ^ 2
  END
 
REM ****** Direction field ******

p = 25
  FOR x = xm TO -xm STEP -2 * xm / p
    FOR t = 0 TO tm STEP tm / p
      x1 = fnf(x, t) / xm: t1 = 2 / tm
      s = p * SQR(x1 ^ 2 + t1 ^ 2)
	  x2 = fnf(x, t) / s: t2 = 1 / s

      LINE t*1000, x*1000, (t + t2) * 1000, (x + x2) * 1000, 9
      CIRCLE (t + t2) * 1000, (x + x2) * 1000, .003 * tm * 1000, 9
    NEXT t
  NEXT x

REM ****** Step-by-step method ******

WHILE 1
  t = 0
  LOCATE 2, 1: INPUT "x0"; x
  h = .01
    REPEAT
       GOSUB Runge
       t = t + h
       PSET t * 1000, x * 1000, 14
    UNTIL ABS(t - tm) < h / 2 OR ABS(x) > xm
    LOCATE 4, 1: PRINT "t="; t
    LOCATE 5, 1: PRINT "x="; x
WEND

REM ****** Subroutines ******

LABEL Euler
   
    c1 = h * fnf(x, t)
    x = x + c1
    RETURN

LABEL ImpEuler
   
    c1 = h * fnf(x, t)
    c2 = h * fnf(x + c1, t + h)
    x = x + (c1 + c2) / 2
    RETURN

LABEL Runge
   
    c1 = h * fnf(x, t)
    c2 = h * fnf(x + c1 / 2, t + h / 2)
    c3 = h * fnf(x + c2 / 2, t + h / 2)
    c4 = h * fnf(x + c3, t + h)
    x = x + (c1 + 2 * c2 + 2 * c3 + c4) / 6
    RETURN

