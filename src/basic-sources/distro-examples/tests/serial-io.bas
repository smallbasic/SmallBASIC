' CHAT
' Serial I/O - example
' SmallBASIC 0.6.0

print "Press 'q' for exit..."

open "com1:" AS #1
while !eof(1)
	' receive
	if lof(1)
		k=input$(1,1)
		print k;
	fi

	' send
	k=inkey$
	if k then
		if k="q" then exit
		print #1, k;
		print cat(1);k;cat(0);
	fi
wend
close #1

