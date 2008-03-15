#!/usr/local/bin/sbasic -g
'
' Using pen() function
'

print "Use MemoPad key to exit"

pen on
while pen(0)=0:wend
pset pen(4),pen(5)
while 1
  if pen(0)
    line pen(4),pen(5)
  fi
wend
pen off


