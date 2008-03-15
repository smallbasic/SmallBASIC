
' compare-function
func cmp(x(),y())
if x(0)<y(0)
  cmp=-1
elif x(0)>y(0)
  cmp=1
else
  cmp=0
fi
end

' ---------------
dim a(4)

' give 5 random values
for i=0 to 4
  a(i) = rnd
next

' print it unsorted
print a

' sort it
sort a

' print the result
print a

' --------------
dim a(4)

for i=0 to 4
  a(i) = [rnd,rnd]
next

print a
sort a use cmp(x,y)
print a



