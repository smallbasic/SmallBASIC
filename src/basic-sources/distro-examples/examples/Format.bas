'
'	Using the PRINT USING and the FORMAT$()
'
RANDOMIZE TIMER
FOR i=0 TO 19
	a << RND*10000
NEXT

? "Display 19 random values"
?
PRINT USG "##: #,###,##0.00 _+";
FOR i=0 TO 19
	PRINT USG; i+1, a(i)
	s = s + a(i)
NEXT
?         "__________________"
?
? FORMAT$("Total ###,###.#0 =", s)
