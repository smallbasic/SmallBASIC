#!/usr/local/bin/sbasic -g
' keys.bas
' 11/02/2001

while 1
	k=inkey
	if len(k)
		if len(k)=2
			? "H/W #"+asc(right(k,1))
		else
			? k; " "; asc(k)
		fi
	fi
wend
