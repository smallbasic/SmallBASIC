'test.bas
'25/02/2002
pen on
f=freefile
dim num(6)
dim name$(3)
Kill "MEMO:golfmatch"
cls
for i=1 to 2
input "name",name$(i)
next i
cls
open "MEMO:golfmatch" for output as #f
for i = 1 to 2
print #f, name$(i)
next i
close #f
pen off
end
pen off
