#!/usr/bin/sbasic

? cat(1);"Generic";cat(0)
a=1+1.2
if a<>2.2 then ? "2.2: ERROR!"
a=fre(0)
if 0.5<>1/2 then ? "0.5: ERROR!"
if .6<>(1+2-3*4/5) then ? "0.6 <> ";(1+2-3*4/5)

if -.6<>-.6 then throw "-.6 not -.6"
if ("0.001" <> str(1/1000)) then throw "0.001 not " + str(1/1000)
if ("0.001" <> str(0.001)) then throw "0.001 not " + str(1/1000)

?
? cat(1);"Auto type convertion";cat(0)
? "n=i+n = 2.2  = ";1+1.2
? "n=n+i = 2.2  = ";1.2+1
? "n=n+n = 2.2  = ";1.1+1.1
? "i=i+i = 2    = ";1+1
? "n=i+s = 2.2  = ";1+"1.2"
? "n=i+s = -.1  = ";1+"-1.1"
? "n=n+s = 2.2  = ";1.1+"1.1"
? "n=s+i = 2    = ";"1"+1
? "n=s+n = 2.2  = ";"1"+1.2
? "s=i+s = 1a   = ";1+"a"
? "s=n+s = 1.1a = ";1.1+"a"
? "s=s+i = a1   = ";"a"+1
? "s=s+n = a1.1 = ";"a"+1.1
? "s=s+s = 11   = ";"1"+"1"
?
? cat(1);"Compare";cat(0)
? "s=s   = 1    = ";"1"="1"
? "s=i   = 1    = ";"1"=1
? "s=n   = 1    = ";"1.1"=1.1
? "i=s   = 1    = ";1="1"
? "n=s   = 1    = ";1="1.1"
? "n=n   = 1    = ";1.1=1.1
? "n=i   = 1    = ";1.0=1
? "i=n   = 1    = ";1=1.0
? "i=i   = 1    = ";1=1
? "i>i   = 1    = ";2>1
? "i>=i  = 1    = ";1>=1
? "i<=i  = 1    = ";1<=2
? "i<>i  = 1    = ";1<>2
?
? cat(1);"Array";cat(0)
dim a(10), b(10), c(10)
for i=1 to 10
	a(i)=i
next
for i=1 to 10
	let b(i)=10-i
	let c(10-(i-1))=i
	z=10-(i-1)
	if c(z)<>i then ? "error"
next
for i=1 to 10
	if a(i)+b(i)<>10 then ? "error"
next
let z=a(4)
if z<>a(4) then ? "error"
?
? cat(1);"Nested arrays";cat(0)
A(0)=B
if A(0) <> B then ? "error"
if A(0) = B  then ? "true"
if NOT (A(0) <> B) then ? "true"
?
? cat(1);"Testing base-convertion...";cat(0)
if (0xFF <> 255) then ? "0xFF<>255"
if (0b11 <> 3 ) then ? "0b11<>3"
if (&hFF <> 255) then ? "&HFF<>255"
if (&b11 <> 3 ) then ? "&b11<>3"
? cat(1);"Scientific notation";cat(0);" = 2E+.3 = "; 2e+.3; " --- 2E-2 = 0.02 = "; 2e-2
? cat(1);"Bob's bug";cat(0);" = -30 = ";
k=30
? "-"+k

func func2(a,b)
 func2 = a
end
func func3(a,b,c)
 func3 = 22
end
if (func2(func3(Board,Move,False),b) <> 22) then
  throw "nested func call failed"
endif
rem NOTE: allowing: func2 (arg1), (arg2) broken nested funcs
if (func2((98+1), (2)) <> 99) then
  throw "bracketed args failed"
endif

rem test fix for http://sourceforge.net/p/smallbasic/bugs/34/
steps = 0
end_el = 100
for i = 1 to end_el step 0.1
  steps++
next
expected = 1 + ((end_el-1) * 10)
if (expected != steps) then throw "incorrect step count:" + steps + " " + expected

rem test fix for http://smallbasic.sourceforge.net/?q=node/1444
func assertEq(a, b, pline)
  if a != b
    throw "Line: " + pline + " result: '" + a + "' != expected: '" + b + "'"
  endif
end

assertEq format("\\ \\", "abcde"),   "abc",  PROGLINE
assertEq format(" \\ \\", "abcde"),  " abc", PROGLINE
assertEq format("\\ \\ ", "abcde"),  "abc ", PROGLINE
assertEq format(" \\ \\ ", "abcde"), " abc ",PROGLINE
assertEq format("\\ $ \\", "abcde"), "ab$cd",PROGLINE
assertEq format(" \\$\\", "abcde"),  " a$b", PROGLINE
assertEq format("\\$\\ ", "abcde"),  "a$b ", PROGLINE
assertEq format(" \\$\\ ", "abcde"), " a$b ",PROGLINE
assertEq format(" \\$\\ ", "abcde"), " a$b ",PROGLINE
assertEq format("\\abcde\\", "ABbcde"),  "AabcdeB",PROGLINE

rem http://smallbasic.sourceforge.net/?q=node/1445
rem http://smallbasic.sourceforge.net/?q=node/1461
rem https://en.wikipedia.org/wiki/XOR_gate
rem https://en.wikipedia.org/wiki/XNOR_gate

rem Logical
assertEq 0 OR 0,  FALSE, PROGLINE
assertEq 0 OR 1,  TRUE,  PROGLINE
assertEq 1 OR 0,  TRUE,  PROGLINE
assertEq 1 OR 1,  TRUE,  PROGLINE
assertEq 0 == 0,  TRUE,  PROGLINE
assertEq 0 == 1,  FALSE, PROGLINE
assertEq 1 == 0,  FALSE, PROGLINE
assertEq 1 == 1,  TRUE,  PROGLINE
assertEq 0 != 0,  FALSE, PROGLINE
assertEq 0 != 1,  TRUE,  PROGLINE
assertEq 1 != 0,  TRUE,  PROGLINE
assertEq 1 != 1,  FALSE, PROGLINE
assertEq 0 AND 0, FALSE, PROGLINE
assertEq 0 AND 1, FALSE, PROGLINE
assertEq 1 AND 0, FALSE, PROGLINE
assertEq 1 AND 1, TRUE,  PROGLINE

rem Bitwise
assertEq 0 BOR 0,  0, PROGLINE
assertEq 7 BOR 6,  7, PROGLINE
assertEq 100 BOR   0, 100, PROGLINE
assertEq 1 BOR 1,  1, PROGLINE
assertEq 0 BAND 0, 0, PROGLINE
assertEq 0 BAND 1, 0, PROGLINE
assertEq 1 BAND 0, 0, PROGLINE
assertEq 1 BAND 1, 1, PROGLINE
assertEq 0 NAND 0, 0xffffffffffffffff, PROGLINE
assertEq 0 NAND 1, 0xffffffffffffffff, PROGLINE
assertEq 1 NAND 0, 0xffffffffffffffff, PROGLINE
assertEq 1 NAND 1, 0xfffffffffffffffe, PROGLINE
assertEq 0 NOR 0,  0xffffffffffffffff, PROGLINE
assertEq 0 NOR 1,  0xfffffffffffffffe, PROGLINE
assertEq 1 NOR 0,  0xfffffffffffffffe, PROGLINE
assertEq 1 NOR 1,  0xfffffffffffffffe, PROGLINE
assertEq 0 XOR 0,  0, PROGLINE
assertEq 0 XOR 1,  1,  PROGLINE
assertEq 1 XOR 0,  1,  PROGLINE
assertEq 1 XOR 1,  0, PROGLINE
assertEq 0 XNOR 0, 0xffffffffffffffff,  PROGLINE
assertEq 0 XNOR 1, 0xfffffffffffffffe, PROGLINE
assertEq 1 XNOR 0, 0xfffffffffffffffe, PROGLINE
assertEq 1 XNOR 1, 0xffffffffffffffff,  PROGLINE

' mixed
assertEq FALSE,   0, PROGLINE
assertEq TRUE,    1, PROGLINE
assertEq !FALSE,  1, PROGLINE
assertEq 0 AND 0, 0, PROGLINE
assertEq 0 AND 1, 0, PROGLINE
assertEq 1 AND 1, 1, PROGLINE
assertEq 0 OR 0,  0, PROGLINE
assertEq 0 OR 1,  1, PROGLINE
assertEq 1 OR 1,  1, PROGLINE
assertEq NOT 0,   1, PROGLINE
assertEq NOT 1,   0, PROGLINE
assertEq 0 <> 0,  0, PROGLINE
assertEq 0 <> 1,  1, PROGLINE
assertEq 1 <> 1,  0, PROGLINE

' The IMP operator returns 0 if the first operand is 1 and its second operand is 0.  In all other
' cases, it returns 1. The operation is perform on each bit position in the two arguments.
' !a || b
' 1100
' 1010
' 1011
assertEq 0xcccccccccccccccc Imp 0xaaaaaaaaaaaaaaaa, 0xbbbbbbbbbbbbbbbb, PROGLINE
assertEq 0xcccccccc Imp 0xaaaaaaaa, 0xbbbbbbbb, PROGLINE
assertEq 0xcccc Imp 0xaaaa, 0xbbbb, PROGLINE
assertEq 0xcc Imp 0xaa, 0xbb, PROGLINE
assertEq 0xc Imp 0xa, 0xb, PROGLINE

' The Eqv operator returns 1 if and only if both inputs are equal.
' 1100
' 1010
' 1001
assertEq 0xcccccccccccccccc Eqv 0xaaaaaaaaaaaaaaaa, 0x9999999999999999, PROGLINE
assertEq 0xcccccccc Eqv 0xaaaaaaaa, 0x99999999, PROGLINE
assertEq 0xcccc Eqv 0xaaaa, 0x9999, PROGLINE
assertEq 0xcc Eqv 0xaa, 0x99, PROGLINE
assertEq 0xc Eqv 0xa, 0x9, PROGLINE

' bit shift operators
assertEq 0xFF  LSHIFT 1, 0x1FE, PROGLINE
assertEq 0x1FE RSHIFT 1, 0xFF,  PROGLINE

' value of i when for for does not execute body
for i=10 to 9 step 2:next
assertEq i, 10, PROGLINE
for i=1 to 2 step -1: next
assertEq i, 1, PROGLINE

' 0.12.10
func x
  return 101
end
func f(x)
  if (x==1) then return "x"
  if (x==2) then return "y"
  return x()-1
end
if f(99) != 100 then throw "func return error"

sub my_sub(arg1)
  sub test_sub(arg2)
     if (arg2==111) then return
     print "failed 1"
  end
  test_sub(arg1)
  if (true) then
     if (arg1==111) then return
  endif
  print "failed 2"
  return
end
my_sub(111)

rem --- FUNC method uses SELF for temporary storage
func Grid()
  func get_x
     return self.x
  end
  sub incr_x
     self.x++
  end
  result = {}
  result.x = 0
  result.get_x = @get_x
  result.incr_x = @incr_x
  return result
end

g=Grid()
gx = 0
for i = 0 to 2
  g.incr_x()
  gx = g.get_x()
next i

if (gx != 3) then
  throw "method error"
endif

func GridBox(og)
  func gridWrapper()
    self.my_g.incr_x()
    return self.my_g.x
  end
  result = {}
  result.my_g = og
  result.nested = @gridWrapper
  result.my_x = 3
  return result
end

gb = GridBox(g)
bx = gb.nested()
if (bx != 4) then
  throw "nested method error1: " + bx
endif
bx = gb.my_x
if (bx  != 3) then
  throw "nested method error2: " + bx
endif

rem -------
func tuple()
  return [10,20,30]
end

(a1,b1,c1) = tuple()
if a1 != 10 or b1 != 20 or c1 != 30 then
  throw "packing error"
endif

rem --- test var_eval inside packed let
aaa=[1,2,3]
[_a1,_a2,_a3]=aaa
if _a1 != 1 or _a2 != 2 or _a3 != 3 then
  throw "packing error"
endif

a = nil
b = 0
if a != nil then
  throw "nil error1"
endif
if a == b then
  throw "nil error2"
endif

if (maxint == 0)
  throw "maxint not defined"
endif

local a=[1,2,3,4]
local b=IFF(a[0]==1,999,2)
if (b != 999) then throw "inline local assignment error"

rem ---- cleanup LOCALs from the STACK
frame = 0
while 1
  if (frame == 65536) then
    exit loop
  endif
  rem LOCAL here goes out of scope (poped from stack) in WEND
  rem processing. More efficient to declare local at a higher
  rem block level but need to at least avoid a stack overflow
  local t1 = tick
  frame++
wend

rem ---- optimised array handling
key.x=12
key.y=22
dim aa
aa << key
kk = aa[0]
if (kk.x != 12 or kk.y != 22) then throw "ERR"

rem ensure calling: for i in s.moves()
rem does not cause moves() to be called twice
func KlotskiState()
  func moves()
    if (moves_called) then throw "var_eval error"
    moves_called = true
    return [1,2,3]
  end
  moves_called = false
  for i in moves()
  next i  
  local state = {}
  state.moves = @moves
  return state
end  
s = KlotskiState()
moves_called = false
for i in s.moves()
next i  

