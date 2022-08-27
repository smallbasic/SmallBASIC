' TinyBASIC, by Nicholas Christopoulos
' A SmallBASIC example :)

DIM variables(26)	' variables (one for each letter)
DIM stack(10)		' executor's stack (GOSUB/FOR-NEXT/WHILE-WEND)
DIM labels(), program()

DEF varidx(name) = asc(left(name))-65

ip = -1	' next command to execute (-1 = none, -2 = error)
sp = 0	' stack pointer

CLS
print cat(2);"TinyBASIC v1";cat(-2)
print "A 450-line (with expression parser) SmallBASIC example"
print
print "Type HELP for catalog."
print "Type QUIT to exit..."
print 
print "READY"
print

repeat
	input "> ", inpstr
	inpstr = trim(upper(inpstr))
	cmd = trim(leftof(inpstr+" ", " "))		' get command name
	if len(cmd)
		par = trim(rightof(inpstr, " "))
		if isnumber(cmd)					' store command
			addcmd val(cmd), par
		else								' execute command
			execute cmd, par
		fi
	fi
until cmd="QUIT"
end

' Store command to memory
sub addcmd(num, cmd)
local i, ins, rep

ins = len(labels): rep = -1
for i = 0 to len(labels)-1
	if labels(i) = num then rep=i:exit
	if labels(i) > num then ins=i:exit
next
if rep = -1			' new record
	if len(cmd)		' no error, insert (or append)
		insert labels, ins, num
		insert program, ins, cmd
	fi
else			
	if len(cmd)		' replace
		program(rep) = cmd
	else			' erase
		delete labels, rep
		delete program, rep
	fi
fi
end

' set value to a variable
sub setvar(varname, varval)
local	idx

if len(varname)>1
	TBError "ILLEGAL VARIABLES NAME, USE ONE-CHAR NAMES"
else
	idx = varidx(varname)
	varval = trim(varval)
	if left(varvar) = chr(34) ' it is a string
		variables(idx) = disclose(varval)
	else	' it is an expression
		variables(idx) = tbeval(varval)
	fi
fi
end

' execute a TB command
sub execute(cmd, par)
local idx, i, var, vstr
local parA, tstr, fstr, f, num

if cmd in ["END", "NEW"]
	' new program or end of program; syntax: NEW or END
	ip = -1
	sp = 0
	if cmd="NEW"
		erase labels, commands ' clear program
		dim variables(26) ' clear variables
		print:print "* DONE *":print
	fi
elif cmd in ["QUIT", "REM"]
	' do nothing
elif cmd="LET"
	' assigns a value to a variable; syntax: LET variable = expression
	sinput par; var, "=", vstr
	setvar trim(var), trim(vstr)
elif cmd="LIST"
	' prints the program, syntax: LIST
	if len(labels) 
		for i=0 to len(labels)-1
			print using "####: &"; labels(i); program(i)
		next
	else
		TBError "NO PROGRAM IN MEMORY"
	fi
elif cmd="RUN"
	' run the program, syntax: RUN
	ip = 0
	while ip<len(labels)
		last_ip = ip

		cmd = trim(leftof(program(ip)+" ", " "))
		par = trim(rightof(program(ip), " "))
		execute cmd, par

		if ip = -2
			print "* ERROR AT ";labels(last_ip);" *"
			sp = 0
			exit
		elif ip = -1
			print:print "* DONE *":print
			sp = 0
			exit
		else
			ip = ip + 1
		fi
	wend
elif cmd="INPUT"
	' get a value form console, syntax: INPUT [prompt,] variable
	split par, ",", para, chr(34)+chr(34) use trim(x)
	if len(para) = 0
		ip = -2
	else
		if len(para) = 2
			idx = 1
			input disclose(para(i)); vstr
		else
			idx = 0
			input "? ", vstr
		fi
		setvar para(idx), vstr
	fi
elif cmd="PRINT"
	' print to console, syntax: PRINT [var1 [, varN]]
	split par, ",", para, chr(34)+chr(34)+"()" use trim(x)
	for vstr in para
		if left(vstr)=chr(34)		' print string
			print disclose(vstr); " ";
		else						' print number (expression)
			print tbeval(vstr); " ";
		fi
	next
	print
elif cmd in ["GOTO", "GOSUB"]
	' Syntax: GOTO line or GOSUB line
	search labels, val(par), idx
	if idx = -1
		TBError "LABEL "+par+" DOES NOT EXIST"
	else
		if cmd="GOSUB"
			stack(sp) = ["R", ip]	' "R" = a 'return' command  must read it
			sp = sp + 1
		fi
		ip = idx-1
	fi
elif cmd="RETURN"
	' syntax: RETURN
	if sp > 0
		sp = sp - 1
		if stack(sp)(0) = "R"	' later you can add code for FOR and WHILE 
			ip = stack(sp)(1)
		else
			TBError "STACK MESS"
		fi
	else
		TBError "STACK UNDERFLOW"
	fi
elif cmd="IF"
	' IF! what else?. Syntax: IF expression THEN line [ ELSE line ] 
	sinput par; vstr, " THEN ", tstr, " ELSE ", fstr
	if tbeval(vstr)
		execute "GOTO",tstr
	elif len(fstr)
		execute "GOTO",fstr
	fi
elif cmd="SAVE"
	f=disclose(par)
	if len(f)=0
		TBError "MISSING: FILENAME"
	else
		if isarray(labels) 
		if instr(f, ".TBAS")=0 THEN f=f+".tbas" ELSE f=leftoflast(f, ".TBAS")+".tbas"
			open f for output as #1
			for i=0 to len(labels)-1
				print #1; labels(i); " "; program(i)
			next
			close #1
			print:print "* DONE *":print
		else
			TBError "NO PROGRAM IN MEMORY"
		fi
	fi
elif cmd="LOAD"
	f=disclose(par)
	if len(f)=0
		TBError "MISSING: FILENAME"
	else
		ip = -1
		sp = 0
		erase labels, commands ' clear program
		dim variables(26) ' clear variables
		if instr(f, ".TBAS")=0 THEN f=f+".tbas" ELSE f=leftoflast(f, ".TBAS")+".tbas"
		open f for input as #1
		while not eof(1)
			line input #1; vstr
			num  = leftof (vstr, " ")
			par  = rightof(vstr, " ")
			addcmd val(num), par
		wend
		close #1
		print:print "* DONE *":print
	fi
elif cmd="FILES"
	print files("*.tbas")
elif cmd="HELP"
	PRINT
	print "  ";cat(2);"TinyBASIC, v1";cat(-2)
	PRINT
	print "  * All variables are real numbers."
	print "  * There are 26 variables, one for each letter"
	print "  * INPUT return real number (not string)"
	print "  * IF-THEN accepts only line-numbers (IF x THEN line ELSE line)"
	print "  * PRINT uses only , as separator"
	PRINT
	print "  HELP";tab(15);"This screen"
	print "  NEW";tab(15);"New program"
	print "  RUN";tab(15);"Run program"
	print "  LIST";tab(15);"Prints program to screen"
	print "  SAVE";tab(15);"Saves program to disk"
	print "  LOAD";tab(15);"Loads a program from disk"
	print "  FILES";tab(15);"Prints the list of TB programs"
	print "  REM";tab(15);"Remarks"
	print "  GOTO";tab(15);"Transfers control to ..."
	print "  LET";tab(15);"Assigns a value to a variable"
	print "  PRINT";tab(15);"Prints an expression"
	print "  INPUT";tab(15);"Inputs a value"
	print "  IF";tab(15);" "
	print "  GOSUB";tab(15);" "
	print "  RETURN";tab(15);" "
	print "  END";tab(15);"Terminate the program"
	PRINT
else
	TBError "BAD COMMAND"
fi
end

' Run-time error
sub TBError(errmsg)
PRINT
print chr(7);"* ";errmsg;" *"
PRINT
ip = -2
end

' ==== expression parser ====

' evaluate an expression
def TBEval(expr)
local result, rmn, c

result = 0
expr = ltrim(expr)
if len(expr) then logical result, expr
TBEval = result
end

' number
def valueof(byref expr)
local c, i, v

for i=1 to len(expr)
	c = mid(expr, i, 1)
	if not (c in "0123456789.") then exit
next
if i < len(expr)
	v = left(expr, i-1)
	expr = mid(expr, i)
else
	v = expr
	expr = ""
fi
valueof = val(v)
end

' operators: ( ) or value
sub parenth(byref l, byref expr)
local	op, vname

op = left(expr)
if op = "("
	expr = mid(expr, 2)
	logical l, expr
	if left(expr)=")" then expr = mid(expr, 2)
else
	if op in "0123456789."
		l = valueof(expr)
	' elif, check for function
	else ' variable
		l = variables(varidx(expr))
		expr = if(len(expr)>1, mid(expr, 2), "")
	fi
fi
end

' unary operators: - + NOT
sub unary(byref l, byref expr)
local op

if left(expr,3) = "NOT"
	op="NOT"
	expr = mid(expr,4)
elif left(expr,1) in ["-", "+"]
	op=left(expr)
	expr=mid(expr,2)
fi
parenth l, expr
if op="NOT"
	l = NOT l
elif op="-"
	l = -l
elif op="+"
	' ignore it
fi
end

' operators: * /
sub muldiv(byref l, byref expr)
local	op, r

unary l, expr
while left(expr) in "*/"
	op = left(expr)
	expr = mid(expr, 2)
	unary r, expr
	if op = "*"
		l *= r
	elif op = "/"
		if r=0 
			TBError "DIVISION BY ZERO" 
		else
			l /= r
		fi
	fi
wend
end

' operators: + -
sub addsub(byref l, byref expr)
local	op, r

muldiv l, expr
while left(expr) in "+-"
	op = left(expr)
	expr = mid(expr, 2)
	muldiv r, expr
	if op = "+" 
		l += r
	elif op = "-"
		l -= r
	fi
wend
end

' returns the logical operator
func getlogopr(expr)
local	idx, op3, op2, op1

op3=["AND"]
op2=["OR", "<=", ">=", "=<", "=>", "<>"]
op1=["=", ">", "<"]
search op3, left(expr,3), idx
if idx >= 0 then getlogopr=op3(idx):exit
search op2, left(expr,2), idx
if idx >= 0 then getlogopr=op2(idx):exit
search op1, left(expr,1), idx
if idx >= 0 then getlogopr=op1(idx):exit
getlogopr=""
end

' logical and comparation operators
sub logical(byref l, byref expr)
local	op, r

addsub l, expr
while getlogopr(expr) <> ""
	op = getlogopr(expr)
	expr = mid(expr, len(op)+1)
	addsub r, expr
	if op = "AND"
		l = l AND r
	elif op = "OR"
		l = l OR r
	elif op = "="
		l = (l = r)
	elif op = "<"
		l = l < r
	elif op = ">"
		l = l > r
	elif op = ">=" or op = "=>"
		l = l >= r
	elif op = "<=" or op = "=<"
		l = l <= r
	elif op = "<>"
		l = l <> r
	fi
wend
end
