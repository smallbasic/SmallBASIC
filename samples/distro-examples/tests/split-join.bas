'
' http://smallbasic.sourceforge.net/?q=node/1586
'
dim b(), o()

z="3,18,26,29,30,32,42,45,52,64,65,73,75,81,89;5,7,12,15,25,33,35,44,56,59,60,62,76,79,90;4,10,19,27,28,34,37,43,49,53,57,66,70,82,84;6,13,16,20,39,41,48,51,55,67,69,71,72,85,88;1,8,11,14,21,24,38,40,46,54,58,61,74,80,86;2,9,17,22,23,31,36,47,50,63,68,77,78,83,87"

split z, ";", a
print "split to a: ";a

for f in a
  rem break the ; separate sections into commas separated items
  split f, ",", g
  rem append everything to 'b'
  append b, g
next
print "appended to b: ";b

for m in b
  join m, ",", n
  rem join the b's into n, then append n to o
  append o, n
next
print "appended into o: ";o

rem join the o's back into p, this should now be the same as z, except for ; separators
join o, ",", p
print "joined into p: ";p

if (p <> translate(z, ";", ",")) then
  print p
  print z
  throw "p <> z"
endif

did_fail = false
try
  x=""
  z=""
  join x,",", z
catch
  did_fail = true
end try
if !did_fail then
  throw "Join error"
endif

a1=[,1,2,3,4,5,,7,8,]
a2=["",1,2,3,4,5,"",7,8,""]
if (a1 != a2 or 9 != ubound(a1)) then
  ? a1
  ? a2
  throw "Empty entries not included in array"
endif

s=",1,2,3,4,5,,7,8,"
split s,",",a
if (9 != ubound(a)) then
  ? a
  throw "Final empty entry was ignored"
endif
