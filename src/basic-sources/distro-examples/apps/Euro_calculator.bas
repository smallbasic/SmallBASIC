#!/usr/bin/sbasic
'
'	EURO calculator
'

const euro=340.75
opt1="  DRS to Euro  "
opt2="  Euro to DRS  "

cls
? chr$(27)+"[81m";
? "Euro <-> Drs convertor"
? chr$(27)+"[80m";
? "A SmallBASIC example"
line 0,txth("Q")*2,xmax,txth("Q")*2

label select
? chr$(27)+"[87m";
at 40,50:? opt1
at 40,70:? opt2

pen on
while (pen(0)=0):wend
y=pen(5)
pen off

color 15,0
if y<70
	at 40,50:? opt1
else
	at 40,70:? opt2
fi
color 0,15

at 40,110:? chr$(27)+"[K ";
at 40,130:? chr$(27)+"[K= ";
at 40,110:input k

at 50,130
if y<70
	?round(k/euro,2)," ";chr$(128)
else
	? round(euro*k)," drs"
fi
goto select
