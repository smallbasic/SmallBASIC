open "socl:smallbasic.sf.net:80" as #1
'open "socl:127.0.0.1:80" as #1
print #1, "GET http://smallbasic.sf.net/palm/index.shtml"
while ( not eof(1) )
	lineinput #1, s
	print s
wend

