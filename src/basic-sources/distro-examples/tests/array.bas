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

