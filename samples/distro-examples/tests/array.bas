#!/usr/bin/sbasic

? cat(1);"TEST:";cat(0);" Arrays, unound, lbound"

dim m(1 to 3, -1 to 1)

m(1, -1) = 1
m(2,  0) = 2
m(3,  1) = 3
if m(1,-1)<>1 then ? "ERR 1"
if m(2,0)<>2 then ? "ERR 2"
if m(3,1)<>3 then ? "ERR 3"

for i=1 to 3
	for j=-1 to 1
		m(i,j)=(i*10)+j
	next
next
for i=1 to 3
	for j=-1 to 1
		if m(i,j)<>(i*10)+j then
			? "ERROR (";i;",";j;")"
		fi
	next
next

if lbound(m)<>1 then ?"LBOUND() ERROR"
if ubound(m)<>3 then ?"UBOUND() ERROR"
if lbound(m,2)<>-1 then ?"LBOUND() ERROR"
if ubound(m,2)<>1 then ?"UBOUND() ERROR"

if (isarray(m) == false) then
  throw "m is not an array"
end if

m2 = array("[1,2,3,\"other\"]")
if (isarray(m2) == false) then
  throw "m2 is not an array"
end if

if (isnumber(m2(0)) == false) then
  throw "m2(0) is not an number"
end if

if (isstring(m2(3)) == false) then
  throw "m2(3) is not an string"
end if

m3 = array("{\"cat\":{\"name\":\"lots\"},\"other\":\"thing\",\"zz\":\"thing\",\"zz\":\"memleak\"}")
if (ismap(m3) == false) then
  throw "m3 is not an map"
end if

m4 = byref m3
if (isref(m4) == false) then
  throw "m3 is not an ref"
end if

if m4.cat.name <> "lots" then
  ? m3
  ? m4
  throw "ref/map error"
end if

print "array: " + m4

dim sim
sim << 100
sim(0) --
sim(0) ++
sim(0) /= 2
if (sim(0) <> 50) then
  throw "dim sim not tasty"
fi

rem ==6866== Source and destination overlap in memcpy(0xfc3f090, 0xfc3f096, 13)
rem ==6866==    at 0x4C32513: memcpy@@GLIBC_2.14 (in /usr/lib/valgrind/vgpreload_memcheck-amd64-linux.so)
rem ==6866==    by 0x48D4A3: memcpy (string3.h:53)
rem ==6866==    by 0x48D4A3: comp_text_line_let (scan.c:1874)

dim dots(1)
dots(0).y = 100
dots(0).dy = 1
dots(0).y += dots(0).dy
if (dots(0).y != 101) then
   throw "not 101 !!!"
endif

arr1 = [ 1 , 2 , 3 ; 4 , 5 , 6 ; 7 , 8 , 9       ]
arr2 = array("   [1,2,3;4,5,6;7,8,9]            ")
arr3 = array("[1,2,3    ;4,5,6;           7,8,9]")
if (arr1 != arr2 || arr2 != arr3) then
  throw "array err"
endif

const wormHoles = [[4,  4],  [4, 20], [20,  20], [20, 4], [12,12] ]
if (str(wormHoles) != "[[4,4],[4,20],[20,20],[20,4],[12,12]]") then
  throw wormHoles
endif
