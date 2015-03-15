#!/usr/bin/sbasic

? cat(1);"Generic";cat(0)
a=1+1.2
if a<>2.2 then ? "2.2: ERROR!"
a=fre(0)
if 0.5<>1/2 then ? "0.5: ERROR!"
if .6<>(1+2-3*4/5) then ? "0.6: ERROR (acceptable?) <> ";(1+2-3*4/5)

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
	333 let c(10-(i-1))=i
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
