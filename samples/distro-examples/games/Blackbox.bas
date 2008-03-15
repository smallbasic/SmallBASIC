#sec:Main
'Blackbox - a favorite on my old Mac.
'SmallBASIC version by B. Riess 5/23/01
CONST debug=FALSE
CONST debug2=FALSE
CONST box=10
CONST bxs=11
CONST slv=80-box*bxs/2
DIM locs(0 TO bxs-1,0 TO bxs-1)

CONST esc$=CHR$(27)
CONST n$=esc$+"[0m"
CONST f$=esc$+"[91m"
CONST fw=5
CONST fh=9
CONST g$=esc$+"[90m"
CONST gw=4
CONST gh=9
CONST white=15
CONST gray=10
CONST black=0
CONST new="N"
CONST quit="Q"
CONST trc="T"

CONST trow=asc("A")
CONST rcol=asc("A")+bxs
CONST lcol=asc("a")
CONST brow=asc("a")+bxs

PRINT "You are searching for a number of"
PRINT "objects hidden in the array. You"
PRINT "find them by watching the effect of"
PRINT "laser beams sent into the array. A"
PRINT "beam may hit an object, be deflected"
PRINT "90° by objects, or may emerge. To"
PRINT "send a beam, tap a letter on the"
PRINT "edge. A black circle appears if the"
PRINT "beam hit an object. An open circle"
PRINT "appears if the beam reflected back"
PRINT "out at the entry point (note that an"
PRINT "object on the perimeter will reflect"
AT 3,148 : PRINT "[Continue]"; : GETPEN : CLS
PRINT "a beam entering an adjacent cell)."
PRINT "Otherwise, a letter corresponding to"
PRINT "the exit point appears. If you have"
PRINT "found an object, tap the cell to"
PRINT "make it appear. Tap ";CAT(1);"N";CAT(0);" to start a new"
PRINT "game, ";CAT(1);"Q";CAT(0);" to quit. Tap the number of"
PRINT "objects left to reveal all of them."
PRINT "Tap ";CAT(1);"T";CAT(0);" to toggle a display of the"
PRINT "beam's path."
PRINT
PRINT "           Ready?"
AT 3,148 : PRINT "[Begin]"; : GETPEN


LABEL NEW

CLS
IF !debug THEN
  INPUT "# of objects (max="+STR$(bxs*bxs)+") ";objs
  objs=MAX(MIN(objs,bxs*bxs),1)
ENDIF
IF debug2 THEN
  INPUT "Show coordinates ";d2on
  d2on=LCASE$(LEFT$(LTRIM$(d2on),1))="y"
ENDIF
CLS
FOR x=0 TO bxs-1
  FOR y=0 TO bxs-1
    locs(x,y)=0
  NEXT
NEXT

RECT 0,0,159,159

' display verticals

FOR n=0 TO bxs
  IF n=0 OR n=bxs THEN
    LINE slv+box*n,0,slv+box*n,159
  ELSE
    LINE slv+box*n,slv,slv+box*n,slv+box*bxs
  ENDIF
NEXT


' display horizontals

FOR n=0 TO bxs
  IF n=0 OR n=bxs THEN 
    LINE 0,slv+box*n,159,slv+box*n
  ELSE
    LINE slv,slv+box*n,slv+box*bxs,slv+box*n
  ENDIF
NEXT


' label the rows and columns

FOR i=0 TO bxs-1

  ' top row
  AT slv+1+box*i+(box-fw)/2,slv-2*box+(box-fh)/2
  PRINT f$;CHR$(trow+i);n$;

  ' right column
  AT slv+box*bxs+box+(box-fw)/2,slv+1+box*i+(box-fh)/2
  PRINT f$;CHR$(rcol+i);n$;

  ' left column
  AT slv+1-2*box+(box-fw)/2,slv+1+box*i+(box-fh)/2
  PRINT f$;CHR$(lcol+i);n$;

  ' bottom row
  AT slv+1+box*i+(box-fw)/2,slv+box*bxs+box+(box-fh)/2
  PRINT f$;CHR$(brow+i);n$;

NEXT


' set the objects
IF !debug THEN
  FOR i=1 TO objs
    REPEAT
     x=INT(RND*bxs)
     y=INT(RND*bxs)
    UNTIL locs(x,y)=0
    locs(x,y)=1
  NEXT
ELSE
  objs=0
  REPEAT
    SHOWOBJCT
    GETPEN
    IF NOT inslv THEN
      IF locs(x0,y0) THEN
        SHOWCELL x0,y0,white
        locs(x0,y0)=0
        objs=objs-1
      ELSE
        SHOWCELL x0,y0,gray
        locs(x0,y0)=1
        objs=objs+1
      ENDIF
    ELSE
      FOR x=0 TO bxs-1
        FOR y=0 TO bxs-1
          IF locs(x,y) THEN
            SHOWCELL x,y,white
          ENDIF
        NEXT
      NEXT
      EXIT LOOP
    ENDIF
  UNTIL
ENDIF

hide=1
shown=FALSE
trace=0
traced=FALSE

IF !(debug2 AND d2on) THEN
  AT (slv-TXTW(new)+LEN(new))/2,(slv-TXTH(new))/2
  PRINT new;
  SHOWOBJCT
  AT slv+box*bxs+(slv-TXTW(quit)+LEN(quit))/2,slv+box*bxs+(slv-TXTH(quit))/2
  PRINT quit;
  AT (slv-TXTW(trc)+LEN(trc))/2,slv+box*bxs+(slv-TXTH(trc))/2
  PRINT trc;
ENDIF


' main loop

REPEAT
  GETPEN
  IF incor THEN
    IF inrcol AND inbrow THEN
      ' quit
      CLS
      STOP
    ELSEIF inlcol AND introw THEN
      ' new
      GOTO NEW
    ELSEIF inlcol AND inbrow THEN
      ' trace
      traced=TRUE
      trace=2-trace
	IF !(debug2 AND d2on)
		AT (slv-TXTW(trc)+LEN(trc))/2,slv+box*bxs+(slv-TXTH(trc))/2
		PRINT CAT(trace);trc;CAT(0);
	ENDIF
    ELSEIF inrcol AND introw THEN
      ' reveal
      shown=TRUE
      hide=1-hide
      FOR y=0 TO bxs-1
        FOR x=0 TO bxs-1
          IF locs(x,y)=1 THEN
            SHOWCELL x,y,hide*(white-gray)+gray
          ENDIF
        NEXT
      NEXT
    ENDIF
  ELSEIF inslv THEN

    IF introw THEN
      ' entry in top row, going down
      dx=0
      dy=1

    ELSEIF inbrow THEN
      ' entry in bottom row, going up
      dx=0
      dy=-1

    ELSEIF inlcol THEN
      ' entry in left column, going right
      dx=1
      dy=0

    ELSE
      ' entry in right column, going left
      dx=-1
      dy=0

    ENDIF

    ' preserve entry point
    nx=x0
    ny=y0
    GOTO 1900


1000

    ' reset on change of direction
    nx=x
    ny=y

1100

    IF debug2 AND d2on THEN
      AT 130,0
      PRINT f$;"nx=";nx;"   ";n$;
      AT 130,12
      PRINT f$;"ny=";ny;"   ";n$;

      AT 0,0
      PRINT f$;"x=";x;"   ";n$;
      AT 0,12
      PRINT f$;"y=";y;"   ";n$;

      AT 126,136
      PRINT f$;"dx=";dx;"   ";n$;
      AT 126,148
      PRINT f$;"dy=";dy;"   ";n$;

      AT 0,136
      PRINT f$;"x0=";x0;"     ";n$;
      AT 0,148
      PRINT f$;"y0=";y0;"     ";n$;
    ENDIF

    ' determine if reached an edge
    IF nx=-1 THEN
      outat=lcol+y
    ELSEIF nx=bxs THEN
      outat=rcol+y
    ELSEIF ny=-1 THEN
      outat=trow+x
    ELSEIF ny=bxs THEN
      outat=brow+x
    ELSE
      GOTO 1500
    ENDIF
    IF nx=x0 AND ny=y0 THEN
      ' reflected
      CIRCLE box*x0+box/2+slv,box*y0+box/2+slv,2 COLOR gray
    ELSE
      AT slv+1+box*x0+(box-gw)/2,slv+3+box*y0+(box-gh)/2
      PRINT g$;CHR$(outat);n$;
    ENDIF
    GOTO 2000

1500
    IF locs(nx,ny) THEN
      CIRCLE box*x0+box/2+slv,box*y0+box/2+slv,2 COLOR gray FILLED
      GOTO 2000
    ENDIF

    IF dx=0 THEN
      ' now moving up or down
      IF nx>0 THEN
        IF locs(nx-1,ny) THEN
          ' change to moving right
          dx=1
          dy=0
          GOTO 1000
        ENDIF
      ENDIF
      IF nx<bxs-1 THEN
        IF locs(nx+1,ny) THEN
          ' change to moving left
          dx=-1
          dy=0
          GOTO 1000
        ENDIF
      ENDIF
    ELSE
      ' now moving left or right
      IF ny>0 THEN
        IF locs(nx,ny-1) THEN
          ' change to moving down
          dx=0
          dy=1
          GOTO 1000
        ENDIF
      ENDIF
      IF ny<bxs-1 THEN
        IF locs(nx,ny+1) THEN
          ' change to moving up
          dx=0
          dy=-1
          GOTO 1000
        ENDIF
      ENDIF
    ENDIF

    ' show path
    IF trace THEN
      CIRCLE box*nx+box/2+slv,box*ny+box/2+slv,1 COLOR black FILLED
        IF debug2 AND d2on THEN
          PAUSE
        ELSE
          DELAY 75
        ENDIF
      CIRCLE box*nx+box/2+slv,box*ny+box/2+slv,1 COLOR white FILLED
    ENDIF
    ' end show path

1900
    x=nx
    nx=x+dx
    y=ny
    ny=y+dy
    GOTO 1100

  ELSE

    IF locs(x0,y0)=1 THEN
      locs(x0,y0)=2
      SHOWCELL x0,y0,black
      objs=objs-1
      SHOWOBJCT
      IF objs=0 AND !(shown OR traced) THEN
        RECT 68,49,92,111 COLOR black
        RECT 69,50,93,112 COLOR black
        RECT 69,50,91,110 COLOR white FILLED
        CIRCLE 80,74,6,3 COLOR black FILLED
        CIRCLE 80,101,4 COLOR black FILLED
      ENDIF

    ELSEIF locs(x0,y0)=0 THEN
      BEEP
    ENDIF

  ENDIF

2000

UNTIL


SUB GETPEN

  PEN ON
  REPEAT : DELAY 25 : UNTIL PEN(0)
  WHILE PEN(3) : DELAY 25 : WEND
  p4=PEN(4)
  p5=PEN(5)
  PEN OFF

  inlcol=p4<=slv
  inrcol=p4>slv+box*bxs
  introw=p5<=slv
  inbrow=p5>slv+box*bxs
  incor=(inlcol+inrcol+introw+inbrow)>1
  inslv=inlcol OR inrcol OR introw OR inbrow
  x0=MAX(MIN((p4-slv+box-1)\box,bxs+1),0)-1
  y0=MAX(MIN((p5-slv+box-1)\box,bxs+1),0)-1

  IF debug2 AND d2on THEN
    AT 0,136
    PRINT f$;"p4=";p4;"   ";n$;
    AT 0,148
    PRINT f$;"p5=";p5;"   ";n$;
  ENDIF

END


SUB SHOWCELL (x,y,fill)

  CIRCLE box*x+box/2+slv,box*y+box/2+slv,box/3 COLOR fill FILLED

END


SUB SHOWOBJCT

  IF !(debug2 AND d2on) THEN
    i=STR$(objs)
    AT slv+box*bxs+(slv-TXTW(i)+2)/2-3,(slv-TXTH(i))/2
    PRINT " ";i;"  ";
  ENDIF

END


END
