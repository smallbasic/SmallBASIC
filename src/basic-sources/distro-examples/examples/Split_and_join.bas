s="/etc/abc/filename.ext"

?
? "Original: "; s
?
? "Split it"
?
SPLIT s, "/."+CHR$(10), v()
for i=0 to ubound(v)
	? i;" [";v(i);"]"
next

?
? "Join it"
?
join v(),"/",z
? z

