#!/usr/bin/sbasic -g
' console test
' 27/05/2000

reset=chr$(27)+"[0m"
t$=chr$(9)
bp=chr$(7)

boldOn=chr$(27)+"[1m"
underlineOn=chr$(27)+"[4m"
reverseOn=chr$(27)+"[7m"

boldOff=chr$(27)+"[21m"
underlineOff=chr$(27)+"[24m"
reverseOff=chr$(27)+"[27m"

? cat(1);"BASIC CONTROL CHARS TEST";cat(0)
?
? "Tab test"
?  "T1" +t$+ "T2" +t$+ "T3" +t$+ "T4" +t$+ "T5" +t$+ "T6"
? "beep test:",bp
? "too big line... abcdefghijklmnopqrstuvwxyz0123456789"
? underlineOn + "This is underline" + underlineOff
? boldOn + "This is bold" + boldOff
? reverseOn + "This is reversed" + reverseOff
?
? "Press any key...";
pause

# ------------------------------------------------
cls
for i=0 to 8
    for j=0 to 8
	locate j,i*4
	? j;".";i
    next
next
locate 10,0
? "0123456789012345678901234567890"
pause

# ------------------------------------------------
cls
? "SCROLL TEST"
for i=60 to 1 step -1
	? i
next
? "(i=1): Press any key...";
pause
cls
? cat(1);"COLOR TEST";cat(0)
?
for i=0 to 7
	? chr$(27)+"[3"+chr$(i+48)+"m";
	? "Color",i
next
? reset
? "Press any key...";
pause

# ------------------------------------------------
cls
? cat(1);"COLOR TEST";cat(0)
?
for i=0 to 7
	? chr$(27)+"[4"+chr$(i+48)+"m";
	? "Color",i
next
? reset
? "Press any key...";
pause

# ------------------------------------------------
cls
? cat(1);"SYSTEM FONTS";cat(0)
?
for i=0 to 7
	? CHR$(27)+"[8"+chr$(i+48)+"m";
	? "System font #";i
next
? reset
? "Press any key...";
pause

# ------------------------------------------------
cls
? cat(1);"SmallBASIC FONTS";cat(0)
?
for i=0 to 3
	? CHR$(27)+"[9"+chr$(i+48)+"m";
	? "SB font #";i
next
? reset


