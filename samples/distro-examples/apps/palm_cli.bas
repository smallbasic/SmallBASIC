' main

' select fixed font
font=92
PRINT CAT(font)

' lines per page
lpp=int((ymax+1)/txth("Q"))

' chars per line
IF txtw("i")=txtw("W")
	cpl=int((xmax+1)/txtw("Q"))
ELSE
	cpl=25
FI

PRINT cat(1);"Bob's CLI";cat(0)
PRINT:PRINT CAT(font);
PRINT "Term=";cpl;"x";lpp
PRINT "Press h for help"
PRINT 
REPEAT

	PRINT "* ";
	LINE INPUT cmd$
	SPLIT cmd$," ",cmd

	IF NOT EMPTY(cmd)
		c = cmd(0)
		IF LEN(cmd)=1 THEN cmd=[cmd(0),"*"]
		IF c = "ls" THEN
			DO_LS 
		ELIF c = "cp" OR c = "mv" THEN
			DO_MV 
		ELIF c = "rm" THEN
			DO_DEL
		ELIF c = "nf" THEN
			DO_NF 
		ELIF c = "cat" THEN
			DO_CAT
		ELIF c = "h"
			DoHelp 
		ELIF c = "q"
			EXIT
		ELSE
			? "* BAD COMMAND *"
		ENDIF
		PRINT
	ENDIF

UNTIL c="q"
END

' ------------------------
' Help
Sub DoHelp
local hlp

hlp << "Commands"
hlp << "File names syntax:"
hlp << "  [{m|p|d}:][cards]"
hlp << ""
hlp << "  m: memo"
hlp << "  p: or d: pdoc"
hlp << "  otherwise common files"
hlp << ""
hlp << "  valid wildcards characters"
hlp << "  ? any one char"
hlp << "  * any chars"
hlp << "  [] charset"
hlp << ""
hlp << "ls <file>"
hlp << "  list of files"
hlp << "cp <source> <target>"
hlp << "  copy file"
hlp << "mv <source> <target>"
hlp << "  move file"
hlp << "rm <file>"
hlp << "  remove file"
hlp << "nf <file>"
hlp << "  ... @eof"
hlp << "  new file"
hlp << "cat"
hlp << "  print files"
hlp << "h"
hlp << "  This screen"
hlp << "q"
hlp << "  Quit"
typearray hlp
End

' ------------------------
' replaces tabs with spaces
func reptab(s,tabsize)
local ret, idx, r, l
local sc

ret=s
repeat
	idx=instr(ret, chr(9))
	if idx
		sc=idx mod tabsize
		if sc=0
			sc=tabsize
		else
			sc=tabsize-sc
		fi
		l=leftof(ret,chr(9))
		r=rightof(ret,chr(9))
		ret=l+space(sc)+r
	fi
until idx=0
reptab=ret
end

' ------------------------
'
Sub typearray(byref lines())
Local l,f,j,cont,brk
Local tcpl

l=1:j=1
j=1
tcpl=cpl-5
brk=false
FOR f IN lines
	f=reptab(f,4)

	REPEAT
       	IF j MOD lpp = 0 THEN
			IF Q("MORE") THEN
				brk=true
				EXIT
			fi
        ENDIF
		cont = LEN(f)>tcpl
        IF cont THEN
			f1 = LEFT$(f,tcpl)
            f = MID$(f,tcpl+1)
        ELSE
			f1 = f
		ENDIF
		PRINT l;TAB(4);":";
		PRINT TAB(5);f1
		j = j + 1
	UNTIL cont=0
	IF brk THEN EXIT
	l=l+1
NEXT
End

' ------------------------
'
Sub typefile(fname)
Local l,h,f,j,cont,brk
Local tcpl

l=1:j=1
h=FREEFILE
j=1
tcpl=cpl-5
brk=false
OPEN fname FOR INPUT AS #h
WHILE NOT EOF(h)
	LINEINPUT #h;f
	f=reptab(f,4)

	REPEAT
       	IF j MOD lpp = 0 THEN
			IF Q("MORE") THEN
				brk=true
				EXIT
			fi
        ENDIF
		cont = LEN(f)>tcpl
        IF cont THEN
			f1 = LEFT$(f,tcpl)
            f = MID$(f,tcpl+1)
        ELSE
			f1 = f
		ENDIF
		PRINT l;TAB(4);":";
		PRINT TAB(5);f1
		j = j + 1
	UNTIL cont=0
	IF brk THEN EXIT
	l=l+1
WEND
CLOSE #h
End

' ------------------------
'
Sub	DO_CAT
Local ci,f

FOR ci=1 TO UBOUND(cmd)
	IF PARSE(cmd(ci))
		IF ci > 1 THEN
			IF Q("NEXT") THEN EXIT
		ENDIF
        EnumFiles

		IF LEN(flist) 
			FOR i=0 TO LEN(flist)-1
				TypeFile dir+flist(i)
				IF i<>LEN(flist)-1
					IF Q("NEXT") THEN EXIT
				FI
			NEXT
		ELSE
          PRINT "No files in '";cmd(ci);"'"
		FI
	ELSE
    	PRINT "Invalid field: '";cmd(ci);"'"
    ENDIF
NEXT
END

' ------------------------
'
SUB DO_NF
Local ci

FOR ci=1 TO UBOUND(cmd)
	IF PARSE(cmd(ci))
		IF INSTR(fn,"*") OR INSTR(fn,"?") THEN
			PRINT "Ambiguous field: '";cmd(ci);"'"
		ELSE
			IF EXIST(spec) THEN
				INPUT "Replace '" + spec + "' ";ans
	            IF LCASE$(LEFT$(ans,1)) <> "y" THEN
					c=""
	            ENDIF
			ENDIF
			IF c <> "" THEN
				h = FREEFILE
	            OPEN spec FOR OUTPUT AS #h
	            REPEAT
	               INPUT ">",f
	               IF LCASE$(f) = "@eof" THEN
						EXIT LOOP
	               ENDIF
	               IF LEFT$(f,1) = "'" THEN
						f=MID$(f,2)
	               ENDIF
	               PRINT #h,f
				UNTIL 0
				CLOSE #h
				PRINT "Created '";spec;"'"
			ENDIF
		ENDIF
	ELSE
		PRINT "Invalid field: '";cmd(ci);"'"
	ENDIF
NEXT
END

' ------------------------
'
SUB DO_DEL
Local f, ci

FOR ci=1 TO UBOUND(cmd)
	IF Parse(cmd(ci))
		EnumFiles
		IF LEN(flist)
			FOR f IN flist
				INPUT "Delete '" + dir + f + "' ";ans
				IF LCASE$(LEFT$(ans,1)) = "y"
					KILL dir+f
					PRINT "Deleted"
				ENDIF
			NEXT
		ELSE
			PRINT "No files in '";spec;"'"
		ENDIF
	ELSE
		PRINT "Invalid field: '";par;"'"
	ENDIF
NEXT
END

' ------------------------
'
SUB DO_MV
Local f, ci

FOR ci=1 TO UBOUND(cmd)
    IF UBOUND(cmd) < 2 THEN
		cmd = [cmd(0),cmd(1),""]
    ENDIF
    IF NOT (PARSE (cmd(2)))
		PRINT "Invalid field: '";cmd(2);"'"
    ELSE
		IF INSTR(fn,"*") OR INSTR(fn,"?") THEN
			PRINT "Ambiguous destination"
		ELSE
	        ddir = dir
	        dfn = fn
	        dspec = ddir + dfn
	        IF NOT PARSE( cmd(1))
	          PRINT "Invalid field: '";cmd(1);"'"
	        ELSE
	          EnumFiles
	          j = 0
	          FOR i = 0 TO fcount - 1
	            sspec = dir + flist(i)
	              IF LEN(dfn) = 0 THEN
	                dspec = ddir + flist(i)
	              ENDIF
	              IF sspec = dspec THEN
	                PRINT "Cannot copy/move file to itself"
	              ELSE
	                IF EXIST(dspec) THEN
	                  INPUT "Overwrite '" + dspec + "' with '" + sspec + "' ";ans
	                  IF LCASE$(LEFT$(ans,1)) = "y" THEN
	                    'KILL dspec
	                  ELSE
	                    c = ""
	                  ENDIF
	                ENDIF
	                IF c <> "" AND ddir = "MEMO:" AND dir <> "MEMO:" THEN
	                  h = FREEFILE
	                  OPEN sspec FOR INPUT AS #h
	                  IF LOF(h) > 3935 THEN
	                    PRINT "Too large for MEMO: '";sspec;"'"
	                    c = ""
	                  ENDIF
	                ENDIF
	                IF c = "cp" THEN
	                  COPY sspec,dspec
	                  PRINT "Copied '";sspec;"' to '";dspec;"'"
	                  j = j + 1
	                ELSEIF c = "mv" THEN
	                  RENAME sspec,dspec
	                  PRINT "Moved '";sspec;"' to '";dspec;"'"
	                  j = j + 1
	                ENDIF
	            ENDIF
	          NEXT
	          IF j = 0 THEN
	            PRINT "Nothing to copy"
	          ENDIF
	        ENDIF
	      ENDIF
	    ENDIF
NEXT	  
END

' ------------------------
'
Sub DispFileInfo(fname)
Local f, x, tcpl

tcpl=cpl-7
' cut filename
IF LEN(fname) > tcpl THEN
	f = LEFT$(fname,tcpl-1) + CHR$(133)
ELSE
	f = flist(i)
FI

' print file-size
x=dir+fname
IF ISFILE(x)
	IF ACCESS(x)
		h = FREEFILE
		OPEN x FOR INPUT AS #h
		PRINT LOF(h);TAB(7);f
		CLOSE #h
	ELSE
		PRINT "N/A";TAB(7);f
	FI
ELIF ISDIR(x)
    PRINT "<DIR>";TAB(7);f
FI
End

' ------------------------
'
Sub DO_LS
Local j, ci

j=0
FOR ci=1 TO UBOUND(cmd)
	IF Parse(cmd(ci))
		IF ci>1 AND j>0
			IF Q("NEXT") THEN EXIT
		FI

		EnumFiles
		j = 0
        FOR i=0 TO fcount - 1
			IF j MOD lpp = 0
				IF j > 0
					IF Q("MORE") THEN EXIT
				ELSE
					PRINT "List of ";spec
	   			FI
			FI

			' print fileinfo
			DispFileInfo flist(i)
   			j = j + 1
   		NEXT
		IF j = 0 THEN ? "No files in '";spec;"'"
	ELSE
		PRINT "Invalid field: '";cmd(ci);"'"
		EXIT
   	ENDIF
NEXT
END

' ------------------------
'
Def Q(msg)
Local ans

	Q=False

	PRINT "* ";msg;" (Y/N) *";
	REPEAT
		ans=INKEY$
	UNTIL ans<>""
	PRINT
	IF INSTR("Nn",ans) THEN Q=True
End

' ------------------------
'
Func Parse(field)
Local f,p,pc
Local ermsg

	' init
	Parse=False
	ermsg="Usage: [{m|p|d}:]file"

	' 
	Split field,":",p 
	pc=len(p)
	IF pc>2
		? ermsg
		EXIT
	ELIF pc=1
		p=["",field]
	FI

	dir=p(0)
	f=left(dir,1)
	IF f="m"
		dir="MEMO:"
	ELIF f="p" OR f="d"
		dir="PDOC:"
	ELIF len(dir)
		? ermsg
		EXIT
	FI
	fn=p(1)
	mask=IF(fn,fn,"*")
	spec=dir+mask

	' exit
	Parse=true
End

' ------------------------
'
Sub EnumFiles
	flist=Files(spec)
	fcount=Len(flist)
End


