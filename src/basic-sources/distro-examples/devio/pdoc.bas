'
'	Using PDOC VFS driver
'
'	This program copies the pdoc file 'Shell'
'	to file 's2'
'
open "pdoc:Shell" as #1
open "pdoc:s2" for output as #2
while not eof(1)
    lineinput #1, s
    print #2, s
wend
close #1
close #2



