' format byte
func hbyte(c)
if c<16 
	hbyte="0"+hex(c)
else
	hbyte=hex(c)
fi
end

' format address
func haddr(c)
if c<&H1000
	if c<&h100
		if c<&h10
			haddr = "000"+hex(c)
		else
			haddr = "00"+hex(c)
		fi
	else
		haddr = "0"+hex(c)
	fi
else
	haddr=hex(c)
fi
end

open "s2.pdoc" for input as #1
open "test.b" for output as #2
count=0
? haddr(count); ": ";
while !eof(1)
	c=bgetc(1)
	? "0x";hbyte(c);
	bputc #2; c
	count ++
	if (count mod 16) = 0 then ?:? haddr(count);": "; else ? " ";
wend
close #2
close #1

? run("diff s2.pdoc test.b")
kill "test.b"


