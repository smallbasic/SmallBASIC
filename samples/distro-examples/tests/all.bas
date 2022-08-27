'
' Test for all built-in SUBs and FUNCs
' Originally generated from mkref.bas
'
dim a,b,c
s="catsanddogs"
s1="hello"
s2="there"
x=12.3
d=1:m=1:y=1
n=1234
a=[1,1;2,-1,-2;1,-2,2,1;1,-3,1,3;3,-1,-2]
b=[44,3,4,5,6]
inva=[1,-1,1;2,-1,2;3,2,-1]
def expression(x) = x * 0.1
DATA 1,"a"

print "ACCESS:" +IFF(ACCESS ("/etc/hostname") != 0, "<> 0", "0")
print "APPEND:"; : APPEND c, "1", "2", "3", "4": PRINT c
print "ARC:" ':ARC [STEP] x,y,r,astart,aend [,aspect [,color]] [COLOR color]
print "AT:" ':AT x, y
print "BEEP:" ':BEEP
print "BLOAD:" ':BLOAD "/etc/hostname"
print "BPUTC#:" ':BPUTC# fileN; byte
print "BSAVE:" ':BSAVE filename, address, length
print "CALL:" :aa=@expression: aa=CALL(aa,5):if(aa != 0.5) then throw "CALL failed"
print "CHART:" ':CHART LINECHART|BARCHART, array() [, style [, x1, y1, x2, y2]]
print "CHDIR:" ':CHDIR dir
print "CHMOD:" ':CHMOD file, mode
print "CIRCLE:" ':CIRCLE [STEP] x,y,r [,aspect [, color]] [COLOR color] [FILLED]
print "CLOSE:" ':CLOSE #fileN
print "CLS:" :CLS
print "COLOR:" :COLOR 1,2
print "COPY:" ':COPY "file", "newfile"
print "DATEDMY:";: DATEDMY(2459590,jd,jm,jy): print jd;jm;jy
print "DELAY:" ':DELAY ms
print "DELETE:" :DELETE a, 1
print "DERIV:" ':DERIV x, maxtries, maxerr, BYREF result, BYREF errcode USE expr
print "DIFFEQN:" ':DIFFEQN x0, y0, xf, maxseg, maxerr, BYREF yf, BYREF errcode USE expr
print "DIRWALK:" ':DIRWALK directory [, wildcards] [USE ...]
print "DRAW:" :DRAW ""
print "DRAWPOLY:" ':DRAWPOLY array [,x-origin,y-origin [, scalef [, color]]] [COLOR color] [FILLED]
print "EMPTY:" ':EMPTY (x)
print "ENV:" :ENV("foo=bar"): if (env("foo") != "bar") then throw "env failed"
print "EXPRSEQ:" : EXPRSEQ aa, -30, 70, 5 USE expression(x): if aa != [-3,-0.5,2,4.5,7] then throw "EXPRSEQ failed"
print "FORM(map):" ':FORM(map)
print "IMAGE:" ':IMAGE [#handle | fileName | http://path-to-file.png | image-var | array of pixmap data]
print "INPUT:" ':INPUT #fileN; var1 [,delim] [, var2 [,delim]] ...
print "INPUT:" ':INPUT [prompt,|;] var[, var [, ...]]
print "INSERT:" :InsIn=[1,2,3]:INSERT InsIn,2,"this","that","the","other":if(InsIn != [1,2,"other","the","that","this",3]) then throw "INSERT failed"
print "INTERSECT:" ':INTERSECT Ax, Ay, Bx, By, Cx, Cy, Dx, Dy, BYREF type, BYREF Rx, BYREF Ry
print "JOIN:" :JoinIn=["a","b","c"]:JOIN JoinIn(),"-",JoinOut:if(JoinOut != "a-b-c") then throw("JOIN failed")
print "KILL:" ':KILL "file"
print "LINE:" ':LINE [STEP] x,y [,|STEP x2,y2] [, color| COLOR color]
print "LINEINPUT:" ':LINEINPUT [#fileN] var
print "LINPUT:" ':LINPUT [#fileN] var
print "LOCATE:" ':LOCATE y, x
print "LOCK:" ':LOCK
print "LOGPRINT:" ':LOGPRINT ...
print "M3APPLY:" ': M3APPLY m3x3, polyko
print "M3IDENT:" ':M3IDENT m3x3
print "M3ROTATE:" ':M3ROTATE m3x3, angle
print "M3SCALE:" ':M3SCALE m3x3, x, y, Sx, Sy
print "M3TRANS:" ':M3TRANS m3x3, Tx, Ty
print "MKDIR:" ':MKDIR dir
print "NOSOUND:" :NOSOUND
print "OPEN:" ':OPEN file [FOR {INPUT|OUTPUT|APPEND}] AS #fileN
print "PAINT:" ':PAINT [STEP] x, y [,fill-color [,border-color]]
print "PAUSE:" ':PAUSE [secs]
print "PEN:" : PEN ON: PEN OFF
print "PLAY:" :PLAY "abcdefg"
print "PLOT:" 'PLOT xmin, xmax USE f(x)
print "POLYEXT:" :POLYEXT poly, xmin, ymin, xmax, ymax
print "PRINT:" ':PRINT [USING [format];] [expr|str [,|; [expr|str]] ...
print "PSET:" ':PSET [STEP] x,y [, color| COLOR color]
print "RANDOMIZE:" :RANDOMIZE 111
print "READ:" :READ VarRead1, VarRead2: if(VarRead1 != 1 OR VarRead2 != "a") then throw "READ failed"
print "RECT:" ':RECT [STEP] x,y [,|STEP x2,y2] [, color| COLOR color] [FILLED]
print "REDIM:" ':REDIM x
print "RENAME:" ':RENAME "file", "newname"
print "RMDIR:" ':RMDIR dir
print "ROOT:" ':ROOT low, high, segs, maxerr, BYREF result, BYREF errcode USE expr
print "SEARCH:" ':SEARCH A, key, BYREF ridx [USE cmpfunc]
print "SEEK:" ':SEEK #fileN; pos
print "SHOWPAGE:" :SHOWPAGE
print "SINPUT:" ':SINPUT src; var [, delim] [,var [, delim]] ...
print "SORT:" ':SORT array [USE cmpfunc]
print "SOUND:" ':SOUND freq, dur_ms [, vol] [BG]
print "SPLIT:" ':SPLIT string, delimiters, words() [, pairs] [USE expr]
print "SPRINT:" ':SPRINT var; [USING...;] ...
print "STKDUMP:" ':STKDUMP
print "SWAP:"; :sa=10:sb=20:SWAP sa,sb: PRINT sa;sb
print "THROW:" ':THROW [info [, ...]]
print "TIMEHMS:" ':TIMEHMS hms| timer, BYREF h, BYREF m, BYREF s
print "TLOAD:" ':TLOAD file, BYREF var [, type]
print "TRON:" :TRON
print "TROFF:" :TROFF
print "TSAVE:" ':TSAVE file, var
print "VIEW:" ':VIEW [x1,y1,x2,y2 [,color [,border-color]]]
print "WINDOW:" ':WINDOW [x1,y1,x2,y2]
print "WRITE:" ':WRITE #fileN; var1 [, ...]
print "ABS:" + ABS (12.2222)
print "ABSMAX:" + ABSMAX (1,2,3,4,5,6,7,8,9)
print "ABSMIN:" + ABSMIN (1,2,3,4,5,6,7,8,9)
print "ACOS:" + ACOS (x)
print "ACOSH:" + ACOSH (x)
print "ACOT:" + ACOT (x)
print "ACOTH:" + ACOTH (x)
print "ACSC:" + ACSC (x)
print "ACSCH:" + ACSCH (x)
print "ARRAY:" + ARRAY("[1,2,3,4,5,6,7,8,9]")
print "ASC:" + ASC (s)
print "ASEC:" + ASEC (x)
print "ASECH:" + ASECH (x)
print "ASIN:" + ASIN (x)
print "ASINH:" + ASINH (x)
print "ATAN:" + ATAN (x)
print "ATAN2:" + ATAN2 (x, y)
print "ATANH:" + ATANH (x)
print "ATN:" + ATN (x)
print "BCS:" + BCS (s)
print "BGETC:" '+ BGETC (fileN)
print "BIN:" + BIN (x)
print "CAT:" + CAT (x)
print "CBS:" + CBS (s)
print "CEIL:" + CEIL (x)
print "CHOP:" + CHOP ("123.45$")
print "CHR:" + CHR (87)
print "COS:" + COS (x)
print "COSH:" + COSH (x)
print "COT:" + COT (x)
print "COTH:" + COTH (x)
print "CSC:" + CSC (x)
print "CSCH:" + CSCH (x)
print "DATE:"' + DATE
print "DATEFMT:" + DATEFMT("ddmmyy", 12345) + " " + DATEFMT("yyymmdd", d,m,y): xx=datefmt(0,date)
print "DEFINEKEY:"' + DEFINEKEY k,sub
print "DEG:" + DEG (x)
print "DETERM:"' + DETERM (A, 1)
print "DISCLOSE:" + DISCLOSE("{debraceme}", "{}")
print "ENCLOSE:" + ENCLOSE ("braceme", "{}")
print "EOF:"' + EOF (fileN)
print "EXIST:"' + EXIST (file)
print "EXP:" + EXP (x)
print "FILES:"; FILES ("file-not-found")
print "FIX:" + FIX (x)
print "FLOOR:" + FLOOR (x)
print "FORMAT:" + FORMAT ("XXXX", 9999)
print "FRAC:" + FRAC (x)
print "FRE:" '+ FRE (x)
print "FREEFILE:" + FREEFILE
print "HEX:" + HEX (x)
print "IFF:" + IFF (1+1==2, "1+1=2", "1+1<>2")
print "INKEY:"' + INKEY
print "INPUT:"' + INPUT (len [, fileN])
print "INSTR:" + INSTR (2, s1, s2)
print "INT:" + INT (x)
print "INVERSE:"; INVERSE(inva)
print "ISARRAY:" + ISARRAY (a)
print "ISDIR:" + ISDIR (x)
print "ISFILE:" + ISFILE (x)
print "ISLINK:" + ISLINK (x)
print "ISMAP:" + ISMAP (x)
print "ISNUMBER:" + ISNUMBER (x)
print "ISSTRING:" + ISSTRING (s)
print "JULIAN:" + JULIAN(d,m,y)
print "LBOUND:" + LBOUND (a, 1)
print "LCASE:" + LCASE (s)
print "LEFT:" + LEFT (s,2)
print "LEFTOF:" + LEFTOF (s1, s2)
print "LEFTOFLAST:" + LEFTOFLAST (s1, s2)
print "LEN(d):" + LEN(d)
print "LINEQN:"'; LINEQN (inva, b)
print "LOF:"' + LOF (1)
print "LOG:" + LOG (x)
print "LOG10:" + LOG10 (x)
print "LOWER:" + LOWER (s)
print "LTRIM:" + LTRIM (s)
print "MAX:" + MAX (1,2,3,4,5,6,7,8,9)
print "MID:" + MID (s,2,4)
print "MIN:" + MIN (1,2,3,4,5,6,7,8,9)
print "OCT:" + OCT (x)
print "PEN:" + PEN (1)
print "POINT:" + POINT (1,2)
print "POLYAREA:" + POLYAREA (poly)
print "POLYCENT:"' + POLYCENT(a)
print "POW:" + POW (x, y)
print "PROGLINE:" + PROGLINE
print "PTDISTLN:" + PTDISTLN (Bx,By,Cx,Cy,Ax,Ay)
print "PTDISTSEG:" + PTDISTSEG (Bx,By,Cx,Cy,Ax,Ay)
print "PTSIGN:" + PTSIGN (Ax,Ay,Bx,By,Qx,Qy)
print "RAD:" + RAD (x)
print "REPLACE:" + REPLACE ("source", 1, "sorce", 2)
print "RGB:" + RGB (80,80,80)
print "RGBF:" + RGBF (.1,.1,.1)
print "RIGHT:" + RIGHT (s,2)
print "RIGHTOF:" + RIGHTOF (s1, s2)
print "RIGHTOFLAST:" + RIGHTOFLAST (s1, s2)
print "RINSTR:" + RINSTR (2, s1, s2)
print "RND:" + iff(RND>=0 && RND<=1.0,1,0)
print "ROUND:" + ROUND (x,22)
print "RTRIM:" + RTRIM (s)
print "RUN:" '+ RUN cmdstr
print "SEC:" + SEC (x)
print "SECH:" + SECH (x)
print "SEEK:" '+ SEEK (fileN)
print "SEGCOS:" + SEGCOS (Ax,Ay,Bx,By,Cx,Cy,Dx,Dy)
print "SEGLEN:" + SEGLEN (Ax,Ay,Bx,By)
print "SEGSIN:" + SEGSIN (Ax,Ay,Bx,By,Cx,Cy,Dx,Dy)
print "SEQ:"' + SEQ (100,200,1)
print "SGN:" + SGN (x)
print "SIN:" + SIN (x)
print "SINH:" + SINH (x)
print "SPACE:" + SPACE (2)+ "<"
print "SPC:" + SPC (2) + "<"
print "SQR:" + SQR (x)
print "SQUEEZE:" + SQUEEZE (s)
print "STATMEAN:" + STATMEAN (1,2,3,4,5,6,7,8,9)
print "STATMEANDEV:" + STATMEANDEV (1,2,3,4,5,6,7,8,9)
print "STATSPREADP:" + STATSPREADP (1,2,3,4,5,6,7,8,9)
print "STATSPREADS:" + STATSPREADS (1,2,3,4,5,6,7,8,9)
print "STR:" + STR (5)
print "STRING:"; STRING(5, "Strings")
print "SUM:" + SUM (1,2,3,4,5,6,7,8,9)
print "SUMSQ:" + SUMSQ (1,2,3,4,5,6,7,8,9)
print "TAB:" + TAB (8) + "HERE"
print "TAN:" + TAN (x)
print "TANH:" + TANH (x)
print "TEXTHEIGHT:" + TEXTHEIGHT (s)
print "TEXTWIDTH:" + TEXTWIDTH (s)
print "TICKS:"' + TICKS
print "TIME:"' + TIME
print "TIMER:"' + TIMER
print "TIMESTAMP:" '+ TIMESTAMP filename
print "TRANSLATE:" + TRANSLATE ("source", "what", "with")
print "TRIM(s):" + TRIM("   s with spaces     ")
print "TXTH:" + TXTH (s)
print "TXTW:" + TXTW (s)
print "UBOUND:" + UBOUND (a, 1)
print "UCASE:" + UCASE (s)
print "UPPER:" + UPPER (s)
print "VAL:" + VAL (s)
print "WEEKDAY:" + WEEKDAY(dmy) + " " + WEEKDAY(1,1,2020) + " " + WEEKDAY(JULIAN(d,m,y))
print "XPOS:" + XPOS
print "YPOS:" + YPOS
