' seq. file I/O test

F=FREEFILE

' Create & Write
OPEN "test.dat" FOR OUTPUT AS #F
PRINT #F, "Hello, world!"
CLOSE #F

' Open & Read
OPEN "test.dat" FOR INPUT AS #F
INPUT #F, a$, b$
PRINT a$,b$
'LINEINPUT #F, a$
CLOSE #F

'
PRINT "I read: [";a$;"]+[";b$;"]"

' Append test
OPEN "test.dat" FOR APPEND AS #F
PRINT #F, "One more text line"
CLOSE #F

' EOF & LINE INPUT TEST
OPEN "test.dat" FOR INPUT AS #F
WHILE NOT EOF(F)
	LINEINPUT #F, a$
	PRINT "NL=[";a$;"]"
WEND
CLOSE #F

# find main.cpp in the console folder
has_main = false
func walker(node)
  if (node.name == "main.cpp") then has_main=true
  return 1
end
dirwalk "./", "*.cpp" use walker(x)
if (!has_main) then throw "dirwalk error"

' allow reading and writing of chr(0)
open "output.dat" for output as #F
for x = 0 to 255
  print #F, chr(x);
next
close #F

open "output.dat" for input as #1
for x = 0 to 255
  d = input(1, 1)
  if asc(d) != x then throw "Invalid file read " + d + " <> " + x
next
close #1

' INPUT #F; now supports up to 64 parameters
open "./output.dat" for output as #2
print #2, "1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16"
close #2
open "./output.dat" for input as #2
dim v[0 to 15]
input #2; v[0], "|", v[1], "|", v[2], "|", v[3], "|", &
          v[4], "|", v[5], "|", v[6], "|", v[7], "|", &
          v[8], "|", v[9], "|", v[10],"|", v[11],"|", &
          v[12],"|", v[13],"|", v[14],"|", v[15],"|"
close #2
if v != [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16] then throw "invalid input"
