
dim va(4)

for i=0 to 4
  va(i)=i
next

' array copy
vn()=va()
for i=0 to 4
	if ( va(i) <> vn(i) )
		throw "array copy error"
	fi
next

' ==== procedures

proc1
proc2 "String test"
proc2 1/2
proc3 1,2
proc4 4

i=-1
proc5 va(),4
if i<>-1 then ? "PROC5 failed" else ? "proc5: ok"

i=-1
proc6 i
if i<>6 then ? "PROC6 failed" else ? "proc6: ok"

proc6 va(1)
if va(1)<>6 then ? "PROC6 failed" else ? "proc6: ok"

' recursion
i=1
rec i

'==== functions

? "f(x)=";f(8)
x=8
? f(1)+1+sqr(f(x))+4

'==== return array!
vn()=fillarray
for i=0 to ubound(vn)
  ? vn(i)
next

' passing arrays byref
fillarr2 vn()
for i=0 to ubound(vn)
  ? vn(i)
next

' ===
dim vv(4)
vv()=fill3(vv())

' new code
vv=fill4(vv)

end

' =====================================

' Simple procedure
sub proc1
? "Proc1: no parameters"
end

' 1 parameter procedure
sub proc2(a)
? "Proc2=";a
end

' 2 parameter procedure
sub proc3(a,b)
? "Proc3=";a+b
end

' local procedures
sub proc4(a)

  sub proc5(b)

    sub proc6(b)
      ? "sub-proc6=";b
    end

  ? "sub-proc5=";b
  proc6 b
  end

? "Proc4=";a
proc5 a:REM local sub proc5 has priority
end

' null-arrays as parameter
sub proc5(x(),count)
local i

for i=0 to count
  ? "Proc5 - Array="; x(i)
next
end

' byref
sub proc6(byref n)
n=6
end

func f(x)
f=2^x
end

sub rec(byref n)
local s

n=n+1
s=n
? "R:";n
if n<10
  rec n
fi
? "REXIT:";s
end

func fillarray
local v,i

dim v(16)

for i=0 to 16
	v(i) = 16-i
next
fillarray=v()
end

sub fillarr2(byref v())
local i

for i=0 to 16
	v(i) = i
next
end

func fill3(v())
v(0)=1
fill3=v()
end

func fill4(v)
v(0)=1
fill4=v
end


