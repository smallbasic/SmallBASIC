#!/usr/bin/sbasic -g
'
'

st=ticks
for i=1 to 1000000:next
et=ticks
? "FOR speed: "; ((et-st)/tickspersec); "sec "; round(1000000/((et-st)/tickspersec));" l/s"

st=ticks
i=0
while i<1000000:i=i+1:wend
et=ticks
? "WHILE speed: "; ((et-st)/tickspersec); "sec "; round(1000000/((et-st)/tickspersec));" l/s"

st=ticks
i=0
repeat:i=i+1:until i>=1000000
et=ticks
? "REPEAT speed: "; ((et-st)/tickspersec); "sec "; round(1000000/((et-st)/tickspersec));" l/s"

