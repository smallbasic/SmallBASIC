#!/usr/bin/sbasic -g
DIM A(10)
LET E=5
10 LET A=1
20 CONST B=2
C=3
D=4
CONST F=6
c=a+b+c+d+e+f
IF c<>21 THEN ? "ERROR"
DIM A(10)
a(0)=2
a(1)=3
if a(0)+a(1)<>5 THEN ? "COMMON NAMES ERROR"

x$="abcdef"
if x<>0 then ? "ERROR $1"
if x$<>"abcdef" then ? "ERROR $2"
IF left(x$,2)<>left$(x$,2) then ? "ERROR $3"
