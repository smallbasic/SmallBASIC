#sec:Main

CONST s$=CHR$(27)+"[90m"
CONST n$=CAT(0)
REPEAT
  INPUT x
  IF !x THEN STOP
  x0=x
  f$=""
  f=2
  p=0
  WHILE x>1
    tmp=x/f
    if tmp=int(tmp) THEN
      p=p+1
      x=tmp
    ELSE
      IF p>0 THEN
        f$=f$+str$(f)
        IF p>1 THEN
          f$=f$+s$+str$(p)+n$
        ENDIF
        f$=f$+" x "
        p=0
      ENDIF
      f=f+1+(f>2)
      IF f>sqr(x) THEN f=x
    ENDIF
  WEND
  f$=f$+str$(f)
  IF p=1 THEN
    IF f=x0 THEN
      f$=f$+" is prime"
    ENDIF
  ELSE
    f$=f$+s$+str$(p)+n$
  ENDIF
PRINT f$
UNTIL
END

