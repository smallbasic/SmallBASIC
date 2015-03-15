#!/usr/local/bin/sbasic -g

b1=chr$(27)+"[1m"
b0=chr$(27)+"[21m"
t$=chr$(9)

?b1;"System Information";b0
?
if sbver>=0x506
	? "OS:",osname;
	? "SB:",sbver
	? "Display",xmax+1;"x";ymax+1
	? "Colors",2^bpp
	? "Font:", txtw("a");"x";txth("a")
else
	? "SB: <= 0.5.5"
fi
?
? "Total free memory: "; round(fre(0)/1024);" KB"
? "Stack size: "; round(fre(-2)/1024);" KB"
? "Largest free memory block: "; round(fre(-3)/1024);" KB"

