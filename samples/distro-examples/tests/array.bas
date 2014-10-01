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

m3 = array("{\"cat\":{\"name\":\"lots\"},\"other\":\"thing\",\"zz\":\"thing\"}")
if (ismap(m3) == false) then
  throw "m3 is not an map"
end if

m4 = byref m3
if (isref(m4) == false) then
  throw "m3 is not an ref"
end if

if m4.cat.name <> "lots" then
  throw "ref/map error"
end if

dim sim
sim << 100
sim(0) --
sim(0) ++
sim(0) /= 2
if (sim(0) <> 50) then
  throw "dim sim not tasty"
fi


