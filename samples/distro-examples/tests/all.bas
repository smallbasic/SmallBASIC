'
' Test for all built-in SUBs and FUNCs
' Originally generated from mkref.bas
'
dim a,b
s="catsanddogs"
s1="hello"
s2="there"
x=12.3
d=1:m=1:y=1
n=1234

'ACCESS (file)
APPEND a, "1", "2", "3", "4"
'ARC [STEP] x,y,r,astart,aend [,aspect [,color]] [COLOR color]
'AT x, y
'BEEP
'BLOAD filename[, address]
'BPUTC# fileN; byte
'BSAVE filename, address, length
'CALL (fp)
'CHART LINECHART|BARCHART, array() [, style [, x1, y1, x2, y2]]
'CHDIR dir
'CHMOD file, mode
'CIRCLE [STEP] x,y,r [,aspect [, color]] [COLOR color] [FILLED]
'CLOSE #fileN
'CLS
'COLOR foreground-color [, background-color]
'COPY "file", "newfile"
'DATEDMY dmy| julian_date, BYREF d, BYREF m, BYREF y
'DELAY ms
DELETE a, 1
'DERIV x, maxtries, maxerr, BYREF result, BYREF errcode USE expr
'DIFFEQN x0, y0, xf, maxseg, maxerr, BYREF yf, BYREF errcode USE expr
'DIRWALK directory [, wildcards] [USE ...]
'DRAW "commands"
'DRAWPOLY array [,x-origin,y-origin [, scalef [, color]]] [COLOR color] [FILLED]
'EMPTY (x)
ENV("foo=bar"): if (env("foo") != "bar") then throw "env failed"
'EXPRSEQ BYREF array, xmin, xmax, count USE expression
'FORM(map)
'IMAGE [#handle | fileName | http://path-to-file.png | image-var | array of pixmap data]
'INPUT #fileN; var1 [,delim] [, var2 [,delim]] ...
'INPUT [prompt,|;] var[, var [, ...]]
'INSERT a, idx, val [, val [, ...]]]
'INTERSECT Ax, Ay, Bx, By, Cx, Cy, Dx, Dy, BYREF type, BYREF Rx, BYREF Ry
'JOIN words(), delimiters, string
'KILL "file"
'LINE [STEP] x,y [,|STEP x2,y2] [, color| COLOR color]
'LINEINPUT [#fileN] var
'LINPUT [#fileN] var
'LOCATE y, x
'LOCK
'LOGPRINT "hello"
'M3APPLY m3x3, BYREF poly
'M3IDENT BYREF m3x3
'M3ROTATE BYREF m3x3, angle [, x, y]
'M3SCALE BYREF m3x3, x, y, Sx, Sy
'M3TRANS BYREF m3x3, Tx, Ty
'MKDIR dir
NOSOUND
'OPEN file [FOR {INPUT|OUTPUT|APPEND}] AS #fileN
'PAINT [STEP] x, y [,fill-color [,border-color]]
'PAUSE 0
PEN OFF
PLAY "aaaaa"
'PLOT xmin, xmax USE f(x)
'POLYEXT poly(), BYREF xmin, BYREF ymin, BYREF xmax, BYREF ymax
'PRINT [USING [format];] [expr|str [,|; [expr|str]] ...
'PSET [STEP] x,y [, color| COLOR color]
RANDOMIZE 111
'READ var[, var ...]
'RECT [STEP] x,y [,|STEP x2,y2] [, color| COLOR color] [FILLED]
'REDIM x
'RENAME "file", "newname"
'RMDIR dir
'ROOT low, high, segs, maxerr, BYREF result, BYREF errcode USE expr
'SEARCH A, key, BYREF ridx [USE cmpfunc]
'SEEK #fileN; pos
SHOWPAGE
'SINPUT src; var [, delim] [,var [, delim]] ...
'SORT array [USE cmpfunc]
'SOUND freq, dur_ms [, vol] [BG]
'SPLIT string, delimiters, words() [, pairs] [USE expr]
'SPRINT var; [USING...;] ...
'STKDUMP
SWAP a, b
'THROW [info [, ...]]
'TIMEHMS hms| timer, BYREF h, BYREF m, BYREF s
'TLOAD file, BYREF var [, type]
'TROFF
'TRON
'TSAVE file, var
'VIEW [x1,y1,x2,y2 [,color [,border-color]]]
'WINDOW [x1,y1,x2,y2]
'WRITE #fileN; var1 [, ...]
print ABS (x)
print ABSMAX (1,2,3,4,5)
print ABSMIN (1,2,3,4,5)
print ACOS (x)
print ACOSH (x)
print ACOT (x)
print ACOTH (x)
print ACSC (x)
print ACSCH (x)
'print ARRAY('[1,2,3,4]')
print ASC (s)
print ASEC (x)
print ASECH (x)
print ASIN (x)
print ASINH (x)
print ATAN (x)
print ATAN2 (x, y)
print ATANH (x)
print ATN (x)
print BCS (s)
'print BGETC (fileN)
print BIN (x)
print CAT (x)
print CBS (s)
print CDBL (x)
print CEIL (x)
print CHOP (source)
print CHR (x)
print CINT (x)
print COS (x)
print COSH (x)
print COT (x)
print COTH (x)
print CREAL (x)
print CSC (x)
print CSCH (x)
print DATE
print DATEFMT ("dmy", d,m,y)
'print DEFINEKEY k,sub
print DEG (x)
'print DETERM (A[, toler])
'print DISCLOSE (str[, pairs [, ignore-pairs]])
'print ENCLOSE (str[, pair])
'print ENV expr
'print EOF (fileN)
'print EXIST (file)
print EXP (x)
print FILES (wildcards)
print FIX (x)
print FLOOR (x)
'print FORMAT (format, val)
print FRAC (x)
print FRE (x)
print FREEFILE
print HEX (x)
print IFF (1,2,3)
'print INKEY
'print INPUT (len [, fileN])
'print INSTR ([start,] s1, s2)
print INT (x)
'print INVERSE (A)
print ISARRAY (a)
print ISDIR (x)
print ISFILE (x)
print ISLINK (x)
print ISMAP (x)
print ISNUMBER (x)
print ISREF (x)
print ISSTRING (x)
print JULIAN (d,m,y)
print LBOUND (a, 1)
print LCASE (s)
print LEFT (s, 1)
print LEFTOF (s1, s2)
print LEFTOFLAST (s1, s2)
print LEN(a)
'print LINEQN (a, b [, toler])
'print LOF ("all.bas")
print LOG (x)
print LOG10 (x)
print LOWER (s)
print LTRIM (s)
print MAX (1,2,3,4,5)
print MID (s, 2,4)
print MIN (6,5,4,3,2)
print OCT (x)
'print PEN (0..14)
'print POINT (x [, y])
print POLYAREA (poly)
'print POLYCENT
print POW (10,2)
print PROGLINE
'print PTDISTLN (Bx,By,Cx,Cy,Ax,Ay)
'print PTDISTSEG (Bx,By,Cx,Cy,Ax,Ay)
'print PTSIGN (Ax,Ay,Bx,By,Qx,Qy)
print RAD (33.3)
print REPLACE (s, 2, "zzzz", 2)
print RGB (128,128,128)
print RGBF (.99,.99,.99)
print RIGHT (s,4)
print RIGHTOF (s1, s2)
print RIGHTOFLAST (s1, s2)
print RINSTR (2, s1, s2)
print RND
print ROUND (x, 2)
print RTRIM (s)
'print RUN cmdstr
print SEC (x)
print SECH (x)
'print SEEK (fileN)
print SEGCOS (Ax,Ay,Bx,By,Cx,Cy,Dx,Dy)
print SEGLEN (Ax,Ay,Bx,By)
print SEGSIN (Ax,Ay,Bx,By,Cx,Cy,Dx,Dy)
print SEQ (xmin, xmax, count)
print SGN (x)
print SIN (x)
print SINH (x)
print SPACE (n)
print SPC (n)
print SQR (x)
print SQUEEZE (s)
'print STATMEAN (...)
'print STATMEANDEV (...)
'print STATSPREADP (...)
'print STATSPREADS (...)
print STR (n)
print STRING(10, "strings")
print SUM (1,2,3,4)
print SUMSQ (2,3,4,5)
print TAB (n)
print TAN (x)
print TANH (x)
print TEXTHEIGHT (s)
print TEXTWIDTH (s)
print TICKS
print TIME
print TIMER
'print TIMESTAMP filename
print TRANSLATE ("source", "what", "with")
print TRIM(s)
print TXTH (s)
print TXTW (s)
print UBOUND (a,1)
print UCASE (s)
print UPPER (s)
print VAL (s)
print WEEKDAY(dmy); WEEKDAY(d,m,y); WEEKDAY(JULIAN(d,m,y))
print XPOS
print YPOS
